#!/usr/bin/env python3
"""
VSCode用の設定ファイル一式を自動生成するスクリプト
プラットフォーム別に最適化された設定を生成します
"""

import os
import json
import glob
import subprocess
import platform
import argparse
from pathlib import Path


def get_conan_home():
    """Conanホームディレクトリを取得"""
    try:
        result = subprocess.run(
            [".venv/Scripts/conan.exe", "config", "home"],
            capture_output=True,
            text=True,
            cwd=".",
        )
        return result.stdout.strip()
    except:
        # フォールバック
        return os.path.expanduser("~/.conan2")


def get_glm_include_path():
    """GLMのインクルードパスを取得"""
    conan_home = get_conan_home()
    glm_pattern = f"{conan_home}/p/*glm*/p/include"

    glm_paths = glob.glob(glm_pattern)
    for path in glm_paths:
        if os.path.exists(os.path.join(path, "glm", "glm.hpp")):
            return path

    return None


def get_conan_include_paths():
    """
    Conanパッケージのインクルードパスを取得

    注意: Catch2等のテスト専用ライブラリも含まれますが、
    これはVSCodeのIntelliSenseでテストファイルの編集時に
    ヘッダー補完を提供するために必要です。
    実際のビルドでは、CMakeLists.txtの条件によって
    適切に制御されます。
    """
    conan_home = get_conan_home()
    include_paths = []

    # 基本的なConanパッケージパスパターン
    base_patterns = [
        f"{conan_home}/p/*/p/include",  # パッケージのメインinclude
        f"{conan_home}/p/*/s/src/include",  # ソースビルドのinclude
        f"{conan_home}/p/*/s/*/include",  # その他のinclude
        f"{conan_home}/p/b/*/p/include",  # ビルド成果物のinclude
        f"{conan_home}/p/b/*/b/src/include",  # ビルド中のsrcのinclude
    ]

    excluded_paths = []
    for pattern in base_patterns:
        paths = glob.glob(pattern)
        for path in paths:
            # 実際にヘッダーファイルが存在するディレクトリのみ追加
            if has_header_files(path):
                include_paths.append(path)
            else:
                excluded_paths.append(path)

    # 重複除去
    include_paths = list(dict.fromkeys(include_paths))

    return include_paths


def has_header_files(directory):
    """ディレクトリにヘッダーファイルが存在するかチェック"""
    if not os.path.exists(directory):
        return False

    header_extensions = [".h", ".hpp", ".hxx", ".h++", ".hh"]

    # ディレクトリ直下とサブディレクトリ（1階層）をチェック
    for root, dirs, files in os.walk(directory):
        # 深すぎる階層は避ける（パフォーマンス対策）
        if root != directory:
            dirs[:] = [d for d in dirs if not d.startswith(".")][
                :3
            ]  # 最大3つのサブディレクトリ

        for file in files:
            if any(file.lower().endswith(ext) for ext in header_extensions):
                return True

        # 1階層のみチェック
        if root != directory:
            dirs.clear()

    return False


