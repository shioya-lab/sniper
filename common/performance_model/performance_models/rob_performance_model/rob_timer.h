/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#ifndef ROBTIMER_HPP_
#define ROBTIMER_HPP_

#include "interval_timer.h"
#include "rob_contention.h"
#include "stats.h"
#include "hooks_manager.h"
#include "magic_server.h"
#include "vector_dependencies.h"
#include "register_manager.hpp"
#include "mem_stats_manager.h"
#include "priority_manager.hpp"

#include <deque>
#include <list>

#define ROB_DEBUG_PRINTF(...) { if (enable_rob_debug_seqnumber || (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle)) { fprintf(stderr, __VA_ARGS__); }}
#define KANATA_PRINTF(...) { if (m_active_kanata_gen && m_konata_count < m_konata_count_max) { fprintf(m_core->getKanataFp(), __VA_ARGS__); }}
class RobTimer
{
private:
   class RobEntry
   {
      private:
         static const size_t MAX_INLINE_DEPENDANTS = 8;
         size_t numInlineDependants;
         RobEntry* inlineDependants[MAX_INLINE_DEPENDANTS];
         std::vector<RobEntry*> *vectorDependants;
         std::vector<uint64_t> addressProducers;
         UInt64 commitDependant; // SequenceNumber

      public:
         void init(DynamicMicroOp *uop, UInt64 sequenceNumber);
         void free();

         void addDependant(RobEntry* dep);
         uint64_t getNumDependants() const;
         RobEntry* getDependant(size_t idx) const;

         void addAddressProducer(UInt64 sequenceNumber) { addressProducers.push_back(sequenceNumber); }
         UInt64 getNumAddressProducers() const { return addressProducers.size(); }
         UInt64 getAddressProducer(size_t idx) const { return addressProducers.at(idx); }

         void setCommitDependant(UInt64 sequenceNumber) { commitDependant = sequenceNumber; }
         UInt64 getCommitDependant() { return commitDependant; }

         DynamicMicroOp *uop;
         SubsecondTime fetch;
         SubsecondTime dispatched;
         SubsecondTime ready;    // Once all dependencies are resolved, cycle number that this uop becomes ready for issue
         SubsecondTime readyMax; // While some but not all dependencies are resolved, keep the time of the latest known resolving dependency
         SubsecondTime addressReady;
         SubsecondTime addressReadyMax;
         SubsecondTime issued;
         SubsecondTime done;
         UInt64        lpiq_inserted;
         UInt64        lpiq_released;

         uint64_t global_sequence_id;

         bool kanata_registered;  // Indicate Kanata Format Instruction Registered
         size_t phy_reg_index;    // Physical Register allocated index

         bool front_stall_now;
   };

   const uint64_t dispatchWidth;
   const uint64_t commitWidth;
   const uint64_t windowSize;
   const uint64_t robHwSize;
   const uint64_t rsEntries;
   const uint64_t misprediction_penalty;
   const bool m_store_to_load_forwarding;
   const bool m_no_address_disambiguation;
   const bool inorder;
   const bool vector_inorder;
   const bool lsu_inorder;
   const bool v_to_s_fence;
   const bool m_gather_scatter_merge;
   const bool m_vec_preload;
   uint64_t m_vsetvl_producer = INVALID_SEQNR;
   uint64_t m_konata_count_max;
   uint64_t m_konata_count = 0;

   Core *m_core;

   typedef CircularQueue<RobEntry> Rob;
   Rob rob;
   uint64_t m_num_in_rob;
   uint64_t m_rs_entries_used;
   RobContention *m_rob_contention;
   uint64_t m_num_in_rob_head;

   bool m_roi_started; // due to record roi_start time
   bool m_enable_o3;
   bool m_enable_kanata;
   bool m_active_o3_gen;
   bool m_active_kanata_gen;
   SubsecondTime m_last_kanata_time;
   bool m_kanata_generated_in_this_region;

   // When getMemAccessMerge=true, use this value
   uint64_t m_previous_latency;
   HitWhere::where_t m_previous_hit_where;

