#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>

#include "pandolabo.hpp"
#include "util/test_env.hpp"

using namespace pandora::core;

TEST_CASE("TimelineSemaphore wait/signal info", "[gpu][sync][timeline]") {
  PANDOLABO_REQUIRE_GPU_OR_SKIP();

  std::shared_ptr<gpu_ui::WindowSurface> no_surface;
  auto ctx = std::make_unique<gpu::Context>(no_surface);
  REQUIRE(ctx->isInitialized());

  gpu::TimelineSemaphore ts{ctx};
  auto w = ts.forWait(5u);
  auto s = ts.forSignal(7u);

  REQUIRE(w.getWaitValue() == 5u);
  REQUIRE(s.getSignalValue() == 7u);
  // Underlying semaphore handle should be non-null
  REQUIRE(static_cast<bool>(ts.getTimelineSemaphore()));
}

TEST_CASE("SubmitSemaphoreGroup wiring (timeline)", "[gpu][sync][submit]") {
  PANDOLABO_REQUIRE_GPU_OR_SKIP();

  std::shared_ptr<gpu_ui::WindowSurface> no_surface;
  auto ctx = std::make_unique<gpu::Context>(no_surface);
  REQUIRE(ctx->isInitialized());
  REQUIRE(ctx->getPtrDevice() != nullptr);

  gpu::TimelineSemaphore ts{ctx};

  gpu::SubmitSemaphoreGroup group;
  group.setWaitSemaphores(ts.forWait(1u), ts.forWait(2u));
  group.setSignalSemaphores(ts.forSignal(3u));
  group.setWaitStages({PipelineStage::ComputeShader});

  REQUIRE(group.getWaitSemaphores().size() == 2);
  REQUIRE(group.getSignalSemaphores().size() == 1);
  REQUIRE(group.getPtrTimelineSubmitInfo() != nullptr);
}
