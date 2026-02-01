#include "simple_image_computing.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

namespace samples::core {

SimpleImageComputing::SimpleImageComputing() {
  m_ptrContext = std::make_unique<plc::gpu::Context>(nullptr);

  m_ptrComputeCommandDriver = std::make_unique<plc::CommandDriver>(
      *m_ptrContext, plc::QueueFamilyType::Compute);
  m_ptrTransferCommandDriver = std::make_unique<plc::CommandDriver>(
      *m_ptrContext, plc::QueueFamilyType::Transfer);

  m_ptrUniformBuffer =
      plc::createUniqueUniformBuffer(*m_ptrContext, sizeof(float_t));
  const auto mapped_address = m_ptrUniformBuffer->mapMemory(*m_ptrContext);
  std::fill_n(reinterpret_cast<float_t*>(mapped_address),
              m_ptrUniformBuffer->getSize() / sizeof(float_t),
              5.0f);
  m_ptrUniformBuffer->unmapMemory(*m_ptrContext);

  initializeImageResources();
  const auto shader_result = constructShaderResources();
  if (!shader_result.isOk()) {
    return;
  }

  m_isInitialized = true;
}

SimpleImageComputing::~SimpleImageComputing() {
  m_ptrContext->getPtrDevice()->waitIdle();
  stbi_image_free(m_image.data);
}

void SimpleImageComputing::run() {
  if (!m_isInitialized) {
    return;
  }
  plc::gpu::TimelineSemaphore semaphore(*m_ptrContext);

  const auto result_buffer = plc::createStagingBufferFromGPU(
      *m_ptrContext, m_image.width * m_image.height * m_image.channels);
  {
    std::vector<plc::gpu::Buffer> staging_buffers;
    const auto transfer_result = setTransferCommands(staging_buffers);
    if (!transfer_result.isOk()) {
      return;
    }
    const auto compute_result = setComputeCommands(result_buffer);
    if (!compute_result.isOk()) {
      return;
    }

    m_ptrTransferCommandDriver->submit(
        plc::SubmitSemaphoreGroup{}
            .setWaitSemaphores(
                {plc::SubmitSemaphore()
                     .setSemaphore(semaphore)
                     .setValue(0u)
                     .setStageMask(plc::PipelineStage::Transfer)})
            .setSignalSemaphores(
                {plc::SubmitSemaphore()
                     .setSemaphore(semaphore)
                     .setValue(1u)
                     .setStageMask(plc::PipelineStage::Transfer)}));

    m_ptrComputeCommandDriver->submit(
        plc::SubmitSemaphoreGroup{}
            .setWaitSemaphores(
                {plc::SubmitSemaphore()
                     .setSemaphore(semaphore)
                     .setValue(1u)
                     .setStageMask(plc::PipelineStage::Transfer)})
            .setSignalSemaphores(
                {plc::SubmitSemaphore()
                     .setSemaphore(semaphore)
                     .setValue(2u)
                     .setStageMask(plc::PipelineStage::AllCommands)}));

    plc::TimelineSemaphoreDriver{}
        .setSemaphores({semaphore})
        .setValues({2u})
        .wait(*m_ptrContext);
  }

  const auto result_mapped_address = result_buffer.mapMemory(*m_ptrContext);
  const auto& image_size = m_ptrStorageImage->getGraphicalSize();

  const size_t image_buf_size = image_size.width * image_size.height * 4;
  auto image_buf = std::make_unique<unsigned char[]>(image_buf_size);
  std::memcpy(reinterpret_cast<void*>(image_buf.get()),
              result_mapped_address,
              image_buf_size);
  result_buffer.unmapMemory(*m_ptrContext);

  stbi_write_png("examples/output.png",
                 image_size.width,
                 image_size.height,
                 4,
                 image_buf.get(),
                 image_size.width * 4);
}

void SimpleImageComputing::initializeImageResources() {
  m_image.data = stbi_load("examples/core/computing_image/lenna.png",
                           &(m_image.width),
                           &(m_image.height),
                           &(m_image.channels),
                           0);

  const auto image_sub_info =
      plc::ImageSubInfo{}
          .setSize(static_cast<uint32_t>(m_image.width),
                   static_cast<uint32_t>(m_image.height),
                   1u)
          .setMipLevels(1u)
          .setArrayLayers(1u)
          .setSamples(plc::ImageSampleCount::v1)
          .setFormat(plc::DataFormat::R8G8B8A8Unorm)
          .setDimension(plc::ImageDimension::v2D);

  m_ptrImage = std::make_unique<plc::gpu::Image>(
      *m_ptrContext,
      plc::MemoryUsage::GpuOnly,
      plc::TransferType::TransferSrcDst,
      std::vector<plc::ImageUsage>{plc::ImageUsage::Sampled},
      image_sub_info);

  m_ptrStorageImage = std::make_unique<plc::gpu::Image>(
      *m_ptrContext,
      plc::MemoryUsage::GpuOnly,
      plc::TransferType::TransferSrcDst,
      std::vector<plc::ImageUsage>{plc::ImageUsage::Storage},
      image_sub_info);

  {
    const auto image_view_info =
        plc::ImageViewInfo{}
            .setAspect(plc::ImageAspect::Color)
            .setArrayRange(0u, image_sub_info.array_layers)
            .setMipRange(0u, image_sub_info.mip_levels);

    m_ptrImageView = std::make_unique<plc::gpu::ImageView>(
        *m_ptrContext, *m_ptrImage, image_view_info);

    m_ptrStorageImageView = std::make_unique<plc::gpu::ImageView>(
        *m_ptrContext, *m_ptrStorageImage, image_view_info);
  }

  {
    const auto sampler_info =
        plc::SamplerInfo{}
            .setAddressMode(plc::SamplerAddressMode::ClampToBorder)
            .setFilters(plc::SamplerFilter::Linear, plc::SamplerFilter::Linear)
            .setMipmapMode(plc::SamplerMipmapMode::Linear)
            .setLodRange(0.0f, static_cast<float_t>(image_sub_info.mip_levels))
            .setBorderColor(plc::SamplerBorderColor::FloatOpaqueWhite)
            .setUnnormalizedCoordinates(false);

    m_ptrImageSampler =
        std::make_unique<plc::gpu::Sampler>(*m_ptrContext, sampler_info);
  }
}

plc::VoidResult SimpleImageComputing::constructShaderResources() {
  PANDORA_TRY_ASSIGN(
      spirv_binary,
      plc::io::shader::read("examples/core/computing_image/simple_image.comp"));

  m_shaderModuleMap["compute"] =
      plc::gpu::ShaderModule(*m_ptrContext, spirv_binary);

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"compute"});

  m_ptrDescriptorSetLayout = std::make_unique<plc::gpu::DescriptorSetLayout>(
      *m_ptrContext, description_unit);
  m_ptrDescriptorSet = std::make_unique<plc::gpu::DescriptorSet>(
      *m_ptrContext, *m_ptrDescriptorSetLayout);

  std::vector<plc::gpu::BufferDescription> buffer_descriptions;
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("UniformNumber"),
      *m_ptrUniformBuffer);

  std::vector<plc::gpu::ImageDescription> image_descriptions;
  image_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("image"),
      *m_ptrImageView,
      plc::ImageLayout::ShaderReadOnlyOptimal,
      *m_ptrImageSampler);
  image_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("dest_image"),
      *m_ptrStorageImageView,
      plc::ImageLayout::General);

  m_ptrDescriptorSet->updateDescriptorSet(
      *m_ptrContext, buffer_descriptions, image_descriptions);

  m_ptrComputePipeline =
      std::make_unique<plc::Pipeline>(*m_ptrContext,
                                      description_unit,
                                      *m_ptrDescriptorSetLayout,
                                      plc::PipelineBind::Compute);
  m_ptrComputePipeline->constructComputePipeline(
      *m_ptrContext, m_shaderModuleMap.at("compute"));

  return plc::ok();
}

