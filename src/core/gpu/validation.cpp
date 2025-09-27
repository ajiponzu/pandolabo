#include "pandora/core/gpu/validation.hpp"

namespace pandora::core::gpu::validation {
using namespace pandora::core::err;

Severity map_severity(VkDebugUtilsMessageSeverityFlagBitsEXT raw) noexcept {
  switch (raw) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      return Severity::note;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      return Severity::note;  // could be warning later
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      return Severity::warning;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      return Severity::recoverable;  // may escalate
    default:
      return Severity::note;
  }
}

Domain map_domain(VkDebugUtilsMessageTypeFlagsEXT types) noexcept {
  if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
    return Domain::gpu_validation;
  if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
    return Domain::gpu;
  return Domain::gpu;  // general / other -> gpu
}

namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL
messenger_cb(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
             VkDebugUtilsMessageTypeFlagsEXT message_types,
             const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
             void* /*user_data*/) {
  Error e;
  e.domain = map_domain(message_types);
  e.code =
      Code::invalid_state;  // placeholder; refined by future ID classification
  e.severity = map_severity(message_severity);
  e.native_code = callback_data ? callback_data->messageIdNumber : 0u;
  if (callback_data && callback_data->pMessage) {
    e.message = callback_data->pMessage;
  } else {
    e.message = "validation message";
  }
  dispatch_error(e);
  return VK_FALSE;  // never block
}
}  // namespace

vk::UniqueDebugUtilsMessengerEXT attach_debug_messenger(vk::Instance instance) {
  // Build create info
  vk::DebugUtilsMessengerCreateInfoEXT info{};
  info.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                          | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                          | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                          | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  info.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                      | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                      | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  info.setPfnUserCallback(
      reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&messenger_cb));
  // Attempt creation; if extension not enabled this will throw/return error
  // depending on policy upstream.
  return instance.createDebugUtilsMessengerEXTUnique(info);
}

}  // namespace pandora::core::gpu::validation
