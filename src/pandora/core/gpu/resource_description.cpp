#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

namespace pandora::core::gpu {

BufferDescription::BufferDescription(const DescriptorInfo& descriptor_info,
                                     const Buffer& buffer) {
  m_bufferInfo.buffer = buffer.getBuffer();
  m_bufferInfo.size = buffer.getSize();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

BufferDescription::~BufferDescription() {}

vk::DescriptorBufferInfo BufferDescription::createVkBufferInfo() const {
  return vk::DescriptorBufferInfo{}
      .setBuffer(m_bufferInfo.buffer)
      .setRange(m_bufferInfo.size)
      .setOffset(0u);
}

vk::WriteDescriptorSet BufferDescription::createVkWriteDescriptorSet(
    const vk::DescriptorBufferInfo& info) const {
  return vk::WriteDescriptorSet{}
      .setDstBinding(m_writeDescInfo.binding)
      .setDescriptorType(m_writeDescInfo.type)
      .setDstArrayElement(0u)
      .setBufferInfo(info);
}

ImageDescription::ImageDescription(const DescriptorInfo& descriptor_info,
                                   const ImageView& image_view,
                                   ImageLayout dst_image_layout) {
  m_imageInfo.layout = vk_helper::getImageLayout(dst_image_layout);
  m_imageInfo.view = image_view.getImageView();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

ImageDescription::ImageDescription(const DescriptorInfo& descriptor_info,
                                   ImageLayout dst_image_layout,
                                   const Sampler& sampler) {
  m_imageInfo.layout = vk_helper::getImageLayout(dst_image_layout);
  m_imageInfo.sampler = sampler.getSampler();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

ImageDescription::ImageDescription(const DescriptorInfo& descriptor_info,
                                   const ImageView& image_view,
                                   ImageLayout dst_image_layout,
                                   const Sampler& sampler) {
  m_imageInfo.layout = vk_helper::getImageLayout(dst_image_layout);
  m_imageInfo.view = image_view.getImageView();
  m_imageInfo.sampler = sampler.getSampler();

  m_writeDescInfo.binding = descriptor_info.binding;
  m_writeDescInfo.type = descriptor_info.type;
}

ImageDescription::~ImageDescription() {}

vk::DescriptorImageInfo ImageDescription::createVkImageInfo() const {
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

vk::WriteDescriptorSet ImageDescription::createVkWriteDescriptorSet(
    const vk::DescriptorImageInfo& info) const {
  return vk::WriteDescriptorSet{}
      .setDstBinding(m_writeDescInfo.binding)
      .setDescriptorType(m_writeDescInfo.type)
      .setDstArrayElement(0u)
      .setImageInfo(info);
}

}  // namespace pandora::core::gpu
