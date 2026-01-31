# 設計提案（High-level層追加・Low-level維持）

## 方針（要点）

- **Low-level層は現状維持**（Vulkan.hppに近い柔軟さを保持）
- **High-level層は“薄い支援”**（任意利用。強制フレーム化しない）
- **Error Handlingは2層**
  - Low-level: 既存 `Result<T>` / `Error` に準拠
  - High-level: 失敗を収集し、最終的に `Result<T>` で返す

---

## 全体アーキテクチャ（Mermaid）

```mermaid
flowchart TB
  subgraph UserCode[User Code]
    U1[Samples / App]
  end

  subgraph HighLevel[High-level Support Layer]
    H1[Renderer]
    H2[FrameContext]
    H3[ResourceUploader]
    H4[ShaderLibrary]
    H5[PipelineCache]
    H6[ErrorCollector]
  end

  subgraph LowLevel[Existing Low-level Core]
    L1[gpu::Context]
    L2[CommandDriver]
    L3[CommandBuffer]
    L4[RenderKit / Renderpass]
    L5[Pipeline]
    L6[gpu::Buffer/Image/Descriptor]
    L7[ui::Window]
    L8[io::shader]
    L9[Barrier / Sync]
  end

  U1 -->|Optional| H1
  U1 -->|Direct use| L1
  U1 -->|Direct use| L2
  U1 -->|Direct use| L5

  H1 --> L1
  H1 --> L2
  H1 --> L4
  H2 --> L2
  H3 --> L2
  H3 --> L6
  H4 --> L8
  H5 --> L5
  H6 --> L1
  H6 --> L2
```

---

## 既存コードとの関係（Mermaid: Mapping）

```mermaid
flowchart LR
  subgraph Existing[Existing]
    E1[include/pandora/core.hpp]
    E2[gpu::Context]
    E3[CommandDriver]
    E4[RenderKit]
    E5[Pipeline]
    E6[ui::Window]
    E7[io::shader]
  end

  subgraph New[Proposed High-level]
    N1[Renderer]
    N2[FrameContext]
    N3[ResourceUploader]
    N4[ShaderLibrary]
    N5[PipelineCache]
  end

  N1 --> E2
  N1 --> E3
  N1 --> E4
  N1 --> E6
  N2 --> E3
  N3 --> E3
  N3 --> E2
  N4 --> E7
  N5 --> E5
```

---

## 使い分けの思想（柔軟性を保持）

```mermaid
flowchart TB
  User[ユーザー] --> A{使い方}
  A -->|細かく制御したい| Low[Low-level APIのみ]
  A -->|手早く試したい| High[High-level API]
  A -->|混在| Mix[High + Low 混在]
  Mix -->|必要なら| Escape[Low-levelへ降りる]
```

---

## 追加する概念（概要）

- **`Renderer`**: 簡易な描画/フレーム制御入口（内部は既存 `RenderKit` / `CommandDriver`）
- **`FrameContext`**: フレーム同期やSwapchainの細部を隠蔽（必要なら露出も可）
- **`ResourceUploader`**: 転送・バリア・セマフォをテンプレ化
- **`ShaderLibrary`**: SPIR-V読み込みの規約化（現 `io::shader` の上位）
- **`PipelineCache`**: 既存 `Pipeline` を再利用しやすくする薄いキャッシュ

---

# 詳細設計（API境界・責務・既存との接続）

## モジュール責務（Mermaid: 役割分担）

```mermaid
flowchart TB
  subgraph HighLevel[High-level Support Layer]
    R[Renderer]
    F[FrameContext]
    U[ResourceUploader]
    S[ShaderLibrary]
    P[PipelineCache]
    E[ErrorCollector]
  end

  subgraph LowLevel[Existing Low-level Core]
    C[GPU Context]
    D[Command Driver]
    K[RenderKit]
    L[Pipeline]
    G[GPU Resources]
    W[UI Window]
    IO[Shader IO]
    B[Barrier & Sync]
  end

  R -->|Frame orchestration optional| F
  R -->|Uses| K
  R -->|Uses| D
  R -->|Uses| W
  F -->|Owns sync state| D
  U -->|One-shot submit| D
  U -->|Upload targets| G
  S -->|Load SPIR-V| IO
  P -->|Build/cache| L
  E -->|Collect| C
  E -->|Collect| D
```

---

## API境界（High-levelは薄く、Low-levelへ逃げられる）

```mermaid
flowchart LR
  User[User Code] --> HL{High-level API}
  User --> LL{Low-level API}

  HL -->|Optional usage| Renderer
  HL -->|Optional usage| FrameContext
  HL -->|Optional usage| ResourceUploader
  HL -->|Optional usage| ShaderLibrary
  HL -->|Optional usage| PipelineCache

  Renderer --> LL
  FrameContext --> LL
  ResourceUploader --> LL
  ShaderLibrary --> LL
  PipelineCache --> LL

  LL -->|Direct access| gpuContext[gpu::Context]
  LL -->|Direct access| commandDriver[CommandDriver]
  LL -->|Direct access| renderKit[RenderKit]
  LL -->|Direct access| pipeline[Pipeline]
```

