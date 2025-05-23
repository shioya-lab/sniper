#pragma once

#include <map>
#include <list>
#include <cmath>

#include "config.hpp"

typedef enum {
   VecReserveWhenFull,  // ベクトルレジスタがいっぱいになったらReserve
   VecReserveDynamic,   // ベクトルレジスタの割り当てポリシは動的に決める
   VecReserveStatic,    // ベクトルレジスタの割り当てはPCにより静的に決まる
   VecReserveAlways,    // ベクトルレジスタは常にReserve
   VecReserveParOOO,   // 予約に回ったベクトルレジスタはインオーダ
   VecReserveNone       // 予約なし
} vec_reserve_policy_t;

typedef enum {
   None,
   Added,
   Removed
} pri_upd_result_t;

// ------------------------------------------------------------
// 予約機構を使うポリシかどうか
// ------------------------------------------------------------
inline bool isUseNonpriVector(vec_reserve_policy_t res) {
   return res == VecReserveDynamic ||
          res == VecReserveParOOO  ||
          res == VecReserveStatic  ||
          res == VecReserveAlways;
}

extern uint64_t simple_mix_hash(uint64_t key);
extern uint64_t xor_fold_hash(uint64_t address, unsigned int N);

#define HASH_IDX(pc) (xor_fold_hash(simple_mix_hash(pc >> 2), static_cast<size_t>(std::log2(PRIORITY_MAP_SIZE))))

class PriorityManager {

public:
   typedef enum {
      Normal  = 0,
      Reserve = 1,
      High    = 2,
      HighOrigin = 3
   } inst_priority_t;

   typedef struct {
      UInt64          pc;
      inst_priority_t pr;
   } pri_entry_t;

private:
   vec_reserve_policy_t m_vec_reserve_policy;
   const String m_app;

   ComponentTime *m_now;

   const UInt64 PRIORITY_MAP_SIZE = 64;
   std::vector<pri_entry_t> m_priority_map;
   // inline size_t GetPriMapIdx(UInt64 pc) {
   //     return xor_fold_hash(murmur_hash(pc >> 2), static_cast<size_t>(std::log2(PRIORITY_MAP_SIZE)));
   // }

   std::list<UInt64> m_priority_remove_queue;  // Highが依存する命令の削除候補キュー

   size_t target_inst_counter;
   size_t inst_counter;

   public:
      PriorityManager(ComponentTime *now, vec_reserve_policy_t vec_reserve_policy)
      : m_vec_reserve_policy (vec_reserve_policy)
      , m_app(Sim()->getCfg()->getString("general/app"))
      , m_priority_map (PRIORITY_MAP_SIZE)
      {
         m_now = now;
         target_inst_counter = 0;

         registerStatsMetric("rob_timer", 0, "target_inst_count", &target_inst_counter);
         registerStatsMetric("rob_timer", 0, "inst_count", &inst_counter);
      }

   std::list<UInt64>* getPriorityRemoveQueue () {
      return &m_priority_remove_queue;
   }

   void dumpPriorityMap () {
      for (auto it = m_priority_map.begin(); it != m_priority_map.end(); it++) {
         fprintf (stderr, "  pc=%08lx : %s\n", (*it).pc, (*it).pr == 0 ? "Normal" : (*it).pr == 1 ? "Reserve" : "High");
      }
   }

   std::vector<pri_entry_t>* getPriorityMap () {
      return &m_priority_map;
   }

   // priorityがRemoveされれば、trueを返す
   pri_upd_result_t UpdateInstPriority (const MicroOp *uop, bool vec_miss)
   {
      auto pc = uop->getInstruction()->getAddress();
      auto assembly = uop->getInstruction()->getDisassembly();

      // auto it = m_priority_map.find(pc);
      // if (it == m_priority_map.end()) {
      // m_priority_map[pc] = inst_priority_t::HighOrigin;

      auto it = m_priority_map[HASH_IDX(pc)];
      if (it.pc != pc) {
         if (vec_miss) {
            m_priority_map[HASH_IDX(pc)].pc = pc;
            m_priority_map[HASH_IDX(pc)].pr = inst_priority_t::HighOrigin;
            fprintf (stderr, "%ld: pc=%08lx : Set Priority High. priority_map[%ld] %s\n", m_now->getCycleCount(), pc, HASH_IDX(pc), assembly.c_str());
            return pri_upd_result_t::Added;
         }
         return pri_upd_result_t::None;
      } else {
         inst_priority_t priority = it.pr;
         if (priority == inst_priority_t::HighOrigin) {
            if (!vec_miss) {
               m_priority_map[HASH_IDX(pc)].pc = 0; // Removed
               fprintf (stderr, "%ld: pc=%08lx : Remove Priority. %s\n", m_now->getCycleCount(), pc, assembly.c_str());
               // dumpPriorityMap();
               m_priority_remove_queue.push_back (pc);
               return pri_upd_result_t::Removed;
            }
         }
         return pri_upd_result_t::None;
      }
   }

