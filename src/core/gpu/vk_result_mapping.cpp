#include <vulkan/vulkan.hpp>

#include "pandora/core/err/error.hpp"

namespace pandora::core::gpu {
using namespace pandora::core::err;

static inline Error make(
    Domain d, Code c, Severity s, vk::Result r, const char* expr) {
  Error e;
  e.domain = d;
  e.code = c;
  e.severity = s;
  e.native_code = static_cast<std::uint32_t>(r);
  e.message = std::string("vk result ") + std::to_string(e.native_code) + " ("
              + expr + ")";
  return e;
}

Error map_vk_result(vk::Result r, const char* expr_text) {
  using enum Domain;
  using enum Code;
  switch (r) {
    case vk::Result::eSuccess:
      return make(
          internal, Code::ok, Severity::note, r, expr_text);  // caller handles
    case vk::Result::eNotReady:
      return make(gpu, retry_later, Severity::recoverable, r, expr_text);
    case vk::Result::eTimeout:
      return make(gpu, timeout, Severity::recoverable, r, expr_text);
    case vk::Result::eEventSet:
      return make(gpu, Code::note, Severity::note, r, expr_text);
    case vk::Result::eEventReset:
      return make(gpu, Code::note, Severity::note, r, expr_text);
    case vk::Result::eIncomplete:
      return make(gpu, retry_later, Severity::recoverable, r, expr_text);
    case vk::Result::eErrorOutOfHostMemory:
      return make(memory, out_of_memory, Severity::fatal, r, expr_text);
    case vk::Result::eErrorOutOfDeviceMemory:
      return make(memory, out_of_memory, Severity::fatal, r, expr_text);
    case vk::Result::eErrorInitializationFailed:
      return make(gpu, invalid_state, Severity::fatal, r, expr_text);
    case vk::Result::eErrorDeviceLost:
      return make(gpu, device_lost, Severity::fatal, r, expr_text);
    case vk::Result::eErrorMemoryMapFailed:
      return make(memory, invalid_state, Severity::recoverable, r, expr_text);
    case vk::Result::eErrorLayerNotPresent:
      return make(gpu, invalid_argument, Severity::fatal, r, expr_text);
    case vk::Result::eErrorExtensionNotPresent:
      return make(gpu, invalid_argument, Severity::fatal, r, expr_text);
    case vk::Result::eErrorFeatureNotPresent:
      return make(gpu, invalid_argument, Severity::fatal, r, expr_text);
    case vk::Result::eErrorIncompatibleDriver:
      return make(gpu, invalid_state, Severity::fatal, r, expr_text);
    case vk::Result::eErrorTooManyObjects:
      return make(gpu, invalid_state, Severity::recoverable, r, expr_text);
    case vk::Result::eErrorFormatNotSupported:
      return make(gpu, invalid_argument, Severity::recoverable, r, expr_text);
    case vk::Result::eErrorFragmentedPool:
      return make(memory, invalid_state, Severity::recoverable, r, expr_text);
    case vk::Result::eErrorSurfaceLostKHR:
      return make(gpu_swapchain, device_lost, Severity::fatal, r, expr_text);
    case vk::Result::eErrorNativeWindowInUseKHR:
      return make(gpu_swapchain, invalid_state, Severity::fatal, r, expr_text);
    case vk::Result::eSuboptimalKHR:
      return make(gpu_swapchain,
                  swapchain_out_of_date,
                  Severity::recoverable,
                  r,
                  expr_text);
    case vk::Result::eErrorOutOfDateKHR:
      return make(gpu_swapchain,
                  swapchain_out_of_date,
                  Severity::recoverable,
                  r,
                  expr_text);
    case vk::Result::eErrorIncompatibleDisplayKHR:
      return make(gpu_swapchain, invalid_state, Severity::fatal, r, expr_text);
    case vk::Result::eErrorValidationFailedEXT:
      return make(gpu_validation,
                  invalid_argument,
                  Severity::recoverable,
                  r,
                  expr_text);
    case vk::Result::eErrorInvalidShaderNV:
      return make(gpu_shader,
                  shader_compile_failed,
                  Severity::recoverable,
                  r,
                  expr_text);
#if defined(VK_VERSION_1_1) || defined(VK_KHR_maintenance1) \
    || defined(VK_KHR_device_group)
    case vk::Result::eErrorOutOfPoolMemory:
      return make(memory, out_of_memory, Severity::recoverable, r, expr_text);
#endif
    case vk::Result::eErrorInvalidExternalHandle:
      return make(gpu, invalid_argument, Severity::recoverable, r, expr_text);
#ifdef VK_EXT_FRAGMENTATION_EXTENSION
    case vk::Result::eErrorFragmentationEXT:
      return make(memory, invalid_state, Severity::recoverable, r, expr_text);
#endif
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    case vk::Result::eErrorNotPermittedKHR:
      return make(gpu, invalid_state, Severity::fatal, r, expr_text);
#endif
#ifdef VK_EXT_full_screen_exclusive
    case vk::Result::eErrorFullScreenExclusiveModeLostEXT:
      return make(gpu_swapchain,
                  swapchain_out_of_date,
                  Severity::recoverable,
                  r,
                  expr_text);
#endif
#ifdef VK_KHR_deferred_host_operations
    case vk::Result::eThreadIdleKHR:
      return make(gpu, Code::note, Severity::note, r, expr_text);
    case vk::Result::eThreadDoneKHR:
      return make(gpu, Code::note, Severity::note, r, expr_text);
    case vk::Result::eOperationDeferredKHR:
      return make(gpu, retry_later, Severity::recoverable, r, expr_text);
    case vk::Result::eOperationNotDeferredKHR:
      return make(gpu, invalid_state, Severity::recoverable, r, expr_text);
#endif
#ifdef VK_EXT_pipeline_compiler_control
    case vk::Result::ePipelineCompileRequiredEXT:
      return make(
          gpu_pipeline, retry_later, Severity::recoverable, r, expr_text);
#endif
    default:
      return make(gpu, unknown, Severity::recoverable, r, expr_text);
  }
}

}  // namespace pandora::core::gpu
