/*
 * This file is covered under the Interval Academic License, see LICENCE.academic
 */

#ifndef ROB_TIMER_VECTOR_TRACE_H_
#define ROB_TIMER_VECTOR_TRACE_H_

#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include "stats.h"

class DynamicMicroOp;

class VectorIssueTracer
{
public:
   // ベクトル命令の実行フロートレース用データ構造
   struct VectorIssueTrace {
      UInt64 pc;                    // 命令のPC
      UInt64 sequence_number;       // プログラム順序でのシーケンス番号
      UInt64 issue_cycle;           // 発行サイクル
      UInt64 program_order_pos;     // プログラム順序での位置（ブロック内）
      bool is_inorder;              // インオーダで発行されたか（isReserveInst()の結果）
      String disassembly;            // アセンブリ
   };

   // ブロック内の発行パターン
   struct IssuePattern {
      UInt64 id;                            // グローバルパターンID
      std::vector<UInt64> pc_sequence;      // PCのシーケンス（プログラム順序）
      std::vector<UInt64> issue_sequence;   // 発行順序（シーケンス番号）
      UInt64 count;                         // このパターンの出現回数
      UInt64 first_occurrence_cycle;         // 最初の出現サイクル
   };

   VectorIssueTracer();
   ~VectorIssueTracer();

   void traceVectorIssue(DynamicMicroOp *uop, UInt64 issue_cycle);
   void analyzeIssuePatterns();
   void generateIssuePatternGraphviz(const String& filename);

private:
   void detectBlockPattern();

   std::vector<VectorIssueTrace> m_vector_issue_trace;  // 発行トレース
   std::map<std::pair<std::vector<UInt64>, std::vector<bool>>, IssuePattern> m_issue_patterns;  // パターン集計（PCシーケンスとis_inorderシーケンスのペア）
   std::vector<UInt64> m_current_block_pcs;  // 現在のブロックのPCリスト
   std::vector<bool> m_current_block_is_inorder;  // 現在のブロックの各PCに対応するis_inorder情報
   std::set<UInt64> m_entry_points;  // エントリポイント（分岐先など）のセット
   UInt64 m_block_start_cycle;
   UInt64 m_last_issue_cycle;  // 最後の発行サイクル
   UInt64 m_last_pc;  // 最後のPC（連続性チェック用）
   UInt64 m_next_pattern_id;  // 次のパターンID（グローバルIDカウンター）
   const UInt64 m_BLOCK_TIMEOUT;  // ブロック検出のタイムアウト（サイクル）
   const UInt64 m_BLOCK_MIN_SIZE;  // ブロックの最小サイズ
   const UInt64 m_PC_JUMP_THRESHOLD;  // PCジャンプの閾値（エントリポイント検出用）
   bool enable_vector_trace_log;
};

#endif /* ROB_TIMER_VECTOR_TRACE_H_ */

