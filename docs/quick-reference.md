# Vulkan C++ラッパーライブラリ 開発環境クイックリファレンス

## 🚀 クイックスタート

### **必要なツール**
```bash
# Windows必須ツール
- Visual Studio 2022 (Build Tools以上) + MSVC コンパイラ
- Git for Windows
- CMake 3.16+
- Visual Studio Code
- Vulkan SDK 1.4.313+ (公式サイトからダウンロード)
- Python 3.6+ (Conan パッケージマネージャー用)

# Linux/macOS追加要件
- Clang 15+ (推奨: Clang 17+ for C++26 support)
- Vulkan drivers (プラットフォーム対応版)
```

### **1分でセットアップ**
```powershell
# 1. リポジトリクローン
git clone <repo-url>
cd pandolabo

# 2. 環境セットアップ + ビルド + 実行
.\scripts\build.ps1 all

# 3. VS Code起動
code .

# 4. デバッグ実行
F5
```

## 📋 チェックリスト

### **環境確認**
- [ ] CMake 3.16以上がインストール済み
- [ ] C++20対応コンパイラ (MSVC) / C++26対応コンパイラ (Clang)
- [ ] Vulkan SDK 1.4.313+ インストール済み
- [ ] Python 3.6+ (Conan用)
- [ ] Git設定完了
- [ ] VS Code + C++ Extension Pack

### **プロジェクト設定**
- [ ] Conanプロファイル設定 (conanprofile*)
- [ ] CMakeLists.txt設定 (Vulkan動的ディスパッチ設定済み)
- [ ] .vscode/ フォルダ設定（自動生成）
- [ ] VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 マクロ設定
- [ ] テストファイル作成 (Catch2)
- [ ] .gitignore設定

## ⚡ 主要コマンド

### **自動化スクリプト (推奨)**
```powershell
# 🎯 ワンクリック環境構築 + ビルド + 実行
.\scripts\build.ps1 all

# 📋 インタラクティブモード（対話式メニュー）
.\scripts\build.ps1 -Interactive

# 🔧 個別コマンド
.\scripts\build.ps1 setup      # 環境セットアップ (.venv + Conan)
.\scripts\build.ps1 build      # プロジェクト全体ビルド
.\scripts\build.ps1 lib        # ライブラリのみビルド
.\scripts\build.ps1 examples   # サンプルのみビルド
.\scripts\build.ps1 tests      # テストビルド
.\scripts\build.ps1 run        # サンプル実行
.\scripts\build.ps1 vscode     # VS Code設定生成
.\scripts\build.ps1 format     # C++コード一括フォーマット
.\scripts\build.ps1 clean      # ビルドディレクトリ削除

# 🎮 Debug/Release ビルド切り替え
.\scripts\build.ps1 build -Configuration Debug    # デバッグビルド
.\scripts\build.ps1 build -Configuration Release  # リリースビルド
```

### **CMakeコマンド (手動)**
```bash
# Conan依存関係インストール
conan install . --output-folder=build --build=missing

# CMake設定
cmake -B build -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
cmake --build build --config Release

# テスト実行
cd build && ctest -C Release

# インストール
cmake --install build
```

### **VS Codeショートカット**
| 操作 | ショートカット | 説明 |
|------|----------------|------|
| ビルド | `Ctrl+Shift+B` | CMakeビルド実行 |
| デバッグ | `F5` | デバッグ開始 |
| ブレークポイント | `F9` | ブレークポイント設定/解除 |
| ステップオーバー | `F10` | デバッグ実行制御 |
| ステップイン | `F11` | 関数内部に入る |
| コマンドパレット | `Ctrl+Shift+P` | CMakeコマンド実行 |

### **デバッグ設定**
| 設定名 | 説明 | 実行ファイル |
|--------|------|--------------|
| 🐛 Debug Tests (Debug) | テストをデバッグモードで実行 | `tests.exe` |
| 🐛 Debug Example (Debug) | サンプルをデバッグモードで実行 | `example_basic_cube.exe` |
| 🚀 Debug Tests (Release) | テストをリリースモードでデバッグ | `tests.exe` |
| 🚀 Debug Example (Release) | サンプルをリリースモードでデバッグ | `example_basic_cube.exe` |

## 🛠️ 必須VS Code拡張機能

