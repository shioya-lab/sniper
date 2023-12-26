#ifndef PREFETCHER_H
#define PREFETCHER_H

#include "core.h"
#include "fixed_types.h"

#include <vector>

class Prefetcher
{
   public:
      static Prefetcher* createPrefetcher(String type, String configName, core_id_t core_id, UInt32 shared_cores);

      virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, uint64_t uop_idx, core_id_t core_id) = 0;
};

#endif // PREFETCHER_H
