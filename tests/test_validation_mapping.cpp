#include <catch2/catch_test_macros.hpp>

#include "pandora/core/gpu/validation.hpp"

using namespace pandora::core;

TEST_CASE("validation_map_severity_basic") {
  using namespace gpu::validation;
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
          == err::Severity::note);
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
          == err::Severity::note);
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
          == err::Severity::warning);
  REQUIRE(map_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
          == err::Severity::recoverable);
}

TEST_CASE("validation_map_domain_basic") {
  using namespace gpu::validation;
  REQUIRE(map_domain(VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
          == err::Domain::gpu_validation);
  REQUIRE(map_domain(VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
          == err::Domain::gpu);
  REQUIRE(map_domain(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
          == err::Domain::gpu);
}
