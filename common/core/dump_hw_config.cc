#include "dump_hw_config.h"
#include "stats.h"
#include "config.hpp"

#define ADD_METRIC(name, cfg) \
    static uint64_t name = 0; \
    registerStatsMetric("cfg", id, #name, &name); \
    name = Sim()->getCfg()->getInt(cfg);

DumpHwConfig::DumpHwConfig(uint64_t id)
{
    // dump hardware configuration
    registerStatsMetric("cfg", id, "outstanding_loads", &scalar_load_queue);
    scalar_load_queue = Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_loads", id);

    registerStatsMetric("cfg", id, "outstanding_stores", &scalar_store_queue);
    scalar_store_queue = Sim()->getCfg()->getIntArray("perf_model/core/rob_timer/outstanding_stores", id);

    registerStatsMetric("cfg", id, "outstanding_vec_loads", &vec_load_queue);
    vec_load_queue = Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_vec_loads");

    registerStatsMetric("cfg", id, "outstanding_vec_stores", &vec_store_queue);
    vec_store_queue = Sim()->getCfg()->getInt("perf_model/core/rob_timer/outstanding_vec_stores");

    ADD_METRIC(int_prf, "perf_model/core/rob_timer/int_physical_registers");
    ADD_METRIC(fp_prf,  "perf_model/core/rob_timer/float_physical_registers");
    ADD_METRIC(vec_prf, "perf_model/core/rob_timer/vec_physical_registers");

    ADD_METRIC(vlen, "general/vlen");
    ADD_METRIC(dlen, "general/dlen");

    ADD_METRIC(vec_iq_size, "perf_model/core/interval_timer/vec_window_size");

    scalar_alu_iq_size = Sim()->getCfg()->getInt("perf_model/core/interval_timer/alu_window_size");
    scalar_lsu_iq_size = Sim()->getCfg()->getInt("perf_model/core/interval_timer/lsu_window_size");
    scalar_fpu_iq_size = Sim()->getCfg()->getInt("perf_model/core/interval_timer/fpu_window_size");
    registerStatsMetric("cfg", id, "scalar_alu_iq_size", &scalar_alu_iq_size);
    registerStatsMetric("cfg", id, "scalar_lsu_iq_size", &scalar_lsu_iq_size);
    registerStatsMetric("cfg", id, "scalar_fpu_iq_size", &scalar_fpu_iq_size);

    ADD_METRIC(rob_size, "perf_model/core/interval_timer/rob_hw_size");

    ADD_METRIC(dram_latency, "perf_model/dram/latency");

    bool use_bloom_filter = Sim()->getCfg()->getBool("perf_model/core/rob_timer/bloom_filter");
    if (use_bloom_filter) {
        vldq_width = 1 << Sim()->getCfg()->getInt("perf_model/core/rob_timer/bloom_filter_length");
    } else {
        vldq_width = 64;
    }
    registerStatsMetric("cfg", id, "vldq_width", &vldq_width);

    ADD_METRIC(scalar_ldq_size, "perf_model/core/rob_timer/outstanding_loads");
    ADD_METRIC(scalar_stq_size, "perf_model/core/rob_timer/outstanding_stores");
}
