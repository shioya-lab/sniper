/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#include "rob_timer_vector_trace.h"

#include "dynamic_micro_op.h"
#include "instruction.h"
#include "stats.h"

#include <iostream>
#include <algorithm>
#include <cstdio>
#include <iomanip>
#include "config.hpp"
#include "core_manager.h"

#define VECTOR_TRACE_DEBUG_PRINTF(...) { if (enable_vector_trace_log) { fprintf(stderr, __VA_ARGS__); }}

VectorIssueTracer::VectorIssueTracer()
   : m_block_start_cycle(0)
   , m_last_issue_cycle(0)
   , m_last_pc(0)
   , m_next_pattern_id(1)  // パターンIDは1から開始
   , m_BLOCK_TIMEOUT(1000)
   , m_BLOCK_MIN_SIZE(2)
   , m_PC_JUMP_THRESHOLD(256)  // 256バイト以上のジャンプはエントリポイントとみなす
   , enable_vector_trace_log(Sim()->getCfg()->getBoolArray("log/enable_vector_trace_log", 0))
{
}

VectorIssueTracer::~VectorIssueTracer()
{
}

void VectorIssueTracer::traceVectorIssue(DynamicMicroOp *uop, UInt64 issue_cycle)
{
   UInt64 pc = uop->getMicroOp()->getInstruction()->getAddress();
   UInt64 seq_num = uop->getSequenceNumber();

   bool is_branch = uop->getMicroOp()->isBranch();

   // 分岐命令が登場した場合、分岐先をエントリポイントとして記録し、ブロックを終了
   if (is_branch) {
      // 分岐命令のターゲットアドレスをエントリポイントとして記録
      IntPtr branch_target = uop->getBranchTarget();
      if (branch_target != 0) {
         m_entry_points.insert(static_cast<UInt64>(branch_target));
      }
      
      // 分岐命令が登場した = Basic Blockが確定した
      // この時点で m_current_block_pcs には確定したBasic BlockのPCシーケンス（分岐命令を含む）が含まれている
      if (m_current_block_pcs.size() >= m_BLOCK_MIN_SIZE) {
         VECTOR_TRACE_DEBUG_PRINTF ("PC = %08lx: Detect block pattern (branch): %lu\n", pc, m_current_block_pcs.size());
         detectBlockPattern();
      }
      // 新しいブロックの開始（前のブロックをクリア）
      m_current_block_pcs.clear();
      m_current_block_is_inorder.clear();
      m_block_start_cycle = issue_cycle;

      return;
   }
   

   if (!(uop->isLast() && uop->getMicroOp()->isVector())) {
      // Last uop of vector instructionのみ記録
      return;
   }

   VectorIssueTrace trace;
   trace.pc = pc;
   trace.sequence_number = seq_num;
   trace.issue_cycle = issue_cycle;
   trace.disassembly = uop->getMicroOp()->getInstruction()->getDisassembly();
   
   // 一般的な基本ブロック抽出の方法に則る
   // 基本ブロックは：
   // 1. エントリポイントから始まる（プログラム開始、分岐先、大きなPCジャンプ、タイムアウト）
   // 2. 分岐/ジャンプ/呼び出し命令で終わる（分岐命令自体もブロックに含める）
   
   bool is_new_block = false;
   

   // 最初の命令はエントリポイント
   if (m_last_pc == 0) {
      is_new_block = true;
   }
   
   // エントリポイントの検出
   if (m_last_pc != 0) {
      // PCの連続性をチェック（大きなジャンプはエントリポイント）
      UInt64 pc_diff = (pc > m_last_pc) ? (pc - m_last_pc) : (m_last_pc - pc);
      if (pc_diff > m_PC_JUMP_THRESHOLD) {
         // 大きなPCジャンプ = エントリポイント
         is_new_block = true;
      }
   }
   
   // エントリポイントに到達したかチェック
   if (m_entry_points.find(pc) != m_entry_points.end()) {
      is_new_block = true;
   }
   
   // タイムアウトで新しいブロックを開始
   if (issue_cycle - m_last_issue_cycle > m_BLOCK_TIMEOUT) {
      is_new_block = true;
   }
   
   // 新しいブロックの開始
   if (is_new_block) {
      // 前のブロックが存在し、十分なサイズがあれば確定
      if (!m_current_block_pcs.empty() && m_current_block_pcs.size() >= m_BLOCK_MIN_SIZE) {
         VECTOR_TRACE_DEBUG_PRINTF ("PC = %08lx: Detect block pattern (entry point): %lu\n", pc, m_current_block_pcs.size());
         detectBlockPattern();
      }
      // 新しいブロックの開始
      m_current_block_pcs.clear();
      m_current_block_is_inorder.clear();
      m_block_start_cycle = issue_cycle;
   }
   
   m_last_issue_cycle = issue_cycle;

   // インオーダ/アウトオブオーダの判定
   // isReserveInst()を使用: true = Reserve命令（インオーダ実行）、false = 通常命令（アウトオブオーダ実行可能）
   trace.is_inorder = uop->isReserveInst();

   // プログラム順序での位置を決定
   auto it_pos = std::find(m_current_block_pcs.begin(), m_current_block_pcs.end(), pc);
   if (it_pos == m_current_block_pcs.end()) {
      // 新しいPC
      trace.program_order_pos = m_current_block_pcs.size();
      VECTOR_TRACE_DEBUG_PRINTF ("New block instruction: %lu %08lx: %s %s disassembly: %s\n", trace.program_order_pos, pc, (trace.is_inorder ? "InO" : "OoO"), "disassembly: %s\n", trace.disassembly.c_str());
      m_current_block_pcs.push_back(pc);
      m_current_block_is_inorder.push_back(trace.is_inorder);
   } else {
      // 既存のPC（ループ内）
      trace.program_order_pos = std::distance(m_current_block_pcs.begin(), it_pos);
      // 既存のPCでもis_inorderが異なる場合は、別のBasicBlockとして扱うためブロックを分割
      size_t pos = std::distance(m_current_block_pcs.begin(), it_pos);
      if (m_current_block_is_inorder[pos] != trace.is_inorder) {
         // is_inorderが異なる場合、この時点でブロックを確定
         if (m_current_block_pcs.size() >= m_BLOCK_MIN_SIZE) {
            detectBlockPattern();
         }
         // 新しいブロックの開始
         m_current_block_pcs.clear();
         m_current_block_is_inorder.clear();
         m_block_start_cycle = issue_cycle;
         trace.program_order_pos = 0;
         m_current_block_pcs.push_back(pc);
         m_current_block_is_inorder.push_back(trace.is_inorder);
      }
   }

   m_vector_issue_trace.push_back(trace);
   
   
   m_last_pc = pc;
}

