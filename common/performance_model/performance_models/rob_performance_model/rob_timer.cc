/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#include "rob_timer.h"

#include "priority_manager.hpp"
#include "tools.h"
#include "stats.h"
#include "config.hpp"
#include "core_manager.h"
#include "itostr.h"
#include "performance_model.h"
#include "core_model.h"
#include "rob_contention.h"
#include "instruction.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#define LPIQ_SIZE  (1024) * 8

// Define to get per-cycle printout of dispatch, issue, writeback stages
// #define DEBUG_PERCYCLE
//#define STOP_PERCYCLE

// Define to not skip any cycles, but assert that the skip logic is working fine
//#define ASSERT_SKIP

#define TO_STRING(VariableName) # VariableName

RobTimer::RobTimer(
         Core *core, PerformanceModel *_perf, const CoreModel *core_model,
         int misprediction_penalty,
         int dispatch_width,
         int window_size)
      : dispatchWidth(dispatch_width)
      , commitWidth(Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/commit_width", core->getId()))
      , windowSize(window_size) // windowSize = ROB length = 96 for Core2
      , robHwSize (Sim()->getCfg()->getIntArray("perf_model/core/interval_timer/rob_hw_size", core->getId()))
      , rsEntries(Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/rs_entries", core->getId()))
      , misprediction_penalty(misprediction_penalty)
      , m_store_to_load_forwarding(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/store_to_load_forwarding", core->getId()))
      , m_no_address_disambiguation(!Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/address_disambiguation", core->getId()))
      , inorder(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/in_order", core->getId()))
      , vector_inorder(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/vector_inorder", core->getId()))
      , lsu_inorder(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/lsu_inorder", core->getId()))
      , v_to_s_fence(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/v_to_s_fence", core->getId()))
      , m_gather_scatter_merge(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/gather_scatter_merge", core->getId()))
      , m_vec_preload(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/vec_preload", core->getId()))
      , m_vsetvl_producer(0)
      , m_konata_count_max(Sim()->getCfg()->getIntArray("general/konata_count_max", core->getId()))
      , m_konata_count(0)
      , m_core(core)
      , rob(window_size + 255)
      , m_num_in_rob(0)
      , m_rs_entries_used(0)
      , m_rob_contention(
         Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/issue_contention", core->getId())
         ? core_model->createRobContentionModel(core)
         : NULL)
      , m_roi_started(false)
      , m_enable_o3 (Sim()->getCfg()->getBoolArray("log/enable_o3_log", m_core->getId()))
      , m_enable_kanata (Sim()->getCfg()->getBoolArray("log/enable_kanata_log", m_core->getId()))
      , m_active_o3_gen (false)
      , m_active_kanata_gen (false)
      , now(core->getDvfsDomain())
      , frontend_stalled_until(SubsecondTime::Zero())
      , in_icache_miss(false)
      , last_store_done(SubsecondTime::Zero())
      , load_queue("rob_timer.load_queue", core->getId(), Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_loads", core->getId()))
      , store_queue("rob_timer.store_queue", core->getId(), Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_stores", core->getId()))
      , vec_load_queue (Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_vec_loads"))
      , vec_store_queue(Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_vec_stores"))
      , scalar_load_queue (Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_loads"))
      , scalar_store_queue(Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_stores"))
      , m_cfg_bloom_filter(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/bloom_filter", 0))
      , m_vlen(Sim()->getCfg()->getIntArray("general/vlen", core->getId()))
      , nextSequenceNumber(0)
      , will_skip(false)
      , time_skipped(SubsecondTime::Zero())
      , enable_rob_timer_log(Sim()->getCfg()->getBoolArray("log/enable_rob_timer_log", core->getId()))
      , rob_start_cycle(Sim()->getCfg()->getIntArray("log/rob_debug_start_cycle", core->getId()))
      , rob_debug_seqnumber (Sim()->getCfg()->getIntArray("log/rob_debug_seqnumber", core->getId()))
      , enable_gatherscatter_log(Sim()->getCfg()->getBoolArray("log/enable_gatherscatter_log", core->getId()))
      , registerDependencies(new RegisterDependencies())
      , memoryDependencies(new MemoryDependencies())
      , vectorDependencies(new VectorDependencies())
      , m_enable_ooo_check(Sim()->getCfg()->getBoolArray("log/enable_mem_ooo_check", core->getId()))
      , m_ooo_check_region(Sim()->getCfg()->getIntArray("log/mem_ooo_check_region", core->getId()))
      , perf(_perf)
      , m_cpiCurrentFrontEndStall(NULL)
      , m_mlp_histogram(Sim()->getCfg()->getBoolArray("perf_model/core/rob_timer/mlp_histogram", core->getId()))
      , m_bank_info(Sim()->getCfg()->getInt("perf_model/l1_dcache/num_banks"))
      , m_vec_reserve_policy (Sim()->getCfg()->getString("perf_model/core/rob_timer/vec_reserve_policy") == "alloc_when_full" ? VecReserveWhenFull   : 
                              Sim()->getCfg()->getString("perf_model/core/rob_timer/vec_reserve_policy") == "alloc_dynamic"   ? VecReserveDynamic    :
			                     Sim()->getCfg()->getString("perf_model/core/rob_timer/vec_reserve_policy") == "alloc_vecparooo" ? VecReserveParOOO     :
                              Sim()->getCfg()->getString("perf_model/core/rob_timer/vec_reserve_policy") == "alloc_stacic"    ? VecReserveStatic     :
                              Sim()->getCfg()->getString("perf_model/core/rob_timer/vec_reserve_policy") == "alloc_always"    ? VecReserveAlways     : 
			      VecReserveNone)
      , m_last_committed_time(core->getDvfsDomain())
      , m_app(Sim()->getCfg()->getString("general/app"))
      , m_pref_target_log(strtol(Sim()->getCfg()->getStringArray("log/vec_pref_target_pc", core->getId()).c_str(), NULL, 16))
      , m_vec_store_inorder (Sim()->getCfg()->getBoolArray("research_option/vec_store_inorder", core->getId()))  // Vector Store 命令のみインオーダで実行する
{

   registerStatsMetric("rob_timer", core->getId(), "time_skipped", &time_skipped);

   for(int i = 0; i < MicroOp::UOP_SUBTYPE_SIZE; ++i)
   {
      m_uop_type_count[i] = 0;
      registerStatsMetric("rob_timer", core->getId(), String("uop_") + MicroOp::getSubtypeString(MicroOp::uop_subtype_t(i)), &m_uop_type_count[i]);
   }
   m_uops_total = 0;
   m_uops_x87 = 0;
   m_uops_pause = 0;
   registerStatsMetric("rob_timer", core->getId(), "uops_total", &m_uops_total);
   registerStatsMetric("rob_timer", core->getId(), "uops_x87", &m_uops_x87);
   registerStatsMetric("rob_timer", core->getId(), "uops_pause", &m_uops_pause);

   for(int i = 0; i < MicroOp::UOP_SUBTYPE_SIZE; ++i) {
      m_inst_type_count[i] = 0;
      registerStatsMetric("rob_timer", core->getId(), String("inst_") + MicroOp::getSubtypeString(MicroOp::uop_subtype_t(i)), &m_inst_type_count[i]);
   }
   m_inst_total = 0;
   registerStatsMetric("rob_timer", core->getId(), "inst_total", &m_inst_total);

   m_numSerializationInsns = 0;
   m_totalSerializationLatency = 0;

   registerStatsMetric("rob_timer", core->getId(), "numSerializationInsns", &m_numSerializationInsns);
   registerStatsMetric("rob_timer", core->getId(), "totalSerializationLatency", &m_totalSerializationLatency);

   m_totalHiddenDCacheLatency = 0;
   registerStatsMetric("rob_timer", core->getId(), "totalHiddenDCacheLatency", &m_totalHiddenDCacheLatency);

   m_numMfenceInsns = 0;
   m_totalMfenceLatency = 0;

   registerStatsMetric("rob_timer", core->getId(), "numMfenceInsns", &m_numMfenceInsns);
   registerStatsMetric("rob_timer", core->getId(), "totalMfenceLatency", &m_totalMfenceLatency);

   m_cpiBase = SubsecondTime::Zero();
   m_cpiBranchPredictor = SubsecondTime::Zero();
   m_cpiSerialization = SubsecondTime::Zero();
   m_cpiALURSFull = SubsecondTime::Zero();
   m_cpiFPURSFull = SubsecondTime::Zero();
   m_cpiLSURSFull = SubsecondTime::Zero();
   m_cpiVECRSFull = SubsecondTime::Zero();
   m_cpiSPhyRegFull = SubsecondTime::Zero();
   m_cpiVPhyRegFull = SubsecondTime::Zero();

   m_cpiLDQFull  = SubsecondTime::Zero();
   m_cpiSTQFull  = SubsecondTime::Zero();
   m_cpiVLDQFull = SubsecondTime::Zero();
   m_cpiVSTQFull = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "cpiBase", &m_cpiBase);
   registerStatsMetric("rob_timer", core->getId(), "cpiBranchPredictor", &m_cpiBranchPredictor);
   registerStatsMetric("rob_timer", core->getId(), "cpiSerialization", &m_cpiSerialization);
   registerStatsMetric("rob_timer", core->getId(), "cpiALURSFull", &m_cpiALURSFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiFPURSFull", &m_cpiFPURSFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiLSURSFull", &m_cpiLSURSFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiVECRSFull", &m_cpiVECRSFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiSPhyRegFull", &m_cpiSPhyRegFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiVPhyRegFull", &m_cpiVPhyRegFull);

   registerStatsMetric("rob_timer", core->getId(), "cpiLDQFull",  &m_cpiLDQFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiSTQFull",  &m_cpiSTQFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiVLDQFull", &m_cpiVLDQFull);
   registerStatsMetric("rob_timer", core->getId(), "cpiVSTQFull", &m_cpiVSTQFull);

   // Issue Queueの最大数
   m_statsALURSMax = 0;
   m_statsFPURSMax = 0;
   m_statsLSURSMax = 0;
   m_statsVECRSMax = 0;
   registerStatsMetric("rob_timer", core->getId(), "statsALURSMax", &m_statsALURSMax);
   registerStatsMetric("rob_timer", core->getId(), "statsFPURSMax", &m_statsFPURSMax);
   registerStatsMetric("rob_timer", core->getId(), "statsLSURSMax", &m_statsLSURSMax);
   registerStatsMetric("rob_timer", core->getId(), "statsVECRSMax", &m_statsVECRSMax);

   m_intRegisterFull    = 0;
   m_floatRegisterFull  = 0;
   m_vectorRegisterFull = 0;

   registerStatsMetric("rob_timer", core->getId(), "intRegisterFull",    &m_intRegisterFull);
   registerStatsMetric("rob_timer", core->getId(), "floatRegisterFull",  &m_floatRegisterFull);
   registerStatsMetric("rob_timer", core->getId(), "vectorRegisterFull", &m_vectorRegisterFull);

   m_cpiInstructionCache.resize(HitWhere::NUM_HITWHERES, SubsecondTime::Zero());
   for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++)
   {
      if (HitWhereIsValid((HitWhere::where_t)h))
      {
         String name = "cpiInstructionCache" + String(HitWhereString((HitWhere::where_t)h));
         registerStatsMetric("rob_timer", core->getId(), name, &(m_cpiInstructionCache[h]));
      }
   }
   m_cpiDataCache.resize(HitWhere::NUM_HITWHERES, SubsecondTime::Zero());
   for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++)
   {
      if (HitWhereIsValid((HitWhere::where_t)h))
      {
         String name = "cpiDataCache" + String(HitWhereString((HitWhere::where_t)h));
         registerStatsMetric("rob_timer", core->getId(), name, &(m_cpiDataCache[h]));
      }
   }

   for (size_t i = 0; i < frontstall_t::FrontStall_Max; i++){
      m_frontstall[i] = SubsecondTime::Zero();
      String name = "frontStall" + FrontStallString(frontstall_t(i));
      registerStatsMetric("rob_timer", core->getId(), name, &(m_frontstall[i]));
   }

   m_outstandingLongLatencyCycles = SubsecondTime::Zero();
   m_outstandingLongLatencyInsns = SubsecondTime::Zero();
   m_lastAccountedMemoryCycle = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "outstandingLongLatencyInsns", &m_outstandingLongLatencyInsns);
   registerStatsMetric("rob_timer", core->getId(), "outstandingLongLatencyCycles", &m_outstandingLongLatencyCycles);

   m_loads_count = 0;
   m_loads_latency = SubsecondTime::Zero();
   m_stores_count = 0;
   m_stores_latency = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "loads-count", &m_loads_count);
   registerStatsMetric("rob_timer", core->getId(), "loads-latency", &m_loads_latency);
   registerStatsMetric("rob_timer", core->getId(), "stores-count", &m_stores_count);
   registerStatsMetric("rob_timer", core->getId(), "stores-latency", &m_stores_latency);

   m_totalProducerInsDistance = 0;
   m_totalConsumers = 0;
   m_producerInsDistance.resize(windowSize, 0);

   registerStatsMetric("rob_timer", core->getId(), "totalProducerInsDistance", &m_totalProducerInsDistance);
   registerStatsMetric("rob_timer", core->getId(), "totalConsumers", &m_totalConsumers);
   for (unsigned int i = 0; i < m_producerInsDistance.size(); i++)
   {
      String name = "producerInsDistance[" + itostr(i) + "]";
      registerStatsMetric("rob_timer", core->getId(), name, &(m_producerInsDistance[i]));
   }

   if (m_mlp_histogram)
   {
      m_outstandingLoads.resize(HitWhere::NUM_HITWHERES);
      for (unsigned int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++)
      {
         if (HitWhereIsValid((HitWhere::where_t)h))
         {
            m_outstandingLoads[h].resize(MAX_OUTSTANDING, SubsecondTime::Zero());
            for(unsigned int i = 0; i < MAX_OUTSTANDING; ++i)
            {
               String name = String("outstandingLoads.") + HitWhereString((HitWhere::where_t)h) + "[" + itostr(i) + "]";
               registerStatsMetric("rob_timer", core->getId(), name, &(m_outstandingLoads[h][i]));
            }
         }
      }

      m_outstandingLoadsAll.resize(MAX_OUTSTANDING, SubsecondTime::Zero());
      for(unsigned int i = 0; i < MAX_OUTSTANDING; ++i)
      {
         String name = String("outstandingLoadsAll") + "[" + itostr(i) + "]";
         registerStatsMetric("rob_timer", core->getId(), name, &(m_outstandingLoadsAll[i]));
      }
   }

   registerStatsMetric("rob_timer", core->getId(), "VtoS_RdRequests", &m_VtoS_RdRequests);
   registerStatsMetric("rob_timer", core->getId(), "VtoS_WrRequests", &m_VtoS_WrRequests);

   m_VtoS_RdRequests = 0;
   m_VtoS_WrRequests = 0;

   Sim()->getHooksManager()->registerHook(HookType::HOOK_ROI_BEGIN, RobTimer::hookRoiBegin, (UInt64)this);
   Sim()->getHooksManager()->registerHook(HookType::HOOK_ROI_END, RobTimer::hookRoiEnd, (UInt64)this);
   Sim()->getHooksManager()->registerHook(HookType::HOOK_MAGIC_USER, RobTimer::hookSetVL, (UInt64)this);
   m_last_kanata_time = SubsecondTime::Zero();
   m_kanata_generated_in_this_region = false;

   m_alu_window_size = Sim()->getCfg()->getIntArray("perf_model/core/interval_timer/alu_window_size", core->getId());
   m_lsu_window_size = Sim()->getCfg()->getIntArray("perf_model/core/interval_timer/lsu_window_size", core->getId());
   m_fpu_window_size = Sim()->getCfg()->getIntArray("perf_model/core/interval_timer/fpu_window_size", core->getId());
   m_vec_window_size = Sim()->getCfg()->getIntArray("perf_model/core/interval_timer/vec_window_size", core->getId());

   m_alu_num_in_rs = 0;
   m_lsu_num_in_rs = 0;
   m_fpu_num_in_rs = 0;
   m_vec_num_in_rs = 0;

   m_latest_vecmem_commit_time = SubsecondTime::Zero();

   registerStatsMetric("rob_timer", core->getId(), "vec-ooo-issue",    &vec_ooo_issue_count);
   registerStatsMetric("rob_timer", core->getId(), "scalar-ooo-issue", &scalar_ooo_issue_count);

   registerStatsMetric("rob_timer", core->getId(), "inst-vec-reserve-count",    &m_inst_vec_reserve_count);

   registerStatsMetric("rob_timer", core->getId(), "vec-vec-ooo-issue",       &vector_overtake_vector_issue_count);
   registerStatsMetric("rob_timer", core->getId(), "scalar-vec-ooo-issue",    &scalar_overtake_vector_issue_count);
   registerStatsMetric("rob_timer", core->getId(), "vec-scalar-ooo-issue",    &vector_overtake_scalar_issue_count);
   registerStatsMetric("rob_timer", core->getId(), "scalar-scalar-ooo-issue", &scalar_overtake_scalar_issue_count);

   vector_overtake_vector_issue_count = 0;
   scalar_overtake_vector_issue_count = 0;
   vector_overtake_scalar_issue_count = 0;
   scalar_overtake_scalar_issue_count = 0;

   assert((m_ooo_check_region != 0) && !(m_ooo_check_region & (m_ooo_check_region - 1)));
   registerStatsMetric("rob_timer", core->getId(), "ooo_reorder_count", &m_ooo_region_count);

   m_last_lpiq_sequencenumber = 0;

   registerStatsMetric("rob_timer", core->getId(), "phyreg_late_bind_flush_count"    , &m_late_bind_flush_count);
   registerStatsMetric("rob_timer", core->getId(), "phyreg_full_dispatch_stall_count", &m_full_dispatch_stall_count);
   m_late_bind_flush_count = 0;
   m_full_dispatch_stall_count = 0;

   registerStatsMetric("rob_timer", core->getId(), "lpiq_inserted", &m_lpiq_inserted);
   registerStatsMetric("rob_timer", core->getId(), "lpiq_overflow", &m_lpiq_overflow);
   m_lpiq_inserted = 0;
   m_lpiq_overflow = 0;

   registerStatsMetric("rob_timer", core->getId(), "num_vecload", &m_num_vecload);
   registerStatsMetric("rob_timer", core->getId(), "num_vecload_l1d_hit", &m_num_vecload_hit);

   registerStatsMetric ("rob_timer", core->getId(), "preload_count", &m_preload_count);
   m_preload_count = 0;

   m_lowpri_inst_find_mode = false;

   m_mem_stats = new MemStatsManager(&now, &enable_rob_timer_log, &rob_start_cycle);

   m_reg_manager = new RegisterManager (core->getId(), m_vec_reserve_policy);
   // if (isUseNonpriVector (m_vec_reserve_policy)) {
   m_priority_manager = new PriorityManager (&now, m_vec_reserve_policy);
   // } else {
   //    m_priority_manager = NULL;
   // }
}

RobTimer::~RobTimer()
{
   for(Rob::iterator it = this->rob.begin(); it != this->rob.end(); ++it)
      it->free();

   // W-FIFOを使用した命令の頻度順でソートして出力する
   std::vector<std::pair<UInt64, std::pair<UInt64, String>>> v;
   for (auto it = m_lpiq_stats.begin(); it != m_lpiq_stats.end(); it++) {
      // (count, (PC, assembly))
      v.push_back(std::make_pair((it->second).first,
                                 std::make_pair(it->first, (it->second).second)));
   }
   std::sort(v.begin(), v.end());
   // Counting up all LPIQ usage:
   UInt64 lpiq_total = 0;
   for (auto it = v.begin(); it != v.end(); it++) { lpiq_total += it->first; }
   std::cout << "-----------\n";
   std::cout << "LPIQ usage (" << std::dec << lpiq_total << ")\n";
   std::cout << "-----------\n";
   for (auto it = v.begin(); it != v.end(); it++) {
      std::cout << std::hex << (it->second).first << ", " << std::dec << it->first << " : " << (it->second).second << '\n';
   }

   // Vector Dcache Stats を出力する
   std::cout << "-------------------\n";
   std::cout << "Vector DCache usage\n";
   std::cout << "-------------------\n";
   for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++) {
      String name = String(HitWhereString((HitWhere::where_t)h));
      std::cout << name << ", ";
   }
   std::cout << '\n';
   for (auto it = m_vec_dcache_stats.begin(); it != m_vec_dcache_stats.end(); it++) {
      // Calculate L1/L2 hit rate
      UInt64 hit_count = 0, whole_count = 0;
      for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++) {
         whole_count += (it->second)->hitwhere[h];
         if (h != HitWhere::DRAM) {
            hit_count += (it->second)->hitwhere[h];
         }
      }
      if (whole_count < 100) {
         continue;
      }
      std::cout << std::hex << it->first << ", ";
      std::cout << std::fixed << std::setprecision(3) << (static_cast<double>(hit_count)/whole_count) << ", ";

      for (int h = HitWhere::WHERE_FIRST ; h < HitWhere::NUM_HITWHERES ; h++) {
         std::cout << std::dec << (it->second)->hitwhere[h] << ", ";
      }
      std::cout << " : " << (it->second)->assembly << '\n';
   }

   // std::cout << "-------------------\n";
   // std::cout << "Preload usage\n";
   // std::cout << "-------------------\n";
   // for (auto it = m_preload_stats.begin(); it != m_preload_stats.end(); it++) {
   //    std::cout << std::hex << it->first << ", " << std::dec << (it->second).first << " : " << (it->second).second << '\n';
   // }

   generateVectorStats();
   // std::cout << "-------------------------------\n";
   // std::cout << "Vector Instruction Statistics\n";
   // std::cout << "-------------------------------\n";
   // for (auto& entry : m_vec_stats_list) {
   //    fprintf(stderr, "PC=%08lx, %s, %10d, average = %7.2lf",
   //          std::get<0>(entry),
   //          std::get<1>(entry) == PriorityManager::inst_priority_t::Reserve ? "Reserve" :
   //          std::get<1>(entry) == PriorityManager::inst_priority_t::High    ? "High   " : "Normal ",
   //          std::get<3>(entry),
   //          static_cast<double>(std::get<2>(entry)) / static_cast<double>(std::get<3>(entry)));

   //    if (std::get<5>(entry) != 0) {
   //       fprintf(stderr, ", %10d, lpiq_average = %7.2lf",
   //               std::get<5>(entry),
   //               static_cast<double>(std::get<4>(entry)) / static_cast<double>(std::get<5>(entry)));
   //    } else {
   //       fprintf(stderr, ",           ,                  ");
   //    }

   //    fprintf(stderr, ", %s\n", std::get<6>(entry).c_str());
   // }

   delete m_mem_stats;

}

void RobTimer::generateVectorStats ()
{
   // 例：m_vec_stats_listは以下のような型のコンテナと仮定する
   // std::vector<std::tuple<unsigned long, PriorityManager::inst_priority_t, int, int, int, int, std::string>> m_vec_stats_list;

   // 集計用の構造体（各優先度ごと）
   struct PriorityAggregated {
      int total_cycles = 0;
      int inst_count = 0;
      int total_lpiq_cycles = 0;
      int lpiq_count = 0;
      std::string comments;
   };

   // 各PCごとの集計情報
   struct PCAggregated {
      unsigned long pc;
      int total_inst_count = 0; // このPCの全優先度の命令数の合計（ソート用）
      // 出力順は：0 = High, 1 = Normal, 2 = Reserve
      PriorityAggregated prio[3];
   };

   //  std::vector<std::tuple<unsigned long, PriorityManager::inst_priority_t, int, int, int, int, std::string>> m_vec_stats_list;
   // ※ここに m_vec_stats_list のデータが入っていると仮定

   // PCごとの集計用マップ
   std::map<unsigned long, PCAggregated> aggregated;

   // 各レコードをPCで集計
   for (auto& entry : m_vec_stats_list) {
      unsigned long pc         = std::get<0>(entry);
      PriorityManager::inst_priority_t prio_enum = std::get<1>(entry);
      int cycles               = std::get<2>(entry);
      int count                = std::get<3>(entry);
      int lpiq_cycles          = std::get<4>(entry);
      int lpiq_count           = std::get<5>(entry);
      const auto &comment      = std::get<6>(entry);
      // 優先度を出力順に合わせる：High→Normal→Reserve

      int index;
      if (prio_enum == PriorityManager::inst_priority_t::High)
          index = 0;
      else if (prio_enum == PriorityManager::inst_priority_t::Reserve)
          index = 2;
      else
          index = 1; // Normal（それ以外の場合）

      if (aggregated.find(pc) == aggregated.end()) {
         PCAggregated agg;
         agg.pc = pc;
         aggregated[pc] = agg;
      }
      aggregated[pc].total_inst_count += count;
      PriorityAggregated &pagg = aggregated[pc].prio[index];
      pagg.total_cycles       += cycles;
      pagg.inst_count         += count;
      pagg.total_lpiq_cycles  += lpiq_cycles;
      pagg.lpiq_count         += lpiq_count;
      if (!comment.empty()) {
         if (!pagg.comments.empty())
            pagg.comments += " | ";
         pagg.comments += comment.c_str();
      }
   }

   // マップをvectorに移して、命令出現頻度（命令数合計）順にソート（降順）
   std::vector<PCAggregated> sorted;
   for (auto &pair : aggregated) {
       sorted.push_back(pair.second);
   }
   std::sort(sorted.begin(), sorted.end(), [](const PCAggregated &a, const PCAggregated &b) {
       return a.total_inst_count > b.total_inst_count;
   });

   // ヘッダ出力
   std::cout << "------------------------------------------------------------\n";
   std::cout << "Aggregated Vector Instruction Statistics\n";
   std::cout << "------------------------------------------------------------\n";

   // 各PCについて出力
   // 各ブロックは固定フォーマットで出力し、各優先度ごとの情報がなるべく同じ位置に揃うようにする
   const char* prio_names[3] = {"High   ", "Normal ", "Reserve"};
   for (auto &entry : sorted) {
      // 各優先度ごとに出力（固定のカラム幅）
      for (int i = 0; i < 3; i++) {
         PriorityAggregated &p = entry.prio[i];
         if (p.inst_count > 0) {
            double avg = static_cast<double>(p.total_cycles) / p.inst_count;
            if (p.lpiq_count != 0) {
                double lpiq_avg = static_cast<double>(p.total_lpiq_cycles) / p.lpiq_count;
                fprintf(stderr, "PC=%08lx | %s: %10d, avg = %7.2lf, %10d, lpiq_avg = %7.2lf, %s\n",
                        entry.pc, prio_names[i], p.inst_count, avg, p.lpiq_count, lpiq_avg, p.comments.c_str());
            } else {
                fprintf(stderr, "PC=%08lx | %s: %10d, avg = %7.2lf, %10s, lpiq_avg = %7s, %s\n",
                        entry.pc, prio_names[i], p.inst_count, avg, "", "", p.comments.c_str());
            }
         }
      }
   }
}

void RobTimer::RobEntry::init(DynamicMicroOp *_uop, UInt64 sequenceNumber)
{
   ready = SubsecondTime::MaxTime();
   readyMax = SubsecondTime::Zero();
   addressReady = SubsecondTime::MaxTime();
   addressReadyMax = SubsecondTime::Zero();
   issued = SubsecondTime::MaxTime();
   done = SubsecondTime::MaxTime();

   uop = _uop;
   uop->setSequenceNumber(sequenceNumber);

   addressProducers.clear();

   numInlineDependants = 0;
   vectorDependants = NULL;

   commitDependant = 0;

   kanata_registered = false;
   front_stall_now = false;
}

void RobTimer::RobEntry::free()
{
   delete uop;
   if (vectorDependants)
      delete vectorDependants;
}

void RobTimer::RobEntry::addDependant(RobTimer::RobEntry* dep)
{
   if (numInlineDependants < MAX_INLINE_DEPENDANTS)
   {
      inlineDependants[numInlineDependants++] = dep;
   }
   else
   {
      if (vectorDependants == NULL)
      {
         vectorDependants = new std::vector<RobEntry*>();
      }
      vectorDependants->push_back(dep);
   }
}

uint64_t RobTimer::RobEntry::getNumDependants() const
{
   return numInlineDependants + (vectorDependants ? vectorDependants->size() : 0);
}

RobTimer::RobEntry* RobTimer::RobEntry::getDependant(size_t idx) const
{
   if (idx < MAX_INLINE_DEPENDANTS)
   {
      LOG_ASSERT_ERROR(idx < numInlineDependants, "Invalid idx %d", idx);
      return inlineDependants[idx];
   }
   else
   {
      LOG_ASSERT_ERROR(idx - MAX_INLINE_DEPENDANTS < vectorDependants->size(), "Invalid idx %d", idx);
      return (*vectorDependants)[idx - MAX_INLINE_DEPENDANTS];
   }
}

RobTimer::RobEntry *RobTimer::findEntryBySequenceNumber(UInt64 sequenceNumber)
{
   // Assumption: MicroOps in the ROB are numbered sequentially, none of them are removed halfway
   UInt64 first = rob[0].uop->getSequenceNumber();
   UInt64 position = sequenceNumber - first;
   LOG_ASSERT_ERROR(position < rob.size(), "Sequence number %ld outside of ROB", sequenceNumber);
   RobEntry *entry = &rob[position];
   LOG_ASSERT_ERROR(entry->uop->getSequenceNumber() == sequenceNumber, "Sequence number %ld unexpectedly not at ROB position %ld", sequenceNumber, position);
   return entry;
}

boost::tuple<uint64_t,SubsecondTime> RobTimer::simulate(const std::vector<DynamicMicroOp*>& insts)
{
   uint64_t totalInsnExec = 0;
   SubsecondTime totalLat = SubsecondTime::Zero();


   for (std::vector<DynamicMicroOp*>::const_iterator it = insts.begin(); it != insts.end(); it++ )
   {
      if ((*it)->isSquashed())
      {
         delete *it;
         continue;
      }

      RobEntry *entry = &this->rob.next();
      entry->init(*it, nextSequenceNumber++);

      // Add = calculate dependencies, add yourself to list of depenants
      // If no dependants in window: set ready = now()
      uint64_t lowestValidSequenceNumber = this->rob.size() > 0 ? this->rob.front().uop->getSequenceNumber() : 0;

      if (rob_debug_seqnumber != 0 && lowestValidSequenceNumber > 0)
      {
         if (entry->uop->getSequenceNumber() > rob_debug_seqnumber && 
             entry->uop->getSequenceNumber() < rob_debug_seqnumber + 100) {
            fprintf (stderr, "Debug SequenceNuber (%ld) enable.\n", entry->uop->getSequenceNumber());
            enable_rob_debug_seqnumber = true;
         } else {
            enable_rob_debug_seqnumber = false;
         }
      }

      if (entry->uop->getMicroOp()->isStore())
      {
         for(unsigned int i = 0; i < entry->uop->getMicroOp()->getAddressRegistersLength(); ++i)
         {
            dl::Decoder::decoder_reg reg = entry->uop->getMicroOp()->getAddressRegister(i);
            uint64_t addressProducer = this->registerDependencies->peekProducer(reg, lowestValidSequenceNumber);
            if (addressProducer != INVALID_SEQNR)
            {
               RobEntry *prodEntry = this->findEntryBySequenceNumber(addressProducer);
               if (prodEntry->done != SubsecondTime::MaxTime())
                  entry->addressReadyMax = std::max(entry->addressReadyMax, prodEntry->done);
               else
                  entry->addAddressProducer(addressProducer);
            }
         }
         if (entry->getNumAddressProducers() == 0)
            entry->addressReady = entry->addressReadyMax;
      }

      if (entry->uop->getMicroOp()->isVecMem()) { // Vector命令のためにAddressReadyを追加しておく
         for(unsigned int i = 0; i < entry->uop->getMicroOp()->getAddressRegistersLength(); ++i) {
            dl::Decoder::decoder_reg reg = entry->uop->getMicroOp()->getAddressRegister(i);
            uint64_t addressProducer = this->registerDependencies->peekProducer(reg, lowestValidSequenceNumber);
            if (addressProducer != INVALID_SEQNR) {
               RobEntry *prodEntry = this->findEntryBySequenceNumber(addressProducer);
               if (prodEntry->done != SubsecondTime::MaxTime())
                  entry->addressReadyMax = std::max(entry->addressReadyMax, prodEntry->done);
               else
                  entry->addAddressProducer(addressProducer);
            }
         }
         if (entry->getNumAddressProducers() == 0)
            entry->addressReady = entry->addressReadyMax;
      }

      this->registerDependencies->setDependencies(*entry->uop, lowestValidSequenceNumber);
      UInt64 firstUopSeqNum = findFirstUopSeqNumber(entry->uop);
      this->memoryDependencies->setDependencies(*entry->uop, lowestValidSequenceNumber, firstUopSeqNum);
      this->vectorDependencies->setDependencies(*entry->uop);

      setVSETDependencies (*entry->uop, lowestValidSequenceNumber);

      manageInstructionReserve(entry); // 命令の予約を制御する

      if (m_store_to_load_forwarding && entry->uop->getMicroOp()->isLoad() &&
          !entry->uop->getMicroOp()->isVector()) // In Vector, remove dependency for forwarding not support.
      {
         for(unsigned int i = 0; i < entry->uop->getDependenciesLength(); ++i)
         {
            RobEntry *prodEntry = this->findEntryBySequenceNumber(entry->uop->getDependency(i));
            // If we depend on a store
            if (prodEntry->uop->getMicroOp()->isStore() &&
                !prodEntry->uop->getMicroOp()->isVector())  // In Vector, remove dependency for forwarding not support.
            {
               // Remove dependency on the store (which won't execute until it reaches the front of the ROB)
               entry->uop->removeDependency(entry->uop->getDependency(i));

               // Add dependencies to the producers of the value being stored instead
               // Remark: one of these may be producing the store address, but because the store has to be
               //         disambiguated, it's correct to have the load depend on the address producers as well.
               for(unsigned int j = 0; j < prodEntry->uop->getDependenciesLength(); ++j) {
                 entry->uop->addDependency(prodEntry->uop->getDependency(j));
               }
               break;
            }
         }
      }

      // Add ourselves to the dependants list of the uops we depend on
      uint64_t minProducerDistance = UINT64_MAX;
      m_totalConsumers += 1 ;
      uint64_t deps_to_remove[128], num_dtr = 0;
      for(unsigned int i = 0; i < entry->uop->getDependenciesLength(); ++i)
      {
         RobEntry *prodEntry = this->findEntryBySequenceNumber(entry->uop->getDependency(i));
         minProducerDistance = std::min( minProducerDistance,  entry->uop->getSequenceNumber() - prodEntry->uop->getSequenceNumber() );
         if (prodEntry->done != SubsecondTime::MaxTime())
         {
            // If producer is already done (but hasn't reached writeback stage), remove it from our dependency list
            deps_to_remove[num_dtr++] = entry->uop->getDependency(i);
            entry->readyMax = std::max(entry->readyMax, prodEntry->done);
            LOG_ASSERT_ERROR(num_dtr < 128, "dependency list exceeds 128");
         }
         else
         {
            prodEntry->addDependant(entry);
         }
      }

      // Make sure we are in the dependant list of all of our address producers
      for(unsigned int i = 0; i < entry->getNumAddressProducers(); ++i)
      {
        if (rob.size() && entry->getAddressProducer(i) >= rob[0].uop->getSequenceNumber())
        {
          RobEntry *prodEntry = this->findEntryBySequenceNumber(entry->getAddressProducer(i));
          bool found = false;
          for(unsigned int j = 0; j < prodEntry->getNumDependants(); ++j)
            if (prodEntry->getDependant(j) == entry)
            {
              found = true;
              break;
            }
          LOG_ASSERT_ERROR(found == true, "Store %ld depends on %ld for address production, but is not in its dependants list",
                           entry->uop->getSequenceNumber(), prodEntry->uop->getSequenceNumber());
        }
      }

      if (minProducerDistance != UINT64_MAX)
      {
         m_totalProducerInsDistance += minProducerDistance;
         // KENZO: not sure why the distance can be larger than the windowSize, but it happens...
         if (minProducerDistance >= m_producerInsDistance.size())
            minProducerDistance = m_producerInsDistance.size()-1;
         m_producerInsDistance[ minProducerDistance ]++ ;
      }
      else
      {
         // Not depending on any instruction in the rob
         m_producerInsDistance[ 0 ] += 1 ;
      }

      // If there are any dependencies to be removed, do this after iterating over them (don't mess with the list we're reading)
      LOG_ASSERT_ERROR(num_dtr < sizeof(deps_to_remove)/sizeof(deps_to_remove[0]), "Have to remove more dependencies than I expected");
      for(uint64_t i = 0; i < num_dtr; ++i)
         entry->uop->removeDependency(deps_to_remove[i]);
      if (entry->uop->getDependenciesLength() == 0)
      {
         // We have no dependencies in the ROB: mark ourselves as ready
         entry->ready = entry->readyMax;
      }

      // Backup initial dependencies (rollbacks when flush)
      entry->uop->backupInitialDependencies();

      if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         std::cout<<"** simulate: "<< entry->uop->getMicroOp()->toShortString(true) << std::endl << entry->uop->getMicroOp()->toString()<<std::endl;
      }

      if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         std::cout << "Type = " << (*it)->getMicroOp()->getSubtype() <<
             " count = " <<
             m_uop_type_count[(*it)->getMicroOp()->getSubtype()] << '\n';
      }

      m_uop_type_count[(*it)->getMicroOp()->getSubtype()]++;
      m_uops_total++;

      if ((*it)->isFirst()) {
         m_inst_type_count[(*it)->getMicroOp()->getSubtype()]++;
         m_inst_total++;
      }

      if ((*it)->getMicroOp()->isX87()) m_uops_x87++;
      if ((*it)->getMicroOp()->isPause()) m_uops_pause++;

      if (m_uops_total > 10000 && m_uops_x87 > m_uops_total / 20)
         LOG_PRINT_WARNING_ONCE("Significant fraction of x87 instructions encountered, accuracy will be low. Compile without -mno-sse2 -mno-sse3 to avoid.");

      m_priority_manager->countTargetInst(*(*it)->getMicroOp());
   }

   if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
#ifdef STOP_PERCYCLE
   char a;
   std::cin >> a;
#endif
   }

   while (true)
   {
      uint64_t instructionsExecuted;
      SubsecondTime latency;
      execute(instructionsExecuted, latency);
      totalInsnExec += instructionsExecuted;
      totalLat += latency;
      if (latency == SubsecondTime::Zero())
         break;
      if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
#ifdef STOP_PERCYCLE
          std::cin >> a;
#endif
      }

      // // Deadlock possibility check
      // if (m_last_committed_time.getCycleCount() == 0 ? false :
      //     now.getCycleCount() - m_last_committed_time.getCycleCount() >= 100000) {
      //    printRob (true, false);
      //    fprintf (stderr, "Execution DEADLOCKED?, now=%ld, last=%ld",
      //             now.getCycleCount(),
      //             m_last_committed_time.getCycleCount());
      //    exit (EXIT_FAILURE);
      // }

   }

   return boost::tuple<uint64_t,SubsecondTime>(totalInsnExec, totalLat);
}

void RobTimer::synchronize(SubsecondTime time)
{
   // NOTE: depending on how far we jumped ahead (usually a considerable amount),
   //       we may want to flush the ROB and reset other queues
   //printf("RobTimer::synchronize(%lu) %+ld\n", time, (int64_t)time-now);
   now.setElapsedTime(time);
}

SubsecondTime* RobTimer::findCpiComponent()
{
   // Determine the CPI component corresponding to the first non-committed instruction
   for(uint64_t i = 0; i < m_num_in_rob; ++i)
   {
      RobEntry *entry = &rob.at(i);
      DynamicMicroOp *uop = entry->uop;
      // Skip over completed instructions
      if (entry->done < now)
         continue;
      // This is the first instruction in the ROB which is still executing
      // Assume everyone is blocked on this one
      // Assign 100% of this cycle to this guy's CPI component
      if (uop->getMicroOp()->isSerializing() || uop->getMicroOp()->isMemBarrier())
         return &m_cpiSerialization;
      else if (uop->getMicroOp()->isLoad() || uop->getMicroOp()->isStore())
         return &m_cpiDataCache[uop->getDCacheHitWhere()];
      else
         return NULL;
   }
   // No instruction is currently executing
   return NULL;
}

SubsecondTime RobTimer::doDispatch(SubsecondTime **cpiComponent)
{
   SubsecondTime next_event = SubsecondTime::MaxTime();
   SubsecondTime *cpiFrontEnd = NULL;

   static SubsecondTime missed_icache = SubsecondTime::MaxTime();

   if (frontend_stalled_until <= now)
   {
      m_frontstall_idx = frontstall_t::None;

      uint32_t instrs_dispatched = 0, uops_dispatched = 0;

      if (m_num_in_rob == windowSize) {
         m_frontstall_idx = frontstall_t::RobFull;
      }
      if (m_num_in_rob_head >= robHwSize) {
         m_frontstall_idx = frontstall_t::RobFullHead;
         // printRob(true, false);
         // fprintf (stderr, "ROB head overflow, m_num_in_rob = %ld, rob.size() = %d, m_num_in_rob_head = %ld, windowSize = %ld\n",
         //          m_num_in_rob, rob.size(), m_num_in_rob_head, windowSize);
         // exit (EXIT_FAILURE);
      }

      // while(m_num_in_rob < windowSize)
      while (m_num_in_rob_head < robHwSize)
      {
         // if (m_num_in_rob >= rob.size()) {
         //    fprintf (stderr, "ROB overflow, m_num_in_rob = %ld, rob.size() = %d, m_num_in_rob_head = %ld, windowSize = %ld\n",
         //             m_num_in_rob, rob.size(), m_num_in_rob_head, windowSize);
         //    printRob(true, false);
         //    exit (EXIT_FAILURE);
         // }
         LOG_ASSERT_ERROR(m_num_in_rob < rob.size(), "Expected sufficient uops for dispatching in pre-ROB buffer, but didn't find them");
         RobEntry *entry = &rob.at(m_num_in_rob);
         DynamicMicroOp &uop = *entry->uop;

         // if (uop.getMicroOp()->isVecStore() && vec_store_queue == 0) {
         //    if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         //       fprintf(stderr, "Vector Store Queue Overflow");
         //    }
         //    break;
         // }

         // Dispatch up to 4 instructions
         if (uops_dispatched == dispatchWidth)
            break;

         if (m_vec_reserve_policy == vec_reserve_policy_t::VecReserveDynamic) {
            if (entry->uop->isStrongPriorityInst() || entry->uop->isReserveInst()) {
               // 後続の依存している命令にLow Priorityを伝える
               for(size_t idx = 0; idx < entry->getNumDependants(); ++idx) {
                  RobEntry *depEntry = entry->getDependant(idx);
                  if (!depEntry->uop->isStrongPriorityInst()) {
                     depEntry->uop->setReserveInst();
                     ROB_DEBUG_PRINTF ("Set DependEntry Reserve Priority uop_idx=%ld %s\n",
                                       depEntry->uop->getSequenceNumber(),
                                       depEntry->uop->getMicroOp()->toShortString().c_str());
                  }
               }
            }
         }
         //    // 低優先度の命令に依存している or 高優先度の命令に依存している
         //    for(size_t idx = 0; idx < uop.getDependenciesLength(); ++idx) {
         //       RobEntry *waiting_entry = this->findEntryBySequenceNumber(uop.getDependency(idx));

         //       bool is_waiting_entry_vector_dest_reg = waiting_entry->uop->getMicroOp()->getDestinationRegistersLength() &&
         //             Sim()->getDecoder()->is_reg_vector(waiting_entry->uop->getMicroOp()->getDestinationRegister(0));
         //       if (is_waiting_entry_vector_dest_reg &&
         //           (waiting_entry->uop->isReserveInst() ||         // 低優先度の命令に依存する命令はLPIQに入れる
         //            waiting_entry->uop->isStrongPriorityInst())) {  // レイテンシが長いであろう超高優先度命令に依存する命令はLPIQに入れる
         //          ROB_DEBUG_PRINTF ("Set Reserve Priority uop_idx=%ld %s\n", uop.getSequenceNumber(), uop.getMicroOp()->toShortString().c_str());
         //          uop.setReserveInst();
         //          break;
         //       }
         //    }
         // }

         // This is actually in the decode stage, there's a buffer between decode and dispatch
         // so we shouldn't do this here.
         //// First instruction can be any size, but second and subsequent ones may only be single-uop
         //// So, if this is not the first instruction, break if the first uop is not also the last
         //if (instrs_dispatched > 0 && !uop.isLast())
         //   break;

         bool iCacheMiss = (uop.getICacheHitWhere() != HitWhere::L1I);
         if (iCacheMiss)
         {
            if (in_icache_miss)
            {
               // We just took the latency for this instruction, now dispatch it
               ROB_DEBUG_PRINTF("-- icache return\n");
               in_icache_miss = false;
            }
            else
            {
               ROB_DEBUG_PRINTF("-- icache miss (%08lx) (%d)\n", uop.getMicroOp()->getInstruction()->getAddress(), uop.getICacheLatency());
               frontend_stalled_until = now + uop.getICacheLatency();
               in_icache_miss = true;
               entry->fetch = now;
               missed_icache = now;
               // Don't dispatch this instruction yet
               cpiFrontEnd = &m_cpiInstructionCache[uop.getICacheHitWhere()];
               break;
            }
         } else {
           missed_icache = now;
         }

         if (m_active_kanata_gen && m_konata_count < m_konata_count_max && !entry->kanata_registered) {
            entry->kanata_registered = true;
            entry->global_sequence_id = m_core->getGlobalSequenceIdAndInc();
            KANATA_PRINTF ("I\t%ld\t%d\t%d\n", entry->global_sequence_id, 0, 0);
            KANATA_PRINTF ("L\t%ld\t%d\t%08lx:%s\n", entry->global_sequence_id, 0,
                           uop.getMicroOp()->getInstruction()->getAddress(),
                           uop.getMicroOp()->getInstruction()->getDisassembly().c_str());
         }

         // 物理レジスタの確保試行
         if (!entry->uop->getRegisterAllocated() && allocateRegister (entry, &cpiFrontEnd)) {
            break;
         }
         entry->uop->setRegisterAllocated();
         // フロントエンドの資源不足によるストールをチェック
         if (checkFrontendStall (entry, &cpiFrontEnd)) {
            break;
         }

         if (entry->front_stall_now) {
            // stall end
            entry->front_stall_now = false;
            KANATA_PRINTF("E\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "RF"); // Resource Full
         }


         // if (!UpdateReservedBindPhyRegAllocation(m_num_in_rob)) {
         //    cpiFrontEnd = &m_cpiVPhyRegFull;
         //    dl::Decoder *dec = Sim()->getDecoder();
         //    dl::Decoder::decoder_reg dest_reg = uop.getMicroOp()->getDestinationRegister(0);
         //    if (dec->is_reg_int(dest_reg)) {
         //       m_frontstall_idx = frontstall_t::IPhyRegFull;
         //    } else if(dec->is_reg_float(dest_reg)) {
         //       m_frontstall_idx = frontstall_t::FPhyRegFull;
         //    } else if (dec->is_reg_vector(dest_reg)){
         //       m_frontstall_idx = frontstall_t::VPhyRegFull;
         //    } else {
         //       LOG_ASSERT_ERROR (false, "Unknown register type.");
         //    }
         //    break;
         // }

         // if (!ReserveVSTQ (m_num_in_rob)) {
         //    cpiFrontEnd = &m_cpiVSTQFull;
         //    m_frontstall_idx = frontstall_t::VSTQFull;
         //    break;
         // }

         entry->fetch = missed_icache;
         entry->dispatched = now;
         ++m_num_in_rob;
         ++m_rs_entries_used;

         if (uop.isFirst()) {
            ++m_num_in_rob_head;
         }

         if (is_vldq_assign(&uop)) {
            --vec_load_queue;
            // fprintf(stderr, "vec_load_queue descrease: idx=%ld VLDQ=%ld %d %s\n",
            //          entry->uop->getSequenceNumber(), vec_load_queue, entry->uop->isFirst(),
            //    entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
         }
         if (!m_vec_store_inorder && uop.getMicroOp()->isVecStore() && uop.isFirst()) {
            LOG_ASSERT_ERROR(vec_store_queue >= m_vlen / 64, "vec_store_queue is negative");
            vec_store_queue -= m_vlen / 64;
         }
         if (!uop.getMicroOp()->isVector() && uop.getMicroOp()->isLoad()) {
            --scalar_load_queue;
         }
         if (!uop.getMicroOp()->isVector() && uop.getMicroOp()->isStore()) {
            --scalar_store_queue;
         }

         switch (uop.getMicroOp()->getSubtype()) {
            case MicroOp::UOP_SUBTYPE_FP_ADDSUB :
            case MicroOp::UOP_SUBTYPE_FP_MULDIV :
               m_fpu_num_in_rs++;
               m_statsFPURSMax = std::max(m_statsFPURSMax, m_fpu_num_in_rs);
               break;
            case MicroOp::UOP_SUBTYPE_LOAD :
            case MicroOp::UOP_SUBTYPE_STORE :
               m_lsu_num_in_rs ++;
               m_statsLSURSMax = std::max(m_statsLSURSMax, m_lsu_num_in_rs);
               break;
            case MicroOp::UOP_SUBTYPE_GENERIC :
            case MicroOp::UOP_SUBTYPE_BRANCH :
               m_alu_num_in_rs++;
               m_statsALURSMax = std::max(m_statsALURSMax, m_alu_num_in_rs);
               break;
            case MicroOp::UOP_SUBTYPE_VEC_ARITH :
            case MicroOp::UOP_SUBTYPE_VEC_LOAD :
            case MicroOp::UOP_SUBTYPE_VEC_STORE :
               if ((m_vec_reserve_policy == vec_reserve_policy_t::VecReserveDynamic && !uop.isReserveInst() && uop.isFirst()) || 
                   (m_vec_reserve_policy != vec_reserve_policy_t::VecReserveDynamic && uop.isFirst())) {
                  m_vec_num_in_rs++;
                  // fprintf(stderr, "%ld Allocated VecRS   %ld, NumRS=%ld\n", now.getCycleCount(), uop.getSequenceNumber(), m_vec_num_in_rs);
                  m_statsVECRSMax = std::max(m_statsVECRSMax, m_vec_num_in_rs);
               }
               break;
            default :
               LOG_ASSERT_ERROR(false, "Not expected to this point");
         }

         if (uop.getMicroOp()->isLoad() || uop.getMicroOp()->isStore()) {
            KANATA_PRINTF ("L\t%ld\t%d\tAccess=%08lx,\n", entry->global_sequence_id, 1,
                           uop.getAddress().address);
         }
         if (uop.getMicroOp()->isVector()) {
            KANATA_PRINTF ("L\t%ld\t%d\tPhyReg(%ld),\n", entry->global_sequence_id, 1,
                           m_reg_manager->getAllocVectorRegister());
            if (isUseNonpriVector (m_vec_reserve_policy)) {
               KANATA_PRINTF ("L\t%ld\t%d\tResReg(%ld,%ld),\n", entry->global_sequence_id, 1,
                     m_reg_manager->getNonPriVectorRegisters(),
                     m_reg_manager->getNonPriVectorRegisters() < m_reg_manager->getNonPriMaxVectorRegisters() ? m_reg_manager->getNonPriVectorRegisters() : m_reg_manager->getNonPriMaxVectorRegisters());
            }
         }
         for(unsigned int i = 0; i < uop.getDependenciesLength(); ++i) {
            dl::Decoder *dec = Sim()->getDecoder();
            uint64_t lowestValidSequenceNumber = this->rob.size() > 0 ? this->rob.front().uop->getSequenceNumber() : 0;
            if (uop.getDependency(i) >= lowestValidSequenceNumber) {
               RobEntry *producerEntry = this->findEntryBySequenceNumber(uop.getDependency(i));
               if (dec->is_vsetvl(producerEntry->uop->getMicroOp()->getInstructionOpcode())) {
                  continue;
               }
               // KANATA_PRINTF ("W\t%ld\t%ld\t%d\n", entry->global_sequence_id, producerEntry->global_sequence_id, 0);
            }
         }
         if (uop.isInLPIQ()) {
            KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                        "Wf"); // Wait in FIFO
         } else if (uop.getMicroOp()->isVector()) {
            KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                        "Dv"); // Vector
            KANATA_PRINTF("L\t%ld\t%d\tVecRs=%ld\n", entry->global_sequence_id, 2, m_vec_num_in_rs);
         } else {
            KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                        "Ds");
         }

         // KANATA_PRINTF ("L\t%ld\t%d\tVecPhyregs=%ld\n",
         // entry->global_sequence_id, 2, m_phy_registers[2] - 32);
         m_kanata_generated_in_this_region = true;
         // KANATA_PRINTF ("E\t%ld\t%d\t%s\n", uop.getSequenceNumber(), 0, "F");

         uops_dispatched++;
         if (uop.isLast())
            instrs_dispatched++;

         // If uop is already ready, we may need to issue it in the following cycle
         entry->ready = std::max(entry->ready, (now + 1ul).getElapsedTime());
         next_event = std::min(next_event, entry->ready);

         ROB_DEBUG_PRINTF ("DISPATCH uop_idx=%ld %s", entry->uop->getSequenceNumber(), entry->uop->getMicroOp()->toShortString().c_str());

         if (isUseNonpriVector (m_vec_reserve_policy)) {
            ROB_DEBUG_PRINTF (" Priority: %s\n", entry->uop->isReserveInst() ? "RESERVE" : 
                                                 entry->uop->isStrongPriorityInst() ? "STRONG" : 
                                                 "NORMAL");
         } else {
            ROB_DEBUG_PRINTF ("\n");
         }

         #ifdef ASSERT_SKIP
            LOG_ASSERT_ERROR(will_skip == false, "Cycle would have been skipped but stuff happened");
         #endif

         // Mispredicted branch
         if (uop.getMicroOp()->isBranch() && uop.isBranchMispredicted())
         {
            frontend_stalled_until = SubsecondTime::MaxTime();
            ROB_DEBUG_PRINTF ("-- branch mispredict\n");
            cpiFrontEnd = &m_cpiBranchPredictor;
            break;
         }
      }

      m_cpiCurrentFrontEndStall = cpiFrontEnd;
   }
   else
   {
      // Front-end is still stalled: re-use last CPI component
      cpiFrontEnd = m_cpiCurrentFrontEndStall;
   }


   // Find CPI component corresponding to the first executing instruction
   SubsecondTime *cpiRobHead = findCpiComponent();

   if (cpiFrontEnd)
   {
      // Front-end is stalled
      if (cpiRobHead)
      {
         // Have memory/serialization components take precendence over front-end stalls
         *cpiComponent = cpiRobHead;
      }
      else
      {
         *cpiComponent = cpiFrontEnd;
      }
   }
   else if (m_num_in_rob == windowSize)
   {
      *cpiComponent = cpiRobHead ? cpiRobHead : &m_cpiBase;
   }
   else
   {
      *cpiComponent = &m_cpiBase;
   }


   if (m_num_in_rob == windowSize)
      return next_event; // front-end is effectively stalled so wait for another event
   else
      return std::min(frontend_stalled_until, next_event);
}

// Check if the front-end is stalled
// true: stall, false: not stall
bool RobTimer::checkFrontendStall(RobEntry *entry, SubsecondTime **cpiFrontEnd)
{
   auto uop = entry->uop;

   if ((uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_FP_ADDSUB ||
        uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_FP_MULDIV) &&
       m_fpu_num_in_rs > m_fpu_window_size) {
      ROB_DEBUG_PRINTF(
          "doDispatch : seqId=%ld : FPU Instruction Window Overflow\n",
          uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiFPURSFull;
      m_frontstall_idx = frontstall_t::FPURsFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "FPU Instruction Window Overflow");
      }
      return true;
   }
   if ((uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_GENERIC ||
        uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_BRANCH) &&
       m_alu_num_in_rs > m_alu_window_size) {
      ROB_DEBUG_PRINTF(
          "doDispatch : seqId=%ld : ALU Instruction Window Overflow\n",
          uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiALURSFull;
      m_frontstall_idx = frontstall_t::ALURsFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "ALU Instruction Window Overflow");
      }
      return true;
   }
   if ((uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_LOAD ||
        uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_STORE) &&
       m_lsu_num_in_rs > m_lsu_window_size) {
      ROB_DEBUG_PRINTF(
          "doDispatch : seqId=%ld : LSU Instruction Window Overflow\n",
          uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiLSURSFull;
      m_frontstall_idx = frontstall_t::LSURsFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "LSU Instruction Window Overflow");
      }
      return true;
   }
   if (uop->isFirst() &&
       (uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_VEC_ARITH ||
        uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_VEC_LOAD ||
        uop->getMicroOp()->getSubtype() == MicroOp::UOP_SUBTYPE_VEC_STORE)) {
      if (m_vec_reserve_policy == VecReserveDynamic && uop->isInLPIQ()) {
         return false;
      }
      if (m_vec_num_in_rs > m_vec_window_size) {
         ROB_DEBUG_PRINTF(
            "doDispatch : seqId=%ld : VEC_ARITH Instruction Window Overflow\n",
            uop->getSequenceNumber());
         *cpiFrontEnd = &m_cpiVECRSFull;
         m_frontstall_idx = frontstall_t::VECRsFull;
         if (!entry->front_stall_now) {
            // stall start
            entry->front_stall_now = true;
            KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                        "RF"); // Resource Full
            KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                        "Vec Arith Instruction Window Overflow");
         }
         // fprintf(stderr, "doDispatch : seqId=%ld : VEC_ARITH Instruction Window Overflow\n",
         //         uop->getSequenceNumber());
         // printRob(true, false);
         return true;
      }
   }

   // VLDQ full
   if (is_vldq_assign(uop) && vec_load_queue == 0) {
      ROB_DEBUG_PRINTF("doDispatch : seqId=%ld : Vector Load Queue overflow\n",
                       uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiVLDQFull;
      m_frontstall_idx = frontstall_t::VLDQFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "VLDQ Instruction Window Overflow");
         // printRob(true, false);
         // exit(EXIT_FAILURE);
      }
      return true;
   }
   // VSTQ full
   if (!m_vec_store_inorder && vec_store_queue < m_vlen / 64) {
      ROB_DEBUG_PRINTF("doDispatch : seqId=%ld : Vector Store Queue overflow\n",
                       uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiVSTQFull;
      m_frontstall_idx = frontstall_t::VSTQFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "VSTQ Instruction Window Overflow");
      }
      return true;
   }
   // Scalar LDQ full
   if (!uop->getMicroOp()->isVector() && uop->getMicroOp()->isLoad() &&
       scalar_load_queue == 0) {
      ROB_DEBUG_PRINTF("doDispatch : seqId=%ld : Scalar Load Queue overflow\n",
                       uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiLDQFull;
      m_frontstall_idx = frontstall_t::LDQFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "SLDQ Instruction Window Overflow");
      }
      return true;
   }
   // Scalar STQ full
   if (!uop->getMicroOp()->isVector() && uop->getMicroOp()->isStore() &&
       scalar_store_queue == 0) {
      ROB_DEBUG_PRINTF("doDispatch : seqId=%ld : Scalar Store Queue overflow\n",
                       uop->getSequenceNumber());
      *cpiFrontEnd = &m_cpiSTQFull;
      m_frontstall_idx = frontstall_t::STQFull;
      if (!entry->front_stall_now) {
         // stall start
         entry->front_stall_now = true;
         KANATA_PRINTF("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0,
                       "RF"); // Resource Full
         KANATA_PRINTF("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2,
                       "SSTQ Instruction Window Overflow");
      }
      return true;
   }

   return false;
}

