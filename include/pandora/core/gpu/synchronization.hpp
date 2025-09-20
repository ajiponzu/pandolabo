/*
 * synchronization.hpp - GPU synchronization primitives for Pandolabo Vulkan C++
 * wrapper
 *
 * This header contains memory barriers and semaphore classes for managing
 * GPU synchronization and memory access ordering.
 */

#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../structures.hpp"
#include "../types.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Context;
class Buffer;
class Image;
}  // namespace pandora::core::gpu

namespace pandora::core::gpu {

/// @brief Vulkan buffer memory barrier wrapper class
/// This class manages synchronization for buffer memory access operations.
/// It provides control over:
/// - Memory access ordering (read-after-write, write-after-read dependencies)
/// - Queue family ownership transfers for multi-queue operations
/// - Memory visibility between different pipeline stages
///
/// @note For queue family transfers, create two barriers:
/// 1. Release barrier with source queue family index
/// 2. Acquire barrier with destination queue family index
class BufferBarrier {
 private:
  vk::BufferMemoryBarrier m_bufferMemoryBarrier{};

 public:
  /// @brief Construct buffer memory barrier
  /// @param buffer The buffer to synchronize
  /// @param priority_access_flags Memory access types that must complete before
  /// this barrier
  /// @param wait_access_flags Memory access types that wait for this barrier
  BufferBarrier(const Buffer& buffer,
                std::span<const AccessFlag> priority_access_flags,
                std::span<const AccessFlag> wait_access_flags,
                uint32_t src_queue_family = 0u,
                uint32_t dst_queue_family = 0u);
  ~BufferBarrier();

  /// @brief Get const reference to underlying Vulkan barrier
  /// @return Const reference to vk::BufferMemoryBarrier
  const auto& getBarrier() const {
    return m_bufferMemoryBarrier;
  }

  /// @brief Set source queue family index for ownership transfer
  /// @param index Queue family index that currently owns the buffer
  void setSrcQueueFamilyIndex(const uint32_t index) {
    m_bufferMemoryBarrier.setSrcQueueFamilyIndex(index);
  }

  /// @brief Set destination queue family index for ownership transfer
  /// @param index Queue family index that will receive buffer ownership
  void setDstQueueFamilyIndex(const uint32_t index) {
    m_bufferMemoryBarrier.setDstQueueFamilyIndex(index);
  }
};

/// @brief Builder class for BufferBarrier
/// Provides a fluent interface for constructing BufferBarrier instances
class BufferBarrierBuilder {
 private:
  const Buffer* m_ptrBuffer = nullptr;
  std::span<const AccessFlag> m_priorityAccessFlags{};
  std::span<const AccessFlag> m_waitAccessFlags{};
  uint32_t m_srcQueueFamily = 0u;
  uint32_t m_dstQueueFamily = 0u;

  // Private constructor - use create() factory method instead
  BufferBarrierBuilder() = default;

 public:
  /// @brief Static factory method to create a new BufferBarrierBuilder
  /// @return A new BufferBarrierBuilder instance
  static BufferBarrierBuilder create() {
    return BufferBarrierBuilder{};
  }

  /// @brief Set the buffer to synchronize
  /// @param buffer The buffer to apply the barrier to
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setBuffer(const Buffer& buffer) {
    m_ptrBuffer = &buffer;
    return *this;
  }

  /// @brief Set priority access flags
  /// @param flags Memory access types that must complete before this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setPriorityAccessFlags(
      std::span<const AccessFlag> flags) {
    m_priorityAccessFlags = flags;
    return *this;
  }

  /// @brief Set wait access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setWaitAccessFlags(std::span<const AccessFlag> flags) {
    m_waitAccessFlags = flags;
    return *this;
  }

  /// @brief Set source queue family index for ownership transfer
  /// @param index Queue family index that currently owns the buffer
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setSrcQueueFamilyIndex(const uint32_t index) {
    m_srcQueueFamily = index;
    return *this;
  }

  /// @brief Set destination queue family index for ownership transfer
  /// @param index Queue family index that will receive buffer ownership
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setDstQueueFamilyIndex(const uint32_t index) {
    m_dstQueueFamily = index;
    return *this;
  }

  /// @brief Build and return the final BufferBarrier instance
  /// @return BufferBarrier instance
  /// @throws std::runtime_error if buffer is not set
  BufferBarrier build() {
    if (!m_ptrBuffer) {
      throw std::runtime_error(
          "Buffer must be set before building BufferBarrier");
    }

    return BufferBarrier(*m_ptrBuffer,
                         m_priorityAccessFlags,
                         m_waitAccessFlags,
                         m_srcQueueFamily,
                         m_dstQueueFamily);
  }
};

