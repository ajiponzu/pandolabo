#pragma once

#include <cstdlib>
#include <string>

// Provide Catch2 SUCCEED() macro for IntelliSense if available; fallback to
// no-op.
#if defined(__has_include)
  #if __has_include(<catch2/catch_test_macros.hpp>)
    #include <catch2/catch_test_macros.hpp>
  #else
    #ifndef SUCCEED
      #define SUCCEED(msg) ((void)0)
    #endif
  #endif
#else
  #ifndef SUCCEED
    #define SUCCEED(msg) ((void)0)
  #endif
#endif

// Define PANDOLABO_ENABLE_GPU_TESTS=1 in environment to enable GPU tests.
inline bool pandolabo_gpu_tests_enabled() {
  const char* v = std::getenv("PANDOLABO_ENABLE_GPU_TESTS");
  return v && std::string(v) == "1";
}

#define PANDOLABO_REQUIRE_GPU_OR_SKIP()                                \
  do {                                                                 \
    if (!pandolabo_gpu_tests_enabled()) {                              \
      SUCCEED("GPU tests disabled. Set PANDOLABO_ENABLE_GPU_TESTS=1"); \
      return;                                                          \
    }                                                                  \
  } while (0)
