#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Pandolabo ビルドスクリプト (PowerShell版)

.DESCRIPTION
    プラットフォーム検出 + 自動環境構築 + ビルド実行

           # requirements.txtがあればインストール
        $RequirementsFile = Join-Path $ProjectRoot "requirements.txt"
        if (Test-Path $RequirementsFile) {
            Write-Host "📋 requirements.txt からパッケージをインストール中..." -ForegroundColor Yellow
            $reqInstall = Start-Process -FilePath $VenvPython -ArgumentList "-m", "pip", "install", "-r", $RequirementsFile -NoNewWindow -Wait -PassThru
            if ($reqInstall.ExitCode -eq 0) {
                Write-Host "✅ requirements.txt のパッケージがインストールされました。" -ForegroundColor Green
            } else {
                Write-Host "⚠️  一部のパッケージのインストールに失敗しました。" -ForegroundColor Yellow
            }
        }

        # conanfile.txtがあればConanをインストール
        $ConanFile = Join-Path $ProjectRoot "conanfile.txt"
        if (Test-Path $ConanFile) {
            Write-Host "🔧 Conan パッケージマネージャーをインストール中..." -ForegroundColor Yellow
            $conanInstall = Start-Process -FilePath $VenvPython -ArgumentList "-m", "pip", "install", "conan" -NoNewWindow -Wait -PassThru
            if ($conanInstall.ExitCode -eq 0) {
                Write-Host "✅ Conan がインストールされました。" -ForegroundColor Green
            } else {
                Write-Host "⚠️  Conan のインストールに失敗しました。" -ForegroundColor Yellow
            }
        } - Python仮想環境の自動作成 (.venv)
    - 必要なパッケージの自動インストール (requirements.txt)
    - プラットフォーム対応のビルド実行
    - VSCode設定の自動生成

.PARAMETER Command
    実行するコマンド: setup, build, lib, examples, tests, clean, rebuild, run, all, vscode, format

.PARAMETER Interactive
    インタラクティブモードで実行

.EXAMPLE
    .\build.ps1 lib
    .\build.ps1 -Interactive
    .\build.ps1 all
    .\build.ps1 setup    # 開発環境を自動構築
#>

param(
    [Parameter(Position=0)]
    [ValidateSet("setup", "build", "lib", "examples", "tests", "test", "clean", "rebuild", "run", "all", "vscode", "format", "")]
    [string]$Command = "",

    [Parameter()]
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel", "")]
    [string]$Configuration = "Release",

    [Parameter()]
    [switch]$Interactive,

    [Parameter()]
    [string]$Example
)

# スクリプトディレクトリを取得
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent $ScriptDir
$PythonScript = Join-Path $ScriptDir "build.py"

# プロジェクトルートに移動
Set-Location $ProjectRoot

function Show-Menu {
    Write-Host ""
    Write-Host "🚀 Pandolabo ビルドスクリプト" -ForegroundColor Green
    Write-Host "================================" -ForegroundColor Green
    Write-Host "現在の設定: $Configuration ビルド" -ForegroundColor Magenta
    Write-Host ""
    Write-Host "[環境]" -ForegroundColor Yellow
    Write-Host "  1. setup     - 環境セットアップ (.venv + Conan 2.x)" -ForegroundColor Cyan
    Write-Host "  2. clean     - ビルドディレクトリ削除" -ForegroundColor Cyan
    Write-Host "  3. rebuild   - クリーン + セットアップ ($Configuration)" -ForegroundColor Cyan
    Write-Host "  4. vscode    - VSCode設定一式生成 ($Configuration)" -ForegroundColor Cyan
    Write-Host "  f. format    - C++ファイル一括フォーマット" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "[ビルド/実行]" -ForegroundColor Yellow
    Write-Host "  5. build     - 全体ビルド ($Configuration)" -ForegroundColor Cyan
    Write-Host "  6. lib       - ライブラリのみビルド ($Configuration)" -ForegroundColor Cyan
    Write-Host "  7. examples  - Exampleをビルドして実行 ($Configuration)" -ForegroundColor Cyan
    Write-Host "  8. tests     - テストをビルドして実行 ($Configuration)" -ForegroundColor Cyan
    Write-Host "  9. run       - Example実行のみ ($Configuration)" -ForegroundColor Cyan
    Write-Host "  0. all       - setup + build + run ($Configuration)" -ForegroundColor Cyan
    Write-Host "  t. test      - テスト実行 (エイリアス)" -ForegroundColor Green
    Write-Host ""
    Write-Host "設定変更:" -ForegroundColor Yellow
    Write-Host "  d. debug     - Debug ビルドに切り替え" -ForegroundColor Green
    Write-Host "  r. release   - Release ビルドに切り替え" -ForegroundColor Green
    Write-Host "  w. relwithdeb- RelWithDebInfo ビルドに切り替え" -ForegroundColor Green
    Write-Host "  m. minsize   - MinSizeRel ビルドに切り替え" -ForegroundColor Green
    Write-Host ""
    Write-Host "  x. exit      - 終了" -ForegroundColor Red
    Write-Host ""
}

