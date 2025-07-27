#include "simple_image_computing.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

samples::core::SimpleImageComputing::SimpleImageComputing() {
  m_ptrContext = std::make_unique<plc::gpu::Context>(nullptr);

  m_ptrComputeCommandDriver = std::make_unique<plc::CommandDriver>(m_ptrContext, plc::QueueFamilyType::Compute);
  m_ptrTransferCommandDriver = std::make_unique<plc::CommandDriver>(m_ptrContext, plc::QueueFamilyType::Transfer);

  m_ptrUniformBuffer = plc::createUniqueUniformBuffer(m_ptrContext, sizeof(float_t));
  const auto mapped_address = m_ptrUniformBuffer->mapMemory(m_ptrContext);
  std::fill_n(reinterpret_cast<float_t*>(mapped_address), m_ptrUniformBuffer->getSize() / sizeof(float_t), 5.0f);
  m_ptrUniformBuffer->unmapMemory(m_ptrContext);

  initializeImageResources();
  constructShaderResources();
}

samples::core::SimpleImageComputing::~SimpleImageComputing() {
  m_ptrContext->getPtrDevice()->waitIdle();
  stbi_image_free(m_image.data);
}

void samples::core::SimpleImageComputing::run() {
  plc::gpu::TimelineSemaphore semaphore(m_ptrContext);

  const auto result_buffer =
      plc::createStagingBufferFromGPU(m_ptrContext, m_image.width * m_image.height * m_image.channels);
  {
    std::vector<plc::gpu::Buffer> staging_buffers;
    setTransferCommands(staging_buffers);

    setComputeCommands(result_buffer);

    m_ptrTransferCommandDriver->submit(plc::PipelineStage::BottomOfPipe, semaphore);
    m_ptrComputeCommandDriver->submit(plc::PipelineStage::ComputeShader, semaphore);
    semaphore.wait(m_ptrContext);
  }

  const auto result_mapped_address = result_buffer.mapMemory(m_ptrContext);
  const auto& image_size = m_ptrStorageImage->getGraphicalSize();

  const size_t image_buf_size = image_size.width * image_size.height * 4;
  auto image_buf = std::make_unique<unsigned char[]>(image_buf_size);
  std::memcpy(reinterpret_cast<void*>(image_buf.get()), result_mapped_address, image_buf_size);
  result_buffer.unmapMemory(m_ptrContext);

  stbi_write_png("examples/output.png", image_size.width, image_size.height, 4, image_buf.get(), image_size.width * 4);
}

void samples::core::SimpleImageComputing::initializeImageResources() {
  m_image.data =
      stbi_load("examples/core/computing_image/lenna.png", &(m_image.width), &(m_image.height), &(m_image.channels), 0);

  plc::ImageSubInfo image_sub_info;
  image_sub_info.graphical_size.width = static_cast<uint32_t>(m_image.width);
  image_sub_info.graphical_size.height = static_cast<uint32_t>(m_image.height);
  image_sub_info.graphical_size.depth = 1U;
  image_sub_info.mip_levels = 1U;
  image_sub_info.array_layers = 1U;
  image_sub_info.samples = plc::ImageSampleCount::v1;
  image_sub_info.format = plc::DataFormat::R8G8B8A8Unorm;
  image_sub_info.dimension = plc::ImageDimension::v2D;

  m_ptrImage = std::make_unique<plc::gpu::Image>(m_ptrContext,
                                                 plc::MemoryUsage::GpuOnly,
                                                 plc::TransferType::TransferSrcDst,
                                                 std::vector<plc::ImageUsage>{plc::ImageUsage::Sampled},
                                                 image_sub_info);

  m_ptrStorageImage = std::make_unique<plc::gpu::Image>(m_ptrContext,
                                                        plc::MemoryUsage::GpuOnly,
                                                        plc::TransferType::TransferSrcDst,
                                                        std::vector<plc::ImageUsage>{plc::ImageUsage::Storage},
                                                        image_sub_info);

  {
    plc::ImageViewInfo image_view_info{};
    image_view_info.aspect = plc::ImageAspect::Color;
    image_view_info.base_array_layer = 0U;
    image_view_info.array_layers = image_sub_info.array_layers;
    image_view_info.base_mip_level = 0U;
    image_view_info.mip_levels = image_sub_info.mip_levels;

    m_ptrImageView = std::make_unique<plc::gpu::ImageView>(m_ptrContext, *m_ptrImage, image_view_info);

    m_ptrStorageImageView = std::make_unique<plc::gpu::ImageView>(m_ptrContext, *m_ptrStorageImage, image_view_info);
  }

  {
    plc::SamplerInfo sampler_info{};
    sampler_info.address_mode_u = plc::SamplerAddressMode::ClampToBorder;
    sampler_info.address_mode_v = plc::SamplerAddressMode::ClampToBorder;
    sampler_info.address_mode_w = plc::SamplerAddressMode::ClampToBorder;
    sampler_info.mag_filter = plc::SamplerFilter::Linear;
    sampler_info.min_filter = plc::SamplerFilter::Linear;
    sampler_info.mipmap_mode = plc::SamplerMipmapMode::Linear;
    sampler_info.mip_lod_bias = 0.0f;
    sampler_info.anisotropy_enable = false;
    sampler_info.compare_enable = false;
    sampler_info.max_lod = static_cast<float_t>(image_sub_info.mip_levels);
    sampler_info.min_lod = 0.0f;
    sampler_info.border_color = plc::SamplerBorderColor::FloatOpaqueWhite;
    sampler_info.unnormalized_coordinates = false;

    m_ptrImageSampler = std::make_unique<plc::gpu::Sampler>(m_ptrContext, sampler_info);
  }
}

