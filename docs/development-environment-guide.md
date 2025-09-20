# Vulkan C++ラッパーライブラリ開発環境構築ガイド

## 概要

本資料では、**Pandolabo**プロジェクトを例に、`vulkan/vulkan.hpp` (公式Vulkan C++バインディング) を基盤とした独自のVulkanAPIラッパーライブラリ開発環境の構築方法と使用技術について詳しく解説します。

## 🎯 プロジェクト概要

- **プロジェクト名**: Pandolabo
- **種類**: Vulkan C++ラッパーライブラリ (`pandora::core` 名前空間)
- **コアAPI**: `vulkan/vulkan.hpp` (公式Vulkan C++バインディング)
- **C++標準**: C++20 (Windows/MSVC) / C++26 (Linux/macOS/Clang)
- **Vulkanバージョン**: 1.4.313+
- **対象プラットフォーム**: Windows, Linux, macOS
- **開発環境**: Visual Studio Code + 自動化スクリプト

## 📁 プロジェクト構造

```
pandolabo/
├── CMakeLists.txt              # メインCMakeファイル
├── README.md                   # プロジェクト説明
├── conanfile.txt              # Conan依存関係定義
├── LICENSE                     # MITライセンス
├── Doxyfile                   # ドキュメント生成設定
├── .gitignore                 # Git除外設定
├── conanprofile*              # Conanプロファイル設定
│
├── .vscode/                   # VS Code設定（自動生成）
│   ├── tasks.json             # ビルドタスク
│   ├── launch.json            # デバッグ設定
│   ├── settings.json          # プロジェクト設定
│   └── c_cpp_properties.json  # IntelliSense設定
│
├── scripts/                   # ビルド自動化スクリプト
│   ├── build.ps1             # Windows用ビルドスクリプト
│   ├── build.sh              # Linux/macOS用ビルドスクリプト
│   └── generate_vscode_config.py  # VS Code設定自動生成
│
├── cmake/                     # CMake設定ファイル
│   └── pandolaboConfig.cmake.in
│
├── include/                   # パブリックヘッダー
│   ├── pandolabo.hpp         # メインヘッダー
│   └── pandora/              # コアライブラリヘッダー
│       ├── core.hpp
│       └── core/             # GPU、IO、UIモジュール
│           ├── gpu.hpp
│           ├── io.hpp
│           ├── ui.hpp
│           └── gpu/
│               └── vk_helper.hpp
│
├── src/                       # 実装ファイル
│   └── pandora/
│       └── core/             # Vulkan.hpp実装
│           ├── *.cpp         # コア機能実装
│           ├── gpu/          # GPU機能実装
│           ├── io/           # IO機能実装
│           ├── ui/           # UI機能実装
│           └── module_connection/
│
├── examples/                  # 使用例・サンプル
│   ├── CMakeLists.txt
│   ├── (removed) basic_usage.cpp
│   └── core/                 # コア機能サンプル
│       ├── basic_compute/    # 基本コンピュート
│       └── computing_image/  # 画像処理サンプル
│
├── tests/                     # テストファイル
│   ├── CMakeLists.txt
│   ├── test_basic.cpp
│   └── test_library.cpp
│
└── docs/                      # ドキュメント
└── build/                      # ビルド出力（Git除外）
```

## 🛠️ 使用技術スタック

### **グラフィックスAPI & ライブラリ**

#### vulkan/vulkan.hpp (公式Vulkan C++バインディング)
- **役割**: VulkanAPIのModernC++ラッパー
- **特徴**:
  - タイプセーフなVulkan操作
  - RAII（Resource Acquisition Is Initialization）サポート
  - C++例外によるエラーハンドリング
  - 動的ディスパッチローダー対応

```cpp
// vulkan/vulkan.hpp による現代的な記述
#include <vulkan/vulkan.hpp>

// pandora::core による独自ラッパー実装
namespace pandora::core {
  class Context {
    vk::Device device;
    vk::CommandBuffer commandBuffer;
    // 独自の抽象化レイヤー実装
  };
}
```

#### Vulkan SDK 1.4.313+
- **必須コンポーネント**:
  - Vulkan ランタイムライブラリ
  - バリデーションレイヤー
  - SPIRV-Tools (シェーダー最適化・検証)
  - glslang (GLSL → SPIRV コンパイラ)
  - SPIRV-Cross (SPIRV変換ツール)### **パッケージ管理**