   ComponentTime now;
   SubsecondTime frontend_stalled_until;
   bool in_icache_miss;
   SubsecondTime last_store_done;
   ContentionModel load_queue;
   ContentionModel store_queue;
   UInt64 vec_load_queue;
   UInt64 vec_store_queue;
   UInt64 scalar_load_queue;
   UInt64 scalar_store_queue;
   bool m_cfg_bloom_filter;
   UInt64 m_vlen;
   
   uint64_t nextSequenceNumber;
   bool will_skip;
   SubsecondTime time_skipped;

   bool enable_rob_timer_log;
   UInt64 rob_start_cycle;
   UInt64 rob_debug_seqnumber;
   bool enable_rob_debug_seqnumber = false;
   bool enable_gatherscatter_log;

   RegisterDependencies* const registerDependencies;
   MemoryDependencies* const memoryDependencies;
   VectorDependencies* const vectorDependencies;

   int addressMask;

   UInt64 m_uop_type_count[MicroOp::UOP_SUBTYPE_SIZE];
   UInt64 m_uops_total;
   UInt64 m_uops_x87;
   UInt64 m_uops_pause;

   UInt64 m_inst_type_count[MicroOp::UOP_SUBTYPE_SIZE];
   UInt64 m_inst_total;

   uint64_t m_numICacheOverlapped;
   uint64_t m_numBPredOverlapped;
   uint64_t m_numDCacheOverlapped;

   uint64_t m_numLongLatencyLoads;
   uint64_t m_numTotalLongLatencyLoadLatency;

   uint64_t m_numSerializationInsns;
   uint64_t m_totalSerializationLatency;

   uint64_t m_totalHiddenDCacheLatency;
   uint64_t m_totalHiddenLongerDCacheLatency;
   uint64_t m_numHiddenLongerDCacheLatency;

   SubsecondTime m_outstandingLongLatencyInsns;
   SubsecondTime m_outstandingLongLatencyCycles;
   SubsecondTime m_lastAccountedMemoryCycle;

   uint64_t m_loads_count;
   SubsecondTime m_loads_latency;
   uint64_t m_stores_count;
   SubsecondTime m_stores_latency;

   uint64_t m_VtoS_RdRequests;
   uint64_t m_VtoS_WrRequests;

   uint64_t m_alu_num_in_rs;
   uint64_t m_lsu_num_in_rs;
   uint64_t m_fpu_num_in_rs;
   uint64_t m_vec_num_in_rs;

   uint64_t m_alu_window_size;
   uint64_t m_lsu_window_size;
   uint64_t m_fpu_window_size;
   uint64_t m_vec_window_size;

   uint64_t vec_ooo_issue_count;
   uint64_t scalar_ooo_issue_count;

   uint64_t m_inst_vec_reserve_count;

   uint64_t vector_overtake_vector_issue_count;
   uint64_t vector_overtake_scalar_issue_count;
   uint64_t scalar_overtake_vector_issue_count;
   uint64_t scalar_overtake_scalar_issue_count;

   bool m_enable_ooo_check;
   uint64_t m_ooo_check_region;
   uint64_t m_ooo_region_count;

   SubsecondTime m_latest_vecmem_commit_time;

   uint64_t m_totalProducerInsDistance;
   uint64_t m_totalConsumers;
   std::vector<uint64_t> m_producerInsDistance;

   PerformanceModel *perf;

#if DEBUG_IT_INSN_PRINT
   FILE *m_insn_log;
#endif

   uint64_t m_numMfenceInsns;
   uint64_t m_totalMfenceLatency;

   // CPI stacks
   SubsecondTime m_cpiBase;
   SubsecondTime m_cpiBranchPredictor;
   SubsecondTime m_cpiSerialization;
   SubsecondTime m_cpiALURSFull;
   SubsecondTime m_cpiFPURSFull;
   SubsecondTime m_cpiLSURSFull;
   SubsecondTime m_cpiVECRSFull;
   SubsecondTime m_cpiSPhyRegFull;
   SubsecondTime m_cpiVPhyRegFull;

