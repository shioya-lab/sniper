#ifndef PENTIUM_M_BRANCH_PREDICTOR_H
#define PENTIUM_M_BRANCH_PREDICTOR_H

#include "branch_predictor.h"
#include "branch_predictor_return_value.h"
#include "pentium_m_global_predictor.h"
#include "pentium_m_branch_target_buffer.h"
#include "pentium_m_bimodal_table.h"
#include "pentium_m_loop_branch_predictor.h"
#include "pentium_m_indirect_branch_target_buffer.h"

#include <vector>

class PentiumMBranchPredictor : public BranchPredictor
{
public:
   PentiumMBranchPredictor(String name, core_id_t core_id);
   ~PentiumMBranchPredictor();

   bool predict(bool indirect, IntPtr ip, IntPtr target);

   void update(bool predicted, bool actual, bool indirect, IntPtr ip, IntPtr target);

private:

   uint8_t latest_pred_type = 0; // 1: Global, 2: LPB, 3: Bimodal, 4: Indirect BTB
   bool applied_ibib;
   
   void update_pir(bool actual, IntPtr ip, IntPtr target, BranchPredictorReturnValue::BranchType branch_type);
   IntPtr hash_function(IntPtr ip, IntPtr pir);

   PentiumMGlobalPredictor m_global_predictor;
   PentiumMBranchTargetBuffer m_btb;
   PentiumMBimodalTable m_bimodal_table;
   PentiumMLoopBranchPredictor m_lpb;
   PentiumMIndirectBranchTargetBuffer ibtb;


   IntPtr m_pir;

   bool m_last_gp_hit;
   bool m_last_bm_pred;
   bool m_last_lpb_hit;

   UInt64 m_debug_pc;
   FILE *m_debug_fp;

   std::unordered_map<IntPtr, uint64_t> m_incorrect_per_ip;
   // <<ip, predictor_type>, <count, incorrect_count>>
   std::map<std::pair<IntPtr, uint8_t>, std::pair<uint64_t, uint64_t>> m_predictor_per_ip;  // 1: Global, 2: LPB, 3: Bimodal, 4: Indirect BTB

};

#endif
