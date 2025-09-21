# Pandolabo - Modern C++ Graphic

## 注意

- このプロジェクトは開発者本人の自己満足のためのものです．ドキュメントは自分向けの備忘録となっておりますのでご了承ください．

> 参考にされたい場合は完全に自己責任でお願いします．

- ドキュメント，および開発環境自動化スクリプトは全てGithub Copilot(Claude Sonnet 4)を活用して生成．一部誤りを含む可能性あり

> すごいシステムです，みたいな書き方をしているドキュメントはAIが生成しました．あくまで備忘録です

Windows専用C++開発環境 with Vulkan.hpp

- 対応プラットフォーム: Windows 10/11（MSVC + C++23）

## 🚀 自動セットアップ

### 🖥️ Windows (MSVC + C++23)

```powershell
# 🎯 ワンクリック環境構築 + ビルド + 実行
.\scripts\build.ps1 all

# 📋 インタラクティブモード（対話式メニュー）
.\scripts\build.ps1 -Interactive

# 🔧 個別コマンド
.\scripts\build.ps1 setup      # 環境セットアップ (.venv + Conan)
.\scripts\build.ps1 build      # プロジェクト全体ビルド
.\scripts\build.ps1 lib        # ライブラリのみビルド
.\scripts\build.ps1 examples   # サンプルをビルドして実行（対話選択あり/`-Example`指定可）
.\scripts\build.ps1 tests      # テストをビルド
.\scripts\build.ps1 test       # テストをビルドして実行（JUnit出力付き）
.\scripts\build.ps1 run        # サンプル実行のみ（対話選択あり/`-Example`指定可）
.\scripts\build.ps1 vscode     # VS Code設定生成
.\scripts\build.ps1 format     # C++コード一括フォーマット
.\scripts\build.ps1 clean      # ビルドディレクトリ削除

# 🎮 Debug/Release ビルド切り替え
.\scripts\build.ps1 build -Configuration Debug    # デバッグビルド
.\scripts\build.ps1 build -Configuration Release  # リリースビルド
```

## 📋 前提条件

### 🖥️ 前提条件（Windows）

- Python 3.8+（Conan 2.x 用）
- CMake 3.20+
- Git
- Vulkan SDK（公式インストーラーで導入）
- Visual Studio 2022 または Build Tools for Visual Studio 2022（MSVC）
- PowerShell 5.1 以上（Windows標準）

## 🎯 プロジェクト概要

**Pandolabo**は、Vulkan.hppを使用したモダンC++グラフィックスライブラリ


### 🌟 主な機能

- **Vulkan.hpp** による現代的なVulkanAPI活用
- **Windows専用設計**（MSVC前提）
- **自動化されたビルドシステム** (Conan + CMake)
- **完全なデバッグサポート** (VS Code統合)
- **包括的なサンプルとテスト**


### 📦 技術スタック

- **グラフィックスAPI**: Vulkan 1.4+
- **C++標準**: C++23（Windows）
- **パッケージマネージャー**: Conan 2.x（CMakeDeps/Toolchainに一本化）
- **ビルドシステム**: CMake 3.16+（CMakePresets準備中）
- **開発環境**: VS Code (推奨)

## 🏗️ VS Codeでの開発


### 🎯 自動セットアップ（推奨）

```powershell
# VS Code設定を自動生成（Debug/Releaseを選択）
.\scripts\build.ps1 vscode -Configuration Debug
.\scripts\build.ps1 vscode -Configuration Release
```


### 📋 手動セットアップ

1. **プロジェクトを開く：**

   ```powershell
   code .
   ```

2. **ビルド & 実行：**
   - `Ctrl+Shift+B` → ビルド
   - `F5` → デバッグ実行
   - `Ctrl+Shift+P` → `Tasks: Run Task` → 各種タスク選択

### 🐛 デバッグ実行

VS Codeでブレークポイントを使ったデバッグが可能：


### 🎮 デバッグ設定

- **🐛 Debug Tests (Debug)** - テストをデバッグモードで実行
- **🐛 Debug Example (Debug)** - サンプルをデバッグモードで実行
- **🚀 Debug Tests (Release)** - テストをリリースモードでデバッグ
- **🚀 Debug Example (Release)** - サンプルをリリースモードでデバッグ

