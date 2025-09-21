#include "pandora/core/command_buffer.hpp"

#include <algorithm>
#include <functional>
#include <ranges>

#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/pipeline.hpp"
#include "pandora/core/renderpass.hpp"

void pandora::core::CommandBuffer::begin(
    const CommandBeginInfo& command_begin_info) const {
  using vk_helper::getCommandBufferUsageFlagBits;

  vk::CommandBufferBeginInfo begin_info{};
  vk::CommandBufferInheritanceInfo inheritance_info{};

  if (m_isSecondary) {
    inheritance_info.setRenderPass(command_begin_info.getRenderPass())
        .setSubpass(command_begin_info.subpass_index)
        .setFramebuffer(command_begin_info.getFramebuffer())
        .setQueryFlags(vk::QueryControlFlags{0u})
        .setPipelineStatistics(vk::QueryPipelineStatisticFlags{0u})
        .setOcclusionQueryEnable(VK_FALSE);

    begin_info.setPInheritanceInfo(&inheritance_info);
  } else {
    begin_info.setPInheritanceInfo(nullptr);
  }

  begin_info.setFlags(getCommandBufferUsageFlagBits(command_begin_info.usage));
  m_commandBuffer.begin(begin_info);
}

void pandora::core::CommandBuffer::end() const {
  m_commandBuffer.end();
}

void pandora::core::CommandBuffer::setPipelineBarrier(
    const gpu::BufferBarrier& barrier,
    PipelineStage src_stage,
    PipelineStage dst_stage) const {
  m_commandBuffer.pipelineBarrier(
      vk_helper::getPipelineStageFlagBits(src_stage),
      vk_helper::getPipelineStageFlagBits(dst_stage),
      vk::DependencyFlagBits{0u},
      nullptr,
      barrier.getBarrier(),
      nullptr);
}

void pandora::core::CommandBuffer::setPipelineBarrier(
    const gpu::ImageBarrier& barrier,
    PipelineStage src_stage,
    PipelineStage dst_stage) const {
  m_commandBuffer.pipelineBarrier(
      vk_helper::getPipelineStageFlagBits(src_stage),
      vk_helper::getPipelineStageFlagBits(dst_stage),
      vk::DependencyFlagBits{0u},
      nullptr,
      nullptr,
      barrier.getBarrier());
}

void pandora::core::CommandBuffer::bindPipeline(
    const Pipeline& pipeline) const {
  m_commandBuffer.bindPipeline(pipeline.getBindPoint(), pipeline.getPipeline());
}

void pandora::core::CommandBuffer::bindDescriptorSet(
    const Pipeline& pipeline, const gpu::DescriptorSet& descriptor_set) const {
  m_commandBuffer.bindDescriptorSets(pipeline.getBindPoint(),
                                     pipeline.getPipelineLayout(),
                                     0u,
                                     descriptor_set.getDescriptorSet(),
                                     {});
}

void pandora::core::CommandBuffer::pushConstants(
    const Pipeline& pipeline,
    const std::vector<ShaderStage>& dst_stages,
    uint32_t offset,
    const std::vector<float_t>& data) const {
  m_commandBuffer.pushConstants(
      pipeline.getPipelineLayout(),
      std::ranges::fold_left(
          dst_stages | std::views::transform(vk_helper::getShaderStageFlagBits),
          vk::ShaderStageFlags{},
          std::bit_or{}),
      offset,
      static_cast<uint32_t>(sizeof(float_t) * data.size()),
      data.data());
}

void pandora::core::CommandBuffer::resetCommands() const {
  m_commandBuffer.reset(vk::CommandBufferResetFlags{});
}

void pandora::core::TransferCommandBuffer::copyBuffer(
    const gpu::Buffer& staging_buffer, const gpu::Buffer& dst_buffer) const {
  m_commandBuffer.copyBuffer(
      staging_buffer.getBuffer(),
      dst_buffer.getBuffer(),
      vk::BufferCopy{}.setSize(staging_buffer.getSize()));
}

