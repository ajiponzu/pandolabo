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

  // Fluent interface methods
  AttachmentDescription& setFormat(DataFormat fmt) {
    format = fmt;
    return *this;
  }
  AttachmentDescription& setSamples(ImageSampleCount sample_count) {
    samples = sample_count;
    return *this;
  }
  AttachmentDescription& setLoadOp(AttachmentLoadOp op) {
    load_op = op;
    return *this;
  }
  AttachmentDescription& setStoreOp(AttachmentStoreOp op) {
    store_op = op;
    return *this;
  }
  AttachmentDescription& setStencilLoadOp(AttachmentLoadOp op) {
    stencil_load_op = op;
    return *this;
  }
  AttachmentDescription& setStencilStoreOp(AttachmentStoreOp op) {
    stencil_store_op = op;
    return *this;
  }
  AttachmentDescription& setInitialLayout(ImageLayout layout) {
    initial_layout = layout;
    return *this;
  }
  AttachmentDescription& setFinalLayout(ImageLayout layout) {
    final_layout = layout;
    return *this;
  }
  AttachmentDescription& setStencilInitialLayout(ImageLayout layout) {
    stencil_initial_layout = layout;
    return *this;
  }
  AttachmentDescription& setStencilFinalLayout(ImageLayout layout) {
    stencil_final_layout = layout;
    return *this;
  }
  AttachmentDescription& setLayouts(ImageLayout initial, ImageLayout final) {
    initial_layout = initial;
    final_layout = final;
    return *this;
  }
  AttachmentDescription& setStencilLayouts(ImageLayout initial, ImageLayout final) {
    stencil_initial_layout = initial;
    stencil_final_layout = final;
    return *this;
  }
};

/// @brief Reference to an attachment within a subpass
/// Links a subpass to a specific attachment with the required layout
struct AttachmentReference {
  uint32_t index = 0U;   ///< Index of the attachment in the attachment list
  ImageLayout layout{};  ///< Layout the attachment should be in during this subpass

  // Fluent interface methods
  AttachmentReference& setIndex(uint32_t attachment_index) {
    index = attachment_index;
    return *this;
  }
  AttachmentReference& setLayout(ImageLayout attachment_layout) {
    layout = attachment_layout;
    return *this;
  }
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

  // Fluent interface methods
  SubpassEdge& setSrcIndex(uint32_t index) {
    src_index = index;
    return *this;
  }
  SubpassEdge& setDstIndex(uint32_t index) {
    dst_index = index;
    return *this;
  }
  SubpassEdge& setSrcStages(const std::vector<PipelineStage>& stages) {
    src_stages = stages;
    return *this;
  }
  SubpassEdge& setDstStages(const std::vector<PipelineStage>& stages) {
    dst_stages = stages;
    return *this;
  }
  SubpassEdge& setSrcAccess(const std::vector<AccessFlag>& access) {
    src_access = access;
    return *this;
  }
  SubpassEdge& setDstAccess(const std::vector<AccessFlag>& access) {
    dst_access = access;
    return *this;
  }
  SubpassEdge& setDependencyFlag(DependencyFlag flag) {
    dependency_flag = flag;
    return *this;
  }
  SubpassEdge& addSrcStage(PipelineStage stage) {
    src_stages.push_back(stage);
    return *this;
  }
  SubpassEdge& addDstStage(PipelineStage stage) {
    dst_stages.push_back(stage);
    return *this;
  }
  SubpassEdge& addSrcAccess(AccessFlag access) {
    src_access.push_back(access);
    return *this;
  }
  SubpassEdge& addDstAccess(AccessFlag access) {
    dst_access.push_back(access);
    return *this;
  }
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

  // Fluent interface methods
  StencilOpState& setFailOp(StencilOp op) {
    fail_op = op;
    return *this;
  }
  StencilOpState& setPassOp(StencilOp op) {
    pass_op = op;
    return *this;
  }
  StencilOpState& setDepthFailOp(StencilOp op) {
    depth_fail_op = op;
    return *this;
  }
  StencilOpState& setCompareOp(CompareOp op) {
    compare_op = op;
    return *this;
  }
  StencilOpState& setCompareMask(uint32_t mask) {
    compare_mask = mask;
    return *this;
  }
  StencilOpState& setWriteMask(uint32_t mask) {
    write_mask = mask;
    return *this;
  }
  StencilOpState& setReference(uint32_t ref) {
    reference = ref;
    return *this;
  }
  StencilOpState& setOps(StencilOp fail, StencilOp pass, StencilOp depth_fail) {
    fail_op = fail;
    pass_op = pass;
    depth_fail_op = depth_fail;
    return *this;
  }
  StencilOpState& setMasks(uint32_t compare, uint32_t write) {
    compare_mask = compare;
    write_mask = write;
    return *this;
  }
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