---

## フレーム制御の最小フロー（Mermaid: Sequence）

```mermaid
sequenceDiagram
  autonumber
  participant App as App
  participant R as Renderer
  participant F as FrameContext
  participant D as CommandDriver
  participant K as RenderKit
  participant S as Swapchain

  App->>R: beginFrame()
  R->>F: acquire()
  F->>S: acquireNextImage()
  F->>D: reset per-frame commands
  App->>R: record(draw)
  R->>D: record primary commands
  R->>K: begin/end renderpass
  App->>R: endFrame()
  R->>F: submit()
  F->>D: submit + sync
  F->>S: present()
```

---

## ResourceUploaderの責務（転送・バリアのテンプレ化）

```mermaid
flowchart TB
  RU[ResourceUploader] -->|Create staging| Staging[StagingBuffer]
  RU -->|Record copy| Cmd[Transfer CommandBuffer]
  RU -->|Insert barrier| Barrier[Barrier/Sync]
  RU -->|Submit| Queue[CommandDriver/Queue]
  RU -->|Return| Result[Result<T>]
```

---

## エラーハンドリングの設計（Low/Highの二層）

```mermaid
flowchart TB
  subgraph LowLevel
    L1[Result<T> / Error]
  end
  subgraph HighLevel
    H1[ErrorCollector]
    H2[Result<T>]
  end

  L1 -->|propagate| H1
  H1 -->|aggregate| H2
```

---

## High-level APIの最小利用イメージ（概念）

```mermaid
flowchart TB
  App[App] --> R[Renderer]
  R --> F[FrameContext]
  R --> U[ResourceUploader]
  R --> P[PipelineCache]
  R --> S[ShaderLibrary]
```

---

## 既存APIとの統合ポイント（差し込み箇所）

```mermaid
flowchart LR
  HL[High-level Layer]
  LL[Low-level Layer]

  HL -->|Uses| includeCore[include/pandora/core.hpp]
  HL -->|Builds on| gpuContext[gpu::Context]
  HL -->|Builds on| commandDriver[CommandDriver]
  HL -->|Builds on| renderKit[RenderKit]
  HL -->|Builds on| pipeline[Pipeline]
  HL -->|Builds on| ioShader[io::shader]
  HL -->|Builds on| uiWindow[ui::Window]
  LL -->|Unchanged| includeCore
```

---

## 運用方針（混在運用の前提）

- High-levelは**強制しない**。Low-levelだけで完結可能。
- High-level使用中でも、**局所的にLow-levelへ降りる**ことを想定。
- High-levelは**小さく・明示的**に保ち、抽象化の過剰化を避ける。

---

## 次の設計フェーズ（提案）

1. **API命名と公開ヘッダ配置方針の確定**
2. **最小サンプル（10〜20行相当）の構築方針決定**
3. **ErrorCollectorの責務と`Result<T>`変換規約の明文化**
4. **ResourceUploaderの対応範囲（Buffer/Image/Blit/Readback）の確定**

---

# 次フェーズ詳細設計（API命名・公開ヘッダ・最小フロー）

## 公開ヘッダ構成（案）

```mermaid
flowchart TB
  subgraph PublicHeaders[Public Headers]
    H0[pandolabo.hpp]
    H1[pandora/core.hpp]
    H2[pandora/highlevel.hpp]
    H3[pandora/highlevel/renderer.hpp]
    H4[pandora/highlevel/frame_context.hpp]
    H5[pandora/highlevel/resource_uploader.hpp]
    H6[pandora/highlevel/shader_library.hpp]
    H7[pandora/highlevel/pipeline_cache.hpp]
    H8[pandora/highlevel/error_collector.hpp]
  end

  H0 --> H1
  H0 --> H2
  H2 --> H3
  H2 --> H4
  H2 --> H5
  H2 --> H6
  H2 --> H7
  H2 --> H8
```

---

## API命名方針（低レベルの柔軟性を維持）

- High-levelは **短い名詞 + 明示的メソッド** を採用（例: `Renderer`, `FrameContext`）。
- 低レベル型の別名・再ラップは行わない（既存APIの可視性を保持）。
- High-levelは **`create*()` ではなく `build*()` / `prepare*()`** を優先し、
  低レベルの `construct*()` と衝突しないようにする。

---

## 最小利用フロー（概念）

