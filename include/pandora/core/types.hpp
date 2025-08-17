/*
 * types.hpp - Core type definitions for Pandolabo Vulkan C++ wrapper
 *
 * This header contains all enum class definitions used throughout the library.
 * These types provide a high-level abstraction over Vulkan's enum values.
 */

#pragma once

namespace pandora::core {

/// @brief Queue family types for different GPU operations
enum class QueueFamilyType {
  Graphics = 0u,
  Compute,
  Transfer,
};

/// @brief Memory usage patterns for buffer allocation
enum class MemoryUsage {
  Unknown = 0u,
  GpuOnly,
  CpuOnly,
  CpuToGpu,
  GpuToCpu,
};

/// @brief Transfer operation types
enum class TransferType {
  Unknown = 0u,
  TransferSrc,
  TransferDst,
  TransferSrcDst,
};

/// @brief Buffer usage types
enum class BufferUsage {
  Unknown = 0u,
  VertexBuffer,
  IndexBuffer,
  UniformBuffer,
  StorageBuffer,
  StagingBuffer,
};

/// @brief Image usage types
enum class ImageUsage {
  Unknown = 0u,
  Sampled,
  Storage,
  ColorAttachment,
  DepthStencilAttachment,
  TransientAttachment,
  InputAttachment,
};

/// @brief Data format types for images and buffers
enum class DataFormat {
  Unknown = 0u,
  R8Unorm,
  R8Snorm,
  R8Uscaled,
  R8Sscaled,
  R8Uint,
  R8Sint,
  R8Srgb,
  R8G8Unorm,
  R8G8Snorm,
  R8G8Uscaled,
  R8G8Sscaled,
  R8G8Uint,
  R8G8Sint,
  R8G8Srgb,
  R8G8B8Unorm,
  R8G8B8Snorm,
  R8G8B8Uscaled,
  R8G8B8Sscaled,
  R8G8B8Uint,
  R8G8B8Sint,
  R8G8B8Srgb,
  R8G8B8A8Unorm,
  R8G8B8A8Snorm,
  R8G8B8A8Uscaled,
  R8G8B8A8Sscaled,
  R8G8B8A8Uint,
  R8G8B8A8Sint,
  R8G8B8A8Srgb,
  R32Sfloat,
  R32G32Sfloat,
  R32G32B32Sfloat,
  R32G32B32A32Sfloat,
  Depth,
  DepthSfloatStencilUint,
  Depth24UnormStencilUint,
};

/// @brief Image dimension types
enum class ImageDimension {
  Unknown = 0u,
  v1D,
  v2D,
  v3D,
};

/// @brief Image sample count for multisampling
enum class ImageSampleCount {
  Unknown = 0u,
  v1,
  v2,
  v4,
  v8,
  v16,
  v32,
  v64,
};

/// @brief Shader types
enum class ShaderType {
  Vertex = 0u,
  Fragment,
  Compute,
};

/// @brief Image aspect flags
enum class ImageAspect {
  Unknown = 0u,
  Color,
  Depth,
  Stencil,
  DepthStencil,
};

/// @brief Sampler filter modes
enum class SamplerFilter {
  Linear = 0u,
  Nearest,
  Cubic,
};

/// @brief Sampler mipmap modes
enum class SamplerMipmapMode {
  Nearest = 0u,
  Linear,
};

/// @brief Sampler address modes
enum class SamplerAddressMode {
  Repeat = 0u,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder,
  MirrorClampToEdge,
};

/// @brief Sampler border colors
enum class SamplerBorderColor {
  FloatTransparentBlack = 0u,
  IntTransparentBlack,
  FloatOpaqueBlack,
  IntOpaqueBlack,
  FloatOpaqueWhite,
  IntOpaqueWhite,
};

/// @brief Sampler wrap modes
enum class SamplerWrapMode {
  Repeat = 0u,
  MirroredRepeat,
  ClampToEdge,
  ClampToBorder,
  MirrorClampToEdge,
};

/// @brief Sampler comparison operations
enum class SamplerCompareOp {
  Never = 0u,
  Less,
  Equal,
  LessOrEqual,
  Greater,
  NotEqual,
  GreaterOrEqual,
  Always,
};

/// @brief Image layout types
enum class ImageLayout {
  Undefined = 0u,
  General,
  ColorAttachmentOptimal,
  DepthStencilAttachmentOptimal,
  DepthStencilReadOnlyOptimal,
  ShaderReadOnlyOptimal,
  TransferSrcOptimal,
  TransferDstOptimal,
  Preinitialized,
  PresentSrc,
  SharedPresent,
  DepthReadOnlyStencilAttachmentOptimal,
  DepthAttachmentStencilReadOnlyOptimal,
};

/// @brief Pipeline stage flags
enum class PipelineStage {
  TopOfPipe = 0u,
  DrawIndirect,
  VertexInput,
  VertexShader,
  TessellationControlShader,
  TessellationEvaluationShader,
  GeometryShader,
  FragmentShader,
  EarlyFragmentTests,
  LateFragmentTests,
  ColorAttachmentOutput,
  ComputeShader,
  Transfer,
  BottomOfPipe,
  Host,
  AllGraphics,
  AllCommands,
};

/// @brief Memory access flags
enum class AccessFlag {
  Unknown = 0u,
  IndirectCommandRead,
  IndexRead,
  VertexAttributeRead,
  UniformRead,
  InputAttachmentRead,
  ShaderRead,
  ShaderWrite,
  ColorAttachmentRead,
  ColorAttachmentWrite,
  DepthStencilAttachmentRead,
  DepthStencilAttachmentWrite,
  TransferRead,
  TransferWrite,
  HostRead,
  HostWrite,
  MemoryRead,
  MemoryWrite,
};

/// @brief Shader stage flags
enum class ShaderStage {
  Vertex = 0u,
  TessellationControl,
  TessellationEvaluation,
  Geometry,
  Fragment,
  Compute,
};

/// @brief Pipeline bind points
enum class PipelineBind {
  Graphics = 0u,
  Compute,
  RayTracing,
};

}  // namespace pandora::core
