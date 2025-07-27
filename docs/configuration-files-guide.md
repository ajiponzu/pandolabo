# VS Code & CMake設定ファイル詳細解説

## 概要

本資料では、PandolaboプロジェクトのVulkan C++ラッパーライブラリ開発環境で使用されるVS CodeとCMakeの設定ファイルについて、自動生成スクリプトと合わせて詳細な解説を行います。

## 🎯 プロジェクト実態

**Pandolabo**は、`vulkan/vulkan.hpp` (公式Vulkan C++バインディング) を基盤とした**独自のVulkanAPIラッパーライブラリ**です。

### 技術スタック
- **コアAPI**: `vulkan/vulkan.hpp` (公式Vulkan C++バインディング)
- **ツールチェーン**: glslang, SPIRV-Tools, SPIRV-Headers
- **ウィンドウ管理**: GLFW
- **依存管理**: Conan 2.x
- **C++標準**: C++20 (Windows/MSVC), C++26 (Linux/macOS/Clang)

## 🤖 自動設定生成システム

### `scripts/generate_vscode_config.py`

```python
def generate_cpp_properties(debug_mode=True):
    """
    VS Code IntelliSense設定の自動生成
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC マクロとデバッグ設定を適切に設定
    """
    defines = [
        "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"  # Vulkan動的ディスパッチ必須マクロ
    ]
    if debug_mode:
        defines.append("_DEBUG=1")

    return {
        "configurations": [{
            "name": "Win32",
            "defines": defines,
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/build/conan/include"  # Conan依存関係 (Vulkan SDK等)
            ],
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++20",
            "intelliSenseMode": "windows-msvc-x64"
        }]
    }
```

### ビルドスクリプトとの統合

```powershell
# build.ps1 から自動呼び出し
.\scripts\build.ps1 vscode -Configuration Debug   # デバッグ用設定
.\scripts\build.ps1 vscode -Configuration Release # リリース用設定
```

## 📁 VS Code設定ファイル

### `.vscode/settings.json`

```json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Visual Studio 17 2022",
    "cmake.configureArgs": [
        "-DBUILD_TESTS=ON",
        "-DBUILD_EXAMPLES=ON"
    ],
    "cmake.buildArgs": [
        "--config",
        "Release"
    ],
    "cmake.debugConfig": {
        "console": "integratedTerminal"
    },
    "files.associations": {
        "*.h": "cpp",
        "*.hpp": "cpp",
        "*.cpp": "cpp",
        "*.comp": "glsl",
        "*.vert": "glsl",
        "*.frag": "glsl"
    },
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

#### 設定項目の詳細

| 項目 | 説明 | 意図 |
|------|------|------|
| `cmake.buildDirectory` | ビルド出力ディレクトリ | ソースとビルド成果物の分離 |
| `cmake.generator` | ビルドシステム生成器 | Windows環境でのVisual Studio統合 |
| `cmake.configureArgs` | CMake設定引数 | テスト・サンプルビルドを有効化 |
| `cmake.buildArgs` | ビルド引数 | デフォルトでリリースビルド |
| `files.associations` | ファイル関連付け | C++・GLSL ファイルの適切な認識 |

### `.vscode/c_cpp_properties.json` (自動生成)

```json
{
    "configurations": [
        {
            "name": "Win32",
            "defines": [
                "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1",
                "_DEBUG=1"
            ],
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/build/conan/include"
            ],
            "compilerPath": "cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++20",
            "intelliSenseMode": "windows-msvc-x64"
        }
    ],
    "version": 4
}
```

#### C++設定項目の詳細

| 項目 | 説明 | 重要性 |
|------|------|--------|
| `VULKAN_HPP_DISPATCH_LOADER_DYNAMIC` | Vulkan動的ディスパッチローダー有効化 | ⭐⭐⭐ 必須 |
| `_DEBUG` | デバッグモード識別子 | ⭐⭐ デバッグ時のみ |
| `includePath` | インクルードパス指定 | ⭐⭐⭐ IntelliSense必須 |
| `cppStandard` | C++20標準使用 (Windows), C++26 (Linux/macOS) | ⭐⭐⭐ モダンC++機能 |

### Conan依存関係の詳細

```txt
# conanfile.txt - 実際の依存関係
[requires]
vulkan-headers/1.4.313.0     # Vulkan C++ ヘッダー
vulkan-loader/1.4.313.0      # Vulkan ローダー
glfw/3.4                     # ウィンドウ・入力管理
glslang/1.4.313.0           # GLSL → SPIRV コンパイラ
spirv-cross/1.4.313.0       # SPIRV 変換ツール
spirv-tools/1.4.313.0       # SPIRV 最適化・検証
spirv-headers/1.4.313.0     # SPIRV ヘッダー
nlohmann_json/3.11.3        # JSON処理
catch2/3.4.0                # テストフレームワーク
```

### CMakeLists.txt の実際の構成

```cmake
# プラットフォーム別C++標準設定
if(WIN32)
    # Windows: MSVC + C++20 (安定版)
    set(CMAKE_CXX_STANDARD 20)
