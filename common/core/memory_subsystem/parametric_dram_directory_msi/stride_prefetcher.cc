#include "stride_prefetcher.h"
#include "simulator.h"
#include "config.hpp"

#include <cstdlib>

const IntPtr PAGE_SIZE = 4096;
const IntPtr PAGE_MASK = ~(PAGE_SIZE-1);


StridePrefetcher::StridePrefetcher(String configName, core_id_t _core_id, UInt32 _shared_cores)
   : core_id(_core_id)
   , shared_cores(_shared_cores)
   , configName(configName)
   , m_stride_table_size     (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stride/table_size", core_id))
   , m_degree                (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/prefetcher/stride/degree", core_id))
   , m_cache_block_size      (Sim()->getCfg()->getIntArray("perf_model/" + configName + "/cache_block_size", core_id))
   , m_enable_log            (Sim()->getCfg()->getBoolArray("log/enable_stride_prefetcher_log", core_id))
{
}


std::vector<IntPtr>
StridePrefetcher::getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, core_id_t _core)
{
  if (m_enable_log) {
    fprintf(stderr, "  %s StridePrefetcher::getNextAddress(pc=%08lx, addr=%08lx) start :\n", configName.c_str(), pc, current_address);
  }
  std::vector<IntPtr> addresses;

  // Monitor
  for (size_t i = 0; i < m_stride_table.size(); i++){
    Stride* stride = m_stride_table[i];

    if (stride->pc != pc) {
      continue;
    }

    if (stride->addr + stride->stride == current_address) {
      if (stride->confidence >= CONFIDENCE_PREDICTION_THREASHOLD) {
        for (int d = 0; d < m_degree; d++) {
          IntPtr prefetch = current_address + stride->stride * (d + 1);
          addresses.push_back(prefetch);
          if (m_enable_log) {
            fprintf(stderr, "   %s: StridePrefetcher::push_address entry index = %ld, addr = %08lx\n",
                    configName.c_str(), i, prefetch);
          }
        }
      }
      stride->confidence++;
      if (stride->confidence > CONFIDENCE_MAX) {
        stride->confidence = CONFIDENCE_MAX;
      }
    } else if (current_address != stride->addr) {
      stride->confidence--;
      if (stride->confidence < CONFIDENCE_MIN) {
        stride->confidence = CONFIDENCE_MIN;
      }
    }

    if (current_address - stride->addr != 0) {
      stride->stride = current_address - stride->addr;
    }
    stride->addr   = current_address;

    if (m_enable_log) {
      fprintf(stderr, "   %s: StridePrefetcher::current_address pc = %08lx, addr = %08lx, stride = %08lx, entry index = %ld, confidence = %d\n",
              configName.c_str(),
              pc, stride->addr, stride->stride, i, stride->confidence
              );
    }

    // Update the table
    stride->count++;

    return addresses;
  }

  AllocateStride (pc, current_address);

  return addresses;
}


// Allocate a new entry in the stride table.
void StridePrefetcher::AllocateStride (const IntPtr pc, const IntPtr current_address)
{
  Stride *stride = new Stride;
  stride->pc = pc;
  stride->addr = current_address;
  stride->count = 0;
  stride->stride = 0;
  stride->confidence = 0;

  if (m_stride_table.size() >= m_stride_table_size) {
    // m_stride_table.pop_front();
    m_stride_table.erase(m_stride_table.begin());
  }
  m_stride_table.push_back(stride);
  // m_stride_table.touch( target);

  if (m_enable_log) {
    fprintf(stderr, " %s: StridePrefetcher::AllocateStride::pc = %08lx, current_address = %08lx\n",
            configName.c_str(), pc, current_address);
  }
}
