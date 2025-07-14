#include <unordered_map>
#include <fstream>
#include <iomanip> // for hex formatting

#include "branch_predictor.h"
#include "hashed_perceptron.h"

class HashedPerceptron final : public BranchPredictor
{
public:
   HashedPerceptron(String name, core_id_t core_id) : BranchPredictor(name, core_id)
   {
   }

   virtual ~HashedPerceptron ()
   {
   }

   virtual bool predict(bool indirect, IntPtr ip, IntPtr target) override
   {
      return m_predictor.predict_branch(champsim::address { ip });
   }

   virtual void update(bool predicted, bool actual, bool indirect, IntPtr ip, IntPtr target) override
   {
      m_predictor.last_branch_result (champsim::address { ip }, champsim::address { target }, actual, indirect);
      updateCounters (predicted, actual);
   }

  private:
   hashed_perceptron m_predictor;
};
