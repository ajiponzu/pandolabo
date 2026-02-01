/*
 * functions.hpp
 * - Common utility functions for Pandolabo Vulkan C++ wrapper
 */

#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "gpu.hpp"
#include "gpu/vk_helper.hpp"
#include "io.hpp"
#include "structures.hpp"
#include "types.hpp"
#include "ui.hpp"

namespace pandora::core {

class BarrierDependency {
 private:
  std::vector<vk::MemoryBarrier2> m_memoryBarriers{};
  std::vector<vk::BufferMemoryBarrier2> m_bufferBarriers{};
  std::vector<vk::ImageMemoryBarrier2> m_imageBarriers{};

  vk::DependencyInfo m_dependencyInfo{};

 public:
  BarrierDependency() = default;
  ~BarrierDependency() = default;
  BarrierDependency(const BarrierDependency&) = delete;
  BarrierDependency& operator=(const BarrierDependency&) = delete;
  BarrierDependency(BarrierDependency&&) = default;
  BarrierDependency& operator=(BarrierDependency&&) = default;

  BarrierDependency& setMemoryBarriers(
      const std::vector<std::reference_wrapper<const gpu::MemoryBarrier>>&
          memory_barriers);

  BarrierDependency& setBufferBarriers(
      const std::vector<std::reference_wrapper<const gpu::BufferBarrier>>&
          buffer_barriers);

  BarrierDependency& setImageBarriers(
      const std::vector<std::reference_wrapper<const gpu::ImageBarrier>>&
          image_barriers);

  const vk::DependencyInfo& getDependencyInfo() const {
    return m_dependencyInfo;
  }
};

/// @brief Driver class for managing multiple fences
/// This class provides methods to wait on multiple fences
class WaitedFences {
 private:
  std::vector<vk::Fence> m_fences;  ///< Fences

 public:
  WaitedFences(const std::vector<gpu::Fence>& fences);
  ~WaitedFences() = default;
  WaitedFences(const WaitedFences&) = delete;
  WaitedFences& operator=(const WaitedFences&) = delete;
  WaitedFences(WaitedFences&&) = default;
  WaitedFences& operator=(WaitedFences&&) = default;

  /// @brief Wait for multiple fences to be signaled
  /// @param context GPU context for device operations
  /// @param timeout Timeout duration in nanoseconds
  /// @param is_reset If true, reset fences after waiting
  /// @return True if all fences were signaled within the timeout, false
  /// otherwise
  bool wait(const gpu::Context& context,
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
  /// @param context GPU context for device operations
  /// @param timeout Timeout duration in nanoseconds
  /// @return True if all semaphores reached the specified values within the
  /// timeout, false otherwise
  bool wait(const gpu::Context& context,
            uint64_t timeout = std::numeric_limits<uint64_t>::max());

  /// @brief Signal multiple timeline semaphores to reach specified values
  /// @param context GPU context for device operations
  void signal(const gpu::Context& context);
};

template <typename T>
concept SemaphoreConcept = requires(T a) {
  { a.getSemaphore() } -> std::same_as<const vk::Semaphore&>;
};

/// @brief Semaphore used in GPU submission
/// This class encapsulates a semaphore along with its associated timeline value
/// and pipeline stage mask for GPU command submissions. It provides methods to
/// set these parameters and retrieve the underlying Vulkan semaphore submission
/// info.
class SubmitSemaphore {
 private:
  vk::SemaphoreSubmitInfo m_semaphoreSubmitInfo{};

 public:
  SubmitSemaphore() = default;
  ~SubmitSemaphore() = default;

  const vk::SemaphoreSubmitInfo& getSemaphoreSubmitInfo() const {
    return m_semaphoreSubmitInfo;
  }

  template <pandora::core::SemaphoreConcept T>
  SubmitSemaphore& setSemaphore(const T& semaphore) {
    m_semaphoreSubmitInfo.setSemaphore(semaphore.getSemaphore());
    return *this;
  }

  SubmitSemaphore& setValue(uint64_t value) {
    m_semaphoreSubmitInfo.setValue(value);
    return *this;
  }

  SubmitSemaphore& setStageMask(const PipelineStage& stage_mask) {
    m_semaphoreSubmitInfo.setStageMask(
        vk_helper::getPipelineStageFlagBits(stage_mask));
    return *this;
  }
};

/// @brief Group of semaphores used in GPU submission
/// This class manages a group of semaphores to be used for waiting and
/// signaling during GPU command submissions. It provides methods to add wait
/// and signal semaphores, and retrieve the lists of semaphores for submission.
class SubmitSemaphoreGroup {
 private:
  std::vector<vk::SemaphoreSubmitInfo> m_waitSemaphores;
  std::vector<vk::SemaphoreSubmitInfo> m_signalSemaphores;

 public:
  SubmitSemaphoreGroup() = default;
  ~SubmitSemaphoreGroup() = default;

  SubmitSemaphoreGroup& setWaitSemaphores(
      const std::vector<SubmitSemaphore>& semaphores);
  SubmitSemaphoreGroup& setSignalSemaphores(
      const std::vector<SubmitSemaphore>& semaphores);

  const std::vector<vk::SemaphoreSubmitInfo>& getWaitSemaphores() const {
    return m_waitSemaphores;
  }

  const std::vector<vk::SemaphoreSubmitInfo>& getSignalSemaphores() const {
    return m_signalSemaphores;
  }
};

}  // namespace pandora::core
