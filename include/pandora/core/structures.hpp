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
  uint32_t mip_levels = 1U;                          ///< Number of mipmap levels (1 = no mipmaps)
  uint32_t array_layers = 1U;                        ///< Number of array layers (1 = single image)
  ImageSampleCount samples{};                        ///< Multisampling count for MSAA
  DataFormat format{};                               ///< Pixel format (e.g., RGBA8, Depth24Stencil8)
  ImageDimension dimension{};                        ///< Image dimensionality (1D, 2D, 3D)
};

/// @brief Descriptor binding information for shader resources
/// Describes how shader resources (uniforms, textures, buffers) are bound
/// to specific binding points in descriptor sets
struct DescriptorInfo {
  vk::ShaderStageFlags stage_flags{};  ///< Which shader stages can access this resource
  uint32_t binding = 0U;               ///< Binding index in the descriptor set layout
  vk::DescriptorType type{};           ///< Type of descriptor (uniform buffer, sampler, etc.)
  uint32_t size = 0U;                  ///< Number of descriptors in this binding (for arrays)
};

/// @brief Push constant range information for shader uniform data
/// Defines memory ranges for push constants, which provide fast uniform data
/// transfer to shaders without requiring descriptor sets
struct PushConstantRange {
  vk::ShaderStageFlags stage_flags{};  ///< Which shader stages can access these push constants
  uint32_t offset = 0U;                ///< Byte offset within the push constant block
  size_t size = 0U;                    ///< Size in bytes of this push constant range
};

/// @brief Image view information for image resource access
/// @details This struct is not only for image view, but also for image barrier, etc.
/// It's useful to manage image's miplevel and array layers.
struct ImageViewInfo {
  uint32_t base_mip_level;    ///< First mipmap level accessible through this view
  uint32_t mip_levels;        ///< Number of mipmap levels accessible (VK_REMAINING_MIP_LEVELS for all)
  uint32_t base_array_layer;  ///< First array layer accessible through this view
  uint32_t array_layers;      ///< Number of array layers accessible (VK_REMAINING_ARRAY_LAYERS for all)
  ImageAspect aspect;         ///< Which aspects of the image to access (color, depth, stencil)
};

/// @brief Sampler configuration information for texture filtering
/// Configures how textures are sampled, including filtering, addressing,
/// anisotropy, and comparison operations for texture lookups
struct SamplerInfo {
  SamplerFilter mag_filter;           ///< Magnification filter when texel < pixel
  SamplerFilter min_filter;           ///< Minification filter when texel > pixel
  SamplerMipmapMode mipmap_mode;      ///< How to filter between mipmap levels
  SamplerAddressMode address_mode_u;  ///< Addressing mode for U coordinate (outside [0,1])
  SamplerAddressMode address_mode_v;  ///< Addressing mode for V coordinate (outside [0,1])
  SamplerAddressMode address_mode_w;  ///< Addressing mode for W coordinate (outside [0,1])
  float_t mip_lod_bias;               ///< Bias added to LOD calculations
  bool anisotropy_enable;             ///< Whether to enable anisotropic filtering
  float_t max_anisotropy;             ///< Maximum anisotropy ratio (1.0-16.0 typically)
  bool compare_enable;                ///< Whether to enable comparison mode (for shadow mapping)
  SamplerCompareOp compare_op;        ///< Comparison operation when compare_enable is true
  float_t min_lod;                    ///< Minimum level-of-detail parameter
  float_t max_lod;                    ///< Maximum level-of-detail parameter
  SamplerBorderColor border_color;    ///< Border color when using clamp-to-border addressing
  bool unnormalized_coordinates;      ///< Whether to use unnormalized coordinates [0,size] vs [0,1]
};

}  // namespace pandora::core