   UInt64 m_statsALURSMax;
   UInt64 m_statsFPURSMax;
   UInt64 m_statsLSURSMax;
   UInt64 m_statsVECRSMax;

   SubsecondTime m_cpiLDQFull;
   SubsecondTime m_cpiSTQFull;
   SubsecondTime m_cpiVLDQFull;
   SubsecondTime m_cpiVSTQFull;

   UInt64 m_intRegisterFull;
   UInt64 m_floatRegisterFull;
   UInt64 m_vectorRegisterFull;

   typedef enum {
      None,
      ALURsFull,
      FPURsFull,
      LSURsFull,
      VECRsFull,
      LDQFull,
      STQFull,
      VLDQFull,
      VSTQFull,
      IPhyRegFull,
      FPhyRegFull,
      VPhyRegFull,
      RobFull,
      RobFullHead,
      FrontStall_Max
   } frontstall_t;

   String FrontStallString (frontstall_t idx) {
      switch(idx)
      {
         case frontstall_t::None:          return "None";
         case frontstall_t::ALURsFull:     return "ALURsFull";
         case frontstall_t::FPURsFull:     return "FPURsFull";
         case frontstall_t::LSURsFull:     return "LSURsFull";
         case frontstall_t::VECRsFull:     return "VECRsFull";
         case frontstall_t::LDQFull:       return "LDQFull";
         case frontstall_t::STQFull:       return "STQFull";
         case frontstall_t::VLDQFull:      return "VLDQFull";
         case frontstall_t::VSTQFull:      return "VSTQFull";
         case frontstall_t::IPhyRegFull:   return "IntPhyRegFull";
         case frontstall_t::FPhyRegFull:   return "FloatPhyRegFull";
         case frontstall_t::VPhyRegFull:   return "VecPhyRegFull";
         case frontstall_t::RobFull:       return "RobFull";
         case frontstall_t::RobFullHead:   return "RobFullHead";
         case frontstall_t::FrontStall_Max:return "FrontStall_Ma";
         default                        : return "?????";
      }
   }
   frontstall_t m_frontstall_idx;
   SubsecondTime m_frontstall[FrontStall_Max];

   std::vector<SubsecondTime> m_cpiInstructionCache;
   std::vector<SubsecondTime> m_cpiDataCache;

   SubsecondTime *m_cpiCurrentFrontEndStall;

   const bool m_mlp_histogram;
   static const unsigned int MAX_OUTSTANDING = 32;
   std::vector<std::vector<SubsecondTime> > m_outstandingLoads;
   std::vector<SubsecondTime> m_outstandingLoadsAll;

   std::vector<UInt64> m_bank_info;

   RobEntry *findEntryBySequenceNumber(UInt64 sequenceNumber);
   SubsecondTime* findCpiComponent();
   void countOutstandingMemop(SubsecondTime time);
   void printRob(bool is_output=true, bool enable_check=true);

   void execute(uint64_t& instructionsExecuted, SubsecondTime& latency);
   SubsecondTime doDispatch(SubsecondTime **cpiComponent);
   bool checkFrontendStall(RobEntry *entry, SubsecondTime **cpiFrontEnd);  // true: stall, false: not stall
   bool allocateRegister (RobEntry *entry, SubsecondTime **cpiFrontEnd);
   void releaseRegister (RobEntry *entry);
   SubsecondTime doIssue();
   SubsecondTime doCommit(uint64_t& instructionsExecuted);

   void issueInstruction(uint64_t idx, SubsecondTime &next_event);

   // Physical Register: Freelist
   vec_reserve_policy_t m_vec_reserve_policy;

   std::list<UInt64> m_lpiq_fifo;
   UInt64 m_lpiq_inserted;   // LPIQに挿入された回数
   UInt64 m_lpiq_overflow;   // LPIQがオーバーフローした回数

   bool   m_lowpri_inst_find_mode;       // 最長レイテンシの命令に依存する命令を探すモード
   UInt64 m_lowpri_inst_find_mode_start; // 探せないときのタイムアウトに使うカウンタ
   UInt64 m_long_latency_pc;             // 最長レイテンシのPC

