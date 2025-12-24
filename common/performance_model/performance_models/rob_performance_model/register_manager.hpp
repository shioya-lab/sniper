/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#ifndef REGISTER_MANAGER_HPP_
#define REGISTER_MANAGER_HPP_

#include <deque>
#include <list>
#include <cstdint>

#include "stats.h"

#include "priority_manager.hpp"

#define REG_DEBUG_PRINTF(...) { if (m_enable_rob_timer_log /* && now.getCycleCount() >= m_rob_start_cycle */) { fprintf(stderr, __VA_ARGS__); }}

class RegisterManager
{
   enum RegTypes {
      IntRegister = 0,
      FloatRegister = 1,
      VectorRegister = 2
   };

   UInt64 m_core_id;
   vec_reserve_policy_t m_vec_reserve_policy;

   UInt64 m_phy_registers[3];  // 3-types of registers defined: Int/Float/Vector
   UInt64 m_res_reserv_registers;  // 資源予約リスト内の命令の数
   UInt64 m_max_phy_registers[3];  // 3-types of registers defined: Int/Float/Vector
   UInt64 m_maxusage_phy_registers[3];
   UInt64 m_nonpri_max_vec_phy_registers;

   UInt64 m_total_vec_phy_registers;
   UInt64 m_total_vec_phy_count;

   bool   m_enable_rob_timer_log;
   UInt64 m_rob_start_cycle;

  public:

   enum AllocResult_t {
      AllocSuccess = 0,
      AllocFull    = 1,
      AllocReserve = 2,
      AllocChain   = 3
   };

   RegisterManager (UInt64 core_id, vec_reserve_policy_t vec_reserve_policy) {
      m_core_id = core_id;
      m_vec_reserve_policy = vec_reserve_policy;

      m_phy_registers[IntRegister   ] = 32;
      m_phy_registers[FloatRegister ] = 32;
      m_phy_registers[VectorRegister] = 32;
      m_res_reserv_registers = 0;

      m_maxusage_phy_registers[IntRegister   ] = 32;
      m_maxusage_phy_registers[FloatRegister ] = 32;
      m_maxusage_phy_registers[VectorRegister] = 32;

      UInt64 int_regs = Sim()->getCfg()->getInt("perf_model/core/rob_timer/int_physical_registers");
      m_max_phy_registers[IntRegister   ] = (int_regs == 0) ? UINT64_MAX : int_regs;
      UInt64 float_regs = Sim()->getCfg()->getInt("perf_model/core/rob_timer/float_physical_registers");
      m_max_phy_registers[FloatRegister ] = (float_regs == 0) ? UINT64_MAX : float_regs;
      UInt64 vec_regs = Sim()->getCfg()->getInt("perf_model/core/rob_timer/vec_physical_registers");
      m_max_phy_registers[VectorRegister] = (vec_regs == 0) ? UINT64_MAX : vec_regs;

      registerStatsMetric("rob_timer", m_core_id, "int_phyreg_max_usage",   &(m_maxusage_phy_registers[IntRegister   ]));
      registerStatsMetric("rob_timer", m_core_id, "float_phyreg_max_usage", &(m_maxusage_phy_registers[FloatRegister ]));
      registerStatsMetric("rob_timer", m_core_id, "vect_phyreg_max_usage",  &(m_maxusage_phy_registers[VectorRegister]));
      // LOG_ASSERT_ERROR(m_freelist >= 0, "Number of physical register should be larger than 32");

      m_total_vec_phy_registers = 0;
      m_total_vec_phy_count = 0;
      float vec_phy_rate = Sim()->getCfg()->getFloat("perf_model/core/rob_timer/nonpri_max_vec_phy_rate");
      if (vec_reserve_policy == VecReserveAlways) {
         m_nonpri_max_vec_phy_registers = m_max_phy_registers[VectorRegister] - 32;
      } else if (vec_reserve_policy == VecReserveParOOO ||
                 vec_reserve_policy == VecReserveSimple ||
                 vec_reserve_policy == VecReserveStatic) {
        m_nonpri_max_vec_phy_registers = 0;
      } else if (vec_phy_rate == 0.0) {
        m_nonpri_max_vec_phy_registers = Sim()->getCfg()->getInt(
            "perf_model/core/rob_timer/nonpri_max_vec_phy_registers");
      } else {
        m_nonpri_max_vec_phy_registers =
            (m_max_phy_registers[VectorRegister] - 32) * vec_phy_rate;
      }
      m_enable_rob_timer_log = Sim()->getCfg()->getBoolArray("log/enable_rob_timer_log", core_id);
      m_rob_start_cycle      = Sim()->getCfg()->getIntArray("log/rob_debug_start_cycle", core_id);

   }

