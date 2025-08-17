/*
 * rendering_types.hpp - Rendering-related type definitions for Pandolabo core
 * module
 *
 * This header contains enum class definitions used for rendering pipelines,
 * attachment operations, and command buffer management.
 */

#pragma once

namespace pandora::core {

/// @brief Attachment load operations for render pass attachments
/// Defines what to do with attachment data at the start of a render pass
enum class AttachmentLoadOp {
  Load = 0u,  ///< Load existing attachment data (preserve previous contents)
  Clear,      ///< Clear attachment to specified clear value
  DontCare,   ///< Don't care about previous contents (undefined behavior but
              ///< potentially faster)
};

/// @brief Attachment store operations for render pass attachments
/// Defines what to do with attachment data at the end of a render pass
enum class AttachmentStoreOp {
  Store = 0u,  ///< Store attachment data for later use
  DontCare,    ///< Don't store attachment data (contents become undefined)
};

/// @brief Subpass dependency flags
/// Controls the scope and behavior of subpass dependencies
enum class DependencyFlag {
  ByRegion = 0u,  ///< Dependency is by-region (can be more efficient)
  DeviceGroup,    ///< Dependency spans multiple devices in a device group
  ViewLocal,      ///< Dependency is view-local for multiview rendering
  Feedback,       ///< Feedback loop dependency (input and output from same
                  ///< attachment)
};

/// @brief Vertex input rate for vertex attributes
/// Specifies how vertex data advances during rendering
enum class VertexInputRate {
  Vertex = 0u,  ///< Advance per vertex (standard vertex attributes)
  Instance,     ///< Advance per instance (for instanced rendering)
};

/// @brief Primitive topology types
/// Defines how vertices are assembled into geometric primitives
enum class PrimitiveTopology {
  PointList = 0u,              ///< Each vertex is a separate point
  LineList,                    ///< Every two vertices form a line
  LineStrip,                   ///< Connected line segments
  TriangleList,                ///< Every three vertices form a triangle
  TriangleStrip,               ///< Connected triangles sharing edges
  TriangleFan,                 ///< Triangles radiating from a central vertex
  LineListWithAdjacency,       ///< Lines with adjacency information
  LineStripWithAdjacency,      ///< Line strip with adjacency information
  TriangleListWithAdjacency,   ///< Triangles with adjacency information
  TriangleStripWithAdjacency,  ///< Triangle strip with adjacency information
  PatchList,                   ///< Control points for tessellation
};

/// @brief Polygon rendering modes
/// Defines how polygons should be rendered
enum class PolygonMode {
  Fill = 0u,  ///< Fill polygons (solid rendering)
  Line,       ///< Render polygon edges as lines (wireframe)
  Point,      ///< Render polygon vertices as points
};

/// @brief Face culling modes
/// Defines which polygon faces should be culled (not rendered) based on winding
/// order
enum class CullMode {
  None = 0u,     ///< No face culling (render both front and back faces)
  Front,         ///< Cull front-facing polygons
  Back,          ///< Cull back-facing polygons (most common for closed objects)
  FrontAndBack,  ///< Cull both front and back faces (renders nothing)
};

/// @brief Front face winding order
/// Defines which vertex winding order is considered front-facing
enum class FrontFace {
  CounterClockwise =
      0u,     ///< Counter-clockwise winding is front-facing (OpenGL convention)
  Clockwise,  ///< Clockwise winding is front-facing (DirectX convention)
};

/// @brief Depth and stencil comparison operations
/// Defines comparison functions for depth testing and stencil testing
enum class CompareOp {
  Never = 0u,      ///< Comparison never passes
  Less,            ///< Pass if source is less than destination
  Equal,           ///< Pass if source equals destination
  LessOrEqual,     ///< Pass if source is less than or equal to destination
  Greater,         ///< Pass if source is greater than destination
  NotEqual,        ///< Pass if source is not equal to destination
  GreaterOrEqual,  ///< Pass if source is greater than or equal to destination
  Always,          ///< Comparison always passes
};

/// @brief Stencil buffer operations
/// Defines operations performed on stencil buffer values
enum class StencilOp {
  Keep = 0u,          ///< Keep current stencil value
  Zero,               ///< Set stencil value to zero
  Replace,            ///< Replace with reference value
  IncrementAndClamp,  ///< Increment and clamp to maximum value
  DecrementAndClamp,  ///< Decrement and clamp to zero
  Invert,             ///< Bitwise invert stencil value
  IncrementAndWrap,   ///< Increment with wraparound
  DecrementAndWrap,   ///< Decrement with wraparound
};

/// @brief Logical operations for color blending
/// Defines bitwise logical operations between source and destination colors
enum class LogicOp {
  Clear = 0u,    ///< Result = 0 (clear to black)
  And,           ///< Result = src & dst (bitwise AND)
  AndReverse,    ///< Result = src & ~dst (AND with inverted destination)
  Copy,          ///< Result = src (copy source)
  AndInverted,   ///< Result = ~src & dst (AND with inverted source)
  NoOp,          ///< Result = dst (no operation, keep destination)
  Xor,           ///< Result = src ^ dst (bitwise XOR)
  Or,            ///< Result = src | dst (bitwise OR)
  Nor,           ///< Result = ~(src | dst) (bitwise NOR)
  Equivalent,    ///< Result = ~(src ^ dst) (bitwise equivalence)
  Invert,        ///< Result = ~dst (invert destination)
  OrReverse,     ///< Result = src | ~dst (OR with inverted destination)
  CopyInverted,  ///< Result = ~src (copy inverted source)
  OrInverted,    ///< Result = ~src | dst (OR with inverted source)
  Nand,          ///< Result = ~(src & dst) (bitwise NAND)
  Set,           ///< Result = 1 (set to white)
};

/// @brief Blend factor values for color blending
/// Defines factors used in blending equations to combine source and destination
/// colors
enum class BlendFactor {
  Zero = 0u,              ///< Factor = (0, 0, 0, 0)
  One,                    ///< Factor = (1, 1, 1, 1)
  SrcColor,               ///< Factor = (Rs, Gs, Bs, As) - source color
  OneMinusSrcColor,       ///< Factor = (1-Rs, 1-Gs, 1-Bs, 1-As)
  DstColor,               ///< Factor = (Rd, Gd, Bd, Ad) - destination color
  OneMinusDstColor,       ///< Factor = (1-Rd, 1-Gd, 1-Bd, 1-Ad)
  SrcAlpha,               ///< Factor = (As, As, As, As) - source alpha
  OneMinusSrcAlpha,       ///< Factor = (1-As, 1-As, 1-As, 1-As)
  DstAlpha,               ///< Factor = (Ad, Ad, Ad, Ad) - destination alpha
  OneMinusDstAlpha,       ///< Factor = (1-Ad, 1-Ad, 1-Ad, 1-Ad)
  ConstantColor,          ///< Factor = (Rc, Gc, Bc, Ac) - constant color
  OneMinusConstantColor,  ///< Factor = (1-Rc, 1-Gc, 1-Bc, 1-Ac)
  ConstantAlpha,          ///< Factor = (Ac, Ac, Ac, Ac) - constant alpha
  OneMinusConstantAlpha,  ///< Factor = (1-Ac, 1-Ac, 1-Ac, 1-Ac)
  SrcAlphaSaturate,       ///< Factor = (f, f, f, 1) where f = min(As, 1-Ad)
  Src1Color,              ///< Factor = (Rs1, Gs1, Bs1, As1) - dual source color
  OneMinusSrc1Color,      ///< Factor = (1-Rs1, 1-Gs1, 1-Bs1, 1-As1)
  Src1Alpha,              ///< Factor = (As1, As1, As1, As1) - dual source alpha
  OneMinusSrc1Alpha,      ///< Factor = (1-As1, 1-As1, 1-As1, 1-As1)
};

/// @brief Blend operations for color blending
/// Defines mathematical operations between source and destination colors
enum class BlendOp {
  Add = 0u,         ///< Result = src + dst (additive blending)
  Subtract,         ///< Result = src - dst (subtractive blending)
  ReverseSubtract,  ///< Result = dst - src (reverse subtractive blending)
  Min,  ///< Result = min(src, dst) (minimum of source and destination)
  Max,  ///< Result = max(src, dst) (maximum of source and destination)
};

/// @brief Color component flags
/// Defines individual color channels for write masking
enum class ColorComponent {
  R = 0u,  ///< Red color component
  G,       ///< Green color component
  B,       ///< Blue color component
  A,       ///< Alpha (transparency) component
};

/// @brief Dynamic pipeline state options
/// Defines pipeline states that can be changed dynamically without rebuilding
/// the pipeline
enum class DynamicOption {
  Viewport = 0u,       ///< Viewport transformation parameters
  Scissor,             ///< Scissor rectangle for clipping
  LineWidth,           ///< Line width for line primitives
  DepthBias,           ///< Depth bias values for polygon offset
  BlendConstants,      ///< Constant color values for blending
  DepthBounds,         ///< Depth bounds test values
  StencilCompareMask,  ///< Stencil comparison mask
  StencilWriteMask,    ///< Stencil write mask
  StencilReference,    ///< Stencil reference value
};

/// @brief Command buffer usage flags
/// Defines how a command buffer will be used and submitted
enum class CommandBufferUsage {
  OneTimeSubmit = 0u,  ///< Command buffer will be submitted once and reset
  RenderPassContinue,  ///< Secondary command buffer continues a render pass
  SimultaneousUse,     ///< Command buffer can be submitted multiple times
                       ///< simultaneously
};

/// @brief Subpass contents specification
/// Defines how commands within a subpass are provided
enum class SubpassContents {
  Inline = 0u,  ///< Commands are recorded directly in primary command buffer
  SecondaryCommandBuffers,  ///< Commands are provided via secondary command
                            ///< buffers
};

}  // namespace pandora::core
