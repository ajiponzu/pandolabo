#!/usr/bin/env python3
"""
Pandolabo 統一ビルドスクリプト
プラットフォーム検出 + 自動環境構築 + ビルド実行
"""

import os
import sys
import platform
import subprocess
import shutil
import argparse
from pathlib import Path


class PandolaboBuilder:
    def __init__(self, config="Release"):
        self.is_windows = platform.system() == "Windows"
        self.venv_path = Path(".venv")
        self.build_path = Path("build")
        self.config = config

    def log(self, message, color="default"):
        colors = {
            "green": "\033[92m",
            "yellow": "\033[93m",
            "red": "\033[91m",
            "cyan": "\033[96m",
            "default": "\033[0m",
        }
        print(f"{colors.get(color, colors['default'])}{message}{colors['default']}")

    def run_command(self, cmd, check=False):
        """コマンド実行"""
        self.log(f"実行中: {' '.join(cmd) if isinstance(cmd, list) else cmd}", "cyan")
        if isinstance(cmd, str):
            cmd = cmd.split()
        try:
            result = subprocess.run(cmd, check=False, capture_output=False)
            success = result.returncode == 0
            if success:
                self.log(f"✅ コマンド成功 (終了コード: {result.returncode})", "green")
            else:
                self.log(f"❌ コマンド失敗 (終了コード: {result.returncode})", "red")
            return success
        except Exception as e:
            self.log(f"❌ コマンド実行エラー: {e}", "red")
            return False

    def setup_environment(self):
        """開発環境セットアップ"""
        self.log("🚀 Pandolabo 開発環境をセットアップ中...", "green")

        # 仮想環境確認
        if not self.venv_path.exists():
            self.log("❌ .venv仮想環境が見つかりません。", "red")
            self.log("手動で作成してください: python -m venv .venv", "yellow")
            return False

        # Conan確認
        conan_cmd = self._get_conan_command()
        if not conan_cmd:
            self.log("❌ Conanが見つかりません。", "red")
            return False

        # プラットフォーム別処理
        if self.is_windows:
            if self.config.lower() == "debug":
                profile = "config/conan/conanprofile_msvc_debug"
            else:
                profile = "config/conan/conanprofile_msvc"
        elif platform.system() == "Darwin":
            if self.config.lower() == "debug":
                profile = "config/conan/conanprofile_macos_debug"
            else:
                profile = "config/conan/conanprofile_macos"
        else:
            if self.config.lower() == "debug":
                profile = "config/conan/conanprofile_clang_debug"
            else:
                profile = "config/conan/conanprofile_clang"

        # Conan依存関係インストール
        self.log("🔗 Conan依存関係をインストール中...", "yellow")
        conan_install_cmd = [
            conan_cmd,
            "install",
            ".",
            f"--profile={profile}",
            "--build=missing",
            "--output-folder=conan",
        ]

        if not self.run_command(conan_install_cmd):
            return False

        # CMake設定
        self.log("⚙️ CMakeを設定中...", "yellow")
        cmake_cmd = [
            "cmake",
            "-DCMAKE_TOOLCHAIN_FILE=conan/build/generators/conan_toolchain.cmake",
            "-B",
            "build",
        ]

        if self.is_windows:
            cmake_cmd.extend(
                ["-G", "Visual Studio 17 2022", "-DCMAKE_POLICY_DEFAULT_CMP0091=NEW"]
            )
        else:
            cmake_cmd.extend(["-G", "Ninja"])

        return self.run_command(cmake_cmd)

    def _get_conan_command(self):
        """Conanコマンドパスを取得"""
        if self.is_windows:
            conan_path = self.venv_path / "Scripts" / "conan.exe"
        else:
            conan_path = self.venv_path / "bin" / "conan"

        return str(conan_path) if conan_path.exists() else None

    def build(self, target="all"):
        """ビルド実行"""
        self.log(f"🔨 {target} を{self.config}でビルド中...", "green")

        cmd = ["cmake", "--build", "build", "--config", self.config]

        if target != "all":
            cmd.extend(["--target", target])

        return self.run_command(cmd)

    def clean(self):
        """ビルドディレクトリをクリーン"""
        self.log("🧹 ビルドディレクトリをクリーン中...", "yellow")

        # クリーンするディレクトリとファイル
        directories_to_clean = [
            self.build_path,  # build
            Path("build_debug"),  # Debug用ビルドディレクトリ
            Path("conan"),  # Release用Conanディレクトリ
            Path("conan_debug"),  # Debug用Conanディレクトリ
        ]
        files_to_clean = [
            Path("CMakeUserPresets.json"),
            Path("conanbuild.bat"),
            Path("conanrun.bat"),
            Path("conandata.yml"),
            # Debug用Conanファイルも追加
            Path("conanbuild_debug.bat"),
            Path("conanrun_debug.bat"),
        ]

        # ディレクトリを削除
        for dir_path in directories_to_clean:
            if dir_path.exists():
                try:
                    if self.is_windows:
                        # Windowsの場合、読み取り専用属性を削除してから削除
                        import stat

                        def remove_readonly(func, path, _):
                            os.chmod(path, stat.S_IWRITE)
                            func(path)

                        shutil.rmtree(dir_path, onerror=remove_readonly)
                    else:
                        shutil.rmtree(dir_path)
                    self.log(f"✅ {dir_path}/ を削除しました", "green")
                except Exception as e:
                    self.log(f"❌ {dir_path} 削除エラー: {e}", "red")
                    return False
            else:
                self.log(f"ℹ️ {dir_path}/ は存在しません", "cyan")

        # ファイルを削除
        for file_path in files_to_clean:
            if file_path.exists():
                try:
                    file_path.unlink()
                    self.log(f"✅ {file_path} を削除しました", "green")
                except Exception as e:
                    self.log(f"❌ {file_path} 削除エラー: {e}", "red")
                    return False

        return True

    def clean_and_setup(self):
        """クリーンビルド（削除 + セットアップ）"""
        if self.clean():
            return self.setup_environment()
        return False

    def run_example(self):
        """Example実行"""
        if self.is_windows:
            example_path = f"build/examples/{self.config}/basic_usage.exe"
        else:
            example_path = "build/examples/basic_usage"

        if Path(example_path).exists():
            self.log(f"🚀 Example ({self.config}) を実行中...", "green")
            return self.run_command([example_path])
        else:
            self.log(
                f"❌ Example ({self.config}) が見つかりません。先にビルドしてください。",
                "red",
            )
            return False


