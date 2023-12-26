#ifndef __STRIDE_PREFETCHER_H
#define __STRIDE_PREFETCHER_H

#include "core.h"
#include "prefetcher.h"

class StridePrefetcher : public Prefetcher
{
 public:
  StridePrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
  virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, uint64_t uop_idx,
                                             core_id_t core_id);

 private:

  // A confidence counter has hysteresis characteristics.
  static const int CONFIDENCE_INIT = 0;
  static const int CONFIDENCE_MIN  = 0;
  static const int CONFIDENCE_MAX  = 7;
  static const int CONFIDENCE_INC  = 1;
  static const int CONFIDENCE_DEC  = 4;
  static const int CONFIDENCE_PREDICTION_THREASHOLD = 7;

  struct Stride
  {
    IntPtr orig;      // An first accessed address of a stream
    IntPtr addr;      // A current 'start' address of a stream
    IntPtr stride;

    IntPtr pc;

    int  count;     // Access count
    int  confidence;

    Stride() {
      Reset();
    }

    void Reset()
    {
      count = 0;
    }

    String ToString() const;
  };


  const core_id_t core_id;
  const UInt32    shared_cores;
  const String    configName;

  typedef std::vector< Stride *> StrideTable;

  StrideTable m_stride_table;
  size_t m_stride_table_size;

  int m_degree;
  int m_cache_block_size;

  const bool m_enable_log;

  void AllocateStride(const IntPtr pc, const IntPtr address);

};

#endif // __STRIDE_PREFETCHER_H
