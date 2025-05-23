#include "rob_contention.h"
#include "rob_timer.h"
#include <cstdio>

void RobTimer::manageInstructionReserve (RobEntry *entry)
{
  LOG_ASSERT_ERROR(!entry->uop->isReserveInst() &&
                       !entry->uop->isStrongPriorityInst(),
                   "Priority must not allocate before execution");
  if (m_vec_reserve_policy == VecReserveDynamic) {
    // 優先度に応じてベクトル命令を予約に回す方針
    manageInstructionReserveVecPriority(entry);
  } else if (m_vec_reserve_policy == VecReserveParOOO) {
    manageInstructionParOOO(entry);
  } else if (m_vec_reserve_policy == VecReserveAlways) {
    // 常にベクトル命令を予約に回す方針
    manageInstructionReserveVecAll(entry);
  }
}

//
// 自分のエントリが優先命令さ削除対象キューに入っていれば、削除する
//
void RobTimer::RemovePriorityQueue (RobEntry *entry)
{
  UInt64 entry_pc = entry->uop->getMicroOp()->getInstruction()->getAddress();

  auto priority_remove_queue_it = m_priority_manager->getPriorityRemoveQueue();
  auto remove_it = std::find(
      priority_remove_queue_it->begin(), priority_remove_queue_it->end(),
      entry_pc);
  if (remove_it != priority_remove_queue_it->end()) {
    m_priority_manager->removePriority(entry_pc);
    priority_remove_queue_it->erase(remove_it);
    // fprintf (stderr, "size of priority_remove_queue: %ld\n", priority_remove_queue_it->size());
    fprintf (stderr, "%ld: Priority remove propagation phase: from PC=%08lx\n",
                     now.getCycleCount(),
                     entry_pc);

    for (size_t idx = 0; idx < entry->uop->getDependenciesLength(); ++idx) {
      RobEntry *waiting_entry =
          this->findEntryBySequenceNumber(entry->uop->getDependency(idx));

      bool is_waiting_entry_vector_dest_reg =
          waiting_entry->uop->getMicroOp()->getDestinationRegistersLength() &&
          Sim()->getDecoder()->is_reg_vector(
              waiting_entry->uop->getMicroOp()->getDestinationRegister(0));
      if (is_waiting_entry_vector_dest_reg) {
        UInt64 wait_entry_pc =
            waiting_entry->uop->getMicroOp()->getInstruction()->getAddress();
        auto same_it = std::find(
          priority_remove_queue_it->begin(), priority_remove_queue_it->end(),
          wait_entry_pc);
        if (m_priority_manager->getPriority(wait_entry_pc) != PriorityManager::inst_priority_t::High) {
          continue;
        }
        if (same_it == priority_remove_queue_it->end()) {
          priority_remove_queue_it->push_back(wait_entry_pc);
          fprintf(stderr, "%ld: idx=%ld, Priority Remove Candidate: PC=%08lx\n",
                          now.getCycleCount(), idx, wait_entry_pc);
        }
      }
    }
  }
}

/*
 * 優先度の伝搬:
 * 自分が即時割り当ての命令であれば、自分が依存している命令も即時割り当ての命令でなければならない
 */