def generate_settings_json():
    """settings.jsonを生成"""
    is_windows = platform.system() == "Windows"

    # 共通のフォーマット設定
    common_format_settings = {
        # ファイル関連
        "files.associations": {
            "*.hpp": "cpp",
            "*.cpp": "cpp",
            "*.h": "cpp",
            "*.c": "cpp",
            "string": "cpp",
            "iostream": "cpp",
            "vector": "cpp",
            "memory": "cpp",
        },
        "files.trimTrailingWhitespace": True,
        "files.insertFinalNewline": True,
        "files.trimFinalNewlines": True,
        # エディタ設定
        "editor.formatOnSave": True,
        "editor.formatOnType": True,
        "editor.formatOnPaste": True,
        "editor.insertSpaces": True,
        "editor.tabSize": 2,
        "editor.detectIndentation": False,
        "editor.rulers": [80, 120],
        "editor.wordWrap": "bounded",
        "editor.wordWrapColumn": 120,
        "editor.renderWhitespace": "boundary",
        "editor.bracketPairColorization.enabled": True,
        "editor.guides.bracketPairs": "active",
        # C/C++フォーマット設定
        "C_Cpp.formatting": "clangFormat",
        "C_Cpp.clang_format_style": "file",
        "C_Cpp.clang_format_fallbackStyle": "{ BasedOnStyle: Google, IndentWidth: 2, ColumnLimit: 120, AlignAfterOpenBracket: Align, AllowShortFunctionsOnASingleLine: Empty, AllowShortIfStatementsOnASingleLine: false, AllowShortLoopsOnASingleLine: false, BinPackArguments: false, BinPackParameters: false, BreakBeforeBraces: Attach, BreakBeforeBinaryOperators: NonAssignment, AlignOperands: Align, ContinuationIndentWidth: 4, BreakConstructorInitializers: BeforeColon, ConstructorInitializerAllOnOneLineOrOnePerLine: true, IndentCaseLabels: true, KeepEmptyLinesAtTheStartOfBlocks: false, NamespaceIndentation: None, SpaceAfterCStyleCast: false, SpaceBeforeParens: ControlStatements, SpacesInParentheses: false, Standard: c++20 }",
        # C/C++基本設定
        "C_Cpp.intelliSenseEngine": "default",
        "C_Cpp.errorSquiggles": "enabled",
        "C_Cpp.autocomplete": "default",
        "C_Cpp.suggestSnippets": True,
        "C_Cpp.workspaceParsingPriority": "highest",
        "C_Cpp.enhancedColorization": "enabled",
        # CMake設定
        "cmake.configureOnOpen": False,
        "cmake.showOptionsMovedNotification": False,
        # その他のフォーマッタ設定
        "[cpp]": {
            "editor.defaultFormatter": "ms-vscode.cpptools",
            "editor.formatOnSave": True,
            "editor.tabSize": 2,
        },
        "[c]": {
            "editor.defaultFormatter": "ms-vscode.cpptools",
            "editor.formatOnSave": True,
            "editor.tabSize": 2,
        },
        "[json]": {
            "editor.defaultFormatter": "esbenp.prettier-vscode",
            "editor.formatOnSave": True,
            "editor.tabSize": 2,
        },
        "[python]": {
            "editor.defaultFormatter": "ms-python.black-formatter",
            "editor.formatOnSave": True,
            "editor.tabSize": 4,
        },
        "[markdown]": {
            "editor.formatOnSave": True,
            "editor.tabSize": 2,
            "editor.wordWrap": "on",
        },
    }

    if is_windows:
        # GLMのインクルードパスを取得
        glm_include_path = get_glm_include_path()

        # 基本のインクルードパス
        base_include_paths = [
            "include",
            "C:/Users/cpjvm/.conan2/p/vulka*/p/include",
            "C:/Users/cpjvm/.conan2/p/b/spirv*/p/include",
            "C:/Users/cpjvm/.conan2/p/b/glsla*/b/src",
        ]

        # GLMパスが見つかった場合は追加
        if glm_include_path:
            base_include_paths.append(glm_include_path)

        # Windows/MSVC設定
        settings = {
            **common_format_settings,
            "C_Cpp.default.cppStandard": "c++20",
            "C_Cpp.default.cStandard": "c17",
            "C_Cpp.default.compilerPath": "cl.exe",
            "C_Cpp.default.intelliSenseMode": "windows-msvc-x64",
            "C_Cpp.default.includePath": base_include_paths,
        }
    else:
        # Linux/macOS/Clang設定
        settings = {
            **common_format_settings,
            "C_Cpp.default.cppStandard": "c++20",
            "C_Cpp.default.cStandard": "c23",
            "C_Cpp.default.compilerPath": "clang++",
            "C_Cpp.default.intelliSenseMode": "clang-x64",
            "C_Cpp.codeAnalysis.clangTidy.enabled": True,
            "C_Cpp.codeAnalysis.clangTidy.args": [
                "--checks=-*,clang-analyzer-*,bugprone-*,performance-*,readability-*,modernize-*"
            ],
            "C_Cpp.codeAnalysis.clangTidy.checks.disabled": [
                "modernize-concat-nested-namespaces"
            ],
        }

    return settings


