/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#ifndef __ROB_CONTENTION_BOOM_V1_H
#define __ROB_CONTENTION_BOOM_V1_H

#include "rob_contention.h"
#include "contention_model.h"
#include "core_model_boom_v1.h"
#include "dynamic_micro_op_boom_v1.h"

#include <vector>

class RobContentionBoomV1 : public RobContention {
   private:
      const CoreModel *m_core_model;
      uint64_t m_cache_block_mask;
      ComponentTime m_now;

      // port contention
      bool ports[DynamicMicroOpBoomV1::UOP_PORT_SIZE];
      int ports_vecmem;
      int ports_vecarith;
      int ports_memory;
      int ports_generic012;

      size_t m_vlen;
      size_t m_dlen;
      size_t m_vl;
      size_t m_vsize;
      size_t m_vlmul;

      size_t m_working_vl;
      IntPtr m_uop_prev_pc;

      std::vector<SubsecondTime> alu_used_until;
      SubsecondTime vecalu_used_until;
      SubsecondTime vecmem_used_until;
      const uint8_t m_vector_issue_times_max;

   public:
      RobContentionBoomV1(const Core *core, const CoreModel *core_model);

      void initCycle(SubsecondTime now);
      bool tryIssue(const DynamicMicroOp &uop);
      bool noMore();
      void doIssue(DynamicMicroOp &uop);

      void setvl(size_t vl) {
         m_vl = vl;
      }
      void setvtype(size_t vsize, size_t vlmul) {
         m_vsize = vsize;
         m_vlmul = vlmul;
      }

      SubsecondTime get_vecmem_used_until () { return vecmem_used_until; }

};

#endif // __ROB_CONTENTION_BOOM_V1_H
