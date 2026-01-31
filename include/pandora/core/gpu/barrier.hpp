#pragma once

#include <concepts>
#include <functional>
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

/// @brief Vulkan memory barrier wrapper class
/// This class manages synchronization for memory access operations.
/// It provides control over:
/// - Memory access ordering (read-after-write, write-after-read dependencies)
/// - Queue family ownership transfers for multi-queue operations
/// - Memory visibility between different pipeline stages
class MemoryBarrier {
 private:
  vk::MemoryBarrier2 m_memoryBarrier{};

 public:
  /// @brief Construct memory barrier
  /// @param src_access_flags Memory access types that must complete before this
  /// barrier
  /// @param dst_access_flags Memory access types that wait for this barrier
  /// @param src_stages Pipeline stages that must complete before this barrier
  /// @param dst_stages Pipeline stages that wait for this barrier
  MemoryBarrier(const std::vector<AccessFlag>& src_access_flags,
                const std::vector<AccessFlag>& dst_access_flags,
                const std::vector<PipelineStage>& src_stages,
                const std::vector<PipelineStage>& dst_stages);

  ~MemoryBarrier();

  /// @brief Get const reference to underlying Vulkan barrier
  /// @return Const reference to vk::MemoryBarrier
  const auto& getBarrier() const {
    return m_memoryBarrier;
  }
};

class MemoryBarrierBuilder {
 private:
  std::vector<AccessFlag> m_srcAccessFlags{};
  std::vector<AccessFlag> m_dstAccessFlags{};
  std::vector<PipelineStage> m_srcStages{};
  std::vector<PipelineStage> m_dstStages{};

  // Private constructor - use create() factory method instead
  MemoryBarrierBuilder() = default;

 public:
  /// @brief Static factory method to create a new MemoryBarrierBuilder
  /// @return A new MemoryBarrierBuilder instance
  static MemoryBarrierBuilder create() {
    return MemoryBarrierBuilder{};
  }

  /// @brief Set source access flags
  /// @param flags Memory access types that must complete before this barrier
  /// @return Reference to this builder for method chaining
  MemoryBarrierBuilder& setSrcAccessFlags(
      const std::vector<AccessFlag>& flags) {
    m_srcAccessFlags = flags;
    return *this;
  }

