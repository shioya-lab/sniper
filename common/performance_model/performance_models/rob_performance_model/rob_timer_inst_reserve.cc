#include "rob_contention.h"
#include "rob_timer.h"
#include <cstdio>

#define RESERVE_DEBUG_PRINTF(...) { if (enable_reserve_log) { fprintf(stderr, __VA_ARGS__); }}

void RobTimer::manageInstructionReserve (RobEntry *entry)
{
  LOG_ASSERT_ERROR(!entry->uop->isReserveInst() &&
                       !entry->uop->isStrongPriorityInst(),
                   "Priority must not allocate before execution");
  if (m_vec_reserve_policy == VecReserveParOOO) {
    manageInstructionParOOO(entry);
  } else if (m_vec_reserve_policy == VecReserveStatic) {
    manageInstructionStatic (entry);
  } else if (m_vec_reserve_policy == VecReserveNWindow) {
    // ベクトルロードのみをリオーダリング対象とする簡潔な方法
    manageInstructionNWindow(entry);
  } else if (m_vec_reserve_policy == VecReserveAlways) {
    // 常にベクトル命令を予約に回す方針
    manageInstructionReserveVecAll(entry);
  } else if (m_vec_reserve_policy == VecReserveFlow) {
    // ReserveFlow: レジスタフロー解析による動的なInO/OoO判定
    manageInstructionRegisterFlowAnalysis(entry);
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
  // RESERVE_DEBUG_PRINTF ("size of priority_remove_queue: %ld\n", priority_remove_queue_it->size());
  RESERVE_DEBUG_PRINTF ("%ld: Priority remove propagation phase: from PC=%08lx\n",
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
        RESERVE_DEBUG_PRINTF("%ld: idx=%ld, Priority Remove Candidate: PC=%08lx\n",
                        now.getCycleCount(), idx, wait_entry_pc);
      }
    }
  }
}

