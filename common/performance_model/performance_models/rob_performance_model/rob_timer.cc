/*
 * This file is covered under the Interval Academic License, see LICENCE.interval
 */

#include "rob_timer.h"

#include "tools.h"
#include "stats.h"
#include "config.hpp"
#include "core_manager.h"
#include "itostr.h"
#include "performance_model.h"
#include "core_model.h"
#include "rob_contention.h"
#include "instruction.h"

#include <iostream>
#include <sstream>
#include <iomanip>

// Define to get per-cycle printout of dispatch, issue, writeback stages
// #define DEBUG_PERCYCLE
//#define STOP_PERCYCLE

// Define to not skip any cycles, but assert that the skip logic is working fine
//#define ASSERT_SKIP

#define KONATA_CNT_MAX (100000)

RobTimer::RobTimer(
         Core *core, PerformanceModel *_perf, const CoreModel *core_model,
         int misprediction_penalty,
         int dispatch_width,
         int window_size)
      : dispatchWidth(dispatch_width)
      , commitWidth(Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/commit_width", core->getId()))
      , windowSize(window_size) // windowSize = ROB length = 96 for Core2
      , rsEntries(Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/rs_entries", core->getId()))
      , misprediction_penalty(misprediction_penalty)
      , m_store_to_load_forwarding(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/store_to_load_forwarding", core->getId()))
      , m_no_address_disambiguation(!Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/address_disambiguation", core->getId()))
      , inorder(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/in_order", core->getId()))
      , vector_inorder(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/vector_inorder", core->getId()))
      , m_gather_scatter_merge(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/gather_scatter_merge", core->getId()))
      , m_core(core)
      , rob(window_size + 255)
      , m_num_in_rob(0)
      , m_rs_entries_used(0)
      , m_rob_contention(
         Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/issue_contention", core->getId())
         ? core_model->createRobContentionModel(core)
         : NULL)
      , now(core->getDvfsDomain())
      , frontend_stalled_until(SubsecondTime::Zero())
      , in_icache_miss(false)
      , last_store_done(SubsecondTime::Zero())
      , load_queue("rob_timer.load_queue", core->getId(), Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_loads", core->getId()))
      , store_queue("rob_timer.store_queue", core->getId(), Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_stores", core->getId()))
      , vec_load_queue("rob_timer.vec_load_queue", core->getId(), Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_vec_loads", core->getId()))
      , vec_store_queue("rob_timer.vec_store_queue", core->getId(), Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_vec_stores", core->getId()))
      , nextSequenceNumber(0)
      , will_skip(false)
      , time_skipped(SubsecondTime::Zero())
      , enable_debug_printf(false)
      , registerDependencies(new RegisterDependencies())
      , memoryDependencies(new MemoryDependencies())
      , perf(_perf)
      , m_cpiCurrentFrontEndStall(NULL)
      , m_mlp_histogram(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/mlp_histogram", core->getId()))
{

   registerStatsMetric("rob_timer", core->getId(), "time_skipped", &time_skipped);

   for(int i = 0; i < MicroOp::UOP_SUBTYPE_SIZE; ++i)
   {
      m_uop_type_count[i] = 0;
      registerStatsMetric("rob_timer", core->getId(), String("uop_") + MicroOp::getSubtypeString(MicroOp::uop_subtype_t(i)), &m_uop_type_count[i]);
   }
   m_uops_total = 0;
   m_uops_x87 = 0;
   m_uops_pause = 0;
   registerStatsMetric("rob_timer", core->getId(), "uops_total", &m_uops_total);
   registerStatsMetric("rob_timer", core->getId(), "uops_x87", &m_uops_x87);
   registerStatsMetric("rob_timer", core->getId(), "uops_pause", &m_uops_pause);

   m_numSerializationInsns = 0;
   m_totalSerializationLatency = 0;

   registerStatsMetric("rob_timer", core->getId(), "numSerializationInsns", &m_numSerializationInsns);
   registerStatsMetric("rob_timer", core->getId(), "totalSerializationLatency", &m_totalSerializationLatency);

   m_totalHiddenDCacheLatency = 0;
   registerStatsMetric("rob_timer", core->getId(), "totalHiddenDCacheLatency", &m_totalHiddenDCacheLatency);

   m_numMfenceInsns = 0;
   m_totalMfenceLatency = 0;

   registerStatsMetric("rob_timer", core->getId(), "numMfenceInsns", &m_numMfenceInsns);
   registerStatsMetric("rob_timer", core->getId(), "totalMfenceLatency", &m_totalMfenceLatency);

   m_cpiBase = SubsecondTime::Zero();
   m_cpiBranchPredictor = SubsecondTime::Zero();
   m_cpiSerialization = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "cpiBase", &m_cpiBase);
   registerStatsMetric("rob_timer", core->getId(), "cpiBranchPredictor", &m_cpiBranchPredictor);
   registerStatsMetric("rob_timer", core->getId(), "cpiSerialization", &m_cpiSerialization);
   registerStatsMetric("rob_timer", core->getId(), "cpiRSFull", &m_cpiRSFull);

   m_cpiInstructionCache.resize(HitWhere::NUM_HITWHERES, SubsecondTime::Zero());
   for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++)
   {
      if (HitWhereIsValid((HitWhere::where_t)h))
      {
         String name = "cpiInstructionCache" + String(HitWhereString((HitWhere::where_t)h));
         registerStatsMetric("rob_timer", core->getId(), name, &(m_cpiInstructionCache[h]));
      }
   }
   m_cpiDataCache.resize(HitWhere::NUM_HITWHERES, SubsecondTime::Zero());
   for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++)
   {
      if (HitWhereIsValid((HitWhere::where_t)h))
      {
         String name = "cpiDataCache" + String(HitWhereString((HitWhere::where_t)h));
         registerStatsMetric("rob_timer", core->getId(), name, &(m_cpiDataCache[h]));
      }
   }

   m_outstandingLongLatencyCycles = SubsecondTime::Zero();
   m_outstandingLongLatencyInsns = SubsecondTime::Zero();
   m_lastAccountedMemoryCycle = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "outstandingLongLatencyInsns", &m_outstandingLongLatencyInsns);
   registerStatsMetric("rob_timer", core->getId(), "outstandingLongLatencyCycles", &m_outstandingLongLatencyCycles);

   m_loads_count = 0;
   m_loads_latency = SubsecondTime::Zero();
   m_stores_count = 0;
   m_stores_latency = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "loads-count", &m_loads_count);
   registerStatsMetric("rob_timer", core->getId(), "loads-latency", &m_loads_latency);
   registerStatsMetric("rob_timer", core->getId(), "stores-count", &m_stores_count);
   registerStatsMetric("rob_timer", core->getId(), "stores-latency", &m_stores_latency);

   m_totalProducerInsDistance = 0;
   m_totalConsumers = 0;
   m_producerInsDistance.resize(windowSize, 0);

   registerStatsMetric("rob_timer", core->getId(), "totalProducerInsDistance", &m_totalProducerInsDistance);
   registerStatsMetric("rob_timer", core->getId(), "totalConsumers", &m_totalConsumers);
   for (unsigned int i = 0; i < m_producerInsDistance.size(); i++)
   {
      String name = "producerInsDistance[" + itostr(i) + "]";
      registerStatsMetric("rob_timer", core->getId(), name, &(m_producerInsDistance[i]));
   }

   if (m_mlp_histogram)
   {
      m_outstandingLoads.resize(HitWhere::NUM_HITWHERES);
      for (unsigned int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++)
      {
         if (HitWhereIsValid((HitWhere::where_t)h))
         {
            m_outstandingLoads[h].resize(MAX_OUTSTANDING, SubsecondTime::Zero());
            for(unsigned int i = 0; i < MAX_OUTSTANDING; ++i)
            {
               String name = String("outstandingLoads.") + HitWhereString((HitWhere::where_t)h) + "[" + itostr(i) + "]";
               registerStatsMetric("rob_timer", core->getId(), name, &(m_outstandingLoads[h][i]));
            }
         }
      }

      m_outstandingLoadsAll.resize(MAX_OUTSTANDING, SubsecondTime::Zero());
      for(unsigned int i = 0; i < MAX_OUTSTANDING; ++i)
      {
         String name = String("outstandingLoadsAll") + "[" + itostr(i) + "]";
         registerStatsMetric("rob_timer", core->getId(), name, &(m_outstandingLoadsAll[i]));
      }
   }

   Sim()->getHooksManager()->registerHook(HookType::HOOK_ROI_BEGIN, RobTimer::hookRoiBegin, (UInt64)this);
   Sim()->getHooksManager()->registerHook(HookType::HOOK_ROI_END, RobTimer::hookRoiEnd, (UInt64)this);
   m_roi_started = false;
   m_enable_konata = false;
   m_vsetvl_producer = 0;
}

RobTimer::~RobTimer()
{
   for(Rob::iterator it = this->rob.begin(); it != this->rob.end(); ++it)
      it->free();
}

void RobTimer::RobEntry::init(DynamicMicroOp *_uop, UInt64 sequenceNumber)
{
   ready = SubsecondTime::MaxTime();
   readyMax = SubsecondTime::Zero();
   addressReady = SubsecondTime::MaxTime();
   addressReadyMax = SubsecondTime::Zero();
   issued = SubsecondTime::MaxTime();
   done = SubsecondTime::MaxTime();

   uop = _uop;
   uop->setSequenceNumber(sequenceNumber);

   addressProducers.clear();

   numInlineDependants = 0;
   vectorDependants = NULL;
}

void RobTimer::RobEntry::free()
{
   delete uop;
   if (vectorDependants)
      delete vectorDependants;
}

void RobTimer::RobEntry::addDependant(RobTimer::RobEntry* dep)
{
   if (numInlineDependants < MAX_INLINE_DEPENDANTS)
   {
      inlineDependants[numInlineDependants++] = dep;
   }
   else
   {
      if (vectorDependants == NULL)
      {
         vectorDependants = new std::vector<RobEntry*>();
      }
      vectorDependants->push_back(dep);
   }
}

uint64_t RobTimer::RobEntry::getNumDependants() const
{
   return numInlineDependants + (vectorDependants ? vectorDependants->size() : 0);
}

RobTimer::RobEntry* RobTimer::RobEntry::getDependant(size_t idx) const
{
   if (idx < MAX_INLINE_DEPENDANTS)
   {
      LOG_ASSERT_ERROR(idx < numInlineDependants, "Invalid idx %d", idx);
      return inlineDependants[idx];
   }
   else
   {
      LOG_ASSERT_ERROR(idx - MAX_INLINE_DEPENDANTS < vectorDependants->size(), "Invalid idx %d", idx);
      return (*vectorDependants)[idx - MAX_INLINE_DEPENDANTS];
   }
}

RobTimer::RobEntry *RobTimer::findEntryBySequenceNumber(UInt64 sequenceNumber)
{
   // Assumption: MicroOps in the ROB are numbered sequentially, none of them are removed halfway
   UInt64 first = rob[0].uop->getSequenceNumber();
   UInt64 position = sequenceNumber - first;
   LOG_ASSERT_ERROR(position < rob.size(), "Sequence number %ld outside of ROB", sequenceNumber);
   RobEntry *entry = &rob[position];
   LOG_ASSERT_ERROR(entry->uop->getSequenceNumber() == sequenceNumber, "Sequence number %ld unexpectedly not at ROB position %ld", sequenceNumber, position);
   return entry;
}

boost::tuple<uint64_t,SubsecondTime> RobTimer::simulate(const std::vector<DynamicMicroOp*>& insts)
{
   uint64_t totalInsnExec = 0;
   SubsecondTime totalLat = SubsecondTime::Zero();

   for (std::vector<DynamicMicroOp*>::const_iterator it = insts.begin(); it != insts.end(); it++ )
   {
      if ((*it)->isSquashed())
      {
         delete *it;
         continue;
      }

      RobEntry *entry = &this->rob.next();
      entry->init(*it, nextSequenceNumber++);

      // Add = calculate dependencies, add yourself to list of depenants
      // If no dependants in window: set ready = now()
      uint64_t lowestValidSequenceNumber = this->rob.size() > 0 ? this->rob.front().uop->getSequenceNumber() : 0;
      if (entry->uop->getMicroOp()->isStore())
      {
         for(unsigned int i = 0; i < entry->uop->getMicroOp()->getAddressRegistersLength(); ++i)
         {
            dl::Decoder::decoder_reg reg = entry->uop->getMicroOp()->getAddressRegister(i);
            uint64_t addressProducer = this->registerDependencies->peekProducer(reg, lowestValidSequenceNumber);
            if (addressProducer != INVALID_SEQNR)
            {
               RobEntry *prodEntry = this->findEntryBySequenceNumber(addressProducer);
               if (prodEntry->done != SubsecondTime::MaxTime())
                  entry->addressReadyMax = std::max(entry->addressReadyMax, prodEntry->done);
               else
                  entry->addAddressProducer(addressProducer);
            }
         }
         if (entry->getNumAddressProducers() == 0)
            entry->addressReady = entry->addressReadyMax;
      }

      this->registerDependencies->setDependencies(*entry->uop, lowestValidSequenceNumber);
      this->memoryDependencies->setDependencies(*entry->uop, lowestValidSequenceNumber);

      setVSETDependencies (*entry->uop, lowestValidSequenceNumber);

      if (m_store_to_load_forwarding && entry->uop->getMicroOp()->isLoad())
      {
         for(unsigned int i = 0; i < entry->uop->getDependenciesLength(); ++i)
         {
            RobEntry *prodEntry = this->findEntryBySequenceNumber(entry->uop->getDependency(i));
            // If we depend on a store
            if (prodEntry->uop->getMicroOp()->isStore())
            {
               // Remove dependency on the store (which won't execute until it reaches the front of the ROB)
               entry->uop->removeDependency(entry->uop->getDependency(i));

               // Add dependencies to the producers of the value being stored instead
               // Remark: one of these may be producing the store address, but because the store has to be
               //         disambiguated, it's correct to have the load depend on the address producers as well.
               for(unsigned int j = 0; j < prodEntry->uop->getDependenciesLength(); ++j) {
                 entry->uop->addDependency(prodEntry->uop->getDependency(j));
               }
               break;
            }
         }
      }

      // Add ourselves to the dependants list of the uops we depend on
      uint64_t minProducerDistance = UINT64_MAX;
      m_totalConsumers += 1 ;
      uint64_t deps_to_remove[8], num_dtr = 0;
      for(unsigned int i = 0; i < entry->uop->getDependenciesLength(); ++i)
      {
         RobEntry *prodEntry = this->findEntryBySequenceNumber(entry->uop->getDependency(i));
         minProducerDistance = std::min( minProducerDistance,  entry->uop->getSequenceNumber() - prodEntry->uop->getSequenceNumber() );
         if (prodEntry->done != SubsecondTime::MaxTime())
         {
            // If producer is already done (but hasn't reached writeback stage), remove it from our dependency list
            deps_to_remove[num_dtr++] = entry->uop->getDependency(i);
            entry->readyMax = std::max(entry->readyMax, prodEntry->done);
         }
         else
         {
            prodEntry->addDependant(entry);
         }
      }

      #ifdef DEBUG_PERCYCLE
      // Make sure we are in the dependant list of all of our address producers
      for(unsigned int i = 0; i < entry->getNumAddressProducers(); ++i)
      {
         if (rob.size() && entry->getAddressProducer(i) >= rob[0].uop->getSequenceNumber())
         {
            RobEntry *prodEntry = this->findEntryBySequenceNumber(entry->getAddressProducer(i));
            bool found = false;
            for(unsigned int j = 0; j < prodEntry->getNumDependants(); ++j)
               if (prodEntry->getDependant(j) == entry)
               {
                  found = true;
                  break;
               }
            LOG_ASSERT_ERROR(found == true, "Store %ld depends on %ld for address production, but is not in its dependants list",
                             entry->uop->getSequenceNumber(), prodEntry->uop->getSequenceNumber());
         }
      }
      #endif

      if (minProducerDistance != UINT64_MAX)
      {
         m_totalProducerInsDistance += minProducerDistance;
         // KENZO: not sure why the distance can be larger than the windowSize, but it happens...
         if (minProducerDistance >= m_producerInsDistance.size())
            minProducerDistance = m_producerInsDistance.size()-1;
         m_producerInsDistance[ minProducerDistance ]++ ;
      }
      else
      {
         // Not depending on any instruction in the rob
         m_producerInsDistance[ 0 ] += 1 ;
      }

      // If there are any dependencies to be removed, do this after iterating over them (don't mess with the list we're reading)
      LOG_ASSERT_ERROR(num_dtr < sizeof(deps_to_remove)/sizeof(8), "Have to remove more dependencies than I expected");
      for(uint64_t i = 0; i < num_dtr; ++i)
         entry->uop->removeDependency(deps_to_remove[i]);
      if (entry->uop->getDependenciesLength() == 0)
      {
         // We have no dependencies in the ROB: mark ourselves as ready
         entry->ready = entry->readyMax;
      }

      #ifdef DEBUG_PERCYCLE
      if (enable_debug_printf) {
         std::cout<<"** simulate: "<< entry->uop->getMicroOp()->toShortString(true) << std::endl << entry->uop->getMicroOp()->toString()<<std::endl;
      }
      #endif

#ifdef DEBUG_PERCYCLE
      std::cerr << "Type = " << (*it)->getMicroOp()->getSubtype() <<
          " count = " <<
          m_uop_type_count[(*it)->getMicroOp()->getSubtype()] << '\n';
#endif // DEBUG_PERCYCLE

      m_uop_type_count[(*it)->getMicroOp()->getSubtype()]++;
      m_uops_total++;
      if ((*it)->getMicroOp()->isX87()) m_uops_x87++;
      if ((*it)->getMicroOp()->isPause()) m_uops_pause++;

      if (m_uops_total > 10000 && m_uops_x87 > m_uops_total / 20)
         LOG_PRINT_WARNING_ONCE("Significant fraction of x87 instructions encountered, accuracy will be low. Compile without -mno-sse2 -mno-sse3 to avoid.");
   }

#ifdef DEBUG_PERCYCLE
#ifdef STOP_PERCYCLE
   char a;
   std::cin >> a;
#endif
#endif
   while (true)
   {
      uint64_t instructionsExecuted;
      SubsecondTime latency;
      execute(instructionsExecuted, latency);
      totalInsnExec += instructionsExecuted;
      totalLat += latency;
      if (latency == SubsecondTime::Zero())
         break;
#ifdef DEBUG_PERCYCLE
#ifdef STOP_PERCYCLE
          std::cin >> a;
#endif
#endif
   }

   return boost::tuple<uint64_t,SubsecondTime>(totalInsnExec, totalLat);
}

void RobTimer::synchronize(SubsecondTime time)
{
   // NOTE: depending on how far we jumped ahead (usually a considerable amount),
   //       we may want to flush the ROB and reset other queues
   //printf("RobTimer::synchronize(%lu) %+ld\n", time, (int64_t)time-now);
   now.setElapsedTime(time);
}

SubsecondTime* RobTimer::findCpiComponent()
{
   // Determine the CPI component corresponding to the first non-committed instruction
   for(uint64_t i = 0; i < m_num_in_rob; ++i)
   {
      RobEntry *entry = &rob.at(i);
      DynamicMicroOp *uop = entry->uop;
      // Skip over completed instructions
      if (entry->done < now)
         continue;
      // This is the first instruction in the ROB which is still executing
      // Assume everyone is blocked on this one
      // Assign 100% of this cycle to this guy's CPI component
      if (uop->getMicroOp()->isSerializing() || uop->getMicroOp()->isMemBarrier())
         return &m_cpiSerialization;
      else if (uop->getMicroOp()->isLoad() || uop->getMicroOp()->isStore())
         return &m_cpiDataCache[uop->getDCacheHitWhere()];
      else
         return NULL;
   }
   // No instruction is currently executing
   return NULL;
}

SubsecondTime RobTimer::doDispatch(SubsecondTime **cpiComponent)
{
   SubsecondTime next_event = SubsecondTime::MaxTime();
   SubsecondTime *cpiFrontEnd = NULL;

   static SubsecondTime missed_icache = SubsecondTime::MaxTime();

   if (frontend_stalled_until <= now)
   {
      uint32_t instrs_dispatched = 0, uops_dispatched = 0;

      while(m_num_in_rob < windowSize)
      {
         LOG_ASSERT_ERROR(m_num_in_rob < rob.size(), "Expected sufficient uops for dispatching in pre-ROB buffer, but didn't find them");
         RobEntry *entry = &rob.at(m_num_in_rob);
         DynamicMicroOp &uop = *entry->uop;

         // Dispatch up to 4 instructions
         if (uops_dispatched == dispatchWidth)
            break;

         // This is actually in the decode stage, there's a buffer between decode and dispatch
         // so we shouldn't do this here.
         //// First instruction can be any size, but second and subsequent ones may only be single-uop
         //// So, if this is not the first instruction, break if the first uop is not also the last
         //if (instrs_dispatched > 0 && !uop.isLast())
         //   break;

         bool iCacheMiss = (uop.getICacheHitWhere() != HitWhere::L1I);
         if (iCacheMiss)
         {
            if (in_icache_miss)
            {
               // We just took the latency for this instruction, now dispatch it
               #ifdef DEBUG_PERCYCLE
                  std::cout<<"-- icache return"<<std::endl;
               #endif
               in_icache_miss = false;
            }
            else
            {
               #ifdef DEBUG_PERCYCLE
              std::cout<<"-- icache miss (" << std::hex << uop.getMicroOp()->getInstruction()->getAddress() << ") ("<<uop.getICacheLatency()<<")"<<std::endl;
               #endif
               frontend_stalled_until = now + uop.getICacheLatency();
               in_icache_miss = true;
               entry->fetch = now;
               missed_icache = now;
               // Don't dispatch this instruction yet
               cpiFrontEnd = &m_cpiInstructionCache[uop.getICacheHitWhere()];
               break;
            }
         } else {
           missed_icache = now;
         }

         if (m_rs_entries_used == rsEntries)
         {
            cpiFrontEnd = &m_cpiRSFull;
            break;
         }

         entry->fetch = missed_icache;
         entry->dispatched = now;
         ++m_num_in_rob;
         ++m_rs_entries_used;

         uops_dispatched++;
         if (uop.isLast())
            instrs_dispatched++;

         // If uop is already ready, we may need to issue it in the following cycle
         entry->ready = std::max(entry->ready, (now + 1ul).getElapsedTime());
         next_event = std::min(next_event, entry->ready);

         #ifdef DEBUG_PERCYCLE
            std::cout<<"DISPATCH "<<entry->uop->getMicroOp()->toShortString()<<std::endl;
         #endif

         #ifdef ASSERT_SKIP
            LOG_ASSERT_ERROR(will_skip == false, "Cycle would have been skipped but stuff happened");
         #endif

         // Mispredicted branch
         if (uop.getMicroOp()->isBranch() && uop.isBranchMispredicted())
         {
            frontend_stalled_until = SubsecondTime::MaxTime();
            #ifdef DEBUG_PERCYCLE
               std::cout<<"-- branch mispredict"<<std::endl;
            #endif
            cpiFrontEnd = &m_cpiBranchPredictor;
            break;
         }
      }

      m_cpiCurrentFrontEndStall = cpiFrontEnd;
   }
   else
   {
      // Front-end is still stalled: re-use last CPI component
      cpiFrontEnd = m_cpiCurrentFrontEndStall;
   }


   // Find CPI component corresponding to the first executing instruction
   SubsecondTime *cpiRobHead = findCpiComponent();

   if (cpiFrontEnd)
   {
      // Front-end is stalled
      if (cpiRobHead)
      {
         // Have memory/serialization components take precendence over front-end stalls
         *cpiComponent = cpiRobHead;
      }
      else
      {
         *cpiComponent = cpiFrontEnd;
      }
   }
   else if (m_num_in_rob == windowSize)
   {
      *cpiComponent = cpiRobHead ? cpiRobHead : &m_cpiBase;
   }
   else
   {
      *cpiComponent = &m_cpiBase;
   }


   if (m_num_in_rob == windowSize)
      return next_event; // front-end is effectively stalled so wait for another event
   else
      return std::min(frontend_stalled_until, next_event);
}

void RobTimer::issueInstruction(uint64_t idx, SubsecondTime &next_event)
{
   RobEntry *entry = &rob[idx];
   DynamicMicroOp &uop = *entry->uop;

   if ((uop.getMicroOp()->isLoad() || uop.getMicroOp()->isStore())
      && uop.getDCacheHitWhere() == HitWhere::UNKNOWN)
   {
      // Vector instruction, previous access merge, it can be skipped
      if (!uop.getMemAccessMerge()) {
         UInt64 block_start = uop.getAddress().address % uop.getMicroOp()->getMemoryAccessSize();
         UInt64 block_load_size = uop.getMicroOp()->getMemoryAccessSize() - block_start;
         MemoryResult res = m_core->accessMemory(
            Core::NONE,
            uop.getMicroOp()->isVector() ? (uop.getMicroOp()->isLoad() ? Core::READ_VEC : Core::WRITE_VEC) :
            uop.getMicroOp()->isLoad() ? Core::READ : Core::WRITE,
            uop.getAddress().address,
            NULL,
            uop.getMicroOp()->isVector() ? block_load_size : uop.getMicroOp()->getMemoryAccessSize(),
            Core::MEM_MODELED_RETURN,
            uop.getMicroOp()->getInstruction() ? uop.getMicroOp()->getInstruction()->getAddress() : static_cast<uint64_t>(NULL),
            now.getElapsedTime()
         );
         uint64_t latency = SubsecondTime::divideRounded(res.latency, now.getPeriod());
         m_previous_latency = latency;
         m_previous_hit_where = res.hit_where;

         uop.setExecLatency(uop.getExecLatency() + latency); // execlatency already contains bypass latency
         uop.setDCacheHitWhere(res.hit_where);
      } else {
         uop.setExecLatency(uop.getExecLatency() + m_previous_latency); // execlatency already contains bypass latency
         uop.setDCacheHitWhere(m_previous_hit_where);
      }
   }

   if (uop.getMicroOp()->isLoad())
   {
      if (uop.getMicroOp()->isVector()) {
         vec_load_queue.getCompletionTime(now, uop.getExecLatency() * now.getPeriod(), uop.getAddress().address);
      } else {
         load_queue.getCompletionTime(now, uop.getExecLatency() * now.getPeriod(), uop.getAddress().address);
      }
   }
   else if (uop.getMicroOp()->isStore())
   {
      if (uop.getMicroOp()->isVector()) {
         vec_store_queue.getCompletionTime(now, uop.getExecLatency() * now.getPeriod(), uop.getAddress().address);
      } else {
         store_queue.getCompletionTime(now, uop.getExecLatency() * now.getPeriod(), uop.getAddress().address);
      }
   }

   ComponentTime cycle_depend = now + uop.getExecLatency();        // When result is available for dependent instructions
   SubsecondTime cycle_done = cycle_depend + 1ul;                  // When the instruction can be committed

   if (uop.getMicroOp()->isLoad())
   {
      m_loads_count++;
      m_loads_latency += uop.getExecLatency() * now.getPeriod();
   }
   else if (uop.getMicroOp()->isStore())
   {
      m_stores_count++;
      m_stores_latency += uop.getExecLatency() * now.getPeriod();
   }

   if (uop.getMicroOp()->isStore())
   {
      last_store_done = std::max(last_store_done, cycle_done);
      cycle_depend = now + 1ul;                          // For stores, forward the result immediately
      // Stores can be removed from the ROB once they're issued to the memory hierarchy
      // Dependent operations such as SFENCE and synchronization instructions need to wait until last_store_done
      cycle_done = now + 1ul;

      LOG_ASSERT_ERROR(entry->addressReady <= entry->ready, "%ld: Store address cannot be ready (%ld) later than the whole uop is (%ld)",
                       entry->uop->getSequenceNumber(), entry->addressReady.getPS(), entry->ready.getPS());
   }

   if (m_rob_contention)
      m_rob_contention->doIssue(uop);

   entry->issued = now;
   entry->done = cycle_done;

   next_event = std::min(next_event, entry->done);

   --m_rs_entries_used;

   #ifdef DEBUG_PERCYCLE
      std::cout<<"ISSUE    "<<entry->uop->getMicroOp()->toShortString()<<"   latency="<<uop.getExecLatency()<<std::endl;
   #endif

   for(size_t idx = 0; idx < entry->getNumDependants(); ++idx)
   {
      RobEntry *depEntry = entry->getDependant(idx);
      LOG_ASSERT_ERROR(depEntry->uop->getDependenciesLength()> 0, "??");

      // Remove uop from dependency list and update readyMax
      depEntry->readyMax = std::max(depEntry->readyMax, cycle_depend.getElapsedTime());
      depEntry->uop->removeDependency(uop.getSequenceNumber());

      // If all dependencies are resolved, mark the uop ready
      if (depEntry->uop->getDependenciesLength() == 0)
      {
         depEntry->ready = depEntry->readyMax;
         //std::cout<<"    ready @ "<<depEntry->ready<<std::endl;
      }

      // For stores, check if their address has been produced
      if (depEntry->uop->getMicroOp()->isStore() && depEntry->addressReady == SubsecondTime::MaxTime())
      {
         bool ready = true;
         for(unsigned int i = 0; i < depEntry->getNumAddressProducers(); ++i)
         {
            uint64_t addressProducer = depEntry->getAddressProducer(i);
            RobEntry *prodEntry = addressProducer >= this->rob.front().uop->getSequenceNumber()
                                ? this->findEntryBySequenceNumber(addressProducer) : NULL;

            if (prodEntry == entry)
            {
               // The instruction we just executed is producing an address. Update the store's addressReadyMax
               depEntry->addressReadyMax = std::max(depEntry->addressReadyMax, cycle_depend.getElapsedTime());
            }

            if (prodEntry && prodEntry->done == SubsecondTime::MaxTime())
            {
               // An address producer has not yet been issued: address remains not ready
               ready = false;
            }
         }

         if (ready)
         {
            // We did not find any address producing instructions that have not yet been issued.
            // Store address will be ready at addressReadyMax
            depEntry->addressReady = depEntry->addressReadyMax;
         }
      }
   }

   // After issuing a mispredicted branch: allow the ROB to refill after flushing the pipeline
   if (uop.getMicroOp()->isBranch() && uop.isBranchMispredicted())
   {
      frontend_stalled_until = now + (misprediction_penalty - 2); // The frontend needs to start 2 cycles earlier to get a total penalty of <misprediction_penalty>
      #ifdef DEBUG_PERCYCLE
         std::cout<<"-- branch resolve"<<std::endl;
      #endif
   }
}

SubsecondTime RobTimer::doIssue()
{
   uint64_t num_issued = 0;
   SubsecondTime next_event = SubsecondTime::MaxTime();
   bool head_of_queue = true, no_more_load = false, no_more_store = false, have_unresolved_store = false;

   if (m_rob_contention)
      m_rob_contention->initCycle(now);

   int64_t last_vec_issued_idx = -1;
   int64_t issued_vec_inst = 0;
   UInt64  l1d_block_size = Sim()->getCfg()->getInt("perf_model/l1_dcache/cache_block_size");
   UInt64  l1d_num_banks  = Sim()->getCfg()->getInt("perf_model/l1_dcache/num_banks");

   UInt64  *bank_info = new UInt64[l1d_num_banks];
   for (UInt64 i = 0; i < l1d_num_banks; i++) {
     bank_info[i] = 0;
   }

   for(uint64_t i = 0; i < m_num_in_rob; ++i)
   {
      RobEntry *entry = &rob.at(i);
      DynamicMicroOp *uop = entry->uop;


      if (entry->done != SubsecondTime::MaxTime())
      {
         next_event = std::min(next_event, entry->done);
         continue;                     // already done
      }

      next_event = std::min(next_event, entry->ready);


      // See if we can issue this instruction

      bool canIssue = false;

      if (entry->ready > now)
         canIssue = false;          // blocked by dependency

      else if ((no_more_load && uop->getMicroOp()->isLoad()) || (no_more_store && uop->getMicroOp()->isStore()))
         canIssue = false;          // blocked by mfence

      else if (uop->getMicroOp()->isSerializing())
      {
         if (head_of_queue && last_store_done <= now)
            canIssue = true;
         else
            break;
      }

      else if (uop->getMicroOp()->isMemBarrier())
      {
         if (head_of_queue && last_store_done <= now)
            canIssue = true;
         else
            // Don't issue any memory operations following a memory barrier
            no_more_load = no_more_store = true;
            // FIXME: L/SFENCE
      }

      else if (!m_rob_contention && num_issued == dispatchWidth) {
        std::cerr << "  dispatch Width exceeded\n";
         canIssue = false;          // no issue contention: issue width == dispatch width
      }
      else if (uop->getMicroOp()->isLoad() &&
               ( (uop->getMicroOp()->isVector() && !vec_load_queue.hasFreeSlot(now)) ||
                 (!uop->getMicroOp()->isVector() && !load_queue.hasFreeSlot(now)))) {
#ifdef DEBUG_PERCYCLE
        std::cerr << "  load_queue.hasFreeSlot failed " << uop->getMicroOp()->toShortString() <<
            ", index = " << uop->getSequenceNumber() << '\n';
#endif // DEBUG_PERCYCLE
         canIssue = false;          // load queue full

      } else if (uop->getMicroOp()->isLoad() && m_no_address_disambiguation && have_unresolved_store) {
#ifdef DEBUG_PERCYCLE
        std::cerr << "  disambiguation" <<
            ", index = " << uop->getSequenceNumber() << '\n';
#endif // DEBUG_PERCYCLE
         canIssue = false;          // preceding store with unknown address
      }
      else if (uop->getMicroOp()->isStore() && (!head_of_queue ||
                                                ( uop->getMicroOp()->isVector() && !vec_store_queue.hasFreeSlot(now)) ||
                                                (!uop->getMicroOp()->isVector() && !store_queue.hasFreeSlot(now))))
      {
        std::cerr << "  store\n";
         canIssue = false;          // store queue full
      }
      else
         canIssue = true;           // issue!

#ifdef DEBUG_PERCYCLE
        std::cerr << "  hazard check final result : " << uop->getMicroOp()->toShortString() <<
            ", index = " << uop->getSequenceNumber() <<
            (canIssue ? " True" : " False") << std::endl;
#endif // DEBUG_PERCYCLE

      // canIssue already marks issue ports as in use, so do this one last
      if (canIssue && m_rob_contention && ! m_rob_contention->tryIssue(*uop)) {
#ifdef DEBUG_PERCYCLE
        std::cerr << "  tryIssue failed " << uop->getMicroOp()->toShortString() <<
            ", index = " << uop->getSequenceNumber() << '\n';
#endif // DEBUG_PERCYCLE
         // fprintf(stderr, "tryIssue failed\n");
         canIssue = false;          // blocked by structural hazard
      }


      if ((uop->getMicroOp()->isLoad() || uop->getMicroOp()->isStore()) &&
          uop->getMicroOp()->isVector()) {
        if (m_gather_scatter_merge) {
          if (vector_inorder) {
            if (issued_vec_inst < 8 &&
                (issued_vec_inst == 0 ||
                 static_cast<uint64_t>(last_vec_issued_idx + 1) == i)) { // Initial Vector Inst, or sequential Vector inst
              last_vec_issued_idx = i;
              issued_vec_inst++;
              canIssue = canIssue; // Keep can issue
            } else {
              canIssue = false;
            }
          } else { // Vector Out-of-Order
            // fprintf (stderr, "Vector can Issue? (%s) %s : ",
            //          canIssue ? "Yes" : "No",
            //          uop->getMicroOp()->toShortString().c_str());
            if (issued_vec_inst < 8) {
              issued_vec_inst++;
              // canIssue = true;
              // fprintf (stderr, "Enough entry slot: %s (%ld)\n", canIssue ? "Yes" : "No", issued_vec_inst);
              canIssue = canIssue; // Keep can issue
            } else {
              // fprintf (stderr, "Slot fulled: No %ld\n", issued_vec_inst);
              canIssue = false;
            }
          }

          IntPtr cache_line = uop->getAddress().address & ~(l1d_block_size-1);
          IntPtr banked_cache_line = cache_line & ~(l1d_block_size * l1d_num_banks - 1);
          IntPtr bank_index = (cache_line ^ banked_cache_line) / l1d_block_size;

          if (bank_info[bank_index] == 0) {           // first bank acces
#ifdef DEBUG_PERCYCLE
            fprintf (stderr, "%ld %s cacheline bank initiated %08lx with %08lx. bank=%ld. CanIssue = %d\n",
                     uop->getSequenceNumber(),
                     uop->getMicroOp()->toShortString().c_str(),
                     uop->getAddress().address, bank_info[bank_index], bank_index, canIssue);
#endif // DEBUG_PERCYCLE
            bank_info[bank_index] = banked_cache_line;
          } else if (bank_info[bank_index] == banked_cache_line) {
            // Same Bank Access and Can be Merge:
            uop->setMemAccessMerge();
#ifdef DEBUG_PERCYCLE
            fprintf (stderr, "%ld %s cacheline bank can be access %08lx with %08lx. bank=%ld, CanIssue = %d\n",
                     uop->getSequenceNumber(),
                     uop->getMicroOp()->toShortString().c_str(),
                     uop->getAddress().address, bank_info[bank_index], bank_index, canIssue);
#endif // DEBUG_PERCYCLE
          } else {
            canIssue = false;
#ifdef DEBUG_PERCYCLE
            fprintf (stderr, "%ld %s cacheline bank conflict %08lx with %08lx, bank=%ld, CanIssue = %d\n",
                     uop->getSequenceNumber(),
                     uop->getMicroOp()->toShortString().c_str(),
                     uop->getAddress().address, bank_info[bank_index], bank_index, canIssue);
#endif // DEBUG_PERCYCLE
          }

          bank_info[bank_index] = banked_cache_line;
        } else {   // Gather Scatter Merge doesn't happen
            if (uop->getMicroOp()->isVector() && vector_inorder && !head_of_queue) {
              canIssue = false;
            }
          }
      } else if (uop->getMicroOp()->isVector() && vector_inorder && !head_of_queue) {
          canIssue = false;
        }

      if (canIssue)
      {
         num_issued++;
         issueInstruction(i, next_event);

         // Calculate memory-level parallelism (MLP) for long-latency loads (but ignore overlapped misses)
         if (uop->getMicroOp()->isLoad() && uop->isLongLatencyLoad() && uop->getDCacheHitWhere() != HitWhere::L1_OWN)
         {
            if (m_lastAccountedMemoryCycle < now) m_lastAccountedMemoryCycle = now;

            SubsecondTime done = std::max( now.getElapsedTime(), entry->done );
            // Ins will be outstanding for until it is done. By account beforehand I don't need to
            // worry about fast-forwarding simulations
            m_outstandingLongLatencyInsns += (done - now);

            // Only account for the cycles that have not yet been accounted for by other long
            // latency misses (don't account cycles twice).
            if ( done > m_lastAccountedMemoryCycle )
            {
               m_outstandingLongLatencyCycles += done - m_lastAccountedMemoryCycle;
               m_lastAccountedMemoryCycle = done;
            }

            #ifdef ASSERT_SKIP
            LOG_ASSERT_ERROR( m_outstandingLongLatencyInsns >= m_outstandingLongLatencyCycles, "MLP calculation is wrong: MLP cannot be < 1!"  );
            #endif
         }


         #ifdef ASSERT_SKIP
            LOG_ASSERT_ERROR(will_skip == false, "Cycle would have been skipped but stuff happened");
         #endif
      }
      else
      {
         head_of_queue = false;     // Subsequent instructions are not at the head of the ROB

         if (uop->getMicroOp()->isStore() && entry->addressReady > now)
            have_unresolved_store = true;

         if (inorder || (uop->getMicroOp()->isVector() && !uop->isVirtuallyIssued()))
            // In-order: only issue from head of the ROB
            break;
      }

      if (canIssue && uop->getMicroOp()->isVector() &&
          uop->getMicroOp()->UopIdx() == 0) {
#ifdef DEBUG_PERCYCLE
        std::cerr << "Vector Issue Start = " << uop->getMicroOp()->toShortString() <<
            ", index = " << uop->getSequenceNumber() << '\n';
#endif // DEBUG_PERCYCLE
        for (uint64_t j = i+1; j < m_num_in_rob; ++j) {
          RobEntry *subseq_entry = &rob.at(j);
          DynamicMicroOp *subseq_uop = subseq_entry->uop;

          if (subseq_uop->getMicroOp()->getInstruction()->getAddress() ==
              uop->getMicroOp()->getInstruction()->getAddress()) {
#ifdef DEBUG_PERCYCLE
            std::cerr << "  Set Virtually Issue. " << subseq_uop->getMicroOp()->toShortString() <<
                ", index = " << subseq_uop->getSequenceNumber() << '\n';
#endif // DEBUG_PERCYCLE
            subseq_uop->setVirtuallyIssued();
          } else {
            break;
          }
        }
      }

      if (m_rob_contention)
      {
         if (m_rob_contention->noMore())
            break;
      }
      else
      {
         if (num_issued == dispatchWidth)
            break;
      }
   }

   return next_event;
}

SubsecondTime RobTimer::doCommit(uint64_t& instructionsExecuted)
{
   uint64_t num_committed = 0;
   static bool cycle_activated = false;
   static int konata_count = 0;

   while(rob.size() && (rob.front().done <= now))
   {
      RobEntry *entry = &rob.front();

#ifdef DEBUG_PERCYCLE
      std::cout<<"COMMIT   "<<entry->uop->getMicroOp()->toShortString()<<std::endl;
#endif

      // Send instructions to loop tracer, in-order, once we know their issue time
      InstructionTracer::uop_times_t times = {
         entry->dispatched,
         entry->issued,
         entry->done,
         now
      };
      m_core->getPerformanceModel()->traceInstruction(entry->uop, &times);

      if (entry->uop->isLast())
         instructionsExecuted++;

      static FILE *o3_fp = NULL;
      if (o3_fp == NULL) {
        o3_fp = fopen("o3_trace.out", "w");
      }

      Instruction *inst = entry->uop->getMicroOp()->getInstruction();
      if (cycle_activated &&
          inst->getDisassembly().find("add            zero, zero, zero") != std::string::npos) {
      }

      if (enable_debug_printf &&
          cycle_activated &&
          inst->getDisassembly().find("add            zero, zero, ra") != std::string::npos &&
          konata_count < KONATA_CNT_MAX) {
        m_enable_konata = 1;
        std::cout << "KonataStart " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << " "
                  << std::hex << entry->uop->getMicroOp()->getInstruction()->getAddress() << " "
                  << entry->uop->getMicroOp()->getInstruction()->getDisassembly() << '\n';
      }
      if (enable_debug_printf &&
          cycle_activated &&
          inst->getDisassembly().find("add            zero, zero, sp") != std::string::npos) {
        m_enable_konata = 0;
        std::cout << "KonataStop " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << " "
                  << std::hex << entry->uop->getMicroOp()->getInstruction()->getAddress() << " "
                  << entry->uop->getMicroOp()->getInstruction()->getDisassembly() << '\n';
      }
      if (m_enable_konata &&
          konata_count >= KONATA_CNT_MAX) {
        m_enable_konata = 0;
      }

      if (m_enable_konata) {
        konata_count ++;
      }

      if (inst->getDisassembly().find("cycle") != std::string::npos) {
        cycle_activated = true;
      } else {
        cycle_activated = false;
      }

      if (m_enable_konata) {

        uint64_t cycle_fetch    = SubsecondTime::divideRounded(entry->fetch,      m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_dispatch = SubsecondTime::divideRounded(entry->dispatched, m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_issue    = SubsecondTime::divideRounded(entry->issued,     m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_done     = SubsecondTime::divideRounded(entry->done,       m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_commit   = SubsecondTime::divideRounded(times.commit,      m_core->getDvfsDomain()->getPeriod());

        cycle_fetch = cycle_fetch == 0 ? 4 : cycle_fetch;
        Instruction *inst = entry->uop->getMicroOp()->getInstruction();
        fprintf (o3_fp, "O3PipeView:fetch:%ld:0x%08lx:0:%ld:%s\n",
                 (cycle_fetch-3)*500,
                 (uint64_t)inst->getAddress(),
                 entry->uop->getSequenceNumber(),
                 inst->getDisassembly().c_str());
        fprintf (o3_fp, "O3PipeView:decode:%ld\n",                (cycle_dispatch-2)*500);
        fprintf (o3_fp, "O3PipeView:rename:%ld\n",                (cycle_dispatch-1)*500);
        fprintf (o3_fp, "O3PipeView:dispatch:%ld\n",              (cycle_dispatch  )*500);
        fprintf (o3_fp, "O3PipeView:issue:%ld\n",                 (cycle_issue     )*500);
        fprintf (o3_fp, "O3PipeView:complete:%ld\n",              (cycle_done      )*500);
        fprintf (o3_fp, "O3PipeView:retire:%ld:store:0\n",        (cycle_commit    )*500);
      }


      entry->free();
      rob.pop();
      m_num_in_rob--;

      #ifdef ASSERT_SKIP
         LOG_ASSERT_ERROR(will_skip == false, "Cycle would have been skipped but stuff happened");
      #endif

      ++num_committed;
      if (num_committed == commitWidth)
         break;
   }

   if (rob.size())
      return rob.front().done;
   else
      return SubsecondTime::MaxTime();
}

void RobTimer::execute(uint64_t& instructionsExecuted, SubsecondTime& latency)
{
   latency = SubsecondTime::Zero();
   instructionsExecuted = 0;
   SubsecondTime *cpiComponent = NULL;

#ifdef DEBUG_PERCYCLE
   std::cout<<std::endl;
   std::cout<<"Running cycles "<< std::dec << SubsecondTime::divideRounded(now, now.getPeriod())<<std::endl;
#endif

   if (m_roi_started) {
     std::cout << "CycleTrace " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << '\n';
     m_roi_started = false;
   }

   // If frontend not stalled
   if (frontend_stalled_until <= now)
   {
      if (rob.size() < m_num_in_rob + 2*dispatchWidth)
      {
         // We don't have enough instructions to dispatch <dispatchWidth> new ones. Ask for more before doing anything this cycle.
         return;
      }
   }


   // Model dispatch, issue and commit stages
   // Decode stage is not modeled, assumes the decoders can keep up with (up to) dispatchWidth uops per cycle

   SubsecondTime next_dispatch = doDispatch(&cpiComponent);
   SubsecondTime next_issue    = doIssue();
   SubsecondTime next_commit   = doCommit(instructionsExecuted);


   #ifdef DEBUG_PERCYCLE
      #ifdef ASSERT_SKIP
         if (! will_skip)
         {
      #endif
           if (enable_debug_printf) {
             printRob();
           }
      #ifdef ASSERT_SKIP
         }
      #endif
   #endif


   #ifdef DEBUG_PERCYCLE
      std::cout<<"Next event: D("<<SubsecondTime::divideRounded(next_dispatch, now.getPeriod())<<") I("<<SubsecondTime::divideRounded(next_issue, now.getPeriod())<<") C("<<SubsecondTime::divideRounded(next_commit, now.getPeriod())<<")"<<std::endl;
   #endif
   SubsecondTime next_event = std::min(next_dispatch, std::min(next_issue, next_commit));
   SubsecondTime skip;
   if (next_event != SubsecondTime::MaxTime() && next_event > now + 1ul)
   {
      #ifdef DEBUG_PERCYCLE
         std::cout<<"++ Skip "<<SubsecondTime::divideRounded(next_event - now, now.getPeriod())<<std::endl;
      #endif
      will_skip = true;
      skip = next_event - now;
   }
   else
   {
      will_skip = false;
      skip = now.getPeriod();
   }

   #ifdef ASSERT_SKIP
      now += now.getPeriod();
      latency += now.getPeriod();
      if (will_skip)
         time_skipped += now.getPeriod();
   #else
      now += skip;
      latency += skip;
      if (skip > now.getPeriod())
         time_skipped += skip - now.getPeriod();
   #endif

   if (m_mlp_histogram)
      countOutstandingMemop(skip);

   LOG_ASSERT_ERROR(cpiComponent != NULL, "We expected cpiComponent to be set by doDispatch, but it wasn't");
   *cpiComponent += latency;
}

void RobTimer::countOutstandingMemop(SubsecondTime time)
{
   UInt64 counts[HitWhere::NUM_HITWHERES] = {0}, total = 0;

   for(unsigned int i = 0; i < m_num_in_rob; ++i)
   {
      RobEntry *e = &rob.at(i);
      if (e->done != SubsecondTime::MaxTime() && e->done > now && e->uop->getMicroOp()->isLoad())
      {
         ++counts[e->uop->getDCacheHitWhere()];
         ++total;
      }
   }

   for(unsigned int h = 0; h < HitWhere::NUM_HITWHERES; ++h)
      if (counts[h] > 0)
         m_outstandingLoads[h][counts[h] >= MAX_OUTSTANDING ? MAX_OUTSTANDING-1 : counts[h]] += time;
   if (total > 0)
      m_outstandingLoadsAll[total >= MAX_OUTSTANDING ? MAX_OUTSTANDING-1 : total] += time;
}

void RobTimer::printRob()
{
   std::cout<<"** ROB state @ "<<SubsecondTime::divideRounded(now, now.getPeriod())<<"  size("<<m_num_in_rob<<") total("<<rob.size()<<")"<<std::endl;
   if (frontend_stalled_until > now)
   {
      std::cout<<"   Front-end stalled";
      if (frontend_stalled_until != SubsecondTime::MaxTime())
         std::cout << " until " << SubsecondTime::divideRounded(frontend_stalled_until, now.getPeriod());
      if (in_icache_miss)
         std::cout << ", in I-cache miss";
      std::cout<<std::endl;
   }
   std::cout<<"   RS entries: "<<m_rs_entries_used<<std::endl;
   std::cout<<"   Outstanding loads: "<<load_queue.getNumUsed(now)<<"  stores: "<<store_queue.getNumUsed(now)<<std::endl;
   std::cout<<"   Outstanding Vec loads: "<<vec_load_queue.getNumUsed(now)<<"  Vec stores: "<<vec_store_queue.getNumUsed(now)<<std::endl;
   for(unsigned int i = 0; i < rob.size(); ++i)
   {
      std::cout<<"   ["<<std::setw(3)<<i<<"]  ";
      RobEntry *e = &rob.at(i);

      std::ostringstream state;
      if (i >= m_num_in_rob) state<<"PREROB ";
      else if (e->done != SubsecondTime::MaxTime()) {
         uint64_t cycles;
         if (e->done > now)
            cycles = SubsecondTime::divideRounded(e->done-now, now.getPeriod());
         else
            cycles = 0;
         state<<"DONE@+"<<cycles<<"  ";
      }
      else if (e->ready != SubsecondTime::MaxTime()) {
         uint64_t cycles;
         if (e->ready > now)
            cycles = SubsecondTime::divideRounded(e->ready-now, now.getPeriod());
         else
            cycles = 0;
         state<<"READY@+"<<cycles<<"  ";
      }
      else
      {
         state<<"DEPS ";
         for(uint32_t j = 0; j < e->uop->getDependenciesLength(); j++)
            state << std::dec << e->uop->getDependency(j) << " ";
      }
      std::cout<<std::left<<std::setw(20)<<state.str()<<"   ";
      std::cout<<std::right<<std::setw(10)<<e->uop->getSequenceNumber()<<"  ";
      if (e->uop->getMicroOp()->isLoad())
         std::cout<<"LOAD      ";
      else if (e->uop->getMicroOp()->isStore())
         std::cout<<"STORE     ";
      else
         std::cout<<"EXEC ("<<std::right<<std::setw(2)<<e->uop->getExecLatency()<<") ";
      if (e->uop->getMicroOp()->getInstruction())
      {
         std::cout<<std::hex<<e->uop->getMicroOp()->getInstruction()->getAddress()<<std::dec<<": "
                  <<e->uop->getMicroOp()->getInstruction()->getDisassembly();
         if (e->uop->getMicroOp()->isLoad() || e->uop->getMicroOp()->isStore())
            std::cout<<"  {0x"<<std::hex<<e->uop->getAddress().address<<std::dec<<"}";
      }
      else
         std::cout<<"(dynamic)";
      std::cout<<std::endl;
   }
}


void RobTimer::setVSETDependencies(DynamicMicroOp& microOp, uint64_t lowestValidSequenceNumber)
{
  dl::Decoder *dec = Sim()->getDecoder();

  if (dec->is_vsetvl(microOp.getMicroOp()->getInstructionOpcode())) {
    m_vsetvl_producer = microOp.getSequenceNumber();
  } else if (dec->is_vector(microOp.getMicroOp()->getInstructionOpcode(),
                            microOp.getMicroOp()->getDecodedInstruction())) {
    if (m_vsetvl_producer >= lowestValidSequenceNumber) {
      microOp.addDependency(m_vsetvl_producer);
    }
  }
}
