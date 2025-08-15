#include "pandora/core/gpu.hpp"

pandora::core::gpu::DescriptorSet::DescriptorSet(const std::unique_ptr<Context>& ptr_context,
                                                 const DescriptorSetLayout& description_set_layout) {
  const auto& ptr_vk_device = ptr_context->getPtrDevice()->getPtrLogicalDevice();

  m_ptrDescriptorPool = ptr_vk_device->createDescriptorPoolUnique(description_set_layout.getDescriptorPoolInfo());
  m_ptrDescriptorSet =
      std::move(ptr_vk_device
                    ->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo()
                                                       .setDescriptorPool(m_ptrDescriptorPool.get())
                                                       .setSetLayouts(description_set_layout.getDescriptorSetLayout()))
                    .front());
}

pandora::core::gpu::DescriptorSet::~DescriptorSet() {}

void pandora::core::gpu::DescriptorSet::updateDescriptorSet(const std::unique_ptr<Context>& ptr_context,
                                                            const std::vector<BufferDescription>& buffer_descriptions,
                                                            const std::vector<ImageDescription>& image_descriptions) {
  std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
  write_descriptor_sets.reserve(buffer_descriptions.size() + image_descriptions.size());

  for (const auto& buffer_description : buffer_descriptions) {
    write_descriptor_sets.push_back(buffer_description.getWriteDescriptorSet());
    write_descriptor_sets.back().setDstSet(m_ptrDescriptorSet.get());
  }
  for (const auto& image_description : image_descriptions) {
    write_descriptor_sets.push_back(image_description.getWriteDescriptorSet());
    write_descriptor_sets.back().setDstSet(m_ptrDescriptorSet.get());
  }

  ptr_context->getPtrDevice()->getPtrLogicalDevice()->updateDescriptorSets(write_descriptor_sets, nullptr);
}

void pandora::core::gpu::DescriptorSet::freeDescriptorSet(const std::unique_ptr<Context>& ptr_context) {
  ptr_context->getPtrDevice()->getPtrLogicalDevice()->freeDescriptorSets(m_ptrDescriptorPool.get(),
                                                                         m_ptrDescriptorSet.get());
}
