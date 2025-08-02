/*
 * descriptor.hpp - GPU descriptor resources for Pandolabo Vulkan C++ wrapper
 *
 * This header contains descriptor-related classes for managing shader resource bindings.
 * These classes handle buffer and image descriptors, descriptor sets, and layouts.
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
class ImageView;
class Sampler;
class DescriptionUnit;
}  // namespace pandora::core::gpu

namespace pandora::core::gpu {

/// @brief Buffer descriptor wrapper for uniform and storage buffers
/// Manages descriptor set write operations for buffers, encapsulating
/// the Vulkan descriptor buffer info and write descriptor set structures.
/// Used to bind uniform buffers, storage buffers, and other buffer resources to shaders.
class BufferDescription {
 private:
  std::shared_ptr<vk::WriteDescriptorSet> m_ptrWriteDescriptorSet{};  ///< Vulkan write descriptor set
  std::shared_ptr<vk::DescriptorBufferInfo> m_ptrBufferInfo{};        ///< Vulkan buffer descriptor info

 public:
  /// @brief Construct buffer descriptor
  /// @param descriptor_info Binding information (binding index, type, etc.)
  /// @param buffer Buffer object to bind to the descriptor
  BufferDescription(const DescriptorInfo& descriptor_info, const Buffer& buffer);
  ~BufferDescription();

  /// @brief Get write descriptor set for binding
  /// @return Reference to Vulkan write descriptor set
  const auto& getWriteDescriptorSet() const {
    return *m_ptrWriteDescriptorSet;
  }

  /// @brief Get buffer descriptor info
  /// @return Reference to Vulkan buffer descriptor info
  const auto& getBufferInfo() const {
    return *m_ptrBufferInfo;
  }
};

/// @brief Image descriptor wrapper for texture and image resources
/// Manages descriptor set write operations for images, encapsulating
/// the Vulkan descriptor image info and write descriptor set structures.
/// Supports binding of images, storage images, samplers, and combined image samplers to shaders.
/// Multiple constructors support different descriptor types and use cases.
class ImageDescription {
 private:
  std::shared_ptr<vk::WriteDescriptorSet> m_ptrWriteDescriptorSet{};  ///< Vulkan write descriptor set
  std::shared_ptr<vk::DescriptorImageInfo> m_ptrImageInfo{};          ///< Vulkan image descriptor info

 public:
  /// @brief Construct image descriptor for storage images or regular images
  /// @param descriptor_info Binding information (binding index, type, etc.)
  /// @param image_view Image view for the texture or storage image
  /// @param dst_image_layout Target image layout for the descriptor binding
  ImageDescription(const DescriptorInfo& descriptor_info,
                   const ImageView& image_view,
                   const ImageLayout dst_image_layout);

  /// @brief Construct sampler-only descriptor
  /// @param descriptor_info Binding information (binding index, type, etc.)
  /// @param dst_image_layout Target image layout (usually undefined for sampler-only)
  /// @param sampler Sampler object for texture filtering and addressing
  ImageDescription(const DescriptorInfo& descriptor_info, const ImageLayout dst_image_layout, const Sampler& sampler);

  /// @brief Construct combined image sampler descriptor
  /// @param descriptor_info Binding information (binding index, type, etc.)
  /// @param image_view Image view for the texture
  /// @param dst_image_layout Target image layout for the texture
  /// @param sampler Sampler for texture filtering and addressing
  ImageDescription(const DescriptorInfo& descriptor_info,
                   const ImageView& image_view,
                   const ImageLayout dst_image_layout,
                   const Sampler& sampler);
  ~ImageDescription();

  /// @brief Get write descriptor set for binding
  /// @return Reference to Vulkan write descriptor set
  const auto& getWriteDescriptorSet() const {
    return *m_ptrWriteDescriptorSet;
  }

  /// @brief Get image descriptor info
  /// @return Reference to Vulkan image descriptor info
  const auto& getImageInfo() const {
    return *m_ptrImageInfo;
  }
};

/// @brief Descriptor set layout wrapper for managing shader resource bindings
/// Encapsulates Vulkan descriptor set layout creation and management.
/// Defines the types and bindings of resources (buffers, images, samplers)
/// that can be bound to a shader pipeline. Acts as a template for descriptor sets.
///
/// This class maintains a one-to-many relationship with DescriptorSet objects,
/// as multiple descriptor sets can be created from the same layout template.
/// The layout defines the structure while descriptor sets contain actual resource bindings.
class DescriptorSetLayout {
 private:
  vk::UniqueDescriptorSetLayout m_ptrDescriptorSetLayout{};     ///< Vulkan descriptor set layout
  std::vector<vk::DescriptorPoolSize> m_descriptorPoolSizes{};  ///< Pool sizes for descriptor allocation

 public:
  /// @brief Construct descriptor set layout from description unit
  /// @param ptr_context Vulkan context for device operations
  /// @param description_unit Description unit containing binding information
  DescriptorSetLayout(const std::unique_ptr<Context>& ptr_context, const DescriptionUnit& description_unit);

  // Rule of Five
  ~DescriptorSetLayout();
  DescriptorSetLayout(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
  DescriptorSetLayout(DescriptorSetLayout&&) = default;
  DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

  /// @brief Get Vulkan descriptor set layout handle
  /// @return Reference to Vulkan descriptor set layout
  const auto& getDescriptorSetLayout() const {
    return m_ptrDescriptorSetLayout.get();
  }

  /// @brief Get descriptor pool creation info for this layout
  /// @return Vulkan descriptor pool create info with appropriate pool sizes
  vk::DescriptorPoolCreateInfo getDescriptorPoolInfo() const;
};

/// @brief Descriptor set wrapper for binding resources to shaders
/// Manages allocation and updating of descriptor sets from descriptor pools.
/// Provides interface for binding buffers, images, and samplers to shader stages.
/// Represents actual resource bindings that follow the structure defined by DescriptorSetLayout.
///
/// This class handles GPU memory allocation for shader binding data and provides
/// methods to update resource bindings efficiently without recreating the entire set.
class DescriptorSet {
 private:
  vk::UniqueDescriptorPool m_ptrDescriptorPool{};  ///< Vulkan descriptor pool
  vk::UniqueDescriptorSet m_ptrDescriptorSet{};    ///< Vulkan descriptor set

 public:
  /// @brief Construct descriptor set from layout
  /// @param ptr_context Vulkan context for device operations
  /// @param description_set_layout Layout template defining resource structure
  DescriptorSet(const std::unique_ptr<Context>& ptr_context, const DescriptorSetLayout& description_set_layout);

  // Rule of Five
  ~DescriptorSet();
  DescriptorSet(const DescriptorSet&) = delete;
  DescriptorSet& operator=(const DescriptorSet&) = delete;
  DescriptorSet(DescriptorSet&&) = default;
  DescriptorSet& operator=(DescriptorSet&&) = default;

  /// @brief Get Vulkan descriptor set handle
  /// @return Reference to Vulkan descriptor set
  const auto& getDescriptorSet() const {
    return m_ptrDescriptorSet.get();
  }

  /// @brief Update descriptor set with resource bindings
  /// Uploads binding resource information to GPU memory. This allocates
  /// GPU memory for shader binding data and registers the resource descriptions.
  /// The bindings must follow the structure defined by the DescriptorSetLayout.
  /// @param ptr_context Vulkan context for device operations
  /// @param buffer_descriptions List of buffer descriptors to bind
  /// @param image_descriptions List of image descriptors to bind
  void updateDescriptorSet(const std::unique_ptr<Context>& ptr_context,
                           const std::vector<BufferDescription>& buffer_descriptions,
                           const std::vector<ImageDescription>& image_descriptions);

  /// @brief Free GPU memory for descriptor set to prepare for next update
  /// Useful for changing image resource data without changing shader modules
  /// or description units. Provides efficient memory management by allowing
  /// reuse of descriptor sets with different resource bindings.
  /// @param ptr_context Vulkan context for device operations
  void freeDescriptorSet(const std::unique_ptr<Context>& ptr_context);
};

}  // namespace pandora::core::gpu
