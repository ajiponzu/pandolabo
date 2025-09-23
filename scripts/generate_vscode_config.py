#!/usr/bin/env python3
"""
VSCode用の設定ファイル一式を自動生成するスクリプト（Windows/MSVC専用）
"""

import os
import json
import glob
import subprocess
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
    except Exception:
        return os.path.expanduser("~/.conan2")


def get_glm_include_path():
    """GLMのインクルードパスを取得"""
    conan_home = get_conan_home()
    glm_pattern = f"{conan_home}/p/*glm*/p/include"
    for path in glob.glob(glm_pattern):
        if os.path.exists(os.path.join(path, "glm", "glm.hpp")):
            return path
    return None


def has_header_files(directory: str) -> bool:
    """ディレクトリにヘッダーファイルが存在するかチェック"""
    if not os.path.exists(directory):
        return False
    header_exts = (".h", ".hpp", ".hxx", ".hh", ".h++")
    for root, dirs, files in os.walk(directory):
        for f in files:
            if f.lower().endswith(header_exts):
                return True
        # 浅く探索
        if root != directory:
            break
    return False


def get_conan_include_paths():
    """Conanパッケージのインクルードパスを取得"""
    conan_home = get_conan_home()
    include_paths: list[str] = []
    patterns = [
        f"{conan_home}/p/*/p/include",
        f"{conan_home}/p/*/s/src/include",
        f"{conan_home}/p/*/s/*/include",
        f"{conan_home}/p/b/*/p/include",
        f"{conan_home}/p/b/*/b/src/include",
    ]
    for pat in patterns:
        for path in glob.glob(pat):
            if has_header_files(path):
                include_paths.append(path)
    # 重複除去
    dedup = []
    seen = set()
    for p in include_paths:
        if p not in seen:
            seen.add(p)
            dedup.append(p)
    return dedup


