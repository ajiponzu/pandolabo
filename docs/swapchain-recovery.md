# Swapchain Recovery State Machine (pandolabo)

目的: プレゼンテーションエラー (OUT_OF_DATE / SUBOPTIMAL / SURFACE_LOST など) 発生時の再作成フローを明確化し、フレームループを安全に継続/再初期化する指針を提供。

## 状態一覧

| 状態 | 説明 | 終了条件 |
|------|------|----------|
| Active | 利用中正常状態 | acquire/present 成功 |
| NeedsRecreateSoft | SUBOPTIMAL を検知 (即時再作成不要) | フレーム境界で再作成 or degrade 進行 |
| NeedsRecreateHard | OUT_OF_DATE / フルスクリーン喪失 | 直ちに再作成ループへ |
| SurfaceInvalid | SURFACE_LOST / DISPLAY_INCOMPATIBLE | surface 再取得 (UI 再初期化) |
| Recreating | 破棄→新規作成処理中 | 成功で Active / 失敗で Fatal |
| Fatal | device lost 等で継続不能 | デバイス再初期化 or 終了 |

## 遷移 (ASCII 図)

```text
 Active
   | SUBOPTIMAL
   v
 NeedsRecreateSoft --(OUT_OF_DATE)--> NeedsRecreateHard
   | FrameEnd policy / degrade
   v
 Recreating <---- NeedsRecreateHard <---- Active (OUT_OF_DATE)
   | success
   v
 Active

 Active --(SURFACE_LOST)--> SurfaceInvalid --(surface recreate)--> Recreating
 Active --(DEVICE_LOST)--> Fatal
 Recreating --(DEVICE_LOST)--> Fatal
```

## 推奨ポリシー

| イベント | 遷移先 | アクション |
|----------|--------|-----------|
| SUBOPTIMAL | NeedsRecreateSoft | 遅延再作成フラグ立て |
| OUT_OF_DATE | NeedsRecreateHard | 即再作成シーケンス |
| SURFACE_LOST | SurfaceInvalid | surface 再取得 |
| FULL_SCREEN_EXCLUSIVE_MODE_LOST | NeedsRecreateHard | フルスクリーン再取得後再作成 |
| DEVICE_LOST | Fatal | 全再初期化 or 終了 |

## 擬似コード

```cpp
void maybe_recreate_swapchain() {
  if(state == NeedsRecreateSoft && frames_since_flag < kSoftDelay) return; // 遅延閾値
  if(state == NeedsRecreateSoft || state == NeedsRecreateHard || state == SurfaceInvalid) {
    state = Recreating;
    destroy_dependent_resources();
    if(state == SurfaceInvalid) recreate_surface();
    auto res = create_swapchain();
    if(!res) {
      if(res.error().code == Code::device_lost) state = Fatal; else state = NeedsRecreateHard; return;
    }
    recreate_image_views();
    recreate_framebuffers();
    state = Active;
  }
}
```

## ログ指針

| レベル | 条件 | 例 |
|--------|------|----|
| Info | SUBOPTIMAL 初検出 | swapchain: suboptimal scheduling recreate |
| Warn | OUT_OF_DATE | swapchain: out-of-date immediate recreate |
| Error | recreate 失敗 / surface lost | swapchain: recreate failed (vkResult=...) |
| Fatal | device lost | device lost during swapchain recreate |

## メトリクス例

- swapchain_recreate_count_total
- swapchain_recreate_fail_count_total
- swapchain_suboptimal_frames

## 将来拡張

- 遅延再作成本数ヒューリスティック
- 非同期準備 (バックグラウンド生成 + atomic swap)
- surface 破棄/再作成の失敗リトライ戦略
