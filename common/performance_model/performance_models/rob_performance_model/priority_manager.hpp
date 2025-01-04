#pragma once

#include <map>
#include <list>

#include "config.hpp"
class PriorityManager {

public:
   typedef enum {
      Normal = 0,
      Reserve = 1,
      High    = 2
   } inst_priority_t;

private:
   String m_app;

   const String m_methodology;  // static / dynamic

   ComponentTime *m_now;

   std::unordered_map<UInt64, inst_priority_t> m_priority_map;
   std::list<UInt64> m_priority_remove_queue;  // Highが依存する命令の削除候補キュー

   public:
      PriorityManager(String app, ComponentTime *now)
      : m_methodology(Sim()->getCfg()->getString("perf_model/core/rob_timer/priority_methodology"))
      {
         m_app = app;
         m_now = now;
      }

   std::list<UInt64>* getPriorityRemoveQueue () {
      return &m_priority_remove_queue;
   }

   // priorityがRemoveされれば、trueを返す
   bool UpdateInstPriority (UInt64 pc, UInt64 latency)
   {
      auto it = m_priority_map.find(pc);
      if (it == m_priority_map.end()) {
         if (latency > 100) {
            m_priority_map[pc] = inst_priority_t::High;
            // ROB_DEBUG_PRINTF ("%ld pc=%08lx : Set Priority High\n", m_now->getCycleCount(), pc);
            fprintf (stderr, "%ld: pc=%08lx : Set Priority High (latency=%ld)\n", m_now->getCycleCount(), pc, latency);
         }
         return false;
      } else {
         inst_priority_t priority = it->second;
         if (priority == High) {
            if (latency < 30) {
               m_priority_map.erase(pc);
               // ROB_DEBUG_PRINTF ("%ld pc=%08lx : Remove Priority\n", m_now->getCycleCount(), pc);
               fprintf (stderr, "%ld: pc=%08lx : Remove Priority (latency=%ld) \n", m_now->getCycleCount(), pc, latency);
               m_priority_remove_queue.push_back (pc);
               return true;
            }
         }
         return false;
      }
   }

   inst_priority_t getPriority (UInt64 pc) {
      if (m_methodology == "static") {
         return getPriority_Static(pc);
      } else {
         // マップにキー(pc)がある場合はその値を返す
         auto it = m_priority_map.find(pc);
         if (it != m_priority_map.end()) {
            return it->second;
         }
         // ない場合はデフォルト値
         return Normal;
      }
   }

   void setPriority (UInt64 pc, inst_priority_t priority) {
      // マップにキー(pc)がない場合は新規エントリが作られる
      auto it = m_priority_map.find(pc);
      if (it == m_priority_map.end()) {
         fprintf (stderr, "setPriority pc=%08lx as %d\n", pc, priority);
      }
      m_priority_map[pc] = priority;
   }

   void removePriority (UInt64 pc) {
      m_priority_map.erase(pc);
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
};
