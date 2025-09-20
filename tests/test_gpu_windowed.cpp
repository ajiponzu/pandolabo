#include <catch2/catch_test_macros.hpp>

#include "pandolabo.hpp"
#include "util/test_env.hpp"

using namespace pandora::core;

TEST_CASE("Windowed GPU Context initializes and creates swapchain",
          "[gpu][window][context]") {
  PANDOLABO_REQUIRE_GPU_OR_SKIP();

  // Initialize GLFW for Vulkan
  REQUIRE(glfwInit());
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow* window =
      glfwCreateWindow(320, 240, "pandolabo_test", nullptr, nullptr);
  REQUIRE(window != nullptr);

  // Wrap in WindowSurface and create GPU Context with presentation
  {
    auto surface = std::make_shared<gpu_ui::WindowSurface>(window);
    gpu::Context ctx{surface};

    REQUIRE(ctx.isInitialized());
    REQUIRE(static_cast<bool>(ctx.getInstance()));
    REQUIRE(ctx.getPtrDevice() != nullptr);
    REQUIRE(ctx.getPtrSwapchain() != nullptr);
  }

  // After surface/window destroyed, terminate GLFW
  glfwTerminate();
}