else()
    # Linux/macOS: Clang + C++26 (最新版)
    set(CMAKE_CXX_STANDARD 26)
endif()

# Vulkan動的ディスパッチローダー設定
target_compile_definitions(${PROJECT_NAME} PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

# 外部ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glslang::glslang
        glslang::SPIRV
        spirv-cross::spirv-cross
        spirv-tools::spirv-tools
        Vulkan::Loader
        vulkan-headers::vulkan-headers
        nlohmann_json::nlohmann_json
        glfw
)
```

### `.vscode/launch.json` (デバッグ設定)

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "🐛 Debug Tests (Debug)",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/tests/Debug/tests.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "🧪 Build Tests"
        },
        {
            "name": "🚀 Debug Example (Release)",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/examples/Release/basic_usage.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "🔨 Build (Release)"
        }
    ]
}
```
            "intelliSenseMode": "windows-msvc-x64",
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}
```

#### IntelliSense設定の詳細

| 項目 | 値 | 説明 |
|------|-----|------|
| `includePath` | プロジェクト、Catch2パス | ヘッダー検索パス |
| `cppStandard` | c++17 | C++17機能の有効化 |
| `configurationProvider` | cmake-tools | CMakeとの統合 |

### `.vscode/launch.json`

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Tests",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/tests/tests.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "gdb.exe",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build-tests"
        }
    ]
}
```

#### デバッグ設定の詳細

| 項目 | 説明 | 効果 |
|------|------|------|
| `program` | 実行ファイルパス | テスト実行ファイルを指定 |
| `preLaunchTask` | 起動前タスク | ビルドの自動実行 |
| `MIMode` | デバッガモード | GDB互換デバッグ |

### `.vscode/extensions.json`

```json
{
    "recommendations": [
        "ms-vscode.cpptools-extension-pack",
        "ms-vscode.cmake-tools",
        "twxs.cmake",
        "vadimcn.vscode-lldb",
        "matepek.vscode-catch2-test-adapter",
        "ms-vscode.test-adapter-converter",
        "cschlosser.doxdocgen",
        "ms-vscode.hexeditor",
        "eamodio.gitlens",
        "ms-vscode.live-server",
        "yzhang.markdown-all-in-one"
    ]
}
```

#### 推奨拡張機能の詳細

| 拡張機能 | 機能 | 重要度 |
|----------|------|--------|
| `cpptools-extension-pack` | C++統合開発環境 | ⭐⭐⭐⭐⭐ |
| `cmake-tools` | CMakeプロジェクト管理 | ⭐⭐⭐⭐⭐ |
| `catch2-test-adapter` | テストエクスプローラー | ⭐⭐⭐⭐ |
| `doxdocgen` | ドキュメント生成 | ⭐⭐⭐ |
| `gitlens` | Git履歴表示 | ⭐⭐⭐ |

## 📄 CMake設定ファイル

### メインCMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(pandolabo VERSION 1.0.0 LANGUAGES CXX)

# C++標準の設定
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# コンパイラ設定
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# デバッグビルドでのシンボル情報
set(CMAKE_DEBUG_POSTFIX "d")

