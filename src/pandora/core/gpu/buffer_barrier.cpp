#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::gpu::BufferBarrier::BufferBarrier(const Buffer& buffer,
                                                 const std::vector<AccessFlag>& priority_access_flags,
                                                 const std::vector<AccessFlag>& wait_access_flags) {
  m_bufferMemoryBarrier.setBuffer(buffer.getBuffer());
  m_bufferMemoryBarrier.setSize(buffer.getSize());

  auto convertAccessFlags = [](const std::vector<AccessFlag>& access_flags) {
    vk::AccessFlags flags;
    for (const auto& access_flag : access_flags) {
      flags |= vk_helper::getAccessFlagBits(access_flag);
    }
    return flags;
  };

  m_bufferMemoryBarrier.setSrcAccessMask(convertAccessFlags(priority_access_flags));
  m_bufferMemoryBarrier.setDstAccessMask(convertAccessFlags(wait_access_flags));
}

pandora::core::gpu::BufferBarrier::~BufferBarrier() {}
