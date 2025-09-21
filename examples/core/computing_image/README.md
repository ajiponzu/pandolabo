# Computing Image Example

## 概要

Pandolaboライブラリを使用した画像処理のコンピュートシェーダーサンプルです。GPUを使用して画像データにフィルタやエフェクトを適用し、結果を画像ファイルとして出力する方法を示しています。

## 特徴

- **GPU画像処理**: コンピュートシェーダーによる並列画像処理
- **画像ファイルI/O**: PNG画像の読み込みと書き出し
- **テクスチャとイメージ**: Vulkanイメージリソースの活用
- **メモリ効率**: GPU上での直接的な画像データ処理

## ファイル構成

- `main.cpp` - アプリケーションのエントリーポイント
- `simple_image_computing.hpp` - SimpleImageComputingクラスの宣言
- `simple_image_computing.cpp` - SimpleImageComputingクラスの実装
- `simple_image.comp` - 画像処理用GLSLコンピュートシェーダー
- `lenna.png` - サンプル入力画像（Lenna画像）
- `stb_image.h` - 画像読み込みライブラリ
- `stb_image_write.h` - 画像書き出しライブラリ

## 学習ポイント

1. **画像処理パイプライン**: コンピュートシェーダーによる画像フィルタリング
2. **テクスチャサンプリング**: GPU上での画像データアクセス
3. **イメージレイアウト**: Vulkanイメージレイアウトの管理
4. **ワークグループ**: 効率的な並列処理のためのワークグループ設計
5. **ファイルI/O**: STBライブラリを使用した画像ファイル処理

## ビルドと実行

```powershell
# プロジェクトルートから
.\scripts\build.ps1

# 実行メニューで "7" を選択してExampleをビルド・実行
# または直接実行
.\build\examples\Release\example_simple_image.exe
```

## 動作

このexampleは以下の処理を実行します：

1. サンプル画像（lenna.png）を読み込み
2. 画像データをGPUメモリに転送
3. コンピュートシェーダーで画像処理フィルタを適用
4. 処理結果をCPUメモリに転送
5. 処理済み画像を新しいファイルとして保存
6. 処理時間とファイル情報をコンソールに出力

## 画像処理内容

- **フィルタエフェクト**: 色調補正、ブラー、エンボスなど
- **ピクセル単位処理**: 各ピクセルに対する並列計算
- **高速処理**: GPUの並列性を活用した高速画像処理

## 出力ファイル

処理後の画像は以下のような名前で保存されます：

- `output_processed.png` - 処理済み画像
- コンソールに処理時間とファイルサイズを表示

## 関連技術

- **Vulkan Compute Pipeline for Images**
- **GLSL Image Processing Shaders**
- **GPU Memory and Image Resources**
- **STB Image Library**
- **Parallel Image Processing Algorithms**
- **Vulkan Image Layouts and Barriers**
