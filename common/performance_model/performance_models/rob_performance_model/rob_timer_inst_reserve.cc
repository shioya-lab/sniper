#include "rob_contention.h"
#include "rob_timer.h"
#include <cstdio>

void RobTimer::manageInstructionReserve (RobEntry *entry)
{
  LOG_ASSERT_ERROR(!entry->uop->isReserveInst() &&
                       !entry->uop->isStrongPriorityInst(),
                   "Priority must not allocate before execution");
  if (m_vec_reserve_policy == VecReserveParOOO) {
    manageInstructionParOOO(entry);
  } else if (m_vec_reserve_policy == VecReserveStatic) {
    manageInstructionStatic (entry);
  } else if (m_vec_reserve_policy == VecReserveSimple) {
    // ベクトルロードのみをリオーダリング対象とする簡潔な方法
    manageInstructionSimple(entry);
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
  if (remove_it == priority_remove_queue_it->end()) {
    return;
  }
  m_priority_manager->removePriority(entry_pc);
  priority_remove_queue_it->erase(remove_it);
  // fprintf (stderr, "size of priority_remove_queue: %ld\n", priority_remove_queue_it->size());
  fprintf (stderr, "%ld: Priority remove propagation phase: from PC=%08lx\n",
                   now.getCycleCount(),
                   entry_pc);

  // Backpropagate the removing of the priority of the instruction
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


/*
 * 優先度の伝搬:
 * 自分が即時割り当ての命令であれば、自分が依存している命令も即時割り当ての命令でなければならない
 */
void RobTimer::PropagateHighPriorityBackward (const MicroOp* uop)
{
  // size_t reg_idx = 0;
  // for (auto it = m_vect_dest_reg_table.rbegin(); it != m_vect_dest_reg_table.rend(); ++it, ++reg_idx) {
  //   fprintf (stderr, "m_vect_dest_reg_table[%ld]: PC=%08lx, dest_reg=%d\n",
  //     reg_idx, it->pc, it->dest_reg);
  // }
  auto uop_pc = uop->getInstruction()->getAddress();
  // ソース・オペランドを生成する命令をm_vec_histから探索して、優先度をHighにする。
  for (size_t idx = 0; idx < uop->getSourceRegistersLength(); ++idx) {
    dl::Decoder::decoder_reg src_reg = uop->getSourceRegister(idx);
    // fprintf (stderr, " src_reg[%ld] = %d\n", idx, src_reg);
    if (Sim()->getDecoder()->is_reg_vector(src_reg)) {
      for (auto it = m_vect_dest_reg_table.rbegin(); it != m_vect_dest_reg_table.rend(); ++it) {
        if (it->pc != uop_pc && it->dest_reg == src_reg) {
          UInt64 wait_entry_pc = it->pc;
          if (m_priority_manager->getPriority(wait_entry_pc) == PriorityManager::inst_priority_t::High) {
            break;
          }
          // for (auto jt = m_backward_dep_table.begin(); jt != m_backward_dep_table.end(); ++jt) {
          //   fprintf (stderr, "m_backward_dep_table[%ld]: PC=%08lx\n", std::distance(jt, m_backward_dep_table.begin()), *jt);
          // }
          if (std::find(m_backward_dep_table.begin(), m_backward_dep_table.end(), wait_entry_pc) == m_backward_dep_table.end()) {
            if (m_backward_dep_table.size() >= m_BACKWORD_DEP_TABLE_SIZE) {
              m_backward_dep_table.pop_front();
            }
            m_backward_dep_table.push_back(wait_entry_pc);
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
  // m_backward_dep_tableに自分のPCが含まれていれば、優先命令化する。
  if (std::find(m_backward_dep_table.begin(), m_backward_dep_table.end(), entry_pc) != m_backward_dep_table.end()) {
    m_priority_manager->setPriority(entry_pc, PriorityManager::inst_priority_t::High);
    m_priority_manager->AddHighInst(entry_pc);
    entry->uop->setStrongPriorityInst();
    fprintf (stderr, "%ld: Priority backpropagation: PC=%08lx %s\n",
                     now.getCycleCount(), entry_pc, entry->uop->getMicroOp()->getInstruction()->getDisassembly().c_str());
    m_backward_dep_table.erase(std::remove(m_backward_dep_table.begin(), m_backward_dep_table.end(), entry_pc), m_backward_dep_table.end());
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
 * ParOOOの場合の優先度の伝搬などの制御を行う
*/
void RobTimer::manageInstructionStatic (RobEntry *entry)
{
  UInt64 entry_pc = entry->uop->getMicroOp()->getInstruction()->getAddress();
  if (!entry->uop->getMicroOp()->isVector()) {
    return;
  }
  PriorityManager::inst_priority_t priority = m_priority_manager->getPriority_Static(entry_pc);
  if (priority == PriorityManager::inst_priority_t::High) {
    entry->uop->setStrongPriorityInst();
  } else if (entry->uop->getMicroOp()->isVecMem()) {
    entry->uop->setStrongPriorityInst();
  } else {
    /* default: keep instruction priority as normal*/
    entry->uop->setReserveInst();
  }
}

/*
 * VecReserveSimple:
 * ベクトル命令のリオーダリングを簡潔に管理する方法
 * - ループの各イテレーション内で全ベクトル命令をプログラムオーダで履歴に記録
 * - 分岐命令実行時に履歴をクリア（新しいイテレーションの開始）
 * - キャッシュミス率の評価はベクトルロード命令のみで行う
 * - 先頭からキャッシュミス率の悪いベクトルロード命令までの全ベクトル命令をリオーダリング対象とする
 * - それ以外のベクトル命令はインオーダ実行（Reserve）
 */
void RobTimer::manageInstructionSimple (RobEntry *entry)
{
  const MicroOp *uop = entry->uop->getMicroOp();
  UInt64 entry_pc = uop->getInstruction()->getAddress();

  // 分岐命令の場合、履歴をクリア（新しいイテレーションの開始）
  // デバッグ: 履歴サイズが大きい場合、すべての命令の種類をログ出力
  if (m_vec_inst_history.size() > m_VEC_INST_HISTORY_SIZE - 10) {
    fprintf(stderr, "%ld: VecReserveSimple DEBUG: Processing PC=%08lx, isBranch=%d, isVector=%d, history_size=%ld\n",
            now.getCycleCount(), entry_pc, uop->isBranch() ? 1 : 0, uop->isVector() ? 1 : 0, m_vec_inst_history.size());
  }
  
  if (uop->isBranch()) {
    if (!m_vec_inst_history.empty()) {
      fprintf(stderr, "%ld: VecReserveSimple: Branch detected at PC=%08lx, clearing history (size=%ld)\n",
              now.getCycleCount(), entry_pc, m_vec_inst_history.size());
      m_vec_inst_history.clear();

      // リオーダリングリストもクリア（新しいイテレーションでは全てリオーダリング可能）
      m_reordering_target_pcs.clear();
      fprintf(stderr, "%ld: VecReserveSimple: Reordering list cleared. All vec insts in new iteration can be reordered.\n",
              now.getCycleCount());
    }
    return;
  }

  // ベクトル命令でない場合は何もしない
  if (!uop->isVector()) {
    return;
  }

  bool is_vec_load = (uop->getSubtype() == MicroOp::UOP_SUBTYPE_VEC_LOAD);

  // 全ベクトル命令を履歴に追加（プログラムオーダで格納）
  // 同じPCが既に存在する場合は追加しない（同一イテレーション内では一度だけ）
  bool found = false;
  for (const auto &entry : m_vec_inst_history) {
    if (entry.pc == entry_pc) {
      found = true;
      break;
    }
  }

  if (!found) {
    vec_inst_entry_t new_entry;
    new_entry.pc = entry_pc;
    new_entry.is_vec_load = is_vec_load;

    // 履歴サイズを制限（ループが非常に長い場合の対策）
    // サイズが上限に達している場合、古いエントリを削除してから新しいエントリを追加
    if (m_vec_inst_history.size() >= m_VEC_INST_HISTORY_SIZE) {
      fprintf(stderr, "%ld: VecReserveSimple PC=%08lx: WARNING - History size at limit %ld, removing oldest entry. Last 5 entries before removal: ",
              now.getCycleCount(), entry_pc, m_VEC_INST_HISTORY_SIZE);
      // 最後の5つのエントリを表示
      size_t start_idx = (m_vec_inst_history.size() >= 5) ? m_vec_inst_history.size() - 5 : 0;
      for (size_t i = start_idx; i < m_vec_inst_history.size(); i++) {
        fprintf(stderr, "%08lx(%s) ", m_vec_inst_history[i].pc, 
                m_vec_inst_history[i].is_vec_load ? "LOAD" : "OTHER");
      }
      fprintf(stderr, "\n");
      // 履歴がクリアされない原因を調査
      fprintf(stderr, "  DEBUG: History not cleared by branch - checking if branch instructions are being processed...\n");
      m_vec_inst_history.pop_front(); // 最も古いエントリを削除（FIFO方式）
    }

    m_vec_inst_history.push_back(new_entry);

    // ベクトルロード命令の場合、キャッシュミス率をチェックして即座に再構築
    if (is_vec_load) {
      SInt8 counter = m_mem_stats->getSaturationCounter(entry_pc);
      // 飽和カウンタが閾値以上の場合、即座に再構築を実行
      if (counter >= m_MISS_RATE_THRESHOLD) {
        fprintf(stderr, "%ld: VecReserveSimple: High miss-rate detected at PC=%08lx (counter=%d, threshold=%d), triggering immediate rebuild\n",
                now.getCycleCount(), entry_pc, counter, m_MISS_RATE_THRESHOLD);
        rebuildReorderingListSimple();
        m_last_rebuild_cycle = now.getCycleCount();
      }
    }

    // 新しいベクトル命令が追加されたら、定期的にリオーダリングリストを再構築
    if (now.getCycleCount() - m_last_rebuild_cycle >= m_REBUILD_INTERVAL) {
      rebuildReorderingListSimple();
      m_last_rebuild_cycle = now.getCycleCount();
    }
  }

  // リオーダリング対象リストに含まれている場合はNormal（リオーダリング可能）
  // それ以外はReserve（インオーダ実行）
  if (m_reordering_target_pcs.empty()) {
    // リストが空の場合は全部リオーダリング可能（Normal）
    // 何も設定しない（デフォルトでNormal）
    // fprintf(stderr, "%ld: VecReserveSimple: PC=%08lx is Normal (list empty)\n",
    //         now.getCycleCount(), entry_pc);
  } else if (m_reordering_target_pcs.find(entry_pc) != m_reordering_target_pcs.end()) {
    // リオーダリング対象
    // 何も設定しない（デフォルトでNormal）
    // fprintf(stderr, "%ld: VecReserveSimple: PC=%08lx is Normal (in reordering list)\n",
    //         now.getCycleCount(), entry_pc);
  } else {
    // リオーダリング禁止
    entry->uop->setReserveInst();
    fprintf(stderr, "%ld: VecReserveSimple: PC=%08lx is Reserve (not in reordering list)\n",
            now.getCycleCount(), entry_pc);
  }
}

/*
 * リオーダリング対象リストを再構築
 * m_mem_statsの飽和カウンタを使用して、キャッシュミス率の高いベクトルロード命令を検出
 * 先頭からそのベクトルロード命令までの全ベクトル命令をリオーダリング対象とする
 */
void RobTimer::rebuildReorderingListSimple()
{
  m_reordering_target_pcs.clear();

  // 飽和カウンタが最も高い（キャッシュミス率が高い）ベクトルロード命令を探す
  UInt64 worst_pc = 0;
  size_t worst_idx = 0;

  fprintf(stderr, "%ld: VecReserveSimple: Rebuilding reordering list...\n", now.getCycleCount());

  for (size_t i = 0; i < m_vec_inst_history.size(); i++) {
    const auto &entry = m_vec_inst_history[i];
    UInt64 pc = entry.pc;
    bool is_vec_load = entry.is_vec_load;

    // ベクトルロード命令のみ、飽和カウンタを取得
    if (is_vec_load) {
      SInt8 counter = m_mem_stats->getSaturationCounter(pc);

      fprintf(stderr, "  VecInst history[%ld]: PC=%08lx, type=%s, counter=%d\n",
              i, pc, "LOAD", counter);

      // 飽和カウンタが閾値以上の命令を探す
      if (counter >= m_MISS_RATE_THRESHOLD) {
        worst_pc = pc;
        worst_idx = i;
      }
    } else {
      fprintf(stderr, "  VecInst history[%ld]: PC=%08lx, type=%s\n",
              i, pc, "ARITH/STORE");
    }
  }

  // キャッシュミス率の高いベクトルロード命令が見つかった場合
  if (worst_pc != 0) {
    // 先頭からそのベクトルロード命令までの全ベクトル命令をリオーダリング対象とする
    for (size_t i = 0; i <= worst_idx; i++) {
      m_reordering_target_pcs.insert(m_vec_inst_history[i].pc);
    }

    fprintf(stderr, "%ld: VecReserveSimple: Reordering list rebuilt. Worst VecLoad PC=%08lx, list_size=%ld\n",
            now.getCycleCount(), worst_pc, m_reordering_target_pcs.size());

    // デバッグ出力：リオーダリング対象リスト
    fprintf(stderr, "  Reordering targets (all vec insts up to worst load): ");
    for (auto pc : m_reordering_target_pcs) {
      fprintf(stderr, "%08lx ", pc);
    }
    fprintf(stderr, "\n");
  } else {
    // キャッシュミス率が閾値以下の場合、リストは空のまま（全部リオーダリング可能）
    fprintf(stderr, "%ld: VecReserveSimple: No high miss-rate vec load found. All vec insts can be reordered.\n",
            now.getCycleCount());
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
