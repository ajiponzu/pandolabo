#include <ranges>

#include "pandora/core/gpu.hpp"

namespace pandora::core::gpu {

DescriptorSetLayout::DescriptorSetLayout(
    const std::unique_ptr<Context>& ptr_context,
    const DescriptionUnit& description_unit) {
  using D = std::ranges::range_value_t<
      decltype(description_unit.getDescriptorInfoMap() | std::views::values)>;

  const auto descriptor_set_layout_bindings =
      description_unit.getDescriptorInfoMap() | std::views::values
      | std::views::transform(
          [&m_poolSizes = m_descriptorPoolSizes](const D& x) {
            m_poolSizes.push_back(vk::DescriptorPoolSize(x.type, 1u));

            return vk::DescriptorSetLayoutBinding()
                .setBinding(x.binding)
                .setDescriptorType(x.type)
                .setDescriptorCount(1u)
                .setStageFlags(x.stage_flags);
          })
      | std::ranges::to<std::vector<vk::DescriptorSetLayoutBinding>>();

  const auto descriptor_set_layout_info =
      vk::DescriptorSetLayoutCreateInfo{}.setBindings(
          descriptor_set_layout_bindings);

  m_ptrDescriptorSetLayout =
      ptr_context->getPtrDevice()
          ->getPtrLogicalDevice()
          ->createDescriptorSetLayoutUnique(descriptor_set_layout_info);
}

DescriptorSetLayout::~DescriptorSetLayout() {}

vk::DescriptorPoolCreateInfo DescriptorSetLayout::getDescriptorPoolInfo()
    const {
  return vk::DescriptorPoolCreateInfo()
      .setMaxSets(1u)
      .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
      .setPoolSizes(m_descriptorPoolSizes);
}

}  // namespace pandora::core::gpu
