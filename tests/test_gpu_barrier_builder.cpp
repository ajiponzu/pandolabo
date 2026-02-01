#include <catch2/catch_test_macros.hpp>
#include <vulkan/vulkan.hpp>

#include "pandolabo.hpp"

using namespace pandora::core;

TEST_CASE("MemoryBarrierBuilder builds barrier", "[gpu][barrier]") {
  const auto barrier = gpu::MemoryBarrierBuilder::create()
                           .setSrcAccessFlags({AccessFlag::TransferWrite})
                           .setDstAccessFlags({AccessFlag::ShaderRead})
                           .setSrcStages({PipelineStage::Transfer})
                           .setDstStages({PipelineStage::FragmentShader})
                           .build();

  const auto& vk_barrier = barrier.getBarrier();
  (void)vk_barrier;
  SUCCEED("MemoryBarrierBuilder produced a Vulkan barrier");
}

TEST_CASE("Barrier builders validate required parameters", "[gpu][barrier]") {
  auto buffer_builder = gpu::BufferBarrierBuilder::create()
                            .setSrcAccessFlags({AccessFlag::TransferWrite})
                            .setDstAccessFlags({AccessFlag::ShaderRead})
                            .setSrcStages({PipelineStage::Transfer})
                            .setDstStages({PipelineStage::FragmentShader});

  auto buffer_result = buffer_builder.build();
  REQUIRE(buffer_result.isError());
  REQUIRE(buffer_result.error().type() == ErrorType::Validation);

  auto image_builder = gpu::ImageBarrierBuilder::create()
                           .setSrcAccessFlags({AccessFlag::TransferWrite})
                           .setDstAccessFlags({AccessFlag::ShaderRead})
                           .setSrcStages({PipelineStage::Transfer})
                           .setDstStages({PipelineStage::FragmentShader})
                           .setOldLayout(ImageLayout::Undefined)
                           .setNewLayout(ImageLayout::ShaderReadOnlyOptimal);

  auto image_result = image_builder.build();
  REQUIRE(image_result.isError());
  REQUIRE(image_result.error().type() == ErrorType::Validation);
}
