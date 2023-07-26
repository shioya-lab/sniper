/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#include <math.h>

#include "rob_contention_boom_v1.h"
#include "core_model.h"
#include "dynamic_micro_op.h"
#include "core.h"
#include "config.hpp"
#include "simulator.h"
#include "memory_manager_base.h"

#include "config.hpp"

class Instruction;

RobContentionBoomV1::RobContentionBoomV1(const Core *core, const CoreModel *core_model)
   : m_core_model(core_model)
   , m_cache_block_mask(~(core->getMemoryManager()->getCacheBlockSize() - 1))
   , m_now(core->getDvfsDomain())
   , m_vlen(Sim()->getCfg()->getInt("general/vlen"))
   , m_dlen(Sim()->getCfg()->getInt("general/dlen"))
   , alu_used_until(DynamicMicroOpBoomV1::UOP_ALU_SIZE, SubsecondTime::Zero())
   , vecalu_used_until(DynamicMicroOpBoomV1::UOP_ALU_SIZE, SubsecondTime::Zero())
   , vecmem_used_until(DynamicMicroOpBoomV1::UOP_ALU_SIZE, SubsecondTime::Zero())
   , m_vector_issue_times_max(Sim()->getCfg()->getInt("general/vlen") / Sim()->getCfg()->getInt("general/dlen"))
{
  m_vsize = 8;
}

void RobContentionBoomV1::initCycle(SubsecondTime now)
{
   m_now.setElapsedTime(now);
   memset(ports, 0, sizeof(bool) * DynamicMicroOpBoomV1::UOP_PORT_SIZE);
   ports_generic012 = 0;
   ports_memory = 0;
   ports_vecmem = 0;
   ports_vecarith = 0;
}

bool RobContentionBoomV1::tryIssue(const DynamicMicroOp &uop)
{
   // Port contention
   // TODO: Maybe the scheduler is more intelligent and doesn't just assing the first uop in the ROB
   //       that fits a particular free port. We could give precedence to uops that have dependants, etc.
   // NOTE: mixes canIssue and doIssue, in the sense that ports* are incremented immediately.
   //       This works as long as, if we return true, this microop is indeed issued

   const DynamicMicroOpBoomV1 *core_uop_info = uop.getCoreSpecificInfo<DynamicMicroOpBoomV1>();
   DynamicMicroOpBoomV1::uop_port_t uop_port = core_uop_info->getPort();

   LOG_ASSERT_ERROR( uop_port < DynamicMicroOpBoomV1::UOP_PORT_SIZE, "Port Strange");

   if (uop_port == DynamicMicroOpBoomV1::UOP_PORT012)
   {
      if (ports_generic012 >= 3)
         return false;
      else
         ports_generic012++;
   }
   else if (uop_port == DynamicMicroOpBoomV1::UOP_PORT3) {
     // VectorMEM contension
     if (ports_vecmem == 0 && vecmem_used_until > m_now) {
       return false;
     }
     if (uop.getMicroOp()->canVecSquash()) {
       if (ports_vecmem >= 1) {
         return false;
       } else {
         ports_vecmem++;
       }
     } else {
       // Gather&Scatter
       if (ports_vecmem >= Sim()->getCfg()->getInt("general/vlen") / 64) {
         return false;
       } else {
         ports_vecmem++;
       }
     }
   }
   else if (uop_port == DynamicMicroOpBoomV1::UOP_PORT4) {
     // VectorALU contension
     if (ports_vecarith == 0 && vecalu_used_until > m_now) {
       return false;
     }
     if (ports_vecarith >= 1) {
       return false;
     } else {
       ports_vecarith++;
     }
   }
   else if (uop_port == DynamicMicroOpBoomV1::UOP_PORT2) {
    // Scalar Memory
    if (ports_memory >= 2) {
      return false;
    } else {
      ports_memory++;
    }
   } else 
   { // PORT0, PORT1 or PORT2
      if (ports[uop_port])
         return false;
      else if (ports_generic012 >= 3)
         return false;
      else
      {
         ports[uop_port] = true;
         ports_generic012++;
      }
   }

   // ALU contention
   if (DynamicMicroOpBoomV1::uop_alu_t alu = core_uop_info->getAlu())
   {
      if (alu_used_until[alu] > m_now)
         return false;
   }



   return true;
}

void RobContentionBoomV1::doIssue(DynamicMicroOp &uop)
{
  const DynamicMicroOpBoomV1 *core_uop_info = uop.getCoreSpecificInfo<DynamicMicroOpBoomV1>();
  DynamicMicroOpBoomV1::uop_alu_t alu = core_uop_info->getAlu();
  if (alu)
    alu_used_until[alu] = m_now + m_core_model->getAluLatency(uop.getMicroOp());

  if (uop.getMicroOp()->isVector() && (uop.getMicroOp()->isLoad() || uop.getMicroOp()->isStore())) {
    if (uop.getMicroOp()->canVecSquash()) {
      UInt64 access_times = uop.getMicroOp()->getMemoryAccessSize() * (uop.getNumMergedInst() + 1) / m_dlen;

      // printf("%ld : pc=%08x, address = %08x, size = %ld, access_times = %ld\n", uop.getSequenceNumber(), uop.getMicroOp()->getInstructionPointer().address,
      //           uop.getAddress().address,
      //           uop.getMicroOp()->getMemoryAccessSize(), access_times);

      vecmem_used_until = m_now + access_times;
     } else {
      vecmem_used_until = m_now;
     }
  }

  IntPtr uop_pc = uop.getMicroOp()->getInstructionPointer().address;
  if (m_uop_prev_pc != uop_pc) {
    m_working_vl = m_vl;
  }
  if (uop.getMicroOp()->isVector() && !(uop.getMicroOp()->isLoad() || uop.getMicroOp()->isStore())) {
    UInt64 vecalu_latency = m_vlen / m_vsize < m_working_vl ? m_vector_issue_times_max : std::max((int)(m_working_vl * m_vsize / m_dlen), 1);
    vecalu_used_until = m_now + vecalu_latency;
    // std::cout << std::hex << uop_pc << " : m_working_vl = " << std::dec << m_working_vl << ", m_vsize = " << std::dec << m_vsize <<
    //     ", m_dlen = " << m_dlen << 
    //     ", set vecalu_used_until as " << vecalu_latency << '\n';

    m_working_vl = m_working_vl - m_vlen / m_vsize;
  }
  m_uop_prev_pc = uop_pc;
}

bool RobContentionBoomV1::noMore()
{
   // When we issued something to all ports in this cycle, stop walking the rest of the ROB
   if (ports[DynamicMicroOpBoomV1::UOP_PORT2] && ports_generic012 >= 3)
      return true;
   else
      return false;
}
