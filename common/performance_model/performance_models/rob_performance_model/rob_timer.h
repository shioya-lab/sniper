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
   UInt64 m_max_phy_registers[3];  // 3-types of registers defined: Int/Float/Vector
   UInt64 m_maxusage_phy_registers[3];
   std::list<UInt64> m_dispatch_fifo;
   bool m_vec_wfifo_registers[32];
   UInt64 m_wfifo_inserted;   // WFIFOに挿入された回数
   UInt64 m_wfifo_overflow;   // WFIFOがオーバーフローした回数

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

   bool isForceReserved(DynamicMicroOp *uop) {
      bool is_avoid_wfifo = true;
      if (m_app == "bfs") {
         is_avoid_wfifo = \
               (uop->getMicroOp()->getInstruction()->getAddress() >= 0x142e0 &&
                uop->getMicroOp()->getInstruction()->getAddress() <= 0x142f8) ||

               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14948) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14950) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14954) || // VLUXEI64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14958) || // vmslt.vx	v9, v9, zero
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14970) || // vle64.v	v10, (t3)
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14994) || // VMV1R v0, v9
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x149ac) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x149b0) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x149b4) || // VLUXEI64.v
               false;
      } else if (m_app == "cc") {
         is_avoid_wfifo =                                               \
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x1406c) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14070) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14074) || // VLUXEI64.v

               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14078) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x1407c) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14080) || // VLUXEI64.v

               (uop->getMicroOp()->getInstruction()->getAddress() == 0x140c0) || // VLUXEI64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x140e0) || // VLUXEI64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14100) || // VLUXEI64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14108) || // VLUXEI64.v
               false;
      } else if (m_app == "pr") {
         is_avoid_wfifo =                                               \
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x143ac) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x143b0) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x143b4) || // VLUXEI64.v
               false;
      } else if (m_app == "sssp") {
         is_avoid_wfifo =                                               \
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x142e0) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x142ec) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x142f0) || // VLUXEI64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x14298) || // VLE64.v
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x142a4) || // VSLL.vi
               (uop->getMicroOp()->getInstruction()->getAddress() == 0x142a8) || // VLUXEI64.v
               false;
      } else if (m_app == "00") {
         is_avoid_wfifo = (uop->getMicroOp()->getInstruction()->getAddress() == 0x10692);
      } else {
         is_avoid_wfifo = uop->getMicroOp()->isVecLoad() &&
               uop->getMicroOp()->canVecSquash();   // VLE
      }
      return !is_avoid_wfifo;
   }

};

#endif /* ROBTIMER_H_ */
