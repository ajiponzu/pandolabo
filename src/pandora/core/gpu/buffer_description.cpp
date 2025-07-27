#include <iostream>

#include "pandora/core/gpu.hpp"

pandora::core::gpu::BufferDescription::BufferDescription(const DescriptorInfo& descriptor_info, const Buffer& buffer) {
  m_ptrBufferInfo = std::make_shared<vk::DescriptorBufferInfo>();
  m_ptrBufferInfo->setBuffer(buffer.getBuffer());
  m_ptrBufferInfo->setOffset(0);
  m_ptrBufferInfo->setRange(buffer.getSize());

  m_ptrWriteDescriptorSet = std::make_shared<vk::WriteDescriptorSet>();
  m_ptrWriteDescriptorSet->setDstBinding(descriptor_info.binding);
  m_ptrWriteDescriptorSet->setDstArrayElement(0);
  m_ptrWriteDescriptorSet->setDescriptorType(descriptor_info.type);
  m_ptrWriteDescriptorSet->setBufferInfo(*m_ptrBufferInfo);
}

pandora::core::gpu::BufferDescription::~BufferDescription() {}
