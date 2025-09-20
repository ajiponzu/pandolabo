/*
 * functions.hpp
 * - Common utility functions for Pandolabo Vulkan C++ wrapper
 */

#pragma once

#include "gpu.hpp"
#include "io.hpp"
#include "ui.hpp"

namespace pandora::core {

/// @brief Driver class for managing multiple fences
/// This class provides methods to wait on multiple fences
class WaitedFences {
 private:
  std::vector<vk::Fence> m_fences;  ///< Fences

 public:
  WaitedFences(const std::vector<vk::Fence>& fences) : m_fences(fences) {}
  ~WaitedFences() = default;
  WaitedFences(const WaitedFences&) = delete;
  WaitedFences& operator=(const WaitedFences&) = delete;
  WaitedFences(WaitedFences&&) = default;
  WaitedFences& operator=(WaitedFences&&) = default;

  /// @brief Wait for multiple fences to be signaled
  /// @param ptr_context GPU context for device operations
  /// @param timeout Timeout duration in nanoseconds
  /// @param is_reset If true, reset fences after waiting
  /// @return True if all fences were signaled within the timeout, false
  /// otherwise
  bool wait(const std::unique_ptr<gpu::Context>& ptr_context,
            uint64_t timeout = std::numeric_limits<uint64_t>::max(),
            bool is_reset = true);
};

/// @brief Driver class for managing multiple timeline semaphores
/// This class provides methods to wait on and signal multiple timeline
/// semaphores
class TimelineSemaphoreDriver {
 private:
  std::vector<vk::Semaphore> m_semaphores;  ///< Semaphores
  std::vector<uint64_t> m_values;           ///< Values to wait/signal

 public:
  TimelineSemaphoreDriver() = default;
  ~TimelineSemaphoreDriver() = default;
  TimelineSemaphoreDriver(const TimelineSemaphoreDriver&) = delete;
  TimelineSemaphoreDriver& operator=(const TimelineSemaphoreDriver&) = delete;
  TimelineSemaphoreDriver(TimelineSemaphoreDriver&&) = default;
  TimelineSemaphoreDriver& operator=(TimelineSemaphoreDriver&&) = default;

  TimelineSemaphoreDriver& setSemaphores(
      const std::vector<std::reference_wrapper<gpu::TimelineSemaphore>>&
          semaphores);
  TimelineSemaphoreDriver& setValues(const std::vector<uint64_t>& values);

  /// @brief Wait for multiple timeline semaphores to reach specified values
  /// @param ptr_context GPU context for device operations
  /// @param timeout Timeout duration in nanoseconds
  /// @return True if all semaphores reached the specified values within the
  /// timeout, false otherwise
  bool wait(const std::unique_ptr<gpu::Context>& ptr_context,
            uint64_t timeout = std::numeric_limits<uint64_t>::max());

  /// @brief Signal multiple timeline semaphores to reach specified values
  /// @param ptr_context GPU context for device operations
  void signal(const std::unique_ptr<gpu::Context>& ptr_context);
};

}  // namespace pandora::core
