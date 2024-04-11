/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#ifndef __ROB_CONTENTION_NEHALEM_H
#define __ROB_CONTENTION_NEHALEM_H

#include "rob_contention.h"
#include "contention_model.h"
#include "core_model_nehalem.h"
#include "dynamic_micro_op_nehalem.h"

#include <vector>

class RobContentionNehalem : public RobContention {
   private:
      const CoreModel *m_core_model;
      uint64_t m_cache_block_mask;
      ComponentTime m_now;

      // port contention
      bool ports[DynamicMicroOpNehalem::UOP_PORT_SIZE];
      int ports_generic, ports_generic05;

      std::vector<SubsecondTime> alu_used_until;

   public:
      RobContentionNehalem(const Core *core, const CoreModel *core_model);

      void initCycle(SubsecondTime now);
      bool tryIssue(const DynamicMicroOp &uop);
      bool tryPreload () { return false; }
      bool noMore();
      void doIssue(DynamicMicroOp &uop);

      void setvl(size_t vl) { }
      void setvtype(size_t vsize, size_t vlmul) { }
      SubsecondTime get_vecmem_used_until () { return SubsecondTime::Zero(); }
};

#endif // __ROB_CONTENTION_NEHALEM_H