```mermaid
sequenceDiagram
  autonumber
  participant App as App
  participant R as Renderer
  participant U as ResourceUploader
  participant P as PipelineCache
  participant F as FrameContext

  App->>R: create(window, context)
  App->>U: upload(buffer/image)
  App->>P: getOrCreate(pipelineKey)
  loop each frame
    App->>F: begin()
    App->>R: record(draw/dispatch)
    App->>F: end()
  end
```

---

## 最小サンプル（書きやすさの比較）

### Before（Low-level中心）

```cpp
// English comments only
auto window = std::make_unique<pandora::core::ui::Window>("Sample", 800, 600);
auto context = std::make_unique<pandora::core::gpu::Context>(window->getWindowSurface());

auto graphicsDriver = std::make_unique<pandora::core::CommandDriver>(
    context, pandora::core::QueueFamilyType::Graphics);

// Many explicit steps: renderpass, pipeline, buffers, barriers, sync...
// Record command buffers and submit each frame.
```

### After（High-level併用）

```cpp
// English comments only
auto window = std::make_unique<pandora::core::ui::Window>("Sample", 800, 600);
auto context = std::make_unique<pandora::core::gpu::Context>(window->getWindowSurface());

pandora::highlevel::Renderer renderer{window.get(), context.get()};
pandora::highlevel::ResourceUploader uploader{context.get()};
pandora::highlevel::PipelineCache pipelineCache{context.get()};

// Upload resources with one call
uploader.uploadBuffer(vertexBuffer, vertexData);

while (window->update()) {
  auto frame = renderer.beginFrame();
  renderer.record(frame, [&](pandora::highlevel::RenderPassScope& pass) {
    pass.bindPipeline(pipelineCache.getOrCreate(pipelineKey));
    pass.draw(verticesCount);
  });
  renderer.endFrame(frame);
}
```

### 目標（定量イメージ）

- 主要手順の行数を **$\frac{1}{2}$ 〜 $\frac{1}{3}$** に縮小
- 「同期・バリア・転送」の**記述を明示的に減らす**（ただし逃げ道は残す）

---

## ErrorCollectorの責務（明文化）

```mermaid
flowchart TB
  EC[ErrorCollector]
  LL[Low-level Result<T>]
  HL[High-level Result<T>]

  LL -->|collect| EC
  EC -->|merge| HL
```

### ルール

- Low-levelの `Result<T>` を**必ず保持**し、High-levelで失敗を隠蔽しない。
- High-levelは `ErrorCollector` に集約し、呼び出し境界で `Result<T>` に変換する。

---

## ResourceUploaderの対応範囲（案）

```mermaid
flowchart LR
  RU[ResourceUploader] --> B[Buffer Upload]
  RU --> I[Image Upload]
  RU --> R[Readback]
  RU --> C[Copy/Blit]
  RU -. optional .-> M[Mip generation]
```

---

## High-levelとLow-levelの共存指針（明文化）

- High-levelは**薄い“作業短縮”**としてのみ機能させる。
- Low-level APIへの**エスケープを常に可能**にする。
- High-levelは**フレームループの固定化をしない**（利用者が保持できる）。

---

# High-level API仕様（クラス/メソッド一覧・設計仕様）

## クラス構成（Mermaid: classDiagram）

```mermaid
classDiagram
  class Renderer {
    +Renderer(ui::Window&, gpu::Context&)
    +Renderer(shared_ptr<ui::Window>, shared_ptr<gpu::Context>)
    +FrameContext beginFrame()
    +void record(FrameContext&, RenderFn)
    +void endFrame(FrameContext&)
    +void setRenderKit(RenderKit&)
    -std::reference_wrapper<ui::Window> window
    -std::reference_wrapper<gpu::Context> context
    -std::weak_ptr<ui::Window> windowShared
    -std::weak_ptr<gpu::Context> contextShared
  }

  class FrameContext {
    +uint32_t imageIndex
    +uint32_t frameIndex
    +std::reference_wrapper<CommandDriver> driver
    +void begin()
    +void end()
  }

  class ResourceUploader {
    +ResourceUploader(gpu::Context&)
    +ResourceUploader(shared_ptr<gpu::Context>)
    +Result<void> uploadBuffer(Buffer&, Span<std::byte>)
    +Result<void> uploadImage(Image&, ImageViewInfo, Span<std::byte>)
    +Result<void> readbackBuffer(Buffer&, Span<std::byte>)
    -std::reference_wrapper<gpu::Context> context
    -std::weak_ptr<gpu::Context> contextShared
  }

  class PipelineCache {
    +PipelineCache(gpu::Context&)
    +PipelineCache(shared_ptr<gpu::Context>)
    +Pipeline& getOrCreate(PipelineKey)
    -std::reference_wrapper<gpu::Context> context
    -std::weak_ptr<gpu::Context> contextShared
  }

  class ShaderLibrary {
    +ShaderLibrary()
    +Result<ShaderModule> load(std::string_view)
  }

  class ErrorCollector {
    +void add(Error)
    +Result<void> finalize()
  }

  Renderer --> FrameContext
  Renderer --> PipelineCache
  Renderer --> ResourceUploader
```

