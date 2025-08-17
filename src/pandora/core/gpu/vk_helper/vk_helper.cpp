#include "pandora/core/gpu/vk_helper.hpp"

vk::MemoryPropertyFlags vk_helper::getMemoryPropertyFlags(const pandora::core::MemoryUsage memory_usage) {
  switch (memory_usage) {
    using enum pandora::core::MemoryUsage;
    using enum vk::MemoryPropertyFlagBits;

    case GpuOnly:
      return eDeviceLocal;
    case CpuOnly:
      return eHostVisible | eHostCoherent;
    case CpuToGpu:
      return eHostVisible | eDeviceLocal;
    case GpuToCpu:
      return eHostVisible | eHostCached;
    default:
      return eDeviceLocal;
  }
}

vk::AccessFlagBits vk_helper::getAccessFlagBits(const pandora::core::AccessFlag access_flag) {
  switch (access_flag) {
    using enum pandora::core::AccessFlag;
    using enum vk::AccessFlagBits;

    case IndirectCommandRead:
      return eIndirectCommandRead;
    case IndexRead:
      return eIndexRead;
    case VertexAttributeRead:
      return eVertexAttributeRead;
    case UniformRead:
      return eUniformRead;
    case InputAttachmentRead:
      return eInputAttachmentRead;
    case ShaderRead:
      return eShaderRead;
    case ShaderWrite:
      return eShaderWrite;
    case ColorAttachmentRead:
      return eColorAttachmentRead;
    case ColorAttachmentWrite:
      return eColorAttachmentWrite;
    case DepthStencilAttachmentRead:
      return eDepthStencilAttachmentRead;
    case DepthStencilAttachmentWrite:
      return eDepthStencilAttachmentWrite;
    case TransferRead:
      return eTransferRead;
    case TransferWrite:
      return eTransferWrite;
    case HostRead:
      return eHostRead;
    case HostWrite:
      return eHostWrite;
    case MemoryRead:
      return eMemoryRead;
    case MemoryWrite:
      return eMemoryWrite;
    default:
      return eNone;
  }
}

vk::PipelineStageFlagBits vk_helper::getPipelineStageFlagBits(const pandora::core::PipelineStage stage) {
  switch (stage) {
    using enum pandora::core::PipelineStage;
    using enum vk::PipelineStageFlagBits;

    case TopOfPipe:
      return eTopOfPipe;
    case DrawIndirect:
      return eDrawIndirect;
    case VertexInput:
      return eVertexInput;
    case VertexShader:
      return eVertexShader;
    case TessellationControlShader:
      return eTessellationControlShader;
    case TessellationEvaluationShader:
      return eTessellationEvaluationShader;
    case GeometryShader:
      return eGeometryShader;
    case FragmentShader:
      return eFragmentShader;
    case EarlyFragmentTests:
      return eEarlyFragmentTests;
    case LateFragmentTests:
      return eLateFragmentTests;
    case ColorAttachmentOutput:
      return eColorAttachmentOutput;
    case ComputeShader:
      return eComputeShader;
    case Transfer:
      return eTransfer;
    case BottomOfPipe:
      return eBottomOfPipe;
    case Host:
      return eHost;
    case AllGraphics:
      return eAllGraphics;
    case AllCommands:
      return eAllCommands;
    default:
      return eNone;
  }
}

vk::PipelineBindPoint vk_helper::getPipelineBindPoint(const pandora::core::PipelineBind bind_point) {
  switch (bind_point) {
    using enum pandora::core::PipelineBind;
    using enum vk::PipelineBindPoint;

    case Graphics:
      return eGraphics;
    case Compute:
      return eCompute;
    case RayTracing:
      return eRayTracingKHR;
    default:
      return eGraphics;
  }
}