def generate_settings_json():
    """settings.jsonを生成（Windows/MSVC 固定）"""
    common = {
        "files.associations": {
            "*.hpp": "cpp",
            "*.cpp": "cpp",
            "*.h": "cpp",
            "*.c": "cpp",
            "string": "cpp",
            "iostream": "cpp",
            "vector": "cpp",
            "memory": "cpp",
            "print": "cpp",
        },
        "files.trimTrailingWhitespace": True,
        "files.insertFinalNewline": True,
        "files.trimFinalNewlines": True,
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
    "C_Cpp.formatting": "clangFormat",
    "C_Cpp.clang_format_style": "file",
    "C_Cpp.clang_format_fallbackStyle": "{ BasedOnStyle: Google, IndentWidth: 2, ColumnLimit: 80 }",
        "cmake.configureOnOpen": False,
        "cmake.showOptionsMovedNotification": False,
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
    common.update(
        {
            "C_Cpp.default.cppStandard": "c++20",
            "C_Cpp.default.cStandard": "c17",
            "C_Cpp.default.compilerPath": "cl.exe",
            "C_Cpp.default.intelliSenseMode": "windows-msvc-x64",
            # Enable IntelliSense so preprocessor conditions reflect current defines
            "C_Cpp.intelliSenseEngine": "Default",
        }
    )

    return common


def generate_tasks_json():
    """tasks.jsonを生成（Windowsは build.ps1 を使用）"""
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
                    "label": "🧩 Build Example (Release)",
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
                        "-Example",
                        "${input:examplePicker}",
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
                    "label": "🧩 Build Example (Debug)",
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
                        "-Example",
                        "${input:examplePicker}",
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
                    "label": "🧪 Run Tests (Release)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-NoProfile",
                        "-ExecutionPolicy",
                        "Bypass",
                        "-Command",
                        '& "${workspaceFolder}/build/tests/Release/tests.exe" ${input:catchFilter}',
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
                    "label": "🧪 Run Tests (Debug)",
                    "type": "shell",
                    "command": "powershell.exe",
                    "args": [
                        "-NoProfile",
                        "-ExecutionPolicy",
                        "Bypass",
                        "-Command",
                        '& "${workspaceFolder}/build/tests/Debug/tests.exe" ${input:catchFilter}',
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
                        "-Example",
                        "${input:examplePicker}",
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
                        "-Example",
                        "${input:examplePicker}",
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
                        "if ('${file}' -match \\.(cpp|hpp|h|c|cc|cxx)$) { Write-Host 'Formatting: ${file}'; clang-format -i -style=file '${file}' } else { Write-Host 'Not a C++ file: ${file}' }",
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
            "inputs": [
                {
                    "id": "examplePicker",
                    "type": "pickString",
                    "description": "Choose example target to run",
                    "options": [
                        "example_basic_compute",
                        "example_basic_cube",
                        "example_simple_image",
                        "example_square",
                    ],
                    "default": "example_basic_cube",
                },
                {
                    "id": "catchFilter",
                    "type": "promptString",
                    "description": "Catch2 filter (leave empty for all)",
                    "default": "",
                },
            ],
        }
    return tasks


def generate_launch_json():
    """launch.jsonを生成 - Debug/Release両対応（Windows専用）"""
    return {
            "version": "0.2.0",
            "configurations": [
                {
                    "name": "🐛 Debug Tests (Debug)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/tests/Debug/tests.exe",
                    "args": ["${input:catchFilterLaunch}"],
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
                    "program": "${workspaceFolder}/build/examples/Debug/example_basic_cube.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🐛 Build (Debug)",
                },
                {
                    "name": "🚀 Debug Tests (Release)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/tests/Release/tests.exe",
                    "args": ["${input:catchFilterLaunch}"],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🔨 Build (Release)",
                },
                {
                    "name": "🚀 Debug Example (Release)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Release/example_basic_cube.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🔨 Build (Release)",
                },
                {
                    "name": "🐛 Debug Example (Pick - Debug)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Debug/${input:examplePickerLaunch}.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🐛 Build (Debug)",
                },
                {
                    "name": "🚀 Debug Example (Pick - Release)",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/examples/Release/${input:examplePickerLaunch}.exe",
                    "args": [],
                    "stopAtEntry": False,
                    "cwd": "${workspaceFolder}",
                    "environment": [],
                    "console": "integratedTerminal",
                    "preLaunchTask": "🔨 Build (Release)",
                },
            ],
            "inputs": [
                {
                    "id": "examplePickerLaunch",
                    "type": "pickString",
                    "description": "Choose example target to debug",
                    "options": [
                        "example_basic_compute",
                        "example_basic_cube",
                        "example_simple_image",
                        "example_square",
                    ],
                    "default": "example_basic_cube",
                },
                {
                    "id": "catchFilterLaunch",
                    "type": "promptString",
                    "description": "Catch2 filter (leave empty for all)",
                    "default": "",
                },
            ],
        }

def generate_extensions_json():
    """extensions.jsonを生成 (フォーマット拡張機能含む、Windows用)"""
    extensions = {
        "recommendations": [
            "ms-vscode.cpptools",
            "ms-vscode.cpptools-extension-pack",
            "ms-python.python",
            "ms-python.vscode-pylance",
            "ms-python.black-formatter",
            "esbenp.prettier-vscode",
            "bradlc.vscode-tailwindcss",
            "formulahendry.auto-rename-tag",
            "eamodio.gitlens",
            "mhutchie.git-graph",
            "github.copilot",
            "github.copilot-chat",
            "gruntfuggly.todo-tree",
            "shardulm94.trailing-spaces",
            "ms-vscode.hexeditor",
            "streetsidesoftware.code-spell-checker",
            "twxs.cmake",
            "ms-vscode.cmake-tools",
            "yzhang.markdown-all-in-one",
            "davidanson.vscode-markdownlint",
        ]
    }
    extensions["recommendations"].append("ms-vscode.powershell")
    return extensions


def generate_clang_format():
    """.clang-formatファイルを生成"""
    clang_format_config = """---
BasedOnStyle: Google
Language: Cpp
IndentWidth: 2
TabWidth: 2
UseTab: Never
IndentCaseLabels: true
IndentPPDirectives: BeforeHash
IndentRequires: true
ColumnLimit: 80
BreakBeforeBraces: Attach
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterReturnType: None
BinPackArguments: false
BinPackParameters: false
AlignAfterOpenBracket: Align
BreakBeforeBinaryOperators: NonAssignment
AlignOperands: Align
ContinuationIndentWidth: 4
BreakConstructorInitializers: BeforeColon
ConstructorInitializerAllOnOneLineOrOnePerLine: true
ConstructorInitializerIndentWidth: 4
SpaceAfterCStyleCast: false
SpaceBeforeParens: ControlStatements
SpacesInParentheses: false
SpacesInSquareBrackets: false
SpaceInEmptyParentheses: false
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
NamespaceIndentation: None
SortIncludes: true
SortUsingDeclarations: true
Standard: c++20
AccessModifierOffset: -1
PointerAlignment: Left
ReferenceAlignment: Left
ReflowComments: true
CommentPragmas: '^ IWYU pragma:'
MacroBlockBegin: ''
MacroBlockEnd: ''
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
    """c_cpp_properties.jsonを生成（Windows/MSVC 固定）"""
    workspace_folder = Path.cwd()
    # Prefer MSVC build compile_commands if present (typically off)
    compile_commands_path = workspace_folder / "build" / "compile_commands.json"
    compile_commands_exists = compile_commands_path.exists()

    include_paths = ["${workspaceFolder}/include", "${workspaceFolder}/src"]
    include_paths.extend(get_conan_include_paths())

    base_defines = ["VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"]
    platform_defines = ["_WIN32", "_UNICODE", "UNICODE"]
    # Make GPU_DEBUG visible to IntelliSense in Debug only
    debug_defines = ["_DEBUG", "GPU_DEBUG"] if debug_mode else []
    all_defines = platform_defines + base_defines + debug_defines

    config = {
        "configurations": [
            {
                "name": "Win32",
                "includePath": include_paths,
                "defines": all_defines,
                "cStandard": "c17",
                "cppStandard": "c++23",
            }
        ],
        "version": 4,
    }
    config["configurations"][0].update(
        {"compilerPath": "cl.exe", "intelliSenseMode": "windows-msvc-x64"}
    )

    if compile_commands_exists:
        config["configurations"][0][
            "compileCommands"
        ] = "${workspaceFolder}/build/compile_commands.json"

    vscode_dir = workspace_folder / ".vscode"
    vscode_dir.mkdir(exist_ok=True)
    cpp_properties_file = vscode_dir / "c_cpp_properties.json"
    with open(cpp_properties_file, "w", encoding="utf-8") as f:
        json.dump(config, f, indent=2)
    print(f"✅ Generated {cpp_properties_file}")
    return config


def generate_all_vscode_configs(debug_mode=True):
    workspace_folder = Path.cwd()
    vscode_dir = workspace_folder / ".vscode"
    vscode_dir.mkdir(exist_ok=True)

    platform_name = "Windows/MSVC"
    print(f"🎯 Generating VSCode configuration for {platform_name}...")

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

    generate_cpp_properties(debug_mode)
    generate_clang_format()

    print(f"\n🎉 All VSCode configurations generated for {platform_name}!")


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
    debug_mode = not args.release
    generate_all_vscode_configs(debug_mode)