---

## メソッド設計の意図（簡潔）

### `Renderer`

- `beginFrame()` / `endFrame()` はフレーム同期の薄いラップ。
- `record()` は **コマンド記録をラムダで受ける**だけで、Low-levelの直接操作も可能。
- `setRenderKit()` で既存 `RenderKit` を差し込み可能。

### `FrameContext`

- **フレーム毎の状態**（imageIndex, frameIndex, driver）だけを保持。
- `begin()` / `end()` は **必須ではない**が、簡易フローでの利便性を担保。

### `ResourceUploader`

- **転送のテンプレ化**のみを提供。
- バリアやセマフォは内部で最小限に挿入し、詳細制御はLow-levelで行う。

### `PipelineCache`

- パイプラインの生成コストを隠蔽しつつ、**キー設計はユーザー側**に残す。

### `ShaderLibrary`

- 既存 `io::shader` を包む薄いローダ。
- 失敗は `Result<ShaderModule>` で返し、例外隠蔽は行わない。

### `ErrorCollector`

- High-level内部の失敗を集約し、最後に `Result<T>` に変換する。

---

## API境界の厳密化（ルール）

- High-levelは **所有権を奪わない**（`Context`/`Window` は参照保持）。
- High-levelは **低レベル型の型名・値を露出**できることを保証する。
- High-levelは **例外で握りつぶさない**（`Result<T>` の維持）。

---

## メンバ保持ポリシー（rawポインタ回避）

- High-levelのクラスメンバは **rawポインタを保持しない**。
- 非所有参照は `std::reference_wrapper<T>` または `std::weak_ptr<T>` を使用する。
- API引数は `T&` または `const std::unique_ptr<T>&` を受け、
  内部では `std::reference_wrapper<T>` に保持する。
- 共有所有が必要な場合のみ `std::shared_ptr<T>` を受け、内部では `std::weak_ptr<T>` に保持する。

---

# 実装優先度ロードマップ

## フェーズ0（安全性の基盤）

- rawポインタのメンバ保持を排除（Window/Barrier Builders対応済み）
- 参照保持ポリシーを既存コードへ適用

## フェーズ1（最小High-level導入）

- `Renderer` / `FrameContext` の最小実装
- `ResourceUploader` の **Buffer Upload/Readback** まで
- 最小サンプル（Square相当）をHigh-levelで書き直し

## フェーズ2（描画系の実用化）

- `RenderPassScope` の導入（record内の簡易操作）
- `PipelineCache` と `ShaderLibrary` を正式導入
- Basic Cube をHigh-level併用で再構成

## フェーズ3（Compute/Transfer拡張）

- `ResourceUploader` の Image Upload / Blit / Mip 追加
- Compute例をHigh-level併用へ移行

## フェーズ4（エコシステム整備）

- エラーハンドリング統合
- テストとサンプルの整合性チェック自動化
- ドキュメント刷新

---

# エラーハンドリング実装詳細設計

## 目標

- Low-levelの `Result<T>` を **保持**し、High-levelで隠蔽しない。
- High-levelは複数の失敗を収集し、**1回の `Result<T>` に集約**できる。

## ErrorCollectorの振る舞い（Mermaid）

```mermaid
sequenceDiagram
  autonumber
  participant HL as High-level
  participant EC as ErrorCollector
  participant LL as Low-level

  HL->>LL: call()
  LL-->>HL: Result<T>
  HL->>EC: add(Result<T>.error)
  HL->>LL: call()
  LL-->>HL: Result<U>
  HL->>EC: add(Result<U>.error)
  HL->>EC: finalize()
  EC-->>HL: Result<void>
```

## ルール（実装指針）

- `Result<T>` の `error` は **そのまま保持**（wrapのみ許可）
- High-levelの関数は **成功/失敗を必ず返す**（例外隠蔽は禁止）
- `ErrorCollector` は **非スロー**であること（集約は例外にしない）

---

# テスト方針（既存含めて）

## 対象

- 既存 Low-level テストを維持
- High-level追加分のユニットテストを新規追加
- サンプルが **最小構成で動く**ことを確認

## テストレイヤ

1. **Unit**: High-level各クラスの単体検証（依存はモック最小）
2. **Integration**: 既存GPUテストを利用（`PANDOLABO_ENABLE_GPU_TESTS`）
3. **Examples**: Square/Cube/Computeを回帰テスト対象にする

## 運用ルール

- GPU無し環境では Unit だけ通る構成
- GPU有り環境では Integration/Examples を追加実行
- 既存テストを**必ず残し、動作互換性を担保**
