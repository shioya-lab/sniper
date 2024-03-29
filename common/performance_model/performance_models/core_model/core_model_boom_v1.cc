#include "core_model_boom_v1.h"
#include "interval_contention_boom_v1.h"
#include "rob_contention_boom_v1.h"
#include "dynamic_micro_op_boom_v1.h"
#include "log.h"
#include "config.hpp"
#include "simulator.h"

#include <decoder.h>
#include <riscv_meta.h>

// #include <src/asm/types.h>
// #include <src/asm/meta.h>
// #include <src/asm/codec.h>
// #include <src/asm/switch.h>
// #include <src/asm/strings.h>
// #include <src/asm/host-endian.h>
// #include <src/util/fmt.h>
// #include <src/util/util.h>

// using namespace riscv;

static unsigned int instructionLatencies[rv_op_last];
static unsigned int bypassLatencies[DynamicMicroOpBoomV1::UOP_BYPASS_SIZE];

CoreModelBoomV1::CoreModelBoomV1()
{
      // https://github.com/ucb-bar/riscv-boom/blob/master/src/main/scala/exu/execute.scala
   int dfmaLatency = 4;
   int imulLatency = 3;
   for (unsigned int i = 0 ; i < rv_op_last ; i++)
   {
     if (instrlist[i].has_fpu && instrlist[i].has_alu) {
       instructionLatencies[i] = dfmaLatency;
     } else if (instrlist[i].has_alu && instrlist[i].has_mul) {
       instructionLatencies[i] = imulLatency;
     } else if (instrlist[i].has_alu) {
       instructionLatencies[i] = 1;
     } else if (instrlist[i].is_vector && instrlist[i].has_fpu) {
       instructionLatencies[i] = dfmaLatency;
     } else {
       instructionLatencies[i] = 1;
     }
   }

   m_lll_cutoff = Sim()->getCfg()->getInt("perf_model/core/interval_timer/lll_cutoff");
}

#include "instruction.h"

unsigned int CoreModelBoomV1::getInstructionLatency(const MicroOp *uop) const
{
   unsigned int instruction_type = (unsigned int) uop->getInstructionOpcode();
   LOG_ASSERT_ERROR(instruction_type > 0 && instruction_type < rv_op_last, "Invalid instruction type %d, %llx", instruction_type, uop->getInstruction()->getAddress());
   return instructionLatencies[instruction_type];
}

unsigned int CoreModelBoomV1::getAluLatency(const MicroOp *uop) const
{
  switch(uop->getInstructionOpcode()) {
    case rv_op_div:
    case rv_op_divu:
    case rv_op_rem:
    case rv_op_remu:
    case rv_op_divw:
    case rv_op_divuw:
    case rv_op_remw:
    case rv_op_remuw:
    case rv_op_divd:
    case rv_op_divud:
    case rv_op_remd:
    case rv_op_remud:
    case rv_op_fdiv_s:
    case rv_op_fdiv_d:
    case rv_op_fdiv_q:
	case rv_op_vdivu_vv:
	case rv_op_vdiv_vv:
	case rv_op_vremu_vv:
	case rv_op_vrem_vv:
	case rv_op_vdivu_vx:
	case rv_op_vdiv_vx:
	case rv_op_vremu_vx:
	case rv_op_vrem_vx:
      return 32;     // TODO: Latency of div operations need to be more accurately determined
	case rv_op_vfdiv_vv  :
	case rv_op_vfdiv_vf  :
	case rv_op_vfrdiv_vf :
	case rv_op_vfsqrt_v  :
      return 12;
    default:
      return getInstructionLatency(uop);
      // LOG_PRINT_ERROR("Don't know the ALU latency for this MicroOp.");
   }
}

unsigned int CoreModelBoomV1::getBypassLatency(const DynamicMicroOp *uop) const
{
   const DynamicMicroOpBoomV1 *info = uop->getCoreSpecificInfo<DynamicMicroOpBoomV1>();
   DynamicMicroOpBoomV1::uop_bypass_t bypass_type = info->getBypassType();
   LOG_ASSERT_ERROR(bypass_type >=0 && bypass_type < DynamicMicroOpBoomV1::UOP_BYPASS_SIZE, "Invalid bypass type %d", bypass_type);
   return bypassLatencies[bypass_type];
}

unsigned int CoreModelBoomV1::getLongestLatency() const
{
   return 60;
}

IntervalContention* CoreModelBoomV1::createIntervalContentionModel(const Core *core) const
{
   return new IntervalContentionBoomV1(core, this);
}

RobContention* CoreModelBoomV1::createRobContentionModel(const Core *core) const
{
   return new RobContentionBoomV1(core, this);
}

DynamicMicroOp* CoreModelBoomV1::createDynamicMicroOp(Allocator *alloc, const MicroOp *uop, ComponentPeriod period) const
{
   DynamicMicroOpBoomV1 *info = DynamicMicroOp::alloc<DynamicMicroOpBoomV1>(alloc, uop, this, period);
   info->uop_port = DynamicMicroOpBoomV1::getPort(uop);
   info->uop_bypass = DynamicMicroOpBoomV1::getBypassType(uop);
   info->uop_alu = DynamicMicroOpBoomV1::getAlu(uop);
   return info;
}