   inst_priority_t getPriority (UInt64 pc) {
      if (m_vec_reserve_policy == VecReserveStatic) {
         return getPriority_Static(pc);
      } else {
         // マップにキー(pc)がある場合はその値を返す
         // auto it = m_priority_map.find(pc);
         // if (it != m_priority_map.end()) {
         //    return it->second == HighOrigin ? High : it->second;
         // }
         auto it = m_priority_map[HASH_IDX(pc)];
         if (it.pc == pc) {
            return it.pr == inst_priority_t::HighOrigin ? inst_priority_t::High : it.pr;
         }
         // ない場合はデフォルト値
         return Normal;
      }
   }

   void setPriority (UInt64 pc, inst_priority_t priority) {
      // マップにキー(pc)がない場合は新規エントリが作られる
      if (m_priority_map[HASH_IDX(pc)].pc != pc) {
         if (m_priority_map[HASH_IDX(pc)].pc != 0) {
            fprintf (stderr, "Priority Map [%ld].pc = %08lx is overwritten into pc=%08lx\n", HASH_IDX(pc), m_priority_map[HASH_IDX(pc)].pc, pc);
         } else {
            fprintf (stderr, "Priority Map [%ld] put into pc=%08lx\n", HASH_IDX(pc), pc);
         }
      }
      m_priority_map[HASH_IDX(pc)].pc = pc;
      m_priority_map[HASH_IDX(pc)].pr = priority;
      // m_priority_map[pc] = priority;
   }

   void removePriority (UInt64 pc) {
      // m_priority_map.erase(pc);
      m_priority_map[HASH_IDX(pc)].pc = 0;
   }

   inst_priority_t getPriority_Static (UInt64 pc) {
      if (m_app == "bfs") {
         switch (pc) {
            // case 0x142e0 : // vl1re64.v	v8, (t2)
            // case 0x142e4 : // vl1re64.v	v9, (t1)
            //
            // case 0x14484 : // vl1re64.v	v12, (s9)
            //    index = 0;
            //    break;
            // case 0x14488 : // vsll.vi	v12, v12, 3
            // case 0x1448c : // vluxei64.v	v13, (t6), v12
            //
            // case 0x14948 : // vle64.v	v8, (a7)
            //    index = 1;
            //    break;
            // case 0x14950 : // vsll.vi	v8, v8, 3
            // case 0x14954 : // vluxei64.v	v9, (a7), v8
            // case 0x14958 : // vmslt.vx	v9, v9, zero
            // case 0x14970 : // vle64.v	v10, (t3)
            // case 0x14974 : // vmv.v.i	v11, 0
            // case 0x14994 : // vmv1r.v	v0, v9
            case 0x149a4 : // vle64.v	v13, (t0)
            case 0x149a8 : // vsll.vi	v14, v13, 3
            case 0x149ac : // vluxei64.v	v14, (a2), v14
               return inst_priority_t::High;
            default:
               return inst_priority_t::Normal;
         }
      } else if (m_app == "cc") {
         switch (pc) {
            case 0x13c9c:  // vle64.v	v8, (a5)
            case 0x13ca0:  // vsll.vi	v9, v8, 3
            case 0x13ca4:  // vluxei64.v	v9, (a6), v9

            case 0x13d14: // vle64.v	v8, (a5)
            case 0x13d1c: // vsll.vi	v11, v8, 3
            case 0x13d20: // vluxei64.v	v12, (t0), v11

            case 0x13f6c: // vle64.v	v8, (s0)
            case 0x13f70: // vsll.vi	v8, v8, 3
            case 0x13f74: // vluxei64.v	v11, (t6), v8

            case 0x1406c: // vle64.v	v8, (a3)
            case 0x14070: // vsll.vi	v8, v8, 3
            case 0x14074: // vluxei64.v	v9, (a0), v8

            case 0x14078: // vle64.v	v8, (a5)
            case 0x1407c: // vsll.vi	v8, v8, 3
            case 0x14080: // vluxei64.v	v10, (a0), v8
               return inst_priority_t::High;
            default:
               return inst_priority_t::Normal;
         }
      } else if (m_app == "pr") {
         switch (pc) {
            case 0x143ac: // vle64.v	v11, (a7)
            case 0x143b0: // vsll.vi	v11, v11, 3
            case 0x143b4: // vluxei64.v	v11, (a2), v11
               return inst_priority_t::High;
            default:
               return inst_priority_t::Normal;
         }
      } else if (m_app == "sssp") {
         switch (pc) {
            case 0x142e0: // vle64.v	v10, (a4)
            case 0x142ec: // vsll.vi	v10, v10, 3
            case 0x142f0: // vluxei64.v	v10, (t0), v10

            case 0x14298: // vle64.v	v8, (a6)
            case 0x142a4: // vsll.vi	v8, v8, 3
            case 0x142a8: // vluxei64.v	v9, (a1), v8
               return inst_priority_t::High;
         }
      } else if (m_app == "00") {
         switch (pc) {
            case 0x10692:
               return inst_priority_t::High;
            default:
               return inst_priority_t::Normal;
         }
      } else if (m_app == "01") {
         switch (pc) {
            case 0x106a2:
               return inst_priority_t::High;
            default:
               return inst_priority_t::Normal;
         }
      } else if (m_app == "02") { // spmv
         switch (pc) {
            case 0x103f6 : // vle64.v	v24, (t3)
            case 0x103fa : // vle64.v	v8, (t1)
            case 0x103fe : // vsll.vi	v24, v24, 3
            case 0x10404 : // vluxei64.v	v24, (a3), v24
               // ROB_DEBUG_PRINTF("spmv instruction %08lx is priority instruction\n", inst_address);
               return inst_priority_t::High;
            default :
               // ROB_DEBUG_PRINTF("spmv instruction %08lx is NOT priority instruction\n", inst_address);
               return inst_priority_t::Normal;
         }
      } else {
         return inst_priority_t::Normal;
      }
      return inst_priority_t::Normal;
   }

