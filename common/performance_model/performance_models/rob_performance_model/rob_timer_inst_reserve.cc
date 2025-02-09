#include "rob_timer.h"

void RobTimer::manageInstructionReserve (RobEntry *entry)
{
  LOG_ASSERT_ERROR(!entry->uop->isReserveInst() &&
                       !entry->uop->isStrongPriorityInst(),
                   "Priority must not allocate before execution");
  if (m_vec_reserve_policy == VecReserveDynamic) {
    // 優先度に応じてベクトル命令を予約に回す方針
    manageInstructionReserveVecPriority(entry);
  } else if (m_vec_reserve_policy == VecReserveAlways) {
    // 常にベクトル命令を予約に回す方針
    manageInstructionReserveVecAll(entry);
  }
}


void RobTimer::manageInstructionReserveVecPriority(RobEntry *entry)
{
  auto priority_remove_queue_it = m_priority_manager->getPriorityRemoveQueue();
  auto remove_it = std::find(
      priority_remove_queue_it->begin(), priority_remove_queue_it->end(),
      entry->uop->getMicroOp()->getInstruction()->getAddress());
  if (remove_it != priority_remove_queue_it->end()) {
    m_priority_manager->removePriority(
        entry->uop->getMicroOp()->getInstruction()->getAddress());
    priority_remove_queue_it->erase(remove_it);
    ROB_DEBUG_PRINTF("%ld: Priority remove propagation phase: PC=%08lx\n",
                     now.getCycleCount(),
                     entry->uop->getMicroOp()->getInstruction()->getAddress());

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
        priority_remove_queue_it->push_back(wait_entry_pc);
        ROB_DEBUG_PRINTF("Priority Remove Candidate: PC=%08lx\n",
                         wait_entry_pc);
      }
    }
  }

  PriorityManager::inst_priority_t priority = m_priority_manager->getPriority(
      entry->uop->getMicroOp()->getInstruction()->getAddress());

  if (priority == PriorityManager::inst_priority_t::High) {
    entry->uop->setStrongPriorityInst();

    // fprintf (stderr, "simulate(): pc=%08lx Priority instruction\n",
    //          entry->uop->getMicroOp()->getInstruction()->getAddress());

    // if (entry->uop->getMicroOp()->getInstruction()->getAddress() ==
    // 0x149ac) {
    //    printRob();
    // }

    // 優先度の伝搬:
    // 自分が即時割り当ての命令であれば、自分が依存している命令も即時割り当ての命令でなければならない
    for (size_t idx = 0; idx < entry->uop->getDependenciesLength(); ++idx) {
      RobEntry *waiting_entry =
          this->findEntryBySequenceNumber(entry->uop->getDependency(idx));

      // fprintf (stderr, "  dependency(%ld) = %ld\n", idx,
      // entry->uop->getDependency(idx));

      bool is_waiting_entry_vector_dest_reg =
          waiting_entry->uop->getMicroOp()->getDestinationRegistersLength() &&
          Sim()->getDecoder()->is_reg_vector(
              waiting_entry->uop->getMicroOp()->getDestinationRegister(0));
      if (is_waiting_entry_vector_dest_reg) {
        UInt64 wait_entry_pc =
            waiting_entry->uop->getMicroOp()->getInstruction()->getAddress();
        if (m_priority_manager->getPriority(wait_entry_pc) !=
            PriorityManager::inst_priority_t::High) {
          m_priority_manager->setPriority(
              wait_entry_pc, PriorityManager::inst_priority_t::High);

          ROB_DEBUG_PRINTF(
              "%ld: Priority backpropagation: Strong propagated from "
              "PC=%08lx to PC=%08lx\n",
              now.getCycleCount(),
              entry->uop->getMicroOp()->getInstruction()->getAddress(),
              waiting_entry->uop->getMicroOp()->getInstruction()->getAddress());
        }
      }
    }

  } else if (priority == PriorityManager::inst_priority_t::Reserve) {
    entry->uop->setReserveInst();
  } else {
    // 低優先度の命令に依存している or 高優先度の命令に依存している
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
        ROB_DEBUG_PRINTF("Set Reserve Priority uop_idx=%ld %s\n",
                         entry->uop->getSequenceNumber(),
                         entry->uop->getMicroOp()->toShortString().c_str());
        break;
      }
    }
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
