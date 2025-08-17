#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::gpu::BufferDescription::BufferDescription(const DescriptorInfo& descriptor_info, const Buffer& buffer) {
  m_bufferInfo.buffer = buffer.getBuffer();
  m_bufferInfo.size = buffer.getSize();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

pandora::core::gpu::BufferDescription::~BufferDescription() {}

vk::DescriptorBufferInfo pandora::core::gpu::BufferDescription::createVkBufferInfo() const {
  return vk::DescriptorBufferInfo{}.setBuffer(m_bufferInfo.buffer).setRange(m_bufferInfo.size).setOffset(0U);
}

vk::WriteDescriptorSet pandora::core::gpu::BufferDescription::createVkWriteDescriptorSet(
    const vk::DescriptorBufferInfo& info) const {
  return vk::WriteDescriptorSet{}
      .setDstBinding(m_writeDescInfo.binding)
      .setDescriptorType(m_writeDescInfo.type)
      .setDstArrayElement(0U)
      .setBufferInfo(info);
}

pandora::core::gpu::ImageDescription::ImageDescription(const DescriptorInfo& descriptor_info,
                                                       const ImageView& image_view,
                                                       const ImageLayout dst_image_layout) {
  m_imageInfo.layout = vk_helper::getImageLayout(dst_image_layout);
  m_imageInfo.view = image_view.getImageView();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

pandora::core::gpu::ImageDescription::ImageDescription(const DescriptorInfo& descriptor_info,
                                                       const ImageLayout dst_image_layout,
                                                       const Sampler& sampler) {
  m_imageInfo.layout = vk_helper::getImageLayout(dst_image_layout);
  m_imageInfo.sampler = sampler.getSampler();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

pandora::core::gpu::ImageDescription::ImageDescription(const DescriptorInfo& descriptor_info,
                                                       const ImageView& image_view,
                                                       const ImageLayout dst_image_layout,
                                                       const Sampler& sampler) {
  m_imageInfo.layout = vk_helper::getImageLayout(dst_image_layout);
  m_imageInfo.view = image_view.getImageView();
  m_imageInfo.sampler = sampler.getSampler();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

pandora::core::gpu::ImageDescription::~ImageDescription() {}

vk::DescriptorImageInfo pandora::core::gpu::ImageDescription::createVkImageInfo() const {
  vk::DescriptorImageInfo info{};

  if (m_imageInfo.layout) {
    info.setImageLayout(*(m_imageInfo.layout));
  }
  if (m_imageInfo.view) {
    info.setImageView(*(m_imageInfo.view));
  }
  if (m_imageInfo.sampler) {
    info.setSampler(*(m_imageInfo.sampler));
  }

  return info;
}

vk::WriteDescriptorSet pandora::core::gpu::ImageDescription::createVkWriteDescriptorSet(
    const vk::DescriptorImageInfo& info) const {
  return vk::WriteDescriptorSet{}
      .setDstBinding(m_writeDescInfo.binding)
      .setDescriptorType(m_writeDescInfo.type)
      .setDstArrayElement(0U)
      .setImageInfo(info);
}
