#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

static vk::ImageUsageFlags get_transfer_usage(const pandora::core::TransferType transfer_type) {
  switch (transfer_type) {
    using enum pandora::core::TransferType;
    using enum vk::ImageUsageFlagBits;

    case TransferSrc:
      return eTransferSrc;
    case TransferDst:
      return eTransferDst;
    case TransferSrcDst:
      return eTransferSrc | eTransferDst;
    default:
      return vk::ImageUsageFlagBits(0U);
  }
}

static vk::ImageType get_image_type(pandora::core::ImageDimension dimension) {
  switch (dimension) {
    using enum pandora::core::ImageDimension;
    using enum vk::ImageType;

    case v1D:
      return e1D;
    case v2D:
      return e2D;
    case v3D:
      return e3D;
    default:
      return vk::ImageType(0U);
  }
}

pandora::core::gpu::Image::Image(const std::unique_ptr<Context>& ptr_context,
                                 const MemoryUsage memory_usage,
                                 const TransferType transfer_type,
                                 const std::vector<ImageUsage>& image_usages,
                                 const ImageSubInfo& image_sub_info) {
  const auto& ptr_vk_device = ptr_context->getPtrDevice()->getPtrLogicalDevice();

  {
    vk::ImageCreateInfo image_info{};
    {
      const vk::ImageUsageFlags vk_transfer_type = get_transfer_usage(transfer_type);

      vk::ImageUsageFlags vk_image_usages{};
      for (const auto& image_usage : image_usages) {
        vk_image_usages |= vk_helper::getImageUsage(image_usage);
      }

      image_info.setUsage(vk_transfer_type | vk_image_usages);
    }

    {
      const vk::Format vk_format = vk_helper::getFormat(image_sub_info.format);

      image_info.setFormat(vk_format);
      m_format = vk_format;
    }

    {
      vk::Extent3D vk_extent{};
      vk_extent.setWidth(image_sub_info.graphical_size.width);
      vk_extent.setHeight(image_sub_info.graphical_size.height);
      vk_extent.setDepth(image_sub_info.graphical_size.depth);

      image_info.setExtent(vk_extent);
      m_graphicalSize = image_sub_info.graphical_size;
    }

    m_arrayLayers = image_sub_info.array_layers;
    image_info.setArrayLayers(m_arrayLayers);

    m_mipLevels = image_sub_info.mip_levels;
    image_info.setMipLevels(image_sub_info.mip_levels);

    image_info.setImageType(get_image_type(image_sub_info.dimension));
    m_dimension = image_sub_info.dimension;

    image_info.setSamples(vk_helper::getSampleCount(image_sub_info.samples));
    image_info.setTiling(vk::ImageTiling::eOptimal);
    image_info.setSharingMode(vk::SharingMode::eExclusive);
    image_info.setInitialLayout(vk::ImageLayout::eUndefined);
    image_info.setQueueFamilyIndexCount(0U);
    image_info.setPQueueFamilyIndices(nullptr);

    m_ptrImage = ptr_vk_device->createImageUnique(image_info);
  }

  {
    const auto memory_requirements = ptr_vk_device->getImageMemoryRequirements(m_ptrImage.get());

    const vk::MemoryPropertyFlags vk_memory_usage = vk_helper::getMemoryPropertyFlags(memory_usage);

    const auto memory_props = ptr_context->getPtrDevice()->getPhysicalDevice().getMemoryProperties();

    uint32_t memory_type_idx = 0U;
    for (; memory_type_idx < memory_props.memoryTypeCount; memory_type_idx += 1U) {
      if ((memory_requirements.memoryTypeBits & (1U << memory_type_idx)) &&
          (memory_props.memoryTypes.at(memory_type_idx).propertyFlags & vk_memory_usage) == vk_memory_usage) {
        break;
      }
    }

    vk::MemoryAllocateInfo allocation_info{};
    allocation_info.setMemoryTypeIndex(memory_type_idx);
    allocation_info.setAllocationSize(memory_requirements.size);

    m_ptrMemory = ptr_vk_device->allocateMemoryUnique(allocation_info);
  }

  ptr_vk_device->bindImageMemory(m_ptrImage.get(), m_ptrMemory.get(), 0U);
}

pandora::core::gpu::Image::~Image() {}