#### Conan 2.x
- **役割**: C++パッケージマネージャー
- **管理対象（実際の依存関係）**:
  - vulkan-headers/1.4.313.0 (Vulkan C++ヘッダー)
  - vulkan-loader/1.4.313.0 (Vulkan ローダー)
  - glfw/3.4 (ウィンドウ・入力管理)
  - glslang/1.4.313.0 (GLSL→SPIRVコンパイラ)
  - spirv-cross/1.4.313.0 (SPIRV変換ツール)
  - spirv-tools/1.4.313.0 (SPIRV最適化・検証)
  - spirv-headers/1.4.313.0 (SPIRVヘッダー)
  - nlohmann_json/3.11.3 (JSON処理)
  - catch2/3.4.0 (テストフレームワーク)

```txt
# conanfile.txt (実際の設定)
[requires]
vulkan-headers/1.4.313.0
vulkan-loader/1.4.313.0
glfw/3.4
glslang/1.4.313.0
spirv-cross/1.4.313.0
spirv-tools/1.4.313.0
spirv-headers/1.4.313.0
nlohmann_json/3.11.3
catch2/3.4.0

[generators]
CMakeDeps
CMakeToolchain
```

### **ビルドシステム**

#### CMake 3.16+
- **役割**: クロスプラットフォームビルドシステム
- **Vulkan統合**: FindVulkan モジュール + Conan依存関係
- **プラットフォーム別C++標準**:
  - Windows (MSVC): C++20
  - Linux/macOS (Clang): C++26 (設定可能)

```cmake
# CMakeLists.txt (実際の設定)
cmake_minimum_required(VERSION 3.16)
project(pandolabo VERSION 1.0.0 LANGUAGES CXX)

# プラットフォーム別C++標準設定
if(WIN32)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 26)  # Linux/macOS: より高度なC++標準
endif()

# Vulkan動的ディスパッチローダー設定
target_compile_definitions(${PROJECT_NAME} PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)
```

### **テストフレームワーク**

#### Catch2 v3.4.0 (Conan管理)
- **選択理由**:
  - Conanパッケージ管理に統合
  - モダンなC++構文をサポート
  - VS Code Test Explorer対応
  - BDD（Behavior-Driven Development）スタイル対応

```cpp
#include <catch2/catch_test_macros.hpp>
#include "pandolabo.hpp"

TEST_CASE("Vulkan Context creation", "[core]") {
    pandora::core::Context context;
    REQUIRE(context.isValid());
}
```

### **開発環境**

#### Visual Studio Code + 自動化スクリプト
選択した拡張機能:

1. **C/C++ Extension Pack** (`ms-vscode.cpptools-extension-pack`)
   - IntelliSense、デバッグ、ブラウジング
   - VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 マクロ認識

2. **CMake Tools** (`ms-vscode.cmake-tools`)
   - CMakeプロジェクト統合管理

3. **CodeLLDB** (`vadimcn.vscode-lldb`) / **cppvsdbg**
   - 高度なデバッグ機能（プラットフォーム対応）

4. **Test Explorer UI**
   - テストの実行・デバッグ

5. **Doxygen Documentation Generator** (`cschlosser.doxdocgen`)
   - コメント自動生成

#### 自動化された開発環境設定

**generate_vscode_config.py**:
```python
def generate_cpp_properties(debug_mode=True):
    """VS Code IntelliSense設定の自動生成"""
    defines = [
        "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
    ]
    if debug_mode:
        defines.append("_DEBUG=1")

    return {
        "configurations": [{
            "name": "Win32",
            "defines": defines,
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++20"
        }]
    }
```

**settings.json**:
```json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Visual Studio 17 2022",
    "cmake.configureArgs": ["-DBUILD_TESTS=ON"],
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

**c_cpp_properties.json**:
```json
{
    "configurations": [{
        "name": "Win32",
        "includePath": [
            "${workspaceFolder}/include",
            "${workspaceFolder}/build/_deps/catch2-src/src"
        ],
        "cppStandard": "c++17",
        "configurationProvider": "ms-vscode.cmake-tools"
    }]
}
```

### **コード品質管理**

#### 静的解析・フォーマット
推奨ツール:
- **clang-format**: コードフォーマット統一
- **clang-tidy**: 静的解析
- **Doxygen**: ドキュメント生成

#### バージョン管理
- **Git**: 分散バージョン管理
- **.gitignore**: ビルド成果物の除外

### **パッケージ配布**

#### CMake パッケージシステム
```cmake
# パッケージ設定ファイルの生成
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
```

#### CPack統合
```cmake
include(CPack)
set(CPACK_GENERATOR "ZIP;NSIS")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
```

## 💡 技術的特徴・ベストプラクティス

### **Modern C++ Features**

#### C++17機能の活用
```cpp
// structured bindings
auto [result, success] = library.calculate();