   // 特定の命令の回数をカウントする：
   void countTargetInst (MicroOp uop) {
      if (!uop.isLast()) {
         return;
      }
      inst_counter++;
      UInt64 pc = uop.getInstruction()->getAddress();
      if (m_app == "bfs") {
         switch (pc) {
            case 0x14484 : // vl1re64.v	v12, (s9)
            case 0x14488 : // vsll.vi	v12, v12, 3
            case 0x1448c : // vluxei64.v	v13, (t6), v12

            case 0x14948 : // vle64.v	v8, (a7)
            case 0x14950 : // vsll.vi	v8, v8, 3
            case 0x14954 : // vluxei64.v	v9, (a7), v8

            case 0x14970 : // vle64.v	v10, (t3)
            case 0x14974 : // vmv.v.i	v11, 0
            case 0x14994 : // vmv1r.v	v0, v9

            case 0x149a4 : // vle64.v	v13, (t0)
            case 0x149a8 : // vsll.vi	v14, v13, 3
            case 0x149ac : // vluxei64.v	v14, (a2), v14
               target_inst_counter++;
               break;
            default:
               break;
            }
      } else if (m_app == "cc") {
         switch (pc) {
            case 0x13c9c:  // vle64.v	v8, (a5)
            case 0x13ca0:  // vsll.vi	v9, v8, 3
            case 0x13ca4:  // vluxei64.v	v9, (a6), v9

            case 0x13d14: // vle64.v	v8, (a5)
            case 0x13d1c: // vsll.vi	v11, v8, 3
            case 0x13d20: // vluxei64.v	v12, (t0), v11

            case 0x13f6c: // vle64.v	v8, (s0)
            case 0x13f70: // vsll.vi	v8, v8, 3
            case 0x13f74: // vluxei64.v	v11, (t6), v8

            case 0x1406c: // vle64.v	v8, (a3)
            case 0x14070: // vsll.vi	v8, v8, 3
            case 0x14074: // vluxei64.v	v9, (a0), v8

            case 0x14078: // vle64.v	v8, (a5)
            case 0x1407c: // vsll.vi	v8, v8, 3
            case 0x14080: // vluxei64.v	v10, (a0), v8
               target_inst_counter++;
               break;
            default:
               break;
         }
      } else if (m_app == "pr") {
         switch (pc) {
            case 0x143ac: // vle64.v	v11, (a7)
            case 0x143b0: // vsll.vi	v11, v11, 3
            case 0x143b4: // vluxei64.v	v11, (a2), v11
               target_inst_counter++;
               break;
            default:
               break;
         }
      } else if (m_app == "sssp") {
         switch (pc) {
            case 0x142e0: // vle64.v	v10, (a4)
            case 0x142ec: // vsll.vi	v10, v10, 3
            case 0x142f0: // vluxei64.v	v10, (t0), v10

            case 0x14298: // vle64.v	v8, (a6)
            case 0x142a4: // vsll.vi	v8, v8, 3
            case 0x142a8: // vluxei64.v	v9, (a1), v8
               target_inst_counter++;
               break;
            default:
               break;
         }
      }
   }
};