plc::VoidResult SimpleImageComputing::setTransferCommands(
    std::vector<plc::gpu::Buffer>& staging_buffers) {
  const auto command_buffer = m_ptrTransferCommandDriver->getTransfer();

  staging_buffers.push_back(plc::createStagingBufferToGPU(
      *m_ptrContext, m_image.width * m_image.height * m_image.channels));

  auto& staging_buffer = staging_buffers.back();
  const auto mapped_address = staging_buffer.mapMemory(*m_ptrContext);
  std::memcpy(mapped_address, m_image.data, staging_buffer.getSize());
  staging_buffer.unmapMemory(*m_ptrContext);

  command_buffer.begin();

  plc::ImageViewInfo image_view_info = m_ptrImageView->getImageViewInfo();

  {
    PANDORA_TRY_ASSIGN(image_barrier,
                       plc::gpu::ImageBarrierBuilder::create()
                           .setImage(*m_ptrImage)
                           .setSrcAccessFlags({plc::AccessFlag::Unknown})
                           .setDstAccessFlags({plc::AccessFlag::TransferWrite})
                           .setSrcStages({plc::PipelineStage::Transfer})
                           .setDstStages({plc::PipelineStage::Transfer})
                           .setOldLayout(plc::ImageLayout::Undefined)
                           .setNewLayout(plc::ImageLayout::TransferDstOptimal)
                           .setImageViewInfo(image_view_info)
                           .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setImageBarriers({image_barrier}));
  }

  command_buffer.copyBufferToImage(staging_buffer,
                                   *m_ptrImage,
                                   plc::ImageLayout::TransferDstOptimal,
                                   image_view_info);

  {
    PANDORA_TRY_ASSIGN(
        image_barrier,
        plc::gpu::ImageBarrierBuilder::create()
            .setImage(*m_ptrImage)
            .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
            .setDstAccessFlags({plc::AccessFlag::ShaderRead})
            .setSrcStages({plc::PipelineStage::Transfer})
            .setDstStages({plc::PipelineStage::Transfer})
            .setOldLayout(plc::ImageLayout::TransferDstOptimal)
            .setNewLayout(plc::ImageLayout::TransferDstOptimal)
            .setImageViewInfo(image_view_info)
            .setSrcQueueFamilyIndex(
                m_ptrTransferCommandDriver->getQueueFamilyIndex())
            .setDstQueueFamilyIndex(
                m_ptrComputeCommandDriver->getQueueFamilyIndex())
            .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setImageBarriers({image_barrier}));
  }

  command_buffer.end();

  return plc::ok();
}

