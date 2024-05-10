#include "config.hpp"
#include "dynamic_micro_op.h"
#include "log.h"
#include "core_model.h"

DynamicMicroOp::DynamicMicroOp(const MicroOp *uop, const CoreModel *core_model, ComponentPeriod period)
   : m_uop(uop)
   , m_core_model(core_model)
   , m_period(period)
   , m_vector_issue_times_max(Sim()->getCfg()->getInt("general/vlen") / Sim()->getCfg()->getInt("general/dlen"))
{
   LOG_ASSERT_ERROR(period != SubsecondTime::Zero(), "MicroOp Period is == SubsecondTime::Zero()");

   this->squashed = false;

   this->branchTaken = false;
   this->branchMispredicted = false;

   this->intraInstructionDependencies = uop->intraInstructionDependencies;
   this->microOpTypeOffset = uop->microOpTypeOffset;
   this->squashedCount = 0;
   this->dependenciesLength = 0;
   this->commitDependency = UINT64_MAX;

   this->execLatency = m_core_model->getInstructionLatency(uop);

   this->sequenceNumber = INVALID_SEQNR;

   this->dCacheHitWhere = HitWhere::UNKNOWN;
   this->iCacheHitWhere = HitWhere::L1I; // Default to an icache hit
   this->iCacheLatency = 0;

   this->m_forceLongLatencyLoad = false;

   for(uint32_t i = 0 ; i < MAXIMUM_NUMBER_OF_DEPENDENCIES; i++)
      this->dependencies[i] = -1;
   for (uint32_t i = 0; i < MAXIMUM_NUMBER_OF_DEPENDENCIES; i++)
      this->initial_dependencies[i] = -1;
   LOG_ASSERT_ERROR(m_uop != NULL, "uop is NULL");

   this->regDependenciesLength = 0;
   for (uint32_t i = 0; i < MAXIMUM_NUMBER_OF_DEPENDENCIES; i++)
      this->reg_dependencies[i] = -1;

   first = m_uop->isFirst();
   last = m_uop->isLast();

   this->m_mem_access_merge = false;
   this->is_virtually_issued = false;
   this->mergedInstCount = 0;

   this->preload_done = false;
}

DynamicMicroOp::~DynamicMicroOp()
{
}

void DynamicMicroOp::squash(std::vector<DynamicMicroOp*>* array)
{
   squashed = true;

   if (array)
   {
      // Fix up isFirst/isLast after possibly squashing the first/last microop in a list
      for(int index = 0; index < (int)array->size(); ++index)
         if (!(*array)[index]->isSquashed())
         {
            (*array)[index]->setFirst(true);
            break;
         }
      for(int index = array->size() - 1; index >= 0; --index)
         if (!(*array)[index]->isSquashed())
         {
            (*array)[index]->setLast(true);
            break;
         }
   }
}

uint64_t DynamicMicroOp::getDependency(uint32_t index) const
{
   if (index < this->intraInstructionDependencies) {
      return this->sequenceNumber - this->microOpTypeOffset - this->intraInstructionDependencies + index;
   } else {
      assert((index >= this->intraInstructionDependencies) && ((index - this->intraInstructionDependencies) < this->dependenciesLength));
      return this->dependencies[index - this->intraInstructionDependencies];
   }
}

void DynamicMicroOp::addDependency(uint64_t sequenceNumber)
{
   if (!Tools::contains(dependencies, dependenciesLength, sequenceNumber)) {
      assert(this->dependenciesLength < MAXIMUM_NUMBER_OF_DEPENDENCIES);
      dependencies[dependenciesLength] = sequenceNumber;
      dependenciesLength++;
   }
}

