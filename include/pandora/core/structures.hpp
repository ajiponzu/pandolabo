/*
 * structures.hpp - Core structure definitions for Pandolabo Vulkan C++ wrapper
 *
 * This header contains struct definitions used throughout the GPU module.
 * These structures provide high-level abstractions for Vulkan resource descriptions.
 */

#pragma once

#include <vulkan/vulkan.hpp>

#include "module_connection/gpu_ui.hpp"
#include "types.hpp"

namespace pandora::core {

/// @brief Image sub-resource information for image creation
/// Defines the basic properties needed to create GPU images including
/// dimensions, mip levels, array layers, and format specifications
struct ImageSubInfo {
  gpu_ui::GraphicalSize<uint32_t> graphical_size{};  ///< Width and height dimensions of the image
  uint32_t mip_levels = 1u;                          ///< Number of mipmap levels (1 = no mipmaps)
  uint32_t array_layers = 1u;                        ///< Number of array layers (1 = single image)
  ImageSampleCount samples{};                        ///< Multisampling count for MSAA
  DataFormat format{};                               ///< Pixel format (e.g., RGBA8, Depth24Stencil8)
  ImageDimension dimension{};                        ///< Image dimensionality (1D, 2D, 3D)

  // Fluent interface methods
  ImageSubInfo& setGraphicalSize(const gpu_ui::GraphicalSize<uint32_t>& size) {
    graphical_size = size;
    return *this;
  }
  ImageSubInfo& setSize(uint32_t width, uint32_t height, uint32_t depth = 1) {
    graphical_size.width = width;
    graphical_size.height = height;
    graphical_size.depth = depth;
    return *this;
  }
  ImageSubInfo& setMipLevels(uint32_t levels) {
    mip_levels = levels;
    return *this;
  }
  ImageSubInfo& setArrayLayers(uint32_t layers) {
    array_layers = layers;
    return *this;
  }
  ImageSubInfo& setSamples(ImageSampleCount sample_count) {
    samples = sample_count;
    return *this;
  }
  ImageSubInfo& setFormat(DataFormat fmt) {
    format = fmt;
    return *this;
  }
  ImageSubInfo& setDimension(ImageDimension dim) {
    dimension = dim;
    return *this;
  }
};

/// @brief Descriptor binding information for shader resources
/// Describes how shader resources (uniforms, textures, buffers) are bound
/// to specific binding points in descriptor sets
struct DescriptorInfo {
  vk::ShaderStageFlags stage_flags{};  ///< Which shader stages can access this resource
  uint32_t binding = 0u;               ///< Binding index in the descriptor set layout
  vk::DescriptorType type{};           ///< Type of descriptor (uniform buffer, sampler, etc.)
  uint32_t size = 0u;                  ///< Number of descriptors in this binding (for arrays)

  // Fluent interface methods
  DescriptorInfo& setStageFlags(vk::ShaderStageFlags flags) {
    stage_flags = flags;
    return *this;
  }
  DescriptorInfo& setBinding(uint32_t bind) {
    binding = bind;
    return *this;
  }
  DescriptorInfo& setType(vk::DescriptorType descriptor_type) {
    type = descriptor_type;
    return *this;
  }
  DescriptorInfo& setSize(uint32_t descriptor_size) {
    size = descriptor_size;
    return *this;
  }
};

/// @brief Push constant range information for shader uniform data
/// Defines memory ranges for push constants, which provide fast uniform data
/// transfer to shaders without requiring descriptor sets
struct PushConstantRange {
  vk::ShaderStageFlags stage_flags{};  ///< Which shader stages can access these push constants
  uint32_t offset = 0u;                ///< Byte offset within the push constant block
  size_t size = 0u;                    ///< Size in bytes of this push constant range

  // Fluent interface methods
  PushConstantRange& setStageFlags(vk::ShaderStageFlags flags) {
    stage_flags = flags;
    return *this;
  }
  PushConstantRange& setOffset(uint32_t byte_offset) {
    offset = byte_offset;
    return *this;
  }
  PushConstantRange& setSize(size_t byte_size) {
    size = byte_size;
    return *this;
  }
};

/// @brief Image view information for image resource access
/// @details This struct is not only for image view, but also for image barrier, etc.
/// It's useful to manage image's miplevel and array layers.
struct ImageViewInfo {
  uint32_t base_mip_level = 0u;    ///< First mipmap level accessible through this view
  uint32_t mip_levels = 0u;        ///< Number of mipmap levels accessible (VK_REMAINING_MIP_LEVELS for all)
  uint32_t base_array_layer = 0u;  ///< First array layer accessible through this view
  uint32_t array_layers = 0u;      ///< Number of array layers accessible (VK_REMAINING_ARRAY_LAYERS for all)
  ImageAspect aspect;              ///< Which aspects of the image to access (color, depth, stencil)

