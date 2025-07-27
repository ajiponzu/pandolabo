/*
 * debug.hpp - Debug utilities for Pandolabo Vulkan C++ wrapper
 *
 * This header contains debug-related classes and utilities for Vulkan validation.
 * These classes are only available in debug builds.
 */

#pragma once

#include "config.hpp"

#ifdef GPU_DEBUG

  #include <vector>
  #include <vulkan/vulkan.hpp>

namespace pandora::core::gpu::debug {

/// @brief Debug messenger for Vulkan validation layers
/// @details This class is used to create a debug instance and handle validation layer messages.
/// This class is only available in debug mode.
class Messenger {
 private:
  static std::vector<const char*> s_validationLayers;
  vk::UniqueDebugUtilsMessengerEXT m_ptrMessenger;

 public:
  Messenger() {}
  ~Messenger();

  /// @brief Create Vulkan instance with debug validation layers
  /// @param app_info Application information for Vulkan instance
  /// @param extensions Required Vulkan extensions
  /// @return Unique pointer to Vulkan instance with debug support
  vk::UniqueInstance createDebugInstance(const vk::ApplicationInfo& app_info,
                                         const std::vector<const char*>& extensions);

  /// @brief Get validation layers used for debugging
  /// @return Reference to validation layers vector
  const auto& getValidationLayers() const {
    return s_validationLayers;
  }
};

}  // namespace pandora::core::gpu::debug

#endif  // GPU_DEBUG