// ReserveFlow: レジスタフロー解析による動的なInO/OoO判定
void RobTimer::manageInstructionRegisterFlowAnalysis(RobEntry *entry)
{
  // キャッシュミス検出時にPCをテーブルに追加（dispatch付近）
  if (entry->uop->getMicroOp()->isVecLoad()) {
    UInt64 pc = entry->uop->getMicroOp()->getInstruction()->getAddress();
    SInt8 counter = m_mem_stats->getSaturationCounter(pc);
    // Saturation Counterが閾値以上の場合はキャッシュミスと判定
    if (counter >= m_MISS_RATE_THRESHOLD) {
      RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: High miss-rate detected at PC=%08lx (counter=%d, threshold=%d)\n",
              now.getCycleCount(), pc, counter, m_MISS_RATE_THRESHOLD);
      
      // 新しい命令がインオーダトリガ命令になる場合、テーブル内の自分以外の命令に無視フラグを設定
      for (auto it = m_recent_cache_miss_vecload_table.begin(); it != m_recent_cache_miss_vecload_table.end(); ++it) {
        UInt64 table_pc = it->first;
        if (table_pc != pc) {
          // 自分以外の命令に無視フラグを設定
          it->second = true;
          RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: Setting ignore flag for PC=%08lx (new trigger PC=%08lx)\n",
                  now.getCycleCount(), table_pc, pc);
        }
      }
      
      // PCをテーブルに追加（OoO実行の条件開始のトリガとして、無視フラグはfalse）
      if (m_recent_cache_miss_vecload_table.size() >= 8) {
        // テーブルが満杯の場合、古いPCを削除（最初の要素を削除）
        UInt64 removed_pc = m_recent_cache_miss_vecload_table.begin()->first;
        m_recent_cache_miss_vecload_table.erase(m_recent_cache_miss_vecload_table.begin());
        RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: Table full, removing oldest PC=%08lx (table size before removal: %zu)\n",
                now.getCycleCount(), removed_pc, m_recent_cache_miss_vecload_table.size() + 1);
      }
      m_recent_cache_miss_vecload_table[pc] = false;  // 新規追加時は無視フラグはfalse
      RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: Added PC=%08lx to table (table size: %zu)\n",
              now.getCycleCount(), pc, m_recent_cache_miss_vecload_table.size());
    }
  }

  // ReserveFlow: テーブルに含まれるPCの命令が発見された場合、レジスタフラグを設定（OoO実行の条件開始）
  UInt64 pc = entry->uop->getMicroOp()->getInstruction()->getAddress();
  // テーブルに含まれ、かつ無視フラグが立っていない場合のみ、レジスタフラグを設定
  auto it = m_recent_cache_miss_vecload_table.find(pc);
  if (it != m_recent_cache_miss_vecload_table.end() &&
      !it->second &&  // 無視フラグが立っていない（false）場合のみ
      entry->uop->getMicroOp()->getDestinationRegistersLength() > 0) {
    
    RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: Setting ooo_dependency flag for PC=%08lx (trigger instruction)\n",
            now.getCycleCount(), pc);
    
    // デスティネーションレジスタ（ベクトルレジスタのみ）にフラグを設定
    for (uint32_t i = 0; i < entry->uop->getMicroOp()->getDestinationRegistersLength(); i++) {
      dl::Decoder::decoder_reg dest_reg = entry->uop->getMicroOp()->getDestinationRegister(i);
      if (Sim()->getDecoder()->is_reg_vector(dest_reg)) {
        registerDependencies->setOooDependency(dest_reg);
        RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow:   Set ooo_dependency for vector register %u\n",
                now.getCycleCount(), dest_reg);
      }
    }
  } else if (it != m_recent_cache_miss_vecload_table.end() && it->second) {
    // デバッグ: 無視フラグが立っている場合
    RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: PC=%08lx is in table but ignored (ignore flag is set)\n",
            now.getCycleCount(), pc);
  }

  // インオーダ/アウトオブオーダ判定
  // ソースレジスタ（ベクトルレジスタのみ）のooo_dependencyフラグをチェック
  bool should_inorder = false;
  UInt64 pc_check = entry->uop->getMicroOp()->getInstruction()->getAddress();
  for (uint32_t i = 0; i < entry->uop->getMicroOp()->getSourceRegistersLength(); i++) {
    dl::Decoder::decoder_reg source_reg = entry->uop->getMicroOp()->getSourceRegister(i);
    if (Sim()->getDecoder()->is_reg_vector(source_reg) && registerDependencies->hasOooDependency(source_reg)) {
      should_inorder = true;
      RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: PC=%08lx depends on v%u with ooo_dependency flag, forcing in-order execution\n",
              now.getCycleCount(), pc_check, source_reg - 64);
      break;
    }
  }
  if (should_inorder) {
    entry->uop->setReserveInst();  // インオーダ実行を強制
    RESERVE_DEBUG_PRINTF("%ld: VecReserveFlow: PC=%08lx set to Reserve (in-order execution)\n",
            now.getCycleCount(), pc_check);
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
  //   RESERVE_DEBUG_PRINTF ("m_vect_dest_reg_table[%ld]: PC=%08lx, dest_reg=%d\n",
  //     reg_idx, it->pc, it->dest_reg);
  // }
  auto uop_pc = uop->getInstruction()->getAddress();
  // ソース・オペランドを生成する命令をm_vec_histから探索して、優先度をHighにする。
  for (size_t idx = 0; idx < uop->getSourceRegistersLength(); ++idx) {
    dl::Decoder::decoder_reg src_reg = uop->getSourceRegister(idx);
    // RESERVE_DEBUG_PRINTF (" src_reg[%ld] = %d\n", idx, src_reg);
    if (Sim()->getDecoder()->is_reg_vector(src_reg)) {
      for (auto it = m_vect_dest_reg_table.rbegin(); it != m_vect_dest_reg_table.rend(); ++it) {
        if (it->pc != uop_pc && it->dest_reg == src_reg) {
          UInt64 wait_entry_pc = it->pc;
          if (m_priority_manager->getPriority(wait_entry_pc) == PriorityManager::inst_priority_t::High) {
            break;
          }
          // for (auto jt = m_backward_dep_table.begin(); jt != m_backward_dep_table.end(); ++jt) {
          //   RESERVE_DEBUG_PRINTF ("m_backward_dep_table[%ld]: PC=%08lx\n", std::distance(jt, m_backward_dep_table.begin()), *jt);
          // }
          if (std::find(m_backward_dep_table.begin(), m_backward_dep_table.end(), wait_entry_pc) == m_backward_dep_table.end()) {
            if (m_backward_dep_table.size() >= m_BACKWORD_DEP_TABLE_SIZE) {
              m_backward_dep_table.pop_front();
            }
            m_backward_dep_table.push_back(wait_entry_pc);
            RESERVE_DEBUG_PRINTF("%ld: %s idx=%ld(%s), Priority High Candidate: PC=%08lx\n",
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
      // RESERVE_DEBUG_PRINTF("Set Reserve Priority PC=%08lx, uop_idx=%ld %s\n",
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

  // RESERVE_DEBUG_PRINTF ("manageIsntructionPAROOO() PC=%08lx\n", entry_pc);
  // m_backward_dep_tableに自分のPCが含まれていれば、優先命令化する。
  if (std::find(m_backward_dep_table.begin(), m_backward_dep_table.end(), entry_pc) != m_backward_dep_table.end()) {
    m_priority_manager->setPriority(entry_pc, PriorityManager::inst_priority_t::High);
    m_priority_manager->AddHighInst(entry_pc);
    entry->uop->setStrongPriorityInst();
    RESERVE_DEBUG_PRINTF ("%ld: Priority backpropagation: PC=%08lx %s\n",
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
 * VecReserveNWindow:
 * ベクトル命令のリオーダリングを簡潔に管理する方法
 * - ループの各イテレーション内で全ベクトル命令をプログラムオーダで履歴に記録
 * - 分岐命令実行時に履歴をクリア（新しいイテレーションの開始）
 * - キャッシュミス率の評価はベクトルロード命令のみで行う
 * - 先頭からキャッシュミス率の悪いベクトルロード命令までの全ベクトル命令をリオーダリング対象とする
 * - それ以外のベクトル命令はインオーダ実行（Reserve）
 */
void RobTimer::manageInstructionNWindow (RobEntry *entry)
{
  const MicroOp *uop = entry->uop->getMicroOp();
  UInt64 entry_pc = uop->getInstruction()->getAddress();

  if (uop->isBranch()) {  
    // Backwardへの分岐の場合
    // Temporary: PCの方向がBackwardかどうかは、SIFTのトレースでは分からないので、とりあえず全部の分岐命令で処理している
    m_reserve_nwindow_ordering_counter = 0; // リオーダリングを有効にする
    return;
  }

  // ベクトル命令でない場合は何もしない
  if (!uop->isVector()) {
    return;
  }

  bool is_vec_load = (uop->getSubtype() == MicroOp::UOP_SUBTYPE_VEC_LOAD);

  // ベクトルロード命令の場合、キャッシュミス率をチェックしてダウンカウンタを初期化
  if (is_vec_load) {
    SInt8 counter = m_mem_stats->getSaturationCounter(entry_pc);
    // 飽和カウンタが閾値以上の場合、そのPCのダウンカウンタを初期化（すぐには再構築しない）
    if (counter >= m_MISS_RATE_THRESHOLD) {
      // downcounter が 0になっていないPCを、reordering trigger table から削除する
      for (auto it = m_reordering_trigger_table.begin(); it != m_reordering_trigger_table.end();) {
        if (*it != entry_pc && m_mem_stats->getRebuildDowncounter(*it) > 0) {
          RESERVE_DEBUG_PRINTF("%ld:   VecReserveNWindow: Rebuild downcounter is not zero for PC=%08lx, removing from reordering trigger table\n",
                  now.getCycleCount(), *it);
          it = m_reordering_trigger_table.erase(it);
        } else {
          ++it;
        }
      }
      // 自分以外のダウンカウンタをクリア
      m_mem_stats->clearAllRebuildDowncounters();
      // ダウンカウンタが既に0でない場合のみ初期化（既に設定されている場合は上書きしない）
      if (m_mem_stats->getRebuildDowncounter(entry_pc) == 0) {
        // ダウンカウンタの初期値を設定（4）
        const SInt8 REBUILD_DOWNCOUNTER_INIT = 4;
        m_mem_stats->setRebuildDowncounter(entry_pc, REBUILD_DOWNCOUNTER_INIT);
        RESERVE_DEBUG_PRINTF("%ld: VecReserveNWindow: High miss-rate detected at PC=%08lx (counter=%d, threshold=%d), initializing rebuild downcounter to %d\n",
                now.getCycleCount(), entry_pc, counter, m_MISS_RATE_THRESHOLD, REBUILD_DOWNCOUNTER_INIT);
      }
    }
  }

  // ベクトル命令が通過するたびに、すべてのPCのダウンカウンタ（値>0）をデクリメント
  if (uop->isVector() && uop->isLast()) {
    // すべてのPCのダウンカウンタ（値>0）をデクリメントし、0になったPCがあるかチェック
    std::pair<bool, UInt64> result = m_mem_stats->decrementAllRebuildDowncounters();
    bool any_reached_zero = result.first;
    UInt64 rebuild_pc = result.second;
    
    for (auto it = m_reordering_trigger_table.begin(); it != m_reordering_trigger_table.end(); ++it) {
      RESERVE_DEBUG_PRINTF("%ld:   VecReserveNWindow: Checking PC=%08lx, Rebuild downcounter is %d\n",
              now.getCycleCount(), *it, m_mem_stats->getRebuildDowncounter(*it));
    }

    // いずれかのPCのダウンカウンタが0になったら、リオーダリングリストを再構築
    if (any_reached_zero) { 
      RESERVE_DEBUG_PRINTF("%ld: VecReserveNWindow: Rebuild downcounter reached zero for some PC, triggering rebuild at PC=%08lx\n",
              now.getCycleCount(), rebuild_pc);
      // downcounter が 0になっていないPCを、reordering trigger table から削除する
      for (auto it = m_reordering_trigger_table.begin(); it != m_reordering_trigger_table.end();) {
        RESERVE_DEBUG_PRINTF("%ld:   VecReserveNWindow: Checking PC=%08lx, Rebuild downcounter is %d\n",
                now.getCycleCount(), *it, m_mem_stats->getRebuildDowncounter(*it));
        if (m_mem_stats->getRebuildDowncounter(*it) > 0) {
          RESERVE_DEBUG_PRINTF("%ld:   VecReserveNWindow: Rebuild downcounter is not zero for PC=%08lx, removing from reordering trigger table\n",
                  now.getCycleCount(), *it);
          it = m_reordering_trigger_table.erase(it);
        } else {
          ++it;
        }
      }
      // Reordering Trigger Table に追加
      updateReorderingTriggerTable(rebuild_pc);
    }
  }

  if (m_reordering_trigger_table.find(entry_pc) != m_reordering_trigger_table.end()) {
    // ここから先のN命令は、リオーダリングを禁止する
    m_reserve_nwindow_ordering_counter = m_RESERVE_NWINDOW_ORDERING_COUNTER_INIT;
  } else if (m_reserve_nwindow_ordering_counter > 0) {
    // リオーダリング禁止
    entry->uop->setReserveInst();
    RESERVE_DEBUG_PRINTF("%ld: VecReserveNWindow: PC=%08lx is Reserve (now window counter = %d)\n",
            now.getCycleCount(), entry_pc, m_reserve_nwindow_ordering_counter);

    if (uop->isLast()) {
      m_reserve_nwindow_ordering_counter--;
    }
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
  RESERVE_DEBUG_PRINTF ("propagatePriInst() ");
  for (size_t idx = 0; idx < entry->uop->getInitialDependenciesLength(); ++idx) {
    RESERVE_DEBUG_PRINTF (" %ld", idx);
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
        RESERVE_DEBUG_PRINTF (
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
        RESERVE_DEBUG_PRINTF (
            "%ld: Priority backpropagation: Strong propagated from "
            "PC=%08lx to PC=%08lx\n",
            now.getCycleCount(),
            entry_pc,
            waiting_entry->uop->getMicroOp()->getInstruction()->getAddress());
        propagatePriInst (waiting_entry, result);
      }
    }
  }
  RESERVE_DEBUG_PRINTF ("\n");
}
