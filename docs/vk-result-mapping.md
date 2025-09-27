# VkResult → pandolabo Error Mapping

この表は Vulkan の `VkResult` を pandolabo の Error (Domain / Code / Severity) に正規化する際の初期指針です。実装は `gpu_error.hpp` / `gpu_error.cpp` 側で本表に従って行い、必要に応じてログ側で追加メタ情報（リカバリ方針など）を付加します。

> NOTE: 下記に無い result は `Domain::gpu`, `Code::unknown`, `Severity::recoverable` (一部は fatal) のデフォルト扱い後、ログで警告します。

| VkResult | Domain | Code | Severity | Handling Hint |
|----------|--------|------|----------|---------------|
| `VK_SUCCESS` | (none) | ok | note | 変換しない / 早期 return |
| `VK_NOT_READY` | gpu | retry_later | recoverable | 再ポーリング / フレーム継続 |
| `VK_TIMEOUT` | gpu | timeout | recoverable | 待機延長 or 上位警告 |
| `VK_EVENT_SET` | gpu | note | note | イベント通知 (通常ログ不要) |
| `VK_EVENT_RESET` | gpu | note | note | 同上 |
| `VK_INCOMPLETE` | gpu | retry_later | recoverable | 追加呼び出しで完了 (列挙 API) |
| `VK_ERROR_OUT_OF_HOST_MEMORY` | memory | out_of_memory | fatal | 即座にフレーム停止 / 再初期化困難 |
| `VK_ERROR_OUT_OF_DEVICE_MEMORY` | memory | out_of_memory | fatal | デバイス再初期化検討 |
| `VK_ERROR_INITIALIZATION_FAILED` | gpu | invalid_state | fatal | 初期化シーケンス中断 |
| `VK_ERROR_DEVICE_LOST` | gpu | device_lost | fatal | 全リソース破棄→再初期化フロー |
| `VK_ERROR_MEMORY_MAP_FAILED` | memory | invalid_state | recoverable | Map失敗。範囲・同期確認後再試行 (まれ) |
| `VK_ERROR_LAYER_NOT_PRESENT` | gpu | invalid_argument | fatal | 要求レイヤ再構成 or fallback |
| `VK_ERROR_EXTENSION_NOT_PRESENT` | gpu | invalid_argument | fatal | 有効化リスト見直し |
| `VK_ERROR_FEATURE_NOT_PRESENT` | gpu | invalid_argument | fatal | フィーチャ要求調整 |
| `VK_ERROR_INCOMPATIBLE_DRIVER` | gpu | invalid_state | fatal | ドライバ更新案内 |
| `VK_ERROR_TOO_MANY_OBJECTS` | gpu | invalid_state | recoverable | オブジェクト削減 / GC |
| `VK_ERROR_FORMAT_NOT_SUPPORTED` | gpu | invalid_argument | recoverable | 代替フォーマット選択 |
| `VK_ERROR_FRAGMENTED_POOL` | memory | invalid_state | recoverable | プール再作成 / デフラグ |
| `VK_ERROR_SURFACE_LOST_KHR` | gpu_swapchain | device_lost | fatal | surface 再構築 (OS依存) |
| `VK_ERROR_NATIVE_WINDOW_IN_USE_KHR` | gpu_swapchain | invalid_state | fatal | 既存利用との競合解消 |
| `VK_SUBOPTIMAL_KHR` | gpu_swapchain | swapchain_out_of_date | recoverable | フレーム継続しつつ再作成スケジュール |
| `VK_ERROR_OUT_OF_DATE_KHR` | gpu_swapchain | swapchain_out_of_date | recoverable | ただちに再作成 |
| `VK_ERROR_INCOMPATIBLE_DISPLAY_KHR` | gpu_swapchain | invalid_state | fatal | ディスプレイ設定変更必要 |
| `VK_ERROR_VALIDATION_FAILED_EXT` | gpu_validation | invalid_argument | recoverable | 開発中: 警告/停止ポリシー選択 |
| `VK_ERROR_INVALID_SHADER_NV` | gpu_shader | shader_compile_failed | recoverable | シェーダ修正 / フォールバック |
| `VK_ERROR_OUT_OF_POOL_MEMORY` | memory | out_of_memory | recoverable | プール再構築 or 拡張 |
| `VK_ERROR_INVALID_EXTERNAL_HANDLE` | gpu | invalid_argument | recoverable | 外部ハンドル検証 |
| `VK_ERROR_FRAGMENTATION_EXT` | memory | invalid_state | recoverable | 大規模再配置検討 |
| `VK_ERROR_NOT_PERMITTED_KHR` | gpu | invalid_state | fatal | OS権限/セキュリティ設定調整 |
| `VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT` | gpu_swapchain | swapchain_out_of_date | recoverable | フルスクリーン再取得 |
| `VK_THREAD_IDLE_KHR` | gpu | note | note | メタ: パフォーマンス計測用途 |
| `VK_THREAD_DONE_KHR` | gpu | note | note | 同上 |
| `VK_OPERATION_DEFERRED_KHR` | gpu | retry_later | recoverable | 実行遅延、再ポーリング |
| `VK_OPERATION_NOT_DEFERRED_KHR` | gpu | invalid_state | recoverable | 想定外遅延状態。ログ重視 |
| `VK_PIPELINE_COMPILE_REQUIRED_EXT` | gpu_pipeline | retry_later | recoverable | パイプラインカッシュ or 非同期コンパイル誘導 |

## 方針補足

- `Severity::fatal` はフレームループ即時脱出トリガ。`device_lost` / host/device OOM / 初期化失敗 / display incompat など。
- `swapchain_out_of_date` は recoverable だが“再作成必須”。優先度高。
- `retry_later` はポーリング or 非同期再試行オーケストレーション対象 (busy wait 禁止)。
- `invalid_argument` と `invalid_state` の使い分け: ユーザ入力/要求の不正 → argument, 内部状態/環境不整合 → state。
- `note` は統計/診断レベルで例外化しない。

## 実装メモ

```cpp
// 擬似コード (gpu_error.cpp)
Error map_vk_result(vk::Result r, const char* api, const char* detail, uint64_t ctx) {
  switch(r) {
    case vk::Result::eSuccess: /* 呼び出し側で処理 */ break;
    case vk::Result::eTimeout: return make_error(Domain::gpu, Code::timeout, Severity::recoverable, build(api, detail), (uint32_t)r, ctx);
    // ... 表に従い分岐 ...
    default:
      return make_error(Domain::gpu, Code::unknown, Severity::recoverable, build(api, detail), (uint32_t)r, ctx);
  }
}
```

## 今後の拡張候補

- 重大発生頻度計測: `device_lost` / OOM をカウンタ出力
- 動的ポリシー: Debug ビルドは特定 Code (validation) を Fatal 昇格オプション
- 設定ファイルによる Code→Severity 再マップ
