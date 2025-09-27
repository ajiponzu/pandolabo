#pragma once
#include <vulkan/vulkan.hpp>

#include "pandora/core/err/dispatch.hpp"
#include "pandora/core/err/error.hpp"

namespace pandora::core::gpu::validation {

// Normalized severity mapping from raw VkDebugUtils severity flags.
err::Severity map_severity(VkDebugUtilsMessageSeverityFlagBitsEXT raw) noexcept;

// Light-weight category mapping from type flags (future expansion).
err::Domain map_domain(VkDebugUtilsMessageTypeFlagsEXT types) noexcept;

// Create & register a debug utils messenger that forwards messages into
// dispatch_error. Returns created messenger handle (unique) or empty if
// extension unavailable. The instance must already have debug utils extension
// enabled.
[[nodiscard]] vk::UniqueDebugUtilsMessengerEXT attach_debug_messenger(
    vk::Instance instance);

}  // namespace pandora::core::gpu::validation