def generate_tasks_json():
    """tasks.jsonを生成 (build.ps1に依存したシンプルな構成)"""
    is_windows = platform.system() == "Windows"

    if is_windows:
        # Windows用タスク - build.ps1を使用
        tasks = {
            "version": "2.0.0",
            "tasks": [
                {
                    "label": "🔨 Build (Release)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "build",
                        "-Configuration",
                        "Release",
                    ],
                    "group": {"kind": "build", "isDefault": True},
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🐛 Build (Debug)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "build",
                        "-Configuration",
                        "Debug",
                    ],
                    "group": "build",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "📚 Build Library (Release)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "lib",
                        "-Configuration",
                        "Release",
                    ],
                    "group": "build",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "📚 Build Library (Debug)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "lib",
                        "-Configuration",
                        "Debug",
                    ],
                    "group": "build",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🧪 Build Tests",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "tests",
                        "-Configuration",
                        "Release",
                    ],
                    "group": "test",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🚀 Run Example (Release)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "run",
                        "-Configuration",
                        "Release",
                    ],
                    "group": "test",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🚀 Run Example (Debug)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "run",
                        "-Configuration",
                        "Debug",
                    ],
                    "group": "test",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🧹 Clean",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-File",
                        "${workspaceFolder}/scripts/build.ps1",
                        "clean",
                    ],
                    "group": "build",
                    "problemMatcher": "$msCompile",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🎨 Format All C++ Files",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-Command",
                        'Get-ChildItem -Recurse -Include *.cpp,*.hpp,*.h,*.c,*.cc,*.cxx | ForEach-Object { Write-Host "Formatting: $($_.FullName)"; clang-format -i -style=file $_.FullName }',
                    ],
                    "group": "build",
                    "problemMatcher": [],
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
                {
                    "label": "🎨 Format Current File",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-ExecutionPolicy",
                        "Bypass",
                        "-Command",
                        "if ('${file}' -match '\\.(cpp|hpp|h|c|cc|cxx)$') { Write-Host 'Formatting: ${file}'; clang-format -i -style=file '${file}' } else { Write-Host 'Not a C++ file: ${file}' }",
                    ],
                    "group": "build",
                    "problemMatcher": [],
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                },
            ],
        }
    else:
        # Linux/macOS用タスク - build.shを使用（存在する場合）
        tasks = {
            "version": "2.0.0",
            "tasks": [
                {
                    "label": "🔨 Build (Shell)",
                    "type": "shell",
                    "command": "${workspaceFolder}/scripts/build.sh",
                    "args": [],
                    "group": {"kind": "build", "isDefault": True},
                    "problemMatcher": "$gcc",
                    "presentation": {
                        "echo": True,
                        "reveal": "always",
                        "focus": False,
                        "panel": "shared",
                    },
                    "options": {"cwd": "${workspaceFolder}"},
                }
            ],
        }

    return tasks


