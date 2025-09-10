#pragma once

#include <cstdint>
#include <stdint.h>

class DumpHwConfig
{
    uint64_t scalar_load_queue;
    uint64_t scalar_store_queue = 0;
    uint64_t vec_load_queue = 0;
    uint64_t vec_store_queue = 0;
    uint64_t scalar_alu_iq_size = 0;
    uint64_t scalar_lsu_iq_size = 0;
    uint64_t scalar_fpu_iq_size = 0;
    uint64_t vec_iq_size = 0;
    uint64_t vec_load_queue_size = 0;
    
    public:
    DumpHwConfig(uint64_t id);
};

