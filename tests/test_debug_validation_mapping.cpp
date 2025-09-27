#include <catch2/catch_test_macros.hpp>

#include "pandora/core/err/error.hpp"
#include "pandora/core/gpu/debug.hpp"

using namespace pandora::core;

#ifdef GPU_DEBUG
TEST_CASE("debug_map_severity_domain") {
  using namespace gpu::debug;
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
          == err::Severity::note);
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
          == err::Severity::note);
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
          == err::Severity::warning);
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
          == err::Severity::recoverable);

  REQUIRE(map_domain(VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
          == err::Domain::gpu_validation);
  REQUIRE(map_domain(VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
          == err::Domain::gpu);
  REQUIRE(map_domain(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
          == err::Domain::gpu);
}
#else
TEST_CASE("debug_map_severity_domain_noop") {
  SUCCEED("GPU_DEBUG disabled - mapping functions unused");
}
#endif
