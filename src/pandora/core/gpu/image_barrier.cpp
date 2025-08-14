#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::gpu::ImageBarrier::ImageBarrier(const Image& image,
                                               const std::vector<AccessFlag>& priority_access_flags,
                                               const std::vector<AccessFlag>& wait_access_flags,
                                               const ImageLayout old_layout,
                                               const ImageLayout new_layout,
                                               const ImageViewInfo& image_view_info) {
  m_imageMemoryBarrier.setImage(image.getImage());

  {
    vk::AccessFlags access_flags;
    for (const auto& access_flag : priority_access_flags) {
      access_flags |= vk_helper::getAccessFlagBits(access_flag);
    }

    m_imageMemoryBarrier.setSrcAccessMask(access_flags);
  }

  {
    vk::AccessFlags access_flags;
    for (const auto& access_flag : wait_access_flags) {
      access_flags |= vk_helper::getAccessFlagBits(access_flag);
    }
    m_imageMemoryBarrier.setDstAccessMask(access_flags);
  }

  m_imageMemoryBarrier.setOldLayout(vk_helper::getImageLayout(old_layout));
  m_imageMemoryBarrier.setNewLayout(vk_helper::getImageLayout(new_layout));

  {
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

    subresource_range.setBaseMipLevel(image_view_info.base_mip_level);
    subresource_range.setLevelCount(image_view_info.mip_levels);
    subresource_range.setBaseArrayLayer(image_view_info.base_array_layer);
    subresource_range.setLayerCount(image_view_info.array_layers);

    m_imageMemoryBarrier.setSubresourceRange(subresource_range);
  }
}

pandora::core::gpu::ImageBarrier::ImageBarrier(const std::unique_ptr<Context>& ptr_context,
                                               const std::vector<AccessFlag>& priority_access_flags,
                                               const std::vector<AccessFlag>& wait_access_flags,
                                               const ImageLayout old_layout,
                                               const ImageLayout new_layout) {
  m_imageMemoryBarrier.setImage(ptr_context->getPtrSwapchain()->getImage());

  {
    vk::AccessFlags access_flags;
    for (const auto& access_flag : priority_access_flags) {
      access_flags |= vk_helper::getAccessFlagBits(access_flag);
    }

    m_imageMemoryBarrier.setSrcAccessMask(access_flags);
  }

  {
    vk::AccessFlags access_flags;
    for (const auto& access_flag : wait_access_flags) {
      access_flags |= vk_helper::getAccessFlagBits(access_flag);
    }
    m_imageMemoryBarrier.setDstAccessMask(access_flags);
  }

  m_imageMemoryBarrier.setOldLayout(vk_helper::getImageLayout(old_layout));
  m_imageMemoryBarrier.setNewLayout(vk_helper::getImageLayout(new_layout));

  vk::ImageSubresourceRange subresource_range;
  subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
  subresource_range.setBaseMipLevel(0U);
  subresource_range.setLevelCount(1U);
  subresource_range.setBaseArrayLayer(0U);
  subresource_range.setLayerCount(1U);

  m_imageMemoryBarrier.setSubresourceRange(subresource_range);
}

pandora::core::gpu::ImageBarrier::~ImageBarrier() {}
