#include "memory_dependencies.h"

MemoryDependencies::MemoryDependencies()
    : m_gather_scatter_merge(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/gather_scatter_merge", 0)),
      m_cfg_vldq_merge (Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/vldq_merge", 0)),
      m_l1d_block_size(Sim()->getCfg()->getInt("perf_model/l1_dcache/cache_block_size")),
      m_vlen(Sim()->getCfg()->getInt("general/vlen")),
      producers(Sim()->getCfg()->getInt("perf_model/core/interval_timer/window_size")) // Maximum size should be one ROB worth of instructions
{
   clear();
}

MemoryDependencies::~MemoryDependencies()
{
}

void MemoryDependencies::setDependencies(DynamicMicroOp &microOp, uint64_t lowestValidSequenceNumber, uint64_t first_uop_seqnum)
{
   // Remove all entries that are now below lowestValidSequenceNumber
   clean(lowestValidSequenceNumber);

   if (microOp.getMicroOp()->isLoad())
   {
      uint64_t physicalAddress = microOp.getLoadAccess().phys;
      uint64_t memorySize = microOp.getMicroOp()->getMemoryAccessSize();
      if (microOp.getMicroOp()->isVector()) {
         if (m_gather_scatter_merge) {
            memorySize = m_vlen / 8;
         }
         if (m_cfg_vldq_merge) {
            if (microOp.isFirst()) {
               vldq_base_address = physicalAddress;
               vldq_base_size    = memorySize;
               vldq_base_mask    = ~(memorySize-1);
               // fprintf (stderr, "VLDQ Alloc: PC=%08lx %s: 0x%08lx, %lx\n", microOp.getMicroOp()->getInstruction()->getAddress(),
               //          microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
               //          physicalAddress, memorySize);
            } else {
               uint64_t aligned_base, aligned_size;
               calculate_aligned_base_and_power2_size (physicalAddress, memorySize, vldq_base_address, vldq_base_size, &aligned_base, &aligned_size);
               vldq_base_address = aligned_base;
               vldq_base_size    = aligned_size;
               vldq_base_mask    = vldq_base_mask & ~(physicalAddress ^ vldq_base_address);
               // fprintf (stderr, "VLDQ Merge: PC=%08lx %s: 0x%08lx,%lx -> 0x%08lx,%lx,%08lx\n", microOp.getMicroOp()->getInstruction()->getAddress(),
               //          microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
               //          physicalAddress, memorySize, aligned_base, aligned_size, vldq_base_mask);
               physicalAddress = vldq_base_address;
               memorySize      = vldq_base_size;
            }
         }
      }

      uint64_t found_st_idx;
      uint64_t producerSequenceNumber = find(physicalAddress, memorySize, found_st_idx);
      if (producerSequenceNumber != INVALID_SEQNR) /* producer found */
      {
         // fprintf (stderr, "  Producer found: PC=%08lx %s: 0x%08lx, %lx -> 0x%08lx, %lx\n", microOp.getMicroOp()->getInstruction()->getAddress(),
         //          microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
         //          physicalAddress, memorySize, producers.at(found_st_idx).address, producers.at(found_st_idx).size);
         microOp.addDependency(producerSequenceNumber);
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
      add(microOp.getSequenceNumber(), physicalAddress, memorySize);
      // fprintf (stderr, "Store Register: PC=0x%08lx %s: 0x%08lx, %lx\n", microOp.getMicroOp()->getInstruction()->getAddress(),
      //          microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
      //                   physicalAddress, memorySize);

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

uint64_t MemoryDependencies::find(uint64_t address, uint64_t size, uint64_t &index)
{
   // There may be multiple entries with the same address, we want the latest one so traverse list in reverse order
   for(int i = producers.size() - 1; i >= 0; --i)
     if ((producers.at(i).address & ~(size-1)) == (address & ~(size-1))) {
         index = i;
         return producers.at(i).seqnr;
     }
   return INVALID_SEQNR;
}

void MemoryDependencies::update(uint64_t sequenceNumber, uint64_t address, uint64_t size)
{
   // There may be multiple entries with the same address, we want the latest one so traverse list in reverse order
   for(int i = producers.size() - 1; i >= 0; --i) {
      if (producers.at(i).seqnr == sequenceNumber) {
         producers.at(i).address = address;
         producers.at(i).size = size;
      }
   }
}


void MemoryDependencies::findAddress(uint64_t seqNumber,
                                     uint64_t &address,
                                     uint64_t &size)
{
   address = INVALID_SEQNR;
   for(int i = producers.size() - 1; i >= 0; --i) {
      if (producers.at(i).seqnr == seqNumber) {
         address = producers.at(i).address;
         size = producers.at(i).size;
      }
   }
   return;
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