void VectorIssueTracer::detectBlockPattern()
{
   // この関数は、Basic Blockが確定した状態で呼び出される
   // m_current_block_pcs には確定したBasic BlockのPCシーケンス（プログラム順序）が含まれている
  
   if (m_current_block_pcs.size() < m_BLOCK_MIN_SIZE) {
      return;  // ブロックが小さすぎる
   }

   // パターンのキーとしてPCシーケンスとis_inorderシーケンスのペアを使用（確定したBasic BlockのPCシーケンスとis_inorder情報）
   std::pair<std::vector<UInt64>, std::vector<bool>> pattern_key = std::make_pair(m_current_block_pcs, m_current_block_is_inorder);

   auto it = m_issue_patterns.find(pattern_key);
   if (it == m_issue_patterns.end()) {
      // 新しいパターン
      IssuePattern pattern;
      pattern.id = m_next_pattern_id++;  // グローバルIDを割り当て
      pattern.pc_sequence = pattern_key.first;
      pattern.count = 1;
      pattern.first_occurrence_cycle = m_block_start_cycle;
      // このブロック内の発行順序を記録（最後のN個のトレースから）
      // 現在のブロックに関連するトレースを抽出
      for (auto trace_it = m_vector_issue_trace.rbegin(); trace_it != m_vector_issue_trace.rend(); ++trace_it) {
         if (std::find(pattern_key.first.begin(), pattern_key.first.end(), trace_it->pc) != pattern_key.first.end()) {
            // 重複チェック
            if (std::find(pattern.issue_sequence.begin(), pattern.issue_sequence.end(), trace_it->sequence_number) == pattern.issue_sequence.end()) {
               pattern.issue_sequence.insert(pattern.issue_sequence.begin(), trace_it->sequence_number);
            }
         }
         // ブロック開始より前のトレースは無視
         if (trace_it->issue_cycle < m_block_start_cycle) {
            break;
         }
      }
      VECTOR_TRACE_DEBUG_PRINTF ("New pattern [ID=%lu]. Key = %lu %lu %lu instructions\n", pattern.id, pattern_key.first.size(), pattern_key.second.size(), pattern.issue_sequence.size());
      for (size_t i = 0; i < pattern.issue_sequence.size(); ++i) {
         VECTOR_TRACE_DEBUG_PRINTF ("  [%lu] PC: %08lx %s\n", i, pattern.pc_sequence[i], (m_current_block_is_inorder[i] ? "InO" : "OoO"));
      }
      m_issue_patterns[pattern_key] = pattern;
   } else {
      // 既存のパターン
      it->second.count++;
   }
}