void samples::core::SimpleImageComputing::constructShaderResources() {
  const auto spirv_binary = plc::io::shader::read("examples/core/computing_image/simple_image.comp");

  m_shaderModuleMap["compute"] = plc::gpu::ShaderModule(m_ptrContext, spirv_binary);

  const auto description_unit = plc::gpu::DescriptionUnit(m_shaderModuleMap, {"compute"});

  m_ptrDescriptorSetLayout = std::make_unique<plc::gpu::DescriptorSetLayout>(m_ptrContext, description_unit);
  m_ptrDescriptorSet = std::make_unique<plc::gpu::DescriptorSet>(m_ptrContext, *m_ptrDescriptorSetLayout);

  std::vector<plc::gpu::BufferDescription> buffer_descriptions;
  buffer_descriptions.emplace_back(description_unit.getDescriptorInfoMap().at("UniformNumber"), *m_ptrUniformBuffer);

  std::vector<plc::gpu::ImageDescription> image_descriptions;
  image_descriptions.emplace_back(description_unit.getDescriptorInfoMap().at("image"),
                                  *m_ptrImageView,
                                  plc::ImageLayout::ShaderReadOnlyOptimal,
                                  *m_ptrImageSampler);
  image_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("dest_image"), *m_ptrStorageImageView, plc::ImageLayout::General);

  m_ptrDescriptorSet->updateDescriptorSet(m_ptrContext, buffer_descriptions, image_descriptions);

  m_ptrComputePipeline = std::make_unique<plc::Pipeline>(
      m_ptrContext, description_unit, *m_ptrDescriptorSetLayout, plc::PipelineBind::Compute);
  m_ptrComputePipeline->constructComputePipeline(m_ptrContext, m_shaderModuleMap.at("compute"));
}

void samples::core::SimpleImageComputing::setTransferCommands(std::vector<plc::gpu::Buffer>& staging_buffers) {
  const auto command_buffer = m_ptrTransferCommandDriver->getTransfer();

  staging_buffers.push_back(
      plc::createStagingBufferToGPU(m_ptrContext, m_image.width * m_image.height * m_image.channels));

  auto& staging_buffer = staging_buffers.back();
  const auto mapped_address = staging_buffer.mapMemory(m_ptrContext);
  std::memcpy(mapped_address, m_image.data, staging_buffer.getSize());
  staging_buffer.unmapMemory(m_ptrContext);

  command_buffer.begin();

  plc::ImageViewInfo image_view_info = m_ptrImageView->getImageViewInfo();

  {
    const auto image_barrier = plc::gpu::ImageBarrier(*m_ptrImage,
                                                      {plc::AccessFlag::Unknown},
                                                      {plc::AccessFlag::TransferWrite},
                                                      plc::ImageLayout::Undefined,
                                                      plc::ImageLayout::TransferDstOptimal,
                                                      image_view_info);
    command_buffer.setPipelineBarrier(image_barrier, plc::PipelineStage::TopOfPipe, plc::PipelineStage::Transfer);
  }

  command_buffer.copyBufferToImage(staging_buffer, *m_ptrImage, plc::ImageLayout::TransferDstOptimal, image_view_info);

  {
    auto image_barrier = plc::gpu::ImageBarrier(*m_ptrImage,
                                                {plc::AccessFlag::TransferWrite},
                                                {plc::AccessFlag::ShaderRead},
                                                plc::ImageLayout::TransferDstOptimal,
                                                plc::ImageLayout::TransferDstOptimal,
                                                image_view_info);
    image_barrier.setSrcQueueFamilyIndex(m_ptrTransferCommandDriver->getQueueFamilyIndex());
    image_barrier.setDstQueueFamilyIndex(m_ptrComputeCommandDriver->getQueueFamilyIndex());

    command_buffer.setPipelineBarrier(image_barrier, plc::PipelineStage::Transfer, plc::PipelineStage::BottomOfPipe);
  }

  command_buffer.end();
}