void pandora::core::TransferCommandBuffer::copyBufferToImage(
    const gpu::Buffer& buffer,
    const gpu::Image& image,
    ImageLayout image_layout,
    const ImageViewInfo& image_view_info) const {
  const auto& graphical_size = image.getGraphicalSize();

  const auto copy_region =
      vk::BufferImageCopy()
          .setImageSubresource(
              vk::ImageSubresourceLayers{}
                  .setAspectMask(
                      vk_helper::getImageAspectFlags(image_view_info.aspect))
                  .setMipLevel(image_view_info.base_mip_level)
                  .setBaseArrayLayer(image_view_info.base_array_layer)
                  .setLayerCount(image_view_info.array_layers))
          .setImageOffset({0u, 0u, 0u})
          .setBufferRowLength(graphical_size.width)
          .setBufferImageHeight(graphical_size.height)
          .setImageExtent(vk_helper::getExtent3D(graphical_size));

  vk::ImageLayout vk_image_layout = vk_helper::getImageLayout(image_layout);
  if (vk_image_layout != vk::ImageLayout::eTransferDstOptimal
      && vk_image_layout != vk::ImageLayout::eGeneral
      && vk_image_layout != vk::ImageLayout::eSharedPresentKHR) {
    vk_image_layout = vk::ImageLayout::eTransferDstOptimal;
  }

  m_commandBuffer.copyBufferToImage(
      buffer.getBuffer(), image.getImage(), vk_image_layout, copy_region);
}

void pandora::core::TransferCommandBuffer::copyImageToBuffer(
    const gpu::Image& image,
    const gpu::Buffer& buffer,
    ImageLayout image_layout,
    const ImageViewInfo& image_view_info) const {
  const auto& graphical_size = image.getGraphicalSize();

  const auto copy_region =
      vk::BufferImageCopy()
          .setImageSubresource(
              vk::ImageSubresourceLayers()
                  .setAspectMask(
                      vk_helper::getImageAspectFlags(image_view_info.aspect))
                  .setMipLevel(image_view_info.base_mip_level)
                  .setBaseArrayLayer(image_view_info.base_array_layer)
                  .setLayerCount(image_view_info.array_layers))
          .setImageOffset({0u, 0u, 0u})
          .setBufferRowLength(graphical_size.width)
          .setBufferImageHeight(graphical_size.height)
          .setImageExtent(vk_helper::getExtent3D(graphical_size));

  vk::ImageLayout vk_image_layout = vk_helper::getImageLayout(image_layout);
  if (vk_image_layout != vk::ImageLayout::eTransferSrcOptimal
      && vk_image_layout != vk::ImageLayout::eGeneral
      && vk_image_layout != vk::ImageLayout::eSharedPresentKHR) {
    vk_image_layout = vk::ImageLayout::eTransferSrcOptimal;
  }

  m_commandBuffer.copyImageToBuffer(
      image.getImage(), vk_image_layout, buffer.getBuffer(), copy_region);
}

