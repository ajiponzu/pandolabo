# Basic Compute Example

## 概要

Pandolaboライブラリを使用した基本的なコンピュートシェーダーの例です。GPUを使用して並列計算を実行し、結果をCPU側で確認する方法を示しています。

## 特徴

- **コンピュートシェーダー**: GLSL compute shaderを使用した並列計算
- **GPU-CPU間データ転送**: バッファを使用したデータのやり取り
- **基本的なVulkanコンピュートパイプライン**: コンピュート専用パイプラインの構築

## ファイル構成

- `main.cpp` - アプリケーションのエントリーポイント
- `basic_computing.hpp` - BasicComputingクラスの宣言
- `basic_computing.cpp` - BasicComputingクラスの実装
- `basic.comp` - GLSLコンピュートシェーダー

## 学習ポイント

1. **Vulkanコンピュートパイプライン**: コンピュート専用パイプラインの作成方法
2. **バッファ管理**: GPU/CPUバッファの作成とデータ転送
3. **コマンドバッファ**: コンピュート処理のコマンド記録と実行
4. **同期**: GPU処理の完了待ちとデータ取得

## ビルドと実行

```powershell
# プロジェクトルートから
.\scripts\build.ps1

# 実行メニューで "7" を選択してExampleをビルド・実行
# または直接実行
.\build\examples\Release\example_basic_compute.exe
```

## 動作

このexampleは以下の処理を実行します：

1. Vulkanコンテキストとコンピュートパイプラインを初期化
2. 入力データを含むバッファを作成
3. コンピュートシェーダーを実行して並列計算
4. 結果をCPU側に転送して確認
5. 結果をコンソールに出力

## 関連技術

- **Vulkan Compute Pipeline**
- **GLSL Compute Shaders**
- **GPU Buffer Management**
- **Memory Barriers and Synchronization**