void RobTimer::PropagateHighPriorityBackward (const MicroOp* uop)
{

  // ソース・オペランドを生成する命令をm_vec_histから探索して、優先度をHighにする。
  for (size_t idx = 0; idx < uop->getSourceRegistersLength(); ++idx) {
    dl::Decoder::decoder_reg src_reg = uop->getSourceRegister(idx);
    if (Sim()->getDecoder()->is_reg_vector(src_reg)) {
      // m_vec_histに格納されているもっとも最近のPCを探す
      // fprintf (stderr, "  src_reg=%d, ", src_reg);
      // for (auto it = m_vec_reg_hist.rbegin(); it != m_vec_reg_hist.rend(); ++it) {
      //   fprintf (stderr, "  dest_reg=%d, PC=%08lx\n", it->dest_reg, it->pc);
      // }
      // fprintf (stderr, "\n");
      for (auto it = m_vec_reg_hist.rbegin(); it != m_vec_reg_hist.rend(); ++it) {
        if (it->dest_reg == src_reg) {
          UInt64 wait_entry_pc = it->pc;
          if (m_priority_manager->getPriority(wait_entry_pc) == PriorityManager::inst_priority_t::High) {
            break;
          }
          if (std::find(m_high_inst_candidate.begin(), m_high_inst_candidate.end(), wait_entry_pc) == m_high_inst_candidate.end()) {
            if (m_high_inst_candidate.size() >= 8) {
              m_high_inst_candidate.pop_front();
            }
            m_high_inst_candidate.push_back(wait_entry_pc);
            fprintf(stderr, "%ld: %s idx=%ld(%s), Priority High Candidate: PC=%08lx\n",
                            now.getCycleCount(), uop->getInstruction()->getDisassembly().c_str(), idx, Sim()->getDecoder()->reg_name(src_reg), wait_entry_pc);
            break;
          }
          break;
        }
      }
    }
  }

  // UInt64 entry_pc = entry->uop->getMicroOp()->getInstruction()->getAddress();
  // // 優先度の伝搬:
  // // 自分が即時割り当ての命令であれば、自分が依存している命令も即時割り当ての命令でなければならない
  // for (size_t idx = 0; idx < entry->uop->getDependenciesLength(); ++idx) {
  //   RobEntry *waiting_entry =
  //       this->findEntryBySequenceNumber(entry->uop->getDependency(idx));

  //   bool is_waiting_entry_vector_dest_reg =
  //       waiting_entry->uop->getMicroOp()->getDestinationRegistersLength() &&
  //       Sim()->getDecoder()->is_reg_vector(
  //           waiting_entry->uop->getMicroOp()->getDestinationRegister(0));
  //   if (is_waiting_entry_vector_dest_reg) {
  //     UInt64 wait_entry_pc =
  //         waiting_entry->uop->getMicroOp()->getInstruction()->getAddress();
  //     if (m_priority_manager->getPriority(wait_entry_pc) != PriorityManager::inst_priority_t::High) {
  //       m_priority_manager->setPriority(
  //           wait_entry_pc, PriorityManager::inst_priority_t::High);

  //       fprintf(stderr,
  //           "%ld: Priority backpropagation: Strong propagated from "
  //           "PC=%08lx to PC=%08lx\n",
  //           now.getCycleCount(),
  //           entry_pc,
  //           waiting_entry->uop->getMicroOp()->getInstruction()->getAddress());
  //     }
  //   }
  // }
}


/*
 * 優先度情報を順方向に伝搬させる
 * 低優先度の命令に依存している or 高優先度の命令に依存している
 * --> 自分も低優先度の命令になる
 */
void RobTimer::PropagatePriorityFromForward (RobEntry *entry)
{
  for (size_t idx = 0; idx < entry->uop->getDependenciesLength(); ++idx) {
    RobEntry *waiting_entry =
        this->findEntryBySequenceNumber(entry->uop->getDependency(idx));

    bool is_waiting_entry_vector_dest_reg =
        waiting_entry->uop->getMicroOp()->getDestinationRegistersLength() &&
        Sim()->getDecoder()->is_reg_vector(
            waiting_entry->uop->getMicroOp()->getDestinationRegister(0));
    if (is_waiting_entry_vector_dest_reg &&
        (waiting_entry->uop
             ->isReserveInst() || // 低優先度の命令に依存する命令はLPIQに入れる
         waiting_entry->uop
             ->isStrongPriorityInst())) { // レイテンシが長いであろう超高優先度命令に依存する命令はLPIQに入れる
      entry->uop->setReserveInst();
      // fprintf(stderr, "Set Reserve Priority PC=%08lx, uop_idx=%ld %s\n",
      //                 entry->uop->getMicroOp()->getInstruction()->getAddress(),
      //                 entry->uop->getSequenceNumber(),
      //                 entry->uop->getMicroOp()->toShortString().c_str());
      break;
    }
  }
}

/*
 * 優先度の伝搬などの制御を行う
*/
void RobTimer::manageInstructionReserveVecPriority(RobEntry *entry)
{
  UInt64 entry_pc = entry->uop->getMicroOp()->getInstruction()->getAddress();

  // 自分のエントリが優先命令さ削除対象キューに入っていれば、削除する
  RemovePriorityQueue(entry);

  PriorityManager::inst_priority_t priority = m_priority_manager->getPriority(entry_pc);
  if (priority == PriorityManager::inst_priority_t::High) {
    entry->uop->setStrongPriorityInst();
    // PropagateHighPriorityBackward(entry->uop->getMicroOp());
  } else if (priority == PriorityManager::inst_priority_t::Reserve) {
    entry->uop->setReserveInst();
  } else {
    PropagatePriorityFromForward(entry);
  }
}

void RobTimer::manageInstructionReserveVecAll(RobEntry *entry)
{
  if (entry->uop->getMicroOp()->getDestinationRegistersLength() &&
      Sim()->getDecoder()->is_reg_vector(
          entry->uop->getMicroOp()->getDestinationRegister(0))) {
    entry->uop->setReserveInst();
  }
}

