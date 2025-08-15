#include "pandora/core/command_buffer.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>

#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/pipeline.hpp"
#include "pandora/core/renderpass.hpp"

void pandora::core::CommandBuffer::begin(const CommandBeginInfo& command_begin_info) const {
  using vk_helper::getCommandBufferUsageFlagBits;

  vk::CommandBufferBeginInfo begin_info;
  vk::CommandBufferInheritanceInfo inheritance_info;

  if (m_isSecondary) {
    inheritance_info.setRenderPass(command_begin_info.getRenderPass());
    inheritance_info.setSubpass(command_begin_info.subpass_index);
    inheritance_info.setFramebuffer(command_begin_info.getFramebuffer());
    inheritance_info.setQueryFlags(vk::QueryControlFlags(0U));
    inheritance_info.setPipelineStatistics(vk::QueryPipelineStatisticFlags(0U));
    inheritance_info.setOcclusionQueryEnable(VK_FALSE);

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

void pandora::core::CommandBuffer::setPipelineBarrier(const gpu::BufferBarrier& barrier,
                                                      const PipelineStage src_stage,
                                                      const PipelineStage dst_stage) const {
  m_commandBuffer.pipelineBarrier(vk_helper::getPipelineStageFlagBits(src_stage),
                                  vk_helper::getPipelineStageFlagBits(dst_stage),
                                  vk::DependencyFlagBits(0U),
                                  nullptr,
                                  barrier.getBarrier(),
                                  nullptr);
}

void pandora::core::CommandBuffer::setPipelineBarrier(const gpu::ImageBarrier& barrier,
                                                      const PipelineStage src_stage,
                                                      const PipelineStage dst_stage) const {
  m_commandBuffer.pipelineBarrier(vk_helper::getPipelineStageFlagBits(src_stage),
                                  vk_helper::getPipelineStageFlagBits(dst_stage),
                                  vk::DependencyFlagBits(0U),
                                  nullptr,
                                  nullptr,
                                  barrier.getBarrier());
}

void pandora::core::CommandBuffer::bindPipeline(const Pipeline& pipeline) const {
  m_commandBuffer.bindPipeline(pipeline.getBindPoint(), pipeline.getPipeline());
}

void pandora::core::CommandBuffer::bindDescriptorSet(const Pipeline& pipeline,
                                                     const gpu::DescriptorSet& descriptor_set) const {
  m_commandBuffer.bindDescriptorSets(
      pipeline.getBindPoint(), pipeline.getPipelineLayout(), 0U, descriptor_set.getDescriptorSet(), {});
}

void pandora::core::CommandBuffer::pushConstants(const Pipeline& pipeline,
                                                 const std::vector<ShaderStage>& dst_stages,
                                                 const uint32_t offset,
                                                 const std::vector<float_t>& data) const {
  m_commandBuffer.pushConstants(
      pipeline.getPipelineLayout(),
      std::ranges::fold_left(
          dst_stages | std::views::transform(vk_helper::getShaderStageFlagBits), vk::ShaderStageFlags{}, std::bit_or{}),
      offset,
      static_cast<uint32_t>(sizeof(float_t) * data.size()),
      data.data());
}

void pandora::core::CommandBuffer::resetCommands() const {
  m_commandBuffer.reset(vk::CommandBufferResetFlags());
}

void pandora::core::TransferCommandBuffer::copyBuffer(const gpu::Buffer& staging_buffer,
                                                      const gpu::Buffer& dst_buffer) const {
  vk::BufferCopy copy_region;
  copy_region.setSize(staging_buffer.getSize());

  m_commandBuffer.copyBuffer(staging_buffer.getBuffer(), dst_buffer.getBuffer(), copy_region);
}

void pandora::core::TransferCommandBuffer::copyBufferToImage(const gpu::Buffer& buffer,
                                                             const gpu::Image& image,
                                                             const ImageLayout image_layout,
                                                             const ImageViewInfo& image_view_info) const {
  vk::BufferImageCopy copy_region;

  {
    vk::ImageSubresourceLayers subresource;

    subresource.setAspectMask(vk_helper::getImageAspectFlags(image_view_info.aspect));
    subresource.setMipLevel(image_view_info.base_mip_level);
    subresource.setBaseArrayLayer(image_view_info.base_array_layer);
    subresource.setLayerCount(image_view_info.array_layers);

    copy_region.setImageSubresource(subresource);
  }

  copy_region.setImageOffset({0U, 0U, 0U});

  const auto& graphical_size = image.getGraphicalSize();
  copy_region.setBufferRowLength(graphical_size.width);
  copy_region.setBufferImageHeight(graphical_size.height);
  copy_region.setImageExtent({graphical_size.width, graphical_size.height, graphical_size.depth});

  vk::ImageLayout vk_image_layout = vk_helper::getImageLayout(image_layout);
  if (vk_image_layout != vk::ImageLayout::eTransferDstOptimal && vk_image_layout != vk::ImageLayout::eGeneral
      && vk_image_layout != vk::ImageLayout::eSharedPresentKHR) {
    vk_image_layout = vk::ImageLayout::eTransferDstOptimal;
  }

  m_commandBuffer.copyBufferToImage(buffer.getBuffer(), image.getImage(), vk_image_layout, copy_region);
}

void pandora::core::TransferCommandBuffer::copyImageToBuffer(const gpu::Image& image,
                                                             const gpu::Buffer& buffer,
                                                             const ImageLayout image_layout,
                                                             const ImageViewInfo& image_view_info) const {
  vk::BufferImageCopy copy_region;

  {
    vk::ImageSubresourceLayers subresource;

    subresource.setAspectMask(vk_helper::getImageAspectFlags(image_view_info.aspect));
    subresource.setMipLevel(image_view_info.base_mip_level);
    subresource.setBaseArrayLayer(image_view_info.base_array_layer);
    subresource.setLayerCount(image_view_info.array_layers);

    copy_region.setImageSubresource(subresource);
  }

  copy_region.setImageOffset({0U, 0U, 0U});

  const auto& graphical_size = image.getGraphicalSize();
  copy_region.setBufferRowLength(graphical_size.width);
  copy_region.setBufferImageHeight(graphical_size.height);
  copy_region.setImageExtent({graphical_size.width, graphical_size.height, graphical_size.depth});

  vk::ImageLayout vk_image_layout = vk_helper::getImageLayout(image_layout);
  if (vk_image_layout != vk::ImageLayout::eTransferSrcOptimal && vk_image_layout != vk::ImageLayout::eGeneral
      && vk_image_layout != vk::ImageLayout::eSharedPresentKHR) {
    vk_image_layout = vk::ImageLayout::eTransferSrcOptimal;
  }

  m_commandBuffer.copyImageToBuffer(image.getImage(), vk_image_layout, buffer.getBuffer(), copy_region);
}

void pandora::core::TransferCommandBuffer::setMipmaps(const gpu::Image& image, const PipelineStage dst_stage) const {
  ImageViewInfo image_view_info{};
  image_view_info.aspect = pandora::core::ImageAspect::Color;
  image_view_info.base_mip_level = 0U;
  image_view_info.mip_levels = 1U;  // for sending each mip level
  image_view_info.base_array_layer = 0U;
  image_view_info.array_layers = 1U;

  const gpu::ImageBarrier src_image_barrier(image,
                                            {AccessFlag::TransferWrite},
                                            {AccessFlag::TransferRead},
                                            ImageLayout::TransferDstOptimal,
                                            ImageLayout::TransferSrcOptimal,
                                            image_view_info);

  const gpu::ImageBarrier dst_image_barrier(image,
                                            {AccessFlag::TransferRead},
                                            {AccessFlag::ShaderRead},
                                            ImageLayout::TransferSrcOptimal,
                                            ImageLayout::ShaderReadOnlyOptimal,
                                            image_view_info);

  vk::ImageMemoryBarrier src_barrier = src_image_barrier.getBarrier();
  vk::ImageMemoryBarrier dst_barrier = dst_image_barrier.getBarrier();

  if (dst_stage == PipelineStage::Transfer) {
    dst_barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
    dst_barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
  }
  if (dst_stage == PipelineStage::BottomOfPipe) {
    dst_barrier.setNewLayout(vk::ImageLayout::eTransferDstOptimal);
  }

  uint32_t mip_width = image.getGraphicalSize().width;
  uint32_t mip_height = image.getGraphicalSize().height;

  uint32_t mip_level = 1U;
  for (; mip_level < image_view_info.mip_levels; mip_level += 1U) {
    src_barrier.subresourceRange.setBaseMipLevel(mip_level - 1U);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                    vk::PipelineStageFlagBits::eTransfer,
                                    vk::DependencyFlagBits(0U),
                                    nullptr,
                                    nullptr,
                                    src_barrier);

    vk::ImageBlit blit;
    {
      vk::ImageSubresourceLayers subresource;

      subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
      subresource.setMipLevel(mip_level - 1U);
      subresource.setBaseArrayLayer(0U);
      subresource.setLayerCount(1U);
      blit.setSrcSubresource(subresource);
      blit.setSrcOffsets({vk::Offset3D(0U, 0U, 0U),
                          vk::Offset3D(static_cast<int32_t>(mip_width), static_cast<int32_t>(mip_height), 1U)});
    }
    {
      vk::ImageSubresourceLayers subresource;

      subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
      subresource.setMipLevel(mip_level);
      subresource.setBaseArrayLayer(0);
      subresource.setLayerCount(1);
      blit.setDstSubresource(subresource);
      blit.setDstOffsets({vk::Offset3D(0U, 0U, 0U),
                          vk::Offset3D(static_cast<int32_t>(std::max(1U, mip_width / 2U)),
                                       static_cast<int32_t>(std::max(1U, mip_height / 2U)),
                                       1U)});
    }

    m_commandBuffer.blitImage(image.getImage(),
                              vk::ImageLayout::eTransferSrcOptimal,
                              image.getImage(),
                              vk::ImageLayout::eTransferDstOptimal,
                              blit,
                              vk::Filter::eLinear);

    dst_barrier.subresourceRange.setBaseMipLevel(mip_level - 1U);

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                    vk_helper::getPipelineStageFlagBits(dst_stage),
                                    vk::DependencyFlagBits(0U),
                                    nullptr,
                                    nullptr,
                                    dst_barrier);

    mip_width = std::max(1U, mip_width / 2U);
    mip_height = std::max(1U, mip_height / 2U);
  }

  dst_barrier.subresourceRange.setBaseMipLevel(mip_level - 1);
  dst_barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);

  m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                  vk_helper::getPipelineStageFlagBits(dst_stage),
                                  vk::DependencyFlagBits(0),
                                  nullptr,
                                  nullptr,
                                  dst_barrier);
}