vk::ImageUsageFlagBits vk_helper::getImageUsage(const pandora::core::ImageUsage image_usage) {
  switch (image_usage) {
    using enum pandora::core::ImageUsage;
    using enum vk::ImageUsageFlagBits;

    case Sampled:
      return eSampled;
    case Storage:
      return eStorage;
    case ColorAttachment:
      return eColorAttachment;
    case DepthStencilAttachment:
      return eDepthStencilAttachment;
    case TransientAttachment:
      return eTransientAttachment;
    case InputAttachment:
      return eInputAttachment;
    default:
      return eSampled;
  }
}

vk::ImageLayout vk_helper::getImageLayout(const pandora::core::ImageLayout image_layout) {
  switch (image_layout) {
    using enum pandora::core::ImageLayout;
    using enum vk::ImageLayout;

    case Undefined:
      return eUndefined;
    case General:
      return eGeneral;
    case ColorAttachmentOptimal:
      return eColorAttachmentOptimal;
    case DepthStencilAttachmentOptimal:
      return eDepthStencilAttachmentOptimal;
    case DepthStencilReadOnlyOptimal:
      return eDepthStencilReadOnlyOptimal;
    case ShaderReadOnlyOptimal:
      return eShaderReadOnlyOptimal;
    case TransferSrcOptimal:
      return eTransferSrcOptimal;
    case TransferDstOptimal:
      return eTransferDstOptimal;
    case Preinitialized:
      return ePreinitialized;
    case PresentSrc:
      return ePresentSrcKHR;
    case SharedPresent:
      return eSharedPresentKHR;
    case DepthReadOnlyStencilAttachmentOptimal:
      return eDepthReadOnlyStencilAttachmentOptimal;
    case DepthAttachmentStencilReadOnlyOptimal:
      return eDepthAttachmentStencilReadOnlyOptimal;
    default:
      return eUndefined;
  }
}

vk::Format vk_helper::getFormat(const pandora::core::DataFormat format) {
  switch (format) {
    using enum pandora::core::DataFormat;
    using enum vk::Format;

    case R8Unorm:
      return eR8Unorm;
    case R8Snorm:
      return eR8Snorm;
    case R8Uscaled:
      return eR8Uscaled;
    case R8Sscaled:
      return eR8Sscaled;
    case R8Uint:
      return eR8Uint;
    case R8Sint:
      return eR8Sint;
    case R8Srgb:
      return eR8Srgb;
    case R8G8Unorm:
      return eR8G8Unorm;
    case R8G8Snorm:
      return eR8G8Snorm;
    case R8G8Uscaled:
      return eR8G8Uscaled;
    case R8G8Sscaled:
      return eR8G8Sscaled;
    case R8G8Uint:
      return eR8G8Uint;
    case R8G8Sint:
      return eR8G8Sint;
    case R8G8Srgb:
      return eR8G8Srgb;
    case R8G8B8Unorm:
      return eR8G8B8Unorm;
    case R8G8B8Snorm:
      return eR8G8B8Snorm;
    case R8G8B8Uscaled:
      return eR8G8B8Uscaled;
    case R8G8B8Sscaled:
      return eR8G8B8Sscaled;
    case R8G8B8Uint:
      return eR8G8B8Uint;
    case R8G8B8Sint:
      return eR8G8B8Sint;
    case R8G8B8Srgb:
      return eR8G8B8Srgb;
    case R8G8B8A8Unorm:
      return eR8G8B8A8Unorm;
    case R8G8B8A8Snorm:
      return eR8G8B8A8Snorm;
    case R8G8B8A8Uscaled:
      return eR8G8B8A8Uscaled;
    case R8G8B8A8Sscaled:
      return eR8G8B8A8Sscaled;
    case R8G8B8A8Uint:
      return eR8G8B8A8Uint;
    case R8G8B8A8Sint:
      return eR8G8B8A8Sint;
    case R8G8B8A8Srgb:
      return eR8G8B8A8Srgb;
    case R32Sfloat:
      return eR32Sfloat;
    case R32G32Sfloat:
      return eR32G32Sfloat;
    case R32G32B32Sfloat:
      return eR32G32B32Sfloat;
    case R32G32B32A32Sfloat:
      return eR32G32B32A32Sfloat;
    case Depth:
      return eD32Sfloat;
    case DepthSfloatStencilUint:
      return eD32SfloatS8Uint;
    case Depth24UnormStencilUint:
      return eD24UnormS8Uint;
    default:
      return eUndefined;
  }
}