void samples::core::SimpleImageComputing::setComputeCommands(const plc::gpu::Buffer& staging_buffer) {
  static float_t push_timer = 0.0f;
  push_timer += 0.001f;

  const auto command_buffer = m_ptrComputeCommandDriver->getCompute();

  command_buffer.begin();

  {
    const plc::ImageViewInfo image_view_info = m_ptrImageView->getImageViewInfo();

    auto image_barrier = plc::gpu::ImageBarrier(*m_ptrImage,
                                                {plc::AccessFlag::TransferWrite},
                                                {plc::AccessFlag::ShaderRead},
                                                plc::ImageLayout::TransferDstOptimal,
                                                plc::ImageLayout::ShaderReadOnlyOptimal,
                                                image_view_info);
    image_barrier.setSrcQueueFamilyIndex(m_ptrTransferCommandDriver->getQueueFamilyIndex());
    image_barrier.setDstQueueFamilyIndex(m_ptrComputeCommandDriver->getQueueFamilyIndex());

    command_buffer.setPipelineBarrier(
        image_barrier, plc::PipelineStage::BottomOfPipe, plc::PipelineStage::ComputeShader);
  }

  const plc::ImageViewInfo image_view_info = m_ptrStorageImageView->getImageViewInfo();
  {
    const auto image_barrier = plc::gpu::ImageBarrier(*m_ptrStorageImage,
                                                      {plc::AccessFlag::Unknown},
                                                      {plc::AccessFlag::ShaderWrite},
                                                      plc::ImageLayout::Undefined,
                                                      plc::ImageLayout::General,
                                                      image_view_info);

    command_buffer.setPipelineBarrier(image_barrier, plc::PipelineStage::TopOfPipe, plc::PipelineStage::ComputeShader);
  }

  command_buffer.pushConstants(*m_ptrComputePipeline, {plc::ShaderStage::Compute}, 0U, {push_timer});
  command_buffer.bindPipeline(*m_ptrComputePipeline);
  command_buffer.bindDescriptorSet(*m_ptrComputePipeline, *m_ptrDescriptorSet);
  command_buffer.compute(plc::ComputeWorkGroupSize{
      m_ptrImage->getGraphicalSize().width / 4U, m_ptrImage->getGraphicalSize().height / 4U, 1U});

  {
    const auto image_barrier = plc::gpu::ImageBarrier(*m_ptrStorageImage,
                                                      {plc::AccessFlag::ShaderWrite},
                                                      {plc::AccessFlag::TransferRead},
                                                      plc::ImageLayout::General,
                                                      plc::ImageLayout::General,
                                                      image_view_info);

    command_buffer.setPipelineBarrier(image_barrier, plc::PipelineStage::ComputeShader, plc::PipelineStage::Transfer);

    command_buffer.copyImageToBuffer(*m_ptrStorageImage, staging_buffer, plc::ImageLayout::General, image_view_info);
  }

  {
    const auto image_barrier = plc::gpu::ImageBarrier(*m_ptrStorageImage,
                                                      {plc::AccessFlag::TransferRead},
                                                      {plc::AccessFlag::ShaderWrite},
                                                      plc::ImageLayout::General,
                                                      plc::ImageLayout::General,
                                                      image_view_info);
    command_buffer.setPipelineBarrier(image_barrier, plc::PipelineStage::Transfer, plc::PipelineStage::ComputeShader);
  }

  command_buffer.end();
}
