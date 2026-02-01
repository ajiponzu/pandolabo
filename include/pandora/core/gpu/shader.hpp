/*
 * shader.hpp - GPU shader resources for Pandolabo Vulkan C++ wrapper
 *
 * This header contains shader-related classes: ShaderModule and
 * DescriptionUnit. These classes manage shader modules and integrate multiple
 * shader resources.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../structures.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Context;
}

namespace pandora::core::gpu {

/// @brief Shader module wrapper with reflection support
/// This class wraps Vulkan shader modules and automatically parses shader
/// descriptions using SPIRV-Cross reflection. The reflection provides
/// semi-automated handling of shader resources, bindings, and interface
/// information.
class ShaderModule {
 private:
  vk::UniqueShaderModule m_ptrShaderModule{};  ///< Vulkan shader module
  std::string m_entryPointName{};  ///< Shader entry point function name

  std::unordered_map<std::string, DescriptorInfo> m_descriptorInfoMap;
  std::unordered_map<std::string, PushConstantRange> m_pushConstantRangeMap;

  vk::ShaderStageFlagBits m_shaderStageFlag{};

 public:
  ShaderModule() = default;
  ShaderModule(const Context& ptr_context,
               const std::vector<uint32_t>& spirv_binary);

  // Rule of Five
  ~ShaderModule();
  ShaderModule(const ShaderModule&) = delete;
  ShaderModule& operator=(const ShaderModule&) = delete;
  ShaderModule(ShaderModule&&) = default;
  ShaderModule& operator=(ShaderModule&&) = default;

  const auto& getModule() const {
    return m_ptrShaderModule.get();
  }
  const auto& getEntryPointName() const {
    return m_entryPointName;
  }
  const auto& getDescriptorInfoMap() const {
    return m_descriptorInfoMap;
  }
  const auto& getPushConstantRangeMap() const {
    return m_pushConstantRangeMap;
  }
  const auto getShaderStageFlag() const {
    return m_shaderStageFlag;
  }
};

/// @brief Shader description integration unit
/// This class implements a Pandolabo-specific concept for managing multiple
/// shader modules. In graphics pipelines, uniform buffer data is often shared
/// between vertex and fragment shaders. Since ShaderModule represents only one
/// shader binary, this class integrates and manages descriptor information from
/// multiple shader modules for unified resource binding.
class DescriptionUnit {
 private:
  std::unordered_map<std::string, DescriptorInfo>
      m_descriptorInfoMap{};  ///< Integrated descriptor information
  std::unordered_map<std::string, PushConstantRange> m_pushConstantRangeMap;

 public:
  DescriptionUnit(
      const std::unordered_map<std::string, ShaderModule>& shader_module_map,
      const std::vector<std::string>& module_keys);

  // Rule of Zero
  ~DescriptionUnit();

  const auto& getDescriptorInfoMap() const {
    return m_descriptorInfoMap;
  }
  const auto& getPushConstantRangeMap() const {
    return m_pushConstantRangeMap;
  }
};

}  // namespace pandora::core::gpu