vk::SampleCountFlagBits vk_helper::getSampleCount(const pandora::core::ImageSampleCount sample_count) {
  switch (sample_count) {
    using enum pandora::core::ImageSampleCount;
    using enum vk::SampleCountFlagBits;

    case v1:
      return e1;
    case v2:
      return e2;
    case v4:
      return e4;
    case v8:
      return e8;
    case v16:
      return e16;
    case v32:
      return e32;
    case v64:
      return e64;
    default:
      return vk::SampleCountFlagBits{0u};
  }
}

vk::ImageAspectFlags vk_helper::getImageAspectFlags(const pandora::core::ImageAspect image_aspect) {
  vk::ImageSubresourceRange subresource_range;
  switch (image_aspect) {
    using enum pandora::core::ImageAspect;
    using enum vk::ImageAspectFlagBits;

    case Color:
      return eColor;
    case Depth:
      return eDepth;
    case Stencil:
      return eStencil;
    case DepthStencil:
      return eDepth | eStencil;
    default:
      return eColor;
  }
}

vk::ShaderStageFlags vk_helper::getShaderStageFlagBits(const pandora::core::ShaderStage shader_stage) {
  switch (shader_stage) {
    using enum pandora::core::ShaderStage;
    using enum vk::ShaderStageFlagBits;

    case Vertex:
      return eVertex;
    case Fragment:
      return eFragment;
    case Compute:
      return eCompute;
    default:
      return eAll;
  }
}

vk::Extent2D vk_helper::getExtent2D(const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size) {
  return vk::Extent2D(graphical_size.width, graphical_size.height);
}

vk::Extent3D vk_helper::getExtent3D(const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size) {
  return vk::Extent3D(graphical_size.width, graphical_size.height, graphical_size.depth);
}

vk::AttachmentLoadOp vk_helper::getAttachmentLoadOp(const pandora::core::AttachmentLoadOp attachment_load_op) {
  using enum pandora::core::AttachmentLoadOp;
  using enum vk::AttachmentLoadOp;

  switch (attachment_load_op) {
    case Load:
      return eLoad;
    case Clear:
      return eClear;
    case DontCare:
      return eDontCare;
    default:
      return eDontCare;
  }
}

vk::AttachmentStoreOp vk_helper::getAttachmentStoreOp(const pandora::core::AttachmentStoreOp attachment_store_op) {
  switch (attachment_store_op) {
    using enum pandora::core::AttachmentStoreOp;
    using enum vk::AttachmentStoreOp;

    case Store:
      return eStore;
    case DontCare:
      return eDontCare;
    default:
      return eDontCare;
  }
}

// Pipeline-related conversions
vk::CompareOp vk_helper::getCompareOp(const pandora::core::CompareOp compare_op) {
  switch (compare_op) {
    using enum pandora::core::CompareOp;
    using enum vk::CompareOp;

    case Never:
      return eNever;
    case Less:
      return eLess;
    case Equal:
      return eEqual;
    case LessOrEqual:
      return eLessOrEqual;
    case Greater:
      return eGreater;
    case NotEqual:
      return eNotEqual;
    case GreaterOrEqual:
      return eGreaterOrEqual;
    case Always:
      return eAlways;
    default:
      return eAlways;
  }
}

vk::StencilOp vk_helper::getStencilOp(const pandora::core::StencilOp stencil_op) {
  switch (stencil_op) {
    using enum pandora::core::StencilOp;
    using enum vk::StencilOp;

    case Keep:
      return eKeep;
    case Zero:
      return eZero;
    case Replace:
      return eReplace;
    case IncrementAndClamp:
      return eIncrementAndClamp;
    case DecrementAndClamp:
      return eDecrementAndClamp;
    case Invert:
      return eInvert;
    case IncrementAndWrap:
      return eIncrementAndWrap;
    case DecrementAndWrap:
      return eDecrementAndWrap;
    default:
      return eKeep;
  }
}

