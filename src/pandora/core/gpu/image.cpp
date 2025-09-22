#include <algorithm>
#include <functional>
#include <ranges>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

namespace {

vk::ImageUsageFlags get_transfer_usage(
    const pandora::core::TransferType transfer_type) {
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
      return vk::ImageUsageFlagBits(0u);
  }
}

vk::ImageType get_image_type(pandora::core::ImageDimension dimension) {
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
      return vk::ImageType(0u);
  }
}

}  // namespace

namespace pandora::core::gpu {

Image::Image(const std::unique_ptr<Context>& ptr_context,
             MemoryUsage memory_usage,
             TransferType transfer_type,
             const std::vector<ImageUsage>& image_usages,
             const ImageSubInfo& image_sub_info) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  {
    vk::ImageCreateInfo image_info{};
    {
      const auto vk_transfer_type = get_transfer_usage(transfer_type);
      const auto vk_image_usages = std::ranges::fold_left(
          image_usages | std::views::transform(vk_helper::getImageUsage),
          vk::ImageUsageFlags{},
          std::bit_or());

      image_info.setUsage(vk_transfer_type | vk_image_usages);
    }

    {
      const auto vk_format = vk_helper::getFormat(image_sub_info.format);

      m_format = vk_format;
      image_info.setFormat(vk_format);
    }

    m_graphicalSize = image_sub_info.graphical_size;
    m_arrayLayers = image_sub_info.array_layers;
    m_mipLevels = image_sub_info.mip_levels;
    m_dimension = image_sub_info.dimension;

    image_info.setExtent(vk_helper::getExtent3D(m_graphicalSize))
        .setArrayLayers(m_arrayLayers)
        .setMipLevels(m_mipLevels)
        .setImageType(get_image_type(m_dimension))
        .setSamples(vk_helper::getSampleCount(image_sub_info.samples))
        .setTiling(vk::ImageTiling::eOptimal)
        .setSharingMode(vk::SharingMode::eExclusive)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setQueueFamilyIndexCount(0u)
        .setPQueueFamilyIndices(nullptr);

    m_ptrImage = ptr_vk_device->createImageUnique(image_info);
  }

  {
    const auto memory_requirements =
        ptr_vk_device->getImageMemoryRequirements(m_ptrImage.get());
    const auto vk_memory_usage =
        vk_helper::getMemoryPropertyFlags(memory_usage);
    const auto memory_props =
        ptr_context->getPtrDevice()->getPhysicalDevice().getMemoryProperties();

    uint32_t memory_type_idx = 0u;
    for (; memory_type_idx < memory_props.memoryTypeCount;
         memory_type_idx += 1u) {
      if ((memory_requirements.memoryTypeBits & (1u << memory_type_idx))
          && (memory_props.memoryTypes.at(memory_type_idx).propertyFlags
              & vk_memory_usage)
                 == vk_memory_usage) {
        break;
      }
    }

    m_ptrMemory = ptr_vk_device->allocateMemoryUnique(
        vk::MemoryAllocateInfo{}
            .setMemoryTypeIndex(memory_type_idx)
            .setAllocationSize(memory_requirements.size));
  }

  ptr_vk_device->bindImageMemory(m_ptrImage.get(), m_ptrMemory.get(), 0u);
}

Image::~Image() {}

}  // namespace pandora::core::gpu
