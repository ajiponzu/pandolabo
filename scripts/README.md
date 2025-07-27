# Pandolabo ビルドスクリプト

## 📁 スクリプト構成

```
scripts/
├── build.py   - Python メインビルドスクリプト
├── build.ps1  - PowerShell ラッパー（Windows用）
└── build.sh   - Bash ラッパー（Linux/macOS用）

# プロジェクトルート用ランチャー
build.ps1      - PowerShell ランチャー
build.sh       - Bash ランチャー
```

## 🚀 使用方法

### Windows (PowerShell)

```powershell
# 直接実行
.\build.ps1 lib              # ライブラリビルド
.\build.ps1 all              # 全てビルド
.\build.ps1 -Interactive     # インタラクティブモード

# または詳細版
.\scripts\build.ps1 lib
```

### Linux/macOS (Bash)

```bash
# 直接実行
./build.sh lib               # ライブラリビルド
./build.sh all               # 全てビルド
./build.sh --interactive     # インタラクティブモード

# または詳細版
./scripts/build.sh lib
```

### Python直接実行

```bash
# 仮想環境から直接
.venv/Scripts/python.exe scripts/build.py lib    # Windows
.venv/bin/python scripts/build.py lib            # Linux/macOS
```

## 📋 利用可能なコマンド

| コマンド   | 説明                    |
|-----------|------------------------|
| `setup`   | 環境セットアップ          |
| `build`   | 全体ビルド              |
| `lib`     | ライブラリのみ           |
| `examples`| Exampleのみ            |
| `tests`   | テストのみ              |
| `clean`   | クリーンビルド           |
| `run`     | Example実行            |
| `all`     | setup + build + run    |

## 🎯 インタラクティブモード

パラメータなしで実行するとメニューが表示されます：

```powershell
.\build.ps1

🚀 Pandolabo ビルドスクリプト
================================

利用可能なコマンド:
  1. setup     - 環境セットアップ
  2. build     - 全体ビルド
  3. lib       - ライブラリのみ
  4. examples  - Exampleのみ
  5. tests     - テストのみ
  6. clean     - クリーンビルド
  7. run       - Example実行
  8. all       - setup + build + run
  0. exit      - 終了

選択してください (0-8):
```

## 🔧 開発ワークフロー

```powershell
# 1. 初回セットアップ
.\build.ps1 setup

# 2. ライブラリビルド（日常開発）
.\build.ps1 lib

# 3. 全体テスト
.\build.ps1 all

# 4. クリーンビルド（問題発生時）
.\build.ps1 clean
```