# ライブラリターゲット
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
```

#### CMake設定の詳細

| 設定項目 | 値/効果 | 選択理由 |
|----------|---------|----------|
| `CMAKE_CXX_STANDARD` | 17 | Modern C++機能の活用 |
| `CMAKE_CXX_STANDARD_REQUIRED` | ON | 標準バージョンの強制 |
| `CMAKE_CXX_EXTENSIONS` | OFF | 標準準拠の徹底 |
| `CMAKE_DEBUG_POSTFIX` | "d" | デバッグ版の識別 |

### テスト用CMakeLists.txt

```cmake
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE
    ${PROJECT_NAME}
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(tests)
```

#### テスト設定の詳細

| 項目 | 説明 | 効果 |
|------|------|------|
| `FetchContent` | 依存関係の自動取得 | ビルド時のCatch2自動ダウンロード |
| `catch_discover_tests` | テスト自動発見 | CTestとの統合 |
| `Catch2::Catch2WithMain` | メイン関数付きライブラリ | テストファイルの簡素化 |

## 🔧 パッケージ配布設定

### パッケージ設定ファイル

```cmake
# パッケージ設定ファイルの生成
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)
```

### CPack設定

```cmake
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ Static Library")
set(CPACK_GENERATOR "ZIP;NSIS")
```

## 📋 設定ファイルのベストプラクティス

### **CMakeベストプラクティス**

1. **Modern CMake使用**
   ```cmake
   # ❌ 古い方法
   include_directories(include)

   # ✅ Modern CMake
   target_include_directories(target PUBLIC include)
   ```

2. **Generator Expressions活用**
   ```cmake
   target_include_directories(${PROJECT_NAME}
       PUBLIC
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
   )
   ```

3. **バージョン管理**
   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(name VERSION 1.0.0)
   ```

### **VS Code設定ベストプラクティス**

1. **設定の分離**
   - ワークスペース設定 vs ユーザー設定
   - プロジェクト固有設定の明確化

2. **IntelliSense最適化**
   ```json
   {
       "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
       "C_Cpp.intelliSenseEngine": "default"
   }
   ```

3. **拡張機能管理**
   - 必須vs推奨の明確化
   - バージョン指定の検討

## 🚀 実行・デバッグワークフロー

### **開発ワークフロー**

1. **コード編集**
   - IntelliSenseによる自動補完
   - リアルタイム構文チェック

2. **ビルド**
   ```
   Ctrl+Shift+B → CMake: Build
   ```

3. **テスト**
   ```
   Test Explorer → Run All Tests
   ```

4. **デバッグ**
   ```
   F5 → Debug Tests
   ```

### **トラブルシューティング**

#### よくある問題と解決方法

| 問題 | 原因 | 解決方法 |
|------|------|----------|
| IntelliSenseが効かない | configurationProvider未設定 | cmake-toolsを設定 |
| ビルドエラー | CMakeキャッシュ問題 | build/フォルダ削除 |
| テストが見つからない | Catch2統合問題 | Test Adapter拡張確認 |

## 📊 設定ファイルメトリクス

### **ファイルサイズ・複雑性**

| ファイル | 行数 | 複雑性 | 保守性 |
|----------|------|--------|--------|
| CMakeLists.txt | ~80行 | 中 | 高 |
| settings.json | ~15行 | 低 | 高 |
| c_cpp_properties.json | ~20行 | 低 | 高 |
| launch.json | ~25行 | 中 | 高 |

### **設定の再利用性**

- ✅ **高い再利用性**: プロジェクト名の変更のみで他プロジェクトに適用可能
- ✅ **拡張性**: 新機能追加時の設定変更最小限
- ✅ **保守性**: 設定の意図が明確

## 🎯 まとめ

本設定により実現される開発環境：

✅ **統合開発環境**
- VS Code + CMake + Catch2の完全統合

✅ **高効率開発**
- ワンクリックビルド・テスト・デバッグ

✅ **品質保証**
- 静的解析、テスト自動化

✅ **保守性**
- 設定の構造化・文書化

この設定ファイル群は、現代的なC++開発のテンプレートとして、様々なプロジェクトに適用可能です。

---

**更新日**: 2025年7月23日
**対象環境**: Windows 10/11, VS Code, CMake 3.16+
