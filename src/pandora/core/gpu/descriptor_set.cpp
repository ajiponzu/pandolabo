#include <ranges>

#include "pandora/core/gpu.hpp"

pandora::core::gpu::DescriptorSet::DescriptorSet(const std::unique_ptr<Context>& ptr_context,
                                                 const DescriptorSetLayout& description_set_layout) {
  const auto& ptr_vk_device = ptr_context->getPtrDevice()->getPtrLogicalDevice();

  m_ptrDescriptorPool = ptr_vk_device->createDescriptorPoolUnique(description_set_layout.getDescriptorPoolInfo());
  m_ptrDescriptorSet =
      std::move(ptr_vk_device
                    ->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo{}
                                                       .setDescriptorPool(m_ptrDescriptorPool.get())
                                                       .setSetLayouts(description_set_layout.getDescriptorSetLayout()))
                    .front());
}

pandora::core::gpu::DescriptorSet::~DescriptorSet() {}

void pandora::core::gpu::DescriptorSet::updateDescriptorSet(const std::unique_ptr<Context>& ptr_context,
                                                            const std::vector<BufferDescription>& buffer_descriptions,
                                                            const std::vector<ImageDescription>& image_descriptions) {
  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;

  const auto buffer_info_list =
      buffer_descriptions
      | std::views::transform([](const BufferDescription& desc) { return desc.createVkBufferInfo(); })
      | std::ranges::to<std::vector<vk::DescriptorBufferInfo>>();
  for (const auto& [buffer_desc, buffer_info] : std::views::zip(buffer_descriptions, buffer_info_list)) {
    write_descriptor_sets.emplace_back(
        buffer_desc.createVkWriteDescriptorSet(buffer_info).setDstSet(m_ptrDescriptorSet.get()));
  }

  const auto image_info_list =
      image_descriptions | std::views::transform([](const ImageDescription& desc) { return desc.createVkImageInfo(); })
      | std::ranges::to<std::vector<vk::DescriptorImageInfo>>();
  for (const auto& [image_desc, image_info] : std::views::zip(image_descriptions, image_info_list)) {
    write_descriptor_sets.emplace_back(
        image_desc.createVkWriteDescriptorSet(image_info).setDstSet(m_ptrDescriptorSet.get()));
  }

  ptr_context->getPtrDevice()->getPtrLogicalDevice()->updateDescriptorSets(write_descriptor_sets, nullptr);
}

void pandora::core::gpu::DescriptorSet::freeDescriptorSet(const std::unique_ptr<Context>& ptr_context) {
  ptr_context->getPtrDevice()->getPtrLogicalDevice()->freeDescriptorSets(m_ptrDescriptorPool.get(),
                                                                         m_ptrDescriptorSet.get());
}