   // 統計情報 : W-FIFOにどれくらいどの命令が入ったか
   std::unordered_map<UInt64, std::pair<UInt64, String>> m_lpiq_stats;  // first: PC, second: <Count, assembly>
   inline void UpdateLPIQStats(DynamicMicroOp *uop) {
      // Update stats
      auto lpiq_it = m_lpiq_stats.find(uop->getMicroOp()->getInstruction()->getAddress());
      if (lpiq_it == m_lpiq_stats.end()) {
         m_lpiq_stats.insert(std::make_pair(uop->getMicroOp()->getInstruction()->getAddress(),
                                             std::make_pair(1, uop->getMicroOp()->getInstruction()->getDisassembly()))); // Not found
      } else {
         (lpiq_it->second).first++; // Found
      }
   }

   inline bool is_vldq_assign (DynamicMicroOp *uop) {
      if (!uop->getMicroOp()->isVecLoad()) {
         return false;
      } else if (m_cfg_bloom_filter) {
         // Bloom Filter
         return uop->isFirst();
      } else {
         return true;
      }
   }

   inline bool is_vldq_release (DynamicMicroOp *uop) {
      if (!uop->getMicroOp()->isVecLoad()) {
         return false;
      } else if (m_cfg_bloom_filter) {
         // Bloom Filter
         return uop->isLast();
      } else {
         return true;
      }
   }

   // inline bool IsInLPIQ (DynamicMicroOp *uop) {
   //    for (auto id: m_lpiq_fifo) {
   //       if (id == uop->getSequenceNumber()) {
   //          return true;
   //       }
   //    }
   //    return false;
   // }