```json
{
    "recommendations": [
        "ms-vscode.cpptools-extension-pack",    // C++開発 + IntelliSense
        "ms-vscode.cmake-tools",                // CMake統合
        "cschlosser.doxdocgen"                  // ドキュメント自動生成
    ]
}
```

## 📁 Vulkan C++ラッパーライブラリ プロジェクト構造

```
pandolabo/
├── CMakeLists.txt          # ✅ 必須: C++20/26 + Vulkan設定
├── conanfile.txt          # ✅ 必須: Vulkan toolchain依存関係
├── conanprofile*          # ✅ 必須: プラットフォーム別Conan設定
├── .vscode/                # ✅ 自動生成: VS Code設定
│   ├── settings.json
│   ├── c_cpp_properties.json
│   └── extensions.json
├── include/                # ✅ 必須
├── src/                    # ✅ 必須
├── tests/                  # ✅ 推奨
└── .gitignore             # ✅ 必須
```

## 🎯 重要な設定

### **CMakeLists.txt最小設定**
```cmake
cmake_minimum_required(VERSION 3.16)
project(myproject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(myproject STATIC src/main.cpp)
target_include_directories(myproject PUBLIC include)
```

### **VS Code settings.json**
```json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

## 🧪 テスト設定

### **Catch2最小設定**
```cmake
# tests/CMakeLists.txt
include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0)
FetchContent_MakeAvailable(Catch2)

add_executable(tests test_main.cpp)
target_link_libraries(tests PRIVATE myproject Catch2::Catch2WithMain)
```

### **テストファイル例**
```cpp
#include <catch2/catch_test_macros.hpp>
#include <myproject/myclass.h>

TEST_CASE("Basic test", "[myclass]") {
    REQUIRE(true);
}
```

## 🔧 トラブルシューティング

### **よくある問題**

| 問題 | 解決方法 |
|------|----------|
| CMakeが見つからない | PATH環境変数確認 |
| IntelliSenseが効かない | cmake-tools拡張確認 |
| ビルドエラー | build/フォルダ削除して再設定 |
| テストが実行されない | Catch2 Test Adapter拡張確認 |

### **デバッグ手順**
1. `Ctrl+Shift+P` → "CMake: Configure"
2. `Ctrl+Shift+P` → "CMake: Build"
3. 問題が続く場合は`build/`フォルダを削除

## 📦 パッケージ管理

### **vcpkg統合（推奨）**
```bash
# vcpkgインストール
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg && .\bootstrap-vcpkg.bat

# ライブラリインストール
.\vcpkg install catch2

# CMakeで使用
cmake -B build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg.cmake
```

### **FetchContent使用**
```cmake
include(FetchContent)
FetchContent_Declare(library_name
    GIT_REPOSITORY https://github.com/user/repo.git
    GIT_TAG v1.0.0)
FetchContent_MakeAvailable(library_name)
```

## 📚 学習リソース

### **公式ドキュメント**
- [CMake Tutorial](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [Catch2 Documentation](https://github.com/catchorg/Catch2/tree/devel/docs)
- [VS Code C++ Guide](https://code.visualstudio.com/docs/languages/cpp)

### **推奨書籍**
- "Professional CMake: A Practical Guide" - Craig Scott
- "Effective Modern C++" - Scott Meyers
- "C++ Best Practices" - Jason Turner

## 🎯 次のステップ

### **基本環境構築後**
1. **CI/CD設定**: GitHub Actions / Azure DevOps
2. **静的解析**: clang-tidy, PVS-Studio
3. **コードカバレッジ**: gcov, OpenCppCoverage
4. **ドキュメント**: Doxygen + Sphinx

### **高度な機能**
1. **パッケージ配布**: CPack設定
2. **クロスプラットフォーム**: Linux/macOS対応
3. **パフォーマンス**: Benchmarking追加
4. **セキュリティ**: 静的解析強化

## 📞 サポート・コミュニティ

### **コミュニティ**
- [r/cpp](https://reddit.com/r/cpp) - Reddit C++コミュニティ
- [Stack Overflow](https://stackoverflow.com/questions/tagged/c%2b%2b) - 技術Q&A
- [CppCon](https://cppcon.org/) - 年次カンファレンス

### **日本語リソース**
- [cpprefjp](https://cpprefjp.github.io/) - C++リファレンス
- [C++日本標準化委員会](https://isocpp.jp/)

---

**最終更新**: 2025年7月23日
**バージョン**: 1.0
**対象**: C++初心者〜中級者