   ~RegisterManager () {
      std::cout << "Maximum usage of Integer physical registers = " << m_maxusage_phy_registers[IntRegister   ] << '\n';
      std::cout << "Maximum usage of Float   physical registers = " << m_maxusage_phy_registers[FloatRegister ] << '\n';
      std::cout << "Maximum usage of Vector  physical registers = " << m_maxusage_phy_registers[VectorRegister] << '\n';

      std::cout << "-------------------\n";
      std::cout << "Average vec register usage : " << (m_total_vec_phy_registers / m_total_vec_phy_count) << '\n';
      std::cout << "-------------------\n";
   }

   inline UInt64 getAllocIntRegister()    { return m_phy_registers[IntRegister   ]; }
   inline UInt64 getAllocFloatRegister()  { return m_phy_registers[FloatRegister ]; }
   inline UInt64 getAllocVectorRegister() { return m_phy_registers[VectorRegister]; }

   inline UInt64 getNonPriVectorRegisters() { return m_res_reserv_registers; }
   inline UInt64 getNonPriMaxVectorRegisters() { return m_nonpri_max_vec_phy_registers; }

   AllocResult_t AllocateIntRegister () {
      if (m_phy_registers[IntRegister] >= m_max_phy_registers[IntRegister]) {
         return AllocFull;
      }
      m_phy_registers[IntRegister]++;
      m_maxusage_phy_registers[IntRegister] = std::max(m_maxusage_phy_registers[IntRegister], m_phy_registers[IntRegister]);

      return AllocSuccess;
   }

   AllocResult_t AllocateFloatRegister () {
      if (m_phy_registers[FloatRegister] >= m_max_phy_registers[FloatRegister]) {
         return AllocFull;
      }
      m_phy_registers[FloatRegister]++;
      m_maxusage_phy_registers[FloatRegister] = std::max(m_maxusage_phy_registers[FloatRegister], m_phy_registers[FloatRegister]);

      return AllocSuccess;
   }

   AllocResult_t AllocateVectorRegister (DynamicMicroOp *uop) {
     if (m_vec_reserve_policy == VecReserveParOOO ||
         m_vec_reserve_policy == VecReserveSimple ||
         m_vec_reserve_policy == VecReserveStatic) {
       if (uop->isUseNormalRegisterGroup()) {
         return AllocateNormalVecRegister(uop);
       } else {
         // 予約の時はそもそも確保しない
         return AllocResult_t::AllocSuccess;
       }
     } else if (m_vec_reserve_policy != vec_reserve_policy_t::VecReserveNone) {
       // 優先度付き予約
       if (uop->isUseNormalRegisterGroup()) {
         // 通常のレジスタグループから割り当てを行う命令
         return AllocateNormalVecRegister(uop);
       } else {
         return AllocNonpriVecRegisters(uop);
       }
     } else {
       // 予約なし
       return AllocateNormalVecRegister(uop);
     }
   }

   AllocResult_t AllocateNormalVecRegister (DynamicMicroOp *uop) {
      if (m_phy_registers[VectorRegister] >= m_max_phy_registers[VectorRegister]) {
         return AllocFull;
      }
      m_phy_registers[VectorRegister]++;
      m_maxusage_phy_registers[VectorRegister] = std::max(m_maxusage_phy_registers[VectorRegister], m_phy_registers[VectorRegister]);

      REG_DEBUG_PRINTF ("physical register allocate: %ld PC=%08lx %s\n", m_phy_registers[VectorRegister],
                        uop->getMicroOp()->getInstruction()->getAddress(),
                        uop->getMicroOp()->getInstruction()->getDisassembly().c_str()
      );

      return AllocSuccess;
   }


   AllocResult_t AllocNonpriVecRegisters (DynamicMicroOp *uop)
   {

      // RobEntry *entry = this->findEntryBySequenceNumber(uop->getSequenceNumber());
      // if (m_active_kanata_gen && m_konata_count < m_konata_count_max && entry->kanata_registered) {
      //    fprintf(m_core->getKanataFp(), "L\t%ld\t%d\tRes Registers = %ld\n",
      //            entry->global_sequence_id, 2,
      //            m_res_reserv_registers);
      // }
      //

      m_res_reserv_registers ++;
      REG_DEBUG_PRINTF("m_res_reserv_registers = %ld, m_nonpri_max_vec_phy_registers = %ld\n",
                       m_res_reserv_registers, m_nonpri_max_vec_phy_registers);
      if (m_res_reserv_registers <= m_nonpri_max_vec_phy_registers) {
         // 資源予約リストが足りない
         return AllocateNormalVecRegister(uop);
      } else {
         // 資源予約リストが十分
         return AllocReserve;
      }
   }