def generate_launch_json():
    """launch.jsonを生成 - Debug/Release両対応"""
    is_windows = platform.system() == "Windows"

    if is_windows:
        # Windows/MSVC用デバッグ設定
        launch = {
            "version": "0.2.0",
            "configurations": [
                # Debug設定
                {
                    "name": "🐛 Debug Tests (Debug)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/tests/Debug/tests.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🐛 Build (Debug)",
                },
                {
                    "name": "🐛 Debug Example (Debug)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Debug/basic_usage.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🐛 Build (Debug)",
                },
                # Release設定
                {
                    "name": "🚀 Debug Tests (Release)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/tests/Release/tests.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🔨 Build (Release)",
                },
                {
                    "name": "� Debug Example (Release)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Release/basic_usage.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🔨 Build (Release)",
                },
            ],
        }
    else:
        # Linux/macOS/Clang用デバッグ設定
        launch = {
            "version": "0.2.0",
            "configurations": [
                # Debug設定
                {
                    "name": "🐛 Debug Tests (Debug)",
                    "type": "cppdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/tests/Debug/tests",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "externalConsole": False,
                    "MIMode": "gdb",
                    "setupCommands": [
                        {
                            "description": "Enable pretty-printing for gdb",
                            "text": "-enable-pretty-printing",
                            "ignoreFailures": True,
                        }
                    ],
                    "preLaunchTask": "🐛 Build (Debug)",
                    "miDebuggerPath": "/usr/bin/gdb",
                },
                {
                    "name": "🐛 Debug Example (Debug)",
                    "type": "cppdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Debug/basic_usage",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "externalConsole": False,
                    "MIMode": "gdb",
                    "setupCommands": [
                        {
                            "description": "Enable pretty-printing for gdb",
                            "text": "-enable-pretty-printing",
                            "ignoreFailures": True,
                        }
                    ],
                    "preLaunchTask": "🐛 Build (Debug)",
                    "miDebuggerPath": "/usr/bin/gdb",
                },
                # Release設定
                {
                    "name": "🚀 Debug Tests (Release)",
                    "type": "cppdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/tests/Release/tests",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "externalConsole": False,
                    "MIMode": "gdb",
                    "setupCommands": [
                        {
                            "description": "Enable pretty-printing for gdb",
                            "text": "-enable-pretty-printing",
                            "ignoreFailures": True,
                        }
                    ],
                    "preLaunchTask": "🔨 Build (Release)",
                    "miDebuggerPath": "/usr/bin/gdb",
                },
                {
                    "name": "� Debug Example (Release)",
                    "type": "cppdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Release/basic_usage",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "externalConsole": False,
                    "MIMode": "gdb",
                    "setupCommands": [
                        {
                            "description": "Enable pretty-printing for gdb",
                            "text": "-enable-pretty-printing",
                            "ignoreFailures": True,
                        }
                    ],
                    "preLaunchTask": "🔨 Build (Release)",
                    "miDebuggerPath": "/usr/bin/gdb",
                },
            ],
        }

    return launch


def generate_extensions_json():
    """extensions.jsonを生成 (フォーマット拡張機能含む)"""
    extensions = {
        "recommendations": [
            # C/C++開発
            "ms-vscode.cpptools",
            "ms-vscode.cpptools-extension-pack",
            # Python開発
            "ms-python.python",
            "ms-python.vscode-pylance",
            "ms-python.black-formatter",
            # フォーマット・整形
            "esbenp.prettier-vscode",
            "bradlc.vscode-tailwindcss",
            "formulahendry.auto-rename-tag",
            # Git・バージョン管理
            "eamodio.gitlens",
            "mhutchie.git-graph",
            # AI・開発支援
            "github.copilot",
            "github.copilot-chat",
            # エディタ拡張
            "gruntfuggly.todo-tree",
            "shardulm94.trailing-spaces",
            "ms-vscode.hexeditor",
            "streetsidesoftware.code-spell-checker",
            # CMake・ビルド
            "twxs.cmake",
            "ms-vscode.cmake-tools",
            # ドキュメント
            "yzhang.markdown-all-in-one",
            "davidanson.vscode-markdownlint",
        ]
    }

    # プラットフォーム固有の拡張機能
    if platform.system() != "Windows":
        extensions["recommendations"].extend(
            [
                # Linux/macOS向け開発ツール
                "llvm-vs-code-extensions.vscode-clangd",
                "vadimcn.vscode-lldb",
                "webfreak.debug",
            ]
        )
    else:
        extensions["recommendations"].extend(
            [
                # Windows向け開発ツール
                "ms-vscode.powershell"
            ]
        )

    return extensions