// --------------------------------
// 予約に失敗した場合はtrueとなる。
// --------------------------------
bool RobTimer::allocateRegister (RobEntry *entry, SubsecondTime **cpiFrontEnd)
{
   DynamicMicroOp *uop = entry->uop;
   if (uop->getMicroOp()->getDestinationRegistersLength() == 0) {
      // vcpop命令など、書き込み=GPR / 読み込み=物理レジスタの場合もチェックする
      if (m_vec_reserve_policy == VecReserveDynamic && uop->getMicroOp()->isVector()) {
         // ベクトル命令で、レジスタを確保しない命令でも、ベクトルストア命令などは予約レジスタを使用しているならば、LPIQに入る。

         UInt64 first_sequence_number = rob[0].uop->getSequenceNumber();

         UInt64 index = 1;
         if (first_sequence_number > uop->getSequenceNumber() - index) {
            return false;
         }
         RobEntry *firstEntry = findEntryBySequenceNumber(uop->getSequenceNumber() - index);
         while (first_sequence_number > uop->getSequenceNumber() - index && !firstEntry->uop->isFirst()) {
            index++;
            firstEntry = findEntryBySequenceNumber(uop->getSequenceNumber() - index);
         } 
         if (firstEntry->uop->isReserveInst()) {
            uop->setReserveInst();
         }
         InsertLPIQ (uop, DynamicMicroOp::lpiq_t::RESOLVED);
      }
      return false;
   }

   bool allocate_fail = false;
   dl::Decoder *dec = Sim()->getDecoder();
   dl::Decoder::decoder_reg dest_reg = uop->getMicroOp()->getDestinationRegister(0);

   RegisterManager::AllocResult_t alloc_result = m_reg_manager->AllocateRegister (uop);
   if (!dec->is_reg_vector(dest_reg)) {
      // 整数・浮動小数点レジスタ確保
      if (alloc_result != RegisterManager::AllocSuccess) {
         if (!entry->front_stall_now) {
            // stall start
            entry->front_stall_now = true;
            KANATA_PRINTF ("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "RF"); // Resource Full
            KANATA_PRINTF ("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2, "INT/FP Register Full");
         }
         allocate_fail = true;
         *cpiFrontEnd = &m_cpiSPhyRegFull;
         if (dec->is_reg_int(dest_reg)) {
            m_frontstall_idx = frontstall_t::IPhyRegFull;
         } else if(dec->is_reg_float(dest_reg)) {
            m_frontstall_idx = frontstall_t::FPhyRegFull;
         } else {
            LOG_ASSERT_ERROR (false, "Should Int or float register type.");
         }
      } else if (uop->getMicroOp()->isVector()) {
         UInt64 first_sequence_number = rob[0].uop->getSequenceNumber();

         UInt64 index = 1;
         if (first_sequence_number > uop->getSequenceNumber() - index) {
            return false;
         }
         RobEntry *firstEntry = findEntryBySequenceNumber(uop->getSequenceNumber() - index);
         while (first_sequence_number > uop->getSequenceNumber() - index && !firstEntry->uop->isFirst()) {
            index++;
            firstEntry = findEntryBySequenceNumber(uop->getSequenceNumber() - index);
         } 
         if (firstEntry->uop->isReserveInst()) {
            uop->setReserveInst();
         }
         InsertLPIQ (uop, DynamicMicroOp::lpiq_t::RESOLVED);
      }
      return allocate_fail;
   }

   if (isUseNonpriVector (m_vec_reserve_policy)) {
      if (m_vec_reserve_policy != VecReserveParOOO && uop->isReserveInst()) {
         // ParOOOの場合は物理レジスタを確保しない
         // 予約に回る命令であれば、LPIQに格納する
         // LPIQに入れるべき命令の場合
         if (alloc_result == RegisterManager::AllocSuccess) {
            // 予約用のレジスタの確保に成功した場合: 確保したうえでLPIQに入る
            InsertLPIQ (uop, DynamicMicroOp::lpiq_t::RESOLVED);
         } else if (alloc_result == RegisterManager::AllocChain) {
            // Firstではない命令は、Firstの命令の結果に依存している
            InsertLPIQ (uop, DynamicMicroOp::lpiq_t::CHAIN);
         } else if (alloc_result == RegisterManager::AllocReserve) {
            // 予約用のレジスタを確保した場合
            InsertResRegLPIQ (uop);
         } else {
            // 物理レジスタを確保し転向を期待する場合
            InsertTransRegLPIQ (uop);
         }
      } else {
         if (alloc_result == RegisterManager::AllocFull) {
            // uop->setReserveInst();
            allocate_fail = true;
            *cpiFrontEnd = &m_cpiVPhyRegFull;
         } else if (alloc_result == RegisterManager::AllocChain) {
            // fprintf (stderr, "Chain Entry check: %ld, %d\n", entry->uop->getSequenceNumber(), entry->uop->getMicroOp()->UopIdx());
            UInt64 index = 1;
            RobEntry *firstEntry = findEntryBySequenceNumber(entry->uop->getSequenceNumber() - index);
            while (!firstEntry->uop->isFirst()) {
               index++;
               firstEntry = findEntryBySequenceNumber(entry->uop->getSequenceNumber() - index);
            } 
            if (firstEntry->uop->isReserveInst()) {
               uop->setReserveInst();
            }
         }
      }
   } else if (m_vec_reserve_policy == vec_reserve_policy_t::VecReserveWhenFull) {
      // 物理レジスタの確保試行
      if (alloc_result == RegisterManager::AllocFull) {
         dl::Decoder *dec = Sim()->getDecoder();
         dl::Decoder::decoder_reg dest_reg = uop->getMicroOp()->getDestinationRegister(0);
         if (dec->is_reg_vector(dest_reg)){
            m_frontstall_idx = frontstall_t::VPhyRegFull;
         } else {
            LOG_ASSERT_ERROR (false, "Unknown register type.");
         }
         if (!entry->front_stall_now) {
            // stall start
            entry->front_stall_now = true;
            KANATA_PRINTF ("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "RF"); // Resource Full
            KANATA_PRINTF ("L\t%ld\t%d\t%s Register Full\n", entry->global_sequence_id, 2, 
                  dec->is_reg_int(dest_reg) ? "INT" : dec->is_reg_float(dest_reg) ? "FP" : "VEC");
         }
         *cpiFrontEnd = &m_cpiVPhyRegFull;
         allocate_fail = true;
      }
   } else {
      // 物理レジスタの確保試行
      if (alloc_result == RegisterManager::AllocFull) {
         dl::Decoder *dec = Sim()->getDecoder();
         dl::Decoder::decoder_reg dest_reg = uop->getMicroOp()->getDestinationRegister(0);
         if (dec->is_reg_vector(dest_reg)){
            m_frontstall_idx = frontstall_t::VPhyRegFull;
         } else {
            LOG_ASSERT_ERROR (false, "Unknown register type.");
         }
         if (!entry->front_stall_now) {
               // stall start
               entry->front_stall_now = true;
               KANATA_PRINTF ("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "RF"); // Resource Full
               KANATA_PRINTF ("L\t%ld\t%d\t%s Register Full\n", entry->global_sequence_id, 2, 
                     dec->is_reg_int(dest_reg) ? "INT" : dec->is_reg_float(dest_reg) ? "FP" : "VEC");
         }
         *cpiFrontEnd = &m_cpiVPhyRegFull;
         allocate_fail = true;
      }
   }

   return allocate_fail;
}

void RobTimer::releaseRegister (RobEntry *entry)
{
   if (!isUseNonpriVector (m_vec_reserve_policy)) {
      m_reg_manager->ReleaseRegister (entry->uop);
      return;
   }
                                                                                     
   // 非優先命令の持っているレジスタは解放時に、LPIQ内のレジスタを渡す                                                           
   dl::Decoder *dec = Sim()->getDecoder();
   bool hasVecDestRegister = entry->uop->getMicroOp()->getDestinationRegistersLength() != 0 &&
       entry->uop->isLast() &&
       dec->is_reg_vector(entry->uop->getMicroOp()->getDestinationRegister(0));
   if (!hasVecDestRegister) {
      m_reg_manager->ReleaseRegister (entry->uop);
      return;
   }

   if (m_vec_reserve_policy == vec_reserve_policy_t::VecReserveParOOO &&
         entry->uop->isUseReserveRegisterGroup()) {
      // 予約命令の場合
      return;
   }

   if (entry->uop->isUseReserveRegisterGroup()) {
      if (unlikely(m_reg_manager->ReleaseRegister (entry->uop))) {
         printRob(true, false);
         LOG_ASSERT_ERROR(false, "Cycle=%ld ReleaseRegister failed", now.getCycleCount());
      }
      bool lowpri_reg_pass_succeeded = false;
      for (auto &f : m_lpiq_fifo) {
         RobEntry *lpiq_entry = findEntryBySequenceNumber(f);
         if (lpiq_entry->uop->isInLPIQ() &&
             lpiq_entry->uop->getMicroOp()->getDestinationRegistersLength() != 0 &&
             dec->is_reg_vector(lpiq_entry->uop->getMicroOp()->getDestinationRegister(0)) &&
             lpiq_entry->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESREG) {
            lpiq_entry->uop->setCommitDependency(DynamicMicroOp::lpiq_t::RESOLVED);
            ROB_DEBUG_PRINTF (" LPIQ physical register obtained : uop_idx=%ld %s\n",
                              lpiq_entry->uop->getSequenceNumber(),
                              lpiq_entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
            lowpri_reg_pass_succeeded = true;
            KANATA_PRINTF ("W\t%ld\t%ld\t%d\n",
                     entry->global_sequence_id,
                     lpiq_entry->global_sequence_id, 0);
            break;
         }
      }
      if (!lowpri_reg_pass_succeeded) {
         // 渡す予約命令が無いので、物理レジスタに戻す
         if (unlikely(m_reg_manager->ForceReleaseVoctorRegister())) {
            printRob(true, false);
            LOG_ASSERT_ERROR(false, "ForceReleaseVoctorRegister failed");
         }
      }
   } else {
      // isUseNormalRegisterGroup()
      // 通常の命令ではあるが、LPIQ内に通常物理レジスタの予約転向を待っている命令が存在している場合
      bool lowpri_reg_pass_succeeded = false;
      for (auto &f : m_lpiq_fifo) {
         RobEntry *lpiq_entry = findEntryBySequenceNumber(f);
         if (lpiq_entry->uop->isInLPIQ() &&
             lpiq_entry->uop->getMicroOp()->getDestinationRegistersLength() != 0 &&
             dec->is_reg_vector(lpiq_entry->uop->getMicroOp()->getDestinationRegister(0)) &&
             lpiq_entry->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::TRANSREG) {
            lpiq_entry->uop->setCommitDependency(DynamicMicroOp::lpiq_t::RESOLVED);
            ROB_DEBUG_PRINTF (" LPIQ physical register obtained : uop_idx=%ld %s\n",
                              lpiq_entry->uop->getSequenceNumber(),
                              lpiq_entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
            KANATA_PRINTF ("W\t%ld\t%ld\t%d\n",
                     entry->global_sequence_id,
                     lpiq_entry->global_sequence_id, 0);
            lowpri_reg_pass_succeeded = true;
            break;
         }
      }
      if (!lowpri_reg_pass_succeeded) {
         // 渡す予約命令が無いので、物理レジスタに戻す
         if (unlikely(m_reg_manager->ForceReleaseVoctorRegister())) {
            printRob(true, false);
            LOG_ASSERT_ERROR(false, "ForceReleaseVoctorRegister failed");
         }
      }
   }
}


void RobTimer::issueInstruction(uint64_t idx, SubsecondTime &next_event)
{
   RobEntry *entry = &rob[idx];
   DynamicMicroOp &uop = *entry->uop;

   if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
      std::cout <<"ISSUE    "<< "(" << entry->uop->getSequenceNumber() << ") " <<
               entry->uop->getMicroOp()->toShortString()<<"   latency="<<uop.getExecLatency()<<std::endl;
   }

   if ((uop.getMicroOp()->isLoad() || uop.getMicroOp()->isStore())
       && uop.getDCacheHitWhere() == HitWhere::UNKNOWN) {
      uint64_t access_size_scale = uop.getMicroOp()->isVector() ? uop.getNumMergedInst() + 1 : 1;
      MemoryResult res = m_core->accessMemory(
          Core::NONE,
          uop.getMicroOp()->isVector() ? (uop.getMicroOp()->isLoad() ? Core::READ_VEC : Core::WRITE_VEC) :
          uop.getMicroOp()->isLoad() ? Core::READ : Core::WRITE,
          uop.getAddress().address,
          NULL,
          uop.getMicroOp()->getMemoryAccessSize() * access_size_scale,
          Core::MEM_MODELED_RETURN,
          uop.getMicroOp()->getInstruction() ? uop.getMicroOp()->getInstruction()->getAddress() : static_cast<uint64_t>(NULL),
          uop.getSequenceNumber(),
          now.getElapsedTime(),
          false  // 通常アクセスでプリフェッチを発生させると、ProcessMemOpFromCoreがループしてLockを取得できない
      );
      uint64_t latency = SubsecondTime::divideRounded(res.latency, now.getPeriod());
      m_previous_latency = latency;
      m_previous_hit_where = res.hit_where;

      if (uop.getMicroOp()->isVecLoad()) {
         UpdateVecDCacheStats(&uop, res.hit_where);
      }

      if (uop.getMicroOp()->isVecLoad()) {
         DynamicMicroOp *last_uop = &uop;
         last_uop->setCacheLastHitAnd (res.hit_where != HitWhere::where_t::DRAM);
         if (!uop.isLast()) {
            // fprintf(stderr, "uop_max_latency seq_idx=%ld, uop_idx=%d, num_uop=%d\n", 
            //         uop.getSequenceNumber(), uop.getMicroOp()->UopIdx(), uop.getMicroOp()->NumUop());
            size_t last_offset = 1;
            RobEntry *uop_last_entry = this->findEntryBySequenceNumber(uop.getSequenceNumber() + last_offset);
            last_uop = uop_last_entry->uop;
            while (!last_uop->isLast()) {
               last_offset++;
               uop_last_entry = this->findEntryBySequenceNumber(uop.getSequenceNumber() + last_offset);
               last_uop = uop_last_entry->uop;
            };
         } else {
            // Last uop
            UpdateVecLoadHit (uop.getCacheLastHitAnd());
         }
      }

      if (isUseNonpriVector (m_vec_reserve_policy)) {
         if (uop.getMicroOp()->isVecLoad()) {
            DynamicMicroOp *last_uop = &uop;
            if (!uop.isLast()) {
               // fprintf(stderr, "uop_max_latency seq_idx=%ld, uop_idx=%d, num_uop=%d\n", 
               //         uop.getSequenceNumber(), uop.getMicroOp()->UopIdx(), uop.getMicroOp()->NumUop());
               size_t last_offset = 1;
               RobEntry *uop_last_entry = this->findEntryBySequenceNumber(uop.getSequenceNumber() + last_offset);
               last_uop = uop_last_entry->uop;
               while (!last_uop->isLast()) {
                  last_offset++;
                  uop_last_entry = this->findEntryBySequenceNumber(uop.getSequenceNumber() + last_offset);
                  last_uop = uop_last_entry->uop;
               };
            }

            // fprintf(stderr, "uop_max_latency pc = %08lx, uop_idx=%ld, trying to update %ld, max_latency=%ld : ", 
            //         uop.getMicroOp()->getInstruction()->getAddress(),
            //         uop.getSequenceNumber(),
            //         last_uop->getSequenceNumber(),
            //         latency);
            if (last_uop->getMemMaxLatency() < latency) {
               // fprintf(stderr, "updated: %ld -> %ld\n", last_uop->getMemMaxLatency(), latency);
               last_uop->setMemMaxLatency(latency);
            } else {
               // fprintf(stderr, "\n");
            }

            if (uop.isLast()) {
               ROB_DEBUG_PRINTF ("uop_max_latency: last pc = %08lx, uop_idx=%ld, max_latency=%ld\n", 
                     uop.getMicroOp()->getInstruction()->getAddress(),
                     uop.getSequenceNumber(),
                     uop.getMemMaxLatency());
               bool vec_miss;
               bool update = m_mem_stats->Update (uop.getMicroOp()->getInstruction()->getAddress(), uop.getMemMaxLatency(), vec_miss);
               
               if (update) {
                  // 命令の属性を変更させるかどうかをチェックする
                  m_priority_manager->UpdateInstPriority (uop.getMicroOp(), vec_miss);
               }
            }
         }
      }
      uop.setExecLatency(uop.getExecLatency() + latency); // execlatency already contains bypass latency
      uop.setDCacheHitWhere(res.hit_where);

      if (m_pref_target_log == 0 || uop.getMicroOp()->getInstruction()->getAddress() == m_pref_target_log) {
         static UInt64 last_address = 0;
         ROB_DEBUG_PRINTF ("memory_access : 0x%08lx,0x%08lx,%ld,%d,%s,last=%lx\n",
                  uop.getMicroOp()->getInstruction()->getAddress(),
                  uop.getAddress().address,
                  latency,
                  uop.getDCacheHitWhere(),
                  HitWhereString(uop.getDCacheHitWhere()),
                  uop.getAddress().address - last_address);
         last_address = uop.getAddress().address;
      }
   }



   if (uop.getMicroOp()->isLoad() && !uop.getMicroOp()->isVector())
   {
      load_queue.getCompletionTime(now, uop.getExecLatency() * now.getPeriod(), uop.getAddress().address);
   }
   else if (uop.getMicroOp()->isStore() && !uop.getMicroOp()->isVector())
   {
      store_queue.getCompletionTime(now, uop.getExecLatency() * now.getPeriod(), uop.getAddress().address);
   }

   uint64_t additional_latency = uop.getVectorIssueMax() - 1;
   if (uop.getMicroOp()->isVecMem() &&
       !uop.getMicroOp()->canVecSquash()) {
     // Gather / Scatter
     additional_latency = 0;
   }

   ComponentTime cycle_depend = now + uop.getExecLatency();        // When result is available for dependent instructions
   ComponentTime cycle_done_raw = cycle_depend;
   if (uop.getMicroOp()->isVector()) {
     cycle_done_raw.addCycleLatency(additional_latency);
   }
   SubsecondTime cycle_done = cycle_done_raw + 1ul;  // When the instruction can be committed

   if (uop.getMicroOp()->isLoad())
   {
      m_loads_count++;
      m_loads_latency += uop.getExecLatency() * now.getPeriod();
   }
   else if (uop.getMicroOp()->isStore())
   {
      m_stores_count++;
      m_stores_latency += uop.getExecLatency() * now.getPeriod();
   }

   if (uop.getMicroOp()->isStore())
   {
      last_store_done = std::max(last_store_done, cycle_done);
      cycle_depend = now + 1ul;                          // For stores, forward the result immediately
      // Stores can be removed from the ROB once they're issued to the memory hierarchy
      // Dependent operations such as SFENCE and synchronization instructions need to wait until last_store_done
      cycle_done = now + 1ul;

      LOG_ASSERT_ERROR(entry->addressReady <= entry->ready, "%ld: Store address cannot be ready (%ld) later than the whole uop is (%ld)",
                       entry->uop->getSequenceNumber(), entry->addressReady.getPS(), entry->ready.getPS());
   }

   if (m_rob_contention)
      m_rob_contention->doIssue(uop);

   entry->issued = now;
   entry->done = cycle_done;

   if (entry->kanata_registered) {
      KANATA_PRINTF ("E\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "Wf");
      if (uop.isPreloadDone()) {
         KANATA_PRINTF ("E\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "P");
      }
      if (uop.getMicroOp()->isVector()) {
         KANATA_PRINTF ("L\t%ld\t%d\tVecRs=%ld\n", entry->global_sequence_id, 2, m_vec_num_in_rs);
         KANATA_PRINTF ("E\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "Dv"); // Vector
      } else {
         KANATA_PRINTF ("E\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "Ds");
      }
      KANATA_PRINTF ("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "X");
      m_kanata_generated_in_this_region = true;
   }

   next_event = std::min(next_event, entry->done);

   --m_rs_entries_used;

   switch (entry->uop->getMicroOp()->getSubtype()) {
      case MicroOp::UOP_SUBTYPE_FP_ADDSUB :
      case MicroOp::UOP_SUBTYPE_FP_MULDIV :
         m_fpu_num_in_rs--;
         break;
      case MicroOp::UOP_SUBTYPE_LOAD :
      case MicroOp::UOP_SUBTYPE_STORE :
         m_lsu_num_in_rs--;
         break;
      case MicroOp::UOP_SUBTYPE_GENERIC :
      case MicroOp::UOP_SUBTYPE_BRANCH :
         m_alu_num_in_rs--;
         break;
      case MicroOp::UOP_SUBTYPE_VEC_ARITH :
      case MicroOp::UOP_SUBTYPE_VEC_LOAD :
      case MicroOp::UOP_SUBTYPE_VEC_STORE :
         if ((m_vec_reserve_policy == vec_reserve_policy_t::VecReserveDynamic && !uop.isReserveInst() && uop.isLast()) || 
             (m_vec_reserve_policy != vec_reserve_policy_t::VecReserveDynamic && uop.isLast())) {
            // fprintf(stderr, "%ld Deallocated VecRS %ld, NumRS=%ld\n", now.getCycleCount(), uop.getSequenceNumber(), m_vec_num_in_rs);
            m_vec_num_in_rs--;
         }
         break;
      default :
         LOG_ASSERT_ERROR(false, "Not expected to this point");
   }

   for(size_t idx = 0; idx < entry->getNumDependants(); ++idx)
   {
      RobEntry *depEntry = entry->getDependant(idx);
      // if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
      //    printf("inst_seqnum = %ld, dep_seqnum = %ld\n", entry->uop->getSequenceNumber(),
      //                                                    depEntry->uop->getSequenceNumber());
      // }
      LOG_ASSERT_ERROR(depEntry->uop->getDependenciesLength()> 0, "??");

      // Remove uop from dependency list and update readyMax

      if (entry->uop->getSequenceNumber() + 1 == depEntry->uop->getSequenceNumber() &&
          entry->uop->getMicroOp()->getInstruction()->getAddress() == depEntry->uop->getMicroOp()->getInstruction()->getAddress()) {
         // ベクトル命令において、分解された命令間の順番依存では、latencyの情報を使用して解放するのではなく、
         // 即時解放しなければならない
         depEntry->readyMax = std::max(depEntry->readyMax, SubsecondTime::Zero());
      } else {
         depEntry->readyMax = std::max(depEntry->readyMax, cycle_depend.getElapsedTime());
      }
      depEntry->uop->removeDependency(uop.getSequenceNumber());

      // If all dependencies are resolved, mark the uop ready
      if (depEntry->uop->getDependenciesLength() == 0)
      {
         depEntry->ready = depEntry->readyMax;
         //std::cout<<"    ready @ "<<depEntry->ready<<std::endl;
      }

      // For stores, check if their address has been produced
      if (depEntry->uop->getMicroOp()->isStore() && depEntry->addressReady == SubsecondTime::MaxTime())
      {
         bool ready = true;
         for(unsigned int i = 0; i < depEntry->getNumAddressProducers(); ++i)
         {
            uint64_t addressProducer = depEntry->getAddressProducer(i);
            RobEntry *prodEntry = addressProducer >= this->rob.front().uop->getSequenceNumber()
                                ? this->findEntryBySequenceNumber(addressProducer) : NULL;

            if (prodEntry == entry)
            {
               // The instruction we just executed is producing an address. Update the store's addressReadyMax
               depEntry->addressReadyMax = std::max(depEntry->addressReadyMax, cycle_depend.getElapsedTime());
            }

            if (prodEntry && prodEntry->done == SubsecondTime::MaxTime())
            {
               // An address producer has not yet been issued: address remains not ready
               ready = false;
            }
         }

         if (ready)
         {
            // We did not find any address producing instructions that have not yet been issued.
            // Store address will be ready at addressReadyMax
            depEntry->addressReady = depEntry->addressReadyMax;
         }
      }
   }

   // After issuing a mispredicted branch: allow the ROB to refill after flushing the pipeline
   if (uop.getMicroOp()->isBranch() && uop.isBranchMispredicted())
   {
      frontend_stalled_until = now + (misprediction_penalty - 2); // The frontend needs to start 2 cycles earlier to get a total penalty of <misprediction_penalty>
      if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         std::cout<<"-- branch resolve"<<std::endl;
      }
   }

   if (uop.getMicroOp()->isVector() && !uop.getMicroOp()->isVecMem()) {
      ROB_DEBUG_PRINTF("UpdateInorderStats : %08lx, Inorder=%d\n",
                       uop.getMicroOp()->getInstruction()->getAddress(), m_1st_issue_in_cycle);
      UpdateInorderStats (uop.getMicroOp()->getInstruction()->getAddress(), m_1st_issue_in_cycle);
      m_1st_issue_in_cycle = false;
   }

   // Update the statistics
   if (uop.getMicroOp()->isVector()) {
      UpdateVectorLatencyStats (&uop);
   }
}

SubsecondTime RobTimer::doIssue()
{
   uint64_t num_issued = 0;
   SubsecondTime next_event = SubsecondTime::MaxTime();
   bool head_of_queue = true, no_more_load = false, no_more_store = false, have_unresolved_store = false;

   // Vec/Scalar Inorder Protocl
   // inorder : Whole instruction inorder
   // dyn_vector_inorder
   bool dyn_inorder = inorder;

   bool vector_someone_cant_be_issued = false;
   bool vector_store_someone_cant_be_issued = false;

   if (m_rob_contention)
      m_rob_contention->initCycle(now);

   bool inhead_vector_existed = false;
   bool inhead_vecmem_existed = false;

   bool v_to_s_fenced = false;
   UInt64  l1d_block_size = Sim()->getCfg()->getInt("perf_model/l1_dcache/cache_block_size");

   std::fill(m_bank_info.begin(), m_bank_info.end(), 0);

   bool vector_someone_wait_issue = false;
   bool scalar_someone_wait_issue = false;

   m_show_rob = false;

   for(uint64_t i = 0; i < m_num_in_rob; ++i)
   {
      RobEntry *entry = &rob.at(i);
      DynamicMicroOp *uop = entry->uop;

      // Vector Inorder Protocol:
      // vector_inorder=true : Arith/Mem Vector issued in-order
      // lsu_inorder: Mem Vector issued in-order
      bool dyn_vector_inorder = vector_inorder;
      if (m_vec_reserve_policy == vec_reserve_policy_t::VecReserveParOOO &&
          uop->isUseReserveRegisterGroup()) {
         // ReserveInorderモードで、Inorder指定された命令は強制的にインオーダモードになる
         dyn_vector_inorder = true;
      }

      inhead_vecmem_existed |= uop->getMicroOp()->isVecMem();

      if (entry->done != SubsecondTime::MaxTime())
      {
         next_event = std::min(next_event, entry->done);
         continue;                     // already done
      }

      next_event = std::min(next_event, entry->ready);


      // See if we can issue this instruction

      bool canIssue = false;

      if (entry->ready > now) {
         canIssue = false;          // blocked by dependency
         // vector_someone_cant_be_issued = dyn_vector_inorder;
      }
      else if ((no_more_load && uop->getMicroOp()->isLoad()) || (no_more_store && uop->getMicroOp()->isStore()))
         canIssue = false;          // blocked by mfence

      else if (uop->getMicroOp()->isSerializing())
      {
         if (head_of_queue && last_store_done <= now)
            canIssue = true;
         else
            break;
      }

      else if (uop->getMicroOp()->isMemBarrier())
      {
         if (head_of_queue && last_store_done <= now)
            canIssue = true;
         else
            // Don't issue any memory operations following a memory barrier
            no_more_load = no_more_store = true;
            // FIXME: L/SFENCE
      }

      else if (!m_rob_contention && num_issued == dispatchWidth) {
        std::cout << "  dispatch Width exceeded\n";
         canIssue = false;          // no issue contention: issue width == dispatch width
      }
      else if (uop->getMicroOp()->isLoad() && !uop->getMicroOp()->isVector() && !load_queue.hasFreeSlot(now)) {
         // LDQ full
         ROB_DEBUG_PRINTF ("Scalar Load Queue Overflow");
         canIssue = false;
      } else if (uop->getMicroOp()->isLoad() && m_no_address_disambiguation && have_unresolved_store) {
         ROB_DEBUG_PRINTF ("  disambiguation, index = %ld\n", uop->getSequenceNumber());
         KANATA_PRINTF ("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2, "disambiguation failed");
         m_kanata_generated_in_this_region = true;
         canIssue = false;          // preceding store with unknown address
      }
      else if (uop->getMicroOp()->isStore() && !uop->getMicroOp()->isVector() && !store_queue.hasFreeSlot(now)) {
         ROB_DEBUG_PRINTF ("seqId=%ld, Scalar Store Queue overflow\n", uop->getSequenceNumber());
         canIssue = false;
      }
      else
         canIssue = true;           // issue!


      if (uop->isInLPIQ()) {
         LOG_ASSERT_ERROR (m_lpiq_fifo.size() > 0, "Uop=%ld has commit dependency, but fifo is empty", uop->getSequenceNumber());
         if (canIssue &&
             (uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESOLVED ||
              uop->getCommitDependency() == DynamicMicroOp::lpiq_t::CHAIN) &&
             uop->getSequenceNumber() == m_lpiq_fifo.front()) {
            uop->removeCommitDependency();
            uop->unsetLPIQ();
            m_lpiq_fifo.pop_front();

         } else {
            canIssue = false;
         }
      }

      if (!canIssue && !uop->getMicroOp()->isVector()) {
         // スカラ命令で命令発行が止まると、それ以降のベクトル命令は発行してはいけない
         vector_someone_cant_be_issued = dyn_vector_inorder;
      }

      bool scalar_lsu_fence = uop->getMicroOp()->isScalarMem() && lsu_inorder && ((m_latest_vecmem_commit_time > now) ||
                                                                                  inhead_vecmem_existed);
      bool v_to_s_block = (v_to_s_fence && inhead_vector_existed && !uop->getMicroOp()->isVector()) || scalar_lsu_fence;

      if (uop->getMicroOp()->isVecMem()) {
         if (!uop->getMicroOp()->canVecSquash()) {
            // Gather/Scatter命令の場合：キャッシュラインマージ操作が入る

            if (uop->getMicroOp()->isVecStore() && 
                        m_vec_store_inorder && vector_store_someone_cant_be_issued) {
               // m_vec_store_inorderが有効だと、ベクトルストアはインオーダで実行されなければならい
               canIssue = false;
            }

            if (canIssue) {

               IntPtr cache_line = uop->getAddress().address & ~(l1d_block_size-1);
               IntPtr banked_cache_line = cache_line & ~(l1d_block_size * m_bank_info.size() - 1);
               IntPtr bank_index = (cache_line ^ banked_cache_line) / l1d_block_size;

               if (m_bank_info[bank_index] == 0) {           // first bank acces
                  if (enable_gatherscatter_log) {
                     fprintf (stderr, "%ld %s cacheline bank initiated %08lx with %08lx. bank=%ld. CanIssue = %d\n",
                              uop->getSequenceNumber(),
                              uop->getMicroOp()->toShortString().c_str(),
                              uop->getAddress().address, m_bank_info[bank_index], bank_index, canIssue);
                  }
                  m_bank_info[bank_index] = banked_cache_line;
               } else if (m_bank_info[bank_index] == banked_cache_line) {
                  // Same Bank Access and Can be Merge:
                  uop->setMemAccessMerge();
                  if (enable_gatherscatter_log) {
                     fprintf (stderr, "%ld %s cacheline bank can be access %08lx with %08lx. bank=%ld, CanIssue = %d\n",
                              uop->getSequenceNumber(),
                              uop->getMicroOp()->toShortString().c_str(),
                              uop->getAddress().address, m_bank_info[bank_index], bank_index, canIssue);
                  }
               } else {
                  canIssue = false;
                  if (enable_gatherscatter_log) {
                     fprintf (stderr, "%ld %s cacheline bank conflict %08lx with %08lx, bank=%ld, CanIssue = %d\n",
                              uop->getSequenceNumber(),
                              uop->getMicroOp()->toShortString().c_str(),
                              uop->getAddress().address, m_bank_info[bank_index], bank_index, canIssue);
                  }
               }

               m_bank_info[bank_index] = banked_cache_line;
            }
         } else {
            // Gather Scatter 以外の命令
            if (uop->getMicroOp()->isVector() && dyn_vector_inorder && vector_someone_cant_be_issued) {
               // vector_someone_cant_be_issuedが立っていると、スカラ命令によってベクトル命令の発行は禁止される
               // ベクトル命令は発行してはならない
               canIssue = false;
            }
            if (uop->getMicroOp()->isVecStore() &&
                        m_vec_store_inorder && vector_store_someone_cant_be_issued) {
               // m_vec_store_inorderが有効だと、ベクトルストアはインオーダで実行されなければならい
               canIssue = false;
            }
         }
      } else if (uop->getMicroOp()->isVector() && dyn_vector_inorder && vector_someone_cant_be_issued) {
         // vector_someone_cant_be_issuedが立っていると、スカラ命令によってベクトル命令の発行は禁止される
         // ベクトル命令は発行してはならない
         KANATA_PRINTF ("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2, "Vector inorder, wait");
         m_kanata_generated_in_this_region = true;
         canIssue = false;
      }

      // Vector to Scalar, Fence mode, Scalar can't continue to isssue.
      inhead_vector_existed |= uop->getMicroOp()->isVector();

      if (v_to_s_block) {
         ROB_DEBUG_PRINTF ("%ld was stopped by Vector to Scalar Fence. PC=%08lx, %s\n",
                           uop->getSequenceNumber(),
                           uop->getAddress().address,
                           uop->getMicroOp()->toShortString().c_str());
         canIssue = false;
         v_to_s_fenced = true;
      }

      // canIssue already marks issue ports as in use, so do this one last
      if (canIssue && m_rob_contention && ! m_rob_contention->tryIssue(*uop)) {
         // if (entry->kanata_registered) {
         //    KANATA_PRINTF ("L\t%ld\t%d\t%s\n", entry->global_sequence_id, 2, "Issue Port, full");
         // }
         canIssue = false;          // blocked by structural hazard
      }

      // 統計情報取得
      if (uop->getMicroOp()->isVector()) {
         // Vector Instructions
         if (canIssue) {
            if (vector_someone_wait_issue) {
               vector_overtake_vector_issue_count ++;
            }
            if (scalar_someone_wait_issue) {
               vector_overtake_scalar_issue_count++;
            }
            if (vector_someone_wait_issue || scalar_someone_wait_issue) {
               vec_ooo_issue_count ++;
            }
         } else if (!uop->isVirtuallyIssued()) {
            vector_someone_wait_issue = true;
         }
      } else {
         // Scalar Instructions
         if (canIssue) {
            if (vector_someone_wait_issue) {
               scalar_overtake_vector_issue_count ++;
            }
            if (scalar_someone_wait_issue) {
               scalar_overtake_scalar_issue_count++;
            }
            if (vector_someone_wait_issue || scalar_someone_wait_issue) {
               scalar_ooo_issue_count++;
            }
         } else {
            scalar_someone_wait_issue = true;
         }
      }

      bool done_preload = false;

      // If Vector and can't be issued, try to preload
      if (uop->isInLPIQ() &&
          m_vec_reserve_policy == vec_reserve_policy_t::VecReserveWhenFull &&
          m_vec_preload &&
          uop->getMicroOp()->isVecMem() && /* uop->getMicroOp()->isLoad() && */
          !uop->isPreloadDone()) {
         if (/* entry->addressReady > now */entry->uop->getDependenciesLength() == 0 && m_rob_contention->tryPreload()) {
            // Pipeline available
            preloadInstruction (i);
            done_preload = true;
         }
      }

      if (canIssue && !done_preload) {
         if (uop->getMicroOp()->isVecLoad()) {
            m_VtoS_RdRequests ++;
         } else if (uop->getMicroOp()->isVecStore()) {
            m_VtoS_WrRequests ++;
         }
      }

      if (canIssue)
      {
         num_issued++;
         issueInstruction(i, next_event);

         // Calculate memory-level parallelism (MLP) for long-latency loads (but ignore overlapped misses)
         if (uop->getMicroOp()->isLoad() && uop->isLongLatencyLoad() && uop->getDCacheHitWhere() != HitWhere::L1_OWN)
         {
            if (m_lastAccountedMemoryCycle < now) m_lastAccountedMemoryCycle = now;

            SubsecondTime done = std::max( now.getElapsedTime(), entry->done );
            // Ins will be outstanding for until it is done. By account beforehand I don't need to
            // worry about fast-forwarding simulations
            m_outstandingLongLatencyInsns += (done - now);

            // Only account for the cycles that have not yet been accounted for by other long
            // latency misses (don't account cycles twice).
            if ( done > m_lastAccountedMemoryCycle )
            {
               m_outstandingLongLatencyCycles += done - m_lastAccountedMemoryCycle;
               m_lastAccountedMemoryCycle = done;
            }

            #ifdef ASSERT_SKIP
            LOG_ASSERT_ERROR( m_outstandingLongLatencyInsns >= m_outstandingLongLatencyCycles, "MLP calculation is wrong: MLP cannot be < 1!"  );
            #endif
         }

         if (uop->getMicroOp()->isStore() && uop->getMicroOp()->isVector()) {
            for(uint64_t younger_index = i + 1; younger_index < m_num_in_rob; younger_index++) {
               RobEntry *younger_entry = &rob.at(younger_index);
               DynamicMicroOp *younger_uop = younger_entry->uop;
               if (!younger_uop->getMicroOp()->isVector() && younger_uop->getMicroOp()->isLoad()) {
                  uint64_t uop_issue_time = SubsecondTime::divideRounded(now, m_core->getDvfsDomain()->getPeriod());
                  uint64_t younger_uop_issue_time = SubsecondTime::divideRounded(younger_entry->issued, m_core->getDvfsDomain()->getPeriod());

                  if (m_enable_ooo_check) {
                     fprintf(stderr, "OoO region check start : %ld(%s):%08lx:%ld <--> %ld(%s):%08lx:%ld : ",
                                    uop->getSequenceNumber(),
                                    uop->getMicroOp()->toShortString().c_str(),
                                    uop->getAddress().address,
                                    uop_issue_time,
                                    younger_uop->getSequenceNumber(),
                                    younger_uop->getMicroOp()->toShortString().c_str(),
                                    younger_uop->getAddress().address,
                                    younger_uop_issue_time);
                  }

                  if (younger_entry->issued <= now &&
                      (younger_uop->getAddress().address & ~(m_ooo_check_region-1)) == (uop->getAddress().address & ~(m_ooo_check_region-1))) {
                     m_ooo_region_count ++;
                     if (m_enable_ooo_check) {
                        fprintf(stderr, "detected\n");
                     }
                  } else {
                     if (m_enable_ooo_check) {
                        fprintf(stderr, "\n");
                     }
                  }
               }
            }
         }

         if (uop->getMicroOp()->isVector() && uop->isReserveInst() && uop->isFirst()) {
            m_inst_vec_reserve_count++;
         }

         #ifdef ASSERT_SKIP
            LOG_ASSERT_ERROR(will_skip == false, "Cycle would have been skipped but stuff happened");
         #endif
      }
      else
      {
         head_of_queue = false;     // Subsequent instructions are not at the head of the ROB

         if (uop->getMicroOp()->isVector() && dyn_vector_inorder && !uop->isVirtuallyIssued()) {
            // 後続のベクトル命令が発行されていない場合
            vector_someone_cant_be_issued = true; // Vector can't continue
         }

         if (uop->getMicroOp()->isVector() &&
             m_vec_store_inorder && !uop->isVirtuallyIssued()) {
            vector_store_someone_cant_be_issued = true; // Vector store can't continue
         }

         if (uop->getMicroOp()->isStore() && entry->addressReady > now)
            have_unresolved_store = true;

         if (dyn_inorder || v_to_s_fenced)
            // In-order: only issue from head of the ROB
            break;
      }

      if (canIssue && uop->getMicroOp()->isVector() &&
          uop->getMicroOp()->UopIdx() == 0) {
        uop->setVirtuallyIssued();
        for (uint64_t j = i+1; j < m_num_in_rob; ++j) {
          RobEntry *subseq_entry = &rob.at(j);
          DynamicMicroOp *subseq_uop = subseq_entry->uop;

          if (subseq_uop->getMicroOp()->getInstruction()->getAddress() ==
              uop->getMicroOp()->getInstruction()->getAddress()) {
            subseq_uop->setVirtuallyIssued();
          } else {
            break;
          }
        }
      }

      if (m_rob_contention)
      {
         if (m_rob_contention->noMore())
            break;
      }
      else
      {
         if (num_issued == dispatchWidth)
            break;
      }
   }

   return next_event;
}

SubsecondTime RobTimer::doCommit(uint64_t& instructionsExecuted)
{
   uint64_t num_committed = 0;
   static bool cycle_activated = false;

   while(rob.size() && (rob.front().done <= now))
   {
      RobEntry *entry = &rob.front();

      if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         std::cout<<"COMMIT   " << "(" << entry->uop->getSequenceNumber() << ") " <<
               entry->uop->getMicroOp()->toShortString()<< "(uop = " << entry->uop->getSequenceNumber() << ")" << std::endl;
      }

      // Send instructions to loop tracer, in-order, once we know their issue time
      InstructionTracer::uop_times_t times = {
         entry->dispatched,
         entry->issued,
         entry->done,
         now
      };
      m_core->getPerformanceModel()->traceInstruction(entry->uop, &times);

      if (entry->uop->isLast())
         instructionsExecuted++;

      if (entry->uop->getSequenceNumber() != 0 && entry->uop->getSequenceNumber() % 10000 == 0) {
         fprintf (stderr, "inst exec %ld (now = %ld cycle)\n", entry->uop->getSequenceNumber(), now.getCycleCount());
      }
      m_last_committed_time = now;

      Instruction *inst = entry->uop->getMicroOp()->getInstruction();
      if (cycle_activated &&
          inst->getDisassembly().find("add            zero, zero, zero") != std::string::npos) {
      }

      if (cycle_activated &&
          inst->getDisassembly().find("add            zero, zero, ra") != std::string::npos &&
          m_konata_count < m_konata_count_max) {

         m_active_o3_gen     = m_enable_o3;
         m_active_kanata_gen = m_enable_kanata;

         std::cout << "KonataStart " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << " "
                   << std::hex << entry->uop->getMicroOp()->getInstruction()->getAddress() << " "
                   << entry->uop->getMicroOp()->getInstruction()->getDisassembly() << '\n';
      }
      if (enable_rob_timer_log &&
          cycle_activated &&
          inst->getDisassembly().find("add            zero, zero, sp") != std::string::npos) {
        m_active_o3_gen = false;
        m_active_kanata_gen = false;
        std::cout << "KonataStop " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << " "
                  << std::hex << entry->uop->getMicroOp()->getInstruction()->getAddress() << " "
                  << entry->uop->getMicroOp()->getInstruction()->getDisassembly() << '\n';
      }
      if (m_active_o3_gen &&
          m_konata_count >= m_konata_count_max) {
        m_active_o3_gen = false;
      }
      if (m_active_kanata_gen &&
          m_konata_count >= m_konata_count_max) {
         std::cout << "Kanata stop to generate : exceeded " << std::dec << m_konata_count_max << " count.\n";
         m_active_kanata_gen = false;
      }

      if (m_active_kanata_gen) {
        m_konata_count ++;
      }

      if (inst->getDisassembly().find("cycle") != std::string::npos) {
        cycle_activated = true;
      } else {
        cycle_activated = false;
      }

      if (m_active_o3_gen) {

        uint64_t cycle_fetch    = SubsecondTime::divideRounded(entry->fetch,      m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_dispatch = SubsecondTime::divideRounded(entry->dispatched, m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_issue    = SubsecondTime::divideRounded(entry->issued,     m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_done     = SubsecondTime::divideRounded(entry->done,       m_core->getDvfsDomain()->getPeriod());
        uint64_t cycle_commit   = SubsecondTime::divideRounded(times.commit,      m_core->getDvfsDomain()->getPeriod());

        cycle_fetch = cycle_fetch == 0 ? 4 : cycle_fetch;
        Instruction *inst = entry->uop->getMicroOp()->getInstruction();
        fprintf (m_core->getO3Fp(), "O3PipeView:fetch:%ld:0x%08lx:0:%ld:%s\n",
                 (cycle_fetch-3)*500,
                 (uint64_t)inst->getAddress(),
                 entry->uop->getSequenceNumber(),
                 inst->getDisassembly().c_str());
        fprintf (m_core->getO3Fp(), "O3PipeView:decode:%ld\n",                (cycle_dispatch-2)*500);
        fprintf (m_core->getO3Fp(), "O3PipeView:rename:%ld\n",                (cycle_dispatch-1)*500);
        fprintf (m_core->getO3Fp(), "O3PipeView:dispatch:%ld\n",              (cycle_dispatch  )*500);
        fprintf (m_core->getO3Fp(), "O3PipeView:issue:%ld\n",                 (cycle_issue     )*500);
        fprintf (m_core->getO3Fp(), "O3PipeView:complete:%ld\n",              (cycle_done      )*500);
        fprintf (m_core->getO3Fp(), "O3PipeView:retire:%ld:store:0\n",        (cycle_commit    )*500);
      }

      if (!entry->uop->getMicroOp()->isVector() && entry->uop->getMicroOp()->isLoad()) {
         scalar_load_queue++;
      }
      if (!entry->uop->getMicroOp()->isVector() && entry->uop->getMicroOp()->isStore()) {
         scalar_store_queue++;
      }
      if (is_vldq_release(entry->uop)) {
         vec_load_queue++;
         // fprintf(stderr, "vec_load_queue increase: idx=%ld VLDQ=%ld %d %s\n",
         //          entry->uop->getSequenceNumber(), vec_load_queue, entry->uop->isFirst(),
         //          entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
      }
      if (!m_vec_store_inorder && entry->uop->getMicroOp()->isVecStore() && entry->uop->isLast()) {
         vec_store_queue += m_vlen / 64;
      }

      releaseRegister (entry);

      if (entry->kanata_registered) {
         KANATA_PRINTF ("E\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "Cm");
         KANATA_PRINTF ("R\t%ld\t%ld\t%d\n", entry->global_sequence_id, entry->uop->getSequenceNumber(), 0);
         m_kanata_generated_in_this_region = true;
      }

      if (entry->uop->isLast()) {
         m_num_in_rob_head--;
      }

      entry->free();
      rob.pop();
      m_num_in_rob--;


      #ifdef ASSERT_SKIP
         LOG_ASSERT_ERROR(will_skip == false, "Cycle would have been skipped but stuff happened");
      #endif

      ++num_committed;
      if (num_committed == commitWidth)
         break;
   }

   if (rob.size())
      return rob.front().done;
   else
      return SubsecondTime::MaxTime();
}

void RobTimer::execute(uint64_t& instructionsExecuted, SubsecondTime& latency)
{
   latency = SubsecondTime::Zero();
   instructionsExecuted = 0;
   SubsecondTime *cpiComponent = NULL;

   if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
      std::cout<<std::endl;
      std::cout<<"Running cycles "<< std::dec << SubsecondTime::divideRounded(now, now.getPeriod())<<std::endl;
   }

   if (m_kanata_generated_in_this_region && m_last_kanata_time != now) {
      KANATA_PRINTF("C\t%ld\t// %ld\n",
                    SubsecondTime::divideRounded(now - m_last_kanata_time,
                                                 now.getPeriod()),
                    now.getCycleCount());
      m_last_kanata_time = now;
      m_kanata_generated_in_this_region = false;
   }

   if (m_roi_started) {
     std::cout << "CycleTrace " << std::dec << SubsecondTime::divideRounded(now, now.getPeriod()) << '\n';
     m_roi_started = false;
   }

   // If frontend not stalled
   if (frontend_stalled_until <= now)
   {
      if (rob.size() < std::min(m_num_in_rob + 2*dispatchWidth, windowSize))
      {
         // We don't have enough instructions to dispatch <dispatchWidth> new ones. Ask for more before doing anything this cycle.
         return;
      }
   }


   // Model dispatch, issue and commit stages
   // Decode stage is not modeled, assumes the decoders can keep up with (up to) dispatchWidth uops per cycle

   SubsecondTime next_dispatch = doDispatch(&cpiComponent);
   releaseLPIQ (); // LPIQの先頭でハザードが消えていれば，それはLPIQから取り出す．
   m_1st_issue_in_cycle = true;
   SubsecondTime next_issue    = doIssue();
   SubsecondTime next_commit   = doCommit(instructionsExecuted);

   for (unsigned int i = 0; i < rob.size(); ++i) {
      RobEntry *e = &rob.at(i);

      if (e->done == now) {
         if (!e->kanata_registered) {
            DynamicMicroOp *uop = e->uop;
            e->global_sequence_id = m_core->getGlobalSequenceIdAndInc();
            KANATA_PRINTF("I\t%ld\t%d\t%d\n", e->global_sequence_id, 0, 0);
            KANATA_PRINTF(
                "L\t%ld\t%d\t%08lx:%s\n", e->global_sequence_id, 0,
                uop->getMicroOp()->getInstruction()->getAddress(),
                uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
            m_kanata_generated_in_this_region = true;
         } else {
            KANATA_PRINTF("E\t%ld\t%d\t%s\n", e->global_sequence_id, 0, "X");
            KANATA_PRINTF("S\t%ld\t%d\t%s\n", e->global_sequence_id, 0, "Cm");
            m_kanata_generated_in_this_region = true;
         }
         m_konata_count++;
      }
   }

   if (true) {
      #ifdef ASSERT_SKIP
         if (! will_skip)
         {
      #endif
           printRob(enable_rob_debug_seqnumber || ((enable_rob_timer_log || m_show_rob) && now.getCycleCount() >= rob_start_cycle));
      #ifdef ASSERT_SKIP
         }
      #endif
   }

   m_reg_manager->UpdateRegisterStats();

   if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
      std::cout << "Next event: D(" << SubsecondTime::divideRounded(next_dispatch, now.getPeriod())
                << ") I(" << SubsecondTime::divideRounded(next_issue, now.getPeriod())
                << ") C(" <<SubsecondTime::divideRounded(next_commit, now.getPeriod())
                << ")"<<std::endl;
   }
   SubsecondTime next_event = std::min(next_dispatch, std::min(next_issue, next_commit));
   SubsecondTime skip;
   if (next_event != SubsecondTime::MaxTime() && next_event > now + 1ul)
   {
      if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         std::cout<<"++ Skip "<<SubsecondTime::divideRounded(next_event - now, now.getPeriod())<<std::endl;
      }
      will_skip = true;
      skip = next_event - now;
   }
   else
   {
      will_skip = false;
      skip = now.getPeriod();
   }

   // プリフェッチの可否は毎サイクルチェックする
   HitWhere::where_t result = m_core->doPrefetch (now.getElapsedTime(), Core::NONE, Core::READ_VEC);
   if (result == HitWhere::L1_OWN) {
      will_skip = false;
      skip = now.getPeriod();
   }

   // プリフェッチのKanata用トレースの出力
   if (m_enable_kanata) {
      // fprintf (stderr, "prefetch_arrive_list = %ld\n", m_core->prefetch_arrive_list.size());
      for (auto it = m_core->prefetch_arrive_list.begin(); it != m_core->prefetch_arrive_list.end();) {
         // UInt64 global_id = it->first;
         UInt64 cycle     = it->second;
         // KANATA_PRINTF ("  // %ld %ld\n", now.getElapsedTime().getNS(), cycle);
         if (now.getElapsedTime().getNS() > cycle) {
            // KANATA_PRINTF ("E\t%ld\t%d\tP\n", global_id, 0);
            // KANATA_PRINTF ("R\t%ld\t%d\t0\n", global_id, 0);
            it = m_core->prefetch_arrive_list.erase(it);
         } else {
            it++;
         }
      }
   }

   #ifdef ASSERT_SKIP
      now += now.getPeriod();
      latency += now.getPeriod();
      if (will_skip)
         time_skipped += now.getPeriod();
   #else
      now += skip;
      latency += skip;
      if (skip > now.getPeriod())
         time_skipped += skip - now.getPeriod();
   #endif

   if (m_mlp_histogram)
      countOutstandingMemop(skip);

   LOG_ASSERT_ERROR(cpiComponent != NULL, "We expected cpiComponent to be set by doDispatch, but it wasn't");
   *cpiComponent += latency;

   m_frontstall[m_frontstall_idx] += latency;
}

void RobTimer::countOutstandingMemop(SubsecondTime time)
{
   UInt64 counts[HitWhere::NUM_HITWHERES] = {0}, total = 0;

   for(unsigned int i = 0; i < m_num_in_rob; ++i)
   {
      RobEntry *e = &rob.at(i);
      if (e->done != SubsecondTime::MaxTime() && e->done > now && e->uop->getMicroOp()->isLoad())
      {
         ++counts[e->uop->getDCacheHitWhere()];
         ++total;
      }
   }

   for(unsigned int h = 0; h < HitWhere::NUM_HITWHERES; ++h)
      if (counts[h] > 0)
         m_outstandingLoads[h][counts[h] >= MAX_OUTSTANDING ? MAX_OUTSTANDING-1 : counts[h]] += time;
   if (total > 0)
      m_outstandingLoadsAll[total >= MAX_OUTSTANDING ? MAX_OUTSTANDING-1 : total] += time;
}


#define DEBUG_COUT_IF(out,x) do { if (is_output) { out << x; } } while (0)

void RobTimer::printRob(bool is_output, bool enable_check)
{
   if (Sim()->getCfg()->getBoolArray("log/regression_mode", m_core->getId())) {
      return;
   }

   DEBUG_COUT_IF (std::cout, "** ROB state @ "<<SubsecondTime::divideRounded(now, now.getPeriod())<<"  size("<<m_num_in_rob<<") size_head(" << m_num_in_rob_head <<") total("<<rob.size()<<")" << std::endl);
   if (frontend_stalled_until > now)
   {
      DEBUG_COUT_IF (std::cout, "   Front-end stalled");
      if (frontend_stalled_until != SubsecondTime::MaxTime())
         DEBUG_COUT_IF (std::cout, " until " << SubsecondTime::divideRounded(frontend_stalled_until, now.getPeriod()));
      if (in_icache_miss)
         DEBUG_COUT_IF (std::cout, ", in I-cache )miss");
      DEBUG_COUT_IF (std::cout, std::endl);
   }

   DEBUG_COUT_IF (std::cout, "   Int Regs  : "<< std::dec << m_reg_manager->getAllocIntRegister()    << std::endl);
   DEBUG_COUT_IF (std::cout, "   Float Regs: "<< std::dec << m_reg_manager->getAllocFloatRegister()  << std::endl);
   DEBUG_COUT_IF (std::cout, "   Vec Regs  : "<< std::dec << m_reg_manager->getAllocVectorRegister() << std::endl);
   if (m_vec_reserve_policy != VecReserveNone) {
      DEBUG_COUT_IF (std::cout, "     Low Priority  : "<< std::dec << m_reg_manager->getNonPriVectorRegisters() << std::endl);
   }
   DEBUG_COUT_IF (std::cout, "   LPIQ entries: "<< m_lpiq_fifo.size() << " ");
   if (m_lpiq_fifo.size() > 0) {
      DEBUG_COUT_IF (std::cout,  ", head=" << m_lpiq_fifo.front() << ", ");
      auto it = m_lpiq_fifo.begin();
      for (int i = 0; it != m_lpiq_fifo.end() && i < 16; it++, i++) {
         DEBUG_COUT_IF (std::cout, *it << " ");
      }
   }
   DEBUG_COUT_IF (std::cout,  "\n");

   DEBUG_COUT_IF (std::cout, "   RS entries: "<<m_rs_entries_used<<std::endl);
   DEBUG_COUT_IF (std::cout, "   Vec RS entries : " << m_vec_num_in_rs << ", max = " << m_vec_window_size << std::endl);
   DEBUG_COUT_IF (std::cout, "   Outstanding loads: "<<load_queue.getNumUsed(now)<<"  stores: "<<store_queue.getNumUsed(now)<<std::endl);
   DEBUG_COUT_IF (std::cout, "   VLDQ entries remained: "<< vec_load_queue << "  VSTQ entries remained: "<< vec_store_queue << std::endl);

   std::unordered_map<UInt64, PriorityManager::inst_priority_t> *priority_map = m_priority_manager->getPriorityMap();
   DEBUG_COUT_IF (std::cout, "   Priority Manager High   : ");
   for (auto it = priority_map->begin(); it != priority_map->end(); it++) {
      if (it->second == PriorityManager::inst_priority_t::High) {
         DEBUG_COUT_IF (std::cout, std::hex << it->first << ",");
      }
   }
   DEBUG_COUT_IF (std::cout, "\n");
   DEBUG_COUT_IF (std::cout, "   Priority Manager Reserve: ");
   for (auto it = priority_map->begin(); it != priority_map->end(); it++) {
      if (it->second == PriorityManager::inst_priority_t::Reserve) {
         DEBUG_COUT_IF (std::cout, std::hex << it->first << ",");
      }
   }
   DEBUG_COUT_IF (std::cout, "\n");

   static size_t vec_store_queue_max = Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_vec_stores");
   LOG_ASSERT_ERROR(vec_store_queue <= vec_store_queue_max, "Vec Store Queue exceeded default value. %ld <= %ld",
                     vec_store_queue, vec_store_queue_max);

   size_t vecstore_count = 0;

   UInt64 vecreg_normal_alloc_count = 0;
   UInt64 vecreg_lowpri_alloc_count = 0;

   bool   normal_decided = false;
   bool   lowpri_decided = false;

   for(unsigned int i = 0; i < rob.size(); ++i)
   {
      RobEntry *e = &rob.at(i);

      DEBUG_COUT_IF (std::cout, "   ["<<std::setw(3)<<i<<"]  ");

      std::ostringstream state;

      dl::Decoder *dec = Sim()->getDecoder();
      if (/* i < m_num_in_rob && */
          !normal_decided &&
          e->uop->getMicroOp()->isVector() &&
          e->uop->getMicroOp()->getDestinationRegistersLength() != 0 &&
          dec->is_reg_vector(e->uop->getMicroOp()->getDestinationRegister(0))
      ) {
         if (!e->uop->getRegisterAllocated()) {
            DEBUG_COUT_IF (state, "    ");
         } else if (isUseNonpriVector (m_vec_reserve_policy)) {
            if (e->uop->isUseNormalRegisterGroup()) {
               vecreg_normal_alloc_count++;
               DEBUG_COUT_IF (state, std::setw(3) << (vecreg_normal_alloc_count) << ' ');
               normal_decided = true;
            } else {
               DEBUG_COUT_IF (state, "    ");
            }
         } else if (m_vec_reserve_policy == VecReserveWhenFull) {
            if (!e->uop->isInLPIQ() ||
                (e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESOLVED)) {
               vecreg_normal_alloc_count++;
               DEBUG_COUT_IF (state, std::setw(3) << (vecreg_normal_alloc_count) << ' ');
               normal_decided = true;
            } else {
               DEBUG_COUT_IF (state, "    ");
            }
         } else {
            vecreg_normal_alloc_count++;
            DEBUG_COUT_IF (state, std::setw(3) << (vecreg_normal_alloc_count) << ' ');
            normal_decided = true;
         }
      } else {
         DEBUG_COUT_IF (state, "    ");
      }

      if (m_vec_reserve_policy == VecReserveParOOO) {
         // Inorderの場合は，予約はカウントしない
      } else if (i < m_num_in_rob &&
          !lowpri_decided &&
          e->uop->getMicroOp()->isVector() &&
          e->uop->getMicroOp()->getDestinationRegistersLength() != 0 &&
          dec->is_reg_vector(e->uop->getMicroOp()->getDestinationRegister(0))
      ) {
         if (isUseNonpriVector (m_vec_reserve_policy)) {
            if (e->uop->isUseReserveRegisterGroup() &&
                (e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESOLVED ||
                 e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::CHAIN)) {
               // 予約のレジスタが渡されて、レジスタ待ちが解放されている(レジスタ保持中)
               ++vecreg_lowpri_alloc_count;
               DEBUG_COUT_IF (state, std::setw(3) << (vecreg_lowpri_alloc_count) << ' ');
            } else {
               DEBUG_COUT_IF (state, "    ");
            }
         }
         lowpri_decided = true;
      } else {
         DEBUG_COUT_IF (state, "    ");
      }

      if (e->uop->isLast()) {
         normal_decided = false;
         lowpri_decided = false;
      }

      // if (i < m_num_in_rob &&
      //     e->uop->getMicroOp()->isVector() &&
      //     e->uop->isFirst() &&
      //     e->uop->getMicroOp()->getDestinationRegistersLength() != 0 &&
      //     dec->is_reg_vector(e->uop->getMicroOp()->getDestinationRegister(0))) {
      //    if (e->uop->isInLPIQ() && e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESREG) {
      //       break_vecreg_normal_alloc_count = true;
      //    }
      // }

      if (i < m_num_in_rob && e->uop->getMicroOp()->isVector()) {
         if (e->uop->isStrongPriorityInst()) {
            DEBUG_COUT_IF (state, " P ");
         } else if (e->uop->isReserveInst()) {
            DEBUG_COUT_IF (state, " R ");
         } else {
            DEBUG_COUT_IF (state, " N ");  // Normal
         }
      } else {
         DEBUG_COUT_IF (state, "   ");
      }

      if (e->uop->getMicroOp()->isVector()) {
         if (e->uop->isInLPIQ() && e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESREG) {
            DEBUG_COUT_IF (state, "LPIQ(PR) ");
         } else if (e->uop->isInLPIQ() && e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::TRANSREG) {
            DEBUG_COUT_IF (state, "LPIQ(TR) ");
         } else if (e->uop->isInLPIQ() && e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESOLVED) {
            DEBUG_COUT_IF (state, "LPIQ(ok) ");
         } else if (e->uop->isInLPIQ() && e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::CHAIN) {
            DEBUG_COUT_IF (state, "LPIQ(--) ");
         } else if (e->uop->isInLPIQ() && e->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::SQ) {
            DEBUG_COUT_IF (state, "LPIQ(SQ) ");
         } else if (i >= m_num_in_rob) {
            DEBUG_COUT_IF (state, "PREROB    ");
         } else {
            DEBUG_COUT_IF (state, "          ");
         }
      } else {
         DEBUG_COUT_IF (state, "          ");
      }

      if (e->done != SubsecondTime::MaxTime()) {
         uint64_t cycles;
         if (e->done > now)
            cycles = SubsecondTime::divideRounded(e->done-now, now.getPeriod());
         else
            cycles = 0;
         DEBUG_COUT_IF (state, "DONE@+"<<cycles<<"  ");
      }
      else if (e->ready != SubsecondTime::MaxTime()) {
         uint64_t cycles;
         if (e->ready > now)
            cycles = SubsecondTime::divideRounded(e->ready-now, now.getPeriod());
         else
            cycles = 0;
         DEBUG_COUT_IF (state, "READY@+"<<cycles<<"  ");
      }
      else
      {
         DEBUG_COUT_IF (state, "DEPS ");
         for(uint32_t j = 0; j < std::min(e->uop->getDependenciesLength(), 4U); j++) {
            DEBUG_COUT_IF (state, std::dec << e->uop->getDependency(j) << " ");
         }
         if (e->uop->getDependenciesLength() > 4) {
            DEBUG_COUT_IF (state, "...");
         }
      }
      DEBUG_COUT_IF (std::cout, std::left<<std::setw(48)<<state.str()<<"   ");
      DEBUG_COUT_IF (std::cout, std::right<<std::setw(10)<<e->uop->getSequenceNumber()<<"  ");
      if (e->uop->getMicroOp()->isLoad())
         DEBUG_COUT_IF (std::cout, "LOAD      ");
      else if (e->uop->getMicroOp()->isStore())
         DEBUG_COUT_IF (std::cout, "STORE     ");
      else
         DEBUG_COUT_IF (std::cout, "EXEC ("<<std::right<<std::setw(2)<<e->uop->getExecLatency()<<") ");
      if (e->uop->isFirst()) {
         DEBUG_COUT_IF (std::cout, "F");
      } else if (e->uop->isLast()) {
         DEBUG_COUT_IF (std::cout, "L");
      } else {
         DEBUG_COUT_IF (std::cout, " ");
      }
      if (e->uop->isVirtuallyIssued()) {
         DEBUG_COUT_IF (std::cout, "V");
      } else {
         DEBUG_COUT_IF (std::cout, " ");
      }
      if (e->uop->getMicroOp()->getInstruction())
      {
         DEBUG_COUT_IF (std::cout, std::hex<<e->uop->getMicroOp()->getInstruction()->getAddress()<<std::dec<<": "
                        <<e->uop->getMicroOp()->getInstruction()->getDisassembly());
         if (e->uop->getMicroOp()->isLoad() || e->uop->getMicroOp()->isStore())
            DEBUG_COUT_IF (std::cout, "  {0x"<<std::hex<<e->uop->getAddress().address<<std::dec<<"}");
      }
      else
         DEBUG_COUT_IF (std::cout, "(dynamic)");

      if (e->uop->getMicroOp()->isVecMem()) {
         if (e->uop->isPreloadDone()) {
            DEBUG_COUT_IF (std::cout, " PRELD");
         } else {
            DEBUG_COUT_IF (std::cout, "      ");
         }
      }

      if (e->uop->getMicroOp()->isLoad() || e->uop->getMicroOp()->isStore()) {
         DEBUG_COUT_IF (std::cout, "(" << HitWhereString(e->uop->getDCacheHitWhere()) << ", "
                        << e->uop->getExecLatency() << ")");
      }
      DEBUG_COUT_IF (std::cout, std::endl);

      if (i < m_num_in_rob &&
         //  e->uop->isInLPIQ() &&
          !e->uop->isReserveInst() &&  // In-Orderの命令はVSTQに入れない
          e->uop->getMicroOp()->isVecStore()) {
         // fprintf (stderr, "inflight Vector Store %ld\n", e->uop->getSequenceNumber());
         vecstore_count += 1;
      }
   }

   if (enable_check &&
       (vecreg_normal_alloc_count + 32 + vecreg_lowpri_alloc_count != m_reg_manager->getAllocVectorRegister())) {
      printRob(true, false);
      LOG_ASSERT_ERROR (false, "Cycle = %ld\nVec register count failed.\n"
                        "  vecreg_normal_alloc_count(%ld) + 32 + getNonPriVectorRegisters(%ld) = %d\n"
                        "  getAllocVectorRegister = %d\n",
                        now.getCycleCount(),
                        vecreg_normal_alloc_count, vecreg_lowpri_alloc_count,
                        vecreg_normal_alloc_count + 32 + vecreg_lowpri_alloc_count,
                        m_reg_manager->getAllocVectorRegister());
   }

   // if (enable_check && !m_vec_store_inorder &&
   //    (vec_store_queue_max - vec_store_queue != vecstore_count)) {
   //    printRob(true, false);
   //    LOG_ASSERT_ERROR(false,
   //                  "Vec store count mismatch : vec_store_queue = %ld, vecstore_count = %ld\n",
   //                  vec_store_queue, vecstore_count);
   // }
}


void RobTimer::setVSETDependencies(DynamicMicroOp& microOp, uint64_t lowestValidSequenceNumber)
{
  dl::Decoder *dec = Sim()->getDecoder();

  if (dec->is_vsetvl(microOp.getMicroOp()->getInstructionOpcode())) {
    m_vsetvl_producer = microOp.getSequenceNumber();
  } else if (dec->is_vector(microOp.getMicroOp()->getInstructionOpcode(),
                            microOp.getMicroOp()->getDecodedInstruction())) {
    if (m_vsetvl_producer >= lowestValidSequenceNumber) {
      microOp.addDependency(m_vsetvl_producer);
    }
  }
}


void RobTimer::preloadInstruction(uint64_t rob_idx)
{
   RobEntry *entry = &rob[rob_idx];
   DynamicMicroOp &uop = *entry->uop;

   ROB_DEBUG_PRINTF ("PRELOAD TRY %ld, %s\n", uop.getSequenceNumber(), entry->uop->getMicroOp()->toShortString().c_str());

   if ((uop.getMicroOp()->isLoad() || uop.getMicroOp()->isStore())
       && uop.getDCacheHitWhere() == HitWhere::UNKNOWN) {
      // Vector instruction, previous access merge, it can be skipped

      if (!uop.getMemAccessMerge()) {
         uint64_t access_size_scale = uop.getMicroOp()->isVector() ? uop.getNumMergedInst() + 1 : 1;
         /* MemoryResult res = */ m_core->accessMemory(
             Core::NONE,
             Core::PRELOAD,
             uop.getAddress().address,
             NULL,
             uop.getMicroOp()->getMemoryAccessSize() * access_size_scale,
             Core::MEM_MODELED_RETURN,
             static_cast<uint64_t>(NULL),
             uop.getSequenceNumber(),
             now.getElapsedTime(),
             false /* use_prefetch*/
         );

         // if (enable_rob_timer_log && now.getCycleCount() >= rob_start_cycle) {
         fprintf (stderr, "PRELOAD seqid=%ld, addr=%08lx, %s\n",
                  uop.getSequenceNumber(),
                  uop.getAddress().address,
                  uop.getMicroOp()->getInstruction()->getDisassembly().c_str());
         // }
         ROB_DEBUG_PRINTF ("  Early preload : tryIssue succeeded %s, rod_idx = %ld, index = %ld\n",
                           uop.getMicroOp()->toShortString().c_str(),
                           rob_idx,
                           uop.getSequenceNumber());
         uop.setPreloadDone();
         KANATA_PRINTF ("S\t%ld\t%d\t%s\n", entry->global_sequence_id, 0, "P");
         KANATA_PRINTF ("L\t%ld\t%d\tAddress=%08lx\n", entry->global_sequence_id, 1, uop.getAddress().address);
         m_kanata_generated_in_this_region = true;

         m_preload_count ++;
         UpdatePreloadStats (&uop);
      } else {
         ROB_DEBUG_PRINTF ("  TRY %ld Failure. Merge access failed\n", uop.getSequenceNumber());
      }
   } else {
      ROB_DEBUG_PRINTF ("  TRY %ld Failure. DCacheHitWhere failed\n", uop.getSequenceNumber());
   }
}

bool RobTimer::InsertLPIQ (DynamicMicroOp *uop, DynamicMicroOp::lpiq_t reason)
{
   if (m_lpiq_fifo.size() < LPIQ_SIZE) {
      if (m_lpiq_fifo.size() > 0) {
         LOG_ASSERT_ERROR(m_lpiq_fifo.back() <= uop->getSequenceNumber(), "1. inserted FIFO age should be larger than last entry");
      }

      LOG_ASSERT_ERROR (m_lpiq_fifo.back() != uop->getSequenceNumber(), "uop is same as dispatch.back() = %ld\n", uop->getSequenceNumber());

      m_lpiq_fifo.push_back(uop->getSequenceNumber());
      m_lpiq_inserted ++;
      uop->setCommitDependency (reason);
      uop->setLPIQ ();
      m_last_lpiq_sequencenumber = uop->getSequenceNumber();
      UpdateLPIQStats(uop);

      RobEntry *uop_entry = this->findEntryBySequenceNumber(uop->getSequenceNumber());
      uop_entry->lpiq_inserted = now.getCycleCount();

      ROB_DEBUG_PRINTF ("InsertLPIQ : uop_idx=%ld %s reason=%d\n", uop->getSequenceNumber(),
                        uop->getMicroOp()->getInstruction()->getDisassembly().c_str(), reason);

      return true;
   } else {
      LOG_ASSERT_ERROR(false, "LPIQ fullfilled. Currently it's not supported. Consired increasing LPIQ_SIZE");
      // m_lpiq_overflow++;
      // return false;
   }
}


// ----------------------------------------------------
// If Vector Memory Store,
// ----------------------------------------------------
bool RobTimer::ReserveVSTQ (uint64_t rob_idx)
{
   RobEntry *entry = &rob.at(rob_idx);
   DynamicMicroOp *uop = entry->uop;

   // if (!uop->isFirst()) {
   //    return true;
   // }

   if (uop->getMicroOp()->isVecStore()) {
      // fprintf (stderr, "ReserveVSTQ seqId=%ld, ", uop->getSequenceNumber());
      if (vec_store_queue == 0) {
         // ここに到達したということは、ベクトル命令のベクトル資源が枯渇したことを意味するので、FIFOに格納する。
         if (m_vec_reserve_policy == vec_reserve_policy_t::VecReserveWhenFull && m_lpiq_fifo.size() < LPIQ_SIZE) {
            // if (m_lpiq_fifo.size() > 0) {
            //    LOG_ASSERT_ERROR(m_lpiq_fifo.back() < uop->getSequenceNumber(),
            //                     "0. inserted FIFO age should be larger than last entry");
            // }
            if (m_lpiq_fifo.back() == uop->getSequenceNumber()) {
               // RegisterチェックでW-FIFOに依存関係のあるベクトルストアで，
               // かつSTQの数が足りない
               // --> ハザードの種類をSQに置き換える
               uop->setCommitDependency (DynamicMicroOp::lpiq_t::SQ);
            } else {
               m_lpiq_fifo.push_back(uop->getSequenceNumber());
               m_lpiq_inserted ++;
               UpdateLPIQStats(uop);
               uop->setCommitDependency (DynamicMicroOp::lpiq_t::SQ);
               uop->setLPIQ();
            }
            // fprintf (stderr, "setCommitDependency()\n");
            return true;
         } else {
            // fprintf (stderr, "LPIQ full\n");
            return false;
         }
      } else {
         // fprintf (stderr, "Allocate VSTQ\n");
         vec_store_queue -= 1;
         return true;
      }
   } else {
      // fprintf (stderr, "non Vector Store\n");
      return true;
   }
}


void RobTimer::releaseLPIQ ()
{
   if (m_lpiq_fifo.size() > 0) {
      RobEntry *lpiq_front_entry = this->findEntryBySequenceNumber(m_lpiq_fifo.front());
      if (lpiq_front_entry->uop->isInLPIQ() &&
          lpiq_front_entry->lpiq_inserted > now.getCycleCount() &&
          lpiq_front_entry->uop->getCommitDependency() == DynamicMicroOp::lpiq_t::RESOLVED) {
         lpiq_front_entry->uop->removeCommitDependency();
         m_lpiq_fifo.pop_front();
         lpiq_front_entry->uop->unsetLPIQ ();
         KANATA_PRINTF ("E\t%ld\t%d\t%s\n", lpiq_front_entry->global_sequence_id, 0, "Wf");
         KANATA_PRINTF ("S\t%ld\t%d\t%s\n", lpiq_front_entry->global_sequence_id, 0, "Ds");
         lpiq_front_entry->lpiq_released = now.getCycleCount();
         UpdateVectorLPIQStats (lpiq_front_entry->uop,
                                lpiq_front_entry->lpiq_released - lpiq_front_entry->lpiq_inserted);
         // if (lpiq_front_entry->uop->getMicroOp()->getInstruction()->getAddress() == 0x149a8) {
         //    fprintf (stderr, "0x149a8 LPIQ latency = %ld\n",
         //             lpiq_front_entry->lpiq_released - lpiq_front_entry->lpiq_inserted);
         // }
         ROB_DEBUG_PRINTF ("RobTimer::releaseLPIQ succeeded : uop_idx=%ld %s\n",
                           lpiq_front_entry->uop->getSequenceNumber(),
                           lpiq_front_entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
      } else {
         ROB_DEBUG_PRINTF ("RobTimer::releaseLPIQ failed : uop_idx=%ld %s\n",
                           lpiq_front_entry->uop->getSequenceNumber(),
                           lpiq_front_entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
      }
   } else {
      ROB_DEBUG_PRINTF ("RobTimer::releaseLPIQ none\n");
   }

   return;
}