  /// @brief Set destination access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  MemoryBarrierBuilder& setDstAccessFlags(
      const std::vector<AccessFlag>& flags) {
    m_dstAccessFlags = flags;
    return *this;
  }

  /// @brief Set source pipeline stages
  /// @param stages Pipeline stages that must complete before this barrier
  /// @return Reference to this builder for method chaining
  MemoryBarrierBuilder& setSrcStages(const std::vector<PipelineStage>& stages) {
    m_srcStages = stages;
    return *this;
  }

  /// @brief Set destination pipeline stages
  /// @param stages Pipeline stages that wait for this barrier
  /// @return Reference to this builder for method chaining
  MemoryBarrierBuilder& setDstStages(const std::vector<PipelineStage>& stages) {
    m_dstStages = stages;
    return *this;
  }

  /// @brief Build and return the final MemoryBarrier instance
  /// @return Constructed MemoryBarrier object
  MemoryBarrier build() const {
    return MemoryBarrier(
        m_srcAccessFlags, m_dstAccessFlags, m_srcStages, m_dstStages);
  }
};

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
  vk::BufferMemoryBarrier2 m_bufferMemoryBarrier{};

 public:
  /// @brief Construct buffer memory barrier
  /// @param buffer The buffer to synchronize
  /// @param src_access_flags Memory access types that must complete before
  /// this barrier
  /// @param dst_access_flags Memory access types that wait for this barrier
  /// @param src_stages Pipeline stages that must complete before this barrier
  /// @param dst_stages Pipeline stages that wait for this barrier
  /// @param src_queue_family Queue family index that currently owns the buffer
  /// @param dst_queue_family Queue family index that will receive buffer
  /// ownership
  BufferBarrier(const Buffer& buffer,
                const std::vector<AccessFlag>& src_access_flags,
                const std::vector<AccessFlag>& dst_access_flags,
                const std::vector<PipelineStage>& src_stages,
                const std::vector<PipelineStage>& dst_stages,
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
  std::optional<std::reference_wrapper<const Buffer>> m_buffer{};
  std::vector<AccessFlag> m_srcAccessFlags{};
  std::vector<AccessFlag> m_dstAccessFlags{};
  std::vector<PipelineStage> m_srcStages{};
  std::vector<PipelineStage> m_dstStages{};
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
    m_buffer = buffer;
    return *this;
  }

  /// @brief Set source access flags
  /// @param flags Memory access types that must complete before this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setSrcAccessFlags(
      const std::vector<AccessFlag>& flags) {
    m_srcAccessFlags = flags;
    return *this;
  }

  /// @brief Set destination access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setDstAccessFlags(
      const std::vector<AccessFlag>& flags) {
    m_dstAccessFlags = flags;
    return *this;
  }

  /// @brief Set source pipeline stages
  /// @param stages Pipeline stages that must complete before this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setSrcStages(const std::vector<PipelineStage>& stages) {
    m_srcStages = stages;
    return *this;
  }

  /// @brief Set destination pipeline stages
  /// @param stages Pipeline stages that wait for this barrier
  /// @return Reference to this builder for method chaining
  BufferBarrierBuilder& setDstStages(const std::vector<PipelineStage>& stages) {
    m_dstStages = stages;
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
    if (!m_buffer.has_value()) {
      throw std::runtime_error(
          "Buffer must be set before building BufferBarrier");
    }

    return BufferBarrier(m_buffer->get(),
                         m_srcAccessFlags,
                         m_dstAccessFlags,
                         m_srcStages,
                         m_dstStages,
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
  vk::ImageMemoryBarrier2 m_imageMemoryBarrier{};

 public:
  ImageBarrier(const Image& image,
               const std::vector<AccessFlag>& src_access_flags,
               const std::vector<AccessFlag>& dst_access_flags,
               const std::vector<PipelineStage>& src_stages,
               const std::vector<PipelineStage>& dst_stages,
               ImageLayout old_layout,
               ImageLayout new_layout,
               const ImageViewInfo& image_view_info,
               uint32_t src_queue_family = 0u,
               uint32_t dst_queue_family = 0u);
  ImageBarrier(const std::unique_ptr<Context>& ptr_context,
               const std::vector<AccessFlag>& src_access_flags,
               const std::vector<AccessFlag>& dst_access_flags,
               const std::vector<PipelineStage>& src_stages,
               const std::vector<PipelineStage>& dst_stages,
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
  std::optional<std::reference_wrapper<const Image>> m_image{};
  std::vector<AccessFlag> m_srcAccessFlags{};
  std::vector<AccessFlag> m_dstAccessFlags{};
  std::vector<PipelineStage> m_srcStages{};
  std::vector<PipelineStage> m_dstStages{};
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
    m_image = image;
    return *this;
  }

  /// @brief Set priority access flags
  /// @param flags Memory access types that must complete before this barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setSrcAccessFlags(const std::vector<AccessFlag>& flags) {
    m_srcAccessFlags = flags;
    return *this;
  }

  /// @brief Set wait access flags
  /// @param flags Memory access types that wait for this barrier
  /// @return Reference to this builder for method chaining
  ImageBarrierBuilder& setDstAccessFlags(const std::vector<AccessFlag>& flags) {
    m_dstAccessFlags = flags;
    return *this;
  }

  /// @brief Set source pipeline stages
  /// @param stages Pipeline stages that must complete before this barrier
  ImageBarrierBuilder& setSrcStages(const std::vector<PipelineStage>& stages) {
    m_srcStages = stages;
    return *this;
  }

  /// @brief Set destination pipeline stages
  /// @param stages Pipeline stages that wait for this barrier
  ImageBarrierBuilder& setDstStages(const std::vector<PipelineStage>& stages) {
    m_dstStages = stages;
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
    if (m_image.has_value() && m_imageViewInfo) {
      // Build with image and image view info
      return ImageBarrier(m_image->get(),
                          m_srcAccessFlags,
                          m_dstAccessFlags,
                          m_srcStages,
                          m_dstStages,
                          m_oldLayout,
                          m_newLayout,
                          *m_imageViewInfo,
                          m_srcQueueFamily,
                          m_dstQueueFamily);
    } else if (ptr_context) {
      // Build with context
      return ImageBarrier(ptr_context,
                          m_srcAccessFlags,
                          m_dstAccessFlags,
                          m_srcStages,
                          m_dstStages,
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