plc::VoidResult SimpleImageComputing::setComputeCommands(
    const plc::gpu::Buffer& staging_buffer) {
  static float_t push_timer = 0.0f;
  push_timer += 0.001f;

  const auto command_buffer = m_ptrComputeCommandDriver->getCompute();

  command_buffer.begin();

  {
    const plc::ImageViewInfo image_view_info =
        m_ptrImageView->getImageViewInfo();

    PANDORA_TRY_ASSIGN(
        image_barrier,
        plc::gpu::ImageBarrierBuilder::create()
            .setImage(*m_ptrImage)
            .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
            .setDstAccessFlags({plc::AccessFlag::ShaderRead})
            .setSrcStages({plc::PipelineStage::Transfer})
            .setDstStages({plc::PipelineStage::ComputeShader})
            .setOldLayout(plc::ImageLayout::TransferDstOptimal)
            .setNewLayout(plc::ImageLayout::ShaderReadOnlyOptimal)
            .setImageViewInfo(image_view_info)
            .setSrcQueueFamilyIndex(
                m_ptrTransferCommandDriver->getQueueFamilyIndex())
            .setDstQueueFamilyIndex(
                m_ptrComputeCommandDriver->getQueueFamilyIndex())
            .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setImageBarriers({image_barrier}));
  }

  const plc::ImageViewInfo image_view_info =
      m_ptrStorageImageView->getImageViewInfo();
  {
    PANDORA_TRY_ASSIGN(image_barrier,
                       plc::gpu::ImageBarrierBuilder::create()
                           .setImage(*m_ptrStorageImage)
                           .setSrcAccessFlags({plc::AccessFlag::Unknown})
                           .setDstAccessFlags({plc::AccessFlag::ShaderWrite})
                           .setSrcStages({plc::PipelineStage::Transfer})
                           .setDstStages({plc::PipelineStage::ComputeShader})
                           .setOldLayout(plc::ImageLayout::Undefined)
                           .setNewLayout(plc::ImageLayout::General)
                           .setImageViewInfo(image_view_info)
                           .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setImageBarriers({image_barrier}));
  }

  command_buffer.pushConstants(
      *m_ptrComputePipeline, {plc::ShaderStage::Compute}, 0u, {push_timer});
  command_buffer.bindPipeline(*m_ptrComputePipeline);
  command_buffer.bindDescriptorSet(*m_ptrComputePipeline, *m_ptrDescriptorSet);
  command_buffer.compute(
      plc::ComputeWorkGroupSize{m_ptrImage->getGraphicalSize().width / 4u,
                                m_ptrImage->getGraphicalSize().height / 4u,
                                1u});

  {
    PANDORA_TRY_ASSIGN(image_barrier,
                       plc::gpu::ImageBarrierBuilder::create()
                           .setImage(*m_ptrStorageImage)
                           .setSrcAccessFlags({plc::AccessFlag::ShaderWrite})
                           .setDstAccessFlags({plc::AccessFlag::TransferRead})
                           .setSrcStages({plc::PipelineStage::ComputeShader})
                           .setDstStages({plc::PipelineStage::Transfer})
                           .setOldLayout(plc::ImageLayout::General)
                           .setNewLayout(plc::ImageLayout::General)
                           .setImageViewInfo(image_view_info)
                           .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setImageBarriers({image_barrier}));

    command_buffer.copyImageToBuffer(*m_ptrStorageImage,
                                     staging_buffer,
                                     plc::ImageLayout::General,
                                     image_view_info);
  }

  command_buffer.end();

  return plc::ok();
}

}  // namespace samples::core
