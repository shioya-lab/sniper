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

VectorIssueTracer::VectorIssueTracer()
   : m_block_start_cycle(0)
   , m_last_issue_cycle(0)
   , m_BLOCK_TIMEOUT(1000)
   , m_BLOCK_MIN_SIZE(2)
{
}

VectorIssueTracer::~VectorIssueTracer()
{
}

void VectorIssueTracer::traceVectorIssue(DynamicMicroOp *uop, UInt64 issue_cycle)
{
   if (!uop->isFirst()) {
      // First uopのみ記録
      return;
   }

   UInt64 pc = uop->getMicroOp()->getInstruction()->getAddress();
   UInt64 seq_num = uop->getSequenceNumber();

   VectorIssueTrace trace;
   trace.pc = pc;
   trace.sequence_number = seq_num;
   trace.issue_cycle = issue_cycle;
   trace.disassembly = uop->getMicroOp()->getInstruction()->getDisassembly();

   // 現在のブロックに追加
   // ブロックの区切り：タイムアウト、またはPCが既存ブロックに含まれていない
   bool new_block = false;
   if (m_current_block_pcs.empty()) {
      new_block = true;
   } else if ((issue_cycle - m_last_issue_cycle) > m_BLOCK_TIMEOUT) {
      // タイムアウト
      new_block = true;
   } else {
      // PCが既存ブロックに含まれているかチェック（ループ検出）
      auto it = std::find(m_current_block_pcs.begin(), m_current_block_pcs.end(), pc);
      if (it == m_current_block_pcs.end() && m_current_block_pcs.size() >= m_BLOCK_MIN_SIZE) {
         // 新しいPCで、ブロックが十分大きい場合は新しいブロック
         new_block = true;
      }
   }

   if (new_block) {
      // 新しいブロックの開始
      if (!m_current_block_pcs.empty() && m_current_block_pcs.size() >= m_BLOCK_MIN_SIZE) {
         detectBlockPattern();
      }
      m_current_block_pcs.clear();
      m_block_start_cycle = issue_cycle;
   }

   m_last_issue_cycle = issue_cycle;

   // プログラム順序での位置を決定
   auto it = std::find(m_current_block_pcs.begin(), m_current_block_pcs.end(), pc);
   if (it == m_current_block_pcs.end()) {
      // 新しいPC
      trace.program_order_pos = m_current_block_pcs.size();
      m_current_block_pcs.push_back(pc);
   } else {
      // 既存のPC（ループ内）
      trace.program_order_pos = std::distance(m_current_block_pcs.begin(), it);
   }

   // インオーダ/アウトオブオーダの判定
   // isReserveInst()を使用: true = Reserve命令（インオーダ実行）、false = 通常命令（アウトオブオーダ実行可能）
   trace.is_inorder = uop->isReserveInst();

   m_vector_issue_trace.push_back(trace);
}

void VectorIssueTracer::detectBlockPattern()
{
   if (m_current_block_pcs.size() < m_BLOCK_MIN_SIZE) {
      return;  // ブロックが小さすぎる
   }

   // パターンのキーとしてPCシーケンスを使用
   std::vector<UInt64> pattern_key = m_current_block_pcs;

   auto it = m_issue_patterns.find(pattern_key);
   if (it == m_issue_patterns.end()) {
      // 新しいパターン
      IssuePattern pattern;
      pattern.pc_sequence = pattern_key;
      pattern.count = 1;
      pattern.first_occurrence_cycle = m_block_start_cycle;
      // このブロック内の発行順序を記録（最後のN個のトレースから）
      // 現在のブロックに関連するトレースを抽出
      for (auto trace_it = m_vector_issue_trace.rbegin(); trace_it != m_vector_issue_trace.rend(); ++trace_it) {
         if (std::find(pattern_key.begin(), pattern_key.end(), trace_it->pc) != pattern_key.end()) {
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

   // 最も出現回数の多いパターンを特定
   UInt64 max_count = 0;
   std::vector<UInt64> most_common_pattern;

   for (const auto& pair : m_issue_patterns) {
      if (pair.second.count > max_count) {
         max_count = pair.second.count;
         most_common_pattern = pair.first;
      }
   }

   if (max_count > 0) {
      std::cout << "----------------------------------------\n";
      std::cout << "Vector Issue Pattern Analysis\n";
      std::cout << "----------------------------------------\n";
      std::cout << "Most common pattern (count=" << max_count << "):\n";
      const IssuePattern& pattern = m_issue_patterns[most_common_pattern];
      for (size_t i = 0; i < pattern.pc_sequence.size(); ++i) {
         std::cout << "  [" << i << "] PC=0x" << std::hex << pattern.pc_sequence[i] << std::dec << "\n";
      }
      std::cout << "Total unique patterns: " << m_issue_patterns.size() << "\n";
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
   std::vector<UInt64> most_common_pattern;

   for (const auto& pair : m_issue_patterns) {
      if (pair.second.count > max_count) {
         max_count = pair.second.count;
         most_common_pattern = pair.first;
      }
   }

   if (max_count == 0 || most_common_pattern.empty()) {
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
         if (std::find(most_common_pattern.begin(), most_common_pattern.end(), trace.pc) != most_common_pattern.end()) {
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
   for (UInt64 pc : most_common_pattern) {
      pc_to_node[pc] = node_id++;
      // PCに対応するアセンブリとインオーダ情報を取得
      String disasm = "";
      bool is_inorder = false;
      for (const auto& trace : pattern_traces) {
         if (trace.pc == pc) {
            disasm = trace.disassembly;
            is_inorder = trace.is_inorder;
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
   for (size_t i = 0; i < most_common_pattern.size() - 1; ++i) {
      fprintf(fp, "  node%d -> node%d [style=solid, color=black, label=\"prog\"];\n",
              pc_to_node[most_common_pattern[i]], 
              pc_to_node[most_common_pattern[i+1]]);
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
         auto from_it = std::find(most_common_pattern.begin(), most_common_pattern.end(), trace.pc);
         auto to_it = std::find(most_common_pattern.begin(), most_common_pattern.end(), next_trace.pc);
         if (from_it != most_common_pattern.end() && to_it != most_common_pattern.end()) {
            size_t from_pos = std::distance(most_common_pattern.begin(), from_it);
            size_t to_pos = std::distance(most_common_pattern.begin(), to_it);
            
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
   fprintf(fp, "Pattern Size: %lu instructions\\n", most_common_pattern.size());
   fprintf(fp, "In-Order: %lu, Out-of-Order: %lu\\n", inorder_count, ooo_count);
   fprintf(fp, "Solid: Program Order, Dashed: Issue Order (OoO)\";\n");
   fprintf(fp, "  labelloc=top;\n");
   fprintf(fp, "  fontsize=14;\n");
   fprintf(fp, "}\n");

   fclose(fp);
   std::cout << "Graphviz file generated: " << filename << "\n";
   std::cout << "  Pattern count: " << max_count << "\n";
   std::cout << "  Pattern size: " << most_common_pattern.size() << " instructions\n";
   std::cout << "  In-Order: " << inorder_count << ", Out-of-Order: " << ooo_count << "\n";
}

