# エラー定義と生成ポイントの整理 (pandolabo 向けガイド)

## 目的

pandolabo において「どこにエラー型を定義し」「どこで新規エラー（例外 / `expected` のエラー）を生成すべきか」を層・責務ごとに明確化し、
ホットパスを汚さず一貫性あるドメイン設計を可能にする指針。

---

## 1. エラー“定義”を置く場所

| 位置 | 内容 | 目的 |
|------|------|------|
| `pandora/core/error.hpp` (単一) | `Domain`, `Code`, `Severity`, `Error`, 例外階層 | 全モジュール共通の語彙統一 |
| `core/gpu/gpu_error.hpp` 等ドメイン別ブリッジ | 外部 API 戻り値→`Error` 変換 (`map_vk_result`) | 生戻り値を集中翻訳（散在防止） |
| `core/result.hpp` | `std::expected` alias (`Result<T>`, `Status`) | 例外→値戻り移行の柔軟性 |

→ Error 型は 1 箇所で宣言し、“翻訳関数” を各境界ファイルに置く。

---

## 2. エラーを“生成”すべき境界

| レイヤ | 生成ポイント例 | ドメイン候補 |
|-------|----------------|-------------|
| Vulkan / OS 外部 API 呼び出し | `vkCreateDevice`, `vkAllocateMemory`, `vkQueuePresentKHR` | gpu / memory / gpu_swapchain |
| 複合リソース構築 | Pipeline, Framebuffer, RenderPass | gpu / gpu_pipeline |
| スワップチェーン制御 | acquire / present / recreate | gpu_swapchain |
| メモリアロケーション | `vkAllocateMemory`, サブアロケータ失敗 | memory |
| シェーダ I/O & 反射 | 読込, SPIR-V 解析, descriptor 反射 | io / gpu_shader |
| デスクリプタ管理 | レイアウト生成, セット確保 | gpu_descriptor（無ければ gpu） |
| 同期 / 待機 | Fence wait timeout, Device lost | gpu |
| コマンドバッファ確保 | プール枯渇 / level 不正 | gpu |
| UI / Window | surface 作成, モード取得 | ui / gpu_swapchain |
| ファイル書込 (将来: ログ, キャプチャ) | 書込不可, パス不正 | io |
| Validation Layer ブリッジ | 重大/冗長メッセージ昇格 | gpu_validation |
| 初期化シーケンス | 必須拡張不足, 適合デバイス無し | gpu |

---

## 3. “生成しない” (禁止/抑制) 箇所

| 箇所 | 理由 | 対応 |
|------|------|-----|
| デストラクタ | 例外で `std::terminate` リスク | `noexcept` + 内部ログ |
| ホットパス (draw/dispatch 記録) | 頻発失敗は設計ミス | 事前整合チェック |
| 軽量 Getter | 予期せぬスローで可読性低下 | `assert` / 契約 (将来) |
| 内部コンテナ操作 | STL 例外そのままで十分 | 必要なら文脈付与再throw |
| ログ出力自体 | ログ失敗で再帰的エラー | フォールバック / 無視 |

---

## 4. Domain / Code / Severity 設計指針

| 軸 | 方針 |
|----|------|
| Domain | 再発分析/統計したい“責務境界”粒度 (gpu / gpu_swapchain / gpu_shader / memory / io / ui / validation / logic / transient など) |
| Code | 対処分岐が変わる単位 (out_of_memory / device_lost / swapchain_out_of_date / shader_compile_failed / invalid_argument / timeout / retry_later / not_found / io_failure など) |
| Severity | `note`, `warning`, `recoverable`, `fatal`（ランタイムポリシー分岐の核） |

---

## 5. 生成ワークフロー（推奨）

1. 外部 API 直後

   ```cpp
   auto r = device.createBuffer(...);
   PLB_GPU_CHECK(r, "vkCreateBuffer", "size=..., usage=...", bufferId);
   ```

   - 成功: 続行
   - 失敗: `VkResult`→`Error` 変換→ログ→例外 or `unexpected`
