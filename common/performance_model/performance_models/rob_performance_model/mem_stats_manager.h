#pragma once

#include "interval_timer.h"
#include "rob_contention.h"
#include "stats.h"

#include <unordered_map>
#include <deque>
#include <cstdint>
#include <cstdio>

#define MEMSTATS_DEBUG_PRINTF(...) { if (*enable_rob_timer_log && now->getCycleCount() >= *rob_start_cycle) { fprintf(stderr, __VA_ARGS__); }}

class MemStatsManager {
private:

    ComponentTime *now;

    bool *enable_rob_timer_log;
    UInt64 *rob_start_cycle;

    static constexpr size_t kHistorySize = 16;

    // struct MemStats {
    //     std::deque<UInt64> latencies; // 過去の遅延を記録するデータ構造
    // };
    // std::unordered_map<UInt64, MemStats> m_mem_stats;

    struct MemHitSatCounter {
        SInt8 scounter;
    };
    std::unordered_map<UInt64, MemHitSatCounter> m_mem_stats;

public:
    MemStatsManager (ComponentTime *now, bool *enable_rob_timer_log, UInt64 *rob_start_cycle) {
        this->now = now;
        this->enable_rob_timer_log = enable_rob_timer_log;
        this->rob_start_cycle = rob_start_cycle;
    }

    ~MemStatsManager() {
        std::cout << "-------------------\n";
        std::cout << "Memory Latency Statistics\n";
        std::cout << "-------------------\n";
        for (const auto& mem : m_mem_stats) {
            const auto& scounter = mem.second.scounter;

            // 統計の出力
            fprintf(stderr, "PC=%08lx : Count=%d\n",
                    mem.first, scounter);
        }
    }

    void Remove (UInt64 pc) {
        m_mem_stats.erase(pc);
    }

    /*
     * Check and update saturation counter for cache hit/miss
     *
     * @param pc: PC of the instruction
     * @param latency: latency of the instruction
     * @param miss: true if the memory access of the PC is high possibility MISS, it means saturated counter.
     * @return true if the saturation counter is updated,
     */
    bool Update (UInt64 pc, UInt64 latency, bool &miss) {
        auto& stats = m_mem_stats[pc];

        bool hit = latency <= 4;
        bool updated = false;
        SInt8 THRESHOLD = 2;
        SInt8 THRESHOLD_MAX = 4;
        if (hit) {
            if (stats.scounter != -THRESHOLD_MAX) {
                if (stats.scounter == -THRESHOLD + 1) {
                    updated = true;
                }
                stats.scounter--;
            }
        } else {
            if (stats.scounter != THRESHOLD_MAX) {
                if (stats.scounter == THRESHOLD - 1) {
                    updated = true;
                }
                stats.scounter++;
            }
        }
        miss = stats.scounter >= THRESHOLD; // Saturation Counterが2以上の場合はキャッシュ・ヒット率が低と判定
        return updated;
    }



};