void VectorIssueTracer::analyzeIssuePatterns()
{
   // 最後のブロックも処理
   if (!m_current_block_pcs.empty()) {
      detectBlockPattern();
   }

   if (m_issue_patterns.empty()) {
      std::cout << "----------------------------------------\n";
      std::cout << "Vector Issue Pattern Analysis\n";
      std::cout << "----------------------------------------\n";
      std::cout << "No patterns found.\n";
      return;
   }

   // パターンを出現回数でソート
   std::vector<std::pair<std::pair<std::vector<UInt64>, std::vector<bool>>, const IssuePattern*>> sorted_patterns;
   for (const auto& pair : m_issue_patterns) {
      sorted_patterns.push_back({pair.first, &pair.second});
   }
   std::sort(sorted_patterns.begin(), sorted_patterns.end(),
             [](const auto& a, const auto& b) {
                return a.second->count > b.second->count;
             });

   std::cout << "----------------------------------------\n";
   std::cout << "Vector Issue Pattern Analysis\n";
   std::cout << "----------------------------------------\n";
   std::cout << "Total unique patterns: " << m_issue_patterns.size() << "\n";
   std::cout << "\n";

   // 上位10個（または全パターン）を出力
   size_t top_n = std::min(size_t(10), sorted_patterns.size());
   std::cout << "Top " << top_n << " patterns by occurrence count:\n";
   std::cout << "\n";

   for (size_t rank = 0; rank < top_n; ++rank) {
      const IssuePattern& pattern = *sorted_patterns[rank].second;
      const std::pair<std::vector<UInt64>, std::vector<bool>>& pattern_key = sorted_patterns[rank].first;
      
      std::cout << "Rank #" << (rank + 1) << " [ID=" << pattern.id << "] (count=" << pattern.count << ", size=" << pattern.pc_sequence.size() << " instructions):\n";
      
      // まず、このパターン内で最も長いアセンブリ命令の長さを計算
      size_t max_disassembly_len = 0;
      for (size_t i = 0; i < pattern.pc_sequence.size(); ++i) {
         UInt64 pc = pattern.pc_sequence[i];
         for (const auto& trace : m_vector_issue_trace) {
            if (trace.pc == pc) {
               if (trace.disassembly.length() > max_disassembly_len) {
                  max_disassembly_len = trace.disassembly.length();
               }
               break;
            }
         }
      }
      
      // アセンブリ命令を出力（アライン）
      for (size_t i = 0; i < pattern.pc_sequence.size(); ++i) {
         UInt64 pc = pattern.pc_sequence[i];
         // m_vector_issue_trace から該当PCのアセンブリ情報を取得
         String disassembly = "";
         for (const auto& trace : m_vector_issue_trace) {
            if (trace.pc == pc) {
               disassembly = trace.disassembly;
               break;
            }
         }
         // パターンキーからis_inorder情報を取得
         bool is_inorder = pattern_key.second[i];
         String ino_or_ooo = is_inorder ? "InO" : "OoO";
         
         // アセンブリ命令の後にスペースを追加してアライン
         size_t padding = max_disassembly_len - disassembly.length();
         std::cout << "  [" << std::setw(2) << std::setfill('0') << i << "] PC=0x" << std::hex << pc << std::dec << ": " << disassembly;
         for (size_t j = 0; j < padding; ++j) {
            std::cout << " ";
         }
         std::cout << " [" << ino_or_ooo << "]" << "\n";
      }
      std::cout << "\n";
   }
}