   AllocResult_t AllocateRegister (DynamicMicroOp *uop) {
      dl::Decoder *dec = Sim()->getDecoder();
      bool inst_has_dest =  uop->getMicroOp()->getDestinationRegistersLength() != 0;

      if (inst_has_dest) {
         if (uop->getMicroOp()->isFirst()) {
            dl::Decoder::decoder_reg dest_reg = uop->getMicroOp()->getDestinationRegister(0);
            if (dec->is_reg_int(dest_reg)) {
               return AllocateIntRegister();
            } else if(dec->is_reg_float(dest_reg)) {
               return AllocateFloatRegister ();
            } else if (dec->is_reg_vector(dest_reg)){
               return AllocateVectorRegister (uop);
            } else {
               LOG_ASSERT_ERROR (false, "Unknown register type.");
            }
         } else {
            // Firstではない命令はFirstにChainしている
            return AllocChain;
         }
      } else {
         return AllocSuccess;
      }
   }


   void ReleaseIntRegister () {
      m_phy_registers[IntRegister] --;
   }

   void ReleaseFloatRegister () {
      m_phy_registers[FloatRegister] --;
   }

   // ------------------
   // True: 失敗
   // False: 成功
   // ------------------
   bool ReleaseVectorRegister (DynamicMicroOp *uop) {
      if (m_vec_reserve_policy != vec_reserve_policy_t::VecReserveNone) {
         // 優先度付き予約手法の場合
         if (uop->isUseNormalRegisterGroup()) {
            LOG_ASSERT_ERROR (m_phy_registers[VectorRegister] != 32, "m_phy_registers[VectorRegister] should be larger or qual than 0");
            m_phy_registers[VectorRegister] --;
            REG_DEBUG_PRINTF ("physical register return: %ld PC=%08lx %s\n", m_phy_registers[VectorRegister],
                              uop->getMicroOp()->getInstruction()->getAddress(),
                              uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
         } else {
            if (unlikely(m_res_reserv_registers == 0)) {
               fprintf (stderr, "m_res_reserve_registers should be larger or qual than 0");
               return true;
            }
            m_res_reserv_registers --;
            REG_DEBUG_PRINTF ("physical register low priority return: %ld PC=%08lx %s\n", m_res_reserv_registers,
                              uop->getMicroOp()->getInstruction()->getAddress(),
                              uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
         }
      // } else if (m_vec_reserved_allocation) {
      //    // 通常の予約手法の場合
      //    // 非優先命令において，物理レジスタの資源が解放されれれば，m_lpiq_fifo内の先頭ハザードをRESOLVEDに変更する
      //    if (m_lpiq_fifo.size() != 0) {
      //       bool register_passed = false;
      //       for (auto &f : m_lpiq_fifo) {
      //          RobEntry *waiting_entry = findEntryBySequenceNumber(f);
      //          if (waiting_entry->uop->hasCommitDependency() &&
      //              waiting_entry->uop->getCommitDependency() == DynamicMicroOp::wfifo_t::PHYREG) {
      //             waiting_entry->uop->setCommitDependency(DynamicMicroOp::wfifo_t::RESOLVED);
      //
      //             if (m_active_kanata_gen && m_konata_count < m_konata_count_max) {
      //                fprintf(m_core->getKanataFp(), "W\t%ld\t%ld\t%d\n",
      //                        waiting_entry->global_sequence_id,
      //                        entry->global_sequence_id,
      //                        0);
      //             }
      //
      //             register_passed = true;
      //             break;
      //          }
      //       }
      //       if (!register_passed) {
      //          m_phy_registers[VectorRegister]--;
      //       }
      //    } else {
      //       m_phy_registers[VectorRegister]--;
      //    }
      } else {
         // 予約なしの方法
         LOG_ASSERT_ERROR (m_phy_registers[VectorRegister] != 0, "m_phy_registers[VectorRegister] should be larger or qual than 0");
         m_phy_registers[VectorRegister]--;
      }
      return false;
   }

   bool ForceReleaseVoctorRegister () {
      LOG_ASSERT_ERROR (m_vec_reserve_policy != vec_reserve_policy_t::VecReserveNone, "This function is only valid in m_vec_reserve_policy != vec_reserve_policy_t::VecReserveNone.");
      if (unlikely(m_phy_registers[VectorRegister] == 32)) {
         return true;
      }
      m_phy_registers[VectorRegister] --;
      return false;
   }

   bool ReleaseRegister (DynamicMicroOp *uop) {
      if (uop->getMicroOp()->getDestinationRegistersLength() != 0 && uop->isLast()) {
         dl::Decoder *dec = Sim()->getDecoder();
         if (dec->is_reg_int(uop->getMicroOp()->getDestinationRegister(0))) {
            ReleaseIntRegister ();
         } else if(dec->is_reg_float(uop->getMicroOp()->getDestinationRegister(0))) {
            ReleaseFloatRegister ();
         } else if (dec->is_reg_vector(uop->getMicroOp()->getDestinationRegister(0))){
            return ReleaseVectorRegister (uop);
         } else {
            LOG_ASSERT_ERROR (false, "Unknown register type.");
         }
      }
      return false;
   }

   void UpdateRegisterStats () {
      m_total_vec_phy_registers += m_phy_registers[VectorRegister];
      m_total_vec_phy_count ++;
   }

};

#endif // REGISTER_MANAGER_HPP_