### 他のプロジェクトでの使用

CMakeプロジェクトでの使用例：

```cmake
find_package(pandolabo REQUIRED)
target_link_libraries(your_target PRIVATE pandolabo::pandolabo)
```

## 📁 プロジェクト構造

```text
pandolabo/
├── CMakeLists.txt          # メインCMakeファイル
├── README.md               # このファイル
├── conanfile.txt          # Conan依存関係定義
├── .clang-format          # コードフォーマット設定（自動生成）
├── LICENSE                # MITライセンス
├── Doxyfile              # Doxygen設定
├── scripts/             # ビルド自動化スクリプト
│   ├── build.ps1       # Windows用ビルドスクリプト
│   └── generate_vscode_config.py  # VS Code設定自動生成
├── include/             # パブリックヘッダー
│   ├── pandolabo.hpp   # メインヘッダー
│   └── pandora/        # コアライブラリヘッダー
│       ├── core.hpp
│       └── core/       # GPU、IO、UIモジュール
├── src/                # 実装ファイル
│   └── pandora/
│       └── core/       # Vulkan.hpp実装
├── examples/           # 使用例・サンプル
│   ├── basic_usage.cpp（またはサブディレクトリ）
│   └── core/          # コア機能サンプル
├── tests/             # テストファイル
│   ├── CMakeLists.txt
│   └── test_*.cpp
├── docs/              # ドキュメント
├── cmake/             # CMake設定ファイル
└── .vscode/           # VS Code設定（自動生成）
    ├── tasks.json
    ├── launch.json
    ├── settings.json
    └── c_cpp_properties.json
```

## 🛠️ 開発ガイド


### 📝 コード編集 & フォーマット

```powershell
# C++コードの一括フォーマット（Clang-Format）
.\scripts\build.ps1 format
```


### 🔧 VS Code設定の更新

```powershell
# デバッグモード用設定生成（_DEBUG マクロ付き）
.\scripts\build.ps1 vscode -Configuration Debug

# リリースモード用設定生成（_DEBUG マクロなし）
.\scripts\build.ps1 vscode -Configuration Release
```

### 🏗️ 開発フロー

1. **環境セットアップ**

   ```powershell
   .\scripts\build.ps1 setup
   ```

2. **コード編集**
   - VS Codeでファイルを編集
   - 自動フォーマット（保存時）
   - IntelliSense によるコード補完

3. **ビルド & テスト**

   ```powershell
   .\scripts\build.ps1 build           # ビルド
   .\scripts\build.ps1 test            # テスト実行（JUnit生成）
   .\scripts\build.ps1 examples        # サンプルをビルドして実行
   .\scripts\build.ps1 run -Example …  # サンプル実行のみ
   ```

4. **デバッグ**
   - `F9` でブレークポイント設定
   - `F5` でデバッグ実行開始

### 🧩 新機能の追加

1. **ヘッダーファイル追加**
   - `include/pandora/core/` に `.hpp` ファイル追加

2. **実装ファイル追加**
   - `src/pandora/core/` に `.cpp` ファイル追加

3. **テストファイル追加**
   - `tests/` に `test_*.cpp` ファイル追加

4. **CMakeLists.txt更新**
   - 必要に応じてターゲット追加
   - c++ソース・ヘッダは自動で構成に含まれる

## 📦 ビルド成果物


### 🏗️ ライブラリ出力

- 静的ライブラリ: `build/src/{Debug|Release}/pandolabo.lib`
- 実行ファイル: `build/examples/{Debug|Release}/<example>.exe`
- テスト実行ファイル: `build/tests/{Debug|Release}/tests.exe`

GPUテストはデフォルト無効です。`setx PANDOLABO_ENABLE_GPU_TESTS 1` を実行し新しいシェルを開くと有効化されます（CIでは無効）。

## 📚 開発環境備忘録

- **[開発環境構築ガイド](docs/development-environment-guide.md)** - 使用技術と環境構築の詳細
- **[設定ファイル解説](docs/configuration-files-guide.md)** - VS Code・CMake設定の詳細
- **[クイックリファレンス](docs/quick-reference.md)** - 開発時のコマンド・手順一覧