function Select-Example {
    # 既知のExampleターゲット一覧（CMakeLists.txtに合わせて更新）
    $examples = @(
        "example_basic_compute",
        "example_basic_cube",
        "example_simple_image",
        "example_square"
    )

    Write-Host ""; Write-Host "🎯 実行するExampleを選択してください:" -ForegroundColor Yellow
    for ($i = 0; $i -lt $examples.Count; $i++) {
        Write-Host ("  {0}. {1}" -f ($i+1), $examples[$i]) -ForegroundColor Cyan
    }
    Write-Host "  0. キャンセル" -ForegroundColor Red

    do {
        $idx = Read-Host "番号を入力 (0-{0})" -f $examples.Count
        if ($idx -match '^[0-9]+$') {
            $num = [int]$idx
            if ($num -eq 0) { return $null }
            if ($num -ge 1 -and $num -le $examples.Count) {
                return $examples[$num-1]
            }
        }
        Write-Host "無効な選択です。0-{0} の番号を入力してください。" -f $examples.Count -ForegroundColor Red
    } while ($true)
}

function Get-UserChoice {
    $choices = @{
        "1" = "setup"
        "2" = "clean"
        "3" = "rebuild"
        "4" = "vscode"
        "5" = "build"
        "6" = "lib"
        "7" = "examples"
        "8" = "tests"
        "9" = "run"
        "0" = "all"
        "f" = "format"
        "t" = "test"
        "d" = "debug"
        "r" = "release"
        "w" = "relwithdebinfo"
        "m" = "minsize"
        "x" = "exit"
    }

    do {
        $input = Read-Host "選択してください (0-9, f, t, d/r/w/m, x)"
        if ($choices.ContainsKey($input)) {
            return $choices[$input]
        }
        Write-Host "無効な選択です。0-9, f, t, d/r/w/m, xを入力してください。" -ForegroundColor Red
    } while ($true)
}