   // 統計情報 : ベクトルメモリアクセスのキャッシュ・ヒット・ミス頻度
   class dcache_stats_t {
     public:
      UInt64 hitwhere[HitWhere::NUM_HITWHERES];
      String assembly;
      dcache_stats_t() {
         for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++) {
            hitwhere[h] = 0;
         }
      }
   } ;
   std::unordered_map<UInt64, dcache_stats_t*> m_vec_dcache_stats;  // <PC, <<Hit, Miss>, assembly>>
   inline void UpdateVecDCacheStats(DynamicMicroOp *uop, int hitwhere) {
      // Update stats
      auto vec_dcache_it = m_vec_dcache_stats.find(uop->getMicroOp()->getInstruction()->getAddress());
      if (vec_dcache_it == m_vec_dcache_stats.end()) {
         dcache_stats_t *s = new dcache_stats_t();
         s->hitwhere[hitwhere] = 1;
         s->assembly = uop->getMicroOp()->getInstruction()->getDisassembly();
         m_vec_dcache_stats.insert(std::make_pair(uop->getMicroOp()->getInstruction()->getAddress(), s)); // Not found
      } else {
         // Found
         (vec_dcache_it->second)->hitwhere[hitwhere]++;
      }
   }

   size_t m_num_vecload = 0;
   size_t m_num_vecload_hit = 0;
   inline void UpdateVecLoadHit (bool is_cache_hit) {
      m_num_vecload++;
      m_num_vecload_hit = m_num_vecload_hit + is_cache_hit;
   }

   // 統計情報 : プリロードがどれくらい発行されたか
   std::unordered_map<UInt64, std::pair<UInt64, String>> m_preload_stats;  // first: PC, second: <Count, assembly>
   inline void UpdatePreloadStats(DynamicMicroOp *uop) {
      // Update stats
      auto preload_it = m_preload_stats.find(uop->getMicroOp()->getInstruction()->getAddress());
      if (preload_it == m_preload_stats.end()) {
         m_preload_stats.insert(std::make_pair(uop->getMicroOp()->getInstruction()->getAddress(),
                                               std::make_pair(1, uop->getMicroOp()->getInstruction()->getDisassembly()))); // Not found
      } else {
         (preload_it->second).first++; // Found
      }
   }


   void setVSETDependencies(DynamicMicroOp& microOp, uint64_t lowestValidSequenceNumber);

   bool UpdateNormalBindPhyRegAllocation(uint64_t rob_idx);

   bool m_1st_issue_in_cycle;

   UInt64 m_last_lpiq_sequencenumber;
   bool InsertLPIQ (DynamicMicroOp *uop, DynamicMicroOp::lpiq_t reason);
   bool InsertResRegLPIQ (DynamicMicroOp *uop) {
      return InsertLPIQ (uop, DynamicMicroOp::lpiq_t::RESREG);
   }
   bool InsertTransRegLPIQ (DynamicMicroOp *uop) {
      return InsertLPIQ (uop, DynamicMicroOp::lpiq_t::TRANSREG);
   }
   bool AllocNonpriVecRegisters (uint64_t rob_idx, DynamicMicroOp *uop, dl::Decoder::decoder_reg dest_reg);
   bool UpdateReservedBindPhyRegAllocation(uint64_t rob_idx);
   bool UpdateLateBindPhyRegAllocation(uint64_t rob_idx);
   void preloadInstruction (uint64_t idx);

   RegisterManager *m_reg_manager;

   bool ReserveVSTQ (uint64_t rob_idx);
   void releaseLPIQ ();

   bool UpdateArchRegWAW(uint64_t rob_idx);

   uint64_t m_late_bind_flush_count;
   uint64_t m_full_dispatch_stall_count;

   uint64_t m_preload_count;

   ComponentTime m_last_committed_time;

   const String m_app;
   const UInt64 m_pref_target_log;

   const bool m_vec_store_inorder;

   bool m_show_rob;

   PriorityManager *m_priority_manager; 
   void manageInstructionReserve (RobEntry *entry);
   void manageInstructionReserveVecPriority(RobEntry *entry);
   void manageInstructionReserveVecAll(RobEntry *entry);

   typedef struct {
      UInt64 pc;
      dl::Decoder::decoder_reg dest_reg;
   } vec_reg_hist_entry_t;
   std::deque<vec_reg_hist_entry_t> m_vec_reg_hist;
   std::deque<UInt64> m_high_inst_candidate;

   void RemovePriorityQueue (RobEntry *entry);
   void PropagateHighPriorityBackward (const MicroOp* uop);
   void PropagatePriorityFromForward (RobEntry *entry);

   void manageInstructionParOOO(RobEntry *entry);
   void manageInstructionStatic(RobEntry *entry);
   
   // Find First Instruction
   uint64_t findFirstUopSeqNumber (DynamicMicroOp *uop) {
      UInt64 seqnum = uop->getSequenceNumber();
      if (uop->isFirst()) {
         return seqnum;
      }
      seqnum --;
      RobEntry *firstEntry = findEntryBySequenceNumber(seqnum);
      while (!firstEntry->uop->isFirst()) {
         seqnum--;
         firstEntry = findEntryBySequenceNumber(seqnum);
      } 
      return seqnum;
   }

