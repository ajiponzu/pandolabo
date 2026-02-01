#pragma once

#include <cstdint>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "../structures.hpp"
#include "../types.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Context;
}  // namespace pandora::core::gpu

namespace pandora::core::gpu {

/// @brief Vulkan binary semaphore wrapper
/// This class provides a reference interface to Vulkan binary semaphores.
/// Binary semaphores are used for GPU-GPU synchronization, ensuring that
/// one operation is completed before another begins. They are signaled and
/// waited on by the GPU, and do not support timeline values.
/// This class does not manage the semaphore's lifetime; it is assumed that
/// the semaphore is created and destroyed in swapchain (etc?).
class BinarySemaphore {
 private:
  vk::Semaphore m_semaphore;

 public:
  BinarySemaphore(const vk::Semaphore& semaphore) : m_semaphore(semaphore) {}
  ~BinarySemaphore() = default;

  const auto& getSemaphore() const {
    return m_semaphore;
  }
};

/// @brief Vulkan timeline semaphore wrapper class
/// Timeline semaphores provide advanced synchronization with monotonically
/// increasing values. They enable fine-grained control over GPU/CPU
/// synchronization and support:
/// - Wait-before-signal semantics for complex dependency chains
/// - Host-side waiting and signaling operations
/// - Multiple wait and signal operations on the same semaphore
///
/// @note Requires Vulkan API version 1.2 or higher
/// @note Timeline values must be monotonically increasing
class TimelineSemaphore {
 private:
  vk::UniqueSemaphore m_ptrSemaphore;  ///< Underlying Vulkan timeline semaphore

 public:
  /// @brief Construct timeline semaphore
  /// @param ptr_context GPU context pointer for device access
  TimelineSemaphore(const Context& ptr_context);

  // Rule of Five
  ~TimelineSemaphore();
  TimelineSemaphore(const TimelineSemaphore&) = delete;
  TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
  TimelineSemaphore(TimelineSemaphore&&) = default;
  TimelineSemaphore& operator=(TimelineSemaphore&&) = default;

  const auto& getSemaphore() const {
    return m_ptrSemaphore.get();
  }
};

}  // namespace pandora::core::gpu