void pandora::core::TransferCommandBuffer::setMipmaps(
    const gpu::Image& image, PipelineStage dst_stage) const {
  const auto image_view_info =
      ImageViewInfo{}
          .setAspect(pandora::core::ImageAspect::Color)
          .setMipRange(0u, 1u)  // for sending each mip level
          .setArrayRange(0u, 1u);

  const auto src_image_barrier =
      gpu::ImageBarrierBuilder::create()
          .setImage(image)
          .setPriorityAccessFlags(std::vector{AccessFlag::TransferWrite})
          .setWaitAccessFlags(std::vector{AccessFlag::TransferRead})
          .setOldLayout(ImageLayout::TransferDstOptimal)
          .setNewLayout(ImageLayout::TransferSrcOptimal)
          .setImageViewInfo(image_view_info)
          .build();

  const auto dst_image_barrier =
      gpu::ImageBarrierBuilder::create()
          .setImage(image)
          .setPriorityAccessFlags(std::vector{AccessFlag::TransferRead})
          .setWaitAccessFlags(std::vector{AccessFlag::ShaderRead})
          .setOldLayout(ImageLayout::TransferSrcOptimal)
          .setNewLayout(ImageLayout::ShaderReadOnlyOptimal)
          .setImageViewInfo(image_view_info)
          .build();

  vk::ImageMemoryBarrier src_barrier = src_image_barrier.getBarrier();
  vk::ImageMemoryBarrier dst_barrier = dst_image_barrier.getBarrier();

  if (dst_stage == PipelineStage::Transfer) {
    dst_barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
  }
  if (dst_stage == PipelineStage::BottomOfPipe) {
    dst_barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
  }

  uint32_t mip_width = image.getGraphicalSize().width;
  uint32_t mip_height = image.getGraphicalSize().height;

  uint32_t mip_level = 1u;
  for (; mip_level < image_view_info.mip_levels; mip_level += 1u) {
    src_barrier.subresourceRange.setBaseMipLevel(mip_level - 1u);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                    vk::PipelineStageFlagBits::eTransfer,
                                    vk::DependencyFlagBits{0u},
                                    nullptr,
                                    nullptr,
                                    src_barrier);

    const auto blit =
        vk::ImageBlit()
            .setSrcSubresource(
                vk::ImageSubresourceLayers()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(mip_level - 1u)
                    .setBaseArrayLayer(0u)
                    .setLayerCount(1u))
            .setSrcOffsets({vk::Offset3D(0u, 0u, 0u),
                            vk::Offset3D(mip_width, mip_height, 1u)})
            .setDstSubresource(
                vk::ImageSubresourceLayers()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(mip_level)
                    .setBaseArrayLayer(0u)
                    .setLayerCount(1u))
            .setDstOffsets({vk::Offset3D(0u, 0u, 0u),
                            vk::Offset3D(std::max(1u, mip_width / 2U),
                                         std::max(1u, mip_height / 2U),
                                         1u)});

    m_commandBuffer.blitImage(image.getImage(),
                              vk::ImageLayout::eTransferSrcOptimal,
                              image.getImage(),
                              vk::ImageLayout::eTransferDstOptimal,
                              blit,
                              vk::Filter::eLinear);

    dst_barrier.subresourceRange.setBaseMipLevel(mip_level - 1u);
    m_commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk_helper::getPipelineStageFlagBits(dst_stage),
        vk::DependencyFlagBits{0u},
        nullptr,
        nullptr,
        dst_barrier);

    mip_width = std::max(1u, mip_width / 2U);
    mip_height = std::max(1u, mip_height / 2U);
  }

  dst_barrier.subresourceRange.setBaseMipLevel(mip_level - 1);
  dst_barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);

  m_commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer,
      vk_helper::getPipelineStageFlagBits(dst_stage),
      vk::DependencyFlagBits{0u},
      nullptr,
      nullptr,
      dst_barrier);
}

void pandora::core::TransferCommandBuffer::transferMipmapImages(
    const gpu::Image& image,
    const PipelineStage src_stage,
    const PipelineStage dst_stage,
    std::pair<uint32_t, uint32_t> queue_family_index) const {
  const auto image_view_info = ImageViewInfo{}
                                   .setAspect(pandora::core::ImageAspect::Color)
                                   .setMipRange(0u, 1u)
                                   .setArrayRange(0u, 1u);

  const auto image_barrier =
      gpu::ImageBarrierBuilder::create()
          .setImage(image)
          .setPriorityAccessFlags(std::vector{AccessFlag::TransferWrite})
          .setWaitAccessFlags(std::vector{AccessFlag::TransferRead})
          .setOldLayout(ImageLayout::TransferDstOptimal)
          .setNewLayout(ImageLayout::TransferDstOptimal)
          .setImageViewInfo(image_view_info)
          .build();

  vk::ImageMemoryBarrier barrier = image_barrier.getBarrier();
  barrier.setSrcQueueFamilyIndex(queue_family_index.first)
      .setDstQueueFamilyIndex(queue_family_index.second);

  for (uint32_t mip_level = 1u; mip_level <= image.getMipLevels();
       mip_level += 1u) {
    barrier.subresourceRange.setBaseMipLevel(mip_level - 1u);

    m_commandBuffer.pipelineBarrier(
        vk_helper::getPipelineStageFlagBits(src_stage),
        vk_helper::getPipelineStageFlagBits(dst_stage),
        vk::DependencyFlagBits{0u},
        nullptr,
        nullptr,
        barrier);
  }
}

