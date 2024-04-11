#ifndef __VECTOR_DEPENDENCIES_H
#define __VECTOR_DEPENDENCIES_H

#include "fixed_types.h"
#include <decoder.h>

//extern "C" {
//#include <xed-reg-enum.h>
//}

class DynamicMicroOp;

class VectorDependencies {
private:
    uint64_t previous_vec_seqno;
    IntPtr previous_pc;
public:
  VectorDependencies(){
    previous_vec_seqno = 0;
    previous_pc = 0;
  }

  void setDependencies(DynamicMicroOp& microOp)
  {
    if (!microOp.getMicroOp()->isVector()) {
        previous_vec_seqno = 0;
        previous_pc = 0;
        return;
    }
    if (previous_vec_seqno + 1 == microOp.getSequenceNumber() &&
        previous_pc == microOp.getMicroOp()->getInstruction()->getAddress()) {
        microOp.addDependency(previous_vec_seqno);
    }
    previous_vec_seqno = microOp.getSequenceNumber();
    previous_pc = microOp.getMicroOp()->getInstruction()->getAddress();
  }

  void clear() {
    previous_vec_seqno = 0;
    previous_pc = 0;
  }
};

#endif /* __VECTOR_DEPENDENCIES_H */
