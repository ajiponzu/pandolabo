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
  class WaitInfo {
   private:
    friend class TimelineSemaphore;

    vk::Semaphore m_semaphore;
    uint64_t m_waitValue =
        0u;  ///< Current wait value to be used when waiting on the semaphore

    WaitInfo(const vk::Semaphore& semaphore, uint64_t wait_value)
        : m_semaphore(semaphore), m_waitValue(wait_value) {}

   public:
    ~WaitInfo() = default;

    /// @brief Get current wait value
    /// @return Current wait value
    uint64_t getWaitValue() const {
      return m_waitValue;
    }

    /// @brief Get underlying Vulkan semaphore handle
    /// @return Const reference to the Vulkan semaphore
    const auto& getSemaphore() const {
      return m_semaphore;
    }
  };
  class SignalInfo {
   private:
    friend class TimelineSemaphore;

    vk::Semaphore m_semaphore;
    uint64_t m_signalValue =
        1u;  ///< Current signal value to be used when signaling the semaphore

    SignalInfo(const vk::Semaphore& semaphore, uint64_t signal_value)
        : m_semaphore(semaphore), m_signalValue(signal_value) {}

   public:
    ~SignalInfo() = default;

    /// @brief Get current signal value
    /// @return Current signal value
    uint64_t getSignalValue() const {
      return m_signalValue;
    }

    /// @brief Get underlying Vulkan semaphore handle
    /// @return Const reference to the Vulkan semaphore
    const auto& getSemaphore() const {
      return m_semaphore;
    }
  };

  /// @brief Construct timeline semaphore
  /// @param ptr_context GPU context pointer for device access
  TimelineSemaphore(const std::unique_ptr<Context>& ptr_context);

  // Rule of Five
  ~TimelineSemaphore();
  TimelineSemaphore(const TimelineSemaphore&) = delete;
  TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
  TimelineSemaphore(TimelineSemaphore&&) = default;
  TimelineSemaphore& operator=(TimelineSemaphore&&) = default;

  /// @brief Create submission info for the wait timeline semaphore
  /// @param wait_value
  /// @return Submission info for the timeline semaphore
  auto forWait(uint64_t wait_value) const {
    return WaitInfo{m_ptrSemaphore.get(), wait_value};
  }
  /// @brief Create submission info for the signal timeline semaphore
  /// @param signal_value
  /// @return Submission info for the timeline semaphore
  auto forSignal(uint64_t signal_value) const {
    return SignalInfo{m_ptrSemaphore.get(), signal_value};
  }

  const auto& getTimelineSemaphore() const {
    return m_ptrSemaphore.get();
  }
};

}  // namespace pandora::core::gpu