def generate_clang_format():
    """
    .clang-formatファイルを生成
    C/C++コードの統一的なフォーマット設定
    """
    clang_format_config = """---
# Pandolabo プロジェクト用 clang-format 設定
# Google スタイルをベースにプロジェクト固有の調整を追加

BasedOnStyle: Google
Language: Cpp

# インデント設定
IndentWidth: 2
TabWidth: 2
UseTab: Never
IndentCaseLabels: true
IndentPPDirectives: BeforeHash
IndentRequires: true

# 行幅設定
ColumnLimit: 120

# ブレース設定
BreakBeforeBraces: Attach
BraceWrapping:
  AfterCaseLabel: false
  AfterClass: false
  AfterControlStatement: Never
  AfterEnum: false
  AfterFunction: false
  AfterNamespace: false
  AfterStruct: false
  AfterUnion: false
  BeforeCatch: false
  BeforeElse: false

# 関数・メソッド設定
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterReturnType: None

# パラメータ設定
BinPackArguments: false
BinPackParameters: false
AlignAfterOpenBracket: Align
AllowAllParametersOfDeclarationOnNextLine: false

# 二項演算子の改行設定（パイプライン演算子対応）
BreakBeforeBinaryOperators: NonAssignment
AlignOperands: Align
ContinuationIndentWidth: 4

# コンストラクタ初期化設定
BreakConstructorInitializers: BeforeColon
ConstructorInitializerAllOnOneLineOrOnePerLine: true
ConstructorInitializerIndentWidth: 4

# スペース設定
SpaceAfterCStyleCast: false
SpaceBeforeParens: ControlStatements
SpacesInParentheses: false
SpacesInSquareBrackets: false
SpaceInEmptyParentheses: false

# その他の設定
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
NamespaceIndentation: None
SortIncludes: true
SortUsingDeclarations: true

# C++標準設定 (clang-format 20.1.8 では c++20 が最新サポート)
Standard: c++20

# アクセス修飾子のインデント
AccessModifierOffset: -1

# ポインタ・参照の位置
PointerAlignment: Left
ReferenceAlignment: Left

# コメント設定
ReflowComments: true
CommentPragmas: '^ IWYU pragma:'

# マクロ設定
MacroBlockBegin: ''
MacroBlockEnd: ''

# その他
AllowShortBlocksOnASingleLine: false
AllowShortCaseLabelsOnASingleLine: false
DerivePointerAlignment: false
ExperimentalAutoDetectBinPacking: false
FixNamespaceComments: true
...
"""

    workspace_folder = Path.cwd()
    clang_format_file = workspace_folder / ".clang-format"

    with open(clang_format_file, "w", encoding="utf-8") as f:
        f.write(clang_format_config)

    print(f"✅ Generated {clang_format_file}")
    return clang_format_config


