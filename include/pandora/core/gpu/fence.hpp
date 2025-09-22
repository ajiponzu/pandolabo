#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

// Forward declarations
namespace pandora::core::gpu {
class Context;
}  // namespace pandora::core::gpu

namespace pandora::core::gpu {

/// @brief Vulkan fence wrapper class
/// This class provides a reference interface to Vulkan fences.
/// Fences are used for CPU-GPU synchronization, allowing the CPU to wait
/// for GPU operations to complete. They can be signaled by the GPU and waited
/// on by the CPU. This class does not manage the fence's lifetime; it is
/// assumed that the fence is created and destroyed in swapchain (etc?).
class Fence {
 private:
  vk::Fence m_fence;

 public:
  Fence(const vk::Fence& fence = {}) : m_fence(fence) {}
  ~Fence() = default;

  const auto& getFence() const {
    return m_fence;
  }
};

}  // namespace pandora::core::gpu
