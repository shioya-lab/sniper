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
	rv_op_vle8ff_v = 330,
	rv_op_vle16ff_v = 331,
	rv_op_vle32ff_v = 332,
	rv_op_vle64ff_v = 333,
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
	rv_op_vluxei8_v = 374,
	rv_op_vsuxei8_v = 375,
	rv_op_vluxei16_v = 376,
	rv_op_vsuxei16_v = 377,
	rv_op_vluxei32_v = 378,
	rv_op_vsuxei32_v = 379,
	rv_op_vluxei64_v = 380,
	rv_op_vsuxei64_v = 381,
	rv_op_vloxei8_v = 382,
	rv_op_vsoxei8_v = 383,
	rv_op_vloxei16_v = 384,
	rv_op_vsoxei16_v = 385,
	rv_op_vloxei32_v = 386,
	rv_op_vsoxei32_v = 387,
	rv_op_vloxei64_v = 388,
	rv_op_vsoxei64_v = 389,
	rv_op_vlseg2e8_v = 390,
	rv_op_vsseg2e8_v = 391,
	rv_op_vlseg2e16_v = 392,
	rv_op_vsseg2e16_v = 393,
	rv_op_vlseg2e32_v = 394,
	rv_op_vsseg2e32_v = 395,
	rv_op_vlseg2e64_v = 396,
	rv_op_vsseg2e64_v = 397,
	rv_op_vlsseg2e8_v = 398,
	rv_op_vssseg2e8_v = 399,
	rv_op_vlsseg2e16_v = 400,
	rv_op_vssseg2e16_v = 401,
	rv_op_vlsseg2e32_v = 402,
	rv_op_vssseg2e32_v = 403,
	rv_op_vlsseg2e64_v = 404,
	rv_op_vssseg2e64_v = 405,
	rv_op_vluxseg2ei8_v = 406,
	rv_op_vsuxseg2ei8_v = 407,
	rv_op_vluxseg2ei16_v = 408,
	rv_op_vsuxseg2ei16_v = 409,
	rv_op_vluxseg2ei32_v = 410,
	rv_op_vsuxseg2ei32_v = 411,
	rv_op_vluxseg2ei64_v = 412,
	rv_op_vsuxseg2ei64_v = 413,
	rv_op_vloxseg2ei8_v = 414,
	rv_op_vsoxseg2ei8_v = 415,
	rv_op_vloxseg2ei16_v = 416,
	rv_op_vsoxseg2ei16_v = 417,
	rv_op_vloxseg2ei32_v = 418,
	rv_op_vsoxseg2ei32_v = 419,
	rv_op_vloxseg2ei64_v = 420,
	rv_op_vsoxseg2ei64_v = 421,
	rv_op_vlseg3e8_v = 422,
	rv_op_vsseg3e8_v = 423,
	rv_op_vlseg3e16_v = 424,
	rv_op_vsseg3e16_v = 425,
	rv_op_vlseg3e32_v = 426,
	rv_op_vsseg3e32_v = 427,
	rv_op_vlseg3e64_v = 428,
	rv_op_vsseg3e64_v = 429,
	rv_op_vlsseg3e8_v = 430,
	rv_op_vssseg3e8_v = 431,
	rv_op_vlsseg3e16_v = 432,
	rv_op_vssseg3e16_v = 433,
	rv_op_vlsseg3e32_v = 434,
	rv_op_vssseg3e32_v = 435,
	rv_op_vlsseg3e64_v = 436,
	rv_op_vssseg3e64_v = 437,
	rv_op_vluxseg3ei8_v = 438,
	rv_op_vsuxseg3ei8_v = 439,
	rv_op_vluxseg3ei16_v = 440,
	rv_op_vsuxseg3ei16_v = 441,
	rv_op_vluxseg3ei32_v = 442,
	rv_op_vsuxseg3ei32_v = 443,
	rv_op_vluxseg3ei64_v = 444,
	rv_op_vsuxseg3ei64_v = 445,
	rv_op_vloxseg3ei8_v = 446,
	rv_op_vsoxseg3ei8_v = 447,
	rv_op_vloxseg3ei16_v = 448,
	rv_op_vsoxseg3ei16_v = 449,
	rv_op_vloxseg3ei32_v = 450,
	rv_op_vsoxseg3ei32_v = 451,
	rv_op_vloxseg3ei64_v = 452,
	rv_op_vsoxseg3ei64_v = 453,
	rv_op_vlseg4e8_v = 454,
	rv_op_vsseg4e8_v = 455,
	rv_op_vlseg4e16_v = 456,
	rv_op_vsseg4e16_v = 457,
	rv_op_vlseg4e32_v = 458,
	rv_op_vsseg4e32_v = 459,
	rv_op_vlseg4e64_v = 460,
	rv_op_vsseg4e64_v = 461,
	rv_op_vlsseg4e8_v = 462,
	rv_op_vssseg4e8_v = 463,
	rv_op_vlsseg4e16_v = 464,
	rv_op_vssseg4e16_v = 465,
	rv_op_vlsseg4e32_v = 466,
	rv_op_vssseg4e32_v = 467,
	rv_op_vlsseg4e64_v = 468,
	rv_op_vssseg4e64_v = 469,
	rv_op_vluxseg4ei8_v = 470,
	rv_op_vsuxseg4ei8_v = 471,
	rv_op_vluxseg4ei16_v = 472,
	rv_op_vsuxseg4ei16_v = 473,
	rv_op_vluxseg4ei32_v = 474,
	rv_op_vsuxseg4ei32_v = 475,
	rv_op_vluxseg4ei64_v = 476,
	rv_op_vsuxseg4ei64_v = 477,
	rv_op_vloxseg4ei8_v = 478,
	rv_op_vsoxseg4ei8_v = 479,
	rv_op_vloxseg4ei16_v = 480,
	rv_op_vsoxseg4ei16_v = 481,
	rv_op_vloxseg4ei32_v = 482,
	rv_op_vsoxseg4ei32_v = 483,
	rv_op_vloxseg4ei64_v = 484,
	rv_op_vsoxseg4ei64_v = 485,
	rv_op_vlseg5e8_v = 486,
	rv_op_vsseg5e8_v = 487,
	rv_op_vlseg5e16_v = 488,
	rv_op_vsseg5e16_v = 489,
	rv_op_vlseg5e32_v = 490,
	rv_op_vsseg5e32_v = 491,
	rv_op_vlseg5e64_v = 492,
	rv_op_vsseg5e64_v = 493,
	rv_op_vlsseg5e8_v = 494,
	rv_op_vssseg5e8_v = 495,
	rv_op_vlsseg5e16_v = 496,
	rv_op_vssseg5e16_v = 497,
	rv_op_vlsseg5e32_v = 498,
	rv_op_vssseg5e32_v = 499,
	rv_op_vlsseg5e64_v = 500,
	rv_op_vssseg5e64_v = 501,
	rv_op_vluxseg5ei8_v = 502,
	rv_op_vsuxseg5ei8_v = 503,
	rv_op_vluxseg5ei16_v = 504,
	rv_op_vsuxseg5ei16_v = 505,
	rv_op_vluxseg5ei32_v = 506,
	rv_op_vsuxseg5ei32_v = 507,
	rv_op_vluxseg5ei64_v = 508,
	rv_op_vsuxseg5ei64_v = 509,
	rv_op_vloxseg5ei8_v = 510,
	rv_op_vsoxseg5ei8_v = 511,
	rv_op_vloxseg5ei16_v = 512,
	rv_op_vsoxseg5ei16_v = 513,
	rv_op_vloxseg5ei32_v = 514,
	rv_op_vsoxseg5ei32_v = 515,
	rv_op_vloxseg5ei64_v = 516,
	rv_op_vsoxseg5ei64_v = 517,
	rv_op_vlseg6e8_v = 518,
	rv_op_vsseg6e8_v = 519,
	rv_op_vlseg6e16_v = 520,
	rv_op_vsseg6e16_v = 521,
	rv_op_vlseg6e32_v = 522,
	rv_op_vsseg6e32_v = 523,
	rv_op_vlseg6e64_v = 524,
	rv_op_vsseg6e64_v = 525,
	rv_op_vlsseg6e8_v = 526,
	rv_op_vssseg6e8_v = 527,
	rv_op_vlsseg6e16_v = 528,
	rv_op_vssseg6e16_v = 529,
	rv_op_vlsseg6e32_v = 530,
	rv_op_vssseg6e32_v = 531,
	rv_op_vlsseg6e64_v = 532,
	rv_op_vssseg6e64_v = 533,
	rv_op_vluxseg6ei8_v = 534,
	rv_op_vsuxseg6ei8_v = 535,
	rv_op_vluxseg6ei16_v = 536,
	rv_op_vsuxseg6ei16_v = 537,
	rv_op_vluxseg6ei32_v = 538,
	rv_op_vsuxseg6ei32_v = 539,
	rv_op_vluxseg6ei64_v = 540,
	rv_op_vsuxseg6ei64_v = 541,
	rv_op_vloxseg6ei8_v = 542,
	rv_op_vsoxseg6ei8_v = 543,
	rv_op_vloxseg6ei16_v = 544,
	rv_op_vsoxseg6ei16_v = 545,
	rv_op_vloxseg6ei32_v = 546,
	rv_op_vsoxseg6ei32_v = 547,
	rv_op_vloxseg6ei64_v = 548,
	rv_op_vsoxseg6ei64_v = 549,
	rv_op_vlseg7e8_v = 550,
	rv_op_vsseg7e8_v = 551,
	rv_op_vlseg7e16_v = 552,
	rv_op_vsseg7e16_v = 553,
	rv_op_vlseg7e32_v = 554,
	rv_op_vsseg7e32_v = 555,
	rv_op_vlseg7e64_v = 556,
	rv_op_vsseg7e64_v = 557,
	rv_op_vlsseg7e8_v = 558,
	rv_op_vssseg7e8_v = 559,
	rv_op_vlsseg7e16_v = 560,
	rv_op_vssseg7e16_v = 561,
	rv_op_vlsseg7e32_v = 562,
	rv_op_vssseg7e32_v = 563,
	rv_op_vlsseg7e64_v = 564,
	rv_op_vssseg7e64_v = 565,
	rv_op_vluxseg7ei8_v = 566,
	rv_op_vsuxseg7ei8_v = 567,
	rv_op_vluxseg7ei16_v = 568,
	rv_op_vsuxseg7ei16_v = 569,
	rv_op_vluxseg7ei32_v = 570,
	rv_op_vsuxseg7ei32_v = 571,
	rv_op_vluxseg7ei64_v = 572,
	rv_op_vsuxseg7ei64_v = 573,
	rv_op_vloxseg7ei8_v = 574,
	rv_op_vsoxseg7ei8_v = 575,
	rv_op_vloxseg7ei16_v = 576,
	rv_op_vsoxseg7ei16_v = 577,
	rv_op_vloxseg7ei32_v = 578,
	rv_op_vsoxseg7ei32_v = 579,
	rv_op_vloxseg7ei64_v = 580,
	rv_op_vsoxseg7ei64_v = 581,
	rv_op_vlseg8e8_v = 582,
	rv_op_vsseg8e8_v = 583,
	rv_op_vlseg8e16_v = 584,
	rv_op_vsseg8e16_v = 585,
	rv_op_vlseg8e32_v = 586,
	rv_op_vsseg8e32_v = 587,
	rv_op_vlseg8e64_v = 588,
	rv_op_vsseg8e64_v = 589,
	rv_op_vlsseg8e8_v = 590,
	rv_op_vssseg8e8_v = 591,
	rv_op_vlsseg8e16_v = 592,
	rv_op_vssseg8e16_v = 593,
	rv_op_vlsseg8e32_v = 594,
	rv_op_vssseg8e32_v = 595,
	rv_op_vlsseg8e64_v = 596,
	rv_op_vssseg8e64_v = 597,
	rv_op_vluxseg8ei8_v = 598,
	rv_op_vsuxseg8ei8_v = 599,
	rv_op_vluxseg8ei16_v = 600,
	rv_op_vsuxseg8ei16_v = 601,
	rv_op_vluxseg8ei32_v = 602,
	rv_op_vsuxseg8ei32_v = 603,
	rv_op_vluxseg8ei64_v = 604,
	rv_op_vsuxseg8ei64_v = 605,
	rv_op_vloxseg8ei8_v = 606,
	rv_op_vsoxseg8ei8_v = 607,
	rv_op_vloxseg8ei16_v = 608,
	rv_op_vsoxseg8ei16_v = 609,
	rv_op_vloxseg8ei32_v = 610,
	rv_op_vsoxseg8ei32_v = 611,
	rv_op_vloxseg8ei64_v = 612,
	rv_op_vsoxseg8ei64_v = 613,
	rv_op_vadd_vv = 614,
	rv_op_vsub_vv = 615,
	rv_op_vminu_vv = 616,
	rv_op_vmin_vv = 617,
	rv_op_vmaxu_vv = 618,
	rv_op_vmax_vv = 619,
	rv_op_vand_vv = 620,
	rv_op_vor_vv = 621,
	rv_op_vxor_vv = 622,
	rv_op_vrgather_vv = 623,
	rv_op_vadc_vv = 624,
	rv_op_vmadc_vv = 625,
	rv_op_vsbc_vv = 626,
	rv_op_vmsbc_vv = 627,
	rv_op_vmerge_vvm = 628,
	rv_op_vmseq_vv = 629,
	rv_op_vmsne_vv = 630,
	rv_op_vmsltu_vv = 631,
	rv_op_vmslt_vv = 632,
	rv_op_vmsleu_vv = 633,
	rv_op_vmsle_vv = 634,
	rv_op_vsaddu_vv = 635,
	rv_op_vsadd_vv = 636,
	rv_op_vssubu_vv = 637,
	rv_op_vssub_vv = 638,
	rv_op_vsll_vv = 639,
	rv_op_vsmul_vv = 640,
	rv_op_vsrl_vv = 641,
	rv_op_vsra_vv = 642,
	rv_op_vssrl_vv = 643,
	rv_op_vssra_vv = 644,
	rv_op_vnsrl_vv = 645,
	rv_op_vnsra_vv = 646,
	rv_op_vnclipu_vv = 647,
	rv_op_vnclip_vv = 648,
	rv_op_vwredsumu_vv = 649,
	rv_op_vwredsum_vv = 650,
	rv_op_vdotu_vv = 651,
	rv_op_vdot_vv = 652,
	rv_op_vqmaccu_vv = 653,
	rv_op_vqmacc_vv = 654,
	rv_op_vqmaccus_vv = 655,
	rv_op_vqmaccsu_vv = 656,
	rv_op_vadd_vx = 657,
	rv_op_vsub_vx = 658,
	rv_op_vrsub_vx = 659,
	rv_op_vminu_vx = 660,
	rv_op_vmin_vx = 661,
	rv_op_vmaxu_vx = 662,
	rv_op_vmax_vx = 663,
	rv_op_vand_vx = 664,
	rv_op_vor_vx = 665,
	rv_op_vxor_vx = 666,
	rv_op_vrgather_vx = 667,
	rv_op_vslideup_vx = 668,
	rv_op_vslidedown_vx = 669,
	rv_op_vadc_vx = 670,
	rv_op_vmadc_vx = 671,
	rv_op_vsbc_vx = 672,
	rv_op_vmsbc_vx = 673,
	rv_op_vmerge_vxm = 674,
	rv_op_vmseq_vx = 675,
	rv_op_vmsne_vx = 676,
	rv_op_vmsltu_vx = 677,
	rv_op_vmslt_vx = 678,
	rv_op_vmsleu_vx = 679,
	rv_op_vmsle_vx = 680,
	rv_op_vmsgtu_vx = 681,
	rv_op_vmsgt_vx = 682,
	rv_op_vsaddu_vx = 683,
	rv_op_vsadd_vx = 684,
	rv_op_vssubu_vx = 685,
	rv_op_vssub_vx = 686,
	rv_op_vsll_vx = 687,
	rv_op_vsmul_vx = 688,
	rv_op_vsrl_vx = 689,
	rv_op_vsra_vx = 690,
	rv_op_vssrl_vx = 691,
	rv_op_vssra_vx = 692,
	rv_op_vnsrl_vx = 693,
	rv_op_vnsra_vx = 694,
	rv_op_vnclipu_vx = 695,
	rv_op_vnclip_vx = 696,
	rv_op_vwredsumu_vx = 697,
	rv_op_vwredsum_vx = 698,
	rv_op_vdotu_vx = 699,
	rv_op_vdot_vx = 700,
	rv_op_vqmaccu_vx = 701,
	rv_op_vqmacc_vx = 702,
	rv_op_vqmaccus_vx = 703,
	rv_op_vqmaccsu_vx = 704,
	rv_op_vadd_vi = 705,
	rv_op_vrsub_vi = 706,
	rv_op_vand_vi = 707,
	rv_op_vor_vi = 708,
	rv_op_vxor_vi = 709,
	rv_op_vrgather_vi = 710,
	rv_op_vslideup_vi = 711,
	rv_op_vslidedown_vi = 712,
	rv_op_vadc_vi = 713,
	rv_op_vmadc_vi = 714,
	rv_op_vmv_vi = 715,
	rv_op_vmseq_vi = 716,
	rv_op_vmsne_vi = 717,
	rv_op_vmsleu_vi = 718,
	rv_op_vmsle_vi = 719,
	rv_op_vmsgtu_vi = 720,
	rv_op_vmsgt_vi = 721,
	rv_op_vmv1r = 722,
	rv_op_vmv2r = 723,
	rv_op_vmv4r = 724,
	rv_op_vmv8r = 725,
	rv_op_vsaddu_vi = 726,
	rv_op_vsadd_vi = 727,
	rv_op_vsll_vi = 728,
	rv_op_vsrl_vi = 729,
	rv_op_vsra_vi = 730,
	rv_op_vssrl_vi = 731,
	rv_op_vssra_vi = 732,
	rv_op_vnsrl_vi = 733,
	rv_op_vnsra_vi = 734,
	rv_op_vnclipu_vi = 735,
	rv_op_vnclip_vi = 736,
	rv_op_vredsum_vv = 737,
	rv_op_vredand_vv = 738,
	rv_op_vredor_vv = 739,
	rv_op_vredxor_vv = 740,
	rv_op_vredminu_vv = 741,
	rv_op_vredmin_vv = 742,
	rv_op_vredmaxu_vv = 743,
	rv_op_vredmax_vv = 744,
	rv_op_vaaddu_vv = 745,
	rv_op_vaadd_vv = 746,
	rv_op_vasubu_vv = 747,
	rv_op_vasub_vv = 748,
	rv_op_vmv_x_s = 749,
	rv_op_vpopc_m = 750,
	rv_op_vfirst_m = 751,
	rv_op_vmv_s_x = 752,
	rv_op_vzext_vf8 = 753,
	rv_op_vsext_vf8 = 754,
	rv_op_vzext_vf4 = 755,
	rv_op_vsext_vf4 = 756,
	rv_op_vzext_vf2 = 757,
	rv_op_vsext_vf2 = 758,
	rv_op_vmsbf_m = 759,
	rv_op_vmsof_m = 760,
	rv_op_vmsif_m = 761,
	rv_op_viota_m = 762,
	rv_op_vid_v = 763,
	rv_op_vcompress_vv = 764,
	rv_op_vmandnot_vv = 765,
	rv_op_vmand_vv = 766,
	rv_op_vmor_vv = 767,
	rv_op_vmxor_vv = 768,
	rv_op_vmornot_vv = 769,
	rv_op_vmnand_vv = 770,
	rv_op_vmnor_vv = 771,
	rv_op_vmxnor_vv = 772,
	rv_op_vdivu_vv = 773,
	rv_op_vdiv_vv = 774,
	rv_op_vremu_vv = 775,
	rv_op_vrem_vv = 776,
	rv_op_vmulhu_vv = 777,
	rv_op_vmul_vv = 778,
	rv_op_vmulhsu_vv = 779,
	rv_op_vmulh_vv = 780,
	rv_op_vmadd_vv = 781,
	rv_op_vnmsub_vv = 782,
	rv_op_vmacc_vv = 783,
	rv_op_vnmsac_vv = 784,
	rv_op_vwaddu_vv = 785,
	rv_op_vwadd_vv = 786,
	rv_op_vwsubu_vv = 787,
	rv_op_vwsub_vv = 788,
	rv_op_vwaddu_w_vv = 789,
	rv_op_vwadd_w_vv = 790,
	rv_op_vwsubu_w_vv = 791,
	rv_op_vwsub_w_vv = 792,
	rv_op_vwmulu_vv = 793,
	rv_op_vwmulsu_vv = 794,
	rv_op_vwmul_vv = 795,
	rv_op_vwmaccu_vv = 796,
	rv_op_vwmacc_vv = 797,
	rv_op_vwmaccus_vv = 798,
	rv_op_vwmaccsu_vv = 799,
	rv_op_vaaddu_vx = 800,
	rv_op_vaadd_vx = 801,
	rv_op_vasubu_vx = 802,
	rv_op_vasub_vx = 803,
	rv_op_vslide1up_vx = 804,
	rv_op_vslide1down_vx = 805,
	rv_op_vdivu_vx = 806,
	rv_op_vdiv_vx = 807,
	rv_op_vremu_vx = 808,
	rv_op_vrem_vx = 809,
	rv_op_vmulhu_vx = 810,
	rv_op_vmul_vx = 811,
	rv_op_vmulhsu_vx = 812,
	rv_op_vmulh_vx = 813,
	rv_op_vmadd_vx = 814,
	rv_op_vnmsub_vx = 815,
	rv_op_vmacc_vx = 816,
	rv_op_vnmsac_vx = 817,
	rv_op_vwaddu_vx = 818,
	rv_op_vwadd_vx = 819,
	rv_op_vwsubu_vx = 820,
	rv_op_vwsub_vx = 821,
	rv_op_vwaddu_w_vx = 822,
	rv_op_vwadd_w_vx = 823,
	rv_op_vwsubu_w_vx = 824,
	rv_op_vwsub_w_vx = 825,
	rv_op_vwmulu_vx = 826,
	rv_op_vwmulsu_vx = 827,
	rv_op_vwmul_vx = 828,
	rv_op_vwmaccu_vx = 829,
	rv_op_vwmacc_vx = 830,
	rv_op_vwmaccus_vx = 831,
	rv_op_vwmaccsu_vx = 832,
	rv_op_vfadd_vv = 833,
	rv_op_vfredsum_vv = 834,
	rv_op_vfsub_vv = 835,
	rv_op_vfredosum_vv = 836,
	rv_op_vfmin_vv = 837,
	rv_op_vfredmin_vv = 838,
	rv_op_vfmax_vv = 839,
	rv_op_vfredmax_vv = 840,
	rv_op_vfsgnj_vv = 841,
	rv_op_vfsgnjn_vv = 842,
	rv_op_vfsgnjx_vv = 843,
	rv_op_vfslide1up_vf = 844,
	rv_op_vfslide1down_vf = 845,
	rv_op_vmfeq_vx = 846,
	rv_op_vmfle_vx = 847,
	rv_op_vmflt_vx = 848,
	rv_op_vmfne_vx = 849,
	rv_op_vfdiv_vx = 850,
	rv_op_vfcvt_xu_f_v = 851,
	rv_op_vfcvt_x_f_v = 852,
	rv_op_vfcvt_f_xu_v = 853,
	rv_op_vfcvt_f_x_v = 854,
	rv_op_vfcvt_rtz_xu_f_v = 855,
	rv_op_vfcvt_rtz_x_f_v = 856,
	rv_op_vfwcvt_xu_f_v = 857,
	rv_op_vfwcvt_x_f_v = 858,
	rv_op_vfwcvt_f_xu_v = 859,
	rv_op_vfwcvt_f_x_v = 860,
	rv_op_vfwcvt_f_f_v = 861,
	rv_op_vfwcvt_rtz_xu_f_v = 862,
	rv_op_vfwcvt_rtz_x_f_v = 863,
	rv_op_vfncvt_xu_f_w = 864,
	rv_op_vfncvt_x_f_w = 865,
	rv_op_vfncvt_f_xu_w = 866,
	rv_op_vfncvt_f_x_w = 867,
	rv_op_vfncvt_f_f_w = 868,
	rv_op_vfncvt_rod_f_f_w = 869,
	rv_op_vfncvt_rtz_xu_f_w = 870,
	rv_op_vfncvt_rtz_x_f_w = 871,
	rv_op_vfmul_vv = 872,
	rv_op_vfrsub_vv = 873,
	rv_op_vfmadd_vv = 874,
	rv_op_vfnmadd_vv = 875,
	rv_op_vfmsub_vv = 876,
	rv_op_vfnmsub_vv = 877,
	rv_op_vfmacc_vv = 878,
	rv_op_vfnmacc_vv = 879,
	rv_op_vfmsac_vv = 880,
	rv_op_vfnmsac_vv = 881,
	rv_op_vfwadd_vv = 882,
	rv_op_vfwredsum_vv = 883,
	rv_op_vfwsub_vv = 884,
	rv_op_vfwredosum_vv = 885,
	rv_op_vfwadd_wv = 886,
	rv_op_vfwsub_wv = 887,
	rv_op_vfwmul_vv = 888,
	rv_op_vfdot_vv = 889,
	rv_op_vfwmacc_vv = 890,
	rv_op_vfwnmacc_vv = 891,
	rv_op_vfwmsac_vv = 892,
	rv_op_vfwnmsac_vv = 893,
	rv_op_vfadd_vf = 894,
	rv_op_vfredsum_vf = 895,
	rv_op_vfsub_vf = 896,
	rv_op_vfredosum_vf = 897,
	rv_op_vfmin_vf = 898,
	rv_op_vfredmin_vf = 899,
	rv_op_vfmax_vf = 900,
	rv_op_vfredmax_vf = 901,
	rv_op_vfsgnj_vf = 902,
	rv_op_vfsgnjn_vf = 903,
	rv_op_vfsgnjx_vf = 904,
	rv_op_vfmv_s_f = 905,
	rv_op_vfmv_f_s = 906,
	rv_op_vfmv_v_f = 907,
	rv_op_vmfeq_vf = 908,
	rv_op_vmfle_vf = 909,
	rv_op_vmflt_vf = 910,
	rv_op_vmfne_vf = 911,
	rv_op_vmfgt_vf = 912,
	rv_op_vmfge_vf = 913,
	rv_op_vfdiv_vf = 914,
	rv_op_vfrdiv_vf = 915,
	rv_op_vfmul_vf = 916,
	rv_op_vfrsub_vf = 917,
	rv_op_vfmadd_vf = 918,
	rv_op_vfnmadd_vf = 919,
	rv_op_vfmsub_vf = 920,
	rv_op_vfnmsub_vf = 921,
	rv_op_vfmacc_vf = 922,
	rv_op_vfnmacc_vf = 923,
	rv_op_vfmsac_vf = 924,
	rv_op_vfnmsac_vf = 925,
	rv_op_vfwadd_vf = 926,
	rv_op_vfwredsum_vf = 927,
	rv_op_vfwsub_vf = 928,
	rv_op_vfwredosum_vf = 929,
	rv_op_vfwadd_wf = 930,
	rv_op_vfwsub_wf = 931,
	rv_op_vfwmul_vf = 932,
	rv_op_vfdot_vf = 933,
	rv_op_vfwmacc_vf = 934,
	rv_op_vfwnmacc_vf = 935,
	rv_op_vfwmsac_vf = 936,
	rv_op_vfwnmsac_vf = 937,
	rv_op_vfsqrt_v = 938,
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
  bool is_vector;
};

