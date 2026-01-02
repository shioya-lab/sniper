#pragma once

#include "interval_timer.h"
#include "stats.h"

#include <unordered_map>
#include <deque>
#include <cstdint>
#include <cstdio>
#include <cstddef>
#include <functional>

#define MEMSTATS_DEBUG_PRINTF(...) { if (*enable_rob_timer_log && now->getCycleCount() >= *rob_start_cycle) { fprintf(stderr, __VA_ARGS__); }}

class MemStatsManager {
private:

    ComponentTime *now;

    bool *enable_rob_timer_log;
    UInt64 *rob_start_cycle;

    std::size_t m_max_capacity = Sim()->getCfg()->getInt("perf_model/core/rob_timer/miss_stats_size");

    // struct MemStats {
    //     std::deque<UInt64> latencies; // 過去の遅延を記録するデータ構造
    // };
    // std::unordered_map<UInt64, MemStats> m_mem_stats;

    struct MemHitSatCounter {
        UInt64 pc = 0;
        SInt8 scounter = 0;
        SInt8 rebuild_downcounter = 0;  // PCごとのリオーダリングリスト再構築用ダウンカウンタ
    };
    std::unordered_map<std::size_t, MemHitSatCounter> m_mem_stats;

    // ハッシュ関数
    std::size_t hashPC(UInt64 pc) const {
        return std::hash<UInt64>{}(pc) % m_max_capacity;
    }

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
            fprintf(stderr, "Hash=%zu (PC=%08lx) : Count=%d\n",
                    mem.first, mem.second.pc, scounter);
        }
    }

    void Remove (UInt64 pc) {
        std::size_t hash = hashPC(pc);
        m_mem_stats.erase(hash);
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
        std::size_t hash = hashPC(pc);
        auto& stats = m_mem_stats[hash];
        
        // PCをセット（まだセットされていない場合、またはハッシュ衝突で異なるPCが入っている場合は更新）
        if (stats.pc == 0 || stats.pc != pc) {
            stats.pc = pc;
        }

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

    SInt8 getSaturationCounter(UInt64 pc) {
        std::size_t hash = hashPC(pc);
        auto it = m_mem_stats.find(hash);
        if (it == m_mem_stats.end()) {
            return 0;
        }
        return it->second.scounter;
    }

    // 容量制限を変更するメソッド
    void setMaxCapacity(std::size_t max_capacity) {
        m_max_capacity = max_capacity;
        // 既存のデータをクリア（新しいハッシュ範囲に合わせるため）
        m_mem_stats.clear();
    }

    // 現在の容量を取得
    std::size_t getCurrentSize() const {
        return m_mem_stats.size();
    }

    // 最大容量を取得
    std::size_t getMaxCapacity() const {
        return m_max_capacity;
    }

    // PCごとのリオーダリングリスト再構築用のダウンカウンタを初期化
    void setRebuildDowncounter(UInt64 pc, SInt8 value) {
        std::size_t hash = hashPC(pc);
        auto& stats = m_mem_stats[hash];
        stats.rebuild_downcounter = value;
    }


    // すべてのPCのリオーダリングリスト再構築用のダウンカウンタを初期化
    void clearAllRebuildDowncounters() {
        for (auto& pair : m_mem_stats) {
            auto& stats = pair.second;
            stats.rebuild_downcounter = 0;
        }
    }

    // PCごとのダウンカウンタを取得
    SInt8 getRebuildDowncounter(UInt64 pc) {
        std::size_t hash = hashPC(pc);
        auto it = m_mem_stats.find(hash);
        if (it == m_mem_stats.end()) {
            return 0;
        }
        return it->second.rebuild_downcounter;
    }

    // すべてのPCのダウンカウンタ（値>0）をデクリメントし、0になったPCがあるかどうかを返す
    std::pair<bool, UInt64> decrementAllRebuildDowncounters() {
        bool any_reached_zero = false;
        UInt64 pc = 0;
        for (auto& pair : m_mem_stats) {
            auto& stats = pair.second;
            if (stats.rebuild_downcounter > 0) {
                stats.rebuild_downcounter--;
                if (stats.rebuild_downcounter == 0) {
                    any_reached_zero = true;
                    pc = stats.pc;
                }
            }
        }
        return std::make_pair(any_reached_zero, pc);
    }

};