/*
 * ParOOOの場合の優先度の伝搬などの制御を行う
*/
void RobTimer::manageInstructionParOOO(RobEntry *entry)
{
  UInt64 entry_pc = entry->uop->getMicroOp()->getInstruction()->getAddress();

  // 自分のエントリが優先命令さ削除対象キューに入っていれば、削除する
  RemovePriorityQueue(entry);

  // fprintf (stderr, "manageIsntructionPAROOO() PC=%08lx\n", entry_pc);
  // m_high_inst_candidateに自分のPCが含まれていれば、優先命令化する。
  if (std::find(m_high_inst_candidate.begin(), m_high_inst_candidate.end(), entry_pc) != m_high_inst_candidate.end()) {
    m_priority_manager->setPriority(entry_pc, PriorityManager::inst_priority_t::High);
    m_priority_manager->AddHighInst(entry_pc);
    entry->uop->setStrongPriorityInst();
    fprintf (stderr, "%ld: Priority backpropagation: PC=%08lx %s\n",
                     now.getCycleCount(), entry_pc, entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
    m_high_inst_candidate.erase(std::remove(m_high_inst_candidate.begin(), m_high_inst_candidate.end(), entry_pc), m_high_inst_candidate.end());
    // PropagateHighPriorityBackward(entry->uop->getMicroOp());
  }

  PriorityManager::inst_priority_t priority = m_priority_manager->getPriority(entry_pc);
  if (priority == PriorityManager::inst_priority_t::High) {
    // 優先度の伝搬:
    // 自分が即時割り当ての命令であれば、自分が依存している命令も即時割り当ての命令でなければならない
    entry->uop->setStrongPriorityInst();
    // PropagateHighPriorityBackward(entry);
  } else if (priority == PriorityManager::inst_priority_t::Reserve) {
    entry->uop->setReserveInst();
  } else {
    // Reserveの伝搬
    // 自分に依存している命令がReserveならば、自分もReserveになる
    PropagatePriorityFromForward(entry);
  }
}


/*
 * entryをベースに、優先度の情報を伝搬させる：
 * result: Add => 自分が依存する命令も優先度を上げる
 * result: Remove => 自分に依存する命令の優先度を下げる
*/
void RobTimer::propagatePriInst (RobEntry *entry, pri_upd_result_t result)
{
  UInt64 entry_pc = entry->uop->getMicroOp()->getInstruction()->getAddress();

  // 優先度の伝搬:
  // 自分が即時割り当ての命令であれば、自分が依存している命令も即時割り当ての命令でなければならない
  fprintf (stderr, "propagatePriInst() ");
  for (size_t idx = 0; idx < entry->uop->getInitialDependenciesLength(); ++idx) {
    fprintf (stderr, " %ld", idx);
    RobEntry *waiting_entry =
        this->findEntryBySequenceNumber(entry->uop->getInitialDependency(idx));

    bool is_waiting_entry_vector_dest_reg =
        waiting_entry->uop->getMicroOp()->getDestinationRegistersLength() &&
        Sim()->getDecoder()->is_reg_vector(
            waiting_entry->uop->getMicroOp()->getDestinationRegister(0));

    if (is_waiting_entry_vector_dest_reg) {
      UInt64 wait_entry_pc =
          waiting_entry->uop->getMicroOp()->getInstruction()->getAddress();
      if (result == pri_upd_result_t::Added &&
          m_priority_manager->getPriority(wait_entry_pc) != PriorityManager::inst_priority_t::High) {
        m_priority_manager->setPriority(
            wait_entry_pc, PriorityManager::inst_priority_t::High);
        fprintf (stderr,
            "%ld: Priority backpropagation: Strong propagated from "
            "PC=%08lx to PC=%08lx\n",
            now.getCycleCount(),
            entry_pc,
            waiting_entry->uop->getMicroOp()->getInstruction()->getAddress());
        propagatePriInst (waiting_entry, result);
      } else if (result == pri_upd_result_t::Removed &&
                 m_priority_manager->getPriority(wait_entry_pc) == PriorityManager::inst_priority_t::High) {
        m_priority_manager->setPriority(
            wait_entry_pc, PriorityManager::inst_priority_t::High);
        fprintf (stderr,
            "%ld: Priority backpropagation: Strong propagated from "
            "PC=%08lx to PC=%08lx\n",
            now.getCycleCount(),
            entry_pc,
            waiting_entry->uop->getMicroOp()->getInstruction()->getAddress());
        propagatePriInst (waiting_entry, result);
      }
    }
  }
  fprintf (stderr, "\n");
}