void pandora::core::TransferCommandBuffer::transferMipmapImages(
    const gpu::Image& image,
    const PipelineStage src_stage,
    const PipelineStage dst_stage,
    std::pair<uint32_t, uint32_t> queue_family_index) const {
  ImageViewInfo image_view_info{};
  image_view_info.aspect = pandora::core::ImageAspect::Color;
  image_view_info.base_mip_level = 0U;
  image_view_info.mip_levels = 1U;
  image_view_info.base_array_layer = 0U;
  image_view_info.array_layers = 1U;

  const gpu::ImageBarrier image_barrier(image,
                                        {AccessFlag::TransferWrite},
                                        {AccessFlag::ShaderRead},
                                        ImageLayout::TransferDstOptimal,
                                        ImageLayout::TransferDstOptimal,
                                        image_view_info);

  vk::ImageMemoryBarrier barrier = image_barrier.getBarrier();
  barrier.setSrcQueueFamilyIndex(queue_family_index.first);
  barrier.setDstQueueFamilyIndex(queue_family_index.second);

  for (uint32_t mip_level = 1U; mip_level <= image.getMipLevels(); mip_level += 1U) {
    barrier.subresourceRange.setBaseMipLevel(mip_level - 1U);

    m_commandBuffer.pipelineBarrier(vk_helper::getPipelineStageFlagBits(src_stage),
                                    vk_helper::getPipelineStageFlagBits(dst_stage),
                                    vk::DependencyFlagBits(0U),
                                    nullptr,
                                    nullptr,
                                    barrier);
  }
}