2. 複合生成 (例: Pipeline)
   - 各ステップで早期退出
   - 途中確保済リソース解放
   - 最後に総括メッセージで一つの Error
3. スワップチェーン
   - `VK_ERROR_OUT_OF_DATE_KHR` → `swapchain_out_of_date` (recoverable)
4. Transient (`timeout`, `would_block`, `retry_later`)
   - 再試行判断は上位に委譲（ここではリトライしない）
5. Validation Layer
   - コールバックで message ID マップ
   - 昇格/抑制/集約 → 必要なら `gpu_validation` ドメイン Error 生成

---

## 6. 生成粒度の選択

| 粒度 | 利点 | 欠点 | 用途 |
|------|------|-----|-----|
| 各 API | 原因特定が最短 | コード量増 | 単純 create/allocate |
| 機能単位 | 高レベルで簡潔 | 詳細ログ依存 | Pipeline 組み立て |
| 翻訳レイヤ | ドメイン一貫 | 文脈喪失可能 | swapchain / validation |
| 集約 | 一括表示 | リアルタイム性低 | シェーダ複数エラー |

推奨: “API 境界” + “複合最外層” の二層構成。

---

## 7. pandolabo 現行構造へのマッピング

| ファイル | 新エラー生成ポイント |
|----------|----------------------|
| `gpu/context.hpp` | instance/device 作成, queue 取得 |
| `gpu/device.hpp` | feature/extension 不足 |
| `gpu/swapchain.hpp` | create/recreate/acquire/present |
| `gpu/buffer.hpp` / `image.hpp` | create + allocate + bind |
| `gpu/descriptor.hpp` | layout / set alloc |
| `gpu/shader.hpp` | SPIR-V load / reflection |
| `pipeline.hpp` | pipeline layout / create |
| `renderpass.hpp` / framebuffer | renderpass / framebuffer create |
| `command_buffer.*` | allocate / begin / end |
| `synchronization.hpp` / fence/semaphore | create / wait timeout |
| `io.hpp` | shader file read/write |
| `module_connection/gpu_ui.hpp` | surface / mode / present integration |

---

## 8. ホットパスで“発生させない”設計

| 対象 | 戦略 |
|------|------|
| draw/dispatch 記録 | 事前に pipeline / descriptor 完整合保証 |
| descriptor binding | 生成時に layout 整合検証済み前提 |
| push constants | サイズ/範囲はデバッグ初期化で検証し本番は assert のみ |

---

## 9. ログ協調ルール

| タイミング | ルール |
|-----------|-------|
| 翻訳点 | 1 回だけログ（後段で重複禁止） |
| 上位ループ | Severity に応じて: fatal -> 終了/再初期化, recoverable -> 再試行 or 特殊分岐 |
| Transient | ログレベルは warn もしくは info（スパム抑制: サンプリング） |

---

## 10. API シグネチャ整合

| パターン | 推奨戻り値 | 例 |
|----------|-----------|----|
| 構築/重い初期化 | 例外 or `Result<T>` | createSwapchain() |
| 軽量クエリ | 値 | getExtent() |
| 失敗通常分岐 | `Result<T>` | compileShaderVariant() |
| “欠如” | `std::optional<T>` | findCachedPipeline() |
| 簡単成功可否 | `bool` | pollEvent() |

---

## 11. サマリ（短縮版）

- Error 型定義は 1 箇所、翻訳関数はドメイン境界に集約
- “生成すべき” は 外部 API / 複合構築 / 状態遷移 / transient / validation 昇格
- ホットパス・デストラクタ・Getter では生成禁止
- Domain/Code/Severity の三層で対処ポリシーを明確化
- 二層（API 境界 + 複合最外）で詳細ログと総括エラーを分離
- 早期ログは 1 回だけ、上位はポリシー判断に専念

---
