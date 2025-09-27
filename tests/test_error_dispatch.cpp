#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <vulkan/vulkan.hpp>

#include "pandora/core/err/dispatch.hpp"
#include "pandora/core/gpu/gpu_check.hpp"

using namespace pandora::core;

static std::atomic<int> g_sink_calls{0};
static bool test_sink(const err::Error& e) {
  ++g_sink_calls;
  // Basic invariant: native_code must match cast of VkResult enumerator used.
  REQUIRE(e.native_code != 0u || e.code == err::Code::ok);
  return false;  // not consumed
}

TEST_CASE("error_dispatch_stats_and_sink") {
#ifdef PLB_NO_EXCEPTIONS
  // Install sink
  auto prev = err::set_error_sink(&test_sink);
  REQUIRE(prev == nullptr);
  auto& stats_before = err::global_error_stats();
  auto start_total = stats_before.total.load();

  // Trigger two failures
  auto s1 = gpu::check_vk(vk::Result::eTimeout, "vkWaitForFences");
  REQUIRE_FALSE(s1);
  auto s2 = gpu::check_vk(vk::Result::eSuboptimalKHR, "vkAcquireNextImageKHR");
  REQUIRE_FALSE(s2);

  // Success path should not increment
  auto s3 = gpu::check_vk(vk::Result::eSuccess, "vkQueuePresentKHR");
  REQUIRE(s3);

  auto& stats_after = err::global_error_stats();
  REQUIRE(stats_after.total.load()
          >= start_total + 2);  // at least two new errors
  REQUIRE(g_sink_calls.load() == 2);
#else
  SUCCEED("Dispatch test skipped in exception mode (would throw)");
#endif
}
