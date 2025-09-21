# Streaming Resources Example

## 概要

Pandolaboライブラリの新しい同期システム（Timeline Semaphore）を活用した高度なストリーミングレンダリングのサンプルです。毎フレーム新しいリソースをGPUに転送し、古いリソースを適切に破棄しながら連続的にレンダリングする方法を示しています。

## 特徴

- **Timeline Semaphore同期**: CPU-GPU間の高精度な同期制御
- **フレームパイプライニング**: 複数フレームの並列処理
- **動的リソース管理**: 毎フレームの頂点データ生成と転送
- **メモリストリーミング**: 効率的なGPUメモリ使用
- **リアルタイムアニメーション**: プロシージャルな図形生成

## ファイル構成

- `main.cpp` - アプリケーションのエントリーポイント
- `streaming_resources.hpp` - StreamingResourcesクラスの宣言
- `streaming_resources.cpp` - StreamingResourcesクラスの実装
- `streaming.vert` - 頂点シェーダー（GLSL）
- `streaming.frag` - フラグメントシェーダー（GLSL）

## 学習ポイント

1. **Timeline Semaphore**: GPU操作の高精度な同期
2. **フレームインフライト**: 複数フレームの並列処理パターン
3. **動的バッファ管理**: 毎フレームのデータ更新
4. **CPU-GPU同期**: 効率的なパイプライン処理
5. **メモリストリーミング**: 大容量データの効率的な処理
6. **プロシージャルアニメーション**: リアルタイムでの図形生成

## ビルドと実行

```powershell
# プロジェクトルートから
.\scripts\build.ps1

# 実行メニューで "7" を選択してExampleをビルド・実行
# または直接実行
.\build\examples\Release\example_streaming_resources.exe
```

## 動作

このexampleは以下の処理を実行します：

### フレームパイプライン

1. **フレーム同期**: 前のフレームのレンダリング完了を待機
2. **データ生成**: 新しい頂点データをプロシージャルに生成
3. **GPU転送**: 頂点バッファをGPUメモリに転送
4. **転送同期**: Timeline Semaphoreで転送完了を確認
5. **レンダリング**: GPU上で描画処理を実行
6. **レンダリング同期**: レンダリング完了をシグナル

### 表示内容

- **回転する三角形**: 時間に応じて回転するカラフルな三角形
- **波動エフェクト**: Sin波による位置の変動
- **ランダムカラー**: 毎フレーム変化する頂点カラー
- **フレーム統計**: FPS、フレーム数、アクティブバッファ数の表示

## 技術的詳細

### 同期戦略

```cpp
// フレームインフライト（3フレーム並列）
static constexpr size_t FRAMES_IN_FLIGHT = 3;

// 各フレームデータ
struct FrameData {
    std::unique_ptr<plc::BufferWrapper> vertex_buffer;
    std::unique_ptr<plc::gpu::TimelineSemaphore> transfer_semaphore;
    std::unique_ptr<plc::gpu::TimelineSemaphore> render_semaphore;
    uint64_t frame_id;
    bool is_ready_for_render;
};
```

### Timeline Semaphore使用法

- **転送完了待機**: データ転送の完了を確認
- **レンダリング完了待機**: 描画処理の完了を確認
- **フレーム同期**: 古いフレームの処理完了を待機

### メモリ管理

- **CPU-GPU共有メモリ**: 効率的なデータ転送
- **循環バッファ**: 固定数のバッファを再利用
- **自動リソース管理**: RAIIによるリソース解放

## パフォーマンス指標

- **FPS表示**: リアルタイムのフレームレート
- **フレーム数**: 総処理フレーム数
- **バッファ使用量**: アクティブなバッファ数

## 応用例

このサンプルの技術は以下に応用できます：

- **動画ストリーミング**: フレーム単位の動画再生
- **プロシージャルアニメーション**: リアルタイム図形生成
- **パーティクルシステム**: 大量パーティクルの管理
- **テクスチャストリーミング**: 大容量テクスチャの分割読み込み
- **ジオメトリストリーミング**: 大規模3Dモデルの分割処理

## トラブルシューティング

### よくある問題

- **同期エラー**: Timeline Semaphoreの値が正しく設定されているか確認
- **メモリリーク**: フレームデータが適切に解放されているか確認
- **フレーム遅延**: フレームインフライト数を調整して最適化

## 関連技術

- **Vulkan Timeline Semaphore**
- **GPU Memory Management**
- **Frame Pipelining Techniques**
- **Procedural Animation**
- **Real-time Resource Streaming**
- **CPU-GPU Synchronization Patterns**