vk::StencilOpState vk_helper::getStencilOpState(const pandora::core::StencilOpState& state) {
  vk::StencilOpState stencil_op_state{};
  stencil_op_state.setFailOp(getStencilOp(state.fail_op));
  stencil_op_state.setPassOp(getStencilOp(state.pass_op));
  stencil_op_state.setDepthFailOp(getStencilOp(state.depth_fail_op));
  stencil_op_state.setCompareOp(getCompareOp(state.compare_op));
  stencil_op_state.setCompareMask(state.compare_mask);
  stencil_op_state.setWriteMask(state.write_mask);
  stencil_op_state.setReference(state.reference);

  return stencil_op_state;
}

vk::BlendFactor vk_helper::getBlendFactor(const pandora::core::BlendFactor blend_factor) {
  switch (blend_factor) {
    using enum pandora::core::BlendFactor;
    using enum vk::BlendFactor;

    case Zero:
      return eZero;
    case One:
      return eOne;
    case SrcColor:
      return eSrcColor;
    case OneMinusSrcColor:
      return eOneMinusSrcColor;
    case DstColor:
      return eDstColor;
    case OneMinusDstColor:
      return eOneMinusDstColor;
    case SrcAlpha:
      return eSrcAlpha;
    case OneMinusSrcAlpha:
      return eOneMinusSrcAlpha;
    case DstAlpha:
      return eDstAlpha;
    case OneMinusDstAlpha:
      return eOneMinusDstAlpha;
    case ConstantColor:
      return eConstantColor;
    case OneMinusConstantColor:
      return eOneMinusConstantColor;
    case ConstantAlpha:
      return eConstantAlpha;
    case OneMinusConstantAlpha:
      return eOneMinusConstantAlpha;
    case SrcAlphaSaturate:
      return eSrcAlphaSaturate;
    case Src1Color:
      return eSrc1Color;
    case OneMinusSrc1Color:
      return eOneMinusSrc1Color;
    case Src1Alpha:
      return eSrc1Alpha;
    case OneMinusSrc1Alpha:
      return eOneMinusSrc1Alpha;
    default:
      return eZero;
  }
}

vk::BlendOp vk_helper::getBlendOp(const pandora::core::BlendOp blend_op) {
  switch (blend_op) {
    using enum pandora::core::BlendOp;
    using enum vk::BlendOp;

    case Add:
      return eAdd;
    case Subtract:
      return eSubtract;
    case ReverseSubtract:
      return eReverseSubtract;
    case Min:
      return eMin;
    case Max:
      return eMax;
    default:
      return eAdd;
  }
}

vk::ColorComponentFlags vk_helper::getColorComponent(
    const std::vector<pandora::core::ColorComponent>& color_components) {
  vk::ColorComponentFlags flags;
  for (const auto& component : color_components) {
    switch (component) {
      using enum pandora::core::ColorComponent;
      using enum vk::ColorComponentFlagBits;

      case R:
        flags |= eR;
        break;
      case G:
        flags |= eG;
        break;
      case B:
        flags |= eB;
        break;
      case A:
        flags |= eA;
        break;
      default:
        break;
    }
  }

  return flags;
}

// Command buffer related conversions
vk::CommandBufferUsageFlagBits vk_helper::getCommandBufferUsageFlagBits(
    const pandora::core::CommandBufferUsage usage_flags) {
  switch (usage_flags) {
    using enum pandora::core::CommandBufferUsage;
    using enum vk::CommandBufferUsageFlagBits;

    case OneTimeSubmit:
      return eOneTimeSubmit;
    case RenderPassContinue:
      return eRenderPassContinue;
    case SimultaneousUse:
      return eSimultaneousUse;
    default:
      return vk::CommandBufferUsageFlagBits{0u};
  }
}

