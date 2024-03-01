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

#include <deque>

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

      public:
         void init(DynamicMicroOp *uop, UInt64 sequenceNumber);
         void free();

         void addDependant(RobEntry* dep);
         uint64_t getNumDependants() const;
         RobEntry* getDependant(size_t idx) const;

         void addAddressProducer(UInt64 sequenceNumber) { addressProducers.push_back(sequenceNumber); }
         UInt64 getNumAddressProducers() const { return addressProducers.size(); }
         UInt64 getAddressProducer(size_t idx) const { return addressProducers.at(idx); }

         DynamicMicroOp *uop;
         SubsecondTime fetch;
         SubsecondTime dispatched;
         SubsecondTime ready;    // Once all dependencies are resolved, cycle number that this uop becomes ready for issue
         SubsecondTime readyMax; // While some but not all dependencies are resolved, keep the time of the latest known resolving dependency
         SubsecondTime addressReady;
         SubsecondTime addressReadyMax;
         SubsecondTime issued;
         SubsecondTime done;

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
   uint64_t m_vsetvl_producer;
   uint64_t m_konata_count_max;

   Core *m_core;

   typedef CircularQueue<RobEntry> Rob;
   Rob rob;
   uint64_t m_num_in_rob;
   uint64_t m_rs_entries_used;
   RobContention *m_rob_contention;

   bool m_roi_started; // due to record roi_start time
   bool m_enable_o3;
   bool m_enable_kanata;
   SubsecondTime m_last_kanata_time;

   FILE *m_o3_fp;
   FILE *m_kanata_fp;

   // When getMemAccessMerge=true, use this value
   uint64_t m_previous_latency;
   HitWhere::where_t m_previous_hit_where;

   ComponentTime now;
   SubsecondTime frontend_stalled_until;
   bool in_icache_miss;
   SubsecondTime last_store_done;
   ContentionModel load_queue;
   ContentionModel store_queue;
   ContentionModel vec_load_queue;
   ContentionModel vec_store_queue;

   uint64_t nextSequenceNumber;
   bool will_skip;
   SubsecondTime time_skipped;

   bool enable_debug_printf;
   bool enable_rob_timer_log;

   RegisterDependencies* const registerDependencies;
   MemoryDependencies* const memoryDependencies;

   int addressMask;

   UInt64 m_uop_type_count[MicroOp::UOP_SUBTYPE_SIZE];
   UInt64 m_uops_total;
   UInt64 m_uops_x87;
   UInt64 m_uops_pause;

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

   uint64_t vector_overtook_by_vector_issue_count;
   uint64_t vector_overtook_by_scalar_issue_count;

   uint64_t vector_inst_issued_count;
   uint64_t scalar_inst_issued_count;

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
   void printRob();

   void execute(uint64_t& instructionsExecuted, SubsecondTime& latency);
   SubsecondTime doDispatch(SubsecondTime **cpiComponent);
   SubsecondTime doIssue();
   SubsecondTime doCommit(uint64_t& instructionsExecuted);

   void issueInstruction(uint64_t idx, SubsecondTime &next_event);

   // Physical Register: Freelist
   bool m_late_phyreg_allocation;
   typedef struct {
      SubsecondTime time;
      uint64_t      uop_idx;
   } phy_t;
   std::vector<phy_t> m_phy_list;
   uint64_t m_phyreg_max_usage;

   void setVSETDependencies(DynamicMicroOp& microOp, uint64_t lowestValidSequenceNumber);

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
    enable_debug_printf = !enable_debug_printf;
  }

  void roiEnd() {
    std::cout << "CycleTrace " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << '\n';
    // std::cout << "CycleTrace End\n";
    enable_debug_printf = !enable_debug_printf;
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

};

#endif /* ROBTIMER_H_ */
