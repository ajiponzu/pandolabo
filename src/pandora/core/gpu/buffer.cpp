#include <algorithm>
#include <functional>
#include <ranges>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

static vk::BufferUsageFlags get_transfer_usage_flags(
    const pandora::core::TransferType transfer_type) {
  switch (transfer_type) {
    using enum pandora::core::TransferType;
    using enum vk::BufferUsageFlagBits;

    case TransferSrc:
      return eTransferSrc;
    case TransferDst:
      return eTransferDst;
    case TransferSrcDst:
      return eTransferSrc | eTransferDst;
    default:
      return eTransferSrc;
  }
}

static vk::BufferUsageFlagBits get_buffer_usage(
    const pandora::core::BufferUsage buffer_usage) {
  switch (buffer_usage) {
    using enum pandora::core::BufferUsage;
    using enum vk::BufferUsageFlagBits;

    case VertexBuffer:
      return eVertexBuffer;
    case IndexBuffer:
      return eIndexBuffer;
    case UniformBuffer:
      return eUniformBuffer;
    case StorageBuffer:
      return eStorageBuffer;
    case StagingBuffer:
      return eTransferSrc;
    default:
      return eVertexBuffer;
  }
}

pandora::core::gpu::Buffer::Buffer(
    const std::unique_ptr<Context>& ptr_context,
    const MemoryUsage memory_usage,
    const TransferType transfer_type,
    const std::vector<BufferUsage>& buffer_usages,
    const size_t size)
    : m_size(size) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  {
    const auto vk_transfer_type = get_transfer_usage_flags(transfer_type);
    const auto vk_buffer_usages = std::ranges::fold_left(
        buffer_usages | std::views::transform(get_buffer_usage),
        vk::BufferUsageFlags{},
        std::bit_or());

    m_ptrBuffer = ptr_vk_device->createBufferUnique(
        vk::BufferCreateInfo{}
            .setUsage(vk_transfer_type | vk_buffer_usages)
            .setSize(m_size)
            .setSharingMode(vk::SharingMode::eExclusive));
  }

  {
    const auto memory_requirements =
        ptr_vk_device->getBufferMemoryRequirements(m_ptrBuffer.get());
    const auto vk_memory_usage =
        vk_helper::getMemoryPropertyFlags(memory_usage);
    const auto memory_props =
        ptr_context->getPtrDevice()->getPhysicalDevice().getMemoryProperties();

    uint32_t memory_type_idx = 0u;
    for (; memory_type_idx < memory_props.memoryTypeCount;
         memory_type_idx += 1u) {
      if ((memory_requirements.memoryTypeBits & (1 << memory_type_idx))
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

  ptr_vk_device->bindBufferMemory(m_ptrBuffer.get(), m_ptrMemory.get(), 0u);
}

pandora::core::gpu::Buffer::~Buffer() = default;

void* pandora::core::gpu::Buffer::mapMemory(
    const std::unique_ptr<Context>& ptr_context) const {
  return ptr_context->getPtrDevice()->getPtrLogicalDevice()->mapMemory(
      m_ptrMemory.get(), 0u, m_size, {});
}

void pandora::core::gpu::Buffer::unmapMemory(
    const std::unique_ptr<Context>& ptr_context) const {
  ptr_context->getPtrDevice()->getPtrLogicalDevice()->unmapMemory(
      m_ptrMemory.get());
}
