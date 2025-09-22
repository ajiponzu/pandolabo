/*
 * functions.hpp
 * - Common utility functions for Pandolabo Vulkan C++ wrapper
 */

#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

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
  WaitedFences(const std::vector<gpu::Fence>& fences);
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

template <typename T>
concept SemaphoreConcept = requires(T a) {
  { a.getSemaphore() } -> std::same_as<const vk::Semaphore&>;
};

template <typename T>
concept WaitConcept = SemaphoreConcept<T> && requires(T a) {
  { a.getWaitValue() } -> std::same_as<uint64_t>;
};

template <typename T>
concept SignalConcept = SemaphoreConcept<T> && requires(T a) {
  { a.getSignalValue() } -> std::same_as<uint64_t>;
};

template <typename T>
concept WaitSemaphoreConcept =
    (SemaphoreConcept<T> || WaitConcept<T>) && !SignalConcept<T>;

template <typename T>
concept SignalSemaphoreConcept =
    (SemaphoreConcept<T> || SignalConcept<T>) && !WaitConcept<T>;

/// @brief Group of semaphores for GPU submission
/// This class manages a collection of semaphores to be used in GPU command
/// submissions. It allows adding multiple wait and signal semaphores, along
/// with their associated timeline values if applicable. The group can then be
/// used to submit all semaphores at once.
class SubmitSemaphoreGroup {
 private:
  std::vector<vk::Semaphore> m_waitSemaphores;
  std::vector<vk::Semaphore> m_signalSemaphores;

  std::vector<uint64_t> m_waitValues;
  std::vector<uint64_t> m_signalValues;

  mutable std::vector<vk::PipelineStageFlags> m_waitStages;

  vk::TimelineSemaphoreSubmitInfoKHR m_timelineSubmitInfo{};

 public:
  SubmitSemaphoreGroup() = default;
  ~SubmitSemaphoreGroup() = default;
  SubmitSemaphoreGroup(const SubmitSemaphoreGroup&) = delete;
  SubmitSemaphoreGroup& operator=(const SubmitSemaphoreGroup&) = delete;
  SubmitSemaphoreGroup(SubmitSemaphoreGroup&&) = default;
  SubmitSemaphoreGroup& operator=(SubmitSemaphoreGroup&&) = default;

  /// @brief Create wait semaphores
  /// @tparam T Semaphore type
  /// @param semaphore Semaphore object (one or more)
  template <pandora::core::WaitSemaphoreConcept... Ts>
  pandora::core::SubmitSemaphoreGroup& setWaitSemaphores(
      const Ts&... semaphore) {
    m_waitValues.clear();
    m_waitSemaphores.clear();
    m_waitValues.reserve(sizeof...(Ts));
    m_waitSemaphores.reserve(sizeof...(Ts));

    (addWaitSemaphore(semaphore), ...);
    m_timelineSubmitInfo.setWaitSemaphoreValues(m_waitValues);

    return *this;
  }

  /// @brief Create signal semaphores
  /// @tparam T Semaphore type
  /// @param semaphore Semaphore object (one or more)
  template <pandora::core::SignalSemaphoreConcept... Ts>
  pandora::core::SubmitSemaphoreGroup& setSignalSemaphores(
      const Ts&... semaphore) {
    m_signalValues.clear();
    m_signalSemaphores.clear();
    m_signalValues.reserve(sizeof...(Ts));
    m_signalSemaphores.reserve(sizeof...(Ts));

    (addSignalSemaphore(semaphore), ...);
    m_timelineSubmitInfo.setSignalSemaphoreValues(m_signalValues);

    return *this;
  }

  void setWaitStages(const std::vector<PipelineStage>& stages) const;

  /// @brief Get pointer to timeline submit info for vk::SubmitInfo
  /// @return Pointer to vk::TimelineSemaphoreSubmitInfoKHR
  auto getPtrTimelineSubmitInfo() const {
    return &m_timelineSubmitInfo;
  }

  /// @brief Get wait semaphores for vk::SubmitInfo
  /// @return Wait semaphores
  const auto& getWaitSemaphores() const {
    return m_waitSemaphores;
  }

  /// @brief Get signal semaphores for vk::SubmitInfo
  /// @return Signal semaphores
  const auto& getSignalSemaphores() const {
    return m_signalSemaphores;
  }

  /// @brief Get wait pipeline stages for vk::SubmitInfo
  /// @return Wait pipeline stages
  const auto& getWaitStages() const {
    return m_waitStages;
  }

 private:
  /// @brief Add wait semaphore to the submission info
  /// @tparam T Semaphore type
  /// @param semaphore Semaphore object
  template <pandora::core::WaitSemaphoreConcept T>
  void addWaitSemaphore(const T& semaphore) {
    m_waitSemaphores.push_back(semaphore.getSemaphore());
    if constexpr (std::is_same_v<T, gpu::TimelineSemaphore::WaitInfo>) {
      m_waitValues.push_back(
          static_cast<const gpu::TimelineSemaphore::WaitInfo&>(semaphore)
              .getWaitValue());
    } else {
      m_waitValues.push_back(0u);
    }
  }

  /// @brief Add signal semaphore to the submission info
  /// @tparam T Semaphore type
  /// @param semaphore Semaphore object
  template <pandora::core::SignalSemaphoreConcept T>
  void addSignalSemaphore(const T& semaphore) {
    m_signalSemaphores.push_back(semaphore.getSemaphore());
    if constexpr (std::is_same_v<T, gpu::TimelineSemaphore::SignalInfo>) {
      const auto& signal_info =
          static_cast<const gpu::TimelineSemaphore::SignalInfo&>(semaphore);

      m_signalValues.push_back(signal_info.getSignalValue());
    } else {
      m_signalValues.push_back(0u);
    }
  }
};

}  // namespace pandora::core
