#include <catch2/catch_test_macros.hpp>
#include <vulkan/vulkan.hpp>

#include "pandora/core/err/result.hpp"
#include "pandora/core/gpu/gpu_check.hpp"

using namespace pandora::core;

// Helper to invoke check_vk directly (non-exception path assumed for tests when
// PLB_NO_EXCEPTIONS defined).
static err::Status call(vk::Result r) {
  return gpu::check_vk(r, "dummy_call");
}

TEST_CASE("vk_result_mapping_basic_success") {
  auto st = call(vk::Result::eSuccess);
  REQUIRE(st);  // success yields engaged Status
}

TEST_CASE("vk_result_mapping_timeout") {
  auto st = call(vk::Result::eTimeout);
  REQUIRE_FALSE(st);
  auto& e = st.error();
  REQUIRE(e.code == err::Code::timeout);
  REQUIRE(e.severity == err::Severity::recoverable);
}

TEST_CASE("vk_result_mapping_swapchain_suboptimal") {
  auto st = call(vk::Result::eSuboptimalKHR);
  REQUIRE_FALSE(st);
  auto& e = st.error();
  REQUIRE(e.code == err::Code::swapchain_out_of_date);
  REQUIRE(e.severity == err::Severity::recoverable);
}

TEST_CASE("vk_result_mapping_device_lost") {
  auto st = call(vk::Result::eErrorDeviceLost);
  REQUIRE_FALSE(st);
  auto& e = st.error();
  REQUIRE(e.code == err::Code::device_lost);
  REQUIRE(e.severity == err::Severity::fatal);
}

TEST_CASE("vk_result_mapping_unknown") {
  // Choose a value unlikely to be explicitly mapped (use an int cast beyond
  // known range if safe) Vulkan guarantees negative values are errors; pick one
  // not enumerated above if available. For portability simply skip constructing
  // invalid enum; instead rely on a mapped known one.
  auto st = call(static_cast<vk::Result>(-999));
  REQUIRE_FALSE(st);
  auto& e = st.error();
  REQUIRE(e.code == err::Code::unknown);
}

TEST_CASE("PLB_GPU_CHECK_macro_propagates") {
#ifndef PLB_NO_EXCEPTIONS
  SUCCEED("Exception mode not validated here");
#else
  struct Foo {
    err::Status run() {
      PLB_GPU_CHECK(vk::Result::eTimeout);  // should early return unexpected
      return {};                            // not reached
    }
  } foo;
  auto st = foo.run();
  REQUIRE_FALSE(st);
  REQUIRE(st.error().code == err::Code::timeout);
#endif
}
