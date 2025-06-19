#ifndef BRANCH_HASHED_PERCEPTRON_H
#define BRANCH_HASHED_PERCEPTRON_H

#include <array>
#include <cstdint>
#include <tuple>
#include <vector>

#include "branch_predictor.h"
#include "folded_shift_register.h"
#include "config.hpp"
#include "simulator.h"
#include "modules.h"
#include "msl/bits.h"
#include "msl/fwcounter.h"

class hashed_perceptron //  : champsim::modules::branch_predictor
{
    using bits = champsim::data::bits;                 // saves some typing
    constexpr static std::size_t NTABLES = 16;         // this many tables
    constexpr static bits MAX_HIST{232};                // maximum history length
    constexpr static bits MIN_HIST{3};                  // minimum history length (for table 1; table 0 is biases)
    constexpr static std::size_t TABLE_SIZE = 1 << 12; // 12-bit indices for the tables
    constexpr static bits TABLE_INDEX_BITS{champsim::msl::lg2(TABLE_SIZE)};
    constexpr static int THRESHOLD = 1;

    constexpr static std::array<bits, NTABLES> history_lengths = {
        bits{}, MIN_HIST, bits{4}, bits{6}, bits{8}, bits{10}, bits{14}, bits{19},
        bits{26}, bits{36}, bits{49}, bits{67}, bits{91}, bits{125}, bits{170}, MAX_HIST}; // geometric global history lengths

    // tables of 8-bit weights
    std::array<std::array<champsim::msl::sfwcounter<8>, TABLE_SIZE>, NTABLES> tables{};

    // words that store the global history
    using history_type = folded_shift_register<TABLE_INDEX_BITS>;
    std::array<history_type, NTABLES> ghist_words = []()
    {
        decltype(ghist_words) retval;
        std::transform(std::cbegin(history_lengths), std::cend(history_lengths), std::begin(retval), [](const auto len)
                       { return history_type{len}; });
        return retval;
    }();

    int theta = 10;
    int tc = 0; // counter for threshold setting algorithm

    struct perceptron_result
    {
        std::array<uint64_t, std::tuple_size_v<decltype(history_lengths)>> indices = {}; // remember the indices into the tables from prediction to update
        int yout = 0;                                                                    // perceptron sum
    };

    perceptron_result last_result{};

    std::unordered_map<IntPtr, uint64_t> m_incorrect_per_ip;
    UInt64 m_debug_pc;
    FILE *m_debug_fp;

public:
   // using branch_predictor::branch_predictor;
    bool predict_branch(champsim::address pc);
    void last_branch_result(champsim::address pc, champsim::address branch_target, bool taken, uint8_t branch_type);
    void adjust_threshold(bool correct);

    hashed_perceptron()
       : m_debug_pc(strtol(Sim()->getCfg()->getStringArray("log/branch_debug_pc", 0).c_str(), nullptr, 16))
    {
        m_debug_fp = fopen("hashed_perceptron_debug.txt", "w");
        if (!m_debug_fp) {
            std::cerr << "Error: Could not open debug file for writing.\n";
        }
    }

    virtual ~hashed_perceptron ()
    {
        std::ofstream out("branch_mispredicts.csv");
        if (!out.is_open())
        {
            std::cerr << "Error: Could not open CSV file for writing.\n";
            return;
        }

        out << "ip,num_incorrect\n";

        for (const auto& [ip, count] : m_incorrect_per_ip)
        {
            out << "0x" << std::hex << ip << "," << std::dec << count << "\n";
        }

        out.close();

    }

};

#endif
