
#include "pandora/core/gpu.hpp"

pandora::core::gpu::DescriptionUnit::DescriptionUnit(
    const std::unordered_map<std::string, ShaderModule>& shader_module_map,
    const std::vector<std::string>& module_keys) {
  for (const auto& module_key : module_keys) {
    const auto& shader_module = shader_module_map.at(module_key);

    // Merge descriptor infos
    for (const auto& [key, descriptor_info] : shader_module.getDescriptorInfoMap()) {
      auto it = m_descriptorInfoMap.find(key);
      if (it != m_descriptorInfoMap.end()) {
        // merge stage flags because the same descriptor info is used in multiple stages
        it->second.stage_flags |= descriptor_info.stage_flags;
      } else {
        m_descriptorInfoMap[key] = descriptor_info;
      }
    }

    // Merge push constant ranges
    for (const auto& [key, push_constant_range] : shader_module.getPushConstantRangeMap()) {
      auto it = m_pushConstantRangeMap.find(key);
      if (it != m_pushConstantRangeMap.end()) {
        // merge stage flags because the same push constants is used in multiple stages
        it->second.stage_flags |= push_constant_range.stage_flags;
      } else {
        m_pushConstantRangeMap[key] = push_constant_range;
      }
    }
  }
}

pandora::core::gpu::DescriptionUnit::~DescriptionUnit() {}
