#ifndef __DYNAMIC_MICRO_OP_BOOM_V1_H
#define __DYNAMIC_MICRO_OP_BOOM_V1_H

#include "dynamic_micro_op.h"

class MicroOp;

class DynamicMicroOpBoomV1 : public DynamicMicroOp
{
   public:
      enum uop_port_t {
         UOP_PORT0,  // FPU, FMUL, MUL
         UOP_PORT1,  // DIV
         UOP_PORT2,  // Memory
         UOP_PORT3,  // Vector Memory
         UOP_PORT4,  // Vector Arithmetic
         UOP_PORT012,
         UOP_PORT_SIZE,
      };
      uop_port_t uop_port;

      enum uop_alu_t {
         UOP_ALU_NONE = 0,
         UOP_ALU_TRIG,
         UOP_ALU_SIZE,
      };
      uop_alu_t uop_alu;

      enum uop_vecalu_t {
         UOP_VECALU_NONE = 0,
         UOP_VECALU_TRIG,
         UOP_VECALU_SIZE,
      };
      uop_vecalu_t uop_vecalu;

      enum uop_vecmem_t {
         UOP_VECMEM_NONE = 0,
         UOP_VECMEM_TRIG,
         UOP_VECMEM_SIZE,
      };
      uop_vecmem_t uop_vecmem;

      enum uop_bypass_t {
         UOP_BYPASS_NONE,
         UOP_BYPASS_LOAD_FP,
         UOP_BYPASS_FP_STORE,
         UOP_BYPASS_SIZE
      };
      uop_bypass_t uop_bypass;

      static uop_port_t getPort(const MicroOp *uop);
      static uop_bypass_t getBypassType(const MicroOp *uop);
      static uop_alu_t getAlu(const MicroOp *uop);

      virtual const char* getType() const;

      DynamicMicroOpBoomV1(const MicroOp *uop, const CoreModel *core_model, ComponentPeriod period);

      uop_port_t getPort(void) const { return uop_port; }
      uop_bypass_t getBypassType(void) const { return uop_bypass; }
      uop_alu_t getAlu(void) const { return uop_alu; }

      static const char * PortTypeString(DynamicMicroOpBoomV1::uop_port_t port);
};

#endif // __DYNAMIC_MICRO_OP_BOOM_V1_H
