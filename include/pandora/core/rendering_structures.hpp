/*
 * rendering_structures.hpp - Rendering-related structure definitions for Pandolabo core module
 *
 * This header contains structure definitions used for rendering operations,
 * attachments, subpasses, and pipeline configurations.
 */

#pragma once

#include <array>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "rendering_types.hpp"
#include "types.hpp"

namespace pandora::core {

/// @brief Attachment description for render pass configuration
/// Defines how an attachment (color/depth/stencil buffer) should be handled
/// during rendering operations, including load/store operations and layouts
struct AttachmentDescription {
  DataFormat format{};                   ///< Pixel format of the attachment
  ImageSampleCount samples{};            ///< Number of samples for MSAA
  AttachmentLoadOp load_op{};            ///< How to handle existing attachment data at load
  AttachmentStoreOp store_op{};          ///< How to handle attachment data after rendering
  AttachmentLoadOp stencil_load_op{};    ///< Stencil component load operation
  AttachmentStoreOp stencil_store_op{};  ///< Stencil component store operation
  ImageLayout initial_layout{};          ///< Layout when subpass begins
  ImageLayout final_layout{};            ///< Layout when subpass ends
  ImageLayout stencil_initial_layout{};  ///< Initial stencil layout
  ImageLayout stencil_final_layout{};    ///< Final stencil layout
};

/// @brief Reference to an attachment within a subpass
/// Links a subpass to a specific attachment with the required layout
struct AttachmentReference {
  uint32_t index = 0U;   ///< Index of the attachment in the attachment list
  ImageLayout layout{};  ///< Layout the attachment should be in during this subpass
};

/// @brief Dependency between subpasses for synchronization
/// Defines memory and execution dependencies between rendering operations
/// to ensure proper ordering and memory coherency
struct SubpassEdge {
  uint32_t src_index = VK_SUBPASS_EXTERNAL;  ///< Source subpass index (VK_SUBPASS_EXTERNAL for external)
  uint32_t dst_index = 0U;                   ///< Destination subpass index
  std::vector<PipelineStage> src_stages{};   ///< Pipeline stages that must complete in source
  std::vector<PipelineStage> dst_stages{};   ///< Pipeline stages that wait in destination
  std::vector<AccessFlag> src_access{};      ///< Memory access that must complete in source
  std::vector<AccessFlag> dst_access{};      ///< Memory access that waits in destination
  DependencyFlag dependency_flag{};          ///< Additional dependency flags
};

/// @brief Stencil test operation configuration
/// Defines what operations to perform based on stencil test results
struct StencilOpState {
  StencilOp fail_op = StencilOp::Keep;        ///< Operation when stencil test fails
  StencilOp pass_op = StencilOp::Keep;        ///< Operation when stencil test passes
  StencilOp depth_fail_op = StencilOp::Keep;  ///< Operation when depth test fails but stencil passes
  CompareOp compare_op = CompareOp::Always;   ///< Comparison function for stencil test
  uint32_t compare_mask = 0U;                 ///< Mask applied to stencil value before comparison
  uint32_t write_mask = 0U;                   ///< Mask applied when writing to stencil buffer
  uint32_t reference = 0U;                    ///< Reference value for stencil comparison
};

/// @brief Color blending configuration for a single attachment
/// Defines how new fragment colors are combined with existing framebuffer colors
struct ColorBlendAttachment {
  bool is_enabled = false;                       ///< Whether color blending is enabled
  BlendFactor src_color = BlendFactor::One;      ///< Source color blend factor
  BlendFactor dst_color = BlendFactor::Zero;     ///< Destination color blend factor
  BlendOp color_op = BlendOp::Add;               ///< Color blending operation
  BlendFactor src_alpha = BlendFactor::One;      ///< Source alpha blend factor
  BlendFactor dst_alpha = BlendFactor::Zero;     ///< Destination alpha blend factor
  BlendOp alpha_op = BlendOp::Add;               ///< Alpha blending operation
  std::vector<ColorComponent> color_components;  ///< Which color components to write
};

/// @brief Compute shader work group dimensions
/// Specifies the local work group size for compute shader dispatch
struct ComputeWorkGroupSize {
  uint32_t x;  ///< Work group size in X dimension
  uint32_t y;  ///< Work group size in Y dimension
  uint32_t z;  ///< Work group size in Z dimension
};

/// @brief Clear color value for color attachments
/// RGBA color values used when clearing color attachments
struct ClearColor {
  std::array<float_t, 4> color;  ///< RGBA color components (0.0-1.0 range)
};

/// @brief Clear values for depth and stencil attachments
/// Specifies the depth and stencil values to use when clearing depth/stencil buffers
struct ClearDepthStencil {
  float_t depth;     ///< Depth clear value (typically 0.0 or 1.0)
  uint32_t stencil;  ///< Stencil clear value (typically 0)
};

}  // namespace pandora::core