void pandora::core::TransferCommandBuffer::acquireMipmapImages(const gpu::Image& image,
                                                               const PipelineStage src_stage,
                                                               const PipelineStage dst_stage,
                                                               std::pair<uint32_t, uint32_t> queue_family_index) const {
  ImageViewInfo image_view_info{};
  image_view_info.aspect = pandora::core::ImageAspect::Color;
  image_view_info.base_mip_level = 0U;
  image_view_info.mip_levels = 1U;
  image_view_info.base_array_layer = 0U;
  image_view_info.array_layers = 1U;

  const gpu::ImageBarrier image_barrier(image,
                                        {AccessFlag::TransferWrite},
                                        {AccessFlag::ShaderRead},
                                        ImageLayout::TransferDstOptimal,
                                        ImageLayout::ShaderReadOnlyOptimal,
                                        image_view_info);

  vk::ImageMemoryBarrier barrier = image_barrier.getBarrier();
  barrier.setSrcQueueFamilyIndex(queue_family_index.first);
  barrier.setDstQueueFamilyIndex(queue_family_index.second);

  for (uint32_t mip_level = 1U; mip_level <= image.getMipLevels(); mip_level += 1U) {
    barrier.subresourceRange.setBaseMipLevel(mip_level - 1U);

    m_commandBuffer.pipelineBarrier(vk_helper::getPipelineStageFlagBits(src_stage),
                                    vk_helper::getPipelineStageFlagBits(dst_stage),
                                    vk::DependencyFlagBits(0U),
                                    nullptr,
                                    nullptr,
                                    barrier);
  }
}