// if constexpr
template<typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // 整数の処理
    } else {
        // その他の処理
    }
}
```

#### RAII（Resource Acquisition Is Initialization）
```cpp
class Library {
public:
    explicit Library(const std::string& name) : name_(name) {}
    ~Library() = default;  // Rule of Zero
private:
    std::string name_;
};
```

### **テストドリブン開発（TDD）**

#### 包括的テストカバレッジ
```cpp
TEST_CASE("Sum calculation", "[library]") {
    SECTION("Empty vector") {
        std::vector<double> empty;
        REQUIRE(pandolabo::Library::sum(empty) == 0.0);
    }

    SECTION("Single element") {
        std::vector<double> single = {5.0};
        REQUIRE(pandolabo::Library::sum(single) == 5.0);
    }

    SECTION("Multiple elements") {
        std::vector<double> numbers = {1.0, 2.0, 3.0, 4.0, 5.0};
        REQUIRE(pandolabo::Library::sum(numbers) == 15.0);
    }
}
```

### **ドキュメント駆動開発**

#### Doxygen統合
```cpp
/**
 * @brief 数値のリストの合計を計算
 * @param numbers 数値のリスト
 * @return 合計値
 */
static double sum(const std::vector<double>& numbers);
```

## 🚀 ビルド・実行手順

### **初期設定**
```bash
# 1. プロジェクトクローン
git clone <repository-url>
cd pandolabo

# 2. VS Codeで開く
code .

# 3. 推奨拡張機能のインストール（自動提示される）
```

### **ビルド**
```bash
# CMake設定
cmake -B build -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON

# ビルド実行
cmake --build build --config Debug

# または VS Code内で Ctrl+Shift+B
```

### **テスト実行**
```bash
# CTest実行
cd build
ctest -C Debug --verbose

# または VS Code内でTest Explorerから実行
```

### **サンプル実行**
```bash
# 使用例の実行
.\build\examples\Debug\example_basic_cube.exe
```

## 📊 プロジェクトメトリクス

### **成果物**
- **静的ライブラリ**: `pandolabod.lib` (Debug), `pandolabo.lib` (Release)
- **ヘッダーファイル**: パブリックAPI
- **CMakeパッケージ**: 他プロジェクトからの利用可能
- **テスト実行ファイル**: 自動テスト
- **ドキュメント**: Doxygen生成

### **テスト結果**
```
===============================================================================
All tests passed (7 assertions in 3 test cases)

Total Test time (real) = 0.08 sec
100% tests passed, 0 tests failed out of 4
```

## 🔧 カスタマイズ・拡張

### **新機能の追加手順**
1. `include/pandolabo/` にヘッダー追加
2. `src/` に実装追加
3. `tests/` にテスト追加
4. CMakeLists.txt更新（必要に応じて）

### **他プロジェクトでの使用**
```cmake
find_package(pandolabo REQUIRED)
target_link_libraries(your_target PRIVATE pandolabo::pandolabo)
```

## 📚 参考資料

### **公式ドキュメント**
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
- [Catch2 Tutorial](https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md)
- [VS Code C++ Documentation](https://code.visualstudio.com/docs/languages/cpp)

### **ベストプラクティス**
- [C++ Best Practices](https://github.com/cpp-best-practices/cppbestpractices)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [ISO C++](https://isocpp.org/)

### **コミュニティリソース**
- [CppCon Talks](https://www.youtube.com/user/CppCon)
- [C++ Weekly](https://www.youtube.com/c/lefticus1)
- [Meeting C++](https://meetingcpp.com/)

## 🎉 まとめ

本環境構築により、以下が実現されています：

✅ **モダンなツールチェーン**
- CMake + VS Code + Catch2の完全統合

✅ **高品質なコード**
- C++17標準準拠
- 包括的テストカバレッジ
- 静的解析統合

✅ **優れた開発体験**
- IntelliSense完全対応
- ワンクリックビルド・テスト
- デバッグ統合

✅ **スケーラブルな構造**
- パッケージ管理対応
- CI/CD対応準備完了
- ドキュメント自動生成

この環境は、個人プロジェクトから企業レベルの開発まで対応可能な、現代的C++開発のテンプレートとして活用できます。

---

**作成日**: 2025年7月23日
**バージョン**: 1.0.0
**対象**: C++17以上、CMake 3.16以上、VS Code