const riscvinstr instrlist[] = {
    // opcode, has_alu, has_mul, has_div, has_fpu, has_fdiv, has_ifpu, is_memory, is_vector
/*  0 */ { rv_op_illegal           , 0, 0, 0, 0, 0, 0, 0, 0 },  //
/*  1 */ { rv_op_lui               , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  2 */ { rv_op_auipc             , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  3 */ { rv_op_jal               , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  4 */ { rv_op_jalr              , 1, 0, 0, 0, 0, 0, 0, 0 },  //    irect
/*  5 */ { rv_op_beq               , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  6 */ { rv_op_bne               , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  7 */ { rv_op_blt               , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  8 */ { rv_op_bge               , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/*  9 */ { rv_op_bltu              , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/* 10 */ { rv_op_bgeu              , 1, 0, 0, 0, 0, 0, 0, 0 },  //
/* 11 */ { rv_op_lb                , 0, 0, 0, 0, 0, 0, 1, 0 },  //
/* 12 */ { rv_op_lh                , 0, 0, 0, 0, 0, 0, 1, 0 },  //  1
/* 13 */ { rv_op_lw                , 0, 0, 0, 0, 0, 0, 1, 0 },  //  2
/* 14 */ { rv_op_lbu               , 0, 0, 0, 0, 0, 0, 1, 0 },  //  3
/* 15 */ { rv_op_lhu               , 0, 0, 0, 0, 0, 0, 1, 0 },  //  4
/* 16 */ { rv_op_sb                , 0, 0, 0, 0, 0, 0, 1, 0 },  //  5
/* 17 */ { rv_op_sh                , 0, 0, 0, 0, 0, 0, 1, 0 },  //  6
/* 18 */ { rv_op_sw                , 0, 0, 0, 0, 0, 0, 1, 0 },  //  7
/* 19 */ { rv_op_addi              , 1, 0, 0, 0, 0, 0, 0, 0 },  //  8
/* 20 */ { rv_op_slti              , 1, 0, 0, 0, 0, 0, 0, 0 },  //  9
/* 21 */ { rv_op_sltiu             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 10
/* 22 */ { rv_op_xori              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 11
/* 23 */ { rv_op_ori               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 12
/* 24 */ { rv_op_andi              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 13
/* 25 */ { rv_op_slli              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 14
/* 26 */ { rv_op_srli              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 15
/* 27 */ { rv_op_srai              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 16
/* 28 */ { rv_op_add               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 17
/* 29 */ { rv_op_sub               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 18
/* 30 */ { rv_op_sll               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 19
/* 31 */ { rv_op_slt               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 20
/* 32 */ { rv_op_sltu              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 21
/* 33 */ { rv_op_xor               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 22
/* 34 */ { rv_op_srl               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 23
/* 35 */ { rv_op_sra               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 24
/* 36 */ { rv_op_or                , 1, 0, 0, 0, 0, 0, 0, 0 },  // 25
/* 37 */ { rv_op_and               , 1, 0, 0, 0, 0, 0, 0, 0 },  // 26
/* 38 */ { rv_op_fence             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 27
/* 39 */ { rv_op_fence_i           , 1, 0, 0, 0, 0, 0, 0, 0 },  // 28
/* 40 */ { rv_op_lwu               , 0, 0, 0, 0, 0, 0, 1, 0 },  // 29
/* 41 */ { rv_op_ld                , 0, 0, 0, 0, 0, 0, 1, 0 },  // 30
/* 42 */ { rv_op_sd                , 0, 0, 0, 0, 0, 0, 1, 0 },  // 31
/* 43 */ { rv_op_addiw             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 32
/* 44 */ { rv_op_slliw             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 33
/* 45 */ { rv_op_srliw             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 34
/* 46 */ { rv_op_sraiw             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 35
/* 47 */ { rv_op_addw              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 36
/* 48 */ { rv_op_subw              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 37
/* 49 */ { rv_op_sllw              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 38
/* 50 */ { rv_op_srlw              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 39
/* 51 */ { rv_op_sraw              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 40
/* 52 */ { rv_op_ldu               , 0, 0, 0, 0, 0, 0, 0, 0 },  // 41
/* 53 */ { rv_op_lq                , 0, 0, 0, 0, 0, 0, 0, 0 },  // 42
/* 54 */ { rv_op_sq                , 0, 0, 0, 0, 0, 0, 0, 0 },  // 43
/* 55 */ { rv_op_addid             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 44
/* 56 */ { rv_op_sllid             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 45
/* 57 */ { rv_op_srlid             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 46
/* 58 */ { rv_op_sraid             , 1, 0, 0, 0, 0, 0, 0, 0 },  // 47
/* 59 */ { rv_op_addd              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 48
/* 60 */ { rv_op_subd              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 49
/* 61 */ { rv_op_slld              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 50
/* 62 */ { rv_op_srld              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 51
/* 63 */ { rv_op_srad              , 1, 0, 0, 0, 0, 0, 0, 0 },  // 52
/* 64 */ { rv_op_mul               , 1, 1, 0, 0, 0, 0, 0, 0 },  // 53     iply
/* 65 */ { rv_op_mulh              , 1, 1, 0, 0, 0, 0, 0, 0 },  // 54     iply
/* 66 */ { rv_op_mulhsu            , 1, 1, 0, 0, 0, 0, 0, 0 },  // 55     iply
/* 67 */ { rv_op_mulhu             , 1, 1, 0, 0, 0, 0, 0, 0 },  // 56     iply
/* 68 */ { rv_op_div               , 1, 0, 1, 0, 0, 0, 0, 0 },  // 57     de
/* 69 */ { rv_op_divu              , 1, 0, 1, 0, 0, 0, 0, 0 },  // 58     de
/* 70 */ { rv_op_rem               , 1, 0, 1, 0, 0, 0, 0, 0 },  // 59     de
/* 71 */ { rv_op_remu              , 1, 0, 1, 0, 0, 0, 0, 0 },  // 60     de
/* 72 */ { rv_op_mulw              , 1, 1, 0, 0, 0, 0, 0, 0 },  // 61     iply
/* 73 */ { rv_op_divw              , 1, 0, 1, 0, 0, 0, 0, 0 },  // 62     de
/* 74 */ { rv_op_divuw             , 1, 0, 1, 0, 0, 0, 0, 0 },  // 63     de
/* 75 */ { rv_op_remw              , 1, 0, 1, 0, 0, 0, 0, 0 },  // 64     de
/* 76 */ { rv_op_remuw             , 1, 0, 1, 0, 0, 0, 0, 0 },  // 65     de
/* 77 */ { rv_op_muld              , 1, 1, 0, 0, 0, 0, 0, 0 },  // 66
/* 78 */ { rv_op_divd              , 1, 0, 1, 0, 0, 0, 0, 0 },  // 67
/* 79 */ { rv_op_divud             , 1, 0, 1, 0, 0, 0, 0, 0 },  // 68
/* 80 */ { rv_op_remd              , 1, 0, 1, 0, 0, 0, 0, 0 },  // 69
/* 81 */ { rv_op_remud             , 1, 0, 1, 0, 0, 0, 0, 0 },  // 70
/* 82 */ { rv_op_lr_w              , 1, 0, 0, 0, 0, 0, 1, 0 },  // 71
/* 83 */ { rv_op_sc_w              , 1, 0, 0, 0, 0, 0, 1, 0 },  // 72
/* 84 */ { rv_op_amoswap_w         , 1, 0, 0, 0, 0, 0, 1, 0 },  // 73
/* 85 */ { rv_op_amoadd_w          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 74
/* 86 */ { rv_op_amoxor_w          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 75
/* 87 */ { rv_op_amoor_w           , 1, 0, 0, 0, 0, 0, 0, 0 },  // 76
/* 88 */ { rv_op_amoand_w          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 77
/* 89 */ { rv_op_amomin_w          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 78
/* 90 */ { rv_op_amomax_w          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 79
/* 91 */ { rv_op_amominu_w         , 1, 0, 0, 0, 0, 0, 0, 0 },  // 80
/* 92 */ { rv_op_amomaxu_w         , 1, 0, 0, 0, 0, 0, 0, 0 },  // 81
/* 93 */ { rv_op_lr_d              , 1, 0, 0, 0, 0, 0, 1, 0 },  // 82
/* 94 */ { rv_op_sc_d              , 1, 0, 0, 0, 0, 0, 1, 0 },  // 83
/* 95 */ { rv_op_amoswap_d         , 1, 0, 0, 0, 0, 0, 1, 0 },  // 84
/* 96 */ { rv_op_amoadd_d          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 85
/* 97 */ { rv_op_amoxor_d          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 86
/* 98 */ { rv_op_amoor_d           , 1, 0, 0, 0, 0, 0, 0, 0 },  // 87
/* 99 */ { rv_op_amoand_d          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 88
/*100 */ { rv_op_amomin_d          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 89
/*101 */ { rv_op_amomax_d          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 90
/*102 */ { rv_op_amominu_d         , 1, 0, 0, 0, 0, 0, 0, 0 },  // 91
/*103 */ { rv_op_amomaxu_d         , 1, 0, 0, 0, 0, 0, 0, 0 },  // 92
/*104 */ { rv_op_lr_q              , 1, 0, 0, 0, 0, 0, 1, 0 },  // 93
/*105 */ { rv_op_sc_q              , 1, 0, 0, 0, 0, 0, 1, 0 },  // 94
/*106 */ { rv_op_amoswap_q         , 1, 0, 0, 0, 0, 0, 1, 0 },  // 95
/*107 */ { rv_op_amoadd_q          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 96
/*108 */ { rv_op_amoxor_q          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 97
/*109 */ { rv_op_amoor_q           , 1, 0, 0, 0, 0, 0, 0, 0 },  // 98
/*110 */ { rv_op_amoand_q          , 1, 0, 0, 0, 0, 0, 0, 0 },  // 99
/*111 */ { rv_op_amomin_q          , 1, 0, 0, 0, 0, 0, 0, 0 },  //100
/*112 */ { rv_op_amomax_q          , 1, 0, 0, 0, 0, 0, 0, 0 },  //101
/*113 */ { rv_op_amominu_q         , 1, 0, 0, 0, 0, 0, 0, 0 },  //102
/*114 */ { rv_op_amomaxu_q         , 1, 0, 0, 0, 0, 0, 0, 0 },  //103
/*115 */ { rv_op_ecall             , 1, 0, 0, 0, 0, 0, 0, 0 },  //104
/*116 */ { rv_op_ebreak            , 1, 0, 0, 0, 0, 0, 0, 0 },  //105
/*117 */ { rv_op_uret              , 1, 0, 0, 0, 0, 0, 0, 0 },  //106
/*118 */ { rv_op_sret              , 1, 0, 0, 0, 0, 0, 0, 0 },  //107
/*119 */ { rv_op_hret              , 1, 0, 0, 0, 0, 0, 0, 0 },  //108
/*120 */ { rv_op_mret              , 1, 0, 0, 0, 0, 0, 0, 0 },  //109
/*121 */ { rv_op_dret              , 1, 0, 0, 0, 0, 0, 0, 0 },  //110
/*122 */ { rv_op_sfence_vm         , 1, 0, 0, 0, 0, 0, 0, 0 },  //111
/*123 */ { rv_op_sfence_vma        , 1, 0, 0, 0, 0, 0, 0, 0 },  //112
/*124 */ { rv_op_wfi               , 1, 0, 0, 0, 0, 0, 0, 0 },  //113
/*125 */ { rv_op_csrrw             , 1, 0, 0, 0, 0, 0, 0, 0 },  //114
/*126 */ { rv_op_csrrs             , 1, 0, 0, 0, 0, 0, 0, 0 },  //115
/*127 */ { rv_op_csrrc             , 1, 0, 0, 0, 0, 0, 0, 0 },  //116
/*128 */ { rv_op_csrrwi            , 1, 0, 0, 0, 0, 0, 0, 0 },  //117
/*129 */ { rv_op_csrrsi            , 1, 0, 0, 0, 0, 0, 0, 0 },  //118
/*130 */ { rv_op_csrrci            , 1, 0, 0, 0, 0, 0, 0, 0 },  //119
/*131 */ { rv_op_flw               , 1, 0, 0, 1, 0, 0, 1, 0 },  //120
/*132 */ { rv_op_fsw               , 1, 0, 0, 1, 0, 0, 1, 0 },  //121     e
/*133 */ { rv_op_fmadd_s           , 1, 0, 0, 1, 0, 0, 0, 0 },  //122
/*134 */ { rv_op_fmsub_s           , 1, 0, 0, 1, 0, 0, 0, 0 },  //123
/*135 */ { rv_op_fnmsub_s          , 1, 0, 0, 1, 0, 0, 0, 0 },  //124
/*136 */ { rv_op_fnmadd_s          , 1, 0, 0, 1, 0, 0, 0, 0 },  //125
/*137 */ { rv_op_fadd_s            , 1, 0, 0, 1, 0, 0, 0, 0 },  //126
/*138 */ { rv_op_fsub_s            , 1, 0, 0, 1, 0, 0, 0, 0 },  //127
/*139 */ { rv_op_fmul_s            , 1, 0, 0, 1, 1, 0, 0, 0 },  //128
/*140 */ { rv_op_fdiv_s            , 1, 0, 0, 1, 1, 0, 0, 0 },  //129
/*141 */ { rv_op_fsgnj_s           , 1, 0, 0, 1, 0, 0, 0, 0 },  //130
/*142 */ { rv_op_fsgnjn_s          , 1, 0, 0, 1, 0, 0, 0, 0 },  //131
/*143 */ { rv_op_fsgnjx_s          , 1, 0, 0, 1, 0, 0, 0, 0 },  //132
/*144 */ { rv_op_fmin_s            , 1, 0, 0, 1, 0, 0, 0, 0 },  //133
/*145 */ { rv_op_fmax_s            , 1, 0, 0, 1, 0, 0, 0, 0 },  //134
/*146 */ { rv_op_fsqrt_s           , 1, 0, 0, 1, 0, 0, 0, 0 },  //135     t
/*147 */ { rv_op_fle_s             , 1, 0, 0, 1, 0, 0, 0, 0 },  //136
/*148 */ { rv_op_flt_s             , 1, 0, 0, 1, 0, 0, 0, 0 },  //137
/*149 */ { rv_op_feq_s             , 1, 0, 0, 1, 0, 0, 0, 0 },  //138
/*150 */ { rv_op_fcvt_w_s          , 1, 0, 0, 1, 0, 1, 0, 0 },  //139
/*151 */ { rv_op_fcvt_wu_s         , 1, 0, 0, 1, 0, 1, 0, 0 },  //140
/*152 */ { rv_op_fcvt_s_w          , 1, 0, 0, 1, 0, 1, 0, 0 },  //141
/*153 */ { rv_op_fcvt_s_wu         , 1, 0, 0, 1, 0, 1, 0, 0 },  //142
/*154 */ { rv_op_fmv_x_s           , 1, 0, 0, 1, 0, 0, 0, 0 },  //143     e
/*155 */ { rv_op_fclass_s          , 1, 0, 0, 1, 0, 0, 0, 0 },  //144
/*156 */ { rv_op_fmv_s_x           , 1, 0, 0, 1, 0, 0, 0, 0 },  //145     e
/*157 */ { rv_op_fcvt_l_s          , 1, 0, 0, 1, 0, 1, 0, 0 },  //146
/*158 */ { rv_op_fcvt_lu_s         , 1, 0, 0, 1, 0, 1, 0, 0 },  //147
/*159 */ { rv_op_fcvt_s_l          , 1, 0, 0, 1, 0, 1, 0, 0 },  //148
/*160 */ { rv_op_fcvt_s_lu         , 1, 0, 0, 1, 0, 1, 0, 0 },  //149
/*161 */ { rv_op_fld               , 1, 0, 0, 1, 0, 0, 1, 0 },  //150
/*162 */ { rv_op_fsd               , 1, 0, 0, 1, 0, 0, 1, 0 },  //151     e
/*163 */ { rv_op_fmadd_d           , 1, 0, 0, 1, 0, 0, 0, 0 },  //152
/*164 */ { rv_op_fmsub_d           , 1, 0, 0, 1, 0, 0, 0, 0 },  //153
/*165 */ { rv_op_fnmsub_d          , 1, 0, 0, 1, 0, 0, 0, 0 },  //154
/*166 */ { rv_op_fnmadd_d          , 1, 0, 0, 1, 0, 0, 0, 0 },  //155
/*167 */ { rv_op_fadd_d            , 1, 0, 0, 1, 0, 0, 0, 0 },  //156
/*168 */ { rv_op_fsub_d            , 1, 0, 0, 1, 0, 0, 0, 0 },  //157
/*169 */ { rv_op_fmul_d            , 1, 0, 0, 1, 1, 0, 0, 0 },  //158
/*170 */ { rv_op_fdiv_d            , 1, 0, 0, 1, 1, 0, 0, 0 },  //159
/*171 */ { rv_op_fsgnj_d           , 1, 0, 0, 1, 0, 0, 0, 0 },  //160
/*172 */ { rv_op_fsgnjn_d          , 1, 0, 0, 1, 0, 0, 0, 0 },  //161
/*173 */ { rv_op_fsgnjx_d          , 1, 0, 0, 1, 0, 0, 0, 0 },  //162
/*174 */ { rv_op_fmin_d            , 1, 0, 0, 1, 0, 0, 0, 0 },  //163
/*175 */ { rv_op_fmax_d            , 1, 0, 0, 1, 0, 0, 0, 0 },  //164
/*176 */ { rv_op_fcvt_s_d          , 1, 0, 0, 1, 0, 1, 0, 0 },  //165
/*177 */ { rv_op_fcvt_d_s          , 1, 0, 0, 1, 0, 1, 0, 0 },  //166
/*178 */ { rv_op_fsqrt_d           , 1, 0, 0, 1, 0, 0, 0, 0 },  //167     t
/*179 */ { rv_op_fle_d             , 1, 0, 0, 1, 0, 0, 0, 0 },  //168
/*180 */ { rv_op_flt_d             , 1, 0, 0, 1, 0, 0, 0, 0 },  //169
/*181 */ { rv_op_feq_d             , 1, 0, 0, 1, 0, 0, 0, 0 },  //170
/*182 */ { rv_op_fcvt_w_d          , 1, 0, 0, 1, 0, 1, 0, 0 },  //171
/*183 */ { rv_op_fcvt_wu_d         , 1, 0, 0, 1, 0, 1, 0, 0 },  //172
/*184 */ { rv_op_fcvt_d_w          , 1, 0, 0, 1, 0, 1, 0, 0 },  //173
/*185 */ { rv_op_fcvt_d_wu         , 1, 0, 0, 1, 0, 1, 0, 0 },  //174
/*186 */ { rv_op_fclass_d          , 1, 0, 0, 1, 0, 0, 0, 0 },  //175
/*187 */ { rv_op_fcvt_l_d          , 1, 0, 0, 1, 0, 1, 0, 0 },  //176
/*188 */ { rv_op_fcvt_lu_d         , 1, 0, 0, 1, 0, 1, 0, 0 },  //177
/*189 */ { rv_op_fmv_x_d           , 1, 0, 0, 1, 0, 0, 0, 0 },  //178     e
/*190 */ { rv_op_fcvt_d_l          , 1, 0, 0, 1, 0, 1, 0, 0 },  //179
/*191 */ { rv_op_fcvt_d_lu         , 1, 0, 0, 1, 0, 1, 0, 0 },  //180
/*192 */ { rv_op_fmv_d_x           , 1, 0, 0, 1, 0, 0, 0, 0 },  //181     e
/*193 */ { rv_op_flq               , 1, 0, 0, 1, 0, 0, 1, 0 },  //182
/*194 */ { rv_op_fsq               , 1, 0, 0, 1, 0, 0, 1, 0 },  //183
/*195 */ { rv_op_fmadd_q           , 1, 0, 0, 1, 0, 0, 0, 0 },  //184
/*196 */ { rv_op_fmsub_q           , 1, 0, 0, 1, 0, 0, 0, 0 },  //185
/*197 */ { rv_op_fnmsub_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //186
/*198 */ { rv_op_fnmadd_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //187
/*199 */ { rv_op_fadd_q            , 1, 0, 0, 1, 0, 0, 0, 0 },  //188
/*200 */ { rv_op_fsub_q            , 1, 0, 0, 1, 0, 0, 0, 0 },  //189
/*201 */ { rv_op_fmul_q            , 1, 0, 0, 1, 1, 0, 0, 0 },  //190
/*202 */ { rv_op_fdiv_q            , 1, 0, 0, 1, 1, 0, 0, 0 },  //191
/*203 */ { rv_op_fsgnj_q           , 1, 0, 0, 1, 0, 0, 0, 0 },  //192
/*204 */ { rv_op_fsgnjn_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //193
/*205 */ { rv_op_fsgnjx_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //194
/*206 */ { rv_op_fmin_q            , 1, 0, 0, 1, 0, 0, 0, 0 },  //195
/*207 */ { rv_op_fmax_q            , 1, 0, 0, 1, 0, 0, 0, 0 },  //196
/*208 */ { rv_op_fcvt_s_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //197
/*209 */ { rv_op_fcvt_q_s          , 1, 0, 0, 1, 0, 0, 0, 0 },  //198
/*210 */ { rv_op_fcvt_d_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //199
/*211 */ { rv_op_fcvt_q_d          , 1, 0, 0, 1, 0, 0, 0, 0 },  //200
/*212 */ { rv_op_fsqrt_q           , 1, 0, 0, 1, 0, 0, 0, 0 },  //201
/*213 */ { rv_op_fle_q             , 1, 0, 0, 1, 0, 0, 0, 0 },  //202
/*214 */ { rv_op_flt_q             , 1, 0, 0, 1, 0, 0, 0, 0 },  //203
/*215 */ { rv_op_feq_q             , 1, 0, 0, 1, 0, 0, 0, 0 },  //204
/*216 */ { rv_op_fcvt_w_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //205
/*217 */ { rv_op_fcvt_wu_q         , 1, 0, 0, 1, 0, 0, 0, 0 },  //206
/*218 */ { rv_op_fcvt_q_w          , 1, 0, 0, 1, 0, 0, 0, 0 },  //207
/*219 */ { rv_op_fcvt_q_wu         , 1, 0, 0, 1, 0, 0, 0, 0 },  //208
/*220 */ { rv_op_fclass_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //209
/*221 */ { rv_op_fcvt_l_q          , 1, 0, 0, 1, 0, 0, 0, 0 },  //210
/*222 */ { rv_op_fcvt_lu_q         , 1, 0, 0, 1, 0, 0, 0, 0 },  //211
/*223 */ { rv_op_fcvt_q_l          , 1, 0, 0, 1, 0, 0, 0, 0 },  //212
/*224 */ { rv_op_fcvt_q_lu         , 1, 0, 0, 1, 0, 0, 0, 0 },  //213
/*225 */ { rv_op_fmv_x_q           , 1, 0, 0, 1, 0, 0, 0, 0 },  //214
/*226 */ { rv_op_fmv_q_x           , 1, 0, 0, 1, 0, 0, 0, 0 },  //215
/*227 */ { rv_op_c_addi4spn        , 1, 0, 0, 0, 0, 0, 0, 0 },  //216
/*228 */ { rv_op_c_fld             , 1, 0, 0, 0, 0, 0, 1, 0 },  //217
/*229 */ { rv_op_c_lw              , 1, 0, 0, 0, 0, 0, 1, 0 },  //218
/*230 */ { rv_op_c_flw             , 1, 0, 0, 0, 0, 0, 1, 0 },  //219
/*231 */ { rv_op_c_fsd             , 1, 0, 0, 0, 0, 0, 1, 0 },  //220
/*232 */ { rv_op_c_sw              , 1, 0, 0, 0, 0, 0, 1, 0 },  //221
/*233 */ { rv_op_c_fsw             , 1, 0, 0, 0, 0, 0, 1, 0 },  //222
/*234 */ { rv_op_c_nop             , 1, 0, 0, 0, 0, 0, 0, 0 },  //223
/*235 */ { rv_op_c_addi            , 1, 0, 0, 0, 0, 0, 0, 0 },  //224
/*236 */ { rv_op_c_jal             , 1, 0, 0, 0, 0, 0, 0, 0 },  //225
/*237 */ { rv_op_c_li              , 1, 0, 0, 0, 0, 0, 0, 0 },  //226
/*238 */ { rv_op_c_addi16sp        , 1, 0, 0, 0, 0, 0, 0, 0 },  //227
/*239 */ { rv_op_c_lui             , 1, 0, 0, 0, 0, 0, 0, 0 },  //228
/*240 */ { rv_op_c_srli            , 1, 0, 0, 0, 0, 0, 0, 0 },  //229
/*241 */ { rv_op_c_srai            , 1, 0, 0, 0, 0, 0, 0, 0 },  //230
/*242 */ { rv_op_c_andi            , 1, 0, 0, 0, 0, 0, 0, 0 },  //231
/*243 */ { rv_op_c_sub             , 1, 0, 0, 0, 0, 0, 0, 0 },  //232
/*244 */ { rv_op_c_xor             , 1, 0, 0, 0, 0, 0, 0, 0 },  //233
/*245 */ { rv_op_c_or              , 1, 0, 0, 0, 0, 0, 0, 0 },  //234
/*246 */ { rv_op_c_and             , 1, 0, 0, 0, 0, 0, 0, 0 },  //235
/*247 */ { rv_op_c_subw            , 1, 0, 0, 0, 0, 0, 0, 0 },  //236
/*248 */ { rv_op_c_addw            , 1, 0, 0, 0, 0, 0, 0, 0 },  //237
/*249 */ { rv_op_c_j               , 1, 0, 0, 0, 0, 0, 0, 0 },  //238
/*250 */ { rv_op_c_beqz            , 1, 0, 0, 0, 0, 0, 0, 0 },  //239
/*251 */ { rv_op_c_bnez            , 1, 0, 0, 0, 0, 0, 0, 0 },  //240
/*252 */ { rv_op_c_slli            , 1, 0, 0, 0, 0, 0, 0, 0 },  //241
/*253 */ { rv_op_c_fldsp           , 1, 0, 0, 0, 0, 0, 0, 0 },  //242
/*254 */ { rv_op_c_lwsp            , 1, 0, 0, 0, 0, 0, 0, 0 },  //243
/*255 */ { rv_op_c_flwsp           , 1, 0, 0, 0, 0, 0, 0, 0 },  //244
/*256 */ { rv_op_c_jr              , 1, 0, 0, 0, 0, 0, 0, 0 },  //245
/*257 */ { rv_op_c_mv              , 1, 0, 0, 0, 0, 0, 0, 0 },  //246
/*258 */ { rv_op_c_ebreak          , 1, 0, 0, 0, 0, 0, 0, 0 },  //247
/*259 */ { rv_op_c_jalr            , 1, 0, 0, 0, 0, 0, 0, 0 },  //248
/*260 */ { rv_op_c_add             , 1, 0, 0, 0, 0, 0, 0, 0 },  //249
/*261 */ { rv_op_c_fsdsp           , 1, 0, 0, 0, 0, 0, 0, 0 },  //250
/*262 */ { rv_op_c_swsp            , 1, 0, 0, 0, 0, 0, 0, 0 },  //251
/*263 */ { rv_op_c_fswsp           , 1, 0, 0, 0, 0, 0, 0, 0 },  //252
/*264 */ { rv_op_c_ld              , 1, 0, 0, 0, 0, 0, 1, 0 },  //253
/*265 */ { rv_op_c_sd              , 1, 0, 0, 0, 0, 0, 1, 0 },  //254
/*266 */ { rv_op_c_addiw           , 1, 0, 0, 0, 0, 0, 0, 0 },  //255
/*267 */ { rv_op_c_ldsp            , 1, 0, 0, 0, 0, 0, 1, 0 },  //256
/*268 */ { rv_op_c_sdsp            , 1, 0, 0, 0, 0, 0, 1, 0 },  //257
/*269 */ { rv_op_c_lq              , 1, 0, 0, 0, 0, 0, 1, 0 },  //258
/*270 */ { rv_op_c_sq              , 1, 0, 0, 0, 0, 0, 1, 0 },  //259
/*271 */ { rv_op_c_lqsp            , 1, 0, 0, 0, 0, 0, 0, 0 },  //260
/*272 */ { rv_op_c_sqsp            , 1, 0, 0, 0, 0, 0, 0, 0 },  //261
/*273 */ { rv_op_nop               , 1, 0, 0, 0, 0, 0, 0, 0 },  //262
/*274 */ { rv_op_mv                , 1, 0, 0, 0, 0, 0, 0, 0 },  //263
/*275 */ { rv_op_not               , 1, 0, 0, 0, 0, 0, 0, 0 },  //264
/*276 */ { rv_op_neg               , 1, 0, 0, 0, 0, 0, 0, 0 },  //265
/*277 */ { rv_op_negw              , 1, 0, 0, 0, 0, 0, 0, 0 },  //266
/*278 */ { rv_op_sext_w            , 1, 0, 0, 0, 0, 0, 0, 0 },  //267
/*279 */ { rv_op_seqz              , 1, 0, 0, 0, 0, 0, 0, 0 },  //268
/*280 */ { rv_op_snez              , 1, 0, 0, 0, 0, 0, 0, 0 },  //269
/*281 */ { rv_op_sltz              , 1, 0, 0, 0, 0, 0, 0, 0 },  //270
/*282 */ { rv_op_sgtz              , 1, 0, 0, 0, 0, 0, 0, 0 },  //271
/*283 */ { rv_op_fmv_s             , 1, 0, 0, 0, 0, 0, 0, 0 },  //272
/*284 */ { rv_op_fabs_s            , 1, 0, 0, 0, 0, 0, 0, 0 },  //273
/*285 */ { rv_op_fneg_s            , 1, 0, 0, 0, 0, 0, 0, 0 },  //274
/*286 */ { rv_op_fmv_d             , 1, 0, 0, 0, 0, 0, 0, 0 },  //275
/*287 */ { rv_op_fabs_d            , 1, 0, 0, 0, 0, 0, 0, 0 },  //276
/*288 */ { rv_op_fneg_d            , 1, 0, 0, 0, 0, 0, 0, 0 },  //277
/*289 */ { rv_op_fmv_q             , 1, 0, 0, 0, 0, 0, 0, 0 },  //278
/*290 */ { rv_op_fabs_q            , 1, 0, 0, 0, 0, 0, 0, 0 },  //279
/*291 */ { rv_op_fneg_q            , 1, 0, 0, 0, 0, 0, 0, 0 },  //280
/*292 */ { rv_op_beqz              , 1, 0, 0, 0, 0, 0, 0, 0 },  //281
/*293 */ { rv_op_bnez              , 1, 0, 0, 0, 0, 0, 0, 0 },  //282
/*294 */ { rv_op_blez              , 1, 0, 0, 0, 0, 0, 0, 0 },  //283
/*295 */ { rv_op_bgez              , 1, 0, 0, 0, 0, 0, 0, 0 },  //284
/*296 */ { rv_op_bltz              , 1, 0, 0, 0, 0, 0, 0, 0 },  //285
/*297 */ { rv_op_bgtz              , 1, 0, 0, 0, 0, 0, 0, 0 },  //286
/*298 */ { rv_op_ble               , 1, 0, 0, 0, 0, 0, 0, 0 },  //287
/*299 */ { rv_op_bleu              , 1, 0, 0, 0, 0, 0, 0, 0 },  //288
/*300 */ { rv_op_bgt               , 1, 0, 0, 0, 0, 0, 0, 0 },  //289
/*301 */ { rv_op_bgtu              , 1, 0, 0, 0, 0, 0, 0, 0 },  //290
/*302 */ { rv_op_j                 , 1, 0, 0, 0, 0, 0, 0, 0 },  //291
/*303 */ { rv_op_ret               , 1, 0, 0, 0, 0, 0, 0, 0 },  //292
/*304 */ { rv_op_jr                , 1, 0, 0, 0, 0, 0, 0, 0 },  //293
/*305 */ { rv_op_rdcycle           , 1, 0, 0, 0, 0, 0, 0, 0 },  //294
/*306 */ { rv_op_rdtime            , 1, 0, 0, 0, 0, 0, 0, 0 },  //295
/*307 */ { rv_op_rdinstret         , 1, 0, 0, 0, 0, 0, 0, 0 },  //296
/*308 */ { rv_op_rdcycleh          , 1, 0, 0, 0, 0, 0, 0, 0 },  //297
/*309 */ { rv_op_rdtimeh           , 1, 0, 0, 0, 0, 0, 0, 0 },  //298
/*310 */ { rv_op_rdinstreth        , 1, 0, 0, 0, 0, 0, 0, 0 },  //299
/*311 */ { rv_op_frcsr             , 1, 0, 0, 0, 0, 0, 0, 0 },  //300
/*312 */ { rv_op_frrm              , 1, 0, 0, 0, 0, 0, 0, 0 },  //301
/*313 */ { rv_op_frflags           , 1, 0, 0, 0, 0, 0, 0, 0 },  //302
/*314 */ { rv_op_fscsr             , 1, 0, 0, 0, 0, 0, 0, 0 },  //303
/*315 */ { rv_op_fsrm              , 1, 0, 0, 0, 0, 0, 0, 0 },  //304
/*316 */ { rv_op_fsflags           , 1, 0, 0, 0, 0, 0, 0, 0 },  //305
/*317 */ { rv_op_fsrmi             , 1, 0, 0, 0, 0, 0, 0, 0 },  //306
/*318 */ { rv_op_fsflagsi          , 1, 0, 0, 0, 0, 0, 0, 0 },  //307
/*319 */ { rv_op_vsetvli           ,1, 0, 0, 0, 0, 0, 0, 0 },  //308  319
/*320 */ { rv_op_vsetivli          ,1, 0, 0, 0, 0, 0, 0, 0 },  //309  319
/*321 */ { rv_op_vsetvl            ,1, 0, 0, 0, 0, 0, 0, 0 },  //310  319
/*322 */ { rv_op_vle8_v            ,0, 0, 0, 0, 0, 0, 1, 1 },  //311  320
/*323 */ { rv_op_vse8_v            ,0, 0, 0, 0, 0, 0, 1, 1 },  //312  321
/*324 */ { rv_op_vle16_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //313  322
/*325 */ { rv_op_vse16_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //314  323
/*326 */ { rv_op_vle32_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //315  324
/*327 */ { rv_op_vse32_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //316  325
/*328 */ { rv_op_vle64_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //317  326
/*329 */ { rv_op_vse64_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //318  327
/*330 */ { rv_op_vle8ff_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //319  328
/*331 */ { rv_op_vle16ff_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //320  329
/*332 */ { rv_op_vle32ff_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //321  330
/*333 */ { rv_op_vle64ff_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //322  331
/*334 */ { rv_op_vl1re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //323  332
/*335 */ { rv_op_vl1re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //324  333
/*336 */ { rv_op_vl1re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //325  334
/*337 */ { rv_op_vl1re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //326  335
/*338 */ { rv_op_vl2re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //327  336
/*339 */ { rv_op_vl2re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //328  337
/*340 */ { rv_op_vl2re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //329  338
/*341 */ { rv_op_vl2re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //330  339
/*342 */ { rv_op_vl4re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //331  340
/*343 */ { rv_op_vl4re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //332  341
/*344 */ { rv_op_vl4re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //333  342
/*345 */ { rv_op_vl4re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //334  343
/*346 */ { rv_op_vl8re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //335  344
/*347 */ { rv_op_vl8re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //336  345
/*348 */ { rv_op_vl8re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //337  346
/*349 */ { rv_op_vl8re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //338  347
/*350 */ { rv_op_vs1re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //339  348
/*351 */ { rv_op_vs1re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //340  349
/*352 */ { rv_op_vs1re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //341  350
/*353 */ { rv_op_vs1re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //342  351
/*354 */ { rv_op_vs2re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //343  352
/*355 */ { rv_op_vs2re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //344  353
/*356 */ { rv_op_vs2re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //345  354
/*357 */ { rv_op_vs2re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //346  355
/*358 */ { rv_op_vs4re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //347  356
/*359 */ { rv_op_vs4re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //348  357
/*360 */ { rv_op_vs4re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //349  358
/*361 */ { rv_op_vs4re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //350  359
/*362 */ { rv_op_vs8re8_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //351  360
/*363 */ { rv_op_vs8re16_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //352  361
/*364 */ { rv_op_vs8re32_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //353  362
/*365 */ { rv_op_vs8re64_v         ,0, 0, 0, 0, 0, 0, 1, 1 },  //354  363
/*366 */ { rv_op_vlse8_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //355  364
/*367 */ { rv_op_vsse8_v           ,0, 0, 0, 0, 0, 0, 1, 1 },  //356  365
/*368 */ { rv_op_vlse16_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //357  366
/*369 */ { rv_op_vsse16_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //358  367
/*370 */ { rv_op_vlse32_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //359  368
/*371 */ { rv_op_vsse32_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //360  369
/*372 */ { rv_op_vlse64_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //361  370
/*373 */ { rv_op_vsse64_v          ,0, 0, 0, 0, 0, 0, 1, 1 },  //362  371
/*374 */ { rv_op_vluxei8_v         ,0, 0, 0, 0, 0, 0, 1, 1 },
/*375 */ { rv_op_vsuxei8_v         ,0, 0, 0, 0, 0, 0, 1, 1 },
/*376 */ { rv_op_vluxei16_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*377 */ { rv_op_vsuxei16_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*378 */ { rv_op_vluxei32_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*379 */ { rv_op_vsuxei32_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*380 */ { rv_op_vluxei64_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*381 */ { rv_op_vsuxei64_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*382 */ { rv_op_vloxei8_v         ,0, 0, 0, 0, 0, 0, 1, 1 },
/*383 */ { rv_op_vsoxei8_v         ,0, 0, 0, 0, 0, 0, 1, 1 },
/*384 */ { rv_op_vloxei16_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*385 */ { rv_op_vsoxei16_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*386 */ { rv_op_vloxei32_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*387 */ { rv_op_vsoxei32_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*388 */ { rv_op_vloxei64_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*389 */ { rv_op_vsoxei64_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*390 */ { rv_op_vlseg2e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*391 */ { rv_op_vsseg2e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*392 */ { rv_op_vlseg2e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*393 */ { rv_op_vsseg2e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*394 */ { rv_op_vlseg2e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*395 */ { rv_op_vsseg2e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*396 */ { rv_op_vlseg2e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*397 */ { rv_op_vsseg2e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*398 */ { rv_op_vlsseg2e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*399 */ { rv_op_vssseg2e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*400 */ { rv_op_vlsseg2e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*401 */ { rv_op_vssseg2e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*402 */ { rv_op_vlsseg2e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*403 */ { rv_op_vssseg2e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*404 */ { rv_op_vlsseg2e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*405 */ { rv_op_vssseg2e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*406 */ { rv_op_vluxseg2ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*407 */ { rv_op_vsuxseg2ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*408 */ { rv_op_vluxseg2ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*409 */ { rv_op_vsuxseg2ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*410 */ { rv_op_vluxseg2ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*411 */ { rv_op_vsuxseg2ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*412 */ { rv_op_vluxseg2ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*413 */ { rv_op_vsuxseg2ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*414 */ { rv_op_vloxseg2ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*415 */ { rv_op_vsoxseg2ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*416 */ { rv_op_vloxseg2ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*417 */ { rv_op_vsoxseg2ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*418 */ { rv_op_vloxseg2ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*419 */ { rv_op_vsoxseg2ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*420 */ { rv_op_vloxseg2ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*421 */ { rv_op_vsoxseg2ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*422 */ { rv_op_vlseg3e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*423 */ { rv_op_vsseg3e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*424 */ { rv_op_vlseg3e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*425 */ { rv_op_vsseg3e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*426 */ { rv_op_vlseg3e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*427 */ { rv_op_vsseg3e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*428 */ { rv_op_vlseg3e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*429 */ { rv_op_vsseg3e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*430 */ { rv_op_vlsseg3e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*431 */ { rv_op_vssseg3e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*432 */ { rv_op_vlsseg3e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*433 */ { rv_op_vssseg3e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*434 */ { rv_op_vlsseg3e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*435 */ { rv_op_vssseg3e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*436 */ { rv_op_vlsseg3e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*437 */ { rv_op_vssseg3e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*438 */ { rv_op_vluxseg3ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*439 */ { rv_op_vsuxseg3ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*440 */ { rv_op_vluxseg3ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*441 */ { rv_op_vsuxseg3ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*442 */ { rv_op_vluxseg3ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*443 */ { rv_op_vsuxseg3ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*444 */ { rv_op_vluxseg3ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*445 */ { rv_op_vsuxseg3ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*446 */ { rv_op_vloxseg3ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*447 */ { rv_op_vsoxseg3ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*448 */ { rv_op_vloxseg3ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*449 */ { rv_op_vsoxseg3ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*450 */ { rv_op_vloxseg3ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*451 */ { rv_op_vsoxseg3ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*452 */ { rv_op_vloxseg3ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*453 */ { rv_op_vsoxseg3ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*454 */ { rv_op_vlseg4e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*455 */ { rv_op_vsseg4e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*456 */ { rv_op_vlseg4e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*457 */ { rv_op_vsseg4e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*458 */ { rv_op_vlseg4e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*459 */ { rv_op_vsseg4e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*460 */ { rv_op_vlseg4e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*461 */ { rv_op_vsseg4e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*462 */ { rv_op_vlsseg4e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*463 */ { rv_op_vssseg4e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*464 */ { rv_op_vlsseg4e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*465 */ { rv_op_vssseg4e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*466 */ { rv_op_vlsseg4e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*467 */ { rv_op_vssseg4e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*468 */ { rv_op_vlsseg4e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*469 */ { rv_op_vssseg4e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*470 */ { rv_op_vluxseg4ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*471 */ { rv_op_vsuxseg4ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*472 */ { rv_op_vluxseg4ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*473 */ { rv_op_vsuxseg4ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*474 */ { rv_op_vluxseg4ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*475 */ { rv_op_vsuxseg4ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*476 */ { rv_op_vluxseg4ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*477 */ { rv_op_vsuxseg4ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*478 */ { rv_op_vloxseg4ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*479 */ { rv_op_vsoxseg4ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*480 */ { rv_op_vloxseg4ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*481 */ { rv_op_vsoxseg4ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*482 */ { rv_op_vloxseg4ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*483 */ { rv_op_vsoxseg4ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*484 */ { rv_op_vloxseg4ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*485 */ { rv_op_vsoxseg4ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*486 */ { rv_op_vlseg5e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*487 */ { rv_op_vsseg5e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*488 */ { rv_op_vlseg5e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*489 */ { rv_op_vsseg5e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*490 */ { rv_op_vlseg5e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*491 */ { rv_op_vsseg5e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*492 */ { rv_op_vlseg5e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*493 */ { rv_op_vsseg5e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*494 */ { rv_op_vlsseg5e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*495 */ { rv_op_vssseg5e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*496 */ { rv_op_vlsseg5e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*497 */ { rv_op_vssseg5e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*498 */ { rv_op_vlsseg5e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*499 */ { rv_op_vssseg5e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*500 */ { rv_op_vlsseg5e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*501 */ { rv_op_vssseg5e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*502 */ { rv_op_vluxseg5ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*503 */ { rv_op_vsuxseg5ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*504 */ { rv_op_vluxseg5ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*505 */ { rv_op_vsuxseg5ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*506 */ { rv_op_vluxseg5ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*507 */ { rv_op_vsuxseg5ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*508 */ { rv_op_vluxseg5ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*509 */ { rv_op_vsuxseg5ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*510 */ { rv_op_vloxseg5ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*511 */ { rv_op_vsoxseg5ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*512 */ { rv_op_vloxseg5ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*513 */ { rv_op_vsoxseg5ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*514 */ { rv_op_vloxseg5ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*515 */ { rv_op_vsoxseg5ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*516 */ { rv_op_vloxseg5ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*517 */ { rv_op_vsoxseg5ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*518 */ { rv_op_vlseg6e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*519 */ { rv_op_vsseg6e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*520 */ { rv_op_vlseg6e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*521 */ { rv_op_vsseg6e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*522 */ { rv_op_vlseg6e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*523 */ { rv_op_vsseg6e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*524 */ { rv_op_vlseg6e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*525 */ { rv_op_vsseg6e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*526 */ { rv_op_vlsseg6e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*527 */ { rv_op_vssseg6e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*528 */ { rv_op_vlsseg6e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*529 */ { rv_op_vssseg6e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*530 */ { rv_op_vlsseg6e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*531 */ { rv_op_vssseg6e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*532 */ { rv_op_vlsseg6e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*533 */ { rv_op_vssseg6e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*534 */ { rv_op_vluxseg6ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*535 */ { rv_op_vsuxseg6ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*536 */ { rv_op_vluxseg6ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*537 */ { rv_op_vsuxseg6ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*538 */ { rv_op_vluxseg6ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*539 */ { rv_op_vsuxseg6ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*540 */ { rv_op_vluxseg6ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*541 */ { rv_op_vsuxseg6ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*542 */ { rv_op_vloxseg6ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*543 */ { rv_op_vsoxseg6ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*544 */ { rv_op_vloxseg6ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*545 */ { rv_op_vsoxseg6ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*546 */ { rv_op_vloxseg6ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*547 */ { rv_op_vsoxseg6ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*548 */ { rv_op_vloxseg6ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*549 */ { rv_op_vsoxseg6ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*550 */ { rv_op_vlseg7e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*551 */ { rv_op_vsseg7e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*552 */ { rv_op_vlseg7e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*553 */ { rv_op_vsseg7e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*554 */ { rv_op_vlseg7e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*555 */ { rv_op_vsseg7e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*556 */ { rv_op_vlseg7e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*557 */ { rv_op_vsseg7e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*558 */ { rv_op_vlsseg7e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*559 */ { rv_op_vssseg7e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*560 */ { rv_op_vlsseg7e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*561 */ { rv_op_vssseg7e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*562 */ { rv_op_vlsseg7e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*563 */ { rv_op_vssseg7e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*564 */ { rv_op_vlsseg7e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*565 */ { rv_op_vssseg7e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*566 */ { rv_op_vluxseg7ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*567 */ { rv_op_vsuxseg7ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*568 */ { rv_op_vluxseg7ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*569 */ { rv_op_vsuxseg7ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*570 */ { rv_op_vluxseg7ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*571 */ { rv_op_vsuxseg7ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*572 */ { rv_op_vluxseg7ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*573 */ { rv_op_vsuxseg7ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*574 */ { rv_op_vloxseg7ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*575 */ { rv_op_vsoxseg7ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*576 */ { rv_op_vloxseg7ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*577 */ { rv_op_vsoxseg7ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*578 */ { rv_op_vloxseg7ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*579 */ { rv_op_vsoxseg7ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*580 */ { rv_op_vloxseg7ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*581 */ { rv_op_vsoxseg7ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*582 */ { rv_op_vlseg8e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*583 */ { rv_op_vsseg8e8_v        ,0, 0, 0, 0, 0, 0, 1, 1 },
/*584 */ { rv_op_vlseg8e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*585 */ { rv_op_vsseg8e16_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*586 */ { rv_op_vlseg8e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*587 */ { rv_op_vsseg8e32_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*588 */ { rv_op_vlseg8e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*589 */ { rv_op_vsseg8e64_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*590 */ { rv_op_vlsseg8e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*591 */ { rv_op_vssseg8e8_v       ,0, 0, 0, 0, 0, 0, 1, 1 },
/*592 */ { rv_op_vlsseg8e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*593 */ { rv_op_vssseg8e16_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*594 */ { rv_op_vlsseg8e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*595 */ { rv_op_vssseg8e32_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*596 */ { rv_op_vlsseg8e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*597 */ { rv_op_vssseg8e64_v      ,0, 0, 0, 0, 0, 0, 1, 1 },
/*598 */ { rv_op_vluxseg8ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*599 */ { rv_op_vsuxseg8ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*600 */ { rv_op_vluxseg8ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*601 */ { rv_op_vsuxseg8ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*602 */ { rv_op_vluxseg8ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*603 */ { rv_op_vsuxseg8ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*604 */ { rv_op_vluxseg8ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*605 */ { rv_op_vsuxseg8ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*606 */ { rv_op_vloxseg8ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*607 */ { rv_op_vsoxseg8ei8_v     ,0, 0, 0, 0, 0, 0, 1, 1 },
/*608 */ { rv_op_vloxseg8ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*609 */ { rv_op_vsoxseg8ei16_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*610 */ { rv_op_vloxseg8ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*611 */ { rv_op_vsoxseg8ei32_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*612 */ { rv_op_vloxseg8ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*613 */ { rv_op_vsoxseg8ei64_v    ,0, 0, 0, 0, 0, 0, 1, 1 },
/*614 */ { rv_op_vadd_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*615 */ { rv_op_vsub_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*616 */ { rv_op_vminu_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*617 */ { rv_op_vmin_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*618 */ { rv_op_vmaxu_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*619 */ { rv_op_vmax_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*620 */ { rv_op_vand_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*621 */ { rv_op_vor_vv            ,1, 0, 0, 0, 0, 0, 0, 1 },
/*622 */ { rv_op_vxor_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*623 */ { rv_op_vrgather_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*624 */ { rv_op_vadc_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*625 */ { rv_op_vmadc_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*626 */ { rv_op_vsbc_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*627 */ { rv_op_vmsbc_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*628 */ { rv_op_vmerge_vvm        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*629 */ { rv_op_vmseq_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*630 */ { rv_op_vmsne_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*631 */ { rv_op_vmsltu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*632 */ { rv_op_vmslt_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*633 */ { rv_op_vmsleu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*634 */ { rv_op_vmsle_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*635 */ { rv_op_vsaddu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*636 */ { rv_op_vsadd_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*637 */ { rv_op_vssubu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*638 */ { rv_op_vssub_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*639 */ { rv_op_vsll_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*640 */ { rv_op_vsmul_vv          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*641 */ { rv_op_vsrl_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*642 */ { rv_op_vsra_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*643 */ { rv_op_vssrl_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*644 */ { rv_op_vssra_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*645 */ { rv_op_vnsrl_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*646 */ { rv_op_vnsra_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*647 */ { rv_op_vnclipu_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*648 */ { rv_op_vnclip_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*649 */ { rv_op_vwredsumu_vv      ,1, 0, 0, 0, 0, 0, 0, 1 },
/*650 */ { rv_op_vwredsum_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*651 */ { rv_op_vdotu_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*652 */ { rv_op_vdot_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*653 */ { rv_op_vqmaccu_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*654 */ { rv_op_vqmacc_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*655 */ { rv_op_vqmaccus_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*656 */ { rv_op_vqmaccsu_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*657 */ { rv_op_vadd_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*658 */ { rv_op_vsub_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*659 */ { rv_op_vrsub_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*660 */ { rv_op_vminu_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*661 */ { rv_op_vmin_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*662 */ { rv_op_vmaxu_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*663 */ { rv_op_vmax_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*664 */ { rv_op_vand_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*665 */ { rv_op_vor_vx            ,1, 0, 0, 0, 0, 0, 0, 1 },
/*666 */ { rv_op_vxor_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*667 */ { rv_op_vrgather_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*668 */ { rv_op_vslideup_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*669 */ { rv_op_vslidedown_vx     ,1, 0, 0, 0, 0, 0, 0, 1 },
/*670 */ { rv_op_vadc_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*671 */ { rv_op_vmadc_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*672 */ { rv_op_vsbc_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*673 */ { rv_op_vmsbc_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*674 */ { rv_op_vmerge_vxm        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*675 */ { rv_op_vmseq_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*676 */ { rv_op_vmsne_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*677 */ { rv_op_vmsltu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*678 */ { rv_op_vmslt_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*679 */ { rv_op_vmsleu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*680 */ { rv_op_vmsle_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*681 */ { rv_op_vmsgtu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*682 */ { rv_op_vmsgt_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*683 */ { rv_op_vsaddu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*684 */ { rv_op_vsadd_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*685 */ { rv_op_vssubu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*686 */ { rv_op_vssub_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*687 */ { rv_op_vsll_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*688 */ { rv_op_vsmul_vx          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*689 */ { rv_op_vsrl_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*690 */ { rv_op_vsra_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*691 */ { rv_op_vssrl_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*692 */ { rv_op_vssra_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*693 */ { rv_op_vnsrl_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*694 */ { rv_op_vnsra_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*695 */ { rv_op_vnclipu_vx        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*696 */ { rv_op_vnclip_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*697 */ { rv_op_vwredsumu_vx      ,1, 0, 0, 0, 0, 0, 0, 1 },
/*698 */ { rv_op_vwredsum_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*699 */ { rv_op_vdotu_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*700 */ { rv_op_vdot_vx           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*701 */ { rv_op_vqmaccu_vx        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*702 */ { rv_op_vqmacc_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*703 */ { rv_op_vqmaccus_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*704 */ { rv_op_vqmaccsu_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*705 */ { rv_op_vadd_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*706 */ { rv_op_vrsub_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*707 */ { rv_op_vand_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*708 */ { rv_op_vor_vi            ,1, 0, 0, 0, 0, 0, 0, 1 },
/*709 */ { rv_op_vxor_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*710 */ { rv_op_vrgather_vi       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*711 */ { rv_op_vslideup_vi       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*712 */ { rv_op_vslidedown_vi     ,1, 0, 0, 0, 0, 0, 0, 1 },
/*713 */ { rv_op_vadc_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*714 */ { rv_op_vmadc_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*715 */ { rv_op_vmv_vi            ,1, 0, 0, 0, 0, 0, 0, 1 },
/*716 */ { rv_op_vmseq_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*717 */ { rv_op_vmsne_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*718 */ { rv_op_vmsleu_vi         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*719 */ { rv_op_vmsle_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*720 */ { rv_op_vmsgtu_vi         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*721 */ { rv_op_vmsgt_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*722 */ { rv_op_vmv1r             ,1, 0, 0, 0, 0, 0, 0, 1 },
/*723 */ { rv_op_vmv2r             ,1, 0, 0, 0, 0, 0, 0, 1 },
/*724 */ { rv_op_vmv4r             ,1, 0, 0, 0, 0, 0, 0, 1 },
/*725 */ { rv_op_vmv8r             ,1, 0, 0, 0, 0, 0, 0, 1 },
/*726 */ { rv_op_vsaddu_vi         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*727 */ { rv_op_vsadd_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*728 */ { rv_op_vsll_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*729 */ { rv_op_vsrl_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*730 */ { rv_op_vsra_vi           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*731 */ { rv_op_vssrl_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*732 */ { rv_op_vssra_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*733 */ { rv_op_vnsrl_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*734 */ { rv_op_vnsra_vi          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*735 */ { rv_op_vnclipu_vi        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*736 */ { rv_op_vnclip_vi         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*737 */ { rv_op_vredsum_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*738 */ { rv_op_vredand_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*739 */ { rv_op_vredor_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*740 */ { rv_op_vredxor_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*741 */ { rv_op_vredminu_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*742 */ { rv_op_vredmin_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*743 */ { rv_op_vredmaxu_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*744 */ { rv_op_vredmax_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*745 */ { rv_op_vaaddu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*746 */ { rv_op_vaadd_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*747 */ { rv_op_vasubu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*748 */ { rv_op_vasub_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*749 */ { rv_op_vmv_x_s           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*750 */ { rv_op_vpopc_m           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*751 */ { rv_op_vfirst_m          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*752 */ { rv_op_vmv_s_x           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*753 */ { rv_op_vzext_vf8         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*754 */ { rv_op_vsext_vf8         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*755 */ { rv_op_vzext_vf4         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*756 */ { rv_op_vsext_vf4         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*757 */ { rv_op_vzext_vf2         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*758 */ { rv_op_vsext_vf2         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*759 */ { rv_op_vmsbf_m           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*760 */ { rv_op_vmsof_m           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*761 */ { rv_op_vmsif_m           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*762 */ { rv_op_viota_m           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*763 */ { rv_op_vid_v             ,1, 0, 0, 0, 0, 0, 0, 1 },
/*764 */ { rv_op_vcompress_vv      ,1, 0, 0, 0, 0, 0, 0, 1 },
/*765 */ { rv_op_vmandnot_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*766 */ { rv_op_vmand_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*767 */ { rv_op_vmor_vv           ,1, 0, 0, 0, 0, 0, 0, 1 },
/*768 */ { rv_op_vmxor_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*769 */ { rv_op_vmornot_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*770 */ { rv_op_vmnand_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*771 */ { rv_op_vmnor_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*772 */ { rv_op_vmxnor_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*773 */ { rv_op_vdivu_vv          ,1, 0, 1, 0, 0, 0, 0, 1 },
/*774 */ { rv_op_vdiv_vv           ,1, 0, 1, 0, 0, 0, 0, 1 },
/*775 */ { rv_op_vremu_vv          ,1, 0, 1, 0, 0, 0, 0, 1 },
/*776 */ { rv_op_vrem_vv           ,1, 0, 1, 0, 0, 0, 0, 1 },
/*777 */ { rv_op_vmulhu_vv         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*778 */ { rv_op_vmul_vv           ,0, 1, 0, 0, 0, 0, 0, 1 },
/*779 */ { rv_op_vmulhsu_vv        ,0, 1, 0, 0, 0, 0, 0, 1 },
/*780 */ { rv_op_vmulh_vv          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*781 */ { rv_op_vmadd_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*782 */ { rv_op_vnmsub_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*783 */ { rv_op_vmacc_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*784 */ { rv_op_vnmsac_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*785 */ { rv_op_vwaddu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*786 */ { rv_op_vwadd_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*787 */ { rv_op_vwsubu_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*788 */ { rv_op_vwsub_vv          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*789 */ { rv_op_vwaddu_w_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*790 */ { rv_op_vwadd_w_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*791 */ { rv_op_vwsubu_w_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*792 */ { rv_op_vwsub_w_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*793 */ { rv_op_vwmulu_vv         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*794 */ { rv_op_vwmulsu_vv        ,0, 1, 0, 0, 0, 0, 0, 1 },
/*795 */ { rv_op_vwmul_vv          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*796 */ { rv_op_vwmaccu_vv        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*797 */ { rv_op_vwmacc_vv         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*798 */ { rv_op_vwmaccus_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*799 */ { rv_op_vwmaccsu_vv       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*800 */ { rv_op_vaaddu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*801 */ { rv_op_vaadd_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*802 */ { rv_op_vasubu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*803 */ { rv_op_vasub_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*804 */ { rv_op_vslide1up_vx      ,1, 0, 0, 0, 0, 0, 0, 1 },
/*805 */ { rv_op_vslide1down_vx    ,1, 0, 0, 0, 0, 0, 0, 1 },
/*806 */ { rv_op_vdivu_vx          ,1, 0, 1, 0, 0, 0, 0, 1 },
/*807 */ { rv_op_vdiv_vx           ,1, 0, 1, 0, 0, 0, 0, 1 },
/*808 */ { rv_op_vremu_vx          ,1, 0, 1, 0, 0, 0, 0, 1 },
/*809 */ { rv_op_vrem_vx           ,1, 0, 1, 0, 0, 0, 0, 1 },
/*810 */ { rv_op_vmulhu_vx         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*811 */ { rv_op_vmul_vx           ,0, 1, 0, 0, 0, 0, 0, 1 },
/*812 */ { rv_op_vmulhsu_vx        ,0, 1, 0, 0, 0, 0, 0, 1 },
/*813 */ { rv_op_vmulh_vx          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*814 */ { rv_op_vmadd_vx          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*815 */ { rv_op_vnmsub_vx         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*816 */ { rv_op_vmacc_vx          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*817 */ { rv_op_vnmsac_vx         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*818 */ { rv_op_vwaddu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*819 */ { rv_op_vwadd_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*820 */ { rv_op_vwsubu_vx         ,1, 0, 0, 0, 0, 0, 0, 1 },
/*821 */ { rv_op_vwsub_vx          ,1, 0, 0, 0, 0, 0, 0, 1 },
/*822 */ { rv_op_vwaddu_w_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*823 */ { rv_op_vwadd_w_vx        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*824 */ { rv_op_vwsubu_w_vx       ,1, 0, 0, 0, 0, 0, 0, 1 },
/*825 */ { rv_op_vwsub_w_vx        ,1, 0, 0, 0, 0, 0, 0, 1 },
/*826 */ { rv_op_vwmulu_vx         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*827 */ { rv_op_vwmulsu_vx        ,0, 1, 0, 0, 0, 0, 0, 1 },
/*828 */ { rv_op_vwmul_vx          ,0, 1, 0, 0, 0, 0, 0, 1 },
/*829 */ { rv_op_vwmaccu_vx        ,0, 1, 0, 0, 0, 0, 0, 1 },
/*830 */ { rv_op_vwmacc_vx         ,0, 1, 0, 0, 0, 0, 0, 1 },
/*831 */ { rv_op_vwmaccus_vx       ,0, 1, 0, 0, 0, 0, 0, 1 },
/*832 */ { rv_op_vwmaccsu_vx       ,0, 1, 0, 0, 0, 0, 0, 1 },
/*833 */ { rv_op_vfadd_vv          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*834 */ { rv_op_vfredsum_vv       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*835 */ { rv_op_vfsub_vv          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*836 */ { rv_op_vfredosum_vv      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*837 */ { rv_op_vfmin_vv          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*838 */ { rv_op_vfredmin_vv       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*839 */ { rv_op_vfmax_vv          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*840 */ { rv_op_vfredmax_vv       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*841 */ { rv_op_vfsgnj_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*842 */ { rv_op_vfsgnjn_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*843 */ { rv_op_vfsgnjx_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*844 */ { rv_op_vfslide1up_vf     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*845 */ { rv_op_vfslide1down_vf   ,0, 0, 0, 1, 0, 0, 0, 1 },
/*846 */ { rv_op_vmfeq_vx          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*847 */ { rv_op_vmfle_vx          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*848 */ { rv_op_vmflt_vx          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*849 */ { rv_op_vmfne_vx          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*850 */ { rv_op_vfdiv_vx          ,0, 0, 0, 0, 1, 0, 0, 1 },
/*851 */ { rv_op_vfcvt_xu_f_v      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*852 */ { rv_op_vfcvt_x_f_v       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*853 */ { rv_op_vfcvt_f_xu_v      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*854 */ { rv_op_vfcvt_f_x_v       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*855 */ { rv_op_vfcvt_rtz_xu_f_v  ,0, 0, 0, 1, 0, 0, 0, 1 },
/*856 */ { rv_op_vfcvt_rtz_x_f_v   ,0, 0, 0, 1, 0, 0, 0, 1 },
/*857 */ { rv_op_vfwcvt_xu_f_v     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*858 */ { rv_op_vfwcvt_x_f_v      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*859 */ { rv_op_vfwcvt_f_xu_v     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*860 */ { rv_op_vfwcvt_f_x_v      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*861 */ { rv_op_vfwcvt_f_f_v      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*862 */ { rv_op_vfwcvt_rtz_xu_f_v ,0, 0, 0, 1, 0, 0, 0, 1 },
/*863 */ { rv_op_vfwcvt_rtz_x_f_v  ,0, 0, 0, 1, 0, 0, 0, 1 },
/*864 */ { rv_op_vfncvt_xu_f_w     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*865 */ { rv_op_vfncvt_x_f_w      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*866 */ { rv_op_vfncvt_f_xu_w     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*867 */ { rv_op_vfncvt_f_x_w      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*868 */ { rv_op_vfncvt_f_f_w      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*869 */ { rv_op_vfncvt_rod_f_f_w  ,0, 0, 0, 1, 0, 0, 0, 1 },
/*870 */ { rv_op_vfncvt_rtz_xu_f_w ,0, 0, 0, 1, 0, 0, 0, 1 },
/*871 */ { rv_op_vfncvt_rtz_x_f_w  ,0, 0, 0, 1, 0, 0, 0, 1 },
/*872 */ { rv_op_vfmul_vv          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*873 */ { rv_op_vfrsub_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*874 */ { rv_op_vfmadd_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*875 */ { rv_op_vfnmadd_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*876 */ { rv_op_vfmsub_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*877 */ { rv_op_vfnmsub_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*878 */ { rv_op_vfmacc_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*879 */ { rv_op_vfnmacc_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*880 */ { rv_op_vfmsac_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*881 */ { rv_op_vfnmsac_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*882 */ { rv_op_vfwadd_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*883 */ { rv_op_vfwredsum_vv      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*884 */ { rv_op_vfwsub_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*885 */ { rv_op_vfwredosum_vv     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*886 */ { rv_op_vfwadd_wv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*887 */ { rv_op_vfwsub_wv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*888 */ { rv_op_vfwmul_vv         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*889 */ { rv_op_vfdot_vv          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*890 */ { rv_op_vfwmacc_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*891 */ { rv_op_vfwnmacc_vv       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*892 */ { rv_op_vfwmsac_vv        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*893 */ { rv_op_vfwnmsac_vv       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*894 */ { rv_op_vfadd_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*895 */ { rv_op_vfredsum_vf       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*896 */ { rv_op_vfsub_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*897 */ { rv_op_vfredosum_vf      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*898 */ { rv_op_vfmin_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*899 */ { rv_op_vfredmin_vf       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*900 */ { rv_op_vfmax_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*901 */ { rv_op_vfredmax_vf       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*902 */ { rv_op_vfsgnj_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*903 */ { rv_op_vfsgnjn_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*904 */ { rv_op_vfsgnjx_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*905 */ { rv_op_vfmv_s_f          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*906 */ { rv_op_vfmv_f_s          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*907 */ { rv_op_vfmv_v_f          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*908 */ { rv_op_vmfeq_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*909 */ { rv_op_vmfle_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*910 */ { rv_op_vmflt_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*911 */ { rv_op_vmfne_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*912 */ { rv_op_vmfgt_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*913 */ { rv_op_vmfge_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*914 */ { rv_op_vfdiv_vf          ,0, 0, 0, 0, 1, 0, 0, 1 },
/*915 */ { rv_op_vfrdiv_vf         ,0, 0, 0, 0, 1, 0, 0, 1 },
/*916 */ { rv_op_vfmul_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*917 */ { rv_op_vfrsub_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*918 */ { rv_op_vfmadd_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*919 */ { rv_op_vfnmadd_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*920 */ { rv_op_vfmsub_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*921 */ { rv_op_vfnmsub_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*922 */ { rv_op_vfmacc_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*923 */ { rv_op_vfnmacc_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*924 */ { rv_op_vfmsac_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*925 */ { rv_op_vfnmsac_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*926 */ { rv_op_vfwadd_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*927 */ { rv_op_vfwredsum_vf      ,0, 0, 0, 1, 0, 0, 0, 1 },
/*928 */ { rv_op_vfwsub_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*929 */ { rv_op_vfwredosum_vf     ,0, 0, 0, 1, 0, 0, 0, 1 },
/*930 */ { rv_op_vfwadd_wf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*931 */ { rv_op_vfwsub_wf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*932 */ { rv_op_vfwmul_vf         ,0, 0, 0, 1, 0, 0, 0, 1 },
/*933 */ { rv_op_vfdot_vf          ,0, 0, 0, 1, 0, 0, 0, 1 },
/*934 */ { rv_op_vfwmacc_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*935 */ { rv_op_vfwnmacc_vf       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*936 */ { rv_op_vfwmsac_vf        ,0, 0, 0, 1, 0, 0, 0, 1 },
/*937 */ { rv_op_vfwnmsac_vf       ,0, 0, 0, 1, 0, 0, 0, 1 },
/*938 */ { rv_op_vfsqrt_v          ,0, 0, 0, 0, 1, 0, 0, 1 },

    { rv_op_last,           0, 0, 0, 0, 0, 0, 0, 0 }
};


#endif // __RISCV_META_H
