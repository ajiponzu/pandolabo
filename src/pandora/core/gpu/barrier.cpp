#include <algorithm>
#include <functional>
#include <ranges>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

inline static constexpr auto g_lamda_convert_access_flags =
    [](const std::vector<pandora::core::AccessFlag>& access_flags) {
      return std::ranges::fold_left(
          access_flags | std::views::transform(vk_helper::getAccessFlagBits), vk::AccessFlags{}, std::bit_or());
    };

pandora::core::gpu::BufferBarrier::BufferBarrier(const Buffer& buffer,
                                                 const std::vector<AccessFlag>& priority_access_flags,
                                                 const std::vector<AccessFlag>& wait_access_flags) {
  m_bufferMemoryBarrier.setBuffer(buffer.getBuffer())
      .setSize(buffer.getSize())
      .setSrcAccessMask(g_lamda_convert_access_flags(priority_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(wait_access_flags));
}

pandora::core::gpu::BufferBarrier::~BufferBarrier() {}

pandora::core::gpu::ImageBarrier::ImageBarrier(const Image& image,
                                               const std::vector<AccessFlag>& priority_access_flags,
                                               const std::vector<AccessFlag>& wait_access_flags,
                                               const ImageLayout old_layout,
                                               const ImageLayout new_layout,
                                               const ImageViewInfo& image_view_info) {
  vk::ImageSubresourceRange subresource_range;

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
      .setSubresourceRange(subresource_range);
}

pandora::core::gpu::ImageBarrier::ImageBarrier(const std::unique_ptr<Context>& ptr_context,
                                               const std::vector<AccessFlag>& priority_access_flags,
                                               const std::vector<AccessFlag>& wait_access_flags,
                                               const ImageLayout old_layout,
                                               const ImageLayout new_layout) {
  m_imageMemoryBarrier.setImage(ptr_context->getPtrSwapchain()->getImage())
      .setSrcAccessMask(g_lamda_convert_access_flags(priority_access_flags))
      .setDstAccessMask(g_lamda_convert_access_flags(wait_access_flags))
      .setOldLayout(vk_helper::getImageLayout(old_layout))
      .setNewLayout(vk_helper::getImageLayout(new_layout))
      .setSubresourceRange(vk::ImageSubresourceRange()
                               .setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setBaseMipLevel(0U)
                               .setLevelCount(1U)
                               .setBaseArrayLayer(0U)
                               .setLayerCount(1U));
}

pandora::core::gpu::ImageBarrier::~ImageBarrier() {}