void pandora::core::ComputeCommandBuffer::compute(const ComputeWorkGroupSize& work_group_size) const {
  m_commandBuffer.dispatch(work_group_size.x, work_group_size.y, work_group_size.z);
}

void pandora::core::GraphicCommandBuffer::setScissor(const gpu_ui::GraphicalSize<uint32_t>& size) const {
  vk::Rect2D scissor;
  scissor.setOffset({0U, 0U});
  scissor.setExtent({size.width, size.height});

  m_commandBuffer.setScissor(0U, scissor);
}

void pandora::core::GraphicCommandBuffer::setViewport(const gpu_ui::GraphicalSize<float_t>& size,
                                                      const float_t min_depth,
                                                      const float_t max_depth) const {
  vk::Viewport viewport;
  viewport.setX(0.0F);
  viewport.setY(0.0F);
  viewport.setWidth(static_cast<float_t>(size.width));
  viewport.setHeight(static_cast<float_t>(size.height));
  viewport.setMinDepth(min_depth);
  viewport.setMaxDepth(max_depth);

  m_commandBuffer.setViewport(0U, viewport);
}

void pandora::core::GraphicCommandBuffer::bindVertexBuffer(const gpu::Buffer& buffer, const uint32_t& offset) const {
  m_commandBuffer.bindVertexBuffers(0U, buffer.getBuffer(), offset);
}

void pandora::core::GraphicCommandBuffer::bindIndexBuffer(const gpu::Buffer& buffer, const uint32_t& offset) const {
  m_commandBuffer.bindIndexBuffer(buffer.getBuffer(), offset, vk::IndexType::eUint32);
}

void pandora::core::GraphicCommandBuffer::draw(const uint32_t vertex_count,
                                               const uint32_t instance_count,
                                               const uint32_t first_vertex,
                                               const uint32_t first_instance) const {
  m_commandBuffer.draw(vertex_count, instance_count, first_vertex, first_instance);
}

void pandora::core::GraphicCommandBuffer::drawIndexed(const uint32_t index_count,
                                                      const uint32_t instance_count,
                                                      const uint32_t first_index,
                                                      const int32_t vertex_offset,
                                                      const uint32_t first_instance) const {
  m_commandBuffer.drawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
}

void pandora::core::GraphicCommandBuffer::beginRenderpass(const RenderKit& render_kit,
                                                          const gpu_ui::GraphicalSize<uint32_t>& render_area,
                                                          const SubpassContents subpass_contents) const {
  if (m_isSecondary) {
    throw std::runtime_error("This command buffer is secondary. You can't use this function.");
  }

  vk::RenderPassBeginInfo render_pass_info;
  render_pass_info.setRenderPass(render_kit.getRenderpass().getRenderPass());
  render_pass_info.setFramebuffer(render_kit.getFramebuffer().getFrameBuffer());
  render_pass_info.setRenderArea(vk::Rect2D({0U, 0U}, {render_area.width, render_area.height}));
  render_pass_info.setClearValues(render_kit.getClearValues());

  using vk_helper::getSubpassContents;
  m_commandBuffer.beginRenderPass(render_pass_info, getSubpassContents(subpass_contents));
}

void pandora::core::GraphicCommandBuffer::endRenderpass() const {
  m_commandBuffer.endRenderPass();
}

void pandora::core::GraphicCommandBuffer::nextSubpass(const SubpassContents subpass_contents) const {
  using vk_helper::getSubpassContents;
  m_commandBuffer.nextSubpass(getSubpassContents(subpass_contents));
}
