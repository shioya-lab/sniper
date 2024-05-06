#include "branch_predictor.h"
#include "pentium_m_indirect_branch_target_buffer.h"
#include "cbp2016/predictor.h"

class TageScL64kb final : public BranchPredictor
{
public:
    TageScL64kb(String name, core_id_t core_id) : BranchPredictor(name, core_id)
    {
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
    }

private:
    PREDICTOR m_predictor;
    PentiumMIndirectBranchTargetBuffer m_ibtb;
};