def main():
    """メイン関数"""
    parser = argparse.ArgumentParser(
        description="Pandolabo 統一ビルドスクリプト",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用例:
  python build.py setup                        - 環境セットアップ
  python build.py build                        - 全体ビルド (Release)
  python build.py build --config Debug         - デバッグビルド
  python build.py lib --config RelWithDebInfo  - ライブラリのみ (RelWithDebInfo)
  python build.py examples --config MinSizeRel - Exampleのみ (MinSizeRel)
  python build.py tests                        - テストのみ
  python build.py clean                        - ビルドディレクトリ削除のみ
  python build.py rebuild                      - クリーン + セットアップ
  python build.py run --config Debug           - Example実行 (Debug)
  python build.py all --config Debug           - setup + build + run (Debug)
        """,
    )

    parser.add_argument(
        "command",
        choices=[
            "setup",
            "build",
            "lib",
            "examples",
            "tests",
            "clean",
            "rebuild",
            "run",
            "all",
        ],
        help="実行するコマンド",
    )

    parser.add_argument(
        "--config",
        "-c",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        default="Release",
        help="ビルド設定 (デフォルト: Release)",
    )

    args = parser.parse_args()

    print("🚀 Pandolabo ビルドスクリプト開始")
    print(f"📋 実行コマンド: {args.command}")
    print(f"⚙️  ビルド設定: {args.config}")

    builder = PandolaboBuilder(config=args.config)

    try:
        if args.command == "setup":
            success = builder.setup_environment()
        elif args.command == "build":
            success = builder.build()
        elif args.command == "lib":
            success = builder.build("pandolabo")
        elif args.command == "examples":
            success = builder.build("basic_usage")
        elif args.command == "tests":
            success = builder.build("tests")
        elif args.command == "clean":
            success = builder.clean()
        elif args.command == "rebuild":
            success = builder.clean_and_setup()
        elif args.command == "run":
            success = builder.run_example()
        elif args.command == "all":
            success = (
                builder.setup_environment()
                and builder.build()
                and builder.run_example()
            )
        else:
            builder.log(f"❓ 不明なコマンド: {args.command}", "red")
            return 1

        if success:
            builder.log(f"✅ 完了！ ({args.config})", "green")
            return 0
        else:
            builder.log(f"❌ 失敗しました。 ({args.config})", "red")
            return 1

    except Exception as e:
        builder.log(f"❌ エラー: {e}", "red")
        return 1


if __name__ == "__main__":
    sys.exit(main())