vk::SubpassContents vk_helper::getSubpassContents(const pandora::core::SubpassContents subpass_contents) {
  switch (subpass_contents) {
    using enum pandora::core::SubpassContents;
    using enum vk::SubpassContents;

    case Inline:
      return eInline;
    case SecondaryCommandBuffers:
      return eSecondaryCommandBuffers;
    default:
      return vk::SubpassContents{0u};
  }
}

// Subpass related conversions
vk::DependencyFlagBits vk_helper::getDependencyFlag(const pandora::core::DependencyFlag dependency_flag) {
  switch (dependency_flag) {
    using enum pandora::core::DependencyFlag;
    using enum vk::DependencyFlagBits;

    case ByRegion:
      return eByRegion;
    case DeviceGroup:
      return eDeviceGroupKHR;
    case ViewLocal:
      return eViewLocalKHR;
    case Feedback:
      return eFeedbackLoopEXT;
    default:
      return eByRegion;
  }
}

// Pipeline state conversions
vk::PrimitiveTopology vk_helper::getPrimitiveTopology(const pandora::core::PrimitiveTopology topology) {
  switch (topology) {
    using enum pandora::core::PrimitiveTopology;
    using enum vk::PrimitiveTopology;

    case PointList:
      return ePointList;
    case LineList:
      return eLineList;
    case LineStrip:
      return eLineStrip;
    case TriangleList:
      return eTriangleList;
    case TriangleStrip:
      return eTriangleStrip;
    case TriangleFan:
      return eTriangleFan;
    case LineListWithAdjacency:
      return eLineListWithAdjacency;
    case LineStripWithAdjacency:
      return eLineStripWithAdjacency;
    case TriangleListWithAdjacency:
      return eTriangleListWithAdjacency;
    case TriangleStripWithAdjacency:
      return eTriangleStripWithAdjacency;
    case PatchList:
      return ePatchList;
    default:
      return eTriangleList;
  }
}

vk::PolygonMode vk_helper::getPolygonMode(const pandora::core::PolygonMode polygon_mode) {
  switch (polygon_mode) {
    using enum pandora::core::PolygonMode;
    using enum vk::PolygonMode;

    case Fill:
      return eFill;
    case Line:
      return eLine;
    case Point:
      return ePoint;
    default:
      return eFill;
  }
}

vk::CullModeFlags vk_helper::getCullMode(const pandora::core::CullMode cull_mode) {
  switch (cull_mode) {
    using enum pandora::core::CullMode;
    using enum vk::CullModeFlagBits;

    case None:
      return eNone;
    case Front:
      return eFront;
    case Back:
      return eBack;
    case FrontAndBack:
      return eFrontAndBack;
    default:
      return eNone;
  }
}

vk::FrontFace vk_helper::getFrontFace(const pandora::core::FrontFace front_face) {
  switch (front_face) {
    using enum pandora::core::FrontFace;
    using enum vk::FrontFace;

    case Clockwise:
      return eClockwise;
    case CounterClockwise:
      return eCounterClockwise;
    default:
      return eCounterClockwise;
  }
}

vk::LogicOp vk_helper::getLogicOp(const pandora::core::LogicOp logic_op) {
  switch (logic_op) {
    using enum pandora::core::LogicOp;
    using enum vk::LogicOp;

    case Clear:
      return eClear;
    case And:
      return eAnd;
    case AndReverse:
      return eAndReverse;
    case Copy:
      return eCopy;
    case AndInverted:
      return eAndInverted;
    case NoOp:
      return eNoOp;
    case Xor:
      return eXor;
    case Or:
      return eOr;
    case Nor:
      return eNor;
    case Equivalent:
      return eEquivalent;
    case Invert:
      return eInvert;
    case OrReverse:
      return eOrReverse;
    case CopyInverted:
      return eCopyInverted;
    case OrInverted:
      return eOrInverted;
    case Nand:
      return eNand;
    case Set:
      return eSet;
    default:
      return eCopy;
  }
}

