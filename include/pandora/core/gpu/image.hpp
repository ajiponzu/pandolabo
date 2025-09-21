/*
 * image.hpp - GPU image resources for Pandolabo Vulkan C++ wrapper
 *
 * This header contains image-related classes: Image, ImageView, and Sampler.
 * These classes manage GPU image resources, views, and sampling operations.
 */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../module_connection/gpu_ui.hpp"
#include "../structures.hpp"
#include "../types.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Context;
}

namespace pandora::core::gpu {

/// @brief GPU image resource wrapper class
/// @details This class manages Vulkan image resources including textures,
/// render targets, etc.
class Image {
 protected:
  vk::UniqueDeviceMemory m_ptrMemory;
  vk::UniqueImage m_ptrImage;

  uint32_t m_mipLevels = 0u;
  uint32_t m_arrayLayers = 0u;
  vk::Format m_format{};
  ImageDimension m_dimension{};
  gpu_ui::GraphicalSize<uint32_t> m_graphicalSize{};

 public:
  Image() = default;

  /// @brief Construct image with specified properties
  /// @param ptr_context GPU context pointer
  /// @param memory_usage Memory usage pattern
  /// @param transfer_type Transfer operation type
  /// @param image_usages Image usage types
  /// @param image_sub_info Image sub-resource information
  Image(const std::unique_ptr<Context>& ptr_context,
        MemoryUsage memory_usage,
        TransferType transfer_type,
        const std::vector<ImageUsage>& image_usages,
        const ImageSubInfo& image_sub_info);
  ~Image();

  /// @brief Move constructor
  /// @param other Image to move from
  Image(Image&& other) noexcept {
    m_ptrMemory = std::move(other.m_ptrMemory);
    m_ptrImage = std::move(other.m_ptrImage);
    m_mipLevels = other.m_mipLevels;
    m_arrayLayers = other.m_arrayLayers;
    m_format = other.m_format;
    m_dimension = other.m_dimension;
    m_graphicalSize = std::move(other.m_graphicalSize);
  }

  /// @brief Move assignment operator
  /// @param other Image to move from
  /// @return Reference to this image
  Image& operator=(Image&& other) noexcept {
    m_ptrMemory = std::move(other.m_ptrMemory);
    m_ptrImage = std::move(other.m_ptrImage);
    m_mipLevels = other.m_mipLevels;
    m_arrayLayers = other.m_arrayLayers;
    m_format = other.m_format;
    m_dimension = other.m_dimension;
    m_graphicalSize = std::move(other.m_graphicalSize);
    return *this;
  }

  /// @brief Get image unique pointer
  /// @return Reference to image unique pointer
  const auto& getPtrImage() const {
    return m_ptrImage;
  }

  /// @brief Get image handle
  /// @return Vulkan image handle
  const auto& getImage() const {
    return m_ptrImage.get();
  }

  /// @brief Get mip levels count
  /// @return Number of mip levels
  auto getMipLevels() const {
    return m_mipLevels;
  }

  /// @brief Get array layers count
  /// @return Number of array layers
  auto getArrayLayers() const {
    return m_arrayLayers;
  }

  /// @brief Get image format
  /// @return Vulkan format
  auto getFormat() const {
    return m_format;
  }

  /// @brief Get image dimension
  /// @return Image dimension type
  auto getDimension() const {
    return m_dimension;
  }

  /// @brief Get image size
  /// @return Graphical size information
  const auto& getGraphicalSize() const {
    return m_graphicalSize;
  }
};

/// @brief Vulkan image view wrapper class
/// @details Image view is separated from image resource because image resource
/// is only memory data and the way to handle image is decided by image view.
class ImageView {
 private:
  vk::UniqueImageView m_ptrImageView;
  std::unique_ptr<ImageViewInfo> m_ptrImageViewInfo;

 public:
  /// @brief Construct image view for the given image
  /// @param ptr_context GPU context pointer
  /// @param image Source image
  /// @param image_view_info Image view configuration
  ImageView(const std::unique_ptr<Context>& ptr_context,
            const Image& image,
            const ImageViewInfo& image_view_info);
  ~ImageView();

  /// @brief Get image view handle
  /// @return Vulkan image view handle
  const auto& getImageView() const {
    return m_ptrImageView.get();
  }

  /// @brief Get image view information
  /// @return Image view configuration
  const auto& getImageViewInfo() const {
    return *m_ptrImageViewInfo;
  }
};

/// @brief Vulkan sampler wrapper class
/// @details Sampler is used for texture filtering, mipmapping, and texture
/// interface on shader.
class Sampler {
 private:
  vk::UniqueSampler m_ptrSampler;

 public:
  /// @brief Construct sampler with specified configuration
  /// @param ptr_context GPU context pointer
  /// @param sampler_info Sampler configuration
  Sampler(const std::unique_ptr<Context>& ptr_context,
          const SamplerInfo& sampler_info);
  ~Sampler();

  /// @brief Get sampler handle
  /// @return Vulkan sampler handle
  const auto& getSampler() const {
    return m_ptrSampler.get();
  }
};

}  // namespace pandora::core::gpu
