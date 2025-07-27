#!/bin/bash
#
# Pandolabo ビルドスクリプト (Bash版)
#
# 機能:
# - プラットフォーム検出 + 自動環境構築 + ビルド実行
# - Python仮想環境の自動作成 (.venv)
# - 必要なパッケージの自動インストール (requirements.txt)
# - Conan 2.x パッケージマネージャーの自動セットアップ
# - インタラクティブモードでの対話的操作
# - Debug/Release ビルド設定切り替え対応
# - C++ファイル一括フォーマット機能
# - 複数のビルドオプション (setup, build, lib, examples, tests, clean, rebuild, run, all, vscode, format)
# - プラットフォーム対応 (Linux, macOS)
#
# 使用方法:
#   ./build.sh [COMMAND] [OPTIONS]
#
# オプション:
#   -c, --config CONFIG    ビルド設定 (Debug, Release, RelWithDebInfo, MinSizeRel)
#   -h, --help            ヘルプ表示
#
# 例:
#   ./build.sh build                         # プロジェクトビルド (Debug)
#   ./build.sh build --config Release        # リリースビルド
#   ./build.sh format                        # C++ファイル一括フォーマット
#   ./build.sh examples --config Debug       # サンプル実行 (Debug)
#   ./build.sh deps                          # 依存関係更新
#   ./build.sh vscode                        # VSCode設定生成
#

# スクリプトディレクトリを取得
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
PYTHON_SCRIPT="$SCRIPT_DIR/build.py"

# カラー出力関数
print_colored() {
    local color=$1
    local message=$2

    case $color in
        "red")    echo -e "\033[31m$message\033[0m" ;;
        "green")  echo -e "\033[32m$message\033[0m" ;;
        "yellow") echo -e "\033[33m$message\033[0m" ;;
        "blue")   echo -e "\033[34m$message\033[0m" ;;
        "cyan")   echo -e "\033[36m$message\033[0m" ;;
        "gray")   echo -e "\033[90m$message\033[0m" ;;
        *)        echo "$message" ;;
    esac
}

# メニュー表示
show_menu() {
    local config=${1:-"Debug"}
    clear
    print_colored "cyan" "🔨 Pandolabo ビルドスクリプト (Bash版)"
    print_colored "cyan" "=========================================="
    echo ""
    print_colored "yellow" "現在の設定: $config"
    echo ""
    print_colored "green" "利用可能なコマンド:"
    echo "  1. build       - プロジェクトをビルド"
    echo "  2. clean       - ビルドファイルをクリア"
    echo "  3. examples    - サンプルを実行"
    echo "  4. deps        - 依存関係を更新"
    echo "  5. conan-install - Conanパッケージをインストール"
    echo "  6. vscode      - VSCode設定を生成"
    echo "  7. format      - C++ファイルをフォーマット"
    echo "  8. config      - ビルド設定を変更"
    echo "  9. exit        - 終了"
    echo ""
    print_colored "cyan" "設定変更は '$0 --config <Config>' または メニューの8番で可能です"
}

# ユーザー選択取得
get_user_choice() {
    while true; do
        read -p "選択してください [1-9]: " choice
        case $choice in
            1) echo "build"; return 0 ;;
            2) echo "clean"; return 0 ;;
            3) echo "examples"; return 0 ;;
            4) echo "deps"; return 0 ;;
            5) echo "conan-install"; return 0 ;;
            6) echo "vscode"; return 0 ;;
            7) echo "format"; return 0 ;;
            8)
                echo ""
                print_colored "yellow" "利用可能な設定:"
                echo "  1. Debug"
                echo "  2. Release"
                echo "  3. RelWithDebInfo"
                echo "  4. MinSizeRel"
                read -p "設定を選択してください [1-4]: " config_choice
                case $config_choice in
                    1) CONFIGURATION="Debug" ;;
                    2) CONFIGURATION="Release" ;;
                    3) CONFIGURATION="RelWithDebInfo" ;;
                    4) CONFIGURATION="MinSizeRel" ;;
                    *) print_colored "red" "❌ 無効な選択です"; continue ;;
                esac
                print_colored "green" "✅ 設定を $CONFIGURATION に変更しました"
                read -p "Enterキーを押して続行..." -r
                return 1  # メニューを再表示
                ;;
            9) echo "exit"; return 0 ;;
            *) print_colored "red" "❌ 無効な選択です。1-9の数字を入力してください。" ;;
        esac
    done
}