def generate_cpp_properties(debug_mode=True):
    """
    c_cpp_properties.jsonを生成
    Conanのインクルードパスを直接指定してCMake Toolsに依存しない設定

    Args:
        debug_mode (bool): Trueの場合、デバッグ用マクロ(_DEBUG)を追加
    """
    workspace_folder = Path.cwd()

    # compile_commands.jsonの存在確認とプラットフォーム判定
    compile_commands_path = workspace_folder / "build" / "compile_commands.json"
    compile_commands_exists = compile_commands_path.exists()
    is_windows = platform.system() == "Windows"

    # 基本のインクルードパス
    include_paths = ["${workspaceFolder}/include", "${workspaceFolder}/src"]

    # Conanのインクルードパスを追加
    conan_include_paths = get_conan_include_paths()
    include_paths.extend(conan_include_paths)

    # 基本のdefines
    base_defines = ["VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"]

    # プラットフォーム固有のdefines
    if is_windows:
        platform_defines = ["_WIN32", "_UNICODE", "UNICODE"]
    else:
        platform_defines = ["__linux__"]

    # デバッグモード用のdefines
    debug_defines = ["_DEBUG"] if debug_mode else []

    # 全てのdefinesを結合
    all_defines = platform_defines + base_defines + debug_defines

    config = {
        "configurations": [
            {
                "name": "Win32" if is_windows else "Linux",
                "includePath": include_paths,
                "defines": all_defines,
                "cStandard": "c17",
                "cppStandard": "c++23",
            }
        ],
        "version": 4,
    }

    # プラットフォーム別設定
    if is_windows:
        config["configurations"][0].update(
            {"compilerPath": "cl.exe", "intelliSenseMode": "windows-msvc-x64"}
        )
    else:
        config["configurations"][0].update(
            {"compilerPath": "/usr/bin/clang++", "intelliSenseMode": "linux-clang-x64"}
        )

    # compile_commands.jsonが存在する場合は追加（全プラットフォーム対応）
    if compile_commands_exists:
        config["configurations"][0][
            "compileCommands"
        ] = "${workspaceFolder}/build/compile_commands.json"

    # .vscodeディレクトリを作成
    vscode_dir = workspace_folder / ".vscode"
    vscode_dir.mkdir(exist_ok=True)

    # ファイルに書き出し
    cpp_properties_file = vscode_dir / "c_cpp_properties.json"
    with open(cpp_properties_file, "w", encoding="utf-8") as f:
        json.dump(config, f, indent=2)

    print(f"✅ Generated {cpp_properties_file}")

    # プラットフォーム別のメッセージ
    if is_windows:
        print("🪟 Windows/MSVC detected - using CMake Tools configurationProvider")
        print("   All dependency paths automatically detected from CMake")
    else:
        # compile_commands.jsonの状態を表示（Linux/macOS）
        if compile_commands_exists:
            print(
                "📋 compile_commands.json found - IntelliSense will use precise build settings"
            )
        else:
            print(
                "⚠️  compile_commands.json not found - run 'cmake -S . -B build -G Ninja' to generate it"
            )

    return config


def generate_all_vscode_configs(debug_mode=True):
    """すべてのVSCode設定ファイルを生成

    Args:
        debug_mode (bool): Trueの場合、デバッグ用マクロ(_DEBUG)を追加
    """
    workspace_folder = Path.cwd()
    vscode_dir = workspace_folder / ".vscode"
    vscode_dir.mkdir(exist_ok=True)

    is_windows = platform.system() == "Windows"
    platform_name = "Windows/MSVC" if is_windows else f"{platform.system()}/Clang"

    print(f"🎯 Generating VSCode configuration for {platform_name}...")

    # 各設定ファイルを生成
    configs = {
        "settings.json": generate_settings_json(),
        "tasks.json": generate_tasks_json(),
        "launch.json": generate_launch_json(),
        "extensions.json": generate_extensions_json(),
    }

    for filename, config in configs.items():
        file_path = vscode_dir / filename
        with open(file_path, "w", encoding="utf-8") as f:
            json.dump(config, f, indent=2)
        print(f"✅ Generated {filename}")

    # c_cpp_properties.jsonも生成
    generate_cpp_properties(debug_mode)

    # .clang-formatファイルも生成
    generate_clang_format()

    print(f"\n🎉 All VSCode configurations generated for {platform_name}!")
    print("   - Code formatting with clang-format (Google style base)")
    print("   - Auto-format on save, type, and paste enabled")
    print("   - Enhanced file associations and editor settings")

    if debug_mode:
        print("   - Debug mode enabled with _DEBUG macro")
    else:
        print("   - Release mode (no debug macros)")

    if is_windows:
        print("   - Build tasks use build.ps1 PowerShell script")
        print("   - Optimized for MSVC compiler with direct Conan include paths")
        print(
            "   - Uses c_cpp_properties.json for IntelliSense (no CMake Tools dependency)"
        )
    else:
        print("   - Build tasks use build.sh shell script")
        print("   - Optimized for Clang compiler with direct Conan include paths")
        print("   - Includes clang-tidy integration and compile_commands.json support")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate VS Code configuration files for Pandolabo project"
    )
    parser.add_argument(
        "--release",
        action="store_true",
        help="Generate configuration for Release mode (default: Debug mode)",
    )

    args = parser.parse_args()
    debug_mode = not args.release  # --releaseが指定されていない場合はDebugモード

    generate_all_vscode_configs(debug_mode)
