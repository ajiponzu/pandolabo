#include <algorithm>
#include <functional>
#include <ranges>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

namespace {

constexpr auto g_lamda_convert_access_flags =
    [](const std::vector<pandora::core::AccessFlag>& access_flags) {
      return std::ranges::fold_left(
          access_flags | std::views::transform(vk_helper::getAccessFlagBits),
          vk::AccessFlags2{},
          std::bit_or());
    };

constexpr auto g_lamda_convert_stage_flags =
    [](const std::vector<pandora::core::PipelineStage>& stages) {
      return std::ranges::fold_left(
          stages | std::views::transform(vk_helper::getPipelineStageFlagBits),
          vk::PipelineStageFlags2{},
          std::bit_or());
    };

}  // namespace

namespace pandora::core::gpu {

MemoryBarrier::MemoryBarrier(const std::vector<AccessFlag>& src_access_flags,
                             const std::vector<AccessFlag>& dst_access_flags,
                             const std::vector<PipelineStage>& src_stages,
                             const std::vector<PipelineStage>& dst_stages) {
  m_memoryBarrier
      .setSrcAccessMask(g_lamda_convert_access_flags(src_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(dst_access_flags))
      .setSrcStageMask(g_lamda_convert_stage_flags(src_stages))
      .setDstStageMask(g_lamda_convert_stage_flags(dst_stages));
}

MemoryBarrier::~MemoryBarrier() {}

BufferBarrier::BufferBarrier(const Buffer& buffer,
                             const std::vector<AccessFlag>& src_access_flags,
                             const std::vector<AccessFlag>& dst_access_flags,
                             const std::vector<PipelineStage>& src_stages,
                             const std::vector<PipelineStage>& dst_stages,
                             uint32_t src_queue_family,
                             uint32_t dst_queue_family) {
  m_bufferMemoryBarrier.setBuffer(buffer.getBuffer())
      .setSize(buffer.getSize())
      .setSrcAccessMask(g_lamda_convert_access_flags(src_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(dst_access_flags))
      .setSrcStageMask(g_lamda_convert_stage_flags(src_stages))
      .setDstStageMask(g_lamda_convert_stage_flags(dst_stages))
      .setSrcQueueFamilyIndex(src_queue_family)
      .setDstQueueFamilyIndex(dst_queue_family);
}

BufferBarrier::~BufferBarrier() {}

ImageBarrier::ImageBarrier(const Image& image,
                           const std::vector<AccessFlag>& src_access_flags,
                           const std::vector<AccessFlag>& dst_access_flags,
                           const std::vector<PipelineStage>& src_stages,
                           const std::vector<PipelineStage>& dst_stages,
                           ImageLayout old_layout,
                           ImageLayout new_layout,
                           const ImageViewInfo& image_view_info,
                           uint32_t src_queue_family,
                           uint32_t dst_queue_family) {
  vk::ImageSubresourceRange subresource_range{};

  switch (image_view_info.aspect) {
    using enum ImageAspect;
    using enum vk::ImageAspectFlagBits;

    case Color:
      subresource_range.setAspectMask(eColor);
      break;
    case Depth:
      subresource_range.setAspectMask(eDepth);
      break;
    case Stencil:
      subresource_range.setAspectMask(eStencil);
      break;
    case DepthStencil:
      subresource_range.setAspectMask(eDepth | eStencil);
      break;
    default:
      break;
  }

  subresource_range.setBaseMipLevel(image_view_info.base_mip_level)
      .setLevelCount(image_view_info.mip_levels)
      .setBaseArrayLayer(image_view_info.base_array_layer)
      .setLayerCount(image_view_info.array_layers);

  m_imageMemoryBarrier.setImage(image.getImage())
      .setSrcAccessMask(g_lamda_convert_access_flags(src_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(dst_access_flags))
      .setSrcStageMask(g_lamda_convert_stage_flags(src_stages))
      .setDstStageMask(g_lamda_convert_stage_flags(dst_stages))
      .setOldLayout(vk_helper::getImageLayout(old_layout))
      .setNewLayout(vk_helper::getImageLayout(new_layout))
      .setSubresourceRange(subresource_range)
      .setSrcQueueFamilyIndex(src_queue_family)
      .setDstQueueFamilyIndex(dst_queue_family);
}

ImageBarrier::ImageBarrier(const std::unique_ptr<Context>& ptr_context,
                           const std::vector<AccessFlag>& src_access_flags,
                           const std::vector<AccessFlag>& dst_access_flags,
                           const std::vector<PipelineStage>& src_stages,
                           const std::vector<PipelineStage>& dst_stages,
                           ImageLayout old_layout,
                           ImageLayout new_layout,
                           uint32_t src_queue_family,
                           uint32_t dst_queue_family) {
  m_imageMemoryBarrier.setImage(ptr_context->getPtrSwapchain()->getImage())
      .setSrcAccessMask(g_lamda_convert_access_flags(src_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(dst_access_flags))
      .setSrcStageMask(g_lamda_convert_stage_flags(src_stages))
      .setDstStageMask(g_lamda_convert_stage_flags(dst_stages))
      .setOldLayout(vk_helper::getImageLayout(old_layout))
      .setNewLayout(vk_helper::getImageLayout(new_layout))
      .setSubresourceRange(vk::ImageSubresourceRange()
                               .setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setBaseMipLevel(0u)
                               .setLevelCount(1u)
                               .setBaseArrayLayer(0u)
                               .setLayerCount(1u))
      .setSrcQueueFamilyIndex(src_queue_family)
      .setDstQueueFamilyIndex(dst_queue_family);
}

ImageBarrier::~ImageBarrier() {}

}  // namespace pandora::core::gpu