void DynamicMicroOp::removeDependency(uint64_t sequenceNumber)
{
   if (sequenceNumber >= this->sequenceNumber - this->microOpTypeOffset - this->intraInstructionDependencies) {
      // Intra-instruction dependency
      while(intraInstructionDependencies && !(sequenceNumber == this->sequenceNumber - this->microOpTypeOffset - this->intraInstructionDependencies)) {
         // Remove the first intra-instruction dependency, but since this is not the one to be removed, add it to the regular dependencies list
         dependencies[dependenciesLength] = this->sequenceNumber - this->microOpTypeOffset - this->intraInstructionDependencies;
         dependenciesLength++;
         LOG_ASSERT_ERROR(dependenciesLength < MAXIMUM_NUMBER_OF_DEPENDENCIES, "dependenciesLength(%u) > MAX(%u)", dependenciesLength, MAXIMUM_NUMBER_OF_DEPENDENCIES);
         intraInstructionDependencies--;
      }
      // Make sure the exit condition was that the dependency to be removed is now the first one, not that we have exhausted the list
      // LOG_ASSERT_ERROR(intraInstructionDependencies > 0, "Something went wrong while removing an intra-instruction dependency");
      // Remove the first intra-instruction dependency by decrementing intraInstructionDependencies
      intraInstructionDependencies--;
   } else {
      // Inter-instruction dependency
      LOG_ASSERT_ERROR(dependenciesLength > 0, "Cannot remove dependency when there are none");
      if (dependencies[dependenciesLength-1] == sequenceNumber)
         ; // sequenceNumber to remove is already at the end, we can just decrement dependenciesLength
      else {
         // Move sequenceNumber to the end of the list
         uint64_t idx = Tools::index(dependencies, dependenciesLength, sequenceNumber);
         LOG_ASSERT_ERROR(idx != UINT64_MAX, "MicroOp dependency list does not contain %ld", sequenceNumber);
         Tools::swap(dependencies, idx, dependenciesLength-1);
      }
      dependenciesLength--;
   }

   // If found,
   removeRegDependency(sequenceNumber);
}


void DynamicMicroOp::addRegDependency(uint64_t sequenceNumber)
{
   if (!Tools::contains(reg_dependencies, regDependenciesLength, sequenceNumber)) {
      assert(this->regDependenciesLength < MAXIMUM_NUMBER_OF_DEPENDENCIES);
      reg_dependencies[regDependenciesLength] = sequenceNumber;
      regDependenciesLength++;
   }
}

void DynamicMicroOp::removeRegDependency(uint64_t sequenceNumber)
{
   // Move sequenceNumber to the end of the list
   uint64_t idx = Tools::index(reg_dependencies, regDependenciesLength, sequenceNumber);
   if(idx == UINT64_MAX) {
      // If these dependency not included in register dependency, just skip
      return;
   }
   LOG_ASSERT_ERROR(regDependenciesLength > 0, "Cannot remove register dependency when there are none");
   Tools::swap(reg_dependencies, idx, regDependenciesLength-1);
   regDependenciesLength--;
}


const Memory::Access& DynamicMicroOp::getLoadAccess() const
{
   assert(this->getMicroOp()->isLoad());
   return this->address;
}

const Memory::Access& DynamicMicroOp::getStoreAccess() const
{
   assert(this->getMicroOp()->isStore());
   return this->address;
}

bool DynamicMicroOp::isLongLatencyLoad() const
{
   LOG_ASSERT_ERROR(getMicroOp()->isLoad(), "Expected a load instruction.");

   uint32_t cutoff = m_core_model->getLongLatencyCutoff();

   // If we are enabled, indicate that this is a long latency load if the latency
   // is above a certain cutoff value
   // Also, honor the forceLLL request if indicated
   return (m_forceLongLatencyLoad || ((cutoff > 0) && (this->execLatency > cutoff)));
}

void DynamicMicroOp::backupInitialDependencies()
{
   for (uint64_t i = 0; i < dependenciesLength; i++) {
      initial_dependencies[i] = dependencies[i];
   }
   initial_intraInstructionDependencies = intraInstructionDependencies;

   return;
}

void DynamicMicroOp::rollbackDependencies(uint64_t sequenceNumber)
{
   intraInstructionDependencies = initial_intraInstructionDependencies;

   for (uint64_t i = 0, j = 0; i < MAXIMUM_NUMBER_OF_DEPENDENCIES; i++) {
      if (initial_dependencies[i] == -1) break;
      if (initial_dependencies[i] >= sequenceNumber) {
         bool found = false;
         for (uint64_t j = 0; j < dependenciesLength; j++) {
            if (dependencies[j] == initial_dependencies[i]) {
               found = true;
               break;
            }
         }
         if (!found) {
            dependencies[dependenciesLength++] = initial_dependencies[i];
         }
      }
   }
   return;
}
