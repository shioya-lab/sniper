#ifndef __VEC_PREFETCHER_H
#define __VEC_PREFETCHER_H

#include "core.h"
#include "prefetcher.h"

class VecPrefetcher : public Prefetcher
{
 public:
  VecPrefetcher(String configName, core_id_t core_id, UInt32 shared_cores);
  virtual std::vector<IntPtr> getNextAddress(IntPtr current_address, Core::mem_op_t mem_op_type, IntPtr pc, uint64_t uop_idx,
                                             core_id_t core_id);

 private:
  const core_id_t core_id;
  const UInt32    shared_cores;
  const String    configName;

  const size_t m_vlen;
  const size_t m_vec_stride_table_size;
  const size_t m_scalar_stride_table_size;
  const unsigned m_degree;
  const unsigned m_cache_block_size;

  // A confidence counter has hysteresis characteristics.
  static const int CONFIDENCE_INIT = 0;
  static const int CONFIDENCE_MIN  = 0;
  static const int CONFIDENCE_MAX  = 7;
  static const int CONFIDENCE_INC  = 1;
  static const int CONFIDENCE_DEC  = 4;
  static const int CONFIDENCE_PREDICTION_THREASHOLD = 7;

  struct VecStride
  {
    const unsigned m_degree;
    const unsigned m_cache_block_size;

    uint64_t last_uop_idx;

    IntPtr base_addr; // A current 'start' address of a stream
    IntPtr last_addr;
    IntPtr stride;

    IntPtr m_pc;

    int vec_size;

    int  count;     // Access count
    int  confidence;

    VecStride(const int degree, const int cache_block_size,
           const IntPtr pc, const IntPtr current_address)
        : m_degree(degree),
          m_cache_block_size (cache_block_size)
    {
      Reset(pc, current_address);
    }

    void Reset(const IntPtr pc, const IntPtr current_address)
    {
      last_uop_idx = 0;
      m_pc = pc;
      base_addr = current_address;
      last_addr = base_addr;
      count = 0;
      stride = 0;
      confidence = 0;
      vec_size = m_cache_block_size;
    }

    String ToString() const;

    bool can_prefetch () {
      return confidence >= 2;
    }

    unsigned int vec_degree () {
      return vec_size / m_cache_block_size * m_degree;
    }
  };
  typedef std::vector< VecStride *> VecStrideTable;
  VecStrideTable m_vec_stride_table;
  void AllocateVecStride(const IntPtr pc, const IntPtr address);

  struct ScalarStride
  {
    const unsigned m_degree;
    const unsigned m_cache_block_size;

    IntPtr base_addr; // A current 'start' address of a stream
    IntPtr last_addr;
    IntPtr stride;

    IntPtr m_pc;

    int  count;     // Access count
    int  confidence;

    ScalarStride(const int degree, const int cache_block_size,
           const IntPtr pc, const IntPtr current_address)
        : m_degree(degree),
          m_cache_block_size (cache_block_size)
    {
      Reset(pc, current_address);
    }

    void Reset(const IntPtr pc, const IntPtr current_address)
    {
      m_pc = pc;
      base_addr = current_address;
      last_addr = base_addr;
      count = 0;
      stride = 0;
      confidence = 0;
    }

    String ToString() const;

    bool can_prefetch () {
      return confidence >= CONFIDENCE_PREDICTION_THREASHOLD;
    }
  };
  typedef std::vector<ScalarStride *> ScalarStrideTable;
  ScalarStrideTable m_scalar_stride_table;
  void AllocateScalarStride(const IntPtr pc, const IntPtr address);

  std::vector<IntPtr> getVectorNextAddress(IntPtr pc, uint64_t uop_idx, IntPtr current_address);
  std::vector<IntPtr> getScalarNextAddress(IntPtr pc, IntPtr current_address);                                             

  const bool m_enable_log;
  UInt64 m_pref_target_log;
};

#endif // __VEC_PREFETCHER_H