  // Fluent interface methods
  ImageViewInfo& setBaseMipLevel(uint32_t level) {
    base_mip_level = level;
    return *this;
  }
  ImageViewInfo& setMipLevels(uint32_t levels) {
    mip_levels = levels;
    return *this;
  }
  ImageViewInfo& setBaseArrayLayer(uint32_t layer) {
    base_array_layer = layer;
    return *this;
  }
  ImageViewInfo& setArrayLayers(uint32_t layers) {
    array_layers = layers;
    return *this;
  }
  ImageViewInfo& setAspect(ImageAspect image_aspect) {
    aspect = image_aspect;
    return *this;
  }
  ImageViewInfo& setMipRange(uint32_t base_level, uint32_t level_count) {
    base_mip_level = base_level;
    mip_levels = level_count;
    return *this;
  }
  ImageViewInfo& setArrayRange(uint32_t base_layer, uint32_t layer_count) {
    base_array_layer = base_layer;
    array_layers = layer_count;
    return *this;
  }
};

/// @brief Sampler configuration information for texture filtering
/// Configures how textures are sampled, including filtering, addressing,
/// anisotropy, and comparison operations for texture lookups
struct SamplerInfo {
  SamplerFilter mag_filter;               ///< Magnification filter when texel < pixel
  SamplerFilter min_filter;               ///< Minification filter when texel > pixel
  SamplerMipmapMode mipmap_mode;          ///< How to filter between mipmap levels
  SamplerAddressMode address_mode_u;      ///< Addressing mode for U coordinate (outside [0,1])
  SamplerAddressMode address_mode_v;      ///< Addressing mode for V coordinate (outside [0,1])
  SamplerAddressMode address_mode_w;      ///< Addressing mode for W coordinate (outside [0,1])
  float_t mip_lod_bias = 0.0f;            ///< Bias added to LOD calculations
  bool anisotropy_enable = false;         ///< Whether to enable anisotropic filtering
  float_t max_anisotropy = 0.0f;          ///< Maximum anisotropy ratio (1.0-16.0 typically)
  bool compare_enable = false;            ///< Whether to enable comparison mode (for shadow mapping)
  SamplerCompareOp compare_op;            ///< Comparison operation when compare_enable is true
  float_t min_lod = 0.0f;                 ///< Minimum level-of-detail parameter
  float_t max_lod = 0.0f;                 ///< Maximum level-of-detail parameter
  SamplerBorderColor border_color;        ///< Border color when using clamp-to-border addressing
  bool unnormalized_coordinates = false;  ///< Whether to use unnormalized coordinates [0,size] vs [0,1]

  // Fluent interface methods
  SamplerInfo& setMagFilter(SamplerFilter filter) {
    mag_filter = filter;
    return *this;
  }
  SamplerInfo& setMinFilter(SamplerFilter filter) {
    min_filter = filter;
    return *this;
  }
  SamplerInfo& setFilters(SamplerFilter mag, SamplerFilter min) {
    mag_filter = mag;
    min_filter = min;
    return *this;
  }
  SamplerInfo& setMipmapMode(SamplerMipmapMode mode) {
    mipmap_mode = mode;
    return *this;
  }
  SamplerInfo& setAddressModeU(SamplerAddressMode mode) {
    address_mode_u = mode;
    return *this;
  }
  SamplerInfo& setAddressModeV(SamplerAddressMode mode) {
    address_mode_v = mode;
    return *this;
  }
  SamplerInfo& setAddressModeW(SamplerAddressMode mode) {
    address_mode_w = mode;
    return *this;
  }
  SamplerInfo& setAddressMode(SamplerAddressMode mode) {
    address_mode_u = address_mode_v = address_mode_w = mode;
    return *this;
  }
  SamplerInfo& setAddressModes(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) {
    address_mode_u = u;
    address_mode_v = v;
    address_mode_w = w;
    return *this;
  }
  SamplerInfo& setMipLodBias(float_t bias) {
    mip_lod_bias = bias;
    return *this;
  }
  SamplerInfo& setAnisotropy(bool enable, float_t max_aniso = 16.0f) {
    anisotropy_enable = enable;
    max_anisotropy = max_aniso;
    return *this;
  }
  SamplerInfo& setCompare(bool enable, SamplerCompareOp op = SamplerCompareOp::Never) {
    compare_enable = enable;
    compare_op = op;
    return *this;
  }
  SamplerInfo& setLodRange(float_t min, float_t max) {
    min_lod = min;
    max_lod = max;
    return *this;
  }
  SamplerInfo& setBorderColor(SamplerBorderColor color) {
    border_color = color;
    return *this;
  }
  SamplerInfo& setUnnormalizedCoordinates(bool unnormalized) {
    unnormalized_coordinates = unnormalized;
    return *this;
  }
};

}  // namespace pandora::core