public:

   RobTimer(Core *core, PerformanceModel *perf, const CoreModel *core_model, int misprediction_penalty, int dispatch_width, int window_size);
   ~RobTimer();

   boost::tuple<uint64_t,SubsecondTime> simulate(const std::vector<DynamicMicroOp*>& insts);
   void synchronize(SubsecondTime time);



  static SInt64 hookRoiBegin(UInt64 object, UInt64 argument) {
    ((RobTimer*)object)->roiBegin(); return 0;
  }

  static SInt64 hookRoiEnd(UInt64 object, UInt64 argument) {
    ((RobTimer*)object)->roiEnd(); return 0;
  }

  void roiBegin() {
    m_roi_started = true;
  }

  void roiEnd() {
    std::cout << "CycleTrace " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << '\n';
    // std::cout << "CycleTrace End\n";
    m_enable_o3 = 0;
  }

   static SInt64 hookSetVL(UInt64 object, UInt64 argument) {
      MagicServer::MagicMarkerType *args = (MagicServer::MagicMarkerType *)argument;

      size_t vl = args->arg0;
      size_t vtype = args->arg1;
      size_t vsize = 8 << ((vtype >> 3) & 0x07);
      size_t vlmul = (vtype & 0x07) + 1;
      ((RobTimer *)object)->m_rob_contention->setvl(vl);
      ((RobTimer *)object)->m_rob_contention->setvtype(vsize, vlmul);

      // std::cout << "Set VL = " << vl << ", vsize = " << std::dec << vsize << ", " << "vlmul = " << vlmul << '\n';
      return 0;
   }

   void propagatePriInst (RobEntry *entry, pri_upd_result_t result);

   bool isPriInst (UInt64 pc) {
      return std::find (pri_insts.begin(), pri_insts.end(), pc) != pri_insts.end();
   }

   bool isPriInst (DynamicMicroOp &uop) {
      UInt64 pc = uop.getMicroOp()->getInstruction()->getAddress();
      return isPriInst (pc);
   }

   MemStatsManager *m_mem_stats;

   // std::unordered_map<UInt64, std::pair<UInt64, UInt64>> m_mem_stats;  // first: PC, second: <Inst Count, Latency Total>
   // void UpdateMemStats (UInt64 pc, UInt64 latency) {
   //    auto &entry = m_mem_stats[pc];
   //    entry.first++;  // 命令数をインクリメント
   //    entry.second += latency;  // 遅延時間を累計
   //    ROB_DEBUG_PRINTF("Updated Mem Status: PC=%08lx, Num=%ld, Average=%f\n",
   //                      pc, entry.first, static_cast<float>(entry.second) / entry.first);
   // }

   std::unordered_map<UInt64, std::pair<UInt64, UInt64>> m_ino_stats;  // first: PC, second: <Whole Count, Inorder Count>
   std::vector <UInt64> nonpri_insts;
   std::vector <UInt64> pri_insts;

   void UpdateInorderStats (UInt64 pc, bool executed_inorder) {
      auto ino_it = m_ino_stats.find(pc);
      if (ino_it == m_ino_stats.end()) {
         // Not found
         m_ino_stats.insert(std::make_pair(pc, std::make_pair(static_cast<UInt64>(executed_inorder), 1)));
         ROB_DEBUG_PRINTF ("Updated: PC=%08lx, new\n", pc);
      } else {
         if (executed_inorder) {
            (ino_it->second).first++;
         }
         (ino_it->second).second++;
         ROB_DEBUG_PRINTF ("Updated: PC=%08lx, InOrder=%ld, Total=%ld\n", ino_it->first, ino_it->second.first, ino_it->second.second);
      }

      // // Dump All Lists
      // for (auto ino: m_ino_stats) {
      //    ROB_DEBUG_PRINTF ("List: PC=%08lx, InOrder=%ld, Total=%ld\n", ino.first, ino.second.first, ino.second.second);
      // }
   }

   // findNonPriInsts:
   // Lookup most in-ordered issued instruction
   UInt64 findNonPriInsts () {
      float max_ino_rate = 0.0;
      UInt64 max_pc = 0;
      for (auto ino: m_ino_stats) {
         if (std::find (nonpri_insts.begin(), nonpri_insts.end(), ino.first) != nonpri_insts.end()) {
            continue;
         }
         float rate = static_cast<float>(ino.second.first) / ino.second.second;
         if (max_ino_rate < rate) {
            max_ino_rate = rate;
            max_pc = ino.first;
         }
      }

      if (max_pc != 0) {
         fprintf (stderr, "%ld : Lack of physical registers: findInorder PC=%08lx, Rate = %f\n", now.getCycleCount(), max_pc, max_ino_rate);
      }
      //

      // LOG_ASSERT_ERROR(max_pc != 0, "Valid PC should be selected.");
      return max_pc;
   }

   void AddNonPriInsts (UInt64 pc) {
      // すでにPriInstに入っているものはNonPriには入れない
      if (std::find (pri_insts.begin(), pri_insts.end(), pc) != pri_insts.end()) {
         return;
      }
      if (std::find (nonpri_insts.begin(), nonpri_insts.end(), pc) == nonpri_insts.end()) {
         nonpri_insts.push_back (pc);
         ROB_DEBUG_PRINTF ("AddNonPriInsts PC=%08lx\n", pc);
         fprintf (stderr, "Add NonPriInsts PC=%08lx\n", pc);
      }

      return;
   }

   bool IsNonPriInsts (UInt64 pc) {
      return std::find (nonpri_insts.begin(), nonpri_insts.end(), pc) != nonpri_insts.end();
   }

   void AddPriInsts (UInt64 pc) {
      if (std::find (pri_insts.begin(), pri_insts.end(), pc) == pri_insts.end()) {
         pri_insts.push_back (pc);
         fprintf (stderr, "  AddPriInsts PC=%08lx\n", pc);
      }

      // ROB_DEBUG_PRINTF ("AddPriInsts PC=%08lx\n", pc);

      return;
   }

   // 型エイリアスを定義
   //                            pc,     priority,                         Exec latency, Exec count     LPIQ latency, LPIQ count,  assembly
   using StatsEntry = std::tuple<UInt64, PriorityManager::inst_priority_t, uint32_t,     uint32_t,      uint32_t,     uint32_t,    String>; // latency と count を保持
   // 統計情報を格納するグローバルリスト
   std::vector<StatsEntry> m_vec_stats_list;

   void UpdateVectorLatencyStats (DynamicMicroOp *uop)
   {
      PriorityManager::inst_priority_t priority = uop->isReserveInst() ? PriorityManager::inst_priority_t::Reserve :
                                 uop->isStrongPriorityInst() ? PriorityManager::inst_priority_t::High :
                                 PriorityManager::inst_priority_t::Normal;
      UInt64 pc = uop->getMicroOp()->getInstruction()->getAddress();
      // 既存のエントリを検索
      for (auto& entry : m_vec_stats_list) {
         UInt64                           existingPc;
         PriorityManager::inst_priority_t existingPriority;
         uint32_t                         existingExecLatency;
         uint32_t                         existingExecCount;
         uint32_t                         existingLPIQLatency;
         uint32_t                         existingLPIQCount;
         String                           disassembly;

         std::tie(existingPc, existingPriority,
                  existingExecLatency, existingExecCount,
                  existingLPIQLatency, existingLPIQCount,
                  disassembly) = entry;

         if (existingPc == pc && existingPriority == priority) {
               // 既存エントリの latency を加算し、count を増やす
               std::get<2>(entry) += uop->getExecLatency();
               std::get<3>(entry) += 1;
               return;
         }
      }
      // 新規エントリを追加 (count は初期値 1)
      m_vec_stats_list.emplace_back(pc,
                             priority,
                             uop->getExecLatency(),
                             1,
                             0, 0,
                             uop->getMicroOp()->getInstruction()->getDisassembly());
   }

   void UpdateVectorLPIQStats (DynamicMicroOp *uop, UInt64 lpiq_latency)
   {
      PriorityManager::inst_priority_t priority = uop->isReserveInst() ? PriorityManager::inst_priority_t::Reserve :
                                 uop->isStrongPriorityInst() ? PriorityManager::inst_priority_t::High :
                                 PriorityManager::inst_priority_t::Normal;
      UInt64 pc = uop->getMicroOp()->getInstruction()->getAddress();
      // 既存のエントリを検索
      for (auto& entry : m_vec_stats_list) {
         UInt64          existingPc;
         PriorityManager::inst_priority_t existingPriority;
         uint32_t        existingExecLatency;
         uint32_t        existingExecCount;
         uint32_t        existingLPIQLatency;
         uint32_t        existingLPIQCount;
         String          disassembly;

         std::tie(existingPc, existingPriority,
                  existingExecLatency, existingExecCount,
                  existingLPIQLatency, existingLPIQCount,
                  disassembly) = entry;

         if (existingPc == pc && existingPriority == priority) {
               // 既存エントリの latency を加算し、count を増やす
               std::get<4>(entry) += lpiq_latency;
               std::get<5>(entry) += 1;
               return;
         }
      }
      // 新規エントリを追加 (count は初期値 1)
      m_vec_stats_list.emplace_back(pc,
                             priority,
                             0, 0,
                             lpiq_latency, 1,
                             uop->getMicroOp()->getInstruction()->getDisassembly());
   }

   void generateVectorStats ();

};

#endif /* ROBTIMER_H_ */