void VectorIssueTracer::generateIssuePatternGraphviz(const String& filename)
{
   FILE* fp = fopen(filename.c_str(), "w");
   if (!fp) {
      fprintf(stderr, "Failed to open file for writing: %s\n", filename.c_str());
      return;
   }

   fprintf(fp, "digraph VectorIssuePatterns {\n");
   fprintf(fp, "  rankdir=LR;\n");
   fprintf(fp, "  node [shape=box, style=rounded];\n");
   fprintf(fp, "  edge [fontsize=10];\n\n");

   // 最も出現回数の多いパターンを特定
   UInt64 max_count = 0;
   std::pair<std::vector<UInt64>, std::vector<bool>> most_common_pattern;

   for (const auto& pair : m_issue_patterns) {
      if (pair.second.count > max_count) {
         max_count = pair.second.count;
         most_common_pattern = pair.first;
      }
   }

   if (max_count == 0 || most_common_pattern.first.empty()) {
      fprintf(fp, "  // No patterns found\n");
      fprintf(fp, "}\n");
      fclose(fp);
      return;
   }

   const IssuePattern& pattern = m_issue_patterns[most_common_pattern];

   // このパターンに関連する発行トレースを抽出（最初の出現のみ）
   std::vector<VectorIssueTrace> pattern_traces;
   UInt64 pattern_start_cycle = pattern.first_occurrence_cycle;
   bool in_pattern = false;
   for (const auto& trace : m_vector_issue_trace) {
      if (trace.issue_cycle >= pattern_start_cycle && 
          trace.issue_cycle < pattern_start_cycle + m_BLOCK_TIMEOUT) {
         if (std::find(most_common_pattern.first.begin(), most_common_pattern.first.end(), trace.pc) != most_common_pattern.first.end()) {
            pattern_traces.push_back(trace);
            in_pattern = true;
         } else if (in_pattern) {
            // パターン外の命令が来たら終了
            break;
         }
      }
   }

   // ノードを作成（PCごと）
   std::map<UInt64, int> pc_to_node;
   std::map<UInt64, bool> pc_to_is_inorder;  // PCごとのインオーダ/アウトオブオーダ情報
   int node_id = 0;
   for (size_t i = 0; i < most_common_pattern.first.size(); ++i) {
      UInt64 pc = most_common_pattern.first[i];
      bool is_inorder = most_common_pattern.second[i];
      pc_to_node[pc] = node_id++;
      // PCに対応するアセンブリを取得
      String disasm = "";
      for (const auto& trace : pattern_traces) {
         if (trace.pc == pc) {
            disasm = trace.disassembly;
            break;
         }
      }
      pc_to_is_inorder[pc] = is_inorder;
      
      // アセンブリが長すぎる場合は短縮
      if (disasm.length() > 40) {
         disasm = disasm.substr(0, 37) + "...";
      }
      // Graphvizのラベル用にエスケープ
      String label = disasm;
      size_t pos = 0;
      while ((pos = label.find("\"", pos)) != String::npos) {
         label.replace(pos, 1, "\\\"");
         pos += 2;
      }
      while ((pos = label.find("\n", pos)) != String::npos) {
         label.replace(pos, 1, "\\n");
         pos += 2;
      }
      
      // インオーダ/アウトオブオーダに応じて色を変更
      String node_color = is_inorder ? "lightblue" : "lightcoral";
      String node_style = is_inorder ? "filled" : "filled";
      String order_label = is_inorder ? "InO" : "OoO";
      
      fprintf(fp, "  node%d [label=\"0x%lx\\n%s\\n[%s]\", style=%s, fillcolor=%s];\n", 
              pc_to_node[pc], pc, label.c_str(), order_label.c_str(), 
              node_style.c_str(), node_color.c_str());
   }

   fprintf(fp, "\n");

   // プログラム順序でのエッジ（実線）
   fprintf(fp, "  // Program order (solid lines)\n");
   for (size_t i = 0; i < most_common_pattern.first.size() - 1; ++i) {
      fprintf(fp, "  node%d -> node%d [style=solid, color=black, label=\"prog\"];\n",
              pc_to_node[most_common_pattern.first[i]], 
              pc_to_node[most_common_pattern.first[i+1]]);
   }

   fprintf(fp, "\n");

   // 発行順序でのエッジ（点線、アウトオブオーダの場合）
   fprintf(fp, "  // Issue order (dashed lines for out-of-order)\n");
   
   // パターンに関連するトレースを発行順序でソート
   std::vector<VectorIssueTrace> sorted_traces = pattern_traces;
   std::sort(sorted_traces.begin(), sorted_traces.end(), 
             [](const VectorIssueTrace& a, const VectorIssueTrace& b) {
                return a.issue_cycle < b.issue_cycle || 
                       (a.issue_cycle == b.issue_cycle && a.sequence_number < b.sequence_number);
             });
   
   // 連続して発行された命令のペアを探す
   for (size_t i = 0; i < sorted_traces.size() - 1; ++i) {
      const auto& trace = sorted_traces[i];
      const auto& next_trace = sorted_traces[i + 1];
      
      // 同じサイクルまたは連続サイクルで発行された
      if (next_trace.issue_cycle <= trace.issue_cycle + 1) {
         int from_node = pc_to_node[trace.pc];
         int to_node = pc_to_node[next_trace.pc];
         
         // プログラム順序と異なる場合はアウトオブオーダ
         auto from_it = std::find(most_common_pattern.first.begin(), most_common_pattern.first.end(), trace.pc);
         auto to_it = std::find(most_common_pattern.first.begin(), most_common_pattern.first.end(), next_trace.pc);
         if (from_it != most_common_pattern.first.end() && to_it != most_common_pattern.first.end()) {
            size_t from_pos = std::distance(most_common_pattern.first.begin(), from_it);
            size_t to_pos = std::distance(most_common_pattern.first.begin(), to_it);
            
            if (from_pos > to_pos) {
               // アウトオブオーダ
               fprintf(fp, "  node%d -> node%d [style=dashed, color=red, label=\"issue(OoO)\"];\n",
                       from_node, to_node);
            }
         }
      }
   }

   fprintf(fp, "\n");
   
   // 統計情報を追加
   UInt64 inorder_count = 0, ooo_count = 0;
   for (const auto& trace : pattern_traces) {
      if (trace.is_inorder) {
         inorder_count++;
      } else {
         ooo_count++;
      }
   }
   
   fprintf(fp, "  label=\"Most Common Pattern (count=%lu)\\n", max_count);
   fprintf(fp, "Pattern Size: %lu instructions\\n", most_common_pattern.first.size());
   fprintf(fp, "In-Order: %lu, Out-of-Order: %lu\\n", inorder_count, ooo_count);
   fprintf(fp, "Solid: Program Order, Dashed: Issue Order (OoO)\";\n");
   fprintf(fp, "  labelloc=top;\n");
   fprintf(fp, "  fontsize=14;\n");
   fprintf(fp, "}\n");

   fclose(fp);
   std::cout << "Graphviz file generated: " << filename << "\n";
   std::cout << "  Pattern count: " << max_count << "\n";
   std::cout << "  Pattern size: " << most_common_pattern.first.size() << " instructions\n";
   std::cout << "  In-Order: " << inorder_count << ", Out-of-Order: " << ooo_count << "\n";
}

