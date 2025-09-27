# Validation Layer Message Classification (pandolabo)

目的: Vulkan Validation Layer からのコールバックを分類し、スパム抑制・重大度昇格・再現性の高い診断を行うための初期ポリシーを定義。

## 分類軸

| 軸 | 説明 | 用途 |
|----|------|------|
| Severity(raw) | VK_DEBUG_UTILS_MESSAGE_SEVERITY_* | 初期フィルタ |
| Type(raw) | GENERAL / VALIDATION / PERFORMANCE / DEVICE_ADDRESS_BINDING 等 | ドメイン推定 |
| Message ID (numeric/string) | 特定パターン識別 | 昇格/抑制キー |
| Frequency | 単位時間出現数 | Rate limit 判定 |
| Resource keys | image/buffer/pipeline handle | コンテキスト付与 |

## ポリシー表 (例)

| Raw Severity | 既定ログレベル | Error 生成? | Rate Limit | 備考 |
|--------------|----------------|-------------|-----------|------|
| VERBOSE | trace | しない | 低閾値 (例: 50/秒) | 大量なら suppression |
| INFO | debug | しない | 中 | 重要統計のみ残すオプション |
| WARNING | warn | 条件付き (型依存) | 中 | swapchain/シェーダ関連は keep |
| ERROR | error | する (recoverable) | 無し | device lost 兆候昇格可能 |

## メッセージ ID 分類例 (概念)

> 正確な ID は Vulkan SDK / layer バージョン依存。ここでは概念カテゴリを示す。

| カテゴリ | 代表例 (説明) | Domain 推定 | Severity 再マップ |
|----------|---------------|-------------|------------------|
| SwapchainExtentMismatch | swapchain extent / framebuffer 不整合 | gpu_swapchain | warn→error (N回継続) |
| DescriptorMissing | descriptor set 未バインド | gpu | warn |
| PipelineLayoutMismatch | push constant / layout 不一致 | gpu_pipeline | error |
| ImageLayoutInvalid | layout 転送/使用不整合 | gpu | error |
| PerformanceSuboptimal | パフォーマンス警告 | gpu | warn (抑制可) |
| RedundantBarrier | 不要バリア | gpu | trace (抑制) |
| DeviceLostHint | タイムアウト/長時間処理ヒント | gpu | error→fatal(閾値) |
| DeprecatedUsage | 非推奨 API | gpu | debug |

## Rate Limiting アルゴリズム案

```cpp
struct MsgKey { uint32_t id; uint64_t resource; };
unordered_map<MsgKey, Counter> counters;
if(++c.count > threshold_in_interval) { if(c.count == threshold_in_interval+1) log_once("suppressed..."); return; }
```text

## Error 昇格ルール例

| 条件 | 昇格先 Severity | 理由 |
|------|-----------------|------|
| 同一 DeviceLostHint 系 ID が N (例:3) 回/秒 | fatal | デバイス喪失直前指標 |
| SwapchainExtentMismatch が連続 M フレーム | recoverable error | 早期再作成判断 |
| DescriptorMissing が同バッチ内 K 回以上 | recoverable error | 設計/バインド漏れ警告 |

## 実装フック設計 (概要)

1. Vulkan debug utils messenger コールバック → RawRecord (id, severity, type, message, objects[])
2. 正規化: Category 推定, Resource/context 付与
3. Rate limiter / Aggregator → 抑制 or カウント更新
4. 昇格判定 (ルールテーブル) → Error 生成 (Domain=gpu_validation)
5. ログ出力 (trace/debug/warn/error)

## ログ整形例

```text
[gpu_validation][warn] DescriptorMissing set=2 binding=5 (frame=123) (count=4)
```

## 将来拡張

- 動的ルールリロード (JSON 設定)
- ルールヒット統計 (prometheus exporter)
- メッセージ→ドキュメント URL 自動リンク
