#ifndef __RISCV_META_H
#define __RISCV_META_H

enum rv_op
{
	rv_op_illegal = 0,
	rv_op_lui = 1,                     	/* Load Upper Immediate */
	rv_op_auipc = 2,                   	/* Add Upper Immediate to PC */
	rv_op_jal = 3,                     	/* Jump and Link */
	rv_op_jalr = 4,                    	/* Jump and Link Register */
	rv_op_beq = 5,                     	/* Branch Equal */
	rv_op_bne = 6,                     	/* Branch Not Equal */
	rv_op_blt = 7,                     	/* Branch Less Than */
	rv_op_bge = 8,                     	/* Branch Greater than Equal */
	rv_op_bltu = 9,                    	/* Branch Less Than Unsigned */
	rv_op_bgeu = 10,                   	/* Branch Greater than Equal Unsigned */
	rv_op_lb = 11,                     	/* Load Byte */
	rv_op_lh = 12,                     	/* Load Half */
	rv_op_lw = 13,                     	/* Load Word */
	rv_op_lbu = 14,                    	/* Load Byte Unsigned */
	rv_op_lhu = 15,                    	/* Load Half Unsigned */
	rv_op_sb = 16,                     	/* Store Byte */
	rv_op_sh = 17,                     	/* Store Half */
	rv_op_sw = 18,                     	/* Store Word */
	rv_op_addi = 19,                   	/* Add Immediate */
	rv_op_slti = 20,                   	/* Set Less Than Immediate */
	rv_op_sltiu = 21,                  	/* Set Less Than Immediate Unsigned */
	rv_op_xori = 22,                   	/* Xor Immediate */
	rv_op_ori = 23,                    	/* Or Immediate */
	rv_op_andi = 24,                   	/* And Immediate */
	rv_op_slli = 25,                   	/* Shift Left Logical Immediate */
	rv_op_srli = 26,                   	/* Shift Right Logical Immediate */
	rv_op_srai = 27,                   	/* Shift Right Arithmetic Immediate */
	rv_op_add = 28,                    	/* Add */
	rv_op_sub = 29,                    	/* Subtract */
	rv_op_sll = 30,                    	/* Shift Left Logical */
	rv_op_slt = 31,                    	/* Set Less Than */
	rv_op_sltu = 32,                   	/* Set Less Than Unsigned */
	rv_op_xor = 33,                    	/* Xor */
	rv_op_srl = 34,                    	/* Shift Right Logical */
	rv_op_sra = 35,                    	/* Shift Right Arithmetic */
	rv_op_or = 36,                     	/* Or */
	rv_op_and = 37,                    	/* And */
	rv_op_fence = 38,                  	/* Fence */
	rv_op_fence_i = 39,                	/* Fence Instruction */
	rv_op_lwu = 40,                    	/* Load Word Unsigned */
	rv_op_ld = 41,                     	/* Load Double */
	rv_op_sd = 42,                     	/* Store Double */
	rv_op_addiw = 43,                  	/* Add Immediate Word */
	rv_op_slliw = 44,                  	/* Shift Left Logical Immediate Word */
	rv_op_srliw = 45,                  	/* Shift Right Logical Immediate Word */
	rv_op_sraiw = 46,                  	/* Shift Right Arithmetic Immediate Word */
	rv_op_addw = 47,                   	/* Add Word */
	rv_op_subw = 48,                   	/* Subtract Word */
	rv_op_sllw = 49,                   	/* Shift Left Logical Word */
	rv_op_srlw = 50,                   	/* Shift Right Logical Word */
	rv_op_sraw = 51,                   	/* Shift Right Arithmetic Word */
	rv_op_ldu = 52,
	rv_op_lq = 53,
	rv_op_sq = 54,
	rv_op_addid = 55,
	rv_op_sllid = 56,
	rv_op_srlid = 57,
	rv_op_sraid = 58,
	rv_op_addd = 59,
	rv_op_subd = 60,
	rv_op_slld = 61,
	rv_op_srld = 62,
	rv_op_srad = 63,
	rv_op_mul = 64,                    	/* Multiply */
	rv_op_mulh = 65,                   	/* Multiply High Signed Signed */
	rv_op_mulhsu = 66,                 	/* Multiply High Signed Unsigned */
	rv_op_mulhu = 67,                  	/* Multiply High Unsigned Unsigned */
	rv_op_div = 68,                    	/* Divide Signed */
	rv_op_divu = 69,                   	/* Divide Unsigned */
	rv_op_rem = 70,                    	/* Remainder Signed */
	rv_op_remu = 71,                   	/* Remainder Unsigned */
	rv_op_mulw = 72,                   	/* Multiple Word */
	rv_op_divw = 73,                   	/* Divide Signed Word */
	rv_op_divuw = 74,                  	/* Divide Unsigned Word */
	rv_op_remw = 75,                   	/* Remainder Signed Word */
	rv_op_remuw = 76,                  	/* Remainder Unsigned Word */
	rv_op_muld = 77,
	rv_op_divd = 78,
	rv_op_divud = 79,
	rv_op_remd = 80,
	rv_op_remud = 81,
	rv_op_lr_w = 82,                   	/* Load Reserved Word */
	rv_op_sc_w = 83,                   	/* Store Conditional Word */
	rv_op_amoswap_w = 84,              	/* Atomic Swap Word */
	rv_op_amoadd_w = 85,               	/* Atomic Add Word */
	rv_op_amoxor_w = 86,               	/* Atomic Xor Word */
	rv_op_amoor_w = 87,                	/* Atomic Or Word */
	rv_op_amoand_w = 88,               	/* Atomic And Word */
	rv_op_amomin_w = 89,               	/* Atomic Minimum Word */
	rv_op_amomax_w = 90,               	/* Atomic Maximum Word */
	rv_op_amominu_w = 91,              	/* Atomic Minimum Unsigned Word */
	rv_op_amomaxu_w = 92,              	/* Atomic Maximum Unsigned Word */
	rv_op_lr_d = 93,                   	/* Load Reserved Double Word */
	rv_op_sc_d = 94,                   	/* Store Conditional Double Word */
	rv_op_amoswap_d = 95,              	/* Atomic Swap Double Word */
	rv_op_amoadd_d = 96,               	/* Atomic Add Double Word */
	rv_op_amoxor_d = 97,               	/* Atomic Xor Double Word */
	rv_op_amoor_d = 98,                	/* Atomic Or Double Word */
	rv_op_amoand_d = 99,               	/* Atomic And Double Word */
	rv_op_amomin_d = 100,              	/* Atomic Minimum Double Word */
	rv_op_amomax_d = 101,              	/* Atomic Maximum Double Word */
	rv_op_amominu_d = 102,             	/* Atomic Minimum Unsigned Double Word */
	rv_op_amomaxu_d = 103,             	/* Atomic Maximum Unsigned Double Word */
	rv_op_lr_q = 104,
	rv_op_sc_q = 105,
	rv_op_amoswap_q = 106,
	rv_op_amoadd_q = 107,
	rv_op_amoxor_q = 108,
	rv_op_amoor_q = 109,
	rv_op_amoand_q = 110,
	rv_op_amomin_q = 111,
	rv_op_amomax_q = 112,
	rv_op_amominu_q = 113,
	rv_op_amomaxu_q = 114,
	rv_op_ecall = 115,                 	/* Environment Call */
	rv_op_ebreak = 116,                	/* Environment Break to Debugger */
	rv_op_uret = 117,                  	/* User Return */
	rv_op_sret = 118,                  	/* System Return */
	rv_op_hret = 119,                  	/* Hypervisor Return */
	rv_op_mret = 120,                  	/* Machine-Mode Return */
	rv_op_dret = 121,                  	/* Debug-Mode Return */
	rv_op_sfence_vm = 122,             	/* Supervisor Memory Management Fence */
	rv_op_sfence_vma = 123,            	/* Supervisor Memory Management Fence */
	rv_op_wfi = 124,                   	/* Wait For Interrupt */
	rv_op_csrrw = 125,                 	/* CSR Atomic Read Write */
	rv_op_csrrs = 126,                 	/* CSR Atomic Set Bit */
	rv_op_csrrc = 127,                 	/* CSR Atomic Clear Bit */
	rv_op_csrrwi = 128,                	/* CSR Atomic Read Write Immediate */
	rv_op_csrrsi = 129,                	/* CSR Atomic Set Bit Immediate */
	rv_op_csrrci = 130,                	/* CSR Atomic Clear Bit Immediate */
	rv_op_flw = 131,                   	/* FP Load (SP) */
	rv_op_fsw = 132,                   	/* FP Store (SP) */
	rv_op_fmadd_s = 133,               	/* FP Fused Multiply Add (SP) */
	rv_op_fmsub_s = 134,               	/* FP Fused Multiply Subtract (SP) */
	rv_op_fnmsub_s = 135,              	/* FP Negate fused Multiply Subtract (SP) */
	rv_op_fnmadd_s = 136,              	/* FP Negate fused Multiply Add (SP) */
	rv_op_fadd_s = 137,                	/* FP Add (SP) */
	rv_op_fsub_s = 138,                	/* FP Subtract (SP) */
	rv_op_fmul_s = 139,                	/* FP Multiply (SP) */
	rv_op_fdiv_s = 140,                	/* FP Divide (SP) */
	rv_op_fsgnj_s = 141,               	/* FP Sign-injection (SP) */
	rv_op_fsgnjn_s = 142,              	/* FP Sign-injection Negate (SP) */
	rv_op_fsgnjx_s = 143,              	/* FP Sign-injection Xor (SP) */
	rv_op_fmin_s = 144,                	/* FP Minimum (SP) */
	rv_op_fmax_s = 145,                	/* FP Maximum (SP) */
	rv_op_fsqrt_s = 146,               	/* FP Square Root (SP) */
	rv_op_fle_s = 147,                 	/* FP Less Than Equal (SP) */
	rv_op_flt_s = 148,                 	/* FP Less Than (SP) */
	rv_op_feq_s = 149,                 	/* FP Equal (SP) */
	rv_op_fcvt_w_s = 150,              	/* FP Convert Float to Word (SP) */
	rv_op_fcvt_wu_s = 151,             	/* FP Convert Float to Word Unsigned (SP) */
	rv_op_fcvt_s_w = 152,              	/* FP Convert Word to Float (SP) */
	rv_op_fcvt_s_wu = 153,             	/* FP Convert Word Unsigned to Float (SP) */
	rv_op_fmv_x_s = 154,               	/* FP Move to Integer Register (SP) */
	rv_op_fclass_s = 155,              	/* FP Classify (SP) */
	rv_op_fmv_s_x = 156,               	/* FP Move from Integer Register (SP) */
	rv_op_fcvt_l_s = 157,              	/* FP Convert Float to Double Word (SP) */
	rv_op_fcvt_lu_s = 158,             	/* FP Convert Float to Double Word Unsigned (SP) */
	rv_op_fcvt_s_l = 159,              	/* FP Convert Double Word to Float (SP) */
	rv_op_fcvt_s_lu = 160,             	/* FP Convert Double Word Unsigned to Float (SP) */
	rv_op_fld = 161,                   	/* FP Load (DP) */
	rv_op_fsd = 162,                   	/* FP Store (DP) */
	rv_op_fmadd_d = 163,               	/* FP Fused Multiply Add (DP) */
	rv_op_fmsub_d = 164,               	/* FP Fused Multiply Subtract (DP) */
	rv_op_fnmsub_d = 165,              	/* FP Negate fused Multiply Subtract (DP) */
	rv_op_fnmadd_d = 166,              	/* FP Negate fused Multiply Add (DP) */
	rv_op_fadd_d = 167,                	/* FP Add (DP) */
	rv_op_fsub_d = 168,                	/* FP Subtract (DP) */
	rv_op_fmul_d = 169,                	/* FP Multiply (DP) */
	rv_op_fdiv_d = 170,                	/* FP Divide (DP) */
	rv_op_fsgnj_d = 171,               	/* FP to Sign-injection (DP) */
	rv_op_fsgnjn_d = 172,              	/* FP to Sign-injection Negate (DP) */
	rv_op_fsgnjx_d = 173,              	/* FP to Sign-injection Xor (DP) */
	rv_op_fmin_d = 174,                	/* FP Minimum (DP) */
	rv_op_fmax_d = 175,                	/* FP Maximum (DP) */
	rv_op_fcvt_s_d = 176,              	/* FP Convert DP to SP */
	rv_op_fcvt_d_s = 177,              	/* FP Convert SP to DP */
	rv_op_fsqrt_d = 178,               	/* Floating Square Root (DP) */
	rv_op_fle_d = 179,                 	/* FP Less Than Equal (DP) */
	rv_op_flt_d = 180,                 	/* FP Less Than (DP) */
	rv_op_feq_d = 181,                 	/* FP Equal (DP) */
	rv_op_fcvt_w_d = 182,              	/* FP Convert Float to Word (DP) */
	rv_op_fcvt_wu_d = 183,             	/* FP Convert Float to Word Unsigned (DP) */
	rv_op_fcvt_d_w = 184,              	/* FP Convert Word to Float (DP) */
	rv_op_fcvt_d_wu = 185,             	/* FP Convert Word Unsigned to Float (DP) */
	rv_op_fclass_d = 186,              	/* FP Classify (DP) */
	rv_op_fcvt_l_d = 187,              	/* FP Convert Float to Double Word (DP) */
	rv_op_fcvt_lu_d = 188,             	/* FP Convert Float to Double Word Unsigned (DP) */
	rv_op_fmv_x_d = 189,               	/* FP Move to Integer Register (DP) */
	rv_op_fcvt_d_l = 190,              	/* FP Convert Double Word to Float (DP) */
	rv_op_fcvt_d_lu = 191,             	/* FP Convert Double Word Unsigned Float (DP) */
	rv_op_fmv_d_x = 192,               	/* FP Move from Integer Register (DP) */
	rv_op_flq = 193,                   	/* FP Load (QP) */
	rv_op_fsq = 194,                   	/* FP Store (QP) */
	rv_op_fmadd_q = 195,               	/* FP Fused Multiply Add (QP) */
	rv_op_fmsub_q = 196,               	/* FP Fused Multiply Subtract (QP) */
	rv_op_fnmsub_q = 197,              	/* FP Negate fused Multiply Subtract (QP) */
	rv_op_fnmadd_q = 198,              	/* FP Negate fused Multiply Add (QP) */
	rv_op_fadd_q = 199,                	/* FP Add (QP) */
	rv_op_fsub_q = 200,                	/* FP Subtract (QP) */
	rv_op_fmul_q = 201,                	/* FP Multiply (QP) */
	rv_op_fdiv_q = 202,                	/* FP Divide (QP) */
	rv_op_fsgnj_q = 203,               	/* FP to Sign-injection (QP) */
	rv_op_fsgnjn_q = 204,              	/* FP to Sign-injection Negate (QP) */
	rv_op_fsgnjx_q = 205,              	/* FP to Sign-injection Xor (QP) */
	rv_op_fmin_q = 206,                	/* FP Minimum (QP) */
	rv_op_fmax_q = 207,                	/* FP Maximum (QP) */
	rv_op_fcvt_s_q = 208,              	/* FP Convert QP to SP */
	rv_op_fcvt_q_s = 209,              	/* FP Convert SP to QP */
	rv_op_fcvt_d_q = 210,              	/* FP Convert QP to DP */
	rv_op_fcvt_q_d = 211,              	/* FP Convert DP to QP */
	rv_op_fsqrt_q = 212,               	/* Floating Square Root (QP) */
	rv_op_fle_q = 213,                 	/* FP Less Than Equal (QP) */
	rv_op_flt_q = 214,                 	/* FP Less Than (QP) */
	rv_op_feq_q = 215,                 	/* FP Equal (QP) */
	rv_op_fcvt_w_q = 216,              	/* FP Convert Float to Word (QP) */
	rv_op_fcvt_wu_q = 217,             	/* FP Convert Float to Word Unsigned (QP) */
	rv_op_fcvt_q_w = 218,              	/* FP Convert Word to Float (QP) */
	rv_op_fcvt_q_wu = 219,             	/* FP Convert Word Unsigned to Float (QP) */
	rv_op_fclass_q = 220,              	/* FP Classify (QP) */
	rv_op_fcvt_l_q = 221,              	/* FP Convert Float to Double Word (QP) */
	rv_op_fcvt_lu_q = 222,             	/* FP Convert Float to Double Word Unsigned (QP) */
	rv_op_fcvt_q_l = 223,              	/* FP Convert Double Word to Float (QP) */
	rv_op_fcvt_q_lu = 224,             	/* FP Convert Double Word Unsigned Float (QP) */
	rv_op_fmv_x_q = 225,               	/* FP Move to Integer Register (QP) */
	rv_op_fmv_q_x = 226,               	/* FP Move from Integer Register (QP) */
	rv_op_c_addi4spn = 227,
	rv_op_c_fld = 228,
	rv_op_c_lw = 229,
	rv_op_c_flw = 230,
	rv_op_c_fsd = 231,
	rv_op_c_sw = 232,
	rv_op_c_fsw = 233,
	rv_op_c_nop = 234,
	rv_op_c_addi = 235,
	rv_op_c_jal = 236,
	rv_op_c_li = 237,
	rv_op_c_addi16sp = 238,
	rv_op_c_lui = 239,
	rv_op_c_srli = 240,
	rv_op_c_srai = 241,
	rv_op_c_andi = 242,
	rv_op_c_sub = 243,
	rv_op_c_xor = 244,
	rv_op_c_or = 245,
	rv_op_c_and = 246,
	rv_op_c_subw = 247,
	rv_op_c_addw = 248,
	rv_op_c_j = 249,
	rv_op_c_beqz = 250,
	rv_op_c_bnez = 251,
	rv_op_c_slli = 252,
	rv_op_c_fldsp = 253,
	rv_op_c_lwsp = 254,
	rv_op_c_flwsp = 255,
	rv_op_c_jr = 256,
	rv_op_c_mv = 257,
	rv_op_c_ebreak = 258,
	rv_op_c_jalr = 259,
	rv_op_c_add = 260,
	rv_op_c_fsdsp = 261,
	rv_op_c_swsp = 262,
	rv_op_c_fswsp = 263,
	rv_op_c_ld = 264,
	rv_op_c_sd = 265,
	rv_op_c_addiw = 266,
	rv_op_c_ldsp = 267,
	rv_op_c_sdsp = 268,
	rv_op_c_lq = 269,
	rv_op_c_sq = 270,
	rv_op_c_lqsp = 271,
	rv_op_c_sqsp = 272,
	rv_op_nop = 273,                   	/* No operation */
	rv_op_mv = 274,                    	/* Copy register */
	rv_op_not = 275,                   	/* One’s complement */
	rv_op_neg = 276,                   	/* Two’s complement */
	rv_op_negw = 277,                  	/* Two’s complement Word */
	rv_op_sext_w = 278,                	/* Sign extend Word */
	rv_op_seqz = 279,                  	/* Set if = zero */
	rv_op_snez = 280,                  	/* Set if ≠ zero */
	rv_op_sltz = 281,                  	/* Set if < zero */
	rv_op_sgtz = 282,                  	/* Set if > zero */
	rv_op_fmv_s = 283,                 	/* Single-precision move */
	rv_op_fabs_s = 284,                	/* Single-precision absolute value */
	rv_op_fneg_s = 285,                	/* Single-precision negate */
	rv_op_fmv_d = 286,                 	/* Double-precision move */
	rv_op_fabs_d = 287,                	/* Double-precision absolute value */
	rv_op_fneg_d = 288,                	/* Double-precision negate */
	rv_op_fmv_q = 289,                 	/* Quadruple-precision move */
	rv_op_fabs_q = 290,                	/* Quadruple-precision absolute value */
	rv_op_fneg_q = 291,                	/* Quadruple-precision negate */
	rv_op_beqz = 292,                  	/* Branch if = zero */
	rv_op_bnez = 293,                  	/* Branch if ≠ zero */
	rv_op_blez = 294,                  	/* Branch if ≤ zero */
	rv_op_bgez = 295,                  	/* Branch if ≥ zero */
	rv_op_bltz = 296,                  	/* Branch if < zero */
	rv_op_bgtz = 297,                  	/* Branch if > zero */
	rv_op_ble = 298,
	rv_op_bleu = 299,
	rv_op_bgt = 300,
	rv_op_bgtu = 301,
	rv_op_j = 302,                     	/* Jump */
	rv_op_ret = 303,                   	/* Return from subroutine */
	rv_op_jr = 304,                    	/* Jump register */
	rv_op_rdcycle = 305,               	/* Read Cycle Counter Status Register */
	rv_op_rdtime = 306,                	/* Read Timer Status register */
	rv_op_rdinstret = 307,             	/* Read Instructions Retired Status Register */
	rv_op_rdcycleh = 308,              	/* Read Cycle Counter Status Register (upper 32-bits on RV32) */
	rv_op_rdtimeh = 309,               	/* Read Timer Status register (upper 32-bits on RV32) */
	rv_op_rdinstreth = 310,            	/* Read Instructions Retired Status Register (upper 32-bits on RV32) */
	rv_op_frcsr = 311,                 	/* Read FP Control and Status Register */
	rv_op_frrm = 312,                  	/* Read FP Rounding Mode */
	rv_op_frflags = 313,               	/* Read FP Accrued Exception Flags */
	rv_op_fscsr = 314,                 	/* Set FP Control and Status Register */
	rv_op_fsrm = 315,                  	/* Set FP Rounding Mode */
	rv_op_fsflags = 316,               	/* Set FP Accrued Exception Flags */
	rv_op_fsrmi = 317,                 	/* Set FP Rounding Mode Immediate */
	rv_op_fsflagsi = 318,              	/* Set FP Accrued Exception Flags Immediate */
	rv_op_vsetvli = 319,
	rv_op_vsetivli = 320,
	rv_op_vsetvl = 321,
	rv_op_vle8_v = 322,
	rv_op_vse8_v = 323,
	rv_op_vle16_v = 324,
	rv_op_vse16_v = 325,
	rv_op_vle32_v = 326,
	rv_op_vse32_v = 327,
	rv_op_vle64_v = 328,
	rv_op_vse64_v = 329,
	rv_op_vleff8_v = 330,
	rv_op_vleff16_v = 331,
	rv_op_vleff32_v = 332,
	rv_op_vleff64_v = 333,
	rv_op_vl1re8_v = 334,
	rv_op_vl1re16_v = 335,
	rv_op_vl1re32_v = 336,
	rv_op_vl1re64_v = 337,
	rv_op_vl2re8_v = 338,
	rv_op_vl2re16_v = 339,
	rv_op_vl2re32_v = 340,
	rv_op_vl2re64_v = 341,
	rv_op_vl4re8_v = 342,
	rv_op_vl4re16_v = 343,
	rv_op_vl4re32_v = 344,
	rv_op_vl4re64_v = 345,
	rv_op_vl8re8_v = 346,
	rv_op_vl8re16_v = 347,
	rv_op_vl8re32_v = 348,
	rv_op_vl8re64_v = 349,
	rv_op_vs1re8_v = 350,
	rv_op_vs1re16_v = 351,
	rv_op_vs1re32_v = 352,
	rv_op_vs1re64_v = 353,
	rv_op_vs2re8_v = 354,
	rv_op_vs2re16_v = 355,
	rv_op_vs2re32_v = 356,
	rv_op_vs2re64_v = 357,
	rv_op_vs4re8_v = 358,
	rv_op_vs4re16_v = 359,
	rv_op_vs4re32_v = 360,
	rv_op_vs4re64_v = 361,
	rv_op_vs8re8_v = 362,
	rv_op_vs8re16_v = 363,
	rv_op_vs8re32_v = 364,
	rv_op_vs8re64_v = 365,
	rv_op_vlse8_v = 366,
	rv_op_vsse8_v = 367,
	rv_op_vlse16_v = 368,
	rv_op_vsse16_v = 369,
	rv_op_vlse32_v = 370,
	rv_op_vsse32_v = 371,
	rv_op_vlse64_v = 372,
	rv_op_vsse64_v = 373,
	rv_op_vadd_vv = 374,
	rv_op_vsub_vv = 375,
	rv_op_vminu_vv = 376,
	rv_op_vmin_vv = 377,
	rv_op_vmaxu_vv = 378,
	rv_op_vmax_vv = 379,
	rv_op_vand_vv = 380,
	rv_op_vor_vv = 381,
	rv_op_vxor_vv = 382,
	rv_op_vrgather_vv = 383,
	rv_op_vadc_vv = 384,
	rv_op_vmadc_vv = 385,
	rv_op_vsbc_vv = 386,
	rv_op_vmsbc_vv = 387,
	rv_op_vmerge_vv = 388,
	rv_op_vmseq_vv = 389,
	rv_op_vmsne_vv = 390,
	rv_op_vmsltu_vv = 391,
	rv_op_vmslt_vv = 392,
	rv_op_vmsleu_vv = 393,
	rv_op_vmsle_vv = 394,
	rv_op_vsaddu_vv = 395,
	rv_op_vsadd_vv = 396,
	rv_op_vssubu_vv = 397,
	rv_op_vssub_vv = 398,
	rv_op_vsll_vv = 399,
	rv_op_vsmul_vv = 400,
	rv_op_vsrl_vv = 401,
	rv_op_vsra_vv = 402,
	rv_op_vssrl_vv = 403,
	rv_op_vssra_vv = 404,
	rv_op_vnsrl_vv = 405,
	rv_op_vnsra_vv = 406,
	rv_op_vnclipu_vv = 407,
	rv_op_vnclip_vv = 408,
	rv_op_vwredsumu_vv = 409,
	rv_op_vwredsum_vv = 410,
	rv_op_vdotu_vv = 411,
	rv_op_vdot_vv = 412,
	rv_op_vqmaccu_vv = 413,
	rv_op_vqmacc_vv = 414,
	rv_op_vqmaccus_vv = 415,
	rv_op_vqmaccsu_vv = 416,
	rv_op_vadd_vx = 417,
	rv_op_vsub_vx = 418,
	rv_op_vrsub_vx = 419,
	rv_op_vminu_vx = 420,
	rv_op_vmin_vx = 421,
	rv_op_vmaxu_vx = 422,
	rv_op_vmax_vx = 423,
	rv_op_vand_vx = 424,
	rv_op_vor_vx = 425,
	rv_op_vxor_vx = 426,
	rv_op_vrgather_vx = 427,
	rv_op_vslideup_vx = 428,
	rv_op_vslidedown_vx = 429,
	rv_op_vadc_vx = 430,
	rv_op_vmadc_vx = 431,
	rv_op_vsbc_vx = 432,
	rv_op_vmsbc_vx = 433,
	rv_op_vmerge_vx = 434,
	rv_op_vmseq_vx = 435,
	rv_op_vmsne_vx = 436,
	rv_op_vmsltu_vx = 437,
	rv_op_vmslt_vx = 438,
	rv_op_vmsleu_vx = 439,
	rv_op_vmsle_vx = 440,
	rv_op_vmsgtu_vx = 441,
	rv_op_vmsgt_vx = 442,
	rv_op_vsaddu_vx = 443,
	rv_op_vsadd_vx = 444,
	rv_op_vssubu_vx = 445,
	rv_op_vssub_vx = 446,
	rv_op_vsll_vx = 447,
	rv_op_vsmul_vx = 448,
	rv_op_vsrl_vx = 449,
	rv_op_vsra_vx = 450,
	rv_op_vssrl_vx = 451,
	rv_op_vssra_vx = 452,
	rv_op_vnsrl_vx = 453,
	rv_op_vnsra_vx = 454,
	rv_op_vnclipu_vx = 455,
	rv_op_vnclip_vx = 456,
	rv_op_vwredsumu_vx = 457,
	rv_op_vwredsum_vx = 458,
	rv_op_vdotu_vx = 459,
	rv_op_vdot_vx = 460,
	rv_op_vqmaccu_vx = 461,
	rv_op_vqmacc_vx = 462,
	rv_op_vqmaccus_vx = 463,
	rv_op_vqmaccsu_vx = 464,
	rv_op_vadd_vi = 465,
	rv_op_vrsub_vi = 466,
	rv_op_vand_vi = 467,
	rv_op_vor_vi = 468,
	rv_op_vxor_vi = 469,
	rv_op_vrgather_vi = 470,
	rv_op_vslideup_vi = 471,
	rv_op_vslidedown_vi = 472,
	rv_op_vadc_vi = 473,
	rv_op_vmadc_vi = 474,
	rv_op_vmv_vi = 475,
	rv_op_vmseq_vi = 476,
	rv_op_vmsne_vi = 477,
	rv_op_vmsleu_vi = 478,
	rv_op_vmsle_vi = 479,
	rv_op_vmsgtu_vi = 480,
	rv_op_vmsgt_vi = 481,
	rv_op_vmv1r = 482,
	rv_op_vmv2r = 483,
	rv_op_vmv4r = 484,
	rv_op_vmv8r = 485,
	rv_op_vsaddu_vi = 486,
	rv_op_vsadd_vi = 487,
	rv_op_vsll_vi = 488,
	rv_op_vsrl_vi = 489,
	rv_op_vsra_vi = 490,
	rv_op_vssrl_vi = 491,
	rv_op_vssra_vi = 492,
	rv_op_vnsrl_vi = 493,
	rv_op_vnsra_vi = 494,
	rv_op_vnclipu_vi = 495,
	rv_op_vnclip_vi = 496,
	rv_op_vredsum_vv = 497,
	rv_op_vredand_vv = 498,
	rv_op_vredor_vv = 499,
	rv_op_vredxor_vv = 500,
	rv_op_vredminu_vv = 501,
	rv_op_vredmin_vv = 502,
	rv_op_vredmaxu_vv = 503,
	rv_op_vredmax_vv = 504,
	rv_op_vaaddu_vv = 505,
	rv_op_vaadd_vv = 506,
	rv_op_vasubu_vv = 507,
	rv_op_vasub_vv = 508,
	rv_op_vmv_x_s = 509,
	rv_op_vpopc_m = 510,
	rv_op_vfirst_m = 511,
	rv_op_vmv_s_x = 512,
	rv_op_vzext_vf8 = 513,
	rv_op_vsext_vf8 = 514,
	rv_op_vzext_vf4 = 515,
	rv_op_vsext_vf4 = 516,
	rv_op_vzext_vf2 = 517,
	rv_op_vsext_vf2 = 518,
	rv_op_vmsbf_m = 519,
	rv_op_vmsof_m = 520,
	rv_op_vmsif_m = 521,
	rv_op_viota_m = 522,
	rv_op_vid_v = 523,
	rv_op_vcompress_vv = 524,
	rv_op_vmandnot_vv = 525,
	rv_op_vmand_vv = 526,
	rv_op_vmor_vv = 527,
	rv_op_vmxor_vv = 528,
	rv_op_vmornot_vv = 529,
	rv_op_vmnand_vv = 530,
	rv_op_vmnor_vv = 531,
	rv_op_vmxnor_vv = 532,
	rv_op_vdivu_vv = 533,
	rv_op_vdiv_vv = 534,
	rv_op_vremu_vv = 535,
	rv_op_vrem_vv = 536,
	rv_op_vmulhu_vv = 537,
	rv_op_vmul_vv = 538,
	rv_op_vmulhsu_vv = 539,
	rv_op_vmulh_vv = 540,
	rv_op_vmadd_vv = 541,
	rv_op_vnmsub_vv = 542,
	rv_op_vmacc_vv = 543,
	rv_op_vnmsac_vv = 544,
	rv_op_vwaddu_vv = 545,
	rv_op_vwadd_vv = 546,
	rv_op_vwsubu_vv = 547,
	rv_op_vwsub_vv = 548,
	rv_op_vwaddu_w_vv = 549,
	rv_op_vwadd_w_vv = 550,
	rv_op_vwsubu_w_vv = 551,
	rv_op_vwsub_w_vv = 552,
	rv_op_vwmulu_vv = 553,
	rv_op_vwmulsu_vv = 554,
	rv_op_vwmul_vv = 555,
	rv_op_vwmaccu_vv = 556,
	rv_op_vwmacc_vv = 557,
	rv_op_vwmaccus_vv = 558,
	rv_op_vwmaccsu_vv = 559,
	rv_op_vaaddu_vx = 560,
	rv_op_vaadd_vx = 561,
	rv_op_vasubu_vx = 562,
	rv_op_vasub_vx = 563,
	rv_op_vslide1up_vx = 564,
	rv_op_vslide1down_vx = 565,
	rv_op_vdivu_vx = 566,
	rv_op_vdiv_vx = 567,
	rv_op_vremu_vx = 568,
	rv_op_vrem_vx = 569,
	rv_op_vmulhu_vx = 570,
	rv_op_vmul_vx = 571,
	rv_op_vmulhsu_vx = 572,
	rv_op_vmulh_vx = 573,
	rv_op_vmadd_vx = 574,
	rv_op_vnmsub_vx = 575,
	rv_op_vmacc_vx = 576,
	rv_op_vnmsac_vx = 577,
	rv_op_vwaddu_vx = 578,
	rv_op_vwadd_vx = 579,
	rv_op_vwsubu_vx = 580,
	rv_op_vwsub_vx = 581,
	rv_op_vwaddu_w_vx = 582,
	rv_op_vwadd_w_vx = 583,
	rv_op_vwsubu_w_vx = 584,
	rv_op_vwsub_w_vx = 585,
	rv_op_vwmulu_vx = 586,
	rv_op_vwmulsu_vx = 587,
	rv_op_vwmul_vx = 588,
	rv_op_vwmaccu_vx = 589,
	rv_op_vwmacc_vx = 590,
	rv_op_vwmaccus_vx = 591,
	rv_op_vwmaccsu_vx = 592,
	rv_op_vfadd_vv = 593,
	rv_op_vfredsum_vv = 594,
	rv_op_vfsub_vv = 595,
	rv_op_vfredosum_vv = 596,
	rv_op_vfmin_vv = 597,
	rv_op_vfredmin_vv = 598,
	rv_op_vfmax_vv = 599,
	rv_op_vfredmax_vv = 600,
	rv_op_vfsgnj_vv = 601,
	rv_op_vfsgnjn_vv = 602,
	rv_op_vfsgnjx_vv = 603,
	rv_op_vmfeq_vx = 604,
	rv_op_vmfle_vx = 605,
	rv_op_vmflt_vx = 606,
	rv_op_vmfne_vx = 607,
	rv_op_vfdiv_vx = 608,
	rv_op_vfcvt_xu_f_v = 609,
	rv_op_vfcvt_x_f_v = 610,
	rv_op_vfcvt_f_xu_v = 611,
	rv_op_vfcvt_f_x_v = 612,
	rv_op_vfcvt_rtz_xu_f_v = 613,
	rv_op_vfcvt_rtz_x_f_v = 614,
	rv_op_vfwcvt_xu_f_v = 615,
	rv_op_vfwcvt_x_f_v = 616,
	rv_op_vfwcvt_f_xu_v = 617,
	rv_op_vfwcvt_f_x_v = 618,
	rv_op_vfwcvt_f_f_v = 619,
	rv_op_vfwcvt_rtz_xu_f_v = 620,
	rv_op_vfwcvt_rtz_x_f_v = 621,
	rv_op_vfncvt_xu_f_w = 622,
	rv_op_vfncvt_x_f_w = 623,
	rv_op_vfncvt_f_xu_w = 624,
	rv_op_vfncvt_f_x_w = 625,
	rv_op_vfncvt_f_f_w = 626,
	rv_op_vfncvt_rod_f_f_w = 627,
	rv_op_vfncvt_rtz_xu_f_w = 628,
	rv_op_vfncvt_rtz_x_f_w = 629,
	rv_op_vfmul_vx = 630,
	rv_op_vfrsub_vx = 631,
	rv_op_vfmadd_vx = 632,
	rv_op_vfnmadd_vx = 633,
	rv_op_vfmsub_vx = 634,
	rv_op_vfnmsub_vx = 635,
	rv_op_vfmacc_vx = 636,
	rv_op_vfnmacc_vx = 637,
	rv_op_vfmsac_vx = 638,
	rv_op_vfnmsac_vx = 639,
	rv_op_vfwadd_vx = 640,
	rv_op_vfwredsum_vx = 641,
	rv_op_vfwsub_vx = 642,
	rv_op_vfwredosum_vx = 643,
	rv_op_vfwadd_wv = 644,
	rv_op_vfwsub_wv = 645,
	rv_op_vfwmul_vx = 646,
	rv_op_vfdot_vx = 647,
	rv_op_vfwmacc_vx = 648,
	rv_op_vfwnmacc_vx = 649,
	rv_op_vfwmsac_vx = 650,
	rv_op_vfwnmsac_vx = 651,
	rv_op_vfadd_vf = 652,
	rv_op_vfredsum_vf = 653,
	rv_op_vfsub_vf = 654,
	rv_op_vfredosum_vf = 655,
	rv_op_vfmin_vf = 656,
	rv_op_vfredmin_vf = 657,
	rv_op_vfmax_vf = 658,
	rv_op_vfredmax_vf = 659,
	rv_op_vfsgnj_vf = 660,
	rv_op_vfsgnjn_vf = 661,
	rv_op_vfsgnjx_vf = 662,
	rv_op_vfmv_s_f = 663,
	rv_op_vfmv_f_s = 664,
	rv_op_vfmv_vf = 665,
	rv_op_vmfeq_vf = 666,
	rv_op_vmfle_vf = 667,
	rv_op_vmflt_vf = 668,
	rv_op_vmfne_vf = 669,
	rv_op_vmfgt_vf = 670,
	rv_op_vmfge_vf = 671,
	rv_op_vfdiv_vf = 672,
	rv_op_vfrdiv_vf = 673,
	rv_op_vfmul_vf = 674,
	rv_op_vfrsub_vf = 675,
	rv_op_vfmadd_vf = 676,
	rv_op_vfnmadd_vf = 677,
	rv_op_vfmsub_vf = 678,
	rv_op_vfnmsub_vf = 679,
	rv_op_vfmacc_vf = 680,
	rv_op_vfnmacc_vf = 681,
	rv_op_vfmsac_vf = 682,
	rv_op_vfnmsac_vf = 683,
	rv_op_vfwadd_vf = 684,
	rv_op_vfwredsum_vf = 685,
	rv_op_vfwsub_vf = 686,
	rv_op_vfwredosum_vf = 687,
	rv_op_vfwadd_wf = 688,
	rv_op_vfwsub_wf = 689,
	rv_op_vfwmul_vf = 690,
	rv_op_vfdot_vf = 691,
	rv_op_vfwmacc_vf = 692,
	rv_op_vfwnmacc_vf = 693,
	rv_op_vfwmsac_vf = 694,
	rv_op_vfwnmsac_vf = 695,
    rv_op_last
};

