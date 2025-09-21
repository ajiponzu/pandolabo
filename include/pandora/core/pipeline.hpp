/*
 * pipeline.hpp - Pipeline and pipeline configuration for Pandolabo core module
 *
 * This header contains classes for managing graphics and compute pipelines,
 * including pipeline state configurations and shader management.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "gpu.hpp"
#include "module_connection/gpu_ui.hpp"
#include "rendering_structures.hpp"
#include "rendering_types.hpp"
#include "types.hpp"

// Forward declarations
namespace pandora::core {
class Renderpass;
}

namespace pandora::core::gpu {
class Context;
class ShaderModule;
class DescriptionUnit;
class DescriptorSetLayout;
}  // namespace pandora::core::gpu

namespace pandora::core {

namespace pipeline {

/// @brief Vertex input configuration for graphics pipelines
/// Manages vertex buffer bindings and attribute descriptions that define
/// how vertex data is fed into the vertex shader stage
class VertexInput {
 public:
  vk::PipelineVertexInputStateCreateInfo
      m_info{};  ///< Vulkan vertex input state
  std::vector<vk::VertexInputBindingDescription>
      m_bindings;  ///< Vertex buffer binding descriptions
  std::vector<vk::VertexInputAttributeDescription>
      m_attributes;  ///< Vertex attribute descriptions

  // Rule of Zero
  VertexInput() = default;
  ~VertexInput() = default;

  /// @brief Add a vertex buffer binding
  /// @param binding Binding index for this vertex buffer
  /// @param stride Size in bytes between consecutive vertex elements
  /// @param input_rate Whether data is per-vertex or per-instance
  void appendBinding(uint32_t binding,
                     uint32_t stride,
                     const VertexInputRate input_rate);

  /// @brief Add a vertex attribute description
  /// @param location Shader input location for this attribute
  /// @param binding Vertex buffer binding this attribute comes from
  /// @param format Data format of the attribute
  /// @param offset Byte offset of this attribute within the vertex
  void appendAttribute(uint32_t location,
                       uint32_t binding,
                       DataFormat format,
                       uint32_t offset);

  // Fluent interface methods
  VertexInput& addBinding(uint32_t binding,
                          uint32_t stride,
                          VertexInputRate input_rate) {
    appendBinding(binding, stride, input_rate);
    return *this;
  }

  VertexInput& addAttribute(uint32_t location,
                            uint32_t binding,
                            DataFormat format,
                            uint32_t offset) {
    appendAttribute(location, binding, format, offset);
    return *this;
  }
};

/// @brief Input assembly configuration for graphics pipelines
/// Controls how vertices are assembled into primitives (points, lines,
/// triangles) and whether primitive restart is enabled for indexed drawing
class InputAssembly {
 public:
  vk::PipelineInputAssemblyStateCreateInfo
      m_info{};  ///< Vulkan input assembly state

  // Rule of Zero
  InputAssembly() = default;
  ~InputAssembly() = default;

  /// @brief Set primitive topology type
  /// @param topology How vertices are assembled (triangles, lines, points,
  /// etc.)
  void setTopology(PrimitiveTopology topology);

  /// @brief Enable or disable primitive restart
  /// @param is_enabled Whether to enable primitive restart with special index
  /// values
  void setRestart(bool is_enabled);

  // Fluent interface methods
  InputAssembly& withTopology(PrimitiveTopology topology) {
    setTopology(topology);
    return *this;
  }

  InputAssembly& withRestart(bool is_enabled) {
    setRestart(is_enabled);
    return *this;
  }
};

/// @brief Tessellation stage configuration for graphics pipelines
/// Configures the tessellation control and evaluation shader stages
/// for subdividing patches into more detailed geometry
class Tessellation {
 public:
  vk::PipelineTessellationStateCreateInfo
      m_info{};  ///< Vulkan tessellation state

  // Rule of Zero
  Tessellation() = default;
  ~Tessellation() = default;

  /// @brief Set number of control points per patch
  /// @param count Number of vertices that define each tessellation patch
  void setPatchControlPoints(uint32_t count);

  // Fluent interface methods
  Tessellation& withPatchControlPoints(uint32_t count) {
    setPatchControlPoints(count);
    return *this;
  }
};

/// @brief Viewport and scissor state configuration for graphics pipelines
/// Defines the viewport transformation from normalized device coordinates to
/// framebuffer coordinates and optional scissor rectangle for pixel clipping
class ViewportState {
 public:
  vk::PipelineViewportStateCreateInfo m_info{};  ///< Vulkan viewport state
  vk::Viewport m_viewport;  ///< Viewport transformation parameters
  vk::Rect2D m_scissor;     ///< Scissor test rectangle

  // Rule of Zero
  ViewportState() = default;
  ~ViewportState() = default;

  /// @brief Set viewport transformation parameters
  /// @param size Width and height of the viewport
  /// @param min_depth Minimum depth value (typically 0.0)
  /// @param max_depth Maximum depth value (typically 1.0)
  void setViewport(const gpu_ui::GraphicalSize<float_t>& size,
                   float_t min_depth,
                   float_t max_depth);

  /// @brief Set scissor test rectangle
  /// @param size Width and height of the scissor rectangle
  void setScissor(const gpu_ui::GraphicalSize<uint32_t>& size);

  // Fluent interface methods
  ViewportState& withViewport(const gpu_ui::GraphicalSize<float_t>& size,
                              float_t min_depth,
                              float_t max_depth) {
    setViewport(size, min_depth, max_depth);
    return *this;
  }

  ViewportState& withScissor(const gpu_ui::GraphicalSize<uint32_t>& size) {
    setScissor(size);
    return *this;
  }
};

/// @brief Rasterization state configuration for graphics pipelines
/// Controls how geometry is converted to fragments, including polygon mode,
/// culling, depth bias, and line width settings
class Rasterization {
 public:
  vk::PipelineRasterizationStateCreateInfo
      m_info{};  ///< Vulkan rasterization state

  // Rule of Zero
  Rasterization() = default;
  ~Rasterization() = default;

  /// @brief Enable or disable depth bias
  /// @param is_enabled Whether to apply depth bias to fragments
  void setDepthBiasEnabled(bool is_enabled);

  /// @brief Configure depth bias parameters
  /// @param constant_factor Constant depth bias factor
  /// @param clamp Maximum depth bias clamp value
  /// @param slope_factor Slope-dependent depth bias factor
  void setDepthBias(float_t constant_factor,
                    float_t clamp,
                    float_t slope_factor);

  /// @brief Enable or disable rasterizer discard
  /// @param is_enabled Whether to discard primitives before rasterization
  void setRasterizerDiscard(bool is_enabled);

  /// @brief Set polygon rendering mode
  /// @param polygon_mode How to render polygons (fill, line, point)
  void setPolygonMode(PolygonMode polygon_mode);

  /// @brief Set face culling mode
  /// @param cull_mode Which faces to cull (none, front, back, front+back)
  void setCullMode(CullMode cull_mode);

  /// @brief Set front face winding order
  /// @param front_face Whether front faces are clockwise or counter-clockwise
  void setFrontFace(FrontFace front_face);

  /// @brief Set line width for line primitives
  /// @param line_width Width of rasterized lines in pixels
  void setLineWidth(float_t line_width);

  // Fluent interface methods
  Rasterization& withDepthBiasEnabled(bool is_enabled) {
    setDepthBiasEnabled(is_enabled);
    return *this;
  }

  Rasterization& withDepthBias(float_t constant_factor,
                               float_t clamp,
                               float_t slope_factor) {
    setDepthBias(constant_factor, clamp, slope_factor);
    return *this;
  }

  Rasterization& withRasterizerDiscard(bool is_enabled) {
    setRasterizerDiscard(is_enabled);
    return *this;
  }

  Rasterization& withPolygonMode(PolygonMode polygon_mode) {
    setPolygonMode(polygon_mode);
    return *this;
  }

  Rasterization& withCullMode(CullMode cull_mode) {
    setCullMode(cull_mode);
    return *this;
  }

  Rasterization& withFrontFace(FrontFace front_face) {
    setFrontFace(front_face);
    return *this;
  }

  Rasterization& withLineWidth(float_t line_width) {
    setLineWidth(line_width);
    return *this;
  }
};

/// @brief Multisample anti-aliasing configuration for graphics pipelines
/// Controls multisampling parameters for anti-aliasing, including sample count,
/// sample shading, and coverage/alpha-to-coverage operations
class Multisample {
 public:
  vk::PipelineMultisampleStateCreateInfo m_info{};

  // Rule of Zero
  Multisample() = default;
  ~Multisample() = default;

  void setSampleCount(const std::unique_ptr<gpu::Context>& ptr_context);
  void setSampleShading(bool is_enabled);
  void setMinSampleShading(float_t min_sample_shading);

  // Fluent interface methods
  Multisample& withSampleCount(
      const std::unique_ptr<gpu::Context>& ptr_context) {
    setSampleCount(ptr_context);
    return *this;
  }

  Multisample& withSampleShading(bool is_enabled) {
    setSampleShading(is_enabled);
    return *this;
  }

  Multisample& withMinSampleShading(float_t min_sample_shading) {
    setMinSampleShading(min_sample_shading);
    return *this;
  }
};

/// @brief Depth and stencil testing configuration for graphics pipelines
/// Controls depth testing, depth writing, stencil testing, and depth bounds
/// testing for per-fragment depth and stencil operations
class DepthStencil {
 public:
  vk::PipelineDepthStencilStateCreateInfo m_info{};

  // Rule of Zero
  DepthStencil() = default;
  ~DepthStencil() = default;

  void setDepthTest(bool is_enabled);
  void setDepthWrite(bool is_enabled);
  void setDepthCompareOp(CompareOp compare_op);
  void setDepthBoundsTest(bool is_enabled);
  void setStencilTest(bool is_enabled);
  void setFrontStencilOp(const StencilOpState& state);
  void setBackStencilOp(const StencilOpState& state);

  // Fluent interface methods
  DepthStencil& withDepthTest(bool is_enabled) {
    setDepthTest(is_enabled);
    return *this;
  }

  DepthStencil& withDepthWrite(bool is_enabled) {
    setDepthWrite(is_enabled);
    return *this;
  }

  DepthStencil& withDepthCompareOp(CompareOp compare_op) {
    setDepthCompareOp(compare_op);
    return *this;
  }

  DepthStencil& withDepthBoundsTest(bool is_enabled) {
    setDepthBoundsTest(is_enabled);
    return *this;
  }

  DepthStencil& withStencilTest(bool is_enabled) {
    setStencilTest(is_enabled);
    return *this;
  }

  DepthStencil& withFrontStencilOp(const StencilOpState& state) {
    setFrontStencilOp(state);
    return *this;
  }

  DepthStencil& withBackStencilOp(const StencilOpState& state) {
    setBackStencilOp(state);
    return *this;
  }
};

/// @brief Color blending configuration for graphics pipelines
/// Controls per-attachment color blending operations, logic operations,
/// and blend constants for fragment color output
class ColorBlend {
 public:
  vk::PipelineColorBlendStateCreateInfo m_info{};
  std::vector<vk::PipelineColorBlendAttachmentState> m_attachments;

  // Rule of Zero
  ColorBlend() = default;
  ~ColorBlend() = default;

  void setLogicOp(bool is_enabled, LogicOp logic_op);
  void appendAttachment(const ColorBlendAttachment& attachment);

  // Fluent interface methods
  ColorBlend& withLogicOp(bool is_enabled, LogicOp logic_op) {
    setLogicOp(is_enabled, logic_op);
    return *this;
  }

  ColorBlend& addAttachment(const ColorBlendAttachment& attachment) {
    appendAttachment(attachment);
    return *this;
  }
};

/// @brief Dynamic state configuration for graphics pipelines
/// Specifies which pipeline states can be changed dynamically at command buffer
/// recording time without recreating the entire pipeline
class DynamicState {
 public:
  vk::PipelineDynamicStateCreateInfo m_info{};
  std::vector<vk::DynamicState> m_states;

  // Rule of Zero
  DynamicState() = default;
  ~DynamicState() = default;

  void appendState(DynamicOption option);

  // Fluent interface methods
  DynamicState& addState(DynamicOption option) {
    appendState(option);
    return *this;
  }
};

/// @brief Graphics pipeline configuration structure
/// Aggregates all graphics pipeline state configurations into a single
/// structure for convenient pipeline creation and management
struct GraphicInfo {
  VertexInput vertex_input{};
  InputAssembly input_assembly{};
  Tessellation tessellation{};
  ViewportState viewport_state{};
  Rasterization rasterization{};
  Multisample multisample{};
  DepthStencil depth_stencil{};
  ColorBlend color_blend{};
  DynamicState dynamic_state{};

  // Rule of Five
  GraphicInfo() = default;
  ~GraphicInfo() = default;

  // Copy constructor
  GraphicInfo(const GraphicInfo& other) = default;

  // Copy assignment operator
  GraphicInfo& operator=(const GraphicInfo& other) = default;

  // Move constructor
  GraphicInfo(GraphicInfo&& other) noexcept = default;

  // Move assignment operator
  GraphicInfo& operator=(GraphicInfo&& other) noexcept = default;
};

/// @brief Builder class for GraphicInfo
/// Provides a fluent interface for constructing GraphicInfo instances
/// and returns a unique_ptr to the final configuration
class GraphicInfoBuilder {
 private:
  std::unique_ptr<GraphicInfo> m_ptrInfo{};

  // Private constructor - use create() factory method instead
  explicit GraphicInfoBuilder(std::unique_ptr<GraphicInfo> ptr_info)
      : m_ptrInfo(std::move(ptr_info)) {}

 public:
  /// @brief Static factory method to create a new GraphicInfoBuilder
  /// @return A new GraphicInfoBuilder instance
  static GraphicInfoBuilder create() {
    return GraphicInfoBuilder{std::make_unique<GraphicInfo>()};
  }

  /// @brief Set vertex input configuration
  /// @param input VertexInput configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setVertexInput(const VertexInput& input) {
    m_ptrInfo->vertex_input = input;
    return *this;
  }

  /// @brief Set input assembly configuration
  /// @param assembly InputAssembly configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setInputAssembly(const InputAssembly& assembly) {
    m_ptrInfo->input_assembly = assembly;
    return *this;
  }

  /// @brief Set tessellation configuration
  /// @param tess Tessellation configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setTessellation(const Tessellation& tess) {
    m_ptrInfo->tessellation = tess;
    return *this;
  }

  /// @brief Set viewport state configuration
  /// @param state ViewportState configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setViewportState(const ViewportState& state) {
    m_ptrInfo->viewport_state = state;
    return *this;
  }

  /// @brief Set rasterization configuration
  /// @param raster Rasterization configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setRasterization(const Rasterization& raster) {
    m_ptrInfo->rasterization = raster;
    return *this;
  }

  /// @brief Set multisample configuration
  /// @param multi Multisample configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setMultisample(const Multisample& multi) {
    m_ptrInfo->multisample = multi;
    return *this;
  }

  /// @brief Set depth stencil configuration
  /// @param depth DepthStencil configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setDepthStencil(const DepthStencil& depth) {
    m_ptrInfo->depth_stencil = depth;
    return *this;
  }

  /// @brief Set color blend configuration
  /// @param blend ColorBlend configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setColorBlend(const ColorBlend& blend) {
    m_ptrInfo->color_blend = blend;
    return *this;
  }

  /// @brief Set dynamic state configuration
  /// @param state DynamicState configuration
  /// @return Reference to this builder for method chaining
  GraphicInfoBuilder& setDynamicState(const DynamicState& state) {
    m_ptrInfo->dynamic_state = state;
    return *this;
  }

  /// @brief Build and return the final GraphicInfo instance
  /// @return unique_ptr to the constructed GraphicInfo
  std::unique_ptr<GraphicInfo> build() {
    return std::move(m_ptrInfo);
  }
};

}  // namespace pipeline

/// @brief Vulkan pipeline and pipeline layout wrapper
/// Encapsulates Vulkan pipeline creation and management.
/// In the Pandolabo project, pipeline and pipeline layout are managed together
/// as they are tightly coupled and separation is not necessary for most use
/// cases.
class Pipeline {
 protected:
  vk::UniquePipeline m_ptrPipeline;              ///< Vulkan pipeline object
  vk::UniquePipelineLayout m_ptrPipelineLayout;  ///< Vulkan pipeline layout
  QueueFamilyType m_queueFamilyType{};  ///< Queue family type for this pipeline
  vk::PipelineBindPoint
      m_bindPoint{};  ///< Pipeline bind point (graphics or compute)

 public:
  Pipeline(const std::unique_ptr<gpu::Context>& ptr_context,
           const gpu::DescriptionUnit& description_unit,
           const gpu::DescriptorSetLayout& descriptor_set_layout,
           PipelineBind bind_point);

  // Rule of Five
  ~Pipeline();
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;
  Pipeline(Pipeline&&) = default;
  Pipeline& operator=(Pipeline&&) = default;

  const auto& getPipeline() const {
    return m_ptrPipeline.get();
  }
  const auto& getPipelineLayout() const {
    return m_ptrPipelineLayout.get();
  }
  auto getQueueFamilyType() const {
    return m_queueFamilyType;
  }
  auto getBindPoint() const {
    return m_bindPoint;
  }

  /// @brief Construct pipeline for compute shader
  /// @param ptr_context Vulkan context for device operations
  /// @param shader_module Compute shader module
  void constructComputePipeline(
      const std::unique_ptr<gpu::Context>& ptr_context,
      const gpu::ShaderModule& shader_module);

  /// @brief Construct pipeline for graphics rendering
  /// @param ptr_context Vulkan context for device operations
  /// @param shader_module_map Map of shader modules by name
  /// @param module_keys Keys order for shader stages (vertex, fragment, etc.)
  /// @param ptr_graphic_info Graphics pipeline configuration
  /// @param render_pass Render pass for this pipeline
  /// @param subpass_index Subpass index using this pipeline
  void constructGraphicsPipeline(
      const std::unique_ptr<gpu::Context>& ptr_context,
      const std::unordered_map<std::string, gpu::ShaderModule>&
          shader_module_map,
      const std::vector<std::string>& module_keys,
      const std::unique_ptr<pipeline::GraphicInfo>& ptr_graphic_info,
      const Renderpass& render_pass,
      uint32_t subpass_index);
};

}  // namespace pandora::core
