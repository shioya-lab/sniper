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

    // UInt64 Update (UInt64 pc, UInt64 latency) {
    //     auto& stats = m_mem_stats[pc];

    //     // 遅延値を記録
    //     if (stats.latencies.size() >= kHistorySize) {
    //         stats.latencies.pop_front(); // 古い値を削除
    //     }
    //     stats.latencies.push_back(latency); // 新しい値を追加

    //     // 最大値と最小値を除去したうえで平均値を計算
    //     UInt64 total_latency = 0;
    //     // UInt64 min_latency = std::numeric_limits<UInt64>::max();
    //     // UInt64 max_latency = 0;
    //     for (const auto& value : stats.latencies) {
    //         total_latency += value;
    //         // min_latency = std::min(min_latency, value);
    //         // max_latency = std::max(max_latency, value);
    //     }
    //     // float average_latency = stats.latencies.size() < 2 ? static_cast <UInt64>(total_latency) / stats.latencies.size() :
    //     //     static_cast<float>(total_latency - min_latency - max_latency) / (stats.latencies.size() - 2);
    //     float average_latency = static_cast <UInt64>(total_latency) / stats.latencies.size();
        
    //     // // 平均値を計算
    //     // UInt64 total_latency = 0;
    //     // for (const auto& value : stats.latencies) {
    //     //     total_latency += value;
    //     // }
    //     // float average_latency = static_cast<float>(total_latency) / stats.latencies.size();

    //     // MEMSTATS_DEBUG_PRINTF("Updated Mem Status: PC=%08lx, Count=%ld, Average=%f\n",
    //     //                  pc, stats.latencies.size(), average_latency);

    //     return static_cast <UInt64>(average_latency);
    // }

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

        // if (pc == 0x000149ac) {
        //     fprintf(stderr, "PC=%08lx : Latency=%ld, Count=%d, Miss=%d", pc, latency, stats.scounter, miss);
        //     if (updated) {
        //         fprintf(stderr, " : Updated\n");
        //     } else {
        //         fprintf(stderr, "\n");
        //     }
        // }
        return updated;  
    }

    // void dumpMemStats(UInt64 pc) {
    //     auto it = m_mem_stats.find(pc);
    //     if (it != m_mem_stats.end()) {
    //         const auto& latencies = it->second.latencies;
    //         fprintf(stderr, "PC=%08lx : Count=%lu : ", pc, latencies.size());
    //         for (const auto& latency : latencies) {
    //             fprintf(stderr, ", %lu", latency);
    //         }
    //         fprintf (stderr, "\n");
    //     }
    // }    

};
