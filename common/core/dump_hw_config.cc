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
}
