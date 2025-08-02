/*
 * synchronization.hpp - GPU synchronization primitives for Pandolabo Vulkan C++ wrapper
 *
 * This header contains memory barriers and semaphore classes for managing
 * GPU synchronization and memory access ordering.
 */

#pragma once

#include <memory>
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
  /// @param priority_access_flags Memory access types that must complete before this barrier
  /// @param wait_access_flags Memory access types that wait for this barrier
  BufferBarrier(const Buffer& buffer,
                const std::vector<AccessFlag>& priority_access_flags,
                const std::vector<AccessFlag>& wait_access_flags);
  ~BufferBarrier();

  /// @brief Get mutable reference to underlying Vulkan barrier
  /// @return Reference to vk::BufferMemoryBarrier
  auto& getBarrier() {
    return m_bufferMemoryBarrier;
  }

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

/// @brief Vulkan image memory barrier wrapper class
/// This class manages synchronization for image memory access and layout transitions.
/// It provides control over:
/// - Memory access ordering for image operations
/// - Image layout transitions (e.g., undefined -> color attachment)
/// - Queue family ownership transfers for multi-queue image operations
/// - Subresource range specification for partial image barriers
///
/// @note For queue family transfers:
/// 1. Release barrier: Set source queue family, keep current layout
/// 2. Acquire barrier: Set destination queue family, perform layout transition
/// @warning Do not change image layout in release barrier, only in acquire barrier
class ImageBarrier {
 private:
  vk::ImageMemoryBarrier m_imageMemoryBarrier{};

 public:
  ImageBarrier(const Image& image,
               const std::vector<AccessFlag>& priority_access_flags,
               const std::vector<AccessFlag>& wait_access_flags,
               const ImageLayout old_layout,
               const ImageLayout new_layout,
               const ImageViewInfo& image_view_info);
  ImageBarrier(const std::unique_ptr<Context>& ptr_context,
               const std::vector<AccessFlag>& priority_access_flags,
               const std::vector<AccessFlag>& wait_access_flags,
               const ImageLayout old_layout,
               const ImageLayout new_layout);
  ~ImageBarrier();

  auto& getBarrier() {
    return m_imageMemoryBarrier;
  }
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

/// @brief Vulkan timeline semaphore wrapper class
/// Timeline semaphores provide advanced synchronization with monotonically increasing values.
/// They enable fine-grained control over GPU/CPU synchronization and support:
/// - Wait-before-signal semantics for complex dependency chains
/// - Host-side waiting and signaling operations
/// - Multiple wait and signal operations on the same semaphore
///
/// @note Requires Vulkan API version 1.2 or higher
/// @note Timeline values must be monotonically increasing
class TimelineSemaphore {
 private:
  vk::UniqueSemaphore m_ptrSemaphore;                         ///< Underlying Vulkan timeline semaphore
  uint64_t m_signalValue = 1U;                                ///< Next value to signal
  uint64_t m_waitValue = 0U;                                  ///< Current value to wait for
  vk::TimelineSemaphoreSubmitInfoKHR m_timelineSubmitInfo{};  ///< Timeline submission info
  std::vector<vk::PipelineStageFlags> m_waitStages{};         ///< Pipeline stages to wait at

 public:
  /// @brief Construct timeline semaphore
  /// @param ptr_context GPU context pointer for device access
  TimelineSemaphore(const std::unique_ptr<Context>& ptr_context);

  // Rule of Five
  ~TimelineSemaphore();
  TimelineSemaphore(const TimelineSemaphore&) = delete;
  TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
  TimelineSemaphore(TimelineSemaphore&&) = default;
  TimelineSemaphore& operator=(TimelineSemaphore&&) = default;

  /// @brief Get underlying Vulkan semaphore handle
  /// @return Const reference to the Vulkan semaphore
  const auto& getSemaphore() const {
    return m_ptrSemaphore.get();
  }
  const auto& getSignalValue() const {
    return m_signalValue;
  }
  const auto& getWaitValue() const {
    return m_waitValue;
  }
  auto getPtrTimelineSubmitInfo() const {
    return &m_timelineSubmitInfo;
  }
  const auto& getBackWaitStage() const {
    return m_waitStages.back();
  }
  const auto& getWaitStage() const {
    return m_waitStages;
  }

  void setWaitStage(const vk::PipelineStageFlagBits wait_stage) {
    m_waitStages.push_back(wait_stage);
  }

  void updateSignalValue() {
    m_signalValue += 1;
  }
  void updateWaitValue() {
    m_waitValue += 1;
  }

  /// @brief Wait until GPU operation completes
  /// Waits until all GPU operations submitted with this semaphore have finished.
  /// @param ptr_context Vulkan context for device operations
  void wait(const std::unique_ptr<Context>& ptr_context);
};

/// @brief Vulkan binary semaphore wrapper
/// This class provides a reference interface to Vulkan binary semaphores and fences.
/// Binary semaphores are used for coarse-grained synchronization between operations.
class BinarySemaphore {
 protected:
  friend class SolidBinarySemaphore;

  vk::Semaphore m_semaphore;
  vk::Fence m_fence;

  BinarySemaphore() = default;
  ~BinarySemaphore() = default;

 public:
  const auto& getSemaphore() const {
    return m_semaphore;
  }
  const auto& getFence() const {
    return m_fence;
  }
};

/// @brief Vulkan binary semaphore implementation
/// This class represents the actual binary semaphore object created for user operations.
/// Manages the lifetime of Vulkan semaphore and fence resources.
class SolidBinarySemaphore {
 private:
  vk::UniqueSemaphore m_ptrSemaphore{};  ///< Vulkan binary semaphore
  vk::UniqueFence m_ptrFence{};          ///< Vulkan fence for CPU-GPU synchronization

 public:
  /// @brief Construct binary semaphore
  /// @param ptr_context Vulkan context for device operations
  SolidBinarySemaphore(const std::unique_ptr<Context>& ptr_context);

  // Rule of Five
  ~SolidBinarySemaphore();
  SolidBinarySemaphore(const SolidBinarySemaphore&) = delete;
  SolidBinarySemaphore& operator=(const SolidBinarySemaphore&) = delete;
  SolidBinarySemaphore(SolidBinarySemaphore&&) = default;
  SolidBinarySemaphore& operator=(SolidBinarySemaphore&&) = default;

  /// @brief Get binary semaphore reference
  /// @return Binary semaphore wrapper for this solid semaphore
  BinarySemaphore getSemaphore() const;

  /// @brief Wait for semaphore completion
  /// @param ptr_context Vulkan context for device operations
  void wait(const std::unique_ptr<Context>& ptr_context);
};

/// @brief Swapchain image acquisition semaphore
/// Specialized binary semaphore used to synchronize the completion of swapchain
/// image acquisition. Signals when the next image is ready for rendering.
class AcquireImageSemaphore : public BinarySemaphore {
 public:
  /// @brief Construct acquire image semaphore
  /// @param ptr_context Vulkan context for device operations
  AcquireImageSemaphore(const std::unique_ptr<Context>& ptr_context);
  ~AcquireImageSemaphore();
};

/// @brief Render completion semaphore
/// Specialized binary semaphore used to synchronize the completion of GPU rendering.
/// Signals when rendering operations are finished and ready for presentation.
class RenderSemaphore : public BinarySemaphore {
 public:
  /// @brief Construct render semaphore
  /// @param ptr_context Vulkan context for device operations
  RenderSemaphore(const std::unique_ptr<Context>& ptr_context);
  ~RenderSemaphore();
};

}  // namespace pandora::core::gpu