  // Fluent interface methods
  ColorBlendAttachment& setEnabled(bool enabled) {
    is_enabled = enabled;
    return *this;
  }
  ColorBlendAttachment& setSrcColorFactor(BlendFactor factor) {
    src_color = factor;
    return *this;
  }
  ColorBlendAttachment& setDstColorFactor(BlendFactor factor) {
    dst_color = factor;
    return *this;
  }
  ColorBlendAttachment& setColorOp(BlendOp op) {
    color_op = op;
    return *this;
  }
  ColorBlendAttachment& setSrcAlphaFactor(BlendFactor factor) {
    src_alpha = factor;
    return *this;
  }
  ColorBlendAttachment& setDstAlphaFactor(BlendFactor factor) {
    dst_alpha = factor;
    return *this;
  }
  ColorBlendAttachment& setAlphaOp(BlendOp op) {
    alpha_op = op;
    return *this;
  }
  ColorBlendAttachment& setColorComponents(const std::vector<ColorComponent>& components) {
    color_components = components;
    return *this;
  }
  ColorBlendAttachment& addColorComponent(ColorComponent component) {
    color_components.push_back(component);
    return *this;
  }
  ColorBlendAttachment& setColorBlend(BlendFactor src, BlendFactor dst, BlendOp op) {
    src_color = src;
    dst_color = dst;
    color_op = op;
    return *this;
  }
  ColorBlendAttachment& setAlphaBlend(BlendFactor src, BlendFactor dst, BlendOp op) {
    src_alpha = src;
    dst_alpha = dst;
    alpha_op = op;
    return *this;
  }
};

/// @brief Compute shader work group dimensions
/// Specifies the local work group size for compute shader dispatch
struct ComputeWorkGroupSize {
  uint32_t x;  ///< Work group size in X dimension
  uint32_t y;  ///< Work group size in Y dimension
  uint32_t z;  ///< Work group size in Z dimension

  // Fluent interface methods
  ComputeWorkGroupSize& setX(uint32_t size_x) {
    x = size_x;
    return *this;
  }
  ComputeWorkGroupSize& setY(uint32_t size_y) {
    y = size_y;
    return *this;
  }
  ComputeWorkGroupSize& setZ(uint32_t size_z) {
    z = size_z;
    return *this;
  }
  ComputeWorkGroupSize& setSize(uint32_t size_x, uint32_t size_y = 1, uint32_t size_z = 1) {
    x = size_x;
    y = size_y;
    z = size_z;
    return *this;
  }
};

/// @brief Clear color value for color attachments
/// RGBA color values used when clearing color attachments
struct ClearColor {
  std::array<float_t, 4> color;  ///< RGBA color components (0.0-1.0 range)

  // Fluent interface methods
  ClearColor& setColor(const std::array<float_t, 4>& rgba) {
    color = rgba;
    return *this;
  }
  ClearColor& setColor(float_t r, float_t g, float_t b, float_t a = 1.0f) {
    color = {r, g, b, a};
    return *this;
  }
  ClearColor& setRed(float_t r) {
    color[0] = r;
    return *this;
  }
  ClearColor& setGreen(float_t g) {
    color[1] = g;
    return *this;
  }
  ClearColor& setBlue(float_t b) {
    color[2] = b;
    return *this;
  }
  ClearColor& setAlpha(float_t a) {
    color[3] = a;
    return *this;
  }
};

/// @brief Clear values for depth and stencil attachments
/// Specifies the depth and stencil values to use when clearing depth/stencil buffers
struct ClearDepthStencil {
  float_t depth = 0.0f;   ///< Depth clear value (typically 0.0 or 1.0)
  uint32_t stencil = 0U;  ///< Stencil clear value (typically 0)

  // Fluent interface methods
  ClearDepthStencil& setDepth(float_t depth_value) {
    depth = depth_value;
    return *this;
  }
  ClearDepthStencil& setStencil(uint32_t stencil_value) {
    stencil = stencil_value;
    return *this;
  }
  ClearDepthStencil& setValues(float_t depth_value, uint32_t stencil_value) {
    depth = depth_value;
    stencil = stencil_value;
    return *this;
  }
};

}  // namespace pandora::core
