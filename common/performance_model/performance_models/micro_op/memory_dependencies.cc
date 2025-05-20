#include <cstdio>
#include <stdint.h>
#include <stdint.h>
#include "memory_dependencies.h"
#include "dynamic_micro_op.h"
#include "stats.h"

MemoryDependencies::MemoryDependencies()
    : m_gather_scatter_merge(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/gather_scatter_merge", 0)),
      m_cfg_vldq_merge (Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/vldq_merge", 0)),
      m_vldq_merge_slots (Sim()->getCfg()->getInt("perf_model/core/rob_timer/vldq_merge_slots")),
      m_cfg_bloom_filter(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/bloom_filter", 0)),
      m_cfg_bloom_filter_len (Sim()->getCfg()->getInt("perf_model/core/rob_timer/bloom_filter_length")),
      m_cfg_bloom_filter_addr_lsb (Sim()->getCfg()->getInt("perf_model/core/rob_timer/bloom_filter_addr_lsb")),
      m_l1d_block_size(Sim()->getCfg()->getInt("perf_model/l1_dcache/cache_block_size")),
      m_vlen(Sim()->getCfg()->getInt("general/vlen")),
      producers(Sim()->getCfg()->getInt("perf_model/core/interval_timer/window_size")) // Maximum size should be one ROB worth of instructions
{
   clear();

   registerStatsMetric("memory_access", 0, "num_vldq_conflict_check", &m_num_vldq_conflict_check);
   registerStatsMetric("memory_access", 0, "num_vldq_conflict",       &m_num_vldq_conflict);
   registerStatsMetric("memory_access", 0, "num_vldq_conflict_false_positive", &m_num_vldq_conflict_false_positive);

}

MemoryDependencies::~MemoryDependencies()
{
}

void MemoryDependencies::mergeVLDQAddressMultiSlot(DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize)
{
   uint64_t pa = physicalAddress;
   uint64_t size = memorySize;
   uint64_t aligned_base, aligned_size;

   // スロットの初期化（必要なら）
   if (vldq_base_addresses.size() != m_vldq_merge_slots) {
      vldq_base_addresses.resize(m_vldq_merge_slots);
      vldq_base_sizes.resize(m_vldq_merge_slots);
      vldq_base_masks.resize(m_vldq_merge_slots);
      vldq_slots_used.assign(m_vldq_merge_slots, false);
   }

   // Step 1: 既存スロットの重複チェック
   for (unsigned int i = 0; i < m_vldq_merge_slots; ++i) {
      if (!vldq_slots_used[i]) continue;

      uint64_t base = vldq_base_addresses[i];
      uint64_t end = base + vldq_base_sizes[i];
      uint64_t pa_end = pa + size;

      bool overlap = !(pa_end <= base || pa >= end); // 範囲がかぶっているか

         if (overlap) {
         // 既存スロットに統合
         calculate_aligned_base_and_power2_size(pa, size, vldq_base_addresses[i],
                                                vldq_base_sizes[i],
                                                &aligned_base, &aligned_size);
         vldq_base_addresses[i] = aligned_base;
         vldq_base_sizes[i] = aligned_size;
         vldq_base_masks[i] &= ~(pa ^ aligned_base);

         physicalAddress = aligned_base;
         memorySize = aligned_size;

         fprintf (stderr, "VLDQ found merge slot(%d): PC=0x%08lx %s: 0x%08lx, %lx merged into 0x%08lx, %lx\n", 
                  i, microOp.getMicroOp()->getInstruction()->getAddress(),
                  microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
                  pa, size, physicalAddress, memorySize);
         return;
      }
   }

   // 空きスロットを探す
   for (unsigned int i = 0; i < m_vldq_merge_slots; ++i) {
      if (!vldq_slots_used[i]) {
         aligned_size = m_vlen / 8;
         // aligned_size = size;
         aligned_base = pa & ~(aligned_size - 1);
         vldq_base_addresses[i] = aligned_base;
         vldq_base_sizes[i] = aligned_size;
         vldq_base_masks[i] = ~(aligned_size - 1);
         vldq_slots_used[i] = true;

         physicalAddress = aligned_base;
         memorySize = aligned_size;

         fprintf (stderr, "VLDQ initial slot(%d): PC=0x%08lx %s: 0x%08lx, %lx\n", 
                  i, microOp.getMicroOp()->getInstruction()->getAddress(),
                  microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
                  physicalAddress, memorySize);
         return;
      }
   }

   // すべてのスロットが使用中 → 近いスロットを見つけてマージ
   uint64_t min_distance = UINT64_MAX;
   unsigned int chosen = 0;
   for (unsigned int i = 0; i < m_vldq_merge_slots; ++i) {
      uint64_t dist = (pa > vldq_base_addresses[i])
                           ? pa - vldq_base_addresses[i]
                           : vldq_base_addresses[i] - pa;
      if (dist < min_distance) {
         min_distance = dist;
         chosen = i;
      }
   }

   // 選ばれたスロットとマージ
   calculate_aligned_base_and_power2_size(pa, size, vldq_base_addresses[chosen],
                                          vldq_base_sizes[chosen],
                                          &aligned_base, &aligned_size);

   vldq_base_addresses[chosen] = aligned_base;
   vldq_base_sizes[chosen] = aligned_size;
   vldq_base_masks[chosen] &= ~(pa ^ aligned_base);

   fprintf(stderr, "VLDQ overflow merge slot(%d): PC=0x%08lx %s: 0x%08lx, %lx merged into 0x%08lx, %lx\n", 
           chosen, microOp.getMicroOp()->getInstruction()->getAddress(),
           microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
           pa, size, aligned_base, aligned_size);

   physicalAddress = aligned_base;
   memorySize = aligned_size;
}


// MurmurHash3風（64bit key対応）
uint64_t murmur_hash(uint64_t key) {
   key ^= key >> 33;
   key *= 0xff51afd7ed558ccdULL;
   key ^= key >> 33;
   key *= 0xc4ceb9fe1a85ec53ULL;
   key ^= key >> 33;
   return key;
}

// FNV-1a風（uint64_t版）
uint64_t fnv1a_hash(uint64_t key) {
   const uint64_t FNV_offset_basis = 14695981039346656037ULL;
   const uint64_t FNV_prime = 1099511628211ULL;

   uint64_t hash = FNV_offset_basis;
   for (int i = 0; i < 8; ++i) {
      uint8_t byte = (key >> (i * 8)) & 0xFF;
      hash ^= byte;
      hash *= FNV_prime;
   }
   return hash;
}

// 自作ミックス関数（高速）
uint64_t simple_mix_hash(uint64_t x) {
   x = (~x) + (x << 21); // x = (x << 21) - x - 1
   x = x ^ (x >> 24);
   x = (x + (x << 3)) + (x << 8); // x * 265
   x = x ^ (x >> 14);
   x = (x + (x << 2)) + (x << 4); // x * 21
   x = x ^ (x >> 28);
   x = x + (x << 31);
   return x;
}

uint64_t xor_fold_hash(uint64_t address, unsigned int N) {
   assert(N > 0 && N <= 64);

   uint64_t mask = (N == 64) ? ~0ULL : ((1ULL << N) - 1);
   uint64_t result = 0;

   // 繰り返しNビットずつ取り出してXOR
   for (int shift = 0; shift < 64; shift += N) {
      result ^= (address >> shift) & mask;
   }

   return result & mask;
}


void MemoryDependencies::RegisterBloomFilter (DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize)
{
   uint64_t pa_biased = physicalAddress >> m_cfg_bloom_filter_addr_lsb;
   uint64_t load_hash0 = xor_fold_hash(murmur_hash(pa_biased),   m_cfg_bloom_filter_len);
   uint64_t load_hash1 = xor_fold_hash(fnv1a_hash(pa_biased),    m_cfg_bloom_filter_len);
   uint64_t load_hash2 = xor_fold_hash(simple_mix_hash(pa_biased), m_cfg_bloom_filter_len);

   if (microOp.getMicroOp()->isFirst()) {
      // Clear the bloom filter list if this is the first load
      bloom_filter_list.clear();
   }

   bloom_filter_list.insert (load_hash0);
   bloom_filter_list.insert (load_hash1);
   bloom_filter_list.insert (load_hash2);

   // if (microOp.getMicroOp()->isLast()) {
   //    fprintf (stderr, "  Bloom Filter: ");
   //    for (auto it = bloom_filter_list.begin(); it != bloom_filter_list.end(); ++it) {
   //       fprintf (stderr, "%ld, ", *it);
   //    }
   //    fprintf (stderr, "\n");
   // }

   m_num_vldq_conflict_check ++;

   // There may be multiple entries with the same address, we want the latest one so traverse list in reverse order
   for(int i = producers.size() - 1; i >= 0; --i) {
      uint64_t st_pa_biased = producers.at(i).address >> m_cfg_bloom_filter_addr_lsb;
      uint64_t store_hash0 = xor_fold_hash(murmur_hash(st_pa_biased), m_cfg_bloom_filter_len);
      uint64_t store_hash1 = xor_fold_hash(fnv1a_hash(st_pa_biased), m_cfg_bloom_filter_len);
      uint64_t store_hash2 = xor_fold_hash(simple_mix_hash(st_pa_biased), m_cfg_bloom_filter_len);
      if ((bloom_filter_list.find(store_hash0) != bloom_filter_list.end()) && 
          (bloom_filter_list.find(store_hash1) != bloom_filter_list.end()) && 
          (bloom_filter_list.find(store_hash2) != bloom_filter_list.end())) {
         // Found a match
         // fprintf (stderr, "  Bloom Filter found: PC=%08lx %s: 0x%08lx, %lx -> 0x%08lx, %lx. ST Hash %ld, %ld, %ld\n", microOp.getMicroOp()->getInstruction()->getAddress(),
         //          microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
         //          physicalAddress, memorySize, producers.at(i).address, producers.at(i).size,
         //          store_hash0, store_hash1, store_hash2);
         microOp.addDependency(producers.at(i).seqnr);

         m_num_vldq_conflict ++;
         uint64_t found_st_idx;
         uint64_t producerSequenceNumber = find(physicalAddress, memorySize, found_st_idx);
         if (producerSequenceNumber == INVALID_SEQNR) /* producer not found */
         {
            // fprintf (stderr, " False positive found: PC=%08lx %s: 0x%08lx, %lx\n", microOp.getMicroOp()->getInstruction()->getAddress(),
            //          microOp.getMicroOp()->getInstruction()->getDisassembly().c_str(), 
            //          physicalAddress, memorySize);
            m_num_vldq_conflict_false_positive ++;
         }
         return;
      }
   }
}

void MemoryDependencies::setDependencies(DynamicMicroOp &microOp, uint64_t lowestValidSequenceNumber, uint64_t first_uop_seqnum)
{
   // Remove all entries that are now below lowestValidSequenceNumber
   clean(lowestValidSequenceNumber);

   if (microOp.getMicroOp()->isLoad())
   {
      uint64_t physicalAddress = microOp.getLoadAccess().phys;
      uint64_t memorySize = microOp.getMicroOp()->getMemoryAccessSize();

      if (m_cfg_bloom_filter && microOp.getMicroOp()->isVector()) {
         RegisterBloomFilter(microOp, physicalAddress, memorySize);
      } else {
         if (microOp.getMicroOp()->isVector()) {
            // if (m_cfg_vldq_merge) {
            //    mergeVLDQAddress(microOp, physicalAddress, memorySize);
            // } 
            if (m_cfg_vldq_merge) {
               if (microOp.isFirst()) {
                  vldq_slots_used.assign(m_vldq_merge_slots, false);
               }
               mergeVLDQAddressMultiSlot(microOp, physicalAddress, memorySize);
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
      }

      if ((membar != INVALID_SEQNR) && (membar > lowestValidSequenceNumber))
      {
         microOp.addDependency(membar);
      }
   }
   else if (microOp.getMicroOp()->isStore())
   {
      uint64_t physicalAddress = microOp.getStoreAccess().phys;
      uint64_t memorySize = microOp.getMicroOp()->getMemoryAccessSize();
      // if (microOp.getMicroOp()->isVector()) {
      //   physicalAddress &= ~(m_l1d_block_size-1);
      //   // memorySize = m_l1d_block_size;
      //   memorySize = m_vlen / 8;
      // }
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
