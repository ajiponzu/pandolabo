#include <catch2/catch_test_macros.hpp>
#include <vulkan/vulkan.hpp>

#include "pandolabo.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

using namespace pandora::core;

TEST_CASE("vk_helper basic conversions", "[vk][helper]") {
  REQUIRE(vk_helper::getImageLayout(ImageLayout::General)
          == vk::ImageLayout::eGeneral);
  REQUIRE(vk_helper::getFormat(DataFormat::R8G8B8A8Unorm)
          == vk::Format::eR8G8B8A8Unorm);
  REQUIRE(vk_helper::getSampleCount(ImageSampleCount::v4)
          == vk::SampleCountFlagBits::e4);
  REQUIRE(vk_helper::getSamplerFilter(SamplerFilter::Nearest)
          == vk::Filter::eNearest);
  REQUIRE(vk_helper::getSamplerAddressMode(SamplerAddressMode::ClampToEdge)
          == vk::SamplerAddressMode::eClampToEdge);
  REQUIRE(vk_helper::getSamplerCompareOp(SamplerCompareOp::Greater)
          == vk::CompareOp::eGreater);
  REQUIRE(vk_helper::getCompareOp(CompareOp::LessOrEqual)
          == vk::CompareOp::eLessOrEqual);
  REQUIRE(vk_helper::getBlendOp(BlendOp::Add) == vk::BlendOp::eAdd);

  const auto memory_flags =
      vk_helper::getMemoryPropertyFlags(MemoryUsage::CpuOnly);
  REQUIRE((memory_flags & vk::MemoryPropertyFlagBits::eHostVisible)
          == vk::MemoryPropertyFlagBits::eHostVisible);
  REQUIRE((memory_flags & vk::MemoryPropertyFlagBits::eHostCoherent)
          == vk::MemoryPropertyFlagBits::eHostCoherent);

  const std::vector<ColorComponent> components = {
      ColorComponent::R,
      ColorComponent::G,
      ColorComponent::B,
      ColorComponent::A,
  };
  const auto component_flags = vk_helper::getColorComponent(components);
  REQUIRE((component_flags & vk::ColorComponentFlagBits::eR)
          == vk::ColorComponentFlagBits::eR);
  REQUIRE((component_flags & vk::ColorComponentFlagBits::eG)
          == vk::ColorComponentFlagBits::eG);
  REQUIRE((component_flags & vk::ColorComponentFlagBits::eB)
          == vk::ColorComponentFlagBits::eB);
  REQUIRE((component_flags & vk::ColorComponentFlagBits::eA)
          == vk::ColorComponentFlagBits::eA);

  const std::vector<AccessFlag> access_flags = {
      AccessFlag::TransferWrite,
      AccessFlag::ShaderRead,
  };
  const auto vk_access = vk_helper::getAccessFlags(access_flags);
  REQUIRE((vk_access & vk::AccessFlagBits2::eTransferWrite)
          == vk::AccessFlagBits2::eTransferWrite);
  REQUIRE((vk_access & vk::AccessFlagBits2::eShaderRead)
          == vk::AccessFlagBits2::eShaderRead);

  const std::vector<PipelineStage> stages = {
      PipelineStage::Transfer,
      PipelineStage::FragmentShader,
  };
  const auto vk_stages = vk_helper::getPipelineStageFlags(stages);
  REQUIRE((vk_stages & vk::PipelineStageFlagBits2::eTransfer)
          == vk::PipelineStageFlagBits2::eTransfer);
  REQUIRE((vk_stages & vk::PipelineStageFlagBits2::eFragmentShader)
          == vk::PipelineStageFlagBits2::eFragmentShader);
}

TEST_CASE("vk_helper stencil op state conversion", "[vk][helper]") {
  StencilOpState state{};
  state
      .setOps(StencilOp::Replace,
              StencilOp::IncrementAndClamp,
              StencilOp::DecrementAndClamp)
      .setCompareOp(CompareOp::Greater)
      .setMasks(0x0Fu, 0xF0u)
      .setReference(7u);

  const auto vk_state = vk_helper::getStencilOpState(state);
  REQUIRE(vk_state.failOp == vk::StencilOp::eReplace);
  REQUIRE(vk_state.passOp == vk::StencilOp::eIncrementAndClamp);
  REQUIRE(vk_state.depthFailOp == vk::StencilOp::eDecrementAndClamp);
  REQUIRE(vk_state.compareOp == vk::CompareOp::eGreater);
  REQUIRE(vk_state.compareMask == 0x0Fu);
  REQUIRE(vk_state.writeMask == 0xF0u);
  REQUIRE(vk_state.reference == 7u);
}