# C++ファイル一括フォーマット
format_cpp_files() {
    print_colored "green" "🎨 C++ファイルをフォーマット中..."

    cd "$PROJECT_ROOT" || {
        print_colored "red" "❌ プロジェクトルートに移動できません"
        return 1
    }

    # clang-formatの存在確認
    if ! command -v clang-format >/dev/null 2>&1; then
        print_colored "red" "❌ clang-formatが見つかりません。インストールしてください。"
        print_colored "cyan" "   Ubuntu/Debian: sudo apt install clang-format"
        print_colored "cyan" "   macOS (Homebrew): brew install clang-format"
        return 1
    fi

    local clang_format_version
    clang_format_version=$(clang-format --version)
    print_colored "green" "✅ clang-formatが見つかりました: $clang_format_version"

    # .clang-formatファイルの存在確認
    local clang_format_file="$PROJECT_ROOT/.clang-format"
    if [ ! -f "$clang_format_file" ]; then
        print_colored "yellow" "⚠️  .clang-formatファイルが見つかりません。VSCode設定を再生成してください。"
        print_colored "cyan" "   実行: ./scripts/build.sh vscode"
        return 1
    fi

    # C++ファイルを検索してフォーマット
    local cpp_files
    mapfile -t cpp_files < <(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cxx" \) ! -path "./build/*" ! -path "./.venv/*")

    if [ ${#cpp_files[@]} -eq 0 ]; then
        print_colored "yellow" "🔍 フォーマット対象のC++ファイルが見つかりません。"
        return 0
    fi

    print_colored "cyan" "📁 ${#cpp_files[@]} 個のC++ファイルを処理中..."

    local success_count=0
    local error_count=0

    for file in "${cpp_files[@]}"; do
        print_colored "gray" "  🔄 $(basename "$file")"
        if clang-format -i -style=file "$file"; then
            ((success_count++))
        else
            ((error_count++))
            print_colored "red" "    ❌ フォーマットに失敗: $(basename "$file")"
        fi
    done

    echo ""
    print_colored "green" "🎉 フォーマット完了!"
    print_colored "green" "   ✅ 成功: $success_count ファイル"
    if [ $error_count -gt 0 ]; then
        print_colored "red" "   ❌ 失敗: $error_count ファイル"
        return 1
    fi
    return 0
}

# Pythonビルドスクリプトを呼び出し
invoke_python_build() {
    local build_command=$1
    local config=${2:-$CONFIGURATION}

    print_colored "green" "🔧 コマンド: $build_command (設定: $config)"

    # コマンドごとの処理
    if [ "$build_command" = "format" ]; then
        format_cpp_files
        return $?
    fi

    if [ "$build_command" = "vscode" ]; then
        print_colored "green" "🎯 VSCode設定一式を生成中..."
        print_colored "cyan" "   設定モード: $CONFIGURATION"
        local vscode_script="$SCRIPT_DIR/generate_vscode_config.py"

        # WSL環境でのパス変換対応
        if command -v wslpath >/dev/null 2>&1; then
            vscode_script=$(wslpath -u "$(wslpath -w "$vscode_script")")
        fi

        if [ -f "$vscode_script" ]; then
            # 仮想環境のPythonを試す
            local venv_python="$PROJECT_ROOT/.venv/bin/python"
            # Windowsの場合のパスも確認
            if [ ! -f "$venv_python" ]; then
                venv_python="$PROJECT_ROOT/.venv/Scripts/python.exe"
            fi

            # Configuration に応じて引数を決定
            local vscode_args=("$vscode_script")
            if [ "$CONFIGURATION" != "Debug" ]; then
                vscode_args+=("--release")
            fi

            if [ -f "$venv_python" ]; then
                "$venv_python" "${vscode_args[@]}"
                local exit_code=$?
                if [ $exit_code -eq 0 ]; then
                    print_colored "green" "✅ VSCode設定一式が更新されました"
                else
                    print_colored "red" "❌ VSCode設定の更新に失敗しました"
                fi
                return $exit_code
            else
                # Fallback to system Python
                python "${vscode_args[@]}"
                local exit_code=$?
                if [ $exit_code -eq 0 ]; then
                    print_colored "green" "✅ VSCode設定一式が更新されました"
                else
                    print_colored "red" "❌ VSCode設定の更新に失敗しました"
                fi
                return $exit_code
            fi
        else
            print_colored "red" "❌ VSCode設定スクリプトが見つかりません: $vscode_script"
            return 1
        fi
    fi

    # Python仮想環境のパスを確認
    local venv_python="$PROJECT_ROOT/.venv/bin/python"
    local venv_dir="$PROJECT_ROOT/.venv"

    # Windowsの場合のパスも確認
    if [ ! -f "$venv_python" ]; then
        venv_python="$PROJECT_ROOT/.venv/Scripts/python.exe"
    fi

    if [ ! -f "$venv_python" ]; then
        print_colored "red" "❌ Python仮想環境が見つかりません: $venv_python"
        print_colored "yellow" "🔧 Python仮想環境を自動作成します..."

        # Pythonコマンドの確認
        if command -v python3 >/dev/null 2>&1; then
            local python_cmd="python3"
            print_colored "green" "✅ Python3 が見つかりました: $(which python3)"
        elif command -v python >/dev/null 2>&1; then
            local python_cmd="python"
            print_colored "green" "✅ Python が見つかりました: $(which python)"
        else
            print_colored "red" "❌ Python が見つかりません。Pythonをインストールしてください。"
            return 1
        fi

        # 仮想環境作成
        print_colored "cyan" "実行コマンド: $python_cmd -m venv $venv_dir"
        if "$python_cmd" -m venv "$venv_dir"; then
            print_colored "green" "✅ Python仮想環境が正常に作成されました。"
        else
            print_colored "red" "❌ 仮想環境の作成に失敗しました。"
            return 1
        fi

        # パスを再設定
        venv_python="$PROJECT_ROOT/.venv/bin/python"
        if [ ! -f "$venv_python" ]; then
            venv_python="$PROJECT_ROOT/.venv/Scripts/python.exe"
        fi

        if [ ! -f "$venv_python" ]; then
            print_colored "red" "❌ 仮想環境の作成後もPythonが見つかりません。"
            return 1
        fi

        # 必要なパッケージをインストール
        print_colored "yellow" "📦 必要なパッケージをインストール中..."
        if "$venv_python" -m pip install --upgrade pip; then
            print_colored "green" "✅ pip が更新されました。"
        fi

        # requirements.txtがあればインストール
        local requirements_file="$PROJECT_ROOT/requirements.txt"
        if [ -f "$requirements_file" ]; then
            print_colored "yellow" "📋 requirements.txt からパッケージをインストール中..."
            if "$venv_python" -m pip install -r "$requirements_file"; then
                print_colored "green" "✅ requirements.txt のパッケージがインストールされました。"
            else
                print_colored "yellow" "⚠️  一部のパッケージのインストールに失敗しました。"
            fi
        fi

        # conanfile.txtがあればConan 2.x系をセットアップ
        local conan_file="$PROJECT_ROOT/conanfile.txt"
        if [ -f "$conan_file" ]; then
            print_colored "yellow" "🔧 Conan 2.x パッケージマネージャーをインストール中..."
            if "$venv_python" -m pip install "conan>=2.0"; then
                print_colored "green" "✅ Conan 2.x がインストールされました。"

                # Conan実行ファイルのパスを取得
                local conan_exe
                if [ -f "$PROJECT_ROOT/.venv/bin/conan" ]; then
                    conan_exe="$PROJECT_ROOT/.venv/bin/conan"
                elif [ -f "$PROJECT_ROOT/.venv/Scripts/conan.exe" ]; then
                    conan_exe="$PROJECT_ROOT/.venv/Scripts/conan.exe"
                fi

                if [ -n "$conan_exe" ] && [ -f "$conan_exe" ]; then
                    # Conanプロファイルの設定
                    print_colored "yellow" "⚙️  Conan プロファイルを設定中..."
                    cd "$PROJECT_ROOT" || return 1
                    if "$conan_exe" profile detect --force; then
                        print_colored "green" "✅ Conan プロファイルが設定されました。"

                        # Conanパッケージの依存関係をインストール
                        print_colored "yellow" "📦 Conan パッケージをインストール中..."
                        if "$conan_exe" install . --output-folder=build --build=missing; then
                            print_colored "green" "✅ Conan パッケージがインストールされました。"
                        else
                            print_colored "yellow" "⚠️  Conan パッケージのインストールに失敗しました。後で手動でインストールしてください。"
                            print_colored "cyan" "手動実行コマンド: $conan_exe install . --output-folder=build --build=missing"
                        fi
                    else
                        print_colored "yellow" "⚠️  Conan プロファイルの設定に失敗しました。"
                        print_colored "cyan" "手動実行コマンド: $conan_exe profile detect --force"
                    fi
                else
                    print_colored "yellow" "⚠️  Conan実行ファイルが見つかりません。"
                fi
            else
                print_colored "yellow" "⚠️  Conan のインストールに失敗しました。"
                print_colored "cyan" "手動実行コマンド: $venv_python -m pip install 'conan>=2.0'"
            fi
        fi
    fi

    if [ ! -f "$PYTHON_SCRIPT" ]; then
        print_colored "red" "❌ ビルドスクリプトが見つかりません: $PYTHON_SCRIPT"
        return 1
    fi

    # Pythonスクリプト実行
    echo ""
    print_colored "green" "🐍 Python ビルドスクリプトを実行中..."
    print_colored "cyan" "実行コマンド: $venv_python $PYTHON_SCRIPT $build_command --config $config"

    # バッファリングを無効にしてリアルタイム出力
    PYTHONUNBUFFERED=1 "$venv_python" "$PYTHON_SCRIPT" "$build_command" --config "$config"
    local exit_code=$?

    print_colored "cyan" "終了コード: $exit_code"
    return $exit_code
}

# メイン処理
main() {
    local config="Debug"
    local action=""
    local interactive=true

    # 引数解析
    while [ $# -gt 0 ]; do
        case $1 in
            -c|--config)
                if [ -n "$2" ] && [ "${2:0:1}" != "-" ]; then
                    config="$2"
                    shift 2
                else
                    print_colored "red" "❌ エラー: --config には値が必要です"
                    exit 1
                fi
                ;;
            --config=*)
                config="${1#*=}"
                if [ -z "$config" ]; then
                    print_colored "red" "❌ エラー: --config には値が必要です"
                    exit 1
                fi
                shift
                ;;
            build|clean|examples|deps|conan-install|vscode|format)
                action="$1"
                interactive=false
                shift
                ;;
            -h|--help)
                print_colored "cyan" "使用法: $0 [オプション] [アクション]"
                echo ""
                print_colored "yellow" "オプション:"
                echo "  -c, --config CONFIG   ビルド設定 (Debug, Release, RelWithDebInfo, MinSizeRel)"
                echo "  -h, --help           このヘルプを表示"
                echo ""
                print_colored "yellow" "アクション:"
                echo "  build               プロジェクトをビルド"
                echo "  clean               ビルドファイルをクリア"
                echo "  examples            サンプルを実行"
                echo "  deps                依存関係を更新"
                echo "  conan-install       Conanパッケージをインストール"
                echo "  vscode              VSCode設定を生成"
                echo "  format              C++ファイルをフォーマット"
                echo ""
                print_colored "green" "例:"
                echo "  $0 --config Release build"
                echo "  $0 -c Debug examples"
                echo "  $0 format"
                exit 0
                ;;
            *)
                print_colored "red" "❌ 不明な引数: $1"
                print_colored "yellow" "ヘルプを表示するには: $0 --help"
                exit 1
                ;;
        esac
    done

    # 設定の検証
    case $config in
        Debug|Release|RelWithDebInfo|MinSizeRel)
            ;;
        *)
            print_colored "red" "❌ 無効な設定: $config"
            print_colored "yellow" "有効な設定: Debug, Release, RelWithDebInfo, MinSizeRel"
            exit 1
            ;;
    esac

    # グローバル設定を更新
    CONFIGURATION="$config"

    # アクション実行
    if [ "$interactive" = true ]; then
        # インタラクティブモード
        while true; do
            show_menu "$config"
            local selected_command
            selected_command=$(get_user_choice)

            if [ $? -eq 1 ]; then
                # 設定変更の場合、configを更新
                config="$CONFIGURATION"
                continue
            fi

            if [ "$selected_command" = "exit" ]; then
                break
            fi

            local result
            result=$(invoke_python_build "$selected_command" "$config")
            local exit_code=$?

            echo ""
            if [ $exit_code -eq 0 ]; then
                print_colored "green" "✅ コマンド '$selected_command' が正常に完了しました。"
            else
                print_colored "red" "❌ コマンド '$selected_command' が失敗しました。(終了コード: $exit_code)"
            fi

            echo ""
            read -p "Enterキーを押して続行..." -r
        done
    else
        # コマンドラインモード
        invoke_python_build "$action" "$config"
        exit $?
    fi

    print_colored "yellow" "👋 ビルドスクリプトを終了しました。"
}

# スクリプト実行
main "$@"
