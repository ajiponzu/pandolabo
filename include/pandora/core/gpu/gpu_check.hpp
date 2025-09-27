/*
 * gpu_check.hpp - Minimal macro front-end for Vulkan result checking
 *
 * 方針 ("最小化版"):
 *  - マクロは式文字列化 (#expr) + 呼び出し位置捕捉のみに限定
 *  - 主要ロジックは inline 関数 check_vk に集約 (ゼロ/極小オーバーヘッド)
 *  - 例外使用可否はコンパイル時フラグで分岐 (PLB_NO_EXCEPTIONS)
 *  - 追加メタ情報 (api_name / detail / context_id) は必要時のみ関数直呼び
 *
 * 提供要素:
 *   PLB_GPU_CHECK(expr)            : 最頻出用。expr が失敗なら内部ハンドラへ。
 *   check_vk(...)                  : 拡張情報付き/ポリシー切替の実体関数。
 *   （後で）PLB_TRY 等の early-return マクロを追加予定。
 */

#pragma once

#include <cstdint>
#include <source_location>
#include <vulkan/vulkan.hpp>

// まだ エラー/ステータス の正式ヘッダが無い段階なので最小限の forward
// 宣言。
namespace pandora::core::err {
struct Error;                                 // 後で正式定義
struct Status;                                // expected 版で利用予定
[[noreturn]] void throw_error(const Error&);  // 例外モード用フック
}  // namespace pandora::core::err

namespace pandora::core::gpu {

// コンパイル時ポリシー: 例外使用 / 非使用
struct error_policy {
  static constexpr bool use_exceptions =
#if defined(PLB_NO_EXCEPTIONS)
      false;
#else
      true;
#endif
};

// 失敗時の内部ディスパッチ (実装は後で .cpp 側で本格化)
// 今は最小化版として: 例外モード時に Error へ変換→throw (仮)、非例外モード時は
// 断片的にログ化しうるフック (TODO) をコメントで残す。
inline void check_vk(
    vk::Result r,
    const char* expr_text,
    const char* api_name = nullptr,
    const char* detail = nullptr,
    std::uint64_t context_id = 0,
    std::source_location loc = std::source_location::current()) {
  if (r == vk::Result::eSuccess)
    return;

  // TODO: map_vk_result(r) → Error ドメイン/コード 変換 (未実装)
  if constexpr (error_policy::use_exceptions) {
    // 暫定: まだ Error 型がないので vk::Result
    // を文字列化して例外化する仮実装も可能だが ここでは forward 宣言した Error
    // 利用を想定 (後続フェーズで差し替え)。
    // ::pandora::core::err::throw_error(mapped_error);
    // 暫定で標準例外を使うこともできるが、ライブラリ一貫性のため空フックのまま。
  } else {
    // 非例外: 将来 Status を返す形にリファクタ。現段階では side-effect 無し。
  }
  (void)expr_text;
  (void)api_name;
  (void)detail;
  (void)context_id;
  (void)loc;  // 未使用警告抑制
}

}  // namespace pandora::core::gpu

// --- 最小マクロ: 式を 1 回評価し文字列化して check_vk へ渡す ---
#define PLB_GPU_CHECK(expr) \
  ::pandora::core::gpu::check_vk(static_cast<vk::Result>((expr)), #expr)

// 拡張情報を指定したい呼び出し向け (必要時のみ使用)
#define PLB_GPU_CHECK_EX(expr, api_name, detail, ctx) \
  ::pandora::core::gpu::check_vk(                     \
      static_cast<vk::Result>((expr)), #expr, (api_name), (detail), (ctx))

// 将来: 直接ドメイン/コード指定で例外化したい用途のために残す (最小化版では空)
#define PLB_THROW_ERROR(domain, code, severity, message) \
  do {                                                   \
    (void)(domain);                                      \
    (void)(code);                                        \
    (void)(severity);                                    \
    (void)(message);                                     \
  } while (0)
