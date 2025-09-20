#include <catch2/catch_test_macros.hpp>

#include "pandolabo.hpp"
#include "util/test_env.hpp"

using namespace pandora::core;

TEST_CASE("Headless GPU Context initializes", "[gpu][context]") {
  PANDOLABO_REQUIRE_GPU_OR_SKIP();

  // Headless context: no window surface
  std::shared_ptr<gpu_ui::WindowSurface> no_surface;
  gpu::Context ctx{no_surface};

  REQUIRE(ctx.isInitialized());
  REQUIRE(static_cast<bool>(ctx.getInstance()));
  REQUIRE(ctx.getPtrDevice() != nullptr);
  // No swapchain expected in headless mode
  REQUIRE(ctx.getPtrSwapchain() == nullptr);
}
