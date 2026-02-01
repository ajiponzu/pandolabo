#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "pandolabo.hpp"
#include "util/test_env.hpp"

using namespace pandora::core;

TEST_CASE("Headless GPU resources create", "[gpu][resource]") {
  PANDOLABO_REQUIRE_GPU_OR_SKIP();

  std::shared_ptr<gpu_ui::WindowSurface> no_surface;
  auto ptr_context = std::make_unique<gpu::Context>(no_surface);

  REQUIRE(ptr_context->isInitialized());
  REQUIRE(ptr_context->getPtrDevice() != nullptr);

  gpu::Buffer buffer{
      ptr_context,
      MemoryUsage::CpuToGpu,
      TransferType::TransferDst,
      {BufferUsage::UniformBuffer},
      256u,
  };
  REQUIRE(buffer.getSize() == 256u);
  REQUIRE(static_cast<bool>(buffer.getPtrBuffer()));

  ImageSubInfo image_sub_info{};
  image_sub_info.setSize(64, 32, 1)
      .setMipLevels(1)
      .setArrayLayers(1)
      .setSamples(ImageSampleCount::v1)
      .setFormat(DataFormat::R8G8B8A8Unorm)
      .setDimension(ImageDimension::v2D);

  gpu::Image image{
      ptr_context,
      MemoryUsage::GpuOnly,
      TransferType::TransferDst,
      {ImageUsage::Sampled},
      image_sub_info,
  };
  REQUIRE(static_cast<bool>(image.getPtrImage()));
  REQUIRE(image.getMipLevels() == 1u);
  REQUIRE(image.getArrayLayers() == 1u);
  REQUIRE(image.getDimension() == ImageDimension::v2D);
  REQUIRE(image.getFormat() == vk::Format::eR8G8B8A8Unorm);
  REQUIRE(image.getGraphicalSize().width == 64u);
  REQUIRE(image.getGraphicalSize().height == 32u);

  ImageViewInfo view_info{};
  view_info.setMipRange(0, 1).setArrayRange(0, 1).setAspect(ImageAspect::Color);
  gpu::ImageView view{ptr_context, image, view_info};
  REQUIRE(static_cast<bool>(view.getImageView()));
  REQUIRE(view.getImageViewInfo().aspect == ImageAspect::Color);

  SamplerInfo sampler_info{};
  sampler_info.setFilters(SamplerFilter::Linear, SamplerFilter::Linear)
      .setMipmapMode(SamplerMipmapMode::Linear)
      .setAddressMode(SamplerAddressMode::Repeat)
      .setLodRange(0.0f, 1.0f);
  gpu::Sampler sampler{ptr_context, sampler_info};
  REQUIRE(static_cast<bool>(sampler.getSampler()));
}
