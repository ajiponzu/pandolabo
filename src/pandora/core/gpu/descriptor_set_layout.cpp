#include "pandora/core/gpu.hpp"

pandora::core::gpu::DescriptorSetLayout::DescriptorSetLayout(const std::unique_ptr<Context>& ptr_context,
                                                             const DescriptionUnit& description_unit) {
  std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings;

  for (const auto& [_key, description] : description_unit.getDescriptorInfoMap()) {
    vk::DescriptorSetLayoutBinding descriptor_set_layout_binding;
    descriptor_set_layout_binding.setBinding(description.binding);
    descriptor_set_layout_binding.setDescriptorType(description.type);
    descriptor_set_layout_binding.setDescriptorCount(1U);
    descriptor_set_layout_binding.setStageFlags(description.stage_flags);

    descriptor_set_layout_bindings.push_back(descriptor_set_layout_binding);
    m_descriptorPoolSizes.push_back(vk::DescriptorPoolSize(description.type, 1U));
  }

  vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info;
  descriptor_set_layout_info.setBindings(descriptor_set_layout_bindings);

  m_ptrDescriptorSetLayout =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createDescriptorSetLayoutUnique(descriptor_set_layout_info);
}

pandora::core::gpu::DescriptorSetLayout::~DescriptorSetLayout() {}

vk::DescriptorPoolCreateInfo pandora::core::gpu::DescriptorSetLayout::getDescriptorPoolInfo() const {
  vk::DescriptorPoolCreateInfo create_info;
  create_info.setMaxSets(1U);
  create_info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
  create_info.setPoolSizes(m_descriptorPoolSizes);

  return create_info;
}
