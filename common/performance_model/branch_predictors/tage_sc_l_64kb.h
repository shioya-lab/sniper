#include <unordered_map>
#include <fstream>
#include <iomanip> // for hex formatting
#include "config.hpp"
#include "simulator.h"

#include "branch_predictor.h"
#include "pentium_m_indirect_branch_target_buffer.h"
#include "cbp2016/predictor.h"

class TageScL64kb final : public BranchPredictor
{
    UInt64 m_debug_pc;
    FILE *m_debug_fp;

public:
    TageScL64kb(String name, core_id_t core_id) : BranchPredictor(name, core_id)
       , m_debug_pc(strtol(Sim()->getCfg()->getStringArray("log/branch_debug_pc", 0).c_str(), nullptr, 16))
    {
        m_debug_fp = fopen("tage_sc_l_64kb_debug.txt", "w");
        if (!m_debug_fp) {
            std::cerr << "Error: Could not open debug file for writing.\n";
        }
    }

    virtual ~TageScL64kb ()
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

    virtual bool predict(bool indirect, IntPtr ip, IntPtr target) override
    {
        return indirect ? m_ibtb.predict(indirect, ip, target) :
                          m_predictor.GetPrediction(ip);
    }

    virtual void update(bool predicted, bool actual, bool indirect, IntPtr ip, IntPtr target) override
    {
        auto optype = indirect ? OPTYPE_JMP_INDIRECT_COND :
                                 OPTYPE_JMP_DIRECT_COND;

        updateCounters(predicted, actual);
        m_predictor.UpdatePredictor(ip, optype, actual, predicted, target);

        if (indirect)
        {
            m_ibtb.update(predicted, actual, indirect, ip, target);
        }

        if (predicted != actual)
        {
            ++m_incorrect_per_ip[ip];
        }
        if (m_debug_pc == ip) {
            fprintf(m_debug_fp, "TAGE: IP=0x%lx, Target=0x%lx, Predicted=%d, Actual=%d, Result=%d\n", ip, target, predicted, actual, predicted == actual);
        }

    }

private:
    PREDICTOR m_predictor;
    PentiumMIndirectBranchTargetBuffer m_ibtb;

    std::unordered_map<IntPtr, uint64_t> m_incorrect_per_ip;
};