/// @brief Vulkan image memory barrier wrapper class
/// This class manages synchronization for image memory access and layout
/// transitions. It provides control over:
/// - Memory access ordering for image operations
/// - Image layout transitions (e.g., undefined -> color attachment)
/// - Queue family ownership transfers for multi-queue image operations
/// - Subresource range specification for partial image barriers
///
/// @note For queue family transfers:
/// 1. Release barrier: Set source queue family, keep current layout
/// 2. Acquire barrier: Set destination queue family, perform layout transition
/// @warning Do not change image layout in release barrier, only in acquire
/// barrier
class ImageBarrier {
 private:
  vk::ImageMemoryBarrier m_imageMemoryBarrier{};

 public:
  ImageBarrier(const Image& image,
               std::span<const AccessFlag> priority_access_flags,
               std::span<const AccessFlag> wait_access_flags,
               const ImageLayout old_layout,
               const ImageLayout new_layout,
               const ImageViewInfo& image_view_info,
               uint32_t src_queue_family = 0u,
               uint32_t dst_queue_family = 0u);
  ImageBarrier(const std::unique_ptr<Context>& ptr_context,
               std::span<const AccessFlag> priority_access_flags,
               std::span<const AccessFlag> wait_access_flags,
               const ImageLayout old_layout,
               const ImageLayout new_layout,
               uint32_t src_queue_family = 0u,
               uint32_t dst_queue_family = 0u);
  ~ImageBarrier();

  const auto& getBarrier() const {
    return m_imageMemoryBarrier;
  }

  void setSrcQueueFamilyIndex(const uint32_t index) {
    m_imageMemoryBarrier.setSrcQueueFamilyIndex(index);
  }

  void setDstQueueFamilyIndex(const uint32_t index) {
    m_imageMemoryBarrier.setDstQueueFamilyIndex(index);
  }
};

/// @brief Builder class for ImageBarrier
/// Provides a fluent interface for constructing ImageBarrier instances
class ImageBarrierBuilder {
 private:
  const Image* m_ptrImage = nullptr;
  std::span<const AccessFlag> m_priorityAccessFlags{};
  std::span<const AccessFlag> m_waitAccessFlags{};
  ImageLayout m_oldLayout = ImageLayout::Undefined;
  ImageLayout m_newLayout = ImageLayout::Undefined;
  std::optional<ImageViewInfo> m_imageViewInfo{};
  uint32_t m_srcQueueFamily = 0u;
  uint32_t m_dstQueueFamily = 0u;

  // Private constructor - use create() factory method instead
  ImageBarrierBuilder() = default;

 public:
  /// @brief Static factory method to create a new ImageBarrierBuilder
  /// @return A new ImageBarrierBuilder instance
  static ImageBarrierBuilder create() {
    return ImageBarrierBuilder{};
  }

  /// @brief Set the image to synchronize
  /// @param image The image to apply the barrier to
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setImage(const Image& image) {
    m_ptrImage = &image;
    return *this;
  }