function Invoke-PythonBuild {
    param(
        [string]$BuildCommand,
        [string]$BuildConfiguration = "Release",
        [string]$BuildExample = $null
    )

    if ($BuildCommand -eq "exit") {
        Write-Host "👋 終了します。" -ForegroundColor Yellow
        return 0
    }

    # 設定変更の処理
    if ($BuildCommand -in @("debug", "release", "relwithdebinfo", "minsize")) {
        switch ($BuildCommand) {
            "debug" { $script:Configuration = "Debug" }
            "release" { $script:Configuration = "Release" }
            "relwithdebinfo" { $script:Configuration = "RelWithDebInfo" }
            "minsize" { $script:Configuration = "MinSizeRel" }
        }
        Write-Host "✅ ビルド設定を $($script:Configuration) に変更しました。" -ForegroundColor Green
        return 0
    }

    # Python仮想環境のパスを確認
    $VenvPython = Join-Path $ProjectRoot ".venv\Scripts\python.exe"
    $VenvDir = Join-Path $ProjectRoot ".venv"

    if (-not (Test-Path $VenvPython)) {
        Write-Host "❌ Python仮想環境が見つかりません: $VenvPython" -ForegroundColor Red
        Write-Host "🔧 Python仮想環境を自動作成します..." -ForegroundColor Yellow

        # Pythonコマンドの確認
        try {
            $pythonCmd = Get-Command python -ErrorAction Stop
            Write-Host "✅ Python が見つかりました: $($pythonCmd.Source)" -ForegroundColor Green
        } catch {
            Write-Host "❌ Python が見つかりません。Pythonをインストールしてください。" -ForegroundColor Red
            return 1
        }

        # 仮想環境作成
        Write-Host "実行コマンド: python -m venv $VenvDir" -ForegroundColor Cyan
        $process = Start-Process -FilePath "python" -ArgumentList "-m", "venv", $VenvDir -NoNewWindow -Wait -PassThru

        if ($process.ExitCode -ne 0) {
            Write-Host "❌ 仮想環境の作成に失敗しました。" -ForegroundColor Red
            return 1
        }

        if (-not (Test-Path $VenvPython)) {
            Write-Host "❌ 仮想環境の作成後もPythonが見つかりません。" -ForegroundColor Red
            return 1
        }

        Write-Host "✅ Python仮想環境が正常に作成されました。" -ForegroundColor Green

        # 必要なパッケージをインストール
        Write-Host "📦 必要なパッケージをインストール中..." -ForegroundColor Yellow
        $pipUpgrade = Start-Process -FilePath $VenvPython -ArgumentList "-m", "pip", "install", "--upgrade", "pip" -NoNewWindow -Wait -PassThru
        if ($pipUpgrade.ExitCode -eq 0) {
            Write-Host "✅ pip が更新されました。" -ForegroundColor Green
        }

        # requirements.txtがあればインストール
        $RequirementsFile = Join-Path $ProjectRoot "requirements.txt"
        if (Test-Path $RequirementsFile) {
            Write-Host "📋 requirements.txt からパッケージをインストール中..." -ForegroundColor Yellow
            $reqInstall = Start-Process -FilePath $VenvPython -ArgumentList "-m", "pip", "install", "-r", $RequirementsFile -NoNewWindow -Wait -PassThru
            if ($reqInstall.ExitCode -eq 0) {
                Write-Host "✅ requirements.txt のパッケージがインストールされました。" -ForegroundColor Green
            } else {
                Write-Host "⚠️  一部のパッケージのインストールに失敗しました。" -ForegroundColor Yellow
            }
        }

        # conanfile.txtがあればConan 2.x系をセットアップ
        $ConanFile = Join-Path $ProjectRoot "conanfile.txt"
        if (Test-Path $ConanFile) {
            Write-Host "🔧 Conan 2.x パッケージマネージャーをインストール中..." -ForegroundColor Yellow
            $conanInstall = Start-Process -FilePath $VenvPython -ArgumentList "-m", "pip", "install", "conan>=2.0" -NoNewWindow -Wait -PassThru
            if ($conanInstall.ExitCode -eq 0) {
                Write-Host "✅ Conan 2.x がインストールされました。" -ForegroundColor Green

                # Conan実行ファイルのパスを取得
                $ConanExe = Join-Path (Split-Path $VenvPython) "conan.exe"
                if (Test-Path $ConanExe) {
                    # Conanプロファイルの設定
                    Write-Host "⚙️  Conan プロファイルを設定中..." -ForegroundColor Yellow
                    $conanProfile = Start-Process -FilePath $ConanExe -ArgumentList "profile", "detect", "--force" -WorkingDirectory $ProjectRoot -NoNewWindow -Wait -PassThru
                    if ($conanProfile.ExitCode -eq 0) {
                        Write-Host "✅ Conan プロファイルが設定されました。" -ForegroundColor Green

                        # Conanパッケージの依存関係をインストール
                        Write-Host "📦 Conan パッケージをインストール中..." -ForegroundColor Yellow
                        $conanInstallPkg = Start-Process -FilePath $ConanExe -ArgumentList "install", ".", "--output-folder=build", "--build=missing" -WorkingDirectory $ProjectRoot -NoNewWindow -Wait -PassThru
                        if ($conanInstallPkg.ExitCode -eq 0) {
                            Write-Host "✅ Conan パッケージがインストールされました。" -ForegroundColor Green
                        } else {
                            Write-Host "⚠️  Conan パッケージのインストールに失敗しました。後で手動でインストールしてください。" -ForegroundColor Yellow
                            Write-Host "手動実行コマンド: $ConanExe install . --output-folder=build --build=missing" -ForegroundColor Cyan
                        }
                    } else {
                        Write-Host "⚠️  Conan プロファイルの設定に失敗しました。" -ForegroundColor Yellow
                        Write-Host "手動実行コマンド: $ConanExe profile detect --force" -ForegroundColor Cyan
                    }
                } else {
                    Write-Host "⚠️  Conan実行ファイルが見つかりません。" -ForegroundColor Yellow
                }
            } else {
                Write-Host "⚠️  Conan のインストールに失敗しました。" -ForegroundColor Yellow
                Write-Host "手動実行コマンド: $VenvPython -m pip install conan>=2.0" -ForegroundColor Cyan
            }
        }
    }

    if (-not (Test-Path $PythonScript)) {
        Write-Host "❌ ビルドスクリプトが見つかりません: $PythonScript" -ForegroundColor Red
        return 1
    }

    # Pythonスクリプト実行
    Write-Host ""
    Write-Host "🐍 Python ビルドスクリプトを実行中..." -ForegroundColor Green
    $argsList = @($PythonScript, $BuildCommand, "--config", $BuildConfiguration)
    if ($BuildExample -and $BuildExample.Trim() -ne "") {
        $argsList += @("--example", $BuildExample)
    }
    Write-Host ("実行コマンド: {0} {1}" -f $VenvPython, ($argsList -join ' ')) -ForegroundColor Cyan

    # バッファリングを無効にしてリアルタイム出力
    $env:PYTHONUNBUFFERED = "1"
    $process = Start-Process -FilePath $VenvPython -ArgumentList $argsList -NoNewWindow -Wait -PassThru
    $exitCode = $process.ExitCode

    Write-Host "終了コード: $exitCode" -ForegroundColor Cyan
    return $exitCode
}

