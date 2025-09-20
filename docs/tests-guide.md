# Tests Guide

This project uses Catch2 for unit tests. VS Code tasks and launch configs are pre-generated for a smooth workflow on Windows/MSVC.

## Run Tests

- Tasks
  - Run: Terminal > Run Task > "🧪 Run Tests (Debug)" or "🧪 Run Tests (Release)"
  - Optional filter: when prompted, enter a Catch2 filter (e.g., `[math]`, `"Basic test"`, `-~[string]`). Leave empty to run all.

- CLI

```powershell
# Build tests
powershell.exe -ExecutionPolicy Bypass -File .\scripts\build.ps1 tests -Configuration Release

# Run tests manually
& "${PWD}\build\tests\Release\tests.exe"  # all tests
& "${PWD}\build\tests\Release\tests.exe" "[math]"  # filtered
```

## Debug Tests

- Launch configs
  - Run and Debug panel > "🐛 Debug Tests (Debug)" or "🚀 Debug Tests (Release)"
  - When prompted, enter a filter if needed.

## GPU-dependent tests

By default, GPU-heavy tests are skipped. To enable them:

```powershell
$env:PANDOLABO_ENABLE_GPU_TESTS = "1"
```

Use helper macro in tests:

```cpp
#include "tests/util/test_env.hpp"

TEST_CASE("GPU feature", "[gpu]") {
  PANDOLABO_REQUIRE_GPU_OR_SKIP();
  // ... GPU initialization and checks ...
}
```

## Tips

- Filters compose: `[render],[compute]` runs tests with either tag. `-~[slow]` excludes.
- Use the "🧪 Build Tests" task to compile only tests quickly.
- Example apps are independent; see example run tasks in VS Code.