vk::DynamicState vk_helper::getDynamicState(const pandora::core::DynamicOption option) {
  switch (option) {
    using enum pandora::core::DynamicOption;
    using enum vk::DynamicState;

    case Viewport:
      return eViewport;
    case Scissor:
      return eScissor;
    case LineWidth:
      return eLineWidth;
    case DepthBias:
      return eDepthBias;
    case BlendConstants:
      return eBlendConstants;
    case DepthBounds:
      return eDepthBounds;
    case StencilCompareMask:
      return eStencilCompareMask;
    case StencilWriteMask:
      return eStencilWriteMask;
    case StencilReference:
      return eStencilReference;
    default:
      return eViewport;
  }
}

vk::VertexInputRate vk_helper::getVertexInputRate(const pandora::core::VertexInputRate input_rate) {
  switch (input_rate) {
    using enum pandora::core::VertexInputRate;
    using enum vk::VertexInputRate;

    case Vertex:
      return eVertex;
    case Instance:
      return eInstance;
    default:
      return eVertex;
  }
}

// Sampler related conversions
vk::Filter vk_helper::getSamplerFilter(const pandora::core::SamplerFilter filter) {
  switch (filter) {
    using enum pandora::core::SamplerFilter;
    using enum vk::Filter;

    case Linear:
      return eLinear;
    case Nearest:
      return eNearest;
    case Cubic:
      return eCubicIMG;
    default:
      return eLinear;
  }
}

vk::SamplerMipmapMode vk_helper::getSamplerMipmapMode(const pandora::core::SamplerMipmapMode mipmap_mode) {
  switch (mipmap_mode) {
    using enum pandora::core::SamplerMipmapMode;
    using enum vk::SamplerMipmapMode;

    case Linear:
      return eLinear;
    case Nearest:
      return eNearest;
    default:
      return eLinear;
  }
}

vk::SamplerAddressMode vk_helper::getSamplerAddressMode(const pandora::core::SamplerAddressMode address_mode) {
  switch (address_mode) {
    using enum pandora::core::SamplerAddressMode;
    using enum vk::SamplerAddressMode;

    case Repeat:
      return eRepeat;
    case MirroredRepeat:
      return eMirroredRepeat;
    case ClampToEdge:
      return eClampToEdge;
    case ClampToBorder:
      return eClampToBorder;
    case MirrorClampToEdge:
      return eMirrorClampToEdge;
    default:
      return eRepeat;
  }
}

vk::CompareOp vk_helper::getSamplerCompareOp(const pandora::core::SamplerCompareOp compare_op) {
  switch (compare_op) {
    using enum pandora::core::SamplerCompareOp;
    using enum vk::CompareOp;

    case Never:
      return eNever;
    case Less:
      return eLess;
    case Equal:
      return eEqual;
    case LessOrEqual:
      return eLessOrEqual;
    case Greater:
      return eGreater;
    case NotEqual:
      return eNotEqual;
    case GreaterOrEqual:
      return eGreaterOrEqual;
    case Always:
      return eAlways;
    default:
      return eLess;
  }
}

vk::BorderColor vk_helper::getSamplerBorderColor(const pandora::core::SamplerBorderColor border_color) {
  switch (border_color) {
    using enum pandora::core::SamplerBorderColor;
    using enum vk::BorderColor;

    case FloatTransparentBlack:
      return eFloatTransparentBlack;
    case IntTransparentBlack:
      return eIntTransparentBlack;
    case FloatOpaqueBlack:
      return eFloatOpaqueBlack;
    case IntOpaqueBlack:
      return eIntOpaqueBlack;
    case FloatOpaqueWhite:
      return eFloatOpaqueWhite;
    case IntOpaqueWhite:
      return eIntOpaqueWhite;
    default:
      return eFloatTransparentBlack;
  }
}
