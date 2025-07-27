# VS Code設定のリロード手順

Vulkan C++ラッパーライブラリ プロジェクトの開発環境でIntelliSenseが正常に動作するように、以下の手順を実行してください：

## 1. 自動設定生成の実行

```powershell
# Windows: VS Code設定を自動生成
.\scripts\build.ps1 vscode -Configuration Debug   # デバッグ用
.\scripts\build.ps1 vscode -Configuration Release # リリース用
```

```bash
# Linux/macOS: VS Code設定を自動生成
./scripts/build.sh vscode
```

## 2. IntelliSenseの再起動
- `Ctrl+Shift+P` でコマンドパレットを開く
- `C/C++: Restart IntelliSense Engine` を実行

## 3. VS Codeの設定確認
- `Ctrl+Shift+P` でコマンドパレットを開く
- `C/C++: Log Diagnostics` を実行してIntelliSenseの状態を確認

## 4. ワークスペースのリロード
- `Ctrl+Shift+P` でコマンドパレットを開く
- `Developer: Reload Window` を実行

## 設定内容

### 重要なマクロ設定
```json
"defines": [
    "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1",  // Vulkan動的ディスパッチ必須マクロ
    "_DEBUG=1"                               // デバッグモード時のみ
]
```

### インクルードパス
- プロジェクトのincludeディレクトリ: `${workspaceFolder}/include`
- Conanパッケージディレクトリ: `${workspaceFolder}/build/conan/include`
- CMakeが生成するcompile_commands.jsonによる正確なパス

### 補完対象ライブラリ (実際の依存関係)
- ✅ vulkan/vulkan.hpp (公式Vulkan C++バインディング)
- ✅ GLFW (ウィンドウ管理: `GLFW/glfw3.h`)
- ✅ glslang (GLSL→SPIRVコンパイラ)
- ✅ SPIRV-Tools (SPIRV最適化・検証)
- ✅ SPIRV-Headers
- ✅ SPIRV-Cross (SPIRV変換ツール)
- ✅ nlohmann/json (JSON処理)
- ✅ Catch2 (テストフレームワーク)

## トラブルシューティング

### IntelliSenseエラーが残る場合
```powershell
# 1. ビルドディレクトリをクリア
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# 2. 再ビルド
.\scripts\build.ps1 setup
.\scripts\build.ps1 build

# 3. VS Code設定再生成
.\scripts\build.ps1 vscode -Configuration Debug

# 4. VS Code再起動
```

### VULKAN_HPP_DISPATCH_LOADER_DYNAMIC エラー
このマクロが認識されない場合は、自動生成スクリプトが正常に動作していない可能性があります。手動で `.vscode/c_cpp_properties.json` を確認し、`"VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"` が定義されていることを確認してください。

### Vulkan関連のインクルードエラー
```cpp
// 正しいインクルード方法
#include <vulkan/vulkan.hpp>  // 公式Vulkan C++バインディング
#include "pandora/core.hpp"   // 独自ラッパーライブラリ
```

## 確認方法
1. `gpu.hpp`ファイルを開く
2. `vk::` と入力して補完が表示されることを確認
3. `#include <GLFW/` と入力して`glfw3.h`が補完候補に表示されることを確認
