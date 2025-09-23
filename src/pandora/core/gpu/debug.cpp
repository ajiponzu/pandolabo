#include "pandora/core/gpu.hpp"

#ifdef GPU_DEBUG

  #include <cstring>
  #include <optional>
  #include <print>
  #include <vector>

std::vector<const char*>
    pandora::core::gpu::debug::Messenger::s_validationLayers = {
        "VK_LAYER_KHRONOS_validation",
};

namespace {

  #pragma warning(push)
  #pragma warning(disable : 4100)
VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
    void* p_user_data) {
  std::println(
      stderr, "[****] validation layer: {}", p_callback_data->pMessage);

  return VK_FALSE;
}
  #pragma warning(pop)

bool check_validation_layer_support(
    const std::vector<const char*>& validation_layers) {
  const auto available_layers = vk::enumerateInstanceLayerProperties();

  for (const auto& layer_name : validation_layers) {
    bool layer_found = false;
    for (const auto& layer_properties : available_layers) {
      if (strcmp(layer_name, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      return false;
    }
  }

  return true;
}

}  // namespace

namespace pandora::core::gpu {

debug::Messenger::~Messenger() {
  m_ptrMessenger.release();
}

vk::UniqueInstance debug::Messenger::createDebugInstance(
    const vk::ApplicationInfo& app_info,
    const std::vector<const char*>& extensions) {
  if (!check_validation_layer_support(s_validationLayers)) {
    return vk::UniqueInstance(nullptr);
  }

  const vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

  const vk::DebugUtilsMessageTypeFlagsEXT type_flags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
      | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
      | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

  // 拡張有無を判定
  const auto ext_contains = [&](const char* name) {
    for (auto* e : extensions) {
      if (std::strcmp(e, name) == 0)
        return true;
    }
    return false;
  };

  const bool has_debug_utils = ext_contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  const bool has_validation_features =
      ext_contains(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

  // Create pNext chain conditionally
  vk::InstanceCreateInfo instance_info{};
  instance_info.setPApplicationInfo(&app_info);
  instance_info.setPEnabledLayerNames(s_validationLayers);
  instance_info.setPEnabledExtensionNames(extensions);

  vk::ValidationFeaturesEXT validation_features{};
  if (has_validation_features) {
    std::vector<vk::ValidationFeatureEnableEXT> enabled_features = {
        vk::ValidationFeatureEnableEXT::eDebugPrintf};
    validation_features.setEnabledValidationFeatures(enabled_features);
    validation_features.setPNext(instance_info.pNext);
    instance_info.setPNext(&validation_features);
  }

  vk::DebugUtilsMessengerCreateInfoEXT messenger_info{};
  messenger_info.setMessageSeverity(severity_flags);
  messenger_info.setMessageType(type_flags);
  messenger_info.setPfnUserCallback(
      reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(
          &debug_utils_messenger_callback));
  if (has_debug_utils) {
    messenger_info.setPNext(instance_info.pNext);
    instance_info.setPNext(&messenger_info);
  }

  auto ptr_vk_instance = vk::createInstanceUnique(instance_info);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(*ptr_vk_instance);
  if (has_debug_utils) {
    m_ptrMessenger =
        ptr_vk_instance->createDebugUtilsMessengerEXTUnique(messenger_info);
  }

  return std::move(ptr_vk_instance);
}
}  // namespace pandora::core::gpu
#endif