struct riscvinstr {
      unsigned int opcode;
      bool has_alu;
      bool has_mul;
	  bool has_div;
      bool has_fpu;
      bool has_fdiv;
      bool has_ifpu;
      bool is_memory;
};

const riscvinstr instrlist[] = {
    // opcode, has_alu, has_mul, has_div, has_fpu, has_fdiv, has_ifpu, is_memory
	{ rv_op_illegal,        0, 0, 0, 0, 0, 0, 0 },  //    0
    { rv_op_lui,            1, 0, 0, 0, 0, 0, 0 },  //    1        RV32I alu
	{ rv_op_auipc,          1, 0, 0, 0, 0, 0, 0 },  //    2        RV32I alu
    { rv_op_jal,            1, 0, 0, 0, 0, 0, 0 },  //    3        RV32I jump
    { rv_op_jalr,           1, 0, 0, 0, 0, 0, 0 },  //    4        RV32I jump indirect
    { rv_op_beq,            1, 0, 0, 0, 0, 0, 0 },  //    5        RV32I branch
	{ rv_op_bne,            1, 0, 0, 0, 0, 0, 0 },  //    6        RV32I branch
	{ rv_op_blt,            1, 0, 0, 0, 0, 0, 0 },  //    7        RV32I branch
	{ rv_op_bge,            1, 0, 0, 0, 0, 0, 0 },  //    8        RV32I branch
	{ rv_op_bltu,           1, 0, 0, 0, 0, 0, 0 },  //    9        RV32I branch
	{ rv_op_bgeu,           1, 0, 0, 0, 0, 0, 0 },  //    10       RV32I branch
    { rv_op_lb,             1, 0, 0, 0, 0, 0, 1 },  //    11       RV32I load
	{ rv_op_lh,             1, 0, 0, 0, 0, 0, 1 },  //    12       RV32I load
	{ rv_op_lw,             1, 0, 0, 0, 0, 0, 1 },  //    13       RV32I load
	{ rv_op_lbu,            1, 0, 0, 0, 0, 0, 1 },  //    14       RV32I load
	{ rv_op_lhu,            1, 0, 0, 0, 0, 0, 1 },  //    15       RV32I load
    { rv_op_sb,             1, 0, 0, 0, 0, 0, 1 },  //    16       RV32I store
	{ rv_op_sh,             1, 0, 0, 0, 0, 0, 1 },  //    17       RV32I store
	{ rv_op_sw,             1, 0, 0, 0, 0, 0, 1 },  //    18       RV32I store
	{ rv_op_addi,           1, 0, 0, 0, 0, 0, 0 },  //    19       RV32I alu
	{ rv_op_slti,           1, 0, 0, 0, 0, 0, 0 },  //    20       RV32I alu
	{ rv_op_sltiu,          1, 0, 0, 0, 0, 0, 0 },  //    21       RV32I alu
	{ rv_op_xori,           1, 0, 0, 0, 0, 0, 0 },  //    22       RV32I alu
	{ rv_op_ori,            1, 0, 0, 0, 0, 0, 0 },  //    23       RV32I alu
	{ rv_op_andi,           1, 0, 0, 0, 0, 0, 0 },  //    24       RV32I alu
	{ rv_op_slli,           1, 0, 0, 0, 0, 0, 0 },  //    25       RV32I alu
	{ rv_op_srli,           1, 0, 0, 0, 0, 0, 0 },  //    26       RV32I alu
	{ rv_op_srai,           1, 0, 0, 0, 0, 0, 0 },  //    27       RV32I alu
	{ rv_op_add,            1, 0, 0, 0, 0, 0, 0 },  //    28       RV32I alu
	{ rv_op_sub,            1, 0, 0, 0, 0, 0, 0 },  //    29       RV32I alu
	{ rv_op_sll,            1, 0, 0, 0, 0, 0, 0 },  //    30       RV32I alu
	{ rv_op_slt,            1, 0, 0, 0, 0, 0, 0 },  //    31       RV32I alu
	{ rv_op_sltu,           1, 0, 0, 0, 0, 0, 0 },  //    32       RV32I alu
	{ rv_op_xor,            1, 0, 0, 0, 0, 0, 0 },  //    33       RV32I alu
	{ rv_op_srl,            1, 0, 0, 0, 0, 0, 0 },  //    34       RV32I alu
	{ rv_op_sra,            1, 0, 0, 0, 0, 0, 0 },  //    35       RV32I alu
	{ rv_op_or,             1, 0, 0, 0, 0, 0, 0 },  //    36       RV32I alu
	{ rv_op_and,            1, 0, 0, 0, 0, 0, 0 },  //    37       RV32I alu
    { rv_op_fence,          1, 0, 0, 0, 0, 0, 0 },  //    38       RV32I fence
	{ rv_op_fence_i,        1, 0, 0, 0, 0, 0, 0 },  //    39       RV32I fence
    { rv_op_lwu,            1, 0, 0, 0, 0, 0, 1 },  //    40       RV32I load
    { rv_op_ld,             1, 0, 0, 0, 0, 0, 1 },  //    41       RV64I load
    { rv_op_sd,             1, 0, 0, 0, 0, 0, 1 },  //    42       RV64I store
    { rv_op_addiw,          1, 0, 0, 0, 0, 0, 0 },  //    43       RV64I alu
	{ rv_op_slliw,          1, 0, 0, 0, 0, 0, 0 },  //    44       RV64I alu
	{ rv_op_srliw,          1, 0, 0, 0, 0, 0, 0 },  //    45       RV64I alu
	{ rv_op_sraiw,          1, 0, 0, 0, 0, 0, 0 },  //    46       RV64I alu
	{ rv_op_addw,           1, 0, 0, 0, 0, 0, 0 },  //    47       RV64I alu
	{ rv_op_subw,           1, 0, 0, 0, 0, 0, 0 },  //    48       RV64I alu
	{ rv_op_sllw,           1, 0, 0, 0, 0, 0, 0 },  //    49       RV64I alu
	{ rv_op_srlw,           1, 0, 0, 0, 0, 0, 0 },  //    50       RV64I alu
	{ rv_op_sraw,           1, 0, 0, 0, 0, 0, 0 },  //    51       RV64I alu
	{ rv_op_ldu,            1, 0, 0, 0, 0, 0, 0 },  //    52
	{ rv_op_lq,             1, 0, 0, 0, 0, 0, 0 },  //    53
	{ rv_op_sq,             1, 0, 0, 0, 0, 0, 0 },  //    54
	{ rv_op_addid,          1, 0, 0, 0, 0, 0, 0 },  //    55
	{ rv_op_sllid,          1, 0, 0, 0, 0, 0, 0 },  //    56
	{ rv_op_srlid,          1, 0, 0, 0, 0, 0, 0 },  //    57
	{ rv_op_sraid,          1, 0, 0, 0, 0, 0, 0 },  //    58
	{ rv_op_addd,           1, 0, 0, 0, 0, 0, 0 },  //    59
	{ rv_op_subd,           1, 0, 0, 0, 0, 0, 0 },  //    60
	{ rv_op_slld,           1, 0, 0, 0, 0, 0, 0 },  //    61
	{ rv_op_srld,           1, 0, 0, 0, 0, 0, 0 },  //    62
	{ rv_op_srad,           1, 0, 0, 0, 0, 0, 0 },  //    63
    { rv_op_mul,            1, 1, 0, 0, 0, 0, 0 },  //    64       RV32M alu multiply
	{ rv_op_mulh,           1, 1, 0, 0, 0, 0, 0 },  //    65       RV32M alu multiply
	{ rv_op_mulhsu,         1, 1, 0, 0, 0, 0, 0 },  //    66       RV32M alu multiply
	{ rv_op_mulhu,          1, 1, 0, 0, 0, 0, 0 },  //    67       RV32M alu multiply
    { rv_op_div,            1, 0, 1, 0, 0, 0, 0 },  //    68       RV32M alu divide
	{ rv_op_divu,           1, 0, 1, 0, 0, 0, 0 },  //    69       RV32M alu divide
	{ rv_op_rem,            1, 0, 1, 0, 0, 0, 0 },  //    70       RV32M alu divide
	{ rv_op_remu,           1, 0, 1, 0, 0, 0, 0 },  //    71       RV32M alu divide
    { rv_op_mulw,           1, 1, 0, 0, 0, 0, 0 },  //    72       RV64M alu multiply
	{ rv_op_divw,           1, 0, 1, 0, 0, 0, 0 },  //    73       RV64M alu divide
	{ rv_op_divuw,          1, 0, 1, 0, 0, 0, 0 },  //    74       RV64M alu divide
	{ rv_op_remw,           1, 0, 1, 0, 0, 0, 0 },  //    75       RV64M alu divide
	{ rv_op_remuw,          1, 0, 1, 0, 0, 0, 0 },  //    76       RV64M alu divide
	{ rv_op_muld,           1, 1, 0, 0, 0, 0, 0 },  //    77
	{ rv_op_divd,           1, 0, 1, 0, 0, 0, 0 },  //    78
	{ rv_op_divud,          1, 0, 1, 0, 0, 0, 0 },  //    79
	{ rv_op_remd,           1, 0, 1, 0, 0, 0, 0 },  //    80
	{ rv_op_remud,          1, 0, 1, 0, 0, 0, 0 },  //    81
    { rv_op_lr_w,           1, 0, 0, 0, 0, 0, 1 },  //    82    RV32A atomic
	{ rv_op_sc_w,           1, 0, 0, 0, 0, 0, 1 },  //    83    RV32A atomic
	{ rv_op_amoswap_w,      1, 0, 0, 0, 0, 0, 1 },  //    84    RV32A atomic
	{ rv_op_amoadd_w,       1, 0, 0, 0, 0, 0, 0 },  //    85    RV32A atomic
	{ rv_op_amoxor_w,       1, 0, 0, 0, 0, 0, 0 },  //    86    RV32A atomic
	{ rv_op_amoor_w,        1, 0, 0, 0, 0, 0, 0 },  //    87    RV32A atomic
	{ rv_op_amoand_w,       1, 0, 0, 0, 0, 0, 0 },  //    88    RV32A atomic
	{ rv_op_amomin_w,       1, 0, 0, 0, 0, 0, 0 },  //    89    RV32A atomic
	{ rv_op_amomax_w,       1, 0, 0, 0, 0, 0, 0 },  //    90    RV32A atomic
	{ rv_op_amominu_w,      1, 0, 0, 0, 0, 0, 0 },  //    91    RV32A atomic
	{ rv_op_amomaxu_w,      1, 0, 0, 0, 0, 0, 0 },  //    92    RV32A atomic
	{ rv_op_lr_d,           1, 0, 0, 0, 0, 0, 1 },  //    93    RV64A atomic
	{ rv_op_sc_d,           1, 0, 0, 0, 0, 0, 1 },  //    94    RV64A atomic
	{ rv_op_amoswap_d,      1, 0, 0, 0, 0, 0, 1 },  //    95    RV64A atomic
	{ rv_op_amoadd_d,       1, 0, 0, 0, 0, 0, 0 },  //    96    RV64A atomic
	{ rv_op_amoxor_d,       1, 0, 0, 0, 0, 0, 0 },  //    97    RV64A atomic
	{ rv_op_amoor_d,        1, 0, 0, 0, 0, 0, 0 },  //    98    RV64A atomic
	{ rv_op_amoand_d,       1, 0, 0, 0, 0, 0, 0 },  //    99    RV64A atomic
	{ rv_op_amomin_d,       1, 0, 0, 0, 0, 0, 0 },  //    100   RV64A atomic
	{ rv_op_amomax_d,       1, 0, 0, 0, 0, 0, 0 },  //    101   RV64A atomic
	{ rv_op_amominu_d,      1, 0, 0, 0, 0, 0, 0 },  //    102   RV64A atomic
	{ rv_op_amomaxu_d,      1, 0, 0, 0, 0, 0, 0 },  //    103   RV64A atomic
	{ rv_op_lr_q,           1, 0, 0, 0, 0, 0, 1 },  //    104
	{ rv_op_sc_q,           1, 0, 0, 0, 0, 0, 1 },  //    105
	{ rv_op_amoswap_q,      1, 0, 0, 0, 0, 0, 1 },  //    106
	{ rv_op_amoadd_q,       1, 0, 0, 0, 0, 0, 0 },  //    107
	{ rv_op_amoxor_q,       1, 0, 0, 0, 0, 0, 0 },  //    108
	{ rv_op_amoor_q,        1, 0, 0, 0, 0, 0, 0 },  //    109
	{ rv_op_amoand_q,       1, 0, 0, 0, 0, 0, 0 },  //    110
	{ rv_op_amomin_q,       1, 0, 0, 0, 0, 0, 0 },  //    111
	{ rv_op_amomax_q,       1, 0, 0, 0, 0, 0, 0 },  //    112
	{ rv_op_amominu_q,      1, 0, 0, 0, 0, 0, 0 },  //    113
	{ rv_op_amomaxu_q,      1, 0, 0, 0, 0, 0, 0 },  //    114
    { rv_op_ecall,          1, 0, 0, 0, 0, 0, 0 },  //    115      RV32S system
	{ rv_op_ebreak,         1, 0, 0, 0, 0, 0, 0 },  //    116      RV32S system
	{ rv_op_uret,           1, 0, 0, 0, 0, 0, 0 },  //    117      RV32S system
	{ rv_op_sret,           1, 0, 0, 0, 0, 0, 0 },  //    118      RV32S system
	{ rv_op_hret,           1, 0, 0, 0, 0, 0, 0 },  //    119      RV32S system
	{ rv_op_mret,           1, 0, 0, 0, 0, 0, 0 },  //    120      RV32S system
	{ rv_op_dret,           1, 0, 0, 0, 0, 0, 0 },  //    121      RV32S system
	{ rv_op_sfence_vm,      1, 0, 0, 0, 0, 0, 0 },  //    122      RV32S system
	{ rv_op_sfence_vma,     1, 0, 0, 0, 0, 0, 0 },  //    123      RV32S system
	{ rv_op_wfi,            1, 0, 0, 0, 0, 0, 0 },  //    124      RV32S system
    { rv_op_csrrw,          1, 0, 0, 0, 0, 0, 0 },  //    124      RV32S csr
	{ rv_op_csrrs,          1, 0, 0, 0, 0, 0, 0 },  //    125      RV32S csr
	{ rv_op_csrrc,          1, 0, 0, 0, 0, 0, 0 },  //    126      RV32S csr
	{ rv_op_csrrwi,         1, 0, 0, 0, 0, 0, 0 },  //    127      RV32S csr
	{ rv_op_csrrsi,         1, 0, 0, 0, 0, 0, 0 },  //    128      RV32S csr
	{ rv_op_csrrci,         1, 0, 0, 0, 0, 0, 0 },  //    129      RV32S csr
    { rv_op_flw,            1, 0, 0, 1, 0, 0, 1 },  //    130      RV32F fpu load
    { rv_op_fsw,            1, 0, 0, 1, 0, 0, 1 },  //    131      RV32F fpu store
    { rv_op_fmadd_s,        1, 0, 0, 1, 0, 0, 0 },  //    132      RV32F fpu fma
	{ rv_op_fmsub_s,        1, 0, 0, 1, 0, 0, 0 },  //    133      RV32F fpu fma
	{ rv_op_fnmsub_s,       1, 0, 0, 1, 0, 0, 0 },  //    134      RV32F fpu fma
	{ rv_op_fnmadd_s,       1, 0, 0, 1, 0, 0, 0 },  //    135      RV32F fpu fma
	{ rv_op_fadd_s,         1, 0, 0, 1, 0, 0, 0 },  //    136      RV32F fpu
	{ rv_op_fsub_s,         1, 0, 0, 1, 0, 0, 0 },  //    137      RV32F fpu
	{ rv_op_fmul_s,         1, 0, 0, 1, 1, 0, 0 },  //    138      RV32F fpu
    { rv_op_fdiv_s,         1, 0, 0, 1, 1, 0, 0 },  //    139      RV32F fpu fdiv
	{ rv_op_fsgnj_s,        1, 0, 0, 1, 0, 0, 0 },  //    140      RV32F fpu
	{ rv_op_fsgnjn_s,       1, 0, 0, 1, 0, 0, 0 },  //    141      RV32F fpu
	{ rv_op_fsgnjx_s,       1, 0, 0, 1, 0, 0, 0 },  //    142      RV32F fpu
	{ rv_op_fmin_s,         1, 0, 0, 1, 0, 0, 0 },  //    143      RV32F fpu
	{ rv_op_fmax_s,         1, 0, 0, 1, 0, 0, 0 },  //    144      RV32F fpu
    { rv_op_fsqrt_s,        1, 0, 0, 1, 0, 0, 0 },  //    145      RV32F fpu fsqrt
	{ rv_op_fle_s,          1, 0, 0, 1, 0, 0, 0 },  //    146      RV32F fpu
	{ rv_op_flt_s,          1, 0, 0, 1, 0, 0, 0 },  //    147      RV32F fpu
	{ rv_op_feq_s,          1, 0, 0, 1, 0, 0, 0 },  //    148      RV32F fpu
	{ rv_op_fcvt_w_s,       1, 0, 0, 1, 0, 1, 0 },  //    149      RV32F fpu fcvt
	{ rv_op_fcvt_wu_s,      1, 0, 0, 1, 0, 1, 0 },  //    150      RV32F fpu fcvt
	{ rv_op_fcvt_s_w,       1, 0, 0, 1, 0, 1, 0 },  //    151      RV32F fpu fcvt
	{ rv_op_fcvt_s_wu,      1, 0, 0, 1, 0, 1, 0 },  //    152      RV32F fpu fcvt
    { rv_op_fmv_x_s,        1, 0, 0, 1, 0, 0, 0 },  //    153      RV32F fpu fmove
	{ rv_op_fclass_s,       1, 0, 0, 1, 0, 0, 0 },  //    154      RV32F fpu
    { rv_op_fmv_s_x,        1, 0, 0, 1, 0, 0, 0 },  //    155      RV32F fpu fmove
    { rv_op_fcvt_l_s,       1, 0, 0, 1, 0, 1, 0 },  //    156      RV64F fpu fcvt
	{ rv_op_fcvt_lu_s,      1, 0, 0, 1, 0, 1, 0 },  //    157      RV64F fpu fcvt
	{ rv_op_fcvt_s_l,       1, 0, 0, 1, 0, 1, 0 },  //    158      RV64F fpu fcvt
	{ rv_op_fcvt_s_lu,      1, 0, 0, 1, 0, 1, 0 },  //    159      RV64F fpu fcvt
    { rv_op_fld,            1, 0, 0, 1, 0, 0, 1 },  //    160      RV32D fpu load
    { rv_op_fsd,            1, 0, 0, 1, 0, 0, 1 },  //    161      RV32D fpu store
	{ rv_op_fmadd_d,        1, 0, 0, 1, 0, 0, 0 },  //    162      RV32D fpu fma
	{ rv_op_fmsub_d,        1, 0, 0, 1, 0, 0, 0 },  //    163      RV32D fpu fma
	{ rv_op_fnmsub_d,       1, 0, 0, 1, 0, 0, 0 },  //    164      RV32D fpu fma
	{ rv_op_fnmadd_d,       1, 0, 0, 1, 0, 0, 0 },  //    165      RV32D fpu fma
	{ rv_op_fadd_d,         1, 0, 0, 1, 0, 0, 0 },  //    166      RV32D fpu
	{ rv_op_fsub_d,         1, 0, 0, 1, 0, 0, 0 },  //    167      RV32D fpu
	{ rv_op_fmul_d,         1, 0, 0, 1, 1, 0, 0 },  //    168      RV32D fpu
    { rv_op_fdiv_d,         1, 0, 0, 1, 1, 0, 0 },  //    169      RV32D fpu fdiv
	{ rv_op_fsgnj_d,        1, 0, 0, 1, 0, 0, 0 },  //    170      RV32D fpu
	{ rv_op_fsgnjn_d,       1, 0, 0, 1, 0, 0, 0 },  //    171      RV32D fpu
	{ rv_op_fsgnjx_d,       1, 0, 0, 1, 0, 0, 0 },  //    172      RV32D fpu
	{ rv_op_fmin_d,         1, 0, 0, 1, 0, 0, 0 },  //    173      RV32D fpu
	{ rv_op_fmax_d,         1, 0, 0, 1, 0, 0, 0 },  //    174      RV32D fpu
    { rv_op_fcvt_s_d,       1, 0, 0, 1, 0, 1, 0 },  //    175      RV32D fpu fcvt
	{ rv_op_fcvt_d_s,       1, 0, 0, 1, 0, 1, 0 },  //    176      RV32D fpu fcvt
    { rv_op_fsqrt_d,        1, 0, 0, 1, 0, 0, 0 },  //    177      RV32D fpu fsqrt
	{ rv_op_fle_d,          1, 0, 0, 1, 0, 0, 0 },  //    178      RV32D fpu
	{ rv_op_flt_d,          1, 0, 0, 1, 0, 0, 0 },  //    179      RV32D fpu
	{ rv_op_feq_d,          1, 0, 0, 1, 0, 0, 0 },  //    180      RV32D fpu
    { rv_op_fcvt_w_d,       1, 0, 0, 1, 0, 1, 0 },  //    181      RV32D fpu fcvt
	{ rv_op_fcvt_wu_d,      1, 0, 0, 1, 0, 1, 0 },  //    182      RV32D fpu fcvt
	{ rv_op_fcvt_d_w,       1, 0, 0, 1, 0, 1, 0 },  //    183      RV32D fpu fcvt
	{ rv_op_fcvt_d_wu,      1, 0, 0, 1, 0, 1, 0 },  //    184      RV32D fpu fcvt
	{ rv_op_fclass_d,       1, 0, 0, 1, 0, 0, 0 },  //    185      RV32D fpu
	{ rv_op_fcvt_l_d,       1, 0, 0, 1, 0, 1, 0 },  //    186      RV64D fpu fcvt
	{ rv_op_fcvt_lu_d,      1, 0, 0, 1, 0, 1, 0 },  //    187      RV64D fpu fcvt
    { rv_op_fmv_x_d,        1, 0, 0, 1, 0, 0, 0 },  //    188      RV64D fpu fmove
	{ rv_op_fcvt_d_l,       1, 0, 0, 1, 0, 1, 0 },  //    189      RV64D fpu fcvt
	{ rv_op_fcvt_d_lu,      1, 0, 0, 1, 0, 1, 0 },  //    190      RV64D fpu fcvt
	{ rv_op_fmv_d_x,        1, 0, 0, 1, 0, 0, 0 },  //    191      RV64D fpu fmove
	{ rv_op_flq,            1, 0, 0, 1, 0, 0, 1 },  //    192
	{ rv_op_fsq,            1, 0, 0, 1, 0, 0, 1 },  //    193
	{ rv_op_fmadd_q,        1, 0, 0, 1, 0, 0, 0 },  //    194
	{ rv_op_fmsub_q,        1, 0, 0, 1, 0, 0, 0 },  //    195
	{ rv_op_fnmsub_q,       1, 0, 0, 1, 0, 0, 0 },  //    196
	{ rv_op_fnmadd_q,       1, 0, 0, 1, 0, 0, 0 },  //    197
	{ rv_op_fadd_q,         1, 0, 0, 1, 0, 0, 0 },  //    198
	{ rv_op_fsub_q,         1, 0, 0, 1, 0, 0, 0 },  //    199
	{ rv_op_fmul_q,         1, 0, 0, 1, 1, 0, 0 },  //    200
	{ rv_op_fdiv_q,         1, 0, 0, 1, 1, 0, 0 },  //    201
	{ rv_op_fsgnj_q,        1, 0, 0, 1, 0, 0, 0 },  //    202
	{ rv_op_fsgnjn_q,       1, 0, 0, 1, 0, 0, 0 },  //    203
	{ rv_op_fsgnjx_q,       1, 0, 0, 1, 0, 0, 0 },  //    204
	{ rv_op_fmin_q,         1, 0, 0, 1, 0, 0, 0 },  //    205
	{ rv_op_fmax_q,         1, 0, 0, 1, 0, 0, 0 },  //    206
	{ rv_op_fcvt_s_q,       1, 0, 0, 1, 0, 0, 0 },  //    207
	{ rv_op_fcvt_q_s,       1, 0, 0, 1, 0, 0, 0 },  //    208
	{ rv_op_fcvt_d_q,       1, 0, 0, 1, 0, 0, 0 },  //    209
	{ rv_op_fcvt_q_d,       1, 0, 0, 1, 0, 0, 0 },  //    210
	{ rv_op_fsqrt_q,        1, 0, 0, 1, 0, 0, 0 },  //    211
	{ rv_op_fle_q,          1, 0, 0, 1, 0, 0, 0 },  //    212
	{ rv_op_flt_q,          1, 0, 0, 1, 0, 0, 0 },  //    213
	{ rv_op_feq_q,          1, 0, 0, 1, 0, 0, 0 },  //    214
	{ rv_op_fcvt_w_q,       1, 0, 0, 1, 0, 0, 0 },  //    215
	{ rv_op_fcvt_wu_q,      1, 0, 0, 1, 0, 0, 0 },  //    216
	{ rv_op_fcvt_q_w,       1, 0, 0, 1, 0, 0, 0 },  //    217
	{ rv_op_fcvt_q_wu,      1, 0, 0, 1, 0, 0, 0 },  //    218
	{ rv_op_fclass_q,       1, 0, 0, 1, 0, 0, 0 },  //    219
	{ rv_op_fcvt_l_q,       1, 0, 0, 1, 0, 0, 0 },  //    220
	{ rv_op_fcvt_lu_q,      1, 0, 0, 1, 0, 0, 0 },  //    221
	{ rv_op_fcvt_q_l,       1, 0, 0, 1, 0, 0, 0 },  //    222
	{ rv_op_fcvt_q_lu,      1, 0, 0, 1, 0, 0, 0 },  //    223
	{ rv_op_fmv_x_q,        1, 0, 0, 1, 0, 0, 0 },  //    224
	{ rv_op_fmv_q_x,        1, 0, 0, 1, 0, 0, 0 },  //    225
	{ rv_op_c_addi4spn,     1, 0, 0, 0, 0, 0, 0 },  //    226
	{ rv_op_c_fld,          1, 0, 0, 0, 0, 0, 1 },  //    227
	{ rv_op_c_lw,           1, 0, 0, 0, 0, 0, 1 },  //    228
	{ rv_op_c_flw,          1, 0, 0, 0, 0, 0, 1 },  //    229
	{ rv_op_c_fsd,          1, 0, 0, 0, 0, 0, 1 },  //    230
	{ rv_op_c_sw,           1, 0, 0, 0, 0, 0, 1 },  //    231
	{ rv_op_c_fsw,          1, 0, 0, 0, 0, 0, 1 },  //    232
	{ rv_op_c_nop,          1, 0, 0, 0, 0, 0, 0 },  //    233
	{ rv_op_c_addi,         1, 0, 0, 0, 0, 0, 0 },  //    234
	{ rv_op_c_jal,          1, 0, 0, 0, 0, 0, 0 },  //    235
	{ rv_op_c_li,           1, 0, 0, 0, 0, 0, 0 },  //    236
	{ rv_op_c_addi16sp,     1, 0, 0, 0, 0, 0, 0 },  //    237
	{ rv_op_c_lui,          1, 0, 0, 0, 0, 0, 0 },  //    238
	{ rv_op_c_srli,         1, 0, 0, 0, 0, 0, 0 },  //    239
	{ rv_op_c_srai,         1, 0, 0, 0, 0, 0, 0 },  //    240
	{ rv_op_c_andi,         1, 0, 0, 0, 0, 0, 0 },  //    241
	{ rv_op_c_sub,          1, 0, 0, 0, 0, 0, 0 },  //    242
	{ rv_op_c_xor,          1, 0, 0, 0, 0, 0, 0 },  //    243
	{ rv_op_c_or,           1, 0, 0, 0, 0, 0, 0 },  //    244
	{ rv_op_c_and,          1, 0, 0, 0, 0, 0, 0 },  //    245
	{ rv_op_c_subw,         1, 0, 0, 0, 0, 0, 0 },  //    246
	{ rv_op_c_addw,         1, 0, 0, 0, 0, 0, 0 },  //    247
	{ rv_op_c_j,            1, 0, 0, 0, 0, 0, 0 },  //    248
	{ rv_op_c_beqz,         1, 0, 0, 0, 0, 0, 0 },  //    249
	{ rv_op_c_bnez,         1, 0, 0, 0, 0, 0, 0 },  //    250
	{ rv_op_c_slli,         1, 0, 0, 0, 0, 0, 0 },  //    251
	{ rv_op_c_fldsp,        1, 0, 0, 0, 0, 0, 0 },  //    252
	{ rv_op_c_lwsp,         1, 0, 0, 0, 0, 0, 0 },  //    253
	{ rv_op_c_flwsp,        1, 0, 0, 0, 0, 0, 0 },  //    254
	{ rv_op_c_jr,           1, 0, 0, 0, 0, 0, 0 },  //    255
	{ rv_op_c_mv,           1, 0, 0, 0, 0, 0, 0 },  //    256
	{ rv_op_c_ebreak,       1, 0, 0, 0, 0, 0, 0 },  //    257
	{ rv_op_c_jalr,         1, 0, 0, 0, 0, 0, 0 },  //    258
	{ rv_op_c_add,          1, 0, 0, 0, 0, 0, 0 },  //    259
	{ rv_op_c_fsdsp,        1, 0, 0, 0, 0, 0, 0 },  //    260
	{ rv_op_c_swsp,         1, 0, 0, 0, 0, 0, 0 },  //    261
	{ rv_op_c_fswsp,        1, 0, 0, 0, 0, 0, 0 },  //    262
	{ rv_op_c_ld,           1, 0, 0, 0, 0, 0, 1 },  //    263
	{ rv_op_c_sd,           1, 0, 0, 0, 0, 0, 1 },  //    264
	{ rv_op_c_addiw,        1, 0, 0, 0, 0, 0, 0 },  //    265
	{ rv_op_c_ldsp,         1, 0, 0, 0, 0, 0, 1 },  //    266
	{ rv_op_c_sdsp,         1, 0, 0, 0, 0, 0, 1 },  //    267
	{ rv_op_c_lq,           1, 0, 0, 0, 0, 0, 1 },  //    268
	{ rv_op_c_sq,           1, 0, 0, 0, 0, 0, 1 },  //    269
	{ rv_op_c_lqsp,         1, 0, 0, 0, 0, 0, 0 },  //    270
	{ rv_op_c_sqsp,         1, 0, 0, 0, 0, 0, 0 },  //    271
    { rv_op_nop,            1, 0, 0, 0, 0, 0, 0 },  //    272
	{ rv_op_mv,             1, 0, 0, 0, 0, 0, 0 },  //    273
	{ rv_op_not,            1, 0, 0, 0, 0, 0, 0 },  //    274
	{ rv_op_neg,            1, 0, 0, 0, 0, 0, 0 },  //    275
	{ rv_op_negw,           1, 0, 0, 0, 0, 0, 0 },  //    276
	{ rv_op_sext_w,         1, 0, 0, 0, 0, 0, 0 },  //    277
	{ rv_op_seqz,           1, 0, 0, 0, 0, 0, 0 },  //    278
	{ rv_op_snez,           1, 0, 0, 0, 0, 0, 0 },  //    279
	{ rv_op_sltz,           1, 0, 0, 0, 0, 0, 0 },  //    280
	{ rv_op_sgtz,           1, 0, 0, 0, 0, 0, 0 },  //    281
	{ rv_op_fmv_s,          1, 0, 0, 0, 0, 0, 0 },  //    282
	{ rv_op_fabs_s,         1, 0, 0, 0, 0, 0, 0 },  //    283
	{ rv_op_fneg_s,         1, 0, 0, 0, 0, 0, 0 },  //    284
	{ rv_op_fmv_d,          1, 0, 0, 0, 0, 0, 0 },  //    285
	{ rv_op_fabs_d,         1, 0, 0, 0, 0, 0, 0 },  //    286
	{ rv_op_fneg_d,         1, 0, 0, 0, 0, 0, 0 },  //    287
	{ rv_op_fmv_q,          1, 0, 0, 0, 0, 0, 0 },  //    288
	{ rv_op_fabs_q,         1, 0, 0, 0, 0, 0, 0 },  //    289
	{ rv_op_fneg_q,         1, 0, 0, 0, 0, 0, 0 },  //    290
	{ rv_op_beqz,           1, 0, 0, 0, 0, 0, 0 },  //    291
	{ rv_op_bnez,           1, 0, 0, 0, 0, 0, 0 },  //    292
	{ rv_op_blez,           1, 0, 0, 0, 0, 0, 0 },  //    293
	{ rv_op_bgez,           1, 0, 0, 0, 0, 0, 0 },  //    294
	{ rv_op_bltz,           1, 0, 0, 0, 0, 0, 0 },  //    295
	{ rv_op_bgtz,           1, 0, 0, 0, 0, 0, 0 },  //    296
	{ rv_op_ble,            1, 0, 0, 0, 0, 0, 0 },  //    297
	{ rv_op_bleu,           1, 0, 0, 0, 0, 0, 0 },  //    298
	{ rv_op_bgt,            1, 0, 0, 0, 0, 0, 0 },  //    299
	{ rv_op_bgtu,           1, 0, 0, 0, 0, 0, 0 },  //    300
	{ rv_op_j,              1, 0, 0, 0, 0, 0, 0 },  //    301
	{ rv_op_ret,            1, 0, 0, 0, 0, 0, 0 },  //    302
	{ rv_op_jr,             1, 0, 0, 0, 0, 0, 0 },  //    303

	{ rv_op_rdcycle,        1, 0, 0, 0, 0, 0, 0 },  //    304      RV32S csr
	{ rv_op_rdtime,         1, 0, 0, 0, 0, 0, 0 },  //    305      RV32S csr
	{ rv_op_rdinstret,      1, 0, 0, 0, 0, 0, 0 },  //    306      RV32S csr
	{ rv_op_rdcycleh,       1, 0, 0, 0, 0, 0, 0 },  //    307      RV32S csr
	{ rv_op_rdtimeh,        1, 0, 0, 0, 0, 0, 0 },  //    308      RV32S csr
	{ rv_op_rdinstreth,     1, 0, 0, 0, 0, 0, 0 },  //    309      RV32S csr

    { rv_op_frcsr,          1, 0, 0, 0, 0, 0, 0 },  //    310      RV32FD csr
	{ rv_op_frrm,           1, 0, 0, 0, 0, 0, 0 },  //    311      RV32FD csr
	{ rv_op_frflags,        1, 0, 0, 0, 0, 0, 0 },  //    312      RV32FD csr
	{ rv_op_fscsr,          1, 0, 0, 0, 0, 0, 0 },  //    313      RV32FD csr
	{ rv_op_fsrm,           1, 0, 0, 0, 0, 0, 0 },  //    314      RV32FD csr
	{ rv_op_fsflags,        1, 0, 0, 0, 0, 0, 0 },  //    315      RV32FD csr
	{ rv_op_fsrmi,          1, 0, 0, 0, 0, 0, 0 },  //    316      RV32FD csr
	{ rv_op_fsflagsi,       1, 0, 0, 0, 0, 0, 0 },  //    317      RV32FD csr


	{ rv_op_vsetvli        ,0, 0, 0, 0, 0, 0, 1 },  // 319,
	{ rv_op_vle8_v         ,0, 0, 0, 0, 0, 0, 1 },  // 320,
	{ rv_op_vse8_v         ,0, 0, 0, 0, 0, 0, 1 },  // 321,
	{ rv_op_vle16_v        ,0, 0, 0, 0, 0, 0, 1 },  // 322,
	{ rv_op_vse16_v        ,0, 0, 0, 0, 0, 0, 1 },  // 323,
	{ rv_op_vle32_v        ,0, 0, 0, 0, 0, 0, 1 },  // 324,
	{ rv_op_vse32_v        ,0, 0, 0, 0, 0, 0, 1 },  // 325,
	{ rv_op_vle64_v        ,0, 0, 0, 0, 0, 0, 1 },  // 326,
	{ rv_op_vse64_v        ,0, 0, 0, 0, 0, 0, 1 },  // 327,
	{ rv_op_vleff8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 328,
	{ rv_op_vleff16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 329,
	{ rv_op_vleff32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 330,
	{ rv_op_vleff64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 331,
	{ rv_op_vl1re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 332,
	{ rv_op_vl1re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 333,
	{ rv_op_vl1re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 334,
	{ rv_op_vl1re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 335,
	{ rv_op_vl2re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 336,
	{ rv_op_vl2re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 337,
	{ rv_op_vl2re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 338,
	{ rv_op_vl2re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 339,
	{ rv_op_vl4re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 340,
	{ rv_op_vl4re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 341,
	{ rv_op_vl4re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 342,
	{ rv_op_vl4re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 343,
	{ rv_op_vl8re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 344,
	{ rv_op_vl8re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 345,
	{ rv_op_vl8re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 346,
	{ rv_op_vl8re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 347,
	{ rv_op_vs1re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 348,
	{ rv_op_vs1re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 349,
	{ rv_op_vs1re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 350,
	{ rv_op_vs1re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 351,
	{ rv_op_vs2re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 352,
	{ rv_op_vs2re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 353,
	{ rv_op_vs2re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 354,
	{ rv_op_vs2re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 355,
	{ rv_op_vs4re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 356,
	{ rv_op_vs4re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 357,
	{ rv_op_vs4re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 358,
	{ rv_op_vs4re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 359,
	{ rv_op_vs8re8_v       ,0, 0, 0, 0, 0, 0, 1 },  // 360,
	{ rv_op_vs8re16_v      ,0, 0, 0, 0, 0, 0, 1 },  // 361,
	{ rv_op_vs8re32_v      ,0, 0, 0, 0, 0, 0, 1 },  // 362,
	{ rv_op_vs8re64_v      ,0, 0, 0, 0, 0, 0, 1 },  // 363,
	{ rv_op_vlse8_v        ,0, 0, 0, 0, 0, 0, 1 },  // 364,
	{ rv_op_vsse8_v        ,0, 0, 0, 0, 0, 0, 1 },  // 365,
	{ rv_op_vlse16_v       ,0, 0, 0, 0, 0, 0, 1 },  // 366,
	{ rv_op_vsse16_v       ,0, 0, 0, 0, 0, 0, 1 },  // 367,
	{ rv_op_vlse32_v       ,0, 0, 0, 0, 0, 0, 1 },  // 368,
	{ rv_op_vsse32_v       ,0, 0, 0, 0, 0, 0, 1 },  // 369,
	{ rv_op_vlse64_v       ,0, 0, 0, 0, 0, 0, 1 },  // 370,
	{ rv_op_vsse64_v       ,0, 0, 0, 0, 0, 0, 1 },  // 371,
	{ rv_op_vadd_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 372,
	{ rv_op_vsub_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 373,
	{ rv_op_vminu_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 374,
	{ rv_op_vmin_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 375,
	{ rv_op_vmaxu_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 376,
	{ rv_op_vmax_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 377,
	{ rv_op_vand_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 378,
	{ rv_op_vor_vv         ,0, 0, 0, 0, 0, 0, 1 },  // 379,
	{ rv_op_vxor_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 380,
	{ rv_op_vrgather_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 381,
	{ rv_op_vadc_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 382,
	{ rv_op_vmadc_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 383,
	{ rv_op_vsbc_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 384,
	{ rv_op_vmsbc_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 385,
	{ rv_op_vmerge_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 386,
	{ rv_op_vmseq_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 387,
	{ rv_op_vmsne_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 388,
	{ rv_op_vmsltu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 389,
	{ rv_op_vmslt_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 390,
	{ rv_op_vmsleu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 391,
	{ rv_op_vmsle_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 392,
	{ rv_op_vsaddu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 393,
	{ rv_op_vsadd_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 394,
	{ rv_op_vssubu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 395,
	{ rv_op_vssub_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 396,
	{ rv_op_vsll_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 397,
	{ rv_op_vsmul_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 398,
	{ rv_op_vsrl_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 399,
	{ rv_op_vsra_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 400,
	{ rv_op_vssrl_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 401,
	{ rv_op_vssra_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 402,
	{ rv_op_vnsrl_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 403,
	{ rv_op_vnsra_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 404,
	{ rv_op_vnclipu_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 405,
	{ rv_op_vnclip_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 406,
	{ rv_op_vwredsumu_vv   ,0, 0, 0, 0, 0, 0, 1 },  // 407,
	{ rv_op_vwredsum_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 408,
	{ rv_op_vdotu_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 409,
	{ rv_op_vdot_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 410,
	{ rv_op_vqmaccu_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 411,
	{ rv_op_vqmacc_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 412,
	{ rv_op_vqmaccus_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 413,
	{ rv_op_vqmaccsu_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 414,
	{ rv_op_vadd_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 415,
	{ rv_op_vsub_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 416,
	{ rv_op_vrsub_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 417,
	{ rv_op_vminu_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 418,
	{ rv_op_vmin_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 419,
	{ rv_op_vmaxu_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 420,
	{ rv_op_vmax_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 421,
	{ rv_op_vand_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 422,
	{ rv_op_vor_vx         ,0, 0, 0, 0, 0, 0, 1 },  // 423,
	{ rv_op_vxor_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 424,
	{ rv_op_vrgather_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 425,
	{ rv_op_vslideup_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 426,
	{ rv_op_vslidedown_vx  ,0, 0, 0, 0, 0, 0, 1 },  // 427,
	{ rv_op_vadc_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 428,
	{ rv_op_vmadc_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 429,
	{ rv_op_vsbc_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 430,
	{ rv_op_vmsbc_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 431,
	{ rv_op_vmerge_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 432,
	{ rv_op_vmseq_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 433,
	{ rv_op_vmsne_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 434,
	{ rv_op_vmsltu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 435,
	{ rv_op_vmslt_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 436,
	{ rv_op_vmsleu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 437,
	{ rv_op_vmsle_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 438,
	{ rv_op_vmsgtu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 439,
	{ rv_op_vmsgt_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 440,
	{ rv_op_vsaddu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 441,
	{ rv_op_vsadd_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 442,
	{ rv_op_vssubu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 443,
	{ rv_op_vssub_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 444,
	{ rv_op_vsll_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 445,
	{ rv_op_vsmul_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 446,
	{ rv_op_vsrl_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 447,
	{ rv_op_vsra_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 448,
	{ rv_op_vssrl_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 449,
	{ rv_op_vssra_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 450,
	{ rv_op_vnsrl_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 451,
	{ rv_op_vnsra_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 452,
	{ rv_op_vnclipu_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 453,
	{ rv_op_vnclip_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 454,
	{ rv_op_vwredsumu_vx   ,0, 0, 0, 0, 0, 0, 1 },  // 455,
	{ rv_op_vwredsum_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 456,
	{ rv_op_vdotu_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 457,
	{ rv_op_vdot_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 458,
	{ rv_op_vqmaccu_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 459,
	{ rv_op_vqmacc_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 460,
	{ rv_op_vqmaccus_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 461,
	{ rv_op_vqmaccsu_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 462,
	{ rv_op_vadd_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 463,
	{ rv_op_vrsub_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 464,
	{ rv_op_vand_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 465,
	{ rv_op_vor_vi         ,0, 0, 0, 0, 0, 0, 1 },  // 466,
	{ rv_op_vxor_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 467,
	{ rv_op_vrgather_vi    ,0, 0, 0, 0, 0, 0, 1 },  // 468,
	{ rv_op_vslideup_vi    ,0, 0, 0, 0, 0, 0, 1 },  // 469,
	{ rv_op_vslidedown_vi  ,0, 0, 0, 0, 0, 0, 1 },  // 470,
	{ rv_op_vadc_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 471,
	{ rv_op_vmadc_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 472,
	{ rv_op_vmv_vi         ,0, 0, 0, 0, 0, 0, 1 },  // 473,
	{ rv_op_vmseq_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 474,
	{ rv_op_vmsne_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 475,
	{ rv_op_vmsleu_vi      ,0, 0, 0, 0, 0, 0, 1 },  // 476,
	{ rv_op_vmsle_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 477,
	{ rv_op_vmsgtu_vi      ,0, 0, 0, 0, 0, 0, 1 },  // 478,
	{ rv_op_vmsgt_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 479,
	{ rv_op_vmv1r          ,0, 0, 0, 0, 0, 0, 1 },
	{ rv_op_vmv2r          ,0, 0, 0, 0, 0, 0, 1 },
	{ rv_op_vmv4r          ,0, 0, 0, 0, 0, 0, 1 },
	{ rv_op_vmv8r          ,0, 0, 0, 0, 0, 0, 1 },
	{ rv_op_vsaddu_vi      ,0, 0, 0, 0, 0, 0, 1 },  // 480,
	{ rv_op_vsadd_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 481,
	{ rv_op_vsll_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 482,
	{ rv_op_vsrl_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 483,
	{ rv_op_vsra_vi        ,0, 0, 0, 0, 0, 0, 1 },  // 484,
	{ rv_op_vssrl_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 485,
	{ rv_op_vssra_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 486,
	{ rv_op_vnsrl_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 487,
	{ rv_op_vnsra_vi       ,0, 0, 0, 0, 0, 0, 1 },  // 488,
	{ rv_op_vnclipu_vi     ,0, 0, 0, 0, 0, 0, 1 },  // 489,
	{ rv_op_vnclip_vi      ,0, 0, 0, 0, 0, 0, 1 },  // 490,
	{ rv_op_vredsum_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 491,
	{ rv_op_vredand_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 492,
	{ rv_op_vredor_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 493,
	{ rv_op_vredxor_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 494,
	{ rv_op_vredminu_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 495,
	{ rv_op_vredmin_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 496,
	{ rv_op_vredmaxu_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 497,
	{ rv_op_vredmax_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 498,
	{ rv_op_vaaddu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 499,
	{ rv_op_vaadd_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 500,
	{ rv_op_vasubu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 501,
	{ rv_op_vasub_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 502,
	{ rv_op_vcompress_vv   ,0, 0, 0, 0, 0, 0, 1 },  // 503,
	{ rv_op_vmandnot_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 504,
	{ rv_op_vmand_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 505,
	{ rv_op_vmor_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 506,
	{ rv_op_vmxor_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 507,
	{ rv_op_vmornot_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 508,
	{ rv_op_vmnand_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 509,
	{ rv_op_vmnor_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 510,
	{ rv_op_vmxnor_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 511,
	{ rv_op_vdivu_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 512,
	{ rv_op_vdiv_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 513,
	{ rv_op_vremu_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 514,
	{ rv_op_vrem_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 515,
	{ rv_op_vmulhu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 516,
	{ rv_op_vmul_vv        ,0, 0, 0, 0, 0, 0, 1 },  // 517,
	{ rv_op_vmulhsu_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 518,
	{ rv_op_vmulh_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 519,
	{ rv_op_vmadd_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 520,
	{ rv_op_vnmsub_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 521,
	{ rv_op_vmacc_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 522,
	{ rv_op_vnmsac_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 523,
	{ rv_op_vwaddu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 524,
	{ rv_op_vwadd_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 525,
	{ rv_op_vwsubu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 526,
	{ rv_op_vwsub_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 527,
	{ rv_op_vwaddu_w_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 528,
	{ rv_op_vwadd_w_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 529,
	{ rv_op_vwsubu_w_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 530,
	{ rv_op_vwsub_w_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 531,
	{ rv_op_vwmulu_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 532,
	{ rv_op_vwmulsu_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 533,
	{ rv_op_vwmul_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 534,
	{ rv_op_vwmaccu_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 535,
	{ rv_op_vwmacc_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 536,
	{ rv_op_vwmaccus_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 537,
	{ rv_op_vwmaccsu_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 538,
	{ rv_op_vaaddu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 539,
	{ rv_op_vaadd_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 540,
	{ rv_op_vasubu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 541,
	{ rv_op_vasub_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 542,
	{ rv_op_vslide1up_vx   ,0, 0, 0, 0, 0, 0, 1 },  // 543,
	{ rv_op_vslide1down_vx ,0, 0, 0, 0, 0, 0, 1 },  // 544,
	{ rv_op_vdivu_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 545,
	{ rv_op_vdiv_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 546,
	{ rv_op_vremu_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 547,
	{ rv_op_vrem_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 548,
	{ rv_op_vmulhu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 549,
	{ rv_op_vmul_vx        ,0, 0, 0, 0, 0, 0, 1 },  // 550,
	{ rv_op_vmulhsu_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 551,
	{ rv_op_vmulh_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 552,
	{ rv_op_vmadd_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 553,
	{ rv_op_vnmsub_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 554,
	{ rv_op_vmacc_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 555,
	{ rv_op_vnmsac_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 556,
	{ rv_op_vwaddu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 557,
	{ rv_op_vwadd_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 558,
	{ rv_op_vwsubu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 559,
	{ rv_op_vwsub_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 560,
	{ rv_op_vwaddu_w_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 561,
	{ rv_op_vwadd_w_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 562,
	{ rv_op_vwsubu_w_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 563,
	{ rv_op_vwsub_w_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 564,
	{ rv_op_vwmulu_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 565,
	{ rv_op_vwmulsu_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 566,
	{ rv_op_vwmul_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 567,
	{ rv_op_vwmaccu_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 568,
	{ rv_op_vwmacc_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 569,
	{ rv_op_vwmaccus_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 570,
	{ rv_op_vwmaccsu_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 571,
	{ rv_op_vfadd_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 572,
	{ rv_op_vfredsum_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 573,
	{ rv_op_vfsub_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 574,
	{ rv_op_vfredosum_vv   ,0, 0, 0, 0, 0, 0, 1 },  // 575,
	{ rv_op_vfmin_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 576,
	{ rv_op_vfredmin_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 577,
	{ rv_op_vfmax_vv       ,0, 0, 0, 0, 0, 0, 1 },  // 578,
	{ rv_op_vfredmax_vv    ,0, 0, 0, 0, 0, 0, 1 },  // 579,
	{ rv_op_vfsgnj_vv      ,0, 0, 0, 0, 0, 0, 1 },  // 580,
	{ rv_op_vfsgnjn_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 581,
	{ rv_op_vfsgnjx_vv     ,0, 0, 0, 0, 0, 0, 1 },  // 582,
	{ rv_op_vmfeq_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 583,
	{ rv_op_vmfle_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 584,
	{ rv_op_vmflt_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 585,
	{ rv_op_vmfne_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 586,
	{ rv_op_vfdiv_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 587,
	{ rv_op_vfmul_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 590,
	{ rv_op_vfrsub_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 591,
	{ rv_op_vfmadd_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 592,
	{ rv_op_vfnmadd_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 593,
	{ rv_op_vfmsub_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 594,
	{ rv_op_vfnmsub_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 595,
	{ rv_op_vfmacc_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 596,
	{ rv_op_vfnmacc_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 597,
	{ rv_op_vfmsac_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 598,
	{ rv_op_vfnmsac_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 599,
	{ rv_op_vfwadd_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 600,
	{ rv_op_vfwredsum_vx   ,0, 0, 0, 0, 0, 0, 1 },  // 601,
	{ rv_op_vfwsub_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 602,
	{ rv_op_vfwredosum_vx  ,0, 0, 0, 0, 0, 0, 1 },  // 603,
	{ rv_op_vfwadd_wv      ,0, 0, 0, 0, 0, 0, 1 },  // 604,
	{ rv_op_vfwsub_wv      ,0, 0, 0, 0, 0, 0, 1 },  // 605,
	{ rv_op_vfwmul_vx      ,0, 0, 0, 0, 0, 0, 1 },  // 606,
	{ rv_op_vfdot_vx       ,0, 0, 0, 0, 0, 0, 1 },  // 607,
	{ rv_op_vfwmacc_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 608,
	{ rv_op_vfwnmacc_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 609,
	{ rv_op_vfwmsac_vx     ,0, 0, 0, 0, 0, 0, 1 },  // 610,
	{ rv_op_vfwnmsac_vx    ,0, 0, 0, 0, 0, 0, 1 },  // 611,
	{ rv_op_vfadd_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 612,
	{ rv_op_vfredsum_vf    ,0, 0, 0, 0, 0, 0, 1 },  // 613,
	{ rv_op_vfsub_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 614,
	{ rv_op_vfredosum_vf   ,0, 0, 0, 0, 0, 0, 1 },  // 615,
	{ rv_op_vfmin_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 616,
	{ rv_op_vfredmin_vf    ,0, 0, 0, 0, 0, 0, 1 },  // 617,
	{ rv_op_vfmax_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 618,
	{ rv_op_vfredmax_vf    ,0, 0, 0, 0, 0, 0, 1 },  // 619,
	{ rv_op_vfsgnj_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 620,
	{ rv_op_vfsgnjn_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 621,
	{ rv_op_vfsgnjx_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 622,
	{ rv_op_vfmv_vf        ,0, 0, 0, 0, 0, 0, 1 },  // 623,
	{ rv_op_vmfeq_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 624,
	{ rv_op_vmfle_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 625,
	{ rv_op_vmflt_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 626,
	{ rv_op_vmfne_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 627,
	{ rv_op_vmfgt_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 628,
	{ rv_op_vmfge_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 629,
	{ rv_op_vfdiv_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 630,
	{ rv_op_vfrdiv_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 631,
	{ rv_op_vfmul_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 632,
	{ rv_op_vfrsub_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 633,
	{ rv_op_vfmadd_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 634,
	{ rv_op_vfnmadd_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 635,
	{ rv_op_vfmsub_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 636,
	{ rv_op_vfnmsub_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 637,
	{ rv_op_vfmacc_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 638,
	{ rv_op_vfnmacc_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 639,
	{ rv_op_vfmsac_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 640,
	{ rv_op_vfnmsac_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 641,
	{ rv_op_vfwadd_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 642,
	{ rv_op_vfwredsum_vf   ,0, 0, 0, 0, 0, 0, 1 },  // 643,
	{ rv_op_vfwsub_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 644,
	{ rv_op_vfwredosum_vf  ,0, 0, 0, 0, 0, 0, 1 },  // 645,
	{ rv_op_vfwadd_wf      ,0, 0, 0, 0, 0, 0, 1 },  // 646,
	{ rv_op_vfwsub_wf      ,0, 0, 0, 0, 0, 0, 1 },  // 647,
	{ rv_op_vfwmul_vf      ,0, 0, 0, 0, 0, 0, 1 },  // 648,
	{ rv_op_vfdot_vf       ,0, 0, 0, 0, 0, 0, 1 },  // 649,
	{ rv_op_vfwmacc_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 650,
	{ rv_op_vfwnmacc_vf    ,0, 0, 0, 0, 0, 0, 1 },  // 651,
	{ rv_op_vfwmsac_vf     ,0, 0, 0, 0, 0, 0, 1 },  // 652,
	{ rv_op_vfwnmsac_vf    ,0, 0, 0, 0, 0, 0, 1 },  // 653,

    { rv_op_last,           0, 0, 0, 0, 0, 0, 0 }
};


#endif // __RISCV_META_H
