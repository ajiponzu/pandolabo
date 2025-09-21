# Pandolabo Examples

## 概要

Pandolaboライブラリの機能を学習するためのサンプルプログラム集です。基本的な使用方法から応用的な技術まで、段階的に学習できるように構成されています。

## Example一覧

各exampleの詳細については、個別のREADMEファイルを参照してください：

### 🟢 初心者向け
- **[Square Example](core/square/README.md)** - 最もシンプルな2D四角形の描画
- **[Basic Cube Example](core/basic_cube/README.md)** - 3Dキューブの回転表示

### 🟡 中級者向け
- **[Basic Compute Example](core/basic_compute/README.md)** - コンピュートシェーダーによる並列計算
- **[Computing Image Example](core/computing_image/README.md)** - GPU上での画像処理フィルタ## 共通の前提条件

### システム要件
- **OS**: Windows 10/11
- **GPU**: Vulkan対応グラフィックスカード
- **RAM**: 8GB以上推奨

### 開発環境
- **Visual Studio**: 2022 Community以上
- **Vulkan SDK**: 1.4.313.0以上
- **CMake**: 3.17.2以上
- **Python**: 3.8以上（ビルドスクリプト用）

### 依存ライブラリ
すべてConan 2.xで自動管理：
- **GLFW**: ウィンドウ管理
- **GLM**: 数学ライブラリ
- **Vulkan**: グラフィックスAPI
- **GLSL**: シェーダーコンパイラ

## ビルドと実行

### 一括ビルド
```powershell
# プロジェクトルートで
.\scripts\build.ps1

# メニューから "7" を選択してExampleをビルド・実行
```

### 個別実行
```powershell
# 特定のexampleを直接実行
.\build\examples\Release\example_square.exe
.\build\examples\Release\example_basic_cube.exe
.\build\examples\Release\example_basic_compute.exe
.\build\examples\Release\example_simple_image.exe
```

## 学習の進め方

### 推奨学習順序

1. **Square Example** → Vulkanの基本概念を理解
2. **Basic Cube Example** → 3Dグラフィックスの基礎を学習
3. **Basic Compute Example** → GPU汎用計算を体験
4. **Computing Image Example** → 実用的なGPU活用を学習

### 各exampleで身につく技術

| Example | グラフィックス | コンピュート | 3D数学 | 画像処理 |
|---------|:--------------:|:------------:|:------:|:--------:|
| Square | ✅ 基礎 | ❌ | ❌ | ❌ |
| Cube | ✅ 中級 | ❌ | ✅ 基礎 | ❌ |
| Compute | ❌ | ✅ 基礎 | ❌ | ❌ |
| Image | ❌ | ✅ 中級 | ❌ | ✅ 基礎 |

## トラブルシューティング

### よくある問題

#### ビルドエラー
```powershell
# 依存関係の再インストール
.\scripts\build.ps1
# メニューで "1" (setup) を選択
```

#### 実行時エラー
- **Vulkan validation layer**: 警告メッセージは一般的に無害
- **GPU driver**: 最新のグラフィックスドライバーに更新
- **Vulkan SDK**: 正しくインストールされているか確認

#### パフォーマンス問題
- **Release build**: Debug buildは大幅に低速
- **GPU compatibility**: 古いGPUでは一部機能に制限

### デバッグのヒント

1. **バリデーションレイヤー**: 詳細なエラー情報を提供
2. **RenderDoc**: フレーム解析ツール（別途インストール）
3. **Visual Studio Debugger**: ステップ実行とブレークポイント

## 参考資料

### 公式ドキュメント
- [Vulkan Specification](https://vulkan.lunarg.com/)
- [GLSL Language Specification](https://www.khronos.org/registry/OpenGL/specs/gl/)
- [GLM Documentation](https://github.com/g-truc/glm)

### 学習リソース
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Learn OpenGL](https://learnopengl.com/) (概念理解用)
- [Real-Time Rendering](http://www.realtimerendering.com/) (理論)

## ライセンス

このexample集はPandolaboライブラリと同じライセンスの下で提供されています。詳細は`LICENSE`ファイルを参照してください。