# メイン処理
try {
    if ($Interactive -or $Command -eq "") {
        # インタラクティブモード
        do {
            Show-Menu
            $selectedCommand = Get-UserChoice

            if ($selectedCommand -eq "exit") {
                break
            }

            # 設定変更コマンドの処理
            if ($selectedCommand -in @("debug", "release", "relwithdebinfo", "minsize")) {
                $result = Invoke-PythonBuild $selectedCommand $Configuration
                Start-Sleep -Seconds 2
                continue
            }

            if ($selectedCommand -eq "vscode") {
                Write-Host ""
                Write-Host "🎯 VSCode設定一式を生成中..." -ForegroundColor Green
                Write-Host "   設定モード: $Configuration" -ForegroundColor Cyan
                $vscodeScript = Join-Path $ScriptDir "generate_vscode_config.py"
                if (Test-Path $vscodeScript) {
                    $VenvPython = Join-Path $ProjectRoot ".venv\Scripts\python.exe"
                    if (-not (Test-Path $VenvPython)) {
                        # Fallback to system Python
                        $VenvPython = "python"
                    }

                    # Configuration に応じて引数を決定
                    if ($Configuration -eq "Debug") {
                        & $VenvPython $vscodeScript
                    } else {
                        & $VenvPython $vscodeScript "--release"
                    }
                    if ($LASTEXITCODE -eq 0) {
                        Write-Host "✅ VSCode設定一式が更新されました" -ForegroundColor Green
                    } else {
                        Write-Host "❌ VSCode設定の更新に失敗しました" -ForegroundColor Red
                    }
                } else {
                    Write-Host "❌ VSCode設定スクリプトが見つかりません" -ForegroundColor Red
                }
                Start-Sleep -Seconds 2
                continue
            }

            if ($selectedCommand -eq "format") {
                Write-Host ""
                Write-Host "🎨 C++ファイルを一括フォーマット中..." -ForegroundColor Green

                # clang-formatが利用可能かチェック
                try {
                    $clangFormatVersion = & clang-format --version 2>$null
                    Write-Host "✅ clang-formatが見つかりました: $clangFormatVersion" -ForegroundColor Green
                } catch {
                    Write-Host "❌ clang-formatが見つかりません。" -ForegroundColor Red
                    Write-Host "   LLVM/Clangをインストールしてください。" -ForegroundColor Yellow
                    Write-Host "   または、Visual Studio 2022のC++ツールをインストールしてください。" -ForegroundColor Yellow
                    Start-Sleep -Seconds 2
                    continue
                }

                # .clang-formatファイルの存在確認
                $clangFormatFile = Join-Path $ProjectRoot ".clang-format"
                if (-not (Test-Path $clangFormatFile)) {
                    Write-Host "⚠️  .clang-formatファイルが見つかりません。VSCode設定を再生成してください。" -ForegroundColor Yellow
                    Write-Host "   実行: .\scripts\build.ps1 vscode" -ForegroundColor Cyan
                    Start-Sleep -Seconds 2
                    continue
                }

                # C++ファイルを検索してフォーマット
                $cppFiles = Get-ChildItem -Recurse -Include *.cpp,*.hpp,*.h,*.c,*.cc,*.cxx | Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\.venv\\" }

                if ($cppFiles.Count -eq 0) {
                    Write-Host "🔍 フォーマット対象のC++ファイルが見つかりません。" -ForegroundColor Yellow
                    Start-Sleep -Seconds 2
                    continue
                }

                Write-Host "📁 $($cppFiles.Count) 個のC++ファイルを処理中..." -ForegroundColor Cyan

                $successCount = 0
                $errorCount = 0

                foreach ($file in $cppFiles) {
                    try {
                        Write-Host "  🔄 $($file.Name)" -ForegroundColor Gray
                        & clang-format -i -style=file $file.FullName
                        if ($LASTEXITCODE -eq 0) {
                            $successCount++
                        } else {
                            $errorCount++
                            Write-Host "    ❌ フォーマットに失敗: $($file.Name)" -ForegroundColor Red
                        }
                    } catch {
                        $errorCount++
                        Write-Host "    ❌ エラー: $($file.Name) - $($_.Exception.Message)" -ForegroundColor Red
                    }
                }

                Write-Host ""
                Write-Host "🎉 フォーマット完了!" -ForegroundColor Green
                Write-Host "   ✅ 成功: $successCount ファイル" -ForegroundColor Green
                if ($errorCount -gt 0) {
                    Write-Host "   ❌ 失敗: $errorCount ファイル" -ForegroundColor Red
                }
                Start-Sleep -Seconds 2
                continue
            }

            if ($selectedCommand -eq "test") {
                Write-Host ""
                Write-Host "🧪 テストをビルドして実行します ($Configuration)" -ForegroundColor Green
                $buildOk = (Invoke-PythonBuild "tests" $Configuration) -eq 0
                if (-not $buildOk) {
                    Write-Host "❌ テストのビルドに失敗しました" -ForegroundColor Red
                    Start-Sleep -Seconds 2
                    continue
                }
                $exe = Join-Path $ProjectRoot ("build/tests/{0}/tests.exe" -f $Configuration)
                if (-not (Test-Path $exe)) {
                    Write-Host "❌ テスト実行ファイルが見つかりません: $exe" -ForegroundColor Red
                    Start-Sleep -Seconds 2
                    continue
                }

                # JUnit出力先
                $resultsDir = Join-Path $ProjectRoot "build/test-results"
                New-Item -ItemType Directory -Force -Path $resultsDir | Out-Null
                $junit = Join-Path $resultsDir ("junit-{0}.xml" -f $Configuration)

                # GPUテストの有効/無効を案内
                if (-not $env:PANDOLABO_ENABLE_GPU_TESTS -or $env:PANDOLABO_ENABLE_GPU_TESTS -eq "0") {
                    Write-Host "ℹ️  GPUテストは無効です。有効化するには 'setx PANDOLABO_ENABLE_GPU_TESTS 1' を設定してください (新しいシェルで有効)。" -ForegroundColor Yellow
                }

                Write-Host "🚀 テスト実行中..." -ForegroundColor Cyan
                & $exe
                $exitCode = $LASTEXITCODE

                # 併せてJUnitレポートも生成
                & $exe --reporter junit --out $junit | Out-Null

                if ($exitCode -eq 0) {
                    Write-Host "✅ すべてのテストに合格しました ($Configuration)" -ForegroundColor Green
                } else {
                    Write-Host "❌ テストに失敗が含まれます (終了コード: $exitCode)" -ForegroundColor Red
                }
                Write-Host "📄 JUnit: $junit" -ForegroundColor DarkCyan
                Start-Sleep -Seconds 2
                continue
            }

            # run/build/all/examples で -Example 未指定の場合は対話選択を促す
            $exampleToUse = $Example
            if ($selectedCommand -in @("run", "build", "all", "examples") -and (-not $exampleToUse -or $exampleToUse.Trim() -eq "")) {
                $chosen = Select-Example
                if ($null -eq $chosen) {
                    Write-Host "⏭️  キャンセルしました。" -ForegroundColor Yellow
                    Start-Sleep -Seconds 2
                    continue
                }
                $exampleToUse = $chosen
            }

            if ($selectedCommand -eq "examples") {
                Write-Host ""
                Write-Host "🎯 Exampleをビルドして実行します: $exampleToUse ($Configuration)" -ForegroundColor Green
                $b = Invoke-PythonBuild "examples" $Configuration $exampleToUse
                if ($b -ne 0) {
                    Write-Host "❌ Exampleのビルドに失敗しました" -ForegroundColor Red
                    Start-Sleep -Seconds 2
                    continue
                }
                $exePath = Join-Path $ProjectRoot ("build/examples/{0}/{1}.exe" -f $Configuration,$exampleToUse)
                if (-not (Test-Path $exePath)) {
                    Write-Host "❌ 実行ファイルが見つかりません: $exePath" -ForegroundColor Red
                    Start-Sleep -Seconds 2
                    continue
                }
                Write-Host "🚀 実行: $exampleToUse" -ForegroundColor Cyan
                & $exePath
                Write-Host "✅ 完了: $exampleToUse" -ForegroundColor Green
                Start-Sleep -Seconds 2
                continue
            }

            $result = Invoke-PythonBuild $selectedCommand $Configuration $exampleToUse

            if ($result -eq 0) {
                Write-Host ""
                Write-Host "✅ コマンド '$selectedCommand' が正常に完了しました。" -ForegroundColor Green
            } else {
                Write-Host ""
                Write-Host "❌ コマンド '$selectedCommand' が失敗しました。(終了コード: $result)" -ForegroundColor Red
            }

            Start-Sleep -Seconds 2

        } while ($true)
    } else {
        # 直接コマンド実行
        if ($Command -eq "vscode") {
            Write-Host "🎯 VSCode設定一式を生成中..." -ForegroundColor Green
            Write-Host "   設定モード: $Configuration" -ForegroundColor Cyan
            $vscodeScript = Join-Path $ScriptDir "generate_vscode_config.py"
            if (Test-Path $vscodeScript) {
                $VenvPython = Join-Path $ProjectRoot ".venv\Scripts\python.exe"
                if (-not (Test-Path $VenvPython)) {
                    # Fallback to system Python
                    $VenvPython = "python"
                }

                # Configuration に応じて引数を決定
                if ($Configuration -eq "Debug") {
                    & $VenvPython $vscodeScript
                } else {
                    & $VenvPython $vscodeScript "--release"
                }
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "✅ VSCode設定一式が更新されました" -ForegroundColor Green
                    exit 0
                } else {
                    Write-Host "❌ VSCode設定の更新に失敗しました" -ForegroundColor Red
                    exit 1
                }
            } else {
                Write-Host "❌ VSCode設定スクリプトが見つかりません" -ForegroundColor Red
                exit 1
            }
        } elseif ($Command -eq "format") {
            Write-Host "🎨 C++ファイルを一括フォーマット中..." -ForegroundColor Green

            # clang-formatが利用可能かチェック
            try {
                $clangFormatVersion = & clang-format --version 2>$null
                Write-Host "✅ clang-formatが見つかりました: $clangFormatVersion" -ForegroundColor Green
            } catch {
                Write-Host "❌ clang-formatが見つかりません。" -ForegroundColor Red
                Write-Host "   LLVM/Clangをインストールしてください。" -ForegroundColor Yellow
                exit 1
            }

            # .clang-formatファイルの存在確認
            $clangFormatFile = Join-Path $ProjectRoot ".clang-format"
            if (-not (Test-Path $clangFormatFile)) {
                Write-Host "⚠️  .clang-formatファイルが見つかりません。VSCode設定を再生成してください。" -ForegroundColor Yellow
                Write-Host "   実行: .\scripts\build.ps1 vscode" -ForegroundColor Cyan
                exit 1
            }

            # C++ファイルを検索してフォーマット
            $cppFiles = Get-ChildItem -Recurse -Include *.cpp,*.hpp,*.h,*.c,*.cc,*.cxx | Where-Object { $_.FullName -notmatch "\\build\\" -and $_.FullName -notmatch "\\.venv\\" }

            if ($cppFiles.Count -eq 0) {
                Write-Host "🔍 フォーマット対象のC++ファイルが見つかりません。" -ForegroundColor Yellow
                exit 0
            }

            Write-Host "📁 $($cppFiles.Count) 個のC++ファイルを処理中..." -ForegroundColor Cyan

            $successCount = 0
            $errorCount = 0

            foreach ($file in $cppFiles) {
                try {
                    Write-Host "  🔄 $($file.Name)" -ForegroundColor Gray
                    & clang-format -i -style=file $file.FullName
                    if ($LASTEXITCODE -eq 0) {
                        $successCount++
                    } else {
                        $errorCount++
                        Write-Host "    ❌ フォーマットに失敗: $($file.Name)" -ForegroundColor Red
                    }
                } catch {
                    $errorCount++
                    Write-Host "    ❌ エラー: $($file.Name) - $($_.Exception.Message)" -ForegroundColor Red
                }
            }

            Write-Host ""
            Write-Host "🎉 フォーマット完了!" -ForegroundColor Green
            Write-Host "   ✅ 成功: $successCount ファイル" -ForegroundColor Green
            if ($errorCount -gt 0) {
                Write-Host "   ❌ 失敗: $errorCount ファイル" -ForegroundColor Red
                exit 1
            }
            exit 0
        } else {
            if ($Command -eq "test") {
                Write-Host "🧪 テストをビルドして実行します ($Configuration)" -ForegroundColor Green
                $buildCode = Invoke-PythonBuild "tests" $Configuration
                if ($buildCode -ne 0) {
                    Write-Host "❌ テストのビルドに失敗しました" -ForegroundColor Red
                    exit $buildCode
                }
                $exe = Join-Path $ProjectRoot ("build/tests/{0}/tests.exe" -f $Configuration)
                if (-not (Test-Path $exe)) {
                    Write-Host "❌ テスト実行ファイルが見つかりません: $exe" -ForegroundColor Red
                    exit 1
                }
                $resultsDir = Join-Path $ProjectRoot "build/test-results"
                New-Item -ItemType Directory -Force -Path $resultsDir | Out-Null
                $junit = Join-Path $resultsDir ("junit-{0}.xml" -f $Configuration)

                if (-not $env:PANDOLABO_ENABLE_GPU_TESTS -or $env:PANDOLABO_ENABLE_GPU_TESTS -eq "0") {
                    Write-Host "ℹ️  GPUテストは無効です。'setx PANDOLABO_ENABLE_GPU_TESTS 1' で有効化可能です。" -ForegroundColor Yellow
                }

                & $exe
                $code = $LASTEXITCODE
                & $exe --reporter junit --out $junit | Out-Null
                Write-Host "📄 JUnit: $junit" -ForegroundColor DarkCyan
                exit $code
            } elseif ($Command -eq "examples") {
                $exampleToUse = $Example
                if (-not $exampleToUse -or $exampleToUse.Trim() -eq "") {
                    $chosen = Select-Example
                    if ($null -eq $chosen) { Write-Host "⏭️  キャンセルしました。" -ForegroundColor Yellow; exit 0 }
                    $exampleToUse = $chosen
                }
                Write-Host "🎯 Exampleをビルドして実行します: $exampleToUse ($Configuration)" -ForegroundColor Green
                $b = Invoke-PythonBuild "examples" $Configuration $exampleToUse
                if ($b -ne 0) { Write-Host "❌ Exampleのビルドに失敗しました" -ForegroundColor Red; exit $b }
                $exePath = Join-Path $ProjectRoot ("build/examples/{0}/{1}.exe" -f $Configuration,$exampleToUse)
                if (-not (Test-Path $exePath)) { Write-Host "❌ 実行ファイルが見つかりません: $exePath" -ForegroundColor Red; exit 1 }
                & $exePath
                exit $LASTEXITCODE
            } else {
                $result = Invoke-PythonBuild $Command $Configuration $Example
                exit $result
            }
        }
    }
} catch {
    Write-Host "❌ エラーが発生しました: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host "👋 ビルドスクリプトを終了しました。" -ForegroundColor Yellow
