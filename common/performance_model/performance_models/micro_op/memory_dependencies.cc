#include "memory_dependencies.h"

MemoryDependencies::MemoryDependencies()
    : m_gather_scatter_merge(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/gather_scatter_merge", 0)),
      m_l1d_block_size(Sim()->getCfg()->getInt("perf_model/l1_dcache/cache_block_size")),
      m_vlen(Sim()->getCfg()->getInt("general/vlen")),
      producers(1024) // Maximum size should be one ROB worth of instructions
{
   clear();
}

MemoryDependencies::~MemoryDependencies()
{
}

void MemoryDependencies::setDependencies(DynamicMicroOp &microOp, uint64_t lowestValidSequenceNumber)
{
   // Remove all entries that are now below lowestValidSequenceNumber
   clean(lowestValidSequenceNumber);

   if (microOp.getMicroOp()->isLoad())
   {
      uint64_t physicalAddress = microOp.getLoadAccess().phys;
      uint64_t memorySize = microOp.getMicroOp()->getMemoryAccessSize();
      if (microOp.getMicroOp()->isVector() && m_gather_scatter_merge) {
        // physicalAddress &= ~(m_l1d_block_size-1);
        // memorySize = m_l1d_block_size;
        memorySize = m_vlen / 8;
      }
      uint64_t producerSequenceNumber = find(physicalAddress, memorySize);
      if (producerSequenceNumber != INVALID_SEQNR) /* producer found */
      {
#ifdef DEBUG_PERCYCLE
        std::cerr << "Forwarding Found: "<< microOp.getMicroOp()->getInstruction()->getDisassembly() << " : " <<
            "(" << std::dec << microOp.getSequenceNumber() << ") : " <<
            std::hex << physicalAddress << ", size = " << std::dec << memorySize <<
            " with " << std::dec << producerSequenceNumber << std::endl;
        std::cerr << "Before getDependenciesLength = " << microOp.getDependenciesLength() << std::endl;
#endif  // DEBUG_PERCYCLE
        microOp.addDependency(producerSequenceNumber);
#ifdef DEBUG_PERCYCLE
        std::cerr << "After  getDependenciesLength = " << microOp.getDependenciesLength() << " : ";
        for (uint32_t i = 0; i < microOp.getDependenciesLength(); i++) {
          std::cerr << microOp.getDependency(i) << ",";
        }
        std::cerr << std::endl;
#endif  // DEBUG_PERCYCLE
      }

      if ((membar != INVALID_SEQNR) && (membar > lowestValidSequenceNumber))
      {
         microOp.addDependency(membar);
      }
   }
   else if (microOp.getMicroOp()->isStore() && m_gather_scatter_merge)
   {
      uint64_t physicalAddress = microOp.getStoreAccess().phys;
      uint64_t memorySize = microOp.getMicroOp()->getMemoryAccessSize();
      if (microOp.getMicroOp()->isVector()) {
        physicalAddress &= ~(m_l1d_block_size-1);
        // memorySize = m_l1d_block_size;
        memorySize = m_vlen / 8;
      }
#ifdef DEBUG_PERCYCLE
      std::cerr << "Set store: "<< microOp.getMicroOp()->getInstruction()->getDisassembly() << " : " <<
          std::hex << physicalAddress << "size = " << std::dec << memorySize <<
          std::endl;
#endif  // DEBUG_PERCYCLE
      add(microOp.getSequenceNumber(), physicalAddress, memorySize);

      // Stores are also dependent on membars
      if ((membar != INVALID_SEQNR) && (membar > lowestValidSequenceNumber))
      {
         microOp.addDependency(membar);
      }

   }
   else if (microOp.getMicroOp()->isMemBarrier())
   {
      // And membars are dependent on previous membars
      if ((membar != INVALID_SEQNR) && (membar > lowestValidSequenceNumber))
      {
         microOp.addDependency(membar);
      }

      // Actual MFENCE instruction
      // All new instructions will have higher sequence numbers
      // Therefore, this will remain sorted
      membar = microOp.getSequenceNumber();
   }
}

void MemoryDependencies::add(uint64_t sequenceNumber, uint64_t address, uint64_t size)
{
   Producer producer = {sequenceNumber, address, size};
   producers.push(producer);
}

uint64_t MemoryDependencies::find(uint64_t address, uint64_t size)
{
   // There may be multiple entries with the same address, we want the latest one so traverse list in reverse order
   for(int i = producers.size() - 1; i >= 0; --i)
     if ((producers.at(i).address & ~(size-1)) == (address & ~(size-1)))
         return producers.at(i).seqnr;
   return INVALID_SEQNR;
}

void MemoryDependencies::clean(uint64_t lowestValidSequenceNumber)
{
   while(!producers.empty() && producers.front().seqnr < lowestValidSequenceNumber)
      producers.pop();
}

void MemoryDependencies::clear()
{
   while(!producers.empty())
      producers.pop();
   membar = INVALID_SEQNR;
}
