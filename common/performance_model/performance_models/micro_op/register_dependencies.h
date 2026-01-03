#ifndef __REGISTER_DEPENDENCIES_H
#define __REGISTER_DEPENDENCIES_H

#include "fixed_types.h"
#include <decoder.h>

//extern "C" {
//#include <xed-reg-enum.h>
//}

class DynamicMicroOp;

class RegisterDependencies {
private:
    // Array containing the sequence number of the producers for each of the registers.
    // FIXME Depending on the architecture we may have too many elements
    // Not easy to get last element statically with the library
  uint64_t producers[280];  //XED_REG_LAST;
  uint64_t producerLength[280];
  
  // 新規追加（ベクトルレジスタのみ使用、1エントリあたり1ビット）
  bool ooo_dependency[280];  // 1ビットフラグ、OoO実行を行う命令への依存を示す、ベクトルレジスタのみ使用（無視条件は外部で設定）
  
public:
  RegisterDependencies();

  void setDependencies(DynamicMicroOp& microOp, uint64_t lowestValidSequenceNumber);
  uint64_t peekProducer(dl::Decoder::decoder_reg reg, uint64_t lowestValidSequenceNumber);

  void clear();
  
  // 新規追加メソッド（フラグ設定・取得機能のみ、条件判定は外部で行う）
  void setOooDependency(dl::Decoder::decoder_reg reg);  // OoO実行依存フラグを設定（1ビット）
  bool hasOooDependency(dl::Decoder::decoder_reg reg);  // OoO実行依存フラグをチェック
  void clearOooDependency(dl::Decoder::decoder_reg reg);  // OoO実行依存フラグをクリア
  void clearAllOooDependency();  // すべてのベクトルレジスタのOoO実行依存フラグをクリア
};

#endif /* __REGISTER_DEPENDENCIES_H */
