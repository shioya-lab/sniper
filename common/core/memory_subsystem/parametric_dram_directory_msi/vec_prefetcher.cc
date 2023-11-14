#include "vec_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);


VecPrefetcher::VecPrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , configName(configName)
   , m_vec_stride_table_size    (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/vec_pref/vec_table_size", core_id))
   , m_scalar_stride_table_size (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/vec_pref/scalar_table_size", core_id))
   , m_degree                   (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/vec_pref/degree", core_id))
   , m_cache_block_size         (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , m_enable_log               (Sim()->getCfg()->getBoolArray("log/enable_vec_prefetcher_log", core_id))
{
  m_last_pc = 0;
}


std::vector<IntPtr>
VecPrefetcher::getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, core_id_t _core)
{
  if (pc == 0) { return std::vector<IntPtr>(0); }

  if (m_enable_log) {
    fprintf(stderr, "  %s VecPrefetcher::getNextAddress(pc=%08lx, addr=%08lx) start :\n", configName.c_str(), pc, current_address);
  }

  bool vec_access = mem_op_type == Core::READ_VEC || mem_op_type == Core::WRITE_VEC;

  if (vec_access) {
    return getVectorNextAddress (pc, current_address);
  } else {
    return getScalarNextAddress (pc, current_address);
  }
  // Monitor
}


std::vector<IntPtr> VecPrefetcher::getVectorNextAddress(IntPtr pc, IntPtr current_address)
{
  std::vector<IntPtr> addresses;

  IntPtr current_block = current_address & ~(m_cache_block_size-1);

  for (size_t i = 0; i < m_vec_stride_table.size(); i++){
    VecStride* entry = m_vec_stride_table[i];

    if (entry->m_pc != pc) {
      continue;
    }

    if (m_last_pc == pc) {
      if (entry->last_addr == current_block) {
        if (m_enable_log) {
          fprintf(stderr, "   %s: VecPrefetcher::Vector same address access. entry index = %ld, last_addr = %08lx (degree = %d, vec_size = %d)\n",
                  configName.c_str(), i, entry->last_addr, entry->vec_degree(), entry->vec_size);
        }
      } else {
        // access sequentially in same PC, increase size
        entry->vec_size += m_cache_block_size;
        entry->last_addr = current_block;
        if (m_enable_log) {
          fprintf(stderr, "   %s: VecPrefetcher::Vector same pc sequential update. entry index = %ld, last addr = %08lx (degree = %d, vec_size = %d)\n",
                  configName.c_str(), i, entry->last_addr, entry->vec_degree(), entry->vec_size);
        }
      }
      return addresses;
    }

    if (m_enable_log) {
      fprintf(stderr, "   %s: VecPrefetcher::Vector current_address pc = %08lx, base_addr = %08lx, last_addr = %08lx, stride = %08lx, entry index = %ld, confidence = %d\n",
              configName.c_str(),
              pc, entry->base_addr, entry->last_addr, entry->stride, i, entry->confidence
              );
    }

    if (entry->base_addr + entry->stride == current_address) {
      if (m_enable_log) {
        fprintf(stderr, "   %s: VecPrefetcher::Vector hits entry_index=%ld, base_addr=%08lx, stride=%08lx, vec_size=%d\n",
                configName.c_str(), i, entry->base_addr, entry->stride,  entry->vec_size);
      }
      if (entry->can_prefetch()) {
        if (m_enable_log) { fprintf(stderr, "   %s: VecPrefetcher::Vector canprefetch = 1\n", configName.c_str()); }
        for (unsigned d = 0; d < entry->m_degree; d++) {
          if (m_enable_log) { fprintf(stderr, "   %s: VecPrefetcher::Vector degree = %d\n", configName.c_str(), entry->m_degree); }
          for (unsigned vd = 0; vd < entry->vec_degree(); vd++) {
            if (m_enable_log) { fprintf(stderr, "   %s: VecPrefetcher::Vector vec_degree = %d\n", configName.c_str(), entry->vec_degree()); }
            IntPtr prefetch = current_address + entry->stride * (d + 1) + vd * m_cache_block_size;
            addresses.push_back(prefetch);
            if (m_enable_log) {
              fprintf(stderr, "   %s: VecPrefetcher::Vector push_address entry index = %ld, prefetch addr = %08lx (degree = %d, vec_size = %d)\n",
                      configName.c_str(), i, prefetch, entry->vec_degree(), entry->vec_size);
            }
          }
        }
      }
      entry->confidence++;
      if (entry->confidence > CONFIDENCE_MAX) {
        entry->confidence = CONFIDENCE_MAX;
      }
    } else if (current_address != entry->base_addr) {
      entry->confidence--;
      if (entry->confidence < CONFIDENCE_MIN) {
        entry->confidence = CONFIDENCE_MIN;
      }
    }

    // Update the table
    if (current_address != entry->base_addr) {
      entry->stride = current_address - entry->base_addr;
    }
    entry->base_addr = current_address;

    if (m_last_pc != pc) {
      entry->vec_size = m_cache_block_size;
    }

    m_last_pc = pc;

    entry->count++;

    return addresses;
  }

  m_last_pc = pc;

  AllocateVecStride (pc, current_address);

  return addresses;

}


std::vector<IntPtr> VecPrefetcher::getScalarNextAddress(IntPtr pc, IntPtr current_address)
{
  std::vector<IntPtr> addresses;

  for (size_t i = 0; i < m_scalar_stride_table.size(); i++){
    ScalarStride* entry = m_scalar_stride_table[i];

    if (entry->m_pc != pc) {
      continue;
    }

    if (m_enable_log) {
      fprintf(stderr, "   %s: VecPrefetcher::Scalar current_address pc = %08lx, base_addr = %08lx, last_addr = %08lx, stride = %08lx, entry index = %ld, confidence = %d\n",
              configName.c_str(),
              pc, entry->base_addr, entry->last_addr, entry->stride, i, entry->confidence
              );
    }

    if (entry->stride != 0 && entry->base_addr + entry->stride == current_address) {
      if (m_enable_log) {
        fprintf(stderr, "   %s: VecPrefetcher::Scalar hits entry_index=%ld, base_addr=%08lx, stride=%08lx\n",
                configName.c_str(), i, entry->base_addr, entry->stride);
      }
      if (entry->can_prefetch()) {
        if (m_enable_log) { fprintf(stderr, "   %s: VecPrefetcher::Scalar canprefetch = 1\n", configName.c_str()); }
        for (unsigned d = 0; d < entry->m_degree; d++) {
          if (m_enable_log) { fprintf(stderr, "   %s: VecPrefetcher::Scalar degree = %d\n", configName.c_str(), entry->m_degree); }
          IntPtr prefetch = current_address + entry->stride * (d + 1);
          if ((prefetch & ~(m_cache_block_size-1)) == (current_address & ~(m_cache_block_size-1))) {
            prefetch += m_cache_block_size;
          }
          addresses.push_back(prefetch);
          if (m_enable_log) {
            fprintf(stderr, "   %s: VecPrefetcher::Scalar push_address entry index = %ld, prefetch addr = %08lx\n",
                    configName.c_str(), i, prefetch);
          }
        }
      }
      entry->confidence++;
      if (entry->confidence > CONFIDENCE_MAX) {
        entry->confidence = CONFIDENCE_MAX;
      }
    } else if (current_address != entry->base_addr) {
      entry->confidence--;
      if (entry->confidence < CONFIDENCE_MIN) {
        entry->confidence = CONFIDENCE_MIN;
      }
    }

    if (current_address != entry->base_addr) {
      entry->stride = current_address - entry->base_addr;
    }
    entry->base_addr = current_address;

    // Update the table
    entry->count++;

    return addresses;
  }

  AllocateScalarStride (pc, current_address);

  return addresses;

}


// Allocate a new entry in the stride table.
void VecPrefetcher::AllocateVecStride (const IntPtr pc, const IntPtr current_address)
{
  VecStride *stride = new VecStride (m_degree, m_cache_block_size,
                                     pc, current_address);

  if (m_vec_stride_table.size() >= m_vec_stride_table_size) {
    // m_vec_stride_table.pop_front();
    if (m_enable_log) {
      fprintf(stderr, "   %s: VecPrefetcher::Vector remove entry. pc = %08lx, addr = %08lx (degree = %d, vec_size = %d)\n",
              configName.c_str(), m_vec_stride_table[0]->m_pc, m_vec_stride_table[0]->base_addr, m_vec_stride_table[0]->vec_degree(), m_vec_stride_table[0]->vec_size);
    }
    m_vec_stride_table.erase(m_vec_stride_table.begin());
  }
  m_vec_stride_table.push_back(stride);
  // m_vec_stride_table.touch( target);

  if (m_enable_log) {
    fprintf(stderr, " %s: VecPrefetcher::AllocateVecStride::pc = %08lx, current_address = %08lx\n",
            configName.c_str(), pc, current_address);
  }
}


// Allocate a new entry in the stride table.
void VecPrefetcher::AllocateScalarStride (const IntPtr pc, const IntPtr current_address)
{
  ScalarStride *stride = new ScalarStride (m_degree, m_cache_block_size,
                                           pc, current_address);

  if (m_scalar_stride_table.size() >= m_scalar_stride_table_size) {
    // m_scalar_stride_table.pop_front();
    if (m_enable_log) {
      fprintf(stderr, "   %s: VecPrefetcher::Scalar remove entry. pc = %08lx, addr = %08lx\n",
              configName.c_str(), m_scalar_stride_table[0]->m_pc, m_scalar_stride_table[0]->base_addr);
    }
    m_scalar_stride_table.erase(m_scalar_stride_table.begin());
  }
  m_scalar_stride_table.push_back(stride);
  // m_scalar_stride_table.touch( target);

  if (m_enable_log) {
    fprintf(stderr, " %s: VecPrefetcher::AllocateScalarStride::pc = %08lx, current_address = %08lx\n",
            configName.c_str(), pc, current_address);
  }
}
