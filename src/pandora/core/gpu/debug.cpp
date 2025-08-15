#include "pandora/core/gpu.hpp"

#ifdef GPU_DEBUG

  #include <cstring>
  #include <format>
  #include <iostream>
  #include <optional>
  #include <vector>

std::vector<const char*> pandora::core::gpu::debug::Messenger::s_validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

  #pragma warning(push)
  #pragma warning(disable : 4100)
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                               VkDebugUtilsMessageTypeFlagsEXT message_types,
                               VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
                               void* p_user_data) {
  std::cerr << std::format("[****] validation layer: {}\n", p_callback_data->pMessage) << std::endl;

  return VK_FALSE;
}
  #pragma warning(pop)

static bool check_validation_layer_support(const std::vector<const char*>& validation_layers) {
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

pandora::core::gpu::debug::Messenger::~Messenger() {
  m_ptrMessenger.release();
}

vk::UniqueInstance pandora::core::gpu::debug::Messenger::createDebugInstance(
    const vk::ApplicationInfo& app_info,
    const std::vector<const char*>& extensions) {
  if (!check_validation_layer_support(s_validationLayers)) {
    return vk::UniqueInstance(nullptr);
  }

  const vk::DebugUtilsMessageSeverityFlagsEXT severity_flags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                                                             | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                                             | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

  const vk::DebugUtilsMessageTypeFlagsEXT type_flags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                                     | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                                     | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

  vk::ValidationFeaturesEXT validation_features;
  std::vector<vk::ValidationFeatureEnableEXT> enabled_features = {vk::ValidationFeatureEnableEXT::eDebugPrintf};
  validation_features.setEnabledValidationFeatures(enabled_features);

  const vk::StructureChain<vk::InstanceCreateInfo, vk::ValidationFeaturesEXT, vk::DebugUtilsMessengerCreateInfoEXT>
      create_info_chain({{}, &app_info, s_validationLayers, extensions},
                        validation_features,
                        {{}, severity_flags, type_flags, &debug_utils_messenger_callback});

  auto ptr_vk_instance = vk::createInstanceUnique(create_info_chain.get<vk::InstanceCreateInfo>());
  VULKAN_HPP_DEFAULT_DISPATCHER.init(*ptr_vk_instance);
  m_ptrMessenger = ptr_vk_instance->createDebugUtilsMessengerEXTUnique(
      create_info_chain.get<vk::DebugUtilsMessengerCreateInfoEXT>());

  return std::move(ptr_vk_instance);
}
#endif
