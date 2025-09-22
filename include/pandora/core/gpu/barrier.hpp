#pragma once

#include <concepts>
#include <memory>
#include <optional>
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
                const std::vector<AccessFlag>& priority_access_flags,
                const std::vector<AccessFlag>& wait_access_flags,
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
  void setSrcQueueFamilyIndex(uint32_t index) {
    m_bufferMemoryBarrier.setSrcQueueFamilyIndex(index);
  }

  /// @brief Set destination queue family index for ownership transfer
  /// @param index Queue family index that will receive buffer ownership
  void setDstQueueFamilyIndex(uint32_t index) {
    m_bufferMemoryBarrier.setDstQueueFamilyIndex(index);
  }
};

/// @brief Builder class for BufferBarrier
/// Provides a fluent interface for constructing BufferBarrier instances
class BufferBarrierBuilder {
 private:
  const Buffer* m_ptrBuffer = nullptr;
  std::vector<AccessFlag> m_priorityAccessFlags{};
  std::vector<AccessFlag> m_waitAccessFlags{};
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
      const std::vector<AccessFlag>& flags) {
    m_priorityAccessFlags = flags;
    return *this;
  }

  /// @brief Set wait access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setWaitAccessFlags(
      const std::vector<AccessFlag>& flags) {
    m_waitAccessFlags = flags;
    return *this;
  }

  /// @brief Set source queue family index for ownership transfer
  /// @param index Queue family index that currently owns the buffer
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setSrcQueueFamilyIndex(uint32_t index) {
    m_srcQueueFamily = index;
    return *this;
  }

  /// @brief Set destination queue family index for ownership transfer
  /// @param index Queue family index that will receive buffer ownership
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setDstQueueFamilyIndex(uint32_t index) {
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
               const std::vector<AccessFlag>& priority_access_flags,
               const std::vector<AccessFlag>& wait_access_flags,
               ImageLayout old_layout,
               ImageLayout new_layout,
               const ImageViewInfo& image_view_info,
               uint32_t src_queue_family = 0u,
               uint32_t dst_queue_family = 0u);
  ImageBarrier(const std::unique_ptr<Context>& ptr_context,
               const std::vector<AccessFlag>& priority_access_flags,
               const std::vector<AccessFlag>& wait_access_flags,
               ImageLayout old_layout,
               ImageLayout new_layout,
               uint32_t src_queue_family = 0u,
               uint32_t dst_queue_family = 0u);
  ~ImageBarrier();

  const auto& getBarrier() const {
    return m_imageMemoryBarrier;
  }

  void setSrcQueueFamilyIndex(uint32_t index) {
    m_imageMemoryBarrier.setSrcQueueFamilyIndex(index);
  }

  void setDstQueueFamilyIndex(uint32_t index) {
    m_imageMemoryBarrier.setDstQueueFamilyIndex(index);
  }
};

/// @brief Builder class for ImageBarrier
/// Provides a fluent interface for constructing ImageBarrier instances
class ImageBarrierBuilder {
 private:
  const Image* m_ptrImage = nullptr;
  std::vector<AccessFlag> m_priorityAccessFlags{};
  std::vector<AccessFlag> m_waitAccessFlags{};
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
      const std::vector<AccessFlag>& flags) {
    m_priorityAccessFlags = flags;
    return *this;
  }

  /// @brief Set wait access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setWaitAccessFlags(
      const std::vector<AccessFlag>& flags) {
    m_waitAccessFlags = flags;
    return *this;
  }

  /// @brief Set old image layout
  /// @param layout The current layout of the image
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setOldLayout(ImageLayout layout) {
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
  ImageBarrierBuilder& setSrcQueueFamilyIndex(uint32_t index) {
    m_srcQueueFamily = index;
    return *this;
  }

  /// @brief Set destination queue family index for ownership transfer
  /// @param index Queue family index that will receive buffer ownership
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setDstQueueFamilyIndex(uint32_t index) {
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

}  // namespace pandora::core::gpu
