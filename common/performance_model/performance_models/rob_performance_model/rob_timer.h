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

#include <deque>
#include <list>

#define ROB_DEBUG_PRINTF(...) { if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) { fprintf(stderr, __VA_ARGS__); }}

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

         uint64_t global_sequence_id;

         bool kanata_registered;  // Indicate Kanata Format Instruction Registered
         size_t phy_reg_index;    // Physical Register allocated index
   };

   const uint64_t dispatchWidth;
   const uint64_t commitWidth;
   const uint64_t windowSize;
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
   uint64_t m_vsetvl_producer;
   uint64_t m_konata_count_max;
   uint64_t m_konata_count = 0;

   Core *m_core;

   typedef CircularQueue<RobEntry> Rob;
   Rob rob;
   uint64_t m_num_in_rob;
   uint64_t m_rs_entries_used;
   RobContention *m_rob_contention;

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

   uint64_t nextSequenceNumber;
   bool will_skip;
   SubsecondTime time_skipped;

   bool enable_rob_timer_log;
   UInt64 rob_start_cycle;
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

   uint64_t m_alu_num_in_rob;
   uint64_t m_lsu_num_in_rob;
   uint64_t m_fpu_num_in_rob;
   uint64_t m_vec_num_in_rob;

   uint64_t m_alu_window_size;
   uint64_t m_lsu_window_size;
   uint64_t m_fpu_window_size;
   uint64_t m_vec_window_size;

   uint64_t vec_ooo_issue_count;
   uint64_t scalar_ooo_issue_count;

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
   SubsecondTime m_cpiRSFull;
   SubsecondTime m_cpiVPhyRegFull;
   SubsecondTime m_cpiVSTQFull;

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
   void printRob(bool is_only_vector = false);
   void checkRob();

   void execute(uint64_t& instructionsExecuted, SubsecondTime& latency);
   SubsecondTime doDispatch(SubsecondTime **cpiComponent);
   SubsecondTime doIssue();
   SubsecondTime doCommit(uint64_t& instructionsExecuted);

   void issueInstruction(uint64_t idx, SubsecondTime &next_event);

   // Physical Register: Freelist
   bool m_vec_late_phyreg_allocation;
   bool m_vec_reserved_allocation;
   enum RegTypes {
      IntRegister = 0,
      FloatRegister = 1,
      VectorRegister = 2
   };
   bool m_gather_always_reserve_allocation; // Gather命令は常に予約に回すオプション

   UInt64 m_phy_registers[3];  // 3-types of registers defined: Int/Float/Vector
   UInt64 m_res_reserv_registers;  // 資源予約リスト内の命令の数
   UInt64 m_max_phy_registers[3];  // 3-types of registers defined: Int/Float/Vector
   UInt64 m_maxusage_phy_registers[3];
   UInt64 m_nonpri_max_vec_phy_registers;
   UInt64 m_total_vec_phy_registers;
   UInt64 m_total_vec_phy_count;
   std::list<UInt64> m_dispatch_fifo;
   bool m_vec_wfifo_registers[32];
   UInt64 m_wfifo_inserted;   // WFIFOに挿入された回数
   UInt64 m_wfifo_overflow;   // WFIFOがオーバーフローした回数

   bool   m_lowpri_inst_find_mode;       // 最長レイテンシの命令に依存する命令を探すモード
   UInt64 m_lowpri_inst_find_mode_start; // 探せないときのタイムアウトに使うカウンタ
   UInt64 m_long_latency_pc;             // 最長レイテンシのPC

   // 統計情報 : W-FIFOにどれくらいどの命令が入ったか
   std::unordered_map<UInt64, std::pair<UInt64, String>> m_wfifo_stats;  // first: PC, second: <Count, assembly>
   inline void UpdateWFIFOStats(DynamicMicroOp *uop) {
      // Update stats
      auto wfifo_it = m_wfifo_stats.find(uop->getMicroOp()->getInstruction()->getAddress());
      if (wfifo_it == m_wfifo_stats.end()) {
         m_wfifo_stats.insert(std::make_pair(uop->getMicroOp()->getInstruction()->getAddress(),
                                             std::make_pair(1, uop->getMicroOp()->getInstruction()->getDisassembly()))); // Not found
      } else {
         (wfifo_it->second).first++; // Found
      }
   }

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

   UInt64 m_last_wfifo_sequencenumber;
   bool InsertWFIFO (DynamicMicroOp *uop, DynamicMicroOp::wfifo_t reason);
   bool InsertPhyRegWFIFO (DynamicMicroOp *uop, dl::Decoder::decoder_reg dest_reg);
   bool AllocNonpriVecRegisters (uint64_t rob_idx, DynamicMicroOp *uop, dl::Decoder::decoder_reg dest_reg);
   bool UpdateReservedBindPhyRegAllocation(uint64_t rob_idx);
   bool UpdateLateBindPhyRegAllocation(uint64_t rob_idx);
   void preloadInstruction (uint64_t idx);

   bool ReserveVSTQ (uint64_t rob_idx);
   void releaseWFIFO ();

   bool UpdateArchRegWAW(uint64_t rob_idx);

   uint64_t m_late_bind_flush_count;
   uint64_t m_full_dispatch_stall_count;

   uint64_t m_preload_count;

   ComponentTime m_last_committed_time;

   const String m_app;

   bool m_show_rob;

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

   UInt64 m_prod_pc_vregs[32];
   void UpdateProdRegister (DynamicMicroOp &uop) {
      if (uop.getMicroOp()->isVector() &&
          uop.getMicroOp()->getDestinationRegistersLength() != 0 && uop.isLast()) {
         dl::Decoder *dec = Sim()->getDecoder();
         if (dec->is_reg_vector(uop.getMicroOp()->getDestinationRegister(0))) {
            dl::Decoder::decoder_reg dest_reg = uop.getMicroOp()->getDestinationRegister(0);
            UInt64 pc = uop.getMicroOp()->getInstruction()->getAddress();
            m_prod_pc_vregs[dest_reg - 64] = pc;
         }
      }
   }

   void PropagatePriInsts (DynamicMicroOp &uop) {
      // fprintf (stderr, "PropagatePriInsts start:\n");
      for(unsigned int i = 0; i < uop.getMicroOp()->getSourceRegistersLength(); ++i) {
         dl::Decoder::decoder_reg sourceRegister = uop.getMicroOp()->getSourceRegister(i);
         // fprintf (stderr, "  source %d\n", sourceRegister);
         dl::Decoder *dec = Sim()->getDecoder();
         if (dec->is_reg_vector(sourceRegister)) {
            // fprintf (stderr, "    AddPriInsts %08lx\n", m_prod_pc_vregs[sourceRegister - 64]);
            AddPriInsts (m_prod_pc_vregs[sourceRegister - 64]);
         }
      }
   }

   bool isPriInst (UInt64 pc) {
      return std::find (pri_insts.begin(), pri_insts.end(), pc) != pri_insts.end();
   }

   bool isPriInst (DynamicMicroOp &uop) {
      UInt64 pc = uop.getMicroOp()->getInstruction()->getAddress();
      return isPriInst (pc);
   }

   // Todo: Gather命令のオペランドを生成する命令は、さらに優先命令として陽に宣言する
   // 0. どの命令PCがどのレジスタを書き込むのかをテーブルとして持っておく
   // 1. ある優先命令が実行されたとき、そのテーブルを参照してどの命令がそのオペランドを生成するかを知る
   // 2. その命令を優先命令化する

   std::unordered_map<UInt64, std::pair<UInt64, UInt64>> m_mem_stats;  // first: PC, second: <Inst Count, Latency Total>
   std::list<UInt64> m_mem_latest_access;  // 直近でアクセスしtあメモリアドレス

   void UpdateMemStats (UInt64 pc, UInt64 latency) {
      auto ino_it = m_mem_stats.find(pc);
      if (ino_it == m_mem_stats.end()) {
         // Not found
         m_mem_stats.insert(std::make_pair(pc, std::make_pair(1, latency)));
         ROB_DEBUG_PRINTF ("Updated Mem Status: PC=%08lx, Latency=%ld\n", pc, latency);
      } else {
         (ino_it->second).first++;
         (ino_it->second).second += latency;
         ROB_DEBUG_PRINTF ("Updated Mem Status: PC=%08lx, Num=%ld, Average=%f\n",
                  ino_it->first, ino_it->second.first, static_cast<float>(ino_it->second.second) / ino_it->second.first);
      }

      if (m_mem_latest_access.size() >= 64) {
         m_mem_latest_access.pop_front();
      }
      m_mem_latest_access.push_back(pc);
   }

   UInt64 findShortLatencyInsts () {

      float max_latency = 0.0;
      UInt64 max_pc = 0;

      for (auto mem: m_mem_stats) {
         if (std::find (nonpri_insts.begin(), nonpri_insts.end(), mem.first) != nonpri_insts.end()) {
            continue;
         }

         float latency = static_cast<float>(mem.second.second) / mem.second.first;
         if (latency > 10) {
            continue;
         }
         if (max_latency > latency || max_latency == 0.0) {
            max_latency = latency;
            max_pc = mem.first;
         }
      }

      if (max_pc != 0) {
         fprintf (stderr, "%ld : findShortLatencyInsts : PC=%08lx, Latency = %f\n", now.getCycleCount(), max_pc, max_latency);
      }

      return max_pc;
   }

   // 比較関数 (出現回数でソート)
   static bool compareByValue(const std::pair<int, int>& a, const std::pair<int, int>& b) {
      return a.second > b.second; // 大きい順にソート
   }

   UInt64 findLongLatencyInsts () {

      // float max_latency = 0.0;
      // UInt64 max_usage = 0;
      // UInt64 max_pc = 0;

      // 要素をカウントするための std::map を使用
      std::map<UInt64, UInt64> countMap;

      // リスト内の各要素をカウント
      for (int pc : m_mem_latest_access) {
         countMap[pc]++;
      }
      // map の内容を vector にコピーして、値（カウント）でソート
      std::vector<std::pair<UInt64, UInt64>> sortedList(countMap.begin(), countMap.end());
      // 出現回数に基づいてソート (大きい順)
      std::sort(sortedList.begin(), sortedList.end(), compareByValue);

      // // 結果を出力
      // std::cout << "  mem_access_list:" << std::endl;
      // for (const auto& pair : countMap) {
      //    std::cout << "    " << std::hex << pair.first << ": " << std::dec << pair.second << std::endl;
      // }

      for (auto mem: sortedList) {
         UInt64 pc = mem.first;

         if (std::find (pri_insts.begin(), pri_insts.end(), pc) != pri_insts.end()) {
            continue;
         }

         if (pc == 0) { continue; }
         float latency = static_cast<float>(m_mem_stats[pc].second) / m_mem_stats[pc].first;
         fprintf (stderr, "  findLongLatencyInsts : PC=%08lx, Latency = %ld, usage = %ld, recent_list = %ld\n",
                  pc, m_mem_stats[pc].second, m_mem_stats[pc].first,
                  countMap[pc]);
         if (latency > 200) {
            fprintf (stderr, "  findLongLatencyInsts : PC=%08lx, Latency = %f, usage = %ld, recent_list = %ld\n",
                     pc, latency, m_mem_stats[pc].first,
                     countMap[pc]);
            return pc;
         }
      }

      fprintf (stderr, "  findLongLatencyInsts : There are no best instruction.\n");
      return 0;

      // for (auto mem: m_mem_stats) {
      //    if (std::find (pri_insts.begin(), pri_insts.end(), mem.first) != pri_insts.end()) {
      //       continue;
      //    }
      //    // 対象のメモリアクセス命令は直近で使用されている必要がある
      //    if (std::find(m_mem_latest_access.begin(), m_mem_latest_access.end(), mem.first) == m_mem_latest_access.end()) {
      //       continue;
      //    }
      //
      //
      //    float latency = static_cast<float>(mem.second.second) / mem.second.first;
      //    if (latency <= 4) {
      //       continue;
      //    }
      //    if (max_latency < latency || max_latency == 0.0) {
      //       max_latency = latency;
      //       max_usage = mem.second.first;
      //       max_pc = mem.first;
      //    }
      // }
      //
      // if (max_pc != 0) {
      //    fprintf (stderr, "%ld : findLongLatencyInsts : PC=%08lx, Latency = %f, usage = %ld\n",
      //             now.getCycleCount(), max_pc, max_latency, max_usage);
      // }
      //
      // return max_pc;
   }


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


   bool isPriorityResourceInst (DynamicMicroOp *uop) {
      UInt64 pc = uop->getMicroOp()->getInstruction()->getAddress();
      bool is_pri = std::find(nonpri_insts.begin(), nonpri_insts.end(), pc) == nonpri_insts.end();

      return is_pri;

      // Not used
      bool is_priority_inst = false;
      UInt64 inst_address = uop->getMicroOp()->getInstruction()->getAddress();

      if (m_app == "bfs") {
         switch (inst_address) {
            case 0x142e0 : // vl1re64.v	v8, (t2)
            case 0x142e4 : // vl1re64.v	v9, (t1)

            case 0x14484 : // vl1re64.v	v12, (s9)
            case 0x14488 : // vsll.vi	v12, v12, 3
            case 0x1448c : // vluxei64.v	v13, (t6), v12

            case 0x14948 : // vle64.v	v8, (a7)
            case 0x14950 : // vsll.vi	v8, v8, 3
            case 0x14954 : // vluxei64.v	v9, (a7), v8
               // case 0x14958 : // vmslt.vx	v9, v9, zero
            case 0x14970 : // vle64.v	v10, (t3)
               // case 0x14974 : // vmv.v.i	v11, 0
               // case 0x14994 : // vmv1r.v	v0, v9
            case 0x149a4 : // vle64.v	v13, (t0)
            case 0x149a8 : // vsll.vi	v14, v13, 3
            case 0x149ac : // vluxei64.v	v14, (a2), v14
               is_priority_inst = true;
               break;
         }
      } else if (m_app == "cc") {
         switch (inst_address) {
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
               is_priority_inst = true;
         }
      } else if (m_app == "pr") {
         switch (inst_address) {
            case 0x143ac: // vle64.v	v11, (a7)
            case 0x143b0: // vsll.vi	v11, v11, 3
            case 0x143b4: // vluxei64.v	v11, (a2), v11
               is_priority_inst = true;
         }
      } else if (m_app == "sssp") {
         switch (inst_address) {
            case 0x142e0: // vle64.v	v10, (a4)
            case 0x142ec: // vsll.vi	v10, v10, 3
            case 0x142f0: // vluxei64.v	v10, (t0), v10

            case 0x14298: // vle64.v	v8, (a6)
            case 0x142a4: // vsll.vi	v8, v8, 3
            case 0x142a8: // vluxei64.v	v9, (a1), v8
               is_priority_inst = true;
         }
      } else if (m_app == "00") {
         is_priority_inst = (inst_address == 0x10692);
      } else if (m_app == "01") {
         is_priority_inst = (inst_address == 0x106a2);
      } else if (m_app == "02") { // spmv
         switch (inst_address) {
            case 0x103f6 : // vle64.v	v24, (t3)
               // case 0x103fa : // vle64.v	v8, (t1)
            case 0x103fe : // vsll.vi	v24, v24, 3
            case 0x10404 : // vluxei64.v	v24, (a3), v24
               ROB_DEBUG_PRINTF("spmv instruction %08lx is priority instruction\n", inst_address);
               is_priority_inst = true;
               break;
            default :
               ROB_DEBUG_PRINTF("spmv instruction %08lx is NOT priority instruction\n", inst_address);
               is_priority_inst = false;
               break;
         }
      } else {
         is_priority_inst = uop->getMicroOp()->isVecLoad() &&
               uop->getMicroOp()->canVecSquash();   // VLE
      }
      return is_priority_inst;
   }

};

#endif /* ROBTIMER_H_ */