  /// @brief Set priority access flags
  /// @param flags Memory access types that must complete before this barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setPriorityAccessFlags(
      std::span<const AccessFlag> flags) {
    m_priorityAccessFlags = flags;
    return *this;
  }

  /// @brief Set wait access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setWaitAccessFlags(std::span<const AccessFlag> flags) {
    m_waitAccessFlags = flags;
    return *this;
  }

  /// @brief Set old image layout
  /// @param layout The current layout of the image
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setOldLayout(const ImageLayout layout) {
    m_oldLayout = layout;
    return *this;
  }

  /// @brief Set new image layout
  /// @param layout The desired layout after the barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setNewLayout(const ImageLayout layout) {
    m_newLayout = layout;
    return *this;
  }

  /// @brief Set image view info for subresource specification
  /// @param info Image view information specifying which parts of the image to
  /// barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setImageViewInfo(const ImageViewInfo& info) {
    m_imageViewInfo = info;
    return *this;
  }

  /// @brief Set source queue family index for ownership transfer
  /// @param index Queue family index that currently owns the buffer
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setSrcQueueFamilyIndex(const uint32_t index) {
    m_srcQueueFamily = index;
    return *this;
  }

  /// @brief Set destination queue family index for ownership transfer
  /// @param index Queue family index that will receive buffer ownership
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setDstQueueFamilyIndex(const uint32_t index) {
    m_dstQueueFamily = index;
    return *this;
  }

  /// @brief Build and return the final ImageBarrier instance
  /// @param ptr_context GPU context pointer (required for context-based
  /// construction)
  /// @return ImageBarrier instance
  /// @throws std::runtime_error if required parameters are not set
  ImageBarrier build(const std::unique_ptr<Context>& ptr_context = nullptr) {
    if (m_ptrImage && m_imageViewInfo) {
      // Build with image and image view info
      return ImageBarrier(*m_ptrImage,
                          m_priorityAccessFlags,
                          m_waitAccessFlags,
                          m_oldLayout,
                          m_newLayout,
                          *m_imageViewInfo,
                          m_srcQueueFamily,
                          m_dstQueueFamily);
    } else if (ptr_context) {
      // Build with context
      return ImageBarrier(ptr_context,
                          m_priorityAccessFlags,
                          m_waitAccessFlags,
                          m_oldLayout,
                          m_newLayout,
                          m_srcQueueFamily,
                          m_dstQueueFamily);
    } else {
      throw std::runtime_error(
          "Either image with ImageViewInfo or context parameter must be "
          "provided for building ImageBarrier");
    }
  }
};

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
    ~WaitInfo() = default;

   public:
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
    ~SignalInfo() = default;

   public:
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

template <typename T>
concept SemaphoreConcept = requires(T a) {
  { a.getSemaphore() } -> std::same_as<const vk::Semaphore&>;
};

template <typename T>
concept WaitConcept = SemaphoreConcept<T> && requires(T a) {
  { a.m_waitValue };
  { a.getWaitValue() } -> std::same_as<uint64_t>;
};

template <typename T>
concept SignalConcept = SemaphoreConcept<T> && requires(T a) {
  { a.m_signalValue };
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
  template <pandora::core::gpu::WaitSemaphoreConcept... Ts>
  pandora::core::gpu::SubmitSemaphoreGroup& setWaitSemaphores(
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
  template <pandora::core::gpu::SignalSemaphoreConcept... Ts>
  pandora::core::gpu::SubmitSemaphoreGroup& setSignalSemaphores(
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
  const auto getPtrTimelineSubmitInfo() const {
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
  template <pandora::core::gpu::WaitSemaphoreConcept T>
  void addWaitSemaphore(const T& semaphore) {
    m_waitSemaphores.push_back(semaphore.getSemaphore());
    if constexpr (std::is_same_v<T, TimelineSemaphore::WaitInfo>) {
      m_waitValues.push_back(
          static_cast<const TimelineSemaphore::WaitInfo&>(semaphore)
              .getWaitValue());
    } else {
      m_waitValues.push_back(0u);
    }
  }

  /// @brief Add signal semaphore to the submission info
  /// @tparam T Semaphore type
  /// @param semaphore Semaphore object
  template <pandora::core::gpu::SignalSemaphoreConcept T>
  void addSignalSemaphore(const T& semaphore) {
    m_signalSemaphores.push_back(semaphore.getSemaphore());
    if constexpr (std::is_same_v<T, TimelineSemaphore::SignalInfo>) {
      const auto& signal_info =
          static_cast<const TimelineSemaphore::SignalInfo&>(semaphore);

      m_signalValues.push_back(signal_info.getSignalValue());
    } else {
      m_signalValues.push_back(0u);
    }
  }
};

}  // namespace pandora::core::gpu
