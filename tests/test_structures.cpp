#include <catch2/catch_test_macros.hpp>
#include <vulkan/vulkan.hpp>

#include "pandolabo.hpp"

using namespace pandora::core;

TEST_CASE("ImageSubInfo fluent setters", "[structures]") {
  ImageSubInfo info{};
  info.setSize(128, 64, 1)
      .setMipLevels(4)
      .setArrayLayers(2)
      .setSamples(ImageSampleCount::v4)
      .setFormat(DataFormat::R8G8B8A8Unorm)
      .setDimension(ImageDimension::v2D);

  REQUIRE(info.graphical_size.width == 128u);
  REQUIRE(info.graphical_size.height == 64u);
  REQUIRE(info.graphical_size.depth == 1u);
  REQUIRE(info.mip_levels == 4u);
  REQUIRE(info.array_layers == 2u);
  REQUIRE(info.samples == ImageSampleCount::v4);
  REQUIRE(info.format == DataFormat::R8G8B8A8Unorm);
  REQUIRE(info.dimension == ImageDimension::v2D);
}

TEST_CASE("DescriptorInfo fluent setters", "[structures]") {
  DescriptorInfo info{};
  info.setStageFlags(vk::ShaderStageFlagBits::eVertex)
      .setBinding(3)
      .setType(vk::DescriptorType::eUniformBuffer)
      .setSize(2);

  REQUIRE(info.stage_flags == vk::ShaderStageFlagBits::eVertex);
  REQUIRE(info.binding == 3u);
  REQUIRE(info.type == vk::DescriptorType::eUniformBuffer);
  REQUIRE(info.size == 2u);
}

TEST_CASE("PushConstantRange fluent setters", "[structures]") {
  PushConstantRange range{};
  range.setStageFlags(vk::ShaderStageFlagBits::eFragment)
      .setOffset(16)
      .setSize(64);

  REQUIRE(range.stage_flags == vk::ShaderStageFlagBits::eFragment);
  REQUIRE(range.offset == 16u);
  REQUIRE(range.size == 64u);
}

TEST_CASE("ImageViewInfo fluent setters", "[structures]") {
  ImageViewInfo view{};
  view.setMipRange(1, 4).setArrayRange(2, 3).setAspect(ImageAspect::Color);

  REQUIRE(view.base_mip_level == 1u);
  REQUIRE(view.mip_levels == 4u);
  REQUIRE(view.base_array_layer == 2u);
  REQUIRE(view.array_layers == 3u);
  REQUIRE(view.aspect == ImageAspect::Color);
}

TEST_CASE("SamplerInfo fluent setters", "[structures]") {
  SamplerInfo info{};
  info.setFilters(SamplerFilter::Nearest, SamplerFilter::Linear)
      .setMipmapMode(SamplerMipmapMode::Linear)
      .setAddressModes(SamplerAddressMode::ClampToEdge,
                       SamplerAddressMode::ClampToBorder,
                       SamplerAddressMode::Repeat)
      .setMipLodBias(1.25f)
      .setAnisotropy(true, 8.0f)
      .setCompare(true, SamplerCompareOp::Less)
      .setLodRange(0.5f, 4.0f)
      .setBorderColor(SamplerBorderColor::FloatOpaqueWhite)
      .setUnnormalizedCoordinates(true);

  REQUIRE(info.mag_filter == SamplerFilter::Nearest);
  REQUIRE(info.min_filter == SamplerFilter::Linear);
  REQUIRE(info.mipmap_mode == SamplerMipmapMode::Linear);
  REQUIRE(info.address_mode_u == SamplerAddressMode::ClampToEdge);
  REQUIRE(info.address_mode_v == SamplerAddressMode::ClampToBorder);
  REQUIRE(info.address_mode_w == SamplerAddressMode::Repeat);
  REQUIRE(info.mip_lod_bias == 1.25f);
  REQUIRE(info.anisotropy_enable);
  REQUIRE(info.max_anisotropy == 8.0f);
  REQUIRE(info.compare_enable);
  REQUIRE(info.compare_op == SamplerCompareOp::Less);
  REQUIRE(info.min_lod == 0.5f);
  REQUIRE(info.max_lod == 4.0f);
  REQUIRE(info.border_color == SamplerBorderColor::FloatOpaqueWhite);
  REQUIRE(info.unnormalized_coordinates);
}
