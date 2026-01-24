#ifndef __MEMORY_DEPENDENCIES_H
#define __MEMORY_DEPENDENCIES_H

#include "fixed_types.h"
#include "circular_queue.h"
#include "dynamic_micro_op.h"
#include "config.hpp"
#include "instruction.h"

class MemoryDependencies
{
   private:
   bool m_gather_scatter_merge;
   bool m_cfg_vldq_merge;
   unsigned int m_vldq_merge_slots;
   bool m_cfg_bloom_filter;
   uint64_t m_cfg_bloom_filter_len;
   uint64_t m_cfg_bloom_filter_addr_lsb;
   
   UInt64 m_l1d_block_size;
   UInt64 m_vlen;

   UInt64 m_num_vldq_conflict_check = 0;
   UInt64 m_num_vldq_conflict = 0;
   UInt64 m_num_vldq_conflict_false_positive = 0;

  struct Producer
      {
         uint64_t seqnr;
         uint64_t address;
         uint64_t size;
      };
      // List of all active writers, ordered by sequence number
      // This makes it easy to remove old entries, just compare the front of the queue with lowestValidSequenceNumber
      // Finding the latest producer is a linear search, starting from the tail (there may be multiple entries
      // with the same address, we want the latest one).
      // Maximum number of entries is the number of instructions in the ROB, actual number of entries equals
      // the number of writes in the ROB which is usually not larger than around 20.
      // More fancy data structures, such as an underdered_map keyed on address, have a much higher overhead on insertion
      // and put significant pressure on malloc()/free(), and are therefore not recommended.
      CircularQueue<Producer> producers;
      uint64_t membar;

      void add(uint64_t sequenceNumber, uint64_t address, uint64_t size);
      uint64_t find(uint64_t address, uint64_t size, uint64_t &index);
      void clean(uint64_t lowestValidSequenceNumber);
      void update(uint64_t sequenceNumber, uint64_t address, uint64_t size);

      void findAddress(UInt64 seqNumber, UInt64 &address, UInt64 &size);

      // uint64_t vldq_base_address = 0;
      // uint64_t vldq_base_size    = 0;
      // uint64_t vldq_base_mask    = 0;
      
      std::vector<uint64_t> vldq_base_addresses;
      std::vector<uint64_t> vldq_base_sizes;
      std::vector<uint64_t> vldq_base_masks;
      std::vector<bool>     vldq_slots_used;

      // Bloom Filter向けの実装
      std::set<uint64_t> bloom_filter_list;

   public:
      MemoryDependencies();
      ~MemoryDependencies();

      void setDependencies(DynamicMicroOp &microOp, uint64_t lowestValidSequenceNumber, uint64_t first_uop_seqnum = INVALID_SEQNR);
      void clear();

      void mergeVLDQAddress(DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize);
      void mergeVLDQAddressMultiSlot(DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize);

      void RegisterBloomFilter (DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize);
      void RegisterBloomFilter2 (DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize);
      void RegisterBloomFilter3 (DynamicMicroOp &microOp, uint64_t &physicalAddress, uint64_t &memorySize);

   private:
      // 最小の2のべき乗を返す（例: 130 -> 256）
      uint64_t roundup_power2(uint64_t x) {
         if (x == 0) return 1;
         x--;
         x |= x >> 1;
         x |= x >> 2;
         x |= x >> 4;
         x |= x >> 8;
         x |= x >> 16;
         x |= x >> 32;  // 64bit対応
         return x + 1;
      }

      void calculate_aligned_base_and_power2_size(uint64_t A, uint64_t Asize,
                                                uint64_t B, uint64_t Bsize,
                                                uint64_t *base, uint64_t *size) {
         uint64_t start = (A < B) ? A : B;
         uint64_t end = (A + Asize > B + Bsize) ? (A + Asize) : (B + Bsize);

         uint64_t range = end - start;
         uint64_t access_size = roundup_power2(range);

         // start を access_size にアライン
         *base = start & ~(access_size - 1);
         // 再計算：base から access_size が end をカバーしていない場合、サイズを2倍にする
         while (*base + access_size < end) {
            access_size <<= 1;
            *base = start & ~(access_size - 1);
         }

         *size = access_size;
      }

};

#endif /* __MEMORY_DEPENDENCIES_H */
