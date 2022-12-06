#include "riscv_decoder.h"
#include <iostream>
#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <cstdarg>
#include <alloca.h>
// Instead of linking to the rv8 binaries, compile the data directly into this object
#include "asm/meta.cc"
#include "asm/format.cc"
#include "asm/strings.cc"

using namespace riscv;

namespace dl
{

const char* reg_name_sym[] = {
      "zero",
      "ra",
      "sp",
      "gp",
      "tp",
      "t0",
      "t1",
      "t2",
      "s0",
      "s1",
      "a0",
      "a1",
      "a2",
      "a3",
      "a4",
      "a5",
      "a6",
      "a7",
      "s2",
      "s3",
      "s4",
      "s5",
      "s6",
      "s7",
      "s8",
      "s9",
      "s10",
      "s11",
      "t3",
      "t4",
      "t5",
      "t6",
      "ft0",
      "ft1",
      "ft2",
      "ft3",
      "ft4",
      "ft5",
      "ft6",
      "ft7",
      "fs0",
      "fs1",
      "fa0",
      "fa1",
      "fa2",
      "fa3",
      "fa4",
      "fa5",
      "fa6",
      "fa7",
      "fs2",
      "fs3",
      "fs4",
      "fs5",
      "fs6",
      "fs7",
      "fs8",
      "fs9",
      "fs10",
      "fs11",
      "ft8",
      "ft9",
      "ft10",
      "ft11",
      "v0",
      "v1",
      "v2",
      "v3",
      "v4",
      "v5",
      "v6",
      "v7",
      "v8",
      "v9",
      "v10",
      "v11",
      "v12",
      "v13",
      "v14",
      "v15",
      "v16",
      "v17",
      "v18",
      "v19",
      "v20",
      "v21",
      "v22",
      "v23",
      "v24",
      "v25",
      "v26",
      "v27",
      "v28",
      "v29",
      "v30",
      "v31",
      nullptr
    };

RISCVDecoder::RISCVDecoder(dl_arch arch, dl_mode mode, dl_syntax syntax)
{
  this->m_arch = arch;
  this->m_mode = mode;
  this->m_syntax = syntax;
  this->m_isa = DL_ISA_RISCV;
}

RISCVDecoder::~RISCVDecoder()
{
}

void RISCVDecoder::decode(DecodedInst * inst)
{
  riscv::decode dec;

  if(inst->get_already_decoded())
    return;

  riscv::inst_t r_inst;
  memcpy(&r_inst, inst->get_code(), 8);  // TODO: num_bytes from sift

  riscv::decode_inst_rv64(dec, r_inst);
  decode_inst_type(dec, r_inst);
  decode_pseudo_inst(dec);

  ((RISCVDecodedInst *)inst)->set_rv8_dec(dec);

  //printf("inst: (%016llx) Size: %d Opcode: %d\n", r_inst, riscv::inst_length(r_inst), dec.op); #DEBUG

  inst->set_already_decoded(true);
}

void RISCVDecoder::decode(DecodedInst * inst, dl_isa isa)
{
  this->decode(inst);
  ((RISCVDecodedInst *)inst)->set_disassembly();
}

/// Change the ISA mode to new_mode
// This function has no real effect for XED and RISCV, because the initialization is already done
void RISCVDecoder::change_isa_mode(dl_isa new_isa)
{
  this->m_isa = new_isa;
}

/// Get the instruction name from the numerical (enum) instruction Id
const char* RISCVDecoder::inst_name(unsigned int inst_id)
{
  return rv_inst_name_sym[inst_id];
}

/// Get the register name from the numerical (enum) register Id
const char* RISCVDecoder::reg_name(unsigned int reg_id)
{
  return reg_name_sym[reg_id];
}

/// Get the largest enclosing register; applies to x86 only (AX/EAX/RAX); ARM and RISCV just returns r;
Decoder::decoder_reg RISCVDecoder::largest_enclosing_register(Decoder::decoder_reg r)
{
  return r;
}

/// Check if this register is invalid
bool RISCVDecoder::invalid_register(decoder_reg r)
{
  bool res = false;
  if (r < reg_set_size && reg_name_sym[r] == NULL)
    return true;
  return res;
}

/// Check if this register holds the program counter
bool RISCVDecoder::reg_is_program_counter(decoder_reg r)
{
  return false;
}

/// True if instruction belongs to instruction group/category
bool RISCVDecoder::inst_in_group(const DecodedInst * inst, unsigned int group_id)
{
  // meta/enums - ignoring for now
  return true;
}

/// Get the number of operands of any type for the specified instruction
unsigned int RISCVDecoder::num_operands(const DecodedInst * inst)
{
  unsigned int num_operands = 0;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const rv_operand_data *operand_data = rv_inst_operand_data[dec->op];
  while (operand_data->type == rv_type_ireg ||
         operand_data->type == rv_type_freg ||
         operand_data->type == rv_type_vreg) {
    num_operands++;
    operand_data++;
  }
  return num_operands;
}
/// Get the number of memory operands of the specified instruction
unsigned int RISCVDecoder::num_memory_operands(const DecodedInst * inst)
{
  unsigned int num_memory_operands = 0;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const char *format = rv_inst_format[dec->op];

  static int vec_lmul = 1;
  static int vec_vsew = 8;

  if (format == rv_fmt_rd_offset_rs1  /* lb, lh, lw, lbu, lhu, lwu, ld, ldu, lq, c.lwsp, c.ld, c.ldsp, c.lq, c.lqsp */
    || format == rv_fmt_frd_offset_rs1 /* flw, fld, flq, c.fld, c.flw, c.fldsp, c.flwsp */
    || format == rv_fmt_rs2_offset_rs1  /* sb, sh, sw, sd, sq, c.sw, c.swsp, c.sd, c.sdsp, c.sq, c.sqsp */
    || format == rv_fmt_frs2_offset_rs1  /* fsw, fsd, fsq, c.fsd, c.fsw, c.fsdsp, c.fswsp */
    || format == rv_fmt_aqrl_rd_rs2_rs1 /* amoswap.w */
    || format == rv_fmt_aqrl_rd_rs2_rs1 /* amoswap.d */
    || format == rv_fmt_aqrl_rd_rs2_rs1 /* amoswap.q */) {
     num_memory_operands++;
  } else if (dec->op == rv_op_vle8_v ||
             dec->op == rv_op_vse8_v ||
             dec->op == rv_op_vlse8_v ||
             dec->op == rv_op_vsse8_v ||
             dec->op == rv_op_vluxei8_v ||
             dec->op == rv_op_vloxei8_v ||
             dec->op == rv_op_vsuxei8_v ||
             dec->op == rv_op_vl1re8_v ||
             dec->op == rv_op_vl2re8_v ||
             dec->op == rv_op_vl4re8_v ||
             dec->op == rv_op_vl8re8_v ||
             dec->op == rv_op_vs1re8_v ||
             dec->op == rv_op_vs2re8_v ||
             dec->op == rv_op_vs4re8_v ||
             dec->op == rv_op_vs8re8_v ||
             dec->op == rv_op_vleff8_v) {
    // num_memory_operands = 16 * vec_lmul;
    num_memory_operands = 16;
  } else if (dec->op == rv_op_vle16_v ||
             dec->op == rv_op_vse16_v ||
             dec->op == rv_op_vlse16_v ||
             dec->op == rv_op_vsse16_v ||
             dec->op == rv_op_vluxei16_v ||
             dec->op == rv_op_vloxei16_v ||
             dec->op == rv_op_vsuxei16_v ||
             dec->op == rv_op_vl1re16_v ||
             dec->op == rv_op_vl2re16_v ||
             dec->op == rv_op_vl4re16_v ||
             dec->op == rv_op_vl8re16_v ||
             dec->op == rv_op_vs1re16_v ||
             dec->op == rv_op_vs2re16_v ||
             dec->op == rv_op_vs4re16_v ||
             dec->op == rv_op_vs8re16_v ||
             dec->op == rv_op_vleff16_v) {
    // num_memory_operands = 8 * vec_lmul;
    num_memory_operands = 8;
  } else if (dec->op == rv_op_vle32_v ||
             dec->op == rv_op_vse32_v ||
             dec->op == rv_op_vlse32_v ||
             dec->op == rv_op_vsse32_v ||
             dec->op == rv_op_vleff32_v ||
             dec->op == rv_op_vluxei32_v ||
             dec->op == rv_op_vloxei32_v ||
             dec->op == rv_op_vsuxei32_v ||
             dec->op == rv_op_vl1re32_v ||
             dec->op == rv_op_vl2re32_v ||
             dec->op == rv_op_vl4re32_v ||
             dec->op == rv_op_vl8re32_v ||
             dec->op == rv_op_vs1re32_v ||
             dec->op == rv_op_vs2re32_v ||
             dec->op == rv_op_vs4re32_v ||
             dec->op == rv_op_vs8re32_v ||
             dec->op == rv_op_vlseg4e32_v ||
             dec->op == rv_op_vlseg8e32_v ||
             dec->op == rv_op_vsseg4e32_v ||
             dec->op == rv_op_vsseg8e32_v) {
    // num_memory_operands = 4 * vec_lmul;
    num_memory_operands = 4;
  } else if (dec->op == rv_op_vle64_v ||
             dec->op == rv_op_vse64_v ||
             dec->op == rv_op_vlse64_v ||
             dec->op == rv_op_vsse64_v ||
             dec->op == rv_op_vluxei64_v ||
             dec->op == rv_op_vloxei64_v ||
             dec->op == rv_op_vsuxei64_v ||
             dec->op == rv_op_vl1re64_v ||
             dec->op == rv_op_vl2re64_v ||
             dec->op == rv_op_vl4re64_v ||
             dec->op == rv_op_vl8re64_v ||
             dec->op == rv_op_vs1re64_v ||
             dec->op == rv_op_vs2re64_v ||
             dec->op == rv_op_vs4re64_v ||
             dec->op == rv_op_vs8re64_v ||
             dec->op == rv_op_vleff64_v) {
    // num_memory_operands = 2 * vec_lmul;
    num_memory_operands = 2;
  } else if (dec->op == rv_op_vsetvli) {
    vec_lmul = 1 << (dec->imm & 0x07);
    vec_vsew = 8 << ((dec->imm >> 3) & 0x07);

    // fprintf (stderr, "LMUL is set to %d, VSEW is set to %d\n", vec_lmul, vec_vsew);
  }

  return num_memory_operands;
}


/// Get the base register of the memory operand pointed by mem_idx
Decoder::decoder_reg RISCVDecoder::mem_base_reg (const DecodedInst * inst, unsigned int mem_idx)
{
  // assert(mem_idx == 0);
  Decoder::decoder_reg reg;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();

  // int type = rv_type_ireg;
  // int operand_name = rv_operand_name_rs1;
  reg = dec->rs1; // assumption: always rs1

  // TODO: for fp ? and compressed instructions?
  return reg;
}

bool RISCVDecoder::mem_base_upate(const DecodedInst *inst, unsigned int mem_idx)
{
  // riscv::decode *dec = ((RISCVDecodedInst *) ist)->get_rv8_dec();
  return false;
}

bool RISCVDecoder::has_index_reg(const DecodedInst *inst, unsigned int mem_idx)
{
  // riscv::decode *dec = ((RISCVDecodedInst *) ist)->get_rv8_dec();
  return false;
}


/// Get the index register of the memory operand pointed by mem_idx
Decoder::decoder_reg RISCVDecoder::mem_index_reg (const DecodedInst * inst, unsigned int mem_idx)
{
  // no index reg
  return 1;
}

/// Check if the operand mem_idx from instruction inst is read from memory
bool RISCVDecoder::op_read_mem(const DecodedInst * inst, unsigned int mem_idx)
{
  // if operation is a load, we must be reading from memory
  bool res = false;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const char *format = rv_inst_format[dec->op];
  if (format == rv_fmt_rd_offset_rs1  /* lb, lh, lw, lbu, lhu, lwu, ld, ldu, lq, c.lwsp, c.ld, c.ldsp, c.lq, c.lqsp */
    || format == rv_fmt_frd_offset_rs1 /* flw, fld, flq, c.fld, c.flw, c.fldsp, c.flwsp */ ) {
     res = true;
  } else if (format == rv_fmt_vd_rs1 ||
             format == rv_fmt_vd_rs1_rs2 ||
             format == rv_fmt_vd_rs1_vs2) {
    res = true;
  }

  return res;
}

/// Check if the operand mem_idx from instruction inst is written to memory
bool RISCVDecoder::op_write_mem(const DecodedInst * inst, unsigned int mem_idx)
{
  // if this operation is a store, we must be writing to memory
  bool res = false;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const char *format = rv_inst_format[dec->op];
  if (format == rv_fmt_rs2_offset_rs1  /* sb, sh, sw, sd, sq, c.sw, c.swsp, c.sd, c.sdsp, c.sq, c.sqsp */
    || format == rv_fmt_frs2_offset_rs1  /* fsw, fsd, fsq, c.fsd, c.fsw, c.fsdsp, c.fswsp */ ) {
     res = true;
  } else if (format == rv_fmt_vs3_rs1 ||
             format == rv_fmt_vs3_rs1_rs2 ||
             format == rv_fmt_vs3_rs1_vs2) {
    res = true;
  }
  return res;
}

/// Check if the operand idx from instruction inst reads from a register
bool RISCVDecoder::op_read_reg (const DecodedInst * inst, unsigned int idx)
{
  bool res = false;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const rv_operand_data *operand_data = rv_inst_operand_data[dec->op];
  if (operand_data[idx].operand_name == rv_operand_name_rd ||
      operand_data[idx].operand_name == rv_operand_name_frd ||
      operand_data[idx].operand_name == rv_operand_name_vd) {
    res = false;
  } else if (operand_data[idx].type == rv_type_ireg) {
    // std::cout << "op_read_reg called : operand_data[idx].operand_name = " << operand_data[idx].operand_name << ", "
    //           << "dec->rd = "  << static_cast<int>(dec->rd)  << ", "
    //           << "dec->rs1 = " << static_cast<int>(dec->rs1) << ", "
    //           << "dec->rs2 = " << static_cast<int>(dec->rs2) << ", "
    //           << "dec->rs3 = " << static_cast<int>(dec->rs3) << "\n";
    if (operand_data[idx].operand_name == rv_operand_name_rs1 && (dec->rs1 == rv_ireg_zero) ||
        operand_data[idx].operand_name == rv_operand_name_rs2 && (dec->rs2 == rv_ireg_zero) ||
        operand_data[idx].operand_name == rv_operand_name_rs3 && (dec->rs3 == rv_ireg_zero)) {
      // std::cout << "   Zero Hit\n";
      res = false;
    } else {
      // std::cout << "   NonZero Hit\n";
      res = true;
    }
  } else if (operand_data[idx].type == rv_type_freg ||
             operand_data[idx].type == rv_type_vreg) {  // what about compressed register?
    res = true;
  }
  return res;
}

/// Check if the operand idx from instruction inst writes a register
bool RISCVDecoder::op_write_reg (const DecodedInst * inst, unsigned int idx)
{
  bool res = false;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const rv_operand_data *operand_data = rv_inst_operand_data[dec->op];

  // std::cout << "op_write_reg called : operand_data[idx].operand_name = " << operand_data[idx].operand_name << ", "
  //           << "dec->rd = "  << static_cast<int>(dec->rd)  << ", "
  //           << "dec->rs1 = " << static_cast<int>(dec->rs1) << ", "
  //           << "dec->rs2 = " << static_cast<int>(dec->rs2) << ", "
  //           << "dec->rs3 = " << static_cast<int>(dec->rs3) << "\n";

  if (operand_data[idx].operand_name != rv_operand_name_rd  &&
      operand_data[idx].operand_name != rv_operand_name_frd &&
      operand_data[idx].operand_name != rv_operand_name_vd) {
    res = false;
  } else if (operand_data[idx].type == rv_type_ireg) {
    if (operand_data[idx].operand_name == rv_operand_name_rd && (dec->rd == rv_ireg_zero)) {
      res = false;
    } else {
      res = true;
    }
  } else if (operand_data[idx].type == rv_type_freg ||
             operand_data[idx].type == rv_type_vreg) {  // what about compressed register?
    res = true;
  }
  return res;
}

/// Check if the operand idx from instruction inst is involved in an address generation operation
    /// (i.e. part of LEA instruction in x86)
    /// None in ARM or RISCV
bool RISCVDecoder::is_addr_gen(const DecodedInst * inst, unsigned int idx)
{
  return false;
}

/// Check if the operand idx from instruction inst is a register
bool RISCVDecoder::op_is_reg (const DecodedInst * inst, unsigned int idx)
{
  bool res = false;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const rv_operand_data *operand_data = rv_inst_operand_data[dec->op];
  if (operand_data[idx].type == rv_type_ireg ||
      operand_data[idx].type == rv_type_freg ||
      operand_data[idx].type == rv_type_vreg) {
    res = true;
  }
  return res;
}

/// Get the register used for operand idx from instruction inst.
/// Function op_is_reg() should be called first.
Decoder::decoder_reg RISCVDecoder::get_op_reg (const DecodedInst * inst, unsigned int idx)
{
  Decoder::decoder_reg reg = 0;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  const rv_operand_data *operand_data = rv_inst_operand_data[dec->op];
  if (operand_data[idx].type == rv_type_ireg ||
      operand_data[idx].type == rv_type_freg ||
      operand_data[idx].type == rv_type_vreg) {
    switch (operand_data[idx].operand_name) {
      case rv_operand_name_rd:    reg = dec->rd;    break;
      case rv_operand_name_rs1:   reg = dec->rs1;   break;
      case rv_operand_name_rs2:   reg = dec->rs2;   break;
      case rv_operand_name_frd:   reg = dec->rd + 32 ;   break;
      case rv_operand_name_frs1:  reg = dec->rs1 + 32;   break;
      case rv_operand_name_frs2:  reg = dec->rs2 + 32;   break;
      case rv_operand_name_frs3:  reg = dec->rs3 + 32;   break;
      case rv_operand_name_vd:    reg = dec->rd  + 64;   break;
      case rv_operand_name_vs1:   reg = dec->rs1 + 64;   break;
      case rv_operand_name_vs2:   reg = dec->rs2 + 64;   break;
      case rv_operand_name_vs3:   reg = dec->rs3 + 64;   break;
      default: reg = 0;
    }
  }
  return reg;
}


/// Get the size in bytes of the memory operand pointed by mem_idx
unsigned int RISCVDecoder::size_mem_op (const DecodedInst * inst, unsigned int mem_idx)
{
  unsigned int size = 0;
  riscv::decode *dec = ((RISCVDecodedInst *)inst)->get_rv8_dec();
  switch(dec->op) {
    case rv_op_lb: 			/* Load Byte */
    case rv_op_lbu: 		/* Load Byte Unsigned */
    case rv_op_flw: 		/* FP Load (SP) */
    case rv_op_sb: 			/* Store Byte */
    case rv_op_fsw: 		/* FP Store (SP) */
    case rv_op_lr_w: 	 	/* Load Reserved Word */
    case rv_op_sc_w: 		/* Store Conditional Word */
                        size = 1;
                        break;
    case rv_op_lh: 			/* Load Half */
    case rv_op_lhu: 		/* Load Half Unsigned */
    case rv_op_sh: 			/* Store Half */
                        size = 2;
                        break;
    case rv_op_lw: 			/* Load Word */
    case rv_op_lwu: 		/* Load Word Unsigned */
    case rv_op_sw: 			/* Store Word */
                        size = 4;
                        break;
    case rv_op_ld: 			/* Load Double */
    case rv_op_fld: 		/* FP Load (DP) */
    case rv_op_sd: 			/* Store Double */
    case rv_op_fsd: 		/* FP Store (DP) */
    case rv_op_lr_d: 		/* Load Reserved Double Word */
    case rv_op_sc_d: 		/* Store Conditional Double Word */
                        size = 8;
                        break;
  }
  return size;
}

/// Get the number of execution micro operations contained in instruction 'ins'
unsigned int RISCVDecoder::get_exec_microops(const DecodedInst *ins, int numLoads, int numStores)
{
  unsigned int num_exec_uops = 1;
  riscv::decode *dec = ((RISCVDecodedInst *)ins)->get_rv8_dec();
  switch(dec->op) {
    case rv_op_lr_w:                   	/* Load Reserved Word */
    case rv_op_sc_w:                   	/* Store Conditional Word */
    case rv_op_lr_d:                   	/* Load Reserved Double Word */
    case rv_op_sc_d:                   	/* Store Conditional Double Word */
    case rv_op_lr_q:
    case rv_op_sc_q:
      num_exec_uops = 1;
      break;
    case rv_op_amoadd_w:               	/* Atomic Add Word */
    case rv_op_amoxor_w:               	/* Atomic Xor Word */
    case rv_op_amoor_w:                	/* Atomic Or Word */
    case rv_op_amoand_w:               	/* Atomic And Word */
    case rv_op_amomin_w:               	/* Atomic Minimum Word */
    case rv_op_amomax_w:               	/* Atomic Maximum Word */
    case rv_op_amominu_w:              	/* Atomic Minimum Unsigned Word */
    case rv_op_amomaxu_w:              	/* Atomic Maximum Unsigned Word */
    case rv_op_amoadd_d:               	/* Atomic Add Double Word */
    case rv_op_amoxor_d:               	/* Atomic Xor Double Word */
    case rv_op_amoor_d:                	/* Atomic Or Double Word */
    case rv_op_amoand_d:               	/* Atomic And Double Word */
    case rv_op_amomin_d:              	/* Atomic Minimum Double Word */
    case rv_op_amomax_d:              	/* Atomic Maximum Double Word */
    case rv_op_amominu_d:             	/* Atomic Minimum Unsigned Double Word */
    case rv_op_amomaxu_d:             	/* Atomic Maximum Unsigned Double Word */
    case rv_op_amoadd_q:
    case rv_op_amoxor_q:
    case rv_op_amoor_q:
    case rv_op_amoand_q:
    case rv_op_amomin_q:
    case rv_op_amomax_q:
    case rv_op_amominu_q:
    case rv_op_amomaxu_q:
      num_exec_uops = 1;
      break;
    case rv_op_amoswap_w:              	/* Atomic Swap Word */
    case rv_op_amoswap_d:              	/* Atomic Swap Double Word */
    case rv_op_amoswap_q:
      num_exec_uops = 1;
      break;
	case rv_op_lb        :                     	/* Load Byte */
	case rv_op_lh        :                     	/* Load Half */
	case rv_op_lw        :                     	/* Load Word */
    case rv_op_ld        :                     	/* Load Double */
	case rv_op_lbu       :                    	/* Load Byte Unsigned */
	case rv_op_lhu       :                    	/* Load Half Unsigned */
	case rv_op_sd        :                    	/* Store Double */
	case rv_op_sb        :                    	/* Store Byte */
	case rv_op_sh        :                    	/* Store Half */
	case rv_op_sw        :                    	/* Store Word */
	case rv_op_c_fld     :
	case rv_op_c_lw      :
	case rv_op_c_flw     :
	case rv_op_c_fsd     :
	case rv_op_c_sw      :
	case rv_op_c_fsw     :
	case rv_op_c_fldsp   :
	case rv_op_c_lwsp    :
	case rv_op_c_flwsp   :
	case rv_op_c_fsdsp   :
	case rv_op_c_swsp    :
	case rv_op_c_fswsp   :
	case rv_op_c_ld      :
	case rv_op_c_sd      :
	case rv_op_c_addiw   :
	case rv_op_c_ldsp    :
	case rv_op_c_sdsp    :
	case rv_op_c_lq      :
	case rv_op_c_sq      :
	case rv_op_c_lqsp    :
	case rv_op_c_sqsp    :
	case rv_op_vle8_v    :
	case rv_op_vse8_v    :
	case rv_op_vle16_v   :
	case rv_op_vse16_v   :
	case rv_op_vle32_v   :
	case rv_op_vse32_v   :
	case rv_op_vle64_v   :
	case rv_op_vse64_v   :
	case rv_op_vleff8_v  :
	case rv_op_vleff16_v :
	case rv_op_vleff32_v :
	case rv_op_vleff64_v :
	case rv_op_vl1re8_v  :
	case rv_op_vl1re16_v :
	case rv_op_vl1re32_v :
	case rv_op_vl1re64_v :
	case rv_op_vl2re8_v  :
	case rv_op_vl2re16_v :
	case rv_op_vl2re32_v :
	case rv_op_vl2re64_v :
	case rv_op_vl4re8_v  :
	case rv_op_vl4re16_v :
	case rv_op_vl4re32_v :
	case rv_op_vl4re64_v :
	case rv_op_vl8re8_v  :
	case rv_op_vl8re16_v :
	case rv_op_vl8re32_v :
	case rv_op_vl8re64_v :
	case rv_op_vs1re8_v  :
	case rv_op_vs1re16_v :
	case rv_op_vs1re32_v :
	case rv_op_vs1re64_v :
	case rv_op_vs2re8_v  :
	case rv_op_vs2re16_v :
	case rv_op_vs2re32_v :
	case rv_op_vs2re64_v :
	case rv_op_vs4re8_v  :
	case rv_op_vs4re16_v :
	case rv_op_vs4re32_v :
	case rv_op_vs4re64_v :
	case rv_op_vs8re8_v  :
	case rv_op_vs8re16_v :
	case rv_op_vs8re32_v :
	case rv_op_vs8re64_v :
	case rv_op_vlse8_v   :
	case rv_op_vsse8_v   :
	case rv_op_vlse16_v  :
	case rv_op_vsse16_v  :
	case rv_op_vlse32_v  :
	case rv_op_vsse32_v  :
	case rv_op_vlse64_v  :
	case rv_op_vsse64_v  :
      num_exec_uops = 0;
      break;
  }

  return num_exec_uops;
}

/// Get the maximum size of the operands of instruction inst in bits
uint16_t RISCVDecoder::get_operand_size(const DecodedInst *ins)
{
  uint16_t max_reg_size = 32;

  // TODO: if register- get sizereg (for now)


  return max_reg_size;
}

/// Check if the opcode is an instruction that performs a cache flush
bool RISCVDecoder::is_cache_flush_opcode(decoder_opcode opcd)
{
  // ARM - DC; RISCV - for privileged?
  return false;
}

/// Check if the opcode is a division instruction
bool RISCVDecoder::is_div_opcode(decoder_opcode opcd)
{
  bool res = false;
  switch(opcd) {
    case rv_op_div:
    case rv_op_divu:
    case rv_op_divw:
    case rv_op_divuw:
    case rv_op_divd:
    case rv_op_divud:
      res = true; break;
  }
  return res;
}

/// Check if the opcode is a pause instruction
bool RISCVDecoder::is_pause_opcode(decoder_opcode opcd)
{
  // None in ARM and RISCV
  return false;
}

/// Check if the opcode is a branch instruction
bool RISCVDecoder::is_branch_opcode(decoder_opcode opcd)
{
  bool res = false;
  switch(opcd) {
    case rv_op_beq:		/* Branch Equal */
    case rv_op_bne:		/* Branch Not Equal */
    case rv_op_blt:		/* Branch Less Than */
    case rv_op_bge:		/* Branch Greater than Equal */
    case rv_op_bltu:	/* Branch Less Than Unsigned */
    case rv_op_bgeu:	/* Branch Greater than Equal Unsigned */
    case rv_op_beqz:	/* Branch if = zero */
    case rv_op_bnez:	/* Branch if ≠ zero */
    case rv_op_blez:	/* Branch if ≤ zero */
    case rv_op_bgez:	/* Branch if ≥ zero */
    case rv_op_bltz:	/* Branch if < zero */
    case rv_op_bgtz:	/* Branch if > zero */
    case rv_op_ble:
    case rv_op_bleu:
    case rv_op_bgt:
    case rv_op_bgtu:
      res = true; break;
  }
  return res;
}

/// Check if the opcode is an add/sub instruction that operates in vector and FP registers
bool RISCVDecoder::is_fpvector_addsub_opcode(decoder_opcode opcd, const DecodedInst* ins)
{
  return false;
}

/// Check if the opcode is a mul/div instruction that operates in vector and FP registers
bool RISCVDecoder::is_fpvector_muldiv_opcode(decoder_opcode opcd, const DecodedInst* ins)
{
  return false;
}

/// Check if the opcode is an instruction that loads or store data on vector and FP registers
bool RISCVDecoder::is_fpvector_ldst_opcode(decoder_opcode opcd, const DecodedInst* ins)
{
  return false;
}

/// Get the value of the last register in the enumeration
Decoder::decoder_reg RISCVDecoder::last_reg()
{
  return dl::last_reg; // enum reg_num defined in riscv_decoder.h
}


RISCVDecodedInst::RISCVDecodedInst(Decoder* d, const uint8_t * code, size_t size, uint64_t address)
{
  this->m_dec = d;
  this->m_code = code;
  this->m_size = size;
  this->m_address = address;
  this->m_already_decoded = false;
}

riscv::inst_t * RISCVDecodedInst::get_rv8_inst() {
  return & rv8_instr;
}

riscv::decode * RISCVDecodedInst::get_rv8_dec() {
  return & rv8_dec;
}

void RISCVDecodedInst::set_rv8_dec(riscv::decode d) {
  rv8_dec = d;
}

/// Get the instruction numerical Id
unsigned int RISCVDecodedInst::inst_num_id() const
{
  riscv::decode dec = this->rv8_dec;
  return dec.op;
}

std::string format_str(const char* fmt, ...) //rv8 src/util/util.cc
{
    std::vector<char> buf(256);
    va_list ap;

    va_start(ap, fmt);
    int len = vsnprintf(buf.data(), buf.capacity(), fmt, ap);
    va_end(ap);

    std::string str;
    if (len >= (int)buf.capacity()) {
        buf.resize(len + 1);
        va_start(ap, fmt);
        vsnprintf(buf.data(), buf.capacity(), fmt, ap);
        va_end(ap);
    }
    str = buf.data();

    return str;
}

/// Get a string with the disassembled instruction
void RISCVDecodedInst::set_disassembly()
{
  riscv::decode dec = this->rv8_dec;
  std::string *args = new std::string;
  const char *fmt = rv_inst_format[dec.op];
  while (*fmt) {
    switch (*fmt) {
      case 'O': *args += rv_inst_name_sym[dec.op]; break;
      case '(': *args += "("; break;
      case ',': *args += ", "; break;
      case ')': *args += ")"; break;
      case '0': *args += rv_ireg_name_sym[dec.rd]; break;
      case '1': *args += rv_ireg_name_sym[dec.rs1]; break;
      case '2': *args += rv_ireg_name_sym[dec.rs2]; break;
      case '3': *args += rv_freg_name_sym[dec.rd]; break;
      case '4': *args += rv_freg_name_sym[dec.rs1]; break;
      case '5': *args += rv_freg_name_sym[dec.rs2]; break;
      case '6': *args += rv_freg_name_sym[dec.rs3]; break;
      case '7': *args += format_str("%d", dec.rs1); break;
      case '8': *args += rv_vreg_name_sym[dec.rd]; break;
      case '9': *args += rv_vreg_name_sym[dec.rs3]; break;
      case 'b': *args += rv_vreg_name_sym[dec.rs1]; break;
      case 'd': *args += rv_vreg_name_sym[dec.rs2]; break;
      case 'i': *args += format_str("%d", dec.imm); break;
      case 'o': *args += format_str("pc %c %td",
        intptr_t(dec.imm) < 0 ? '-' : '+',
        intptr_t(dec.imm) < 0 ? -intptr_t(dec.imm) : intptr_t(dec.imm)); break;
      case 'c': {
        const char * csr_name = rv_csr_name_sym[dec.imm & 0xfff];
        if (csr_name) *args += format_str("%s", csr_name);
        else *args += format_str("0x%03x", dec.imm & 0xfff);
        break;
      }
      case 'r':
        switch(dec.rm) {
          case rv_rm_rne: *args += "rne"; break;
          case rv_rm_rtz: *args += "rtz"; break;
          case rv_rm_rdn: *args += "rdn"; break;
          case rv_rm_rup: *args += "rup"; break;
          case rv_rm_rmm: *args += "rmm"; break;
          case rv_rm_dyn: *args += "dyn"; break;
          default:           *args += "inv"; break;
        }
        break;
      case 'p':
        if (dec.pred & rv_fence_i) *args += "i";
        if (dec.pred & rv_fence_o) *args += "o";
        if (dec.pred & rv_fence_r) *args += "r";
        if (dec.pred & rv_fence_w) *args += "w";
        break;
      case 's':
        if (dec.succ & rv_fence_i) *args += "i";
        if (dec.succ & rv_fence_o) *args += "o";
        if (dec.succ & rv_fence_r) *args += "r";
        if (dec.succ & rv_fence_w) *args += "w";
        break;
      case '\t': while (args->length() < 15) *args += " "; break;
      case 'A': if (dec.aq) *args += ".aq"; break;
      case 'R': if (dec.rl) *args += ".rl"; break;
      default:
        break;
    }
    fmt++;
	}

  m_disassembly = args->c_str();
}

std::string RISCVDecodedInst::disassembly_to_str() const
{
  return this->m_disassembly;
}


/// Check if this instruction is a NOP
bool RISCVDecodedInst::is_nop() const
{
  bool res = false;
  riscv::decode dec = this->rv8_dec;
  if (dec.op == rv_op_nop) {
    return true;
  }
  return res;
}

/// Check if this instruction is atomic
bool RISCVDecodedInst::is_atomic() const
{
  // meta.cc or refer meta/opcode-classes
  bool res = false;
  riscv::decode dec = this->rv8_dec;
  if (dec.codec == rv_codec_r_l || dec.codec == rv_codec_r_a) {
    return true;
  }
  return res;
}

/// Check if instruction is a prefetch
bool RISCVDecodedInst::is_prefetch() const
{
  return false;
}

/// Check if this instruction is serializing (all previous instructions must have been executed)
bool RISCVDecodedInst::is_serializing() const
{
  bool res = false;
  riscv::decode dec = this->rv8_dec;
  switch (dec.op) {
    case rv_op_vsetvli:     /* VSETVLI Instruction */
    case rv_op_vsetvl:     /* VSETVLI Instruction */
    case rv_op_vsetivli:     /* VSETVLI Instruction */
      res = true;
      break;
  }
  return res;
}

/// Check if this instruction is a conditional branch
bool RISCVDecodedInst::is_conditional_branch() const
{
  bool res = false;
  riscv::decode dec = this->rv8_dec;
  switch (dec.op) {
    case rv_op_beq:		/* Branch Equal */
    case rv_op_bne:		/* Branch Not Equal */
    case rv_op_blt:		/* Branch Less Than */
    case rv_op_bge:		/* Branch Greater than Equal */
    case rv_op_bltu:	/* Branch Less Than Unsigned */
    case rv_op_bgeu:	/* Branch Greater than Equal Unsigned */
    case rv_op_beqz:	/* Branch if = zero */
    case rv_op_bnez:	/* Branch if ≠ zero */
    case rv_op_blez:	/* Branch if ≤ zero */
    case rv_op_bgez:	/* Branch if ≥ zero */
    case rv_op_bltz:	/* Branch if < zero */
    case rv_op_bgtz:	/* Branch if > zero */
    case rv_op_ble:
    case rv_op_bleu:
    case rv_op_bgt:
    case rv_op_bgtu:
      res = true;
      break;
    default:
      res = false;
      break;
  }
  return res;
}

/// Check if this instruction is a fence/barrier-type
bool RISCVDecodedInst::is_barrier() const
{
  // if DMB, DSB, and ISB Data Memory Barrier, Data Synchronization Barrier, and Instruction Synchronization Barrier.
  bool res = false;
  riscv::decode dec = this->rv8_dec;
  switch (dec.op) {
    case rv_op_fence:		/* Fence */
    case rv_op_fence_i:		/* Fence Instruction */
    case rv_op_vsetvli:     /* VSETVLI Instruction */
    case rv_op_vsetvl:     /* VSETVLI Instruction */
    case rv_op_vsetivli:     /* VSETVLI Instruction */
      // case rv_op_vsetvli:     /* VSETVLI Instruction */
      res = true;
      break;
  }
  return res;
}

/// Check if in this instruction the result merges the source and destination.
    /// For instance: memory to XMM loads in x86.
bool RISCVDecodedInst::src_dst_merge() const
{
  return false;
}

/// Check if this instruction is from the X87 set (only applicable to Intel instructions)
bool RISCVDecodedInst::is_X87() const
{
  return false;
}

/// Check if this instruction has shift or extender modifiers (ARM)
bool RISCVDecodedInst::has_modifiers() const
{
  //based on op-count - shifter and extender INVALID
  return false;
}

/// Check if this instruction loads or stores pairs of registers using a memory address (ARM)
bool RISCVDecodedInst::is_mem_pair() const
{
  // instr like ldnp, ldpsw, stnp, stp in ARM
  // no load/store pair instructions in RISCV
  return false;
}

bool RISCVDecodedInst::is_indirect_branch() const
{
  bool res;
  riscv::decode dec = this->rv8_dec;
  switch (dec.op) {
    case rv_op_jalr:
    case rv_op_jr:
      res = true;
    default :
      res = false;
  }

  return res;
}

#include "riscv_meta.h"

bool RISCVDecodedInst::is_vector () const
{
  riscv::decode dec = this->rv8_dec;
  return instrlist[dec.op].is_vector;
}


} // namespace dl;
