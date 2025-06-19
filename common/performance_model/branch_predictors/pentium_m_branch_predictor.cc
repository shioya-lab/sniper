#include <unordered_map>
#include <fstream>
#include <iomanip> // for hex formatting

#include "config.hpp"
#include "simulator.h"
#include "pentium_m_branch_predictor.h"

PentiumMBranchPredictor::PentiumMBranchPredictor(String name, core_id_t core_id)
   : BranchPredictor(name, core_id)
   , m_pir(0)
   , m_last_gp_hit(false)
   , m_last_lpb_hit(false)
   , m_debug_pc(strtol(Sim()->getCfg()->getStringArray("log/branch_debug_pc", core_id).c_str(), nullptr, 16))
{
   if ((m_debug_fp = fopen("branch_predictor_debug.txt", "w")) == nullptr) {  
       std::cerr << "Error: Could not open branch predictor debug file.\n";
   }
}

PentiumMBranchPredictor::~PentiumMBranchPredictor()
{
   std::ofstream out("branch_mispredicts.csv");

   if (!out.is_open()) {
       std::cerr << "Error: Could not open CSV file for writing.\n";
       return;
   }

   out << "ip,num_incorrect\n";

   for (const auto& [ip, count] : m_incorrect_per_ip)
   {
       out << "0x" << std::hex << ip << "," << std::dec << count << "\n";
   }
   out.close();

   std::ofstream out2("branch_predictor_per_ip.csv");
   if (!out2.is_open()) {
       std::cerr << "Error: Could not open CSV file for writing.\n";
       return;
   }
   out2 << "ip,predictor,count,miss\n";
   for (const auto& [key, count] : m_predictor_per_ip)
   {
       IntPtr ip = key.first;
       uint8_t predictor = key.second;
       out2 << "0x" << std::hex << ip << "," << static_cast<int>(predictor) << "," << std::dec << count.first << ", " << count.second << "\n";
   }
   out2.close();
}

bool PentiumMBranchPredictor::predict(bool indirect, IntPtr ip, IntPtr target)
{
   BranchPredictorReturnValue global_pred_out = m_global_predictor.lookup(ip, target, m_pir);
   BranchPredictorReturnValue btb_out = m_btb.lookup(ip, target);
   BranchPredictorReturnValue lpb_out = m_lpb.lookup(ip, target);
   
   bool bimodal_out = m_bimodal_table.predict(indirect, ip, target);

   m_last_gp_hit = global_pred_out.hit;
   m_last_bm_pred = bimodal_out;
   m_last_lpb_hit = lpb_out.hit & btb_out.hit;

   // Outcome prediction logic
   bool result;// = ibtb.predict(ip,target);
   applied_ibib = false; // Indirect BTB prediction is not applied by default
   if (global_pred_out.hit )
   {
      result = global_pred_out.prediction;
      m_predictor_per_ip[{ip, 1}].first ++; // Global predictor hit
      latest_pred_type = 1;
   }
   else if (lpb_out.hit & btb_out.hit)
   {
      result = lpb_out.prediction;
      m_predictor_per_ip[{ip, 2}].first ++; // Loop predictor hit
      latest_pred_type = 2;
   }
   else
   {
      result = bimodal_out;
      m_predictor_per_ip[{ip, 3}].first ++; // Bimodal predictor hit
      latest_pred_type = 3;
   }
   if (result == true)
   {
      result = ibtb.predict(indirect,ip,target);
      m_predictor_per_ip[{ip, 4}].first ++; // Indirect BTB hit
      applied_ibib = result == false;
   }
   // TODO FIXME: Failed matches against the target address should force a branch or fetch miss

   return result;
}

void PentiumMBranchPredictor::update(bool predicted, bool actual, bool indirect, IntPtr ip, IntPtr target)
{
   updateCounters(predicted, actual);
   ibtb.update(predicted,actual,indirect,ip,target);
   m_btb.update(predicted, actual, indirect, ip, target);
   m_lpb.update(predicted, actual, ip, target);
   if (!m_last_gp_hit && !m_last_lpb_hit) // Update bimodal predictor only when global and loop predictors missed
      m_bimodal_table.update(predicted, actual, indirect, ip, target);
   bool lpb_or_bm_hit = m_last_lpb_hit || m_last_bm_pred == actual; // Global should only allocate when no loop predictor hit and bimodal was wrong
   if (m_last_gp_hit)
   {
      if (predicted != actual && lpb_or_bm_hit) // Evict from global when mispredict and loop or bimodal hit
         m_global_predictor.evict(ip, m_pir);
      else
         m_global_predictor.update(predicted, actual, indirect, ip, target, m_pir);
   }
   else if (predicted != actual && (m_last_gp_hit || !lpb_or_bm_hit)) // Update on mispredict, but don't allocate when loop or bimodal hit
      m_global_predictor.update(predicted, actual, indirect, ip, target, m_pir);
   // TODO FIXME: Properly propagate the branch type information from the decoder (IndirectBranch information)
   update_pir(actual, ip, target, BranchPredictorReturnValue::ConditionalBranch);

   if (predicted != actual) {
      ++m_incorrect_per_ip[ip];
      m_predictor_per_ip[{ip, latest_pred_type}].second ++; // Increment the count for the predictor used
   }

   if (m_debug_pc == ip) {
      fprintf(m_debug_fp, "%3s%1s: IP=0x%lx, Target=0x%lx, Predicted=%d, Actual=%d, Result=%d\n", 
               latest_pred_type == 1 ? "GP " : latest_pred_type == 2 ? "LPB" : "BM ",
               applied_ibib ? "I" : " ",
               ip, target, predicted, actual, predicted == actual);
   }
}

void PentiumMBranchPredictor::update_pir(bool actual, IntPtr ip, IntPtr target, BranchPredictorReturnValue::BranchType branch_type)
{
   IntPtr rhs;

   if ((branch_type == BranchPredictorReturnValue::ConditionalBranch) & actual)
   {
      rhs = ip >> 4;
   }
   else if (branch_type == BranchPredictorReturnValue::IndirectBranch)
   {
      rhs = (ip >> 4) | target;
   }
   else
   {
      // No PIR update
      return;
   }

   m_pir = ((m_pir << 2) ^ rhs) & 0x7fff;
}