void pandora::core::TransferCommandBuffer::acquireMipmapImages(
    const gpu::Image& image,
    const PipelineStage src_stage,
    const PipelineStage dst_stage,
    std::pair<uint32_t, uint32_t> queue_family_index) const {
  const auto image_view_info = ImageViewInfo{}
                                   .setAspect(pandora::core::ImageAspect::Color)
                                   .setBaseMipLevel(0u)
                                   .setMipLevels(1u)
                                   .setBaseArrayLayer(0u)
                                   .setArrayLayers(1u);

  const auto image_barrier =
      gpu::ImageBarrierBuilder::create()
          .setImage(image)
          .setPriorityAccessFlags(std::vector{AccessFlag::TransferWrite})
          .setWaitAccessFlags(std::vector{AccessFlag::TransferRead})
          .setOldLayout(ImageLayout::TransferDstOptimal)
          .setNewLayout(ImageLayout::ShaderReadOnlyOptimal)
          .setImageViewInfo(image_view_info)
          .build();

  vk::ImageMemoryBarrier barrier = image_barrier.getBarrier();
  barrier.setSrcQueueFamilyIndex(queue_family_index.first);
  barrier.setDstQueueFamilyIndex(queue_family_index.second);

  for (uint32_t mip_level = 1u; mip_level <= image.getMipLevels();
       mip_level += 1u) {
    barrier.subresourceRange.setBaseMipLevel(mip_level - 1u);

    m_commandBuffer.pipelineBarrier(
        vk_helper::getPipelineStageFlagBits(src_stage),
        vk_helper::getPipelineStageFlagBits(dst_stage),
        vk::DependencyFlagBits{0u},
        nullptr,
        nullptr,
        barrier);
  }
}

void pandora::core::ComputeCommandBuffer::compute(
    const ComputeWorkGroupSize& work_group_size) const {
  m_commandBuffer.dispatch(
      work_group_size.x, work_group_size.y, work_group_size.z);
}

void pandora::core::GraphicCommandBuffer::setScissor(
    const gpu_ui::GraphicalSize<uint32_t>& size) const {
  m_commandBuffer.setScissor(
      0u,
      vk::Rect2D().setOffset({0u, 0u}).setExtent(vk_helper::getExtent2D(size)));
}

void pandora::core::GraphicCommandBuffer::setViewport(
    const gpu_ui::GraphicalSize<float_t>& size,
    float_t min_depth,
    float_t max_depth) const {
  m_commandBuffer.setViewport(0u,
                              vk::Viewport{}
                                  .setX(0.0f)
                                  .setY(0.0f)
                                  .setWidth(static_cast<float_t>(size.width))
                                  .setHeight(static_cast<float_t>(size.height))
                                  .setMinDepth(min_depth)
                                  .setMaxDepth(max_depth));
}

void pandora::core::GraphicCommandBuffer::bindVertexBuffer(
    const gpu::Buffer& buffer, const uint32_t& offset) const {
  m_commandBuffer.bindVertexBuffers(0u, buffer.getBuffer(), offset);
}

void pandora::core::GraphicCommandBuffer::bindIndexBuffer(
    const gpu::Buffer& buffer, const uint32_t& offset) const {
  m_commandBuffer.bindIndexBuffer(
      buffer.getBuffer(), offset, vk::IndexType::eUint32);
}

void pandora::core::GraphicCommandBuffer::draw(uint32_t vertex_count,
                                               uint32_t instance_count,
                                               uint32_t first_vertex,
                                               uint32_t first_instance) const {
  m_commandBuffer.draw(
      vertex_count, instance_count, first_vertex, first_instance);
}

void pandora::core::GraphicCommandBuffer::drawIndexed(
    uint32_t index_count,
    uint32_t instance_count,
    uint32_t first_index,
    int32_t vertex_offset,
    uint32_t first_instance) const {
  m_commandBuffer.drawIndexed(
      index_count, instance_count, first_index, vertex_offset, first_instance);
}

void pandora::core::GraphicCommandBuffer::beginRenderpass(
    const RenderKit& render_kit,
    const gpu_ui::GraphicalSize<uint32_t>& render_area,
    const SubpassContents subpass_contents) const {
  if (m_isSecondary) {
    throw std::runtime_error(
        "This command buffer is secondary. You can't use this function.");
  }

  const auto render_pass_info =
      vk::RenderPassBeginInfo()
          .setRenderPass(render_kit.getRenderpass().getRenderPass())
          .setFramebuffer(render_kit.getFramebuffer().getFrameBuffer())
          .setRenderArea(
              vk::Rect2D{{0u, 0u}, vk_helper::getExtent2D(render_area)})
          .setClearValues(render_kit.getClearValues());

  m_commandBuffer.beginRenderPass(
      render_pass_info, vk_helper::getSubpassContents(subpass_contents));
}

void pandora::core::GraphicCommandBuffer::endRenderpass() const {
  m_commandBuffer.endRenderPass();
}

void pandora::core::GraphicCommandBuffer::nextSubpass(
    const SubpassContents subpass_contents) const {
  m_commandBuffer.nextSubpass(vk_helper::getSubpassContents(subpass_contents));
}
