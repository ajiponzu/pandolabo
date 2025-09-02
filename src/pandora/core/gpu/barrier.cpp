#include <algorithm>
#include <functional>
#include <ranges>
#include <span>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

namespace {

constexpr auto g_lamda_convert_access_flags =
    [](const std::span<const pandora::core::AccessFlag>& access_flags) {
      return std::ranges::fold_left(
          access_flags | std::views::transform(vk_helper::getAccessFlagBits),
          vk::AccessFlags{},
          std::bit_or());
    };

}  // namespace

pandora::core::gpu::BufferBarrier::BufferBarrier(
    const Buffer& buffer,
    std::span<const AccessFlag> priority_access_flags,
    std::span<const AccessFlag> wait_access_flags,
    uint32_t src_queue_family,
    uint32_t dst_queue_family) {
  m_bufferMemoryBarrier.setBuffer(buffer.getBuffer())
      .setSize(buffer.getSize())
      .setSrcAccessMask(g_lamda_convert_access_flags(priority_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(wait_access_flags))
      .setSrcQueueFamilyIndex(src_queue_family)
      .setDstQueueFamilyIndex(dst_queue_family);
}

pandora::core::gpu::BufferBarrier::~BufferBarrier() {}

pandora::core::gpu::ImageBarrier::ImageBarrier(
    const Image& image,
    std::span<const AccessFlag> priority_access_flags,
    std::span<const AccessFlag> wait_access_flags,
    const ImageLayout old_layout,
    const ImageLayout new_layout,
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
      .setSrcAccessMask(g_lamda_convert_access_flags(priority_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(wait_access_flags))
      .setOldLayout(vk_helper::getImageLayout(old_layout))
      .setNewLayout(vk_helper::getImageLayout(new_layout))
      .setSubresourceRange(subresource_range)
      .setSrcQueueFamilyIndex(src_queue_family)
      .setDstQueueFamilyIndex(dst_queue_family);
}

pandora::core::gpu::ImageBarrier::ImageBarrier(
    const std::unique_ptr<Context>& ptr_context,
    std::span<const AccessFlag> priority_access_flags,
    std::span<const AccessFlag> wait_access_flags,
    const ImageLayout old_layout,
    const ImageLayout new_layout,
    uint32_t src_queue_family,
    uint32_t dst_queue_family) {
  m_imageMemoryBarrier.setImage(ptr_context->getPtrSwapchain()->getImage())
      .setSrcAccessMask(g_lamda_convert_access_flags(priority_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(wait_access_flags))
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

pandora::core::gpu::ImageBarrier::~ImageBarrier() {}
