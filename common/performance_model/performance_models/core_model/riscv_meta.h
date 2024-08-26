#ifndef __RISCV_META_H
#define __RISCV_META_H

#include "rv_op.h"

struct riscvinstr {
  unsigned int opcode;
  bool has_alu;
  bool has_mul;
  bool has_div;
  bool has_fpu;
  bool has_fdiv;
  bool has_ifpu;
  bool is_memory;
  bool is_vector;
};

#include "rv_instrlist.h"

#endif // __RISCV_META_H
