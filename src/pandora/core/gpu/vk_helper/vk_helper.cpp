#include "pandora/core/gpu/vk_helper.hpp"

vk::MemoryPropertyFlags vk_helper::getMemoryPropertyFlags(const pandora::core::MemoryUsage memory_usage) {
  using MemoryUsage = pandora::core::MemoryUsage;

  switch (memory_usage) {
    case MemoryUsage::GpuOnly:
      return vk::MemoryPropertyFlagBits::eDeviceLocal;
    case MemoryUsage::CpuOnly:
      return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    case MemoryUsage::CpuToGpu:
      return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eDeviceLocal;
    case MemoryUsage::GpuToCpu:
      return vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
    default:
      return vk::MemoryPropertyFlagBits::eDeviceLocal;
  }
}

vk::AccessFlagBits vk_helper::getAccessFlagBits(const pandora::core::AccessFlag access_flag) {
  using AccessFlag = pandora::core::AccessFlag;

  switch (access_flag) {
    case AccessFlag::IndirectCommandRead:
      return vk::AccessFlagBits::eIndirectCommandRead;
    case AccessFlag::IndexRead:
      return vk::AccessFlagBits::eIndexRead;
    case AccessFlag::VertexAttributeRead:
      return vk::AccessFlagBits::eVertexAttributeRead;
    case AccessFlag::UniformRead:
      return vk::AccessFlagBits::eUniformRead;
    case AccessFlag::InputAttachmentRead:
      return vk::AccessFlagBits::eInputAttachmentRead;
    case AccessFlag::ShaderRead:
      return vk::AccessFlagBits::eShaderRead;
    case AccessFlag::ShaderWrite:
      return vk::AccessFlagBits::eShaderWrite;
    case AccessFlag::ColorAttachmentRead:
      return vk::AccessFlagBits::eColorAttachmentRead;
    case AccessFlag::ColorAttachmentWrite:
      return vk::AccessFlagBits::eColorAttachmentWrite;
    case AccessFlag::DepthStencilAttachmentRead:
      return vk::AccessFlagBits::eDepthStencilAttachmentRead;
    case AccessFlag::DepthStencilAttachmentWrite:
      return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    case AccessFlag::TransferRead:
      return vk::AccessFlagBits::eTransferRead;
    case AccessFlag::TransferWrite:
      return vk::AccessFlagBits::eTransferWrite;
    case AccessFlag::HostRead:
      return vk::AccessFlagBits::eHostRead;
    case AccessFlag::HostWrite:
      return vk::AccessFlagBits::eHostWrite;
    case AccessFlag::MemoryRead:
      return vk::AccessFlagBits::eMemoryRead;
    case AccessFlag::MemoryWrite:
      return vk::AccessFlagBits::eMemoryWrite;
    default:
      return vk::AccessFlagBits::eNone;
  }
}

vk::PipelineStageFlagBits vk_helper::getPipelineStageFlagBits(const pandora::core::PipelineStage stage) {
  using PipelineStage = pandora::core::PipelineStage;

  switch (stage) {
    case PipelineStage::TopOfPipe:
      return vk::PipelineStageFlagBits::eTopOfPipe;
    case PipelineStage::DrawIndirect:
      return vk::PipelineStageFlagBits::eDrawIndirect;
    case PipelineStage::VertexInput:
      return vk::PipelineStageFlagBits::eVertexInput;
    case PipelineStage::VertexShader:
      return vk::PipelineStageFlagBits::eVertexShader;
    case PipelineStage::TessellationControlShader:
      return vk::PipelineStageFlagBits::eTessellationControlShader;
    case PipelineStage::TessellationEvaluationShader:
      return vk::PipelineStageFlagBits::eTessellationEvaluationShader;
    case PipelineStage::GeometryShader:
      return vk::PipelineStageFlagBits::eGeometryShader;
    case PipelineStage::FragmentShader:
      return vk::PipelineStageFlagBits::eFragmentShader;
    case PipelineStage::EarlyFragmentTests:
      return vk::PipelineStageFlagBits::eEarlyFragmentTests;
    case PipelineStage::LateFragmentTests:
      return vk::PipelineStageFlagBits::eLateFragmentTests;
    case PipelineStage::ColorAttachmentOutput:
      return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case PipelineStage::ComputeShader:
      return vk::PipelineStageFlagBits::eComputeShader;
    case PipelineStage::Transfer:
      return vk::PipelineStageFlagBits::eTransfer;
    case PipelineStage::BottomOfPipe:
      return vk::PipelineStageFlagBits::eBottomOfPipe;
    case PipelineStage::Host:
      return vk::PipelineStageFlagBits::eHost;
    case PipelineStage::AllGraphics:
      return vk::PipelineStageFlagBits::eAllGraphics;
    case PipelineStage::AllCommands:
      return vk::PipelineStageFlagBits::eAllCommands;
    default:
      return vk::PipelineStageFlagBits::eNone;
  }
}

vk::PipelineBindPoint vk_helper::getPipelineBindPoint(const pandora::core::PipelineBind bind_point) {
  using PipelineBind = pandora::core::PipelineBind;

  switch (bind_point) {
    case PipelineBind::Graphics:
      return vk::PipelineBindPoint::eGraphics;
    case PipelineBind::Compute:
      return vk::PipelineBindPoint::eCompute;
    case PipelineBind::RayTracing:
      return vk::PipelineBindPoint::eRayTracingKHR;
    default:
      return vk::PipelineBindPoint::eGraphics;
  }
}

vk::ImageUsageFlagBits vk_helper::getImageUsage(const pandora::core::ImageUsage image_usage) {
  using ImageUsage = pandora::core::ImageUsage;

  switch (image_usage) {
    case ImageUsage::Sampled:
      return vk::ImageUsageFlagBits::eSampled;
    case ImageUsage::Storage:
      return vk::ImageUsageFlagBits::eStorage;
    case ImageUsage::ColorAttachment:
      return vk::ImageUsageFlagBits::eColorAttachment;
    case ImageUsage::DepthStencilAttachment:
      return vk::ImageUsageFlagBits::eDepthStencilAttachment;
    case ImageUsage::TransientAttachment:
      return vk::ImageUsageFlagBits::eTransientAttachment;
    case ImageUsage::InputAttachment:
      return vk::ImageUsageFlagBits::eInputAttachment;
    default:
      return vk::ImageUsageFlagBits::eSampled;
  }
}

vk::ImageLayout vk_helper::getImageLayout(const pandora::core::ImageLayout image_layout) {
  using ImageLayout = pandora::core::ImageLayout;

  switch (image_layout) {
    case ImageLayout::Undefined:
      return vk::ImageLayout::eUndefined;
    case ImageLayout::General:
      return vk::ImageLayout::eGeneral;
    case ImageLayout::ColorAttachmentOptimal:
      return vk::ImageLayout::eColorAttachmentOptimal;
    case ImageLayout::DepthStencilAttachmentOptimal:
      return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    case ImageLayout::DepthStencilReadOnlyOptimal:
      return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    case ImageLayout::ShaderReadOnlyOptimal:
      return vk::ImageLayout::eShaderReadOnlyOptimal;
    case ImageLayout::TransferSrcOptimal:
      return vk::ImageLayout::eTransferSrcOptimal;
    case ImageLayout::TransferDstOptimal:
      return vk::ImageLayout::eTransferDstOptimal;
    case ImageLayout::Preinitialized:
      return vk::ImageLayout::ePreinitialized;
    case ImageLayout::PresentSrc:
      return vk::ImageLayout::ePresentSrcKHR;
    case ImageLayout::SharedPresent:
      return vk::ImageLayout::eSharedPresentKHR;
    case ImageLayout::DepthReadOnlyStencilAttachmentOptimal:
      return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
    case ImageLayout::DepthAttachmentStencilReadOnlyOptimal:
      return vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
    default:
      return vk::ImageLayout::eUndefined;
  }
}

vk::Format vk_helper::getFormat(const pandora::core::DataFormat format) {
  using DataFormat = pandora::core::DataFormat;

  switch (format) {
    case DataFormat::R8Unorm:
      return vk::Format::eR8Unorm;
    case DataFormat::R8Snorm:
      return vk::Format::eR8Snorm;
    case DataFormat::R8Uscaled:
      return vk::Format::eR8Uscaled;
    case DataFormat::R8Sscaled:
      return vk::Format::eR8Sscaled;
    case DataFormat::R8Uint:
      return vk::Format::eR8Uint;
    case DataFormat::R8Sint:
      return vk::Format::eR8Sint;
    case DataFormat::R8Srgb:
      return vk::Format::eR8Srgb;
    case DataFormat::R8G8Unorm:
      return vk::Format::eR8G8Unorm;
    case DataFormat::R8G8Snorm:
      return vk::Format::eR8G8Snorm;
    case DataFormat::R8G8Uscaled:
      return vk::Format::eR8G8Uscaled;
    case DataFormat::R8G8Sscaled:
      return vk::Format::eR8G8Sscaled;
    case DataFormat::R8G8Uint:
      return vk::Format::eR8G8Uint;
    case DataFormat::R8G8Sint:
      return vk::Format::eR8G8Sint;
    case DataFormat::R8G8Srgb:
      return vk::Format::eR8G8Srgb;
    case DataFormat::R8G8B8Unorm:
      return vk::Format::eR8G8B8Unorm;
    case DataFormat::R8G8B8Snorm:
      return vk::Format::eR8G8B8Snorm;
    case DataFormat::R8G8B8Uscaled:
      return vk::Format::eR8G8B8Uscaled;
    case DataFormat::R8G8B8Sscaled:
      return vk::Format::eR8G8B8Sscaled;
    case DataFormat::R8G8B8Uint:
      return vk::Format::eR8G8B8Uint;
    case DataFormat::R8G8B8Sint:
      return vk::Format::eR8G8B8Sint;
    case DataFormat::R8G8B8Srgb:
      return vk::Format::eR8G8B8Srgb;
    case DataFormat::R8G8B8A8Unorm:
      return vk::Format::eR8G8B8A8Unorm;
    case DataFormat::R8G8B8A8Snorm:
      return vk::Format::eR8G8B8A8Snorm;
    case DataFormat::R8G8B8A8Uscaled:
      return vk::Format::eR8G8B8A8Uscaled;
    case DataFormat::R8G8B8A8Sscaled:
      return vk::Format::eR8G8B8A8Sscaled;
    case DataFormat::R8G8B8A8Uint:
      return vk::Format::eR8G8B8A8Uint;
    case DataFormat::R8G8B8A8Sint:
      return vk::Format::eR8G8B8A8Sint;
    case DataFormat::R8G8B8A8Srgb:
      return vk::Format::eR8G8B8A8Srgb;
    case DataFormat::R32Sfloat:
      return vk::Format::eR32Sfloat;
    case DataFormat::R32G32Sfloat:
      return vk::Format::eR32G32Sfloat;
    case DataFormat::R32G32B32Sfloat:
      return vk::Format::eR32G32B32Sfloat;
    case DataFormat::R32G32B32A32Sfloat:
      return vk::Format::eR32G32B32A32Sfloat;
    case DataFormat::Depth:
      return vk::Format::eD32Sfloat;
    case DataFormat::DepthSfloatStencilUint:
      return vk::Format::eD32SfloatS8Uint;
    case DataFormat::Depth24UnormStencilUint:
      return vk::Format::eD24UnormS8Uint;
    default:
      return vk::Format::eUndefined;
  }
}

vk::SampleCountFlagBits vk_helper::getSampleCount(const pandora::core::ImageSampleCount sample_count) {
  using ImageSampleCount = pandora::core::ImageSampleCount;

  switch (sample_count) {
    case ImageSampleCount::v1:
      return vk::SampleCountFlagBits::e1;
    case ImageSampleCount::v2:
      return vk::SampleCountFlagBits::e2;
    case ImageSampleCount::v4:
      return vk::SampleCountFlagBits::e4;
    case ImageSampleCount::v8:
      return vk::SampleCountFlagBits::e8;
    case ImageSampleCount::v16:
      return vk::SampleCountFlagBits::e16;
    case ImageSampleCount::v32:
      return vk::SampleCountFlagBits::e32;
    case ImageSampleCount::v64:
      return vk::SampleCountFlagBits::e64;
    default:
      return vk::SampleCountFlagBits(0U);
  }
}

vk::ImageAspectFlags vk_helper::getImageAspectFlags(const pandora::core::ImageAspect image_aspect) {
  using ImageAspect = pandora::core::ImageAspect;

  vk::ImageSubresourceRange subresource_range;
  switch (image_aspect) {
    case ImageAspect::Color:
      return vk::ImageAspectFlagBits::eColor;
    case ImageAspect::Depth:
      return vk::ImageAspectFlagBits::eDepth;
    case ImageAspect::Stencil:
      return vk::ImageAspectFlagBits::eStencil;
    case ImageAspect::DepthStencil:
      return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    default:
      return vk::ImageAspectFlagBits::eColor;
  }
}

vk::ShaderStageFlags vk_helper::getShaderStageFlagBits(const pandora::core::ShaderStage shader_stage) {
  using ShaderStage = pandora::core::ShaderStage;

  switch (shader_stage) {
    case ShaderStage::Vertex:
      return vk::ShaderStageFlagBits::eVertex;
    case ShaderStage::Fragment:
      return vk::ShaderStageFlagBits::eFragment;
    case ShaderStage::Compute:
      return vk::ShaderStageFlagBits::eCompute;
    default:
      return vk::ShaderStageFlagBits::eAll;
  }
}

vk::Extent2D vk_helper::getExtent2D(const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size) {
  return vk::Extent2D(graphical_size.width, graphical_size.height);
}

vk::Extent3D vk_helper::getExtent3D(const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size) {
  return vk::Extent3D(graphical_size.width, graphical_size.height, graphical_size.depth);
}

vk::AttachmentLoadOp vk_helper::getAttachmentLoadOp(const pandora::core::AttachmentLoadOp attachment_load_op) {
  using AttachmentLoadOp = pandora::core::AttachmentLoadOp;

  switch (attachment_load_op) {
    case AttachmentLoadOp::Load:
      return vk::AttachmentLoadOp::eLoad;
    case AttachmentLoadOp::Clear:
      return vk::AttachmentLoadOp::eClear;
    case AttachmentLoadOp::DontCare:
      return vk::AttachmentLoadOp::eDontCare;
    default:
      return vk::AttachmentLoadOp::eDontCare;
  }
}

vk::AttachmentStoreOp vk_helper::getAttachmentStoreOp(const pandora::core::AttachmentStoreOp attachment_store_op) {
  using AttachmentStoreOp = pandora::core::AttachmentStoreOp;

  switch (attachment_store_op) {
    case AttachmentStoreOp::Store:
      return vk::AttachmentStoreOp::eStore;
    case AttachmentStoreOp::DontCare:
      return vk::AttachmentStoreOp::eDontCare;
    default:
      return vk::AttachmentStoreOp::eDontCare;
  }
}

// Pipeline-related conversions
vk::CompareOp vk_helper::getCompareOp(const pandora::core::CompareOp compare_op) {
  switch (compare_op) {
    case pandora::core::CompareOp::Never:
      return vk::CompareOp::eNever;
    case pandora::core::CompareOp::Less:
      return vk::CompareOp::eLess;
    case pandora::core::CompareOp::Equal:
      return vk::CompareOp::eEqual;
    case pandora::core::CompareOp::LessOrEqual:
      return vk::CompareOp::eLessOrEqual;
    case pandora::core::CompareOp::Greater:
      return vk::CompareOp::eGreater;
    case pandora::core::CompareOp::NotEqual:
      return vk::CompareOp::eNotEqual;
    case pandora::core::CompareOp::GreaterOrEqual:
      return vk::CompareOp::eGreaterOrEqual;
    case pandora::core::CompareOp::Always:
      return vk::CompareOp::eAlways;
    default:
      return vk::CompareOp::eAlways;
  }
}

vk::StencilOp vk_helper::getStencilOp(const pandora::core::StencilOp stencil_op) {
  switch (stencil_op) {
    case pandora::core::StencilOp::Keep:
      return vk::StencilOp::eKeep;
    case pandora::core::StencilOp::Zero:
      return vk::StencilOp::eZero;
    case pandora::core::StencilOp::Replace:
      return vk::StencilOp::eReplace;
    case pandora::core::StencilOp::IncrementAndClamp:
      return vk::StencilOp::eIncrementAndClamp;
    case pandora::core::StencilOp::DecrementAndClamp:
      return vk::StencilOp::eDecrementAndClamp;
    case pandora::core::StencilOp::Invert:
      return vk::StencilOp::eInvert;
    case pandora::core::StencilOp::IncrementAndWrap:
      return vk::StencilOp::eIncrementAndWrap;
    case pandora::core::StencilOp::DecrementAndWrap:
      return vk::StencilOp::eDecrementAndWrap;
    default:
      return vk::StencilOp::eKeep;
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
    case pandora::core::BlendFactor::Zero:
      return vk::BlendFactor::eZero;
    case pandora::core::BlendFactor::One:
      return vk::BlendFactor::eOne;
    case pandora::core::BlendFactor::SrcColor:
      return vk::BlendFactor::eSrcColor;
    case pandora::core::BlendFactor::OneMinusSrcColor:
      return vk::BlendFactor::eOneMinusSrcColor;
    case pandora::core::BlendFactor::DstColor:
      return vk::BlendFactor::eDstColor;
    case pandora::core::BlendFactor::OneMinusDstColor:
      return vk::BlendFactor::eOneMinusDstColor;
    case pandora::core::BlendFactor::SrcAlpha:
      return vk::BlendFactor::eSrcAlpha;
    case pandora::core::BlendFactor::OneMinusSrcAlpha:
      return vk::BlendFactor::eOneMinusSrcAlpha;
    case pandora::core::BlendFactor::DstAlpha:
      return vk::BlendFactor::eDstAlpha;
    case pandora::core::BlendFactor::OneMinusDstAlpha:
      return vk::BlendFactor::eOneMinusDstAlpha;
    case pandora::core::BlendFactor::ConstantColor:
      return vk::BlendFactor::eConstantColor;
    case pandora::core::BlendFactor::OneMinusConstantColor:
      return vk::BlendFactor::eOneMinusConstantColor;
    case pandora::core::BlendFactor::ConstantAlpha:
      return vk::BlendFactor::eConstantAlpha;
    case pandora::core::BlendFactor::OneMinusConstantAlpha:
      return vk::BlendFactor::eOneMinusConstantAlpha;
    case pandora::core::BlendFactor::SrcAlphaSaturate:
      return vk::BlendFactor::eSrcAlphaSaturate;
    case pandora::core::BlendFactor::Src1Color:
      return vk::BlendFactor::eSrc1Color;
    case pandora::core::BlendFactor::OneMinusSrc1Color:
      return vk::BlendFactor::eOneMinusSrc1Color;
    case pandora::core::BlendFactor::Src1Alpha:
      return vk::BlendFactor::eSrc1Alpha;
    case pandora::core::BlendFactor::OneMinusSrc1Alpha:
      return vk::BlendFactor::eOneMinusSrc1Alpha;
    default:
      return vk::BlendFactor::eZero;
  }
}

vk::BlendOp vk_helper::getBlendOp(const pandora::core::BlendOp blend_op) {
  switch (blend_op) {
    case pandora::core::BlendOp::Add:
      return vk::BlendOp::eAdd;
    case pandora::core::BlendOp::Subtract:
      return vk::BlendOp::eSubtract;
    case pandora::core::BlendOp::ReverseSubtract:
      return vk::BlendOp::eReverseSubtract;
    case pandora::core::BlendOp::Min:
      return vk::BlendOp::eMin;
    case pandora::core::BlendOp::Max:
      return vk::BlendOp::eMax;
    default:
      return vk::BlendOp::eAdd;
  }
}

vk::ColorComponentFlags vk_helper::getColorComponent(
    const std::vector<pandora::core::ColorComponent>& color_components) {
  vk::ColorComponentFlags flags;
  for (const auto& component : color_components) {
    switch (component) {
      case pandora::core::ColorComponent::R:
        flags |= vk::ColorComponentFlagBits::eR;
        break;
      case pandora::core::ColorComponent::G:
        flags |= vk::ColorComponentFlagBits::eG;
        break;
      case pandora::core::ColorComponent::B:
        flags |= vk::ColorComponentFlagBits::eB;
        break;
      case pandora::core::ColorComponent::A:
        flags |= vk::ColorComponentFlagBits::eA;
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
  using CommandBufferUsage = pandora::core::CommandBufferUsage;

  switch (usage_flags) {
    case CommandBufferUsage::OneTimeSubmit:
      return vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    case CommandBufferUsage::RenderPassContinue:
      return vk::CommandBufferUsageFlagBits::eRenderPassContinue;
    case CommandBufferUsage::SimultaneousUse:
      return vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    default:
      return vk::CommandBufferUsageFlagBits(0U);
  }
}

vk::SubpassContents vk_helper::getSubpassContents(const pandora::core::SubpassContents subpass_contents) {
  using SubpassContents = pandora::core::SubpassContents;

  switch (subpass_contents) {
    case SubpassContents::Inline:
      return vk::SubpassContents::eInline;
    case SubpassContents::SecondaryCommandBuffers:
      return vk::SubpassContents::eSecondaryCommandBuffers;
    default:
      return vk::SubpassContents(0U);
  }
}

// Subpass related conversions
vk::DependencyFlagBits vk_helper::getDependencyFlag(const pandora::core::DependencyFlag dependency_flag) {
  using DependencyFlag = pandora::core::DependencyFlag;

  switch (dependency_flag) {
    case DependencyFlag::ByRegion:
      return vk::DependencyFlagBits::eByRegion;
    case DependencyFlag::DeviceGroup:
      return vk::DependencyFlagBits::eDeviceGroupKHR;
    case DependencyFlag::ViewLocal:
      return vk::DependencyFlagBits::eViewLocalKHR;
    case DependencyFlag::Feedback:
      return vk::DependencyFlagBits::eFeedbackLoopEXT;
    default:
      return vk::DependencyFlagBits::eByRegion;
  }
}

// Pipeline state conversions
vk::PrimitiveTopology vk_helper::getPrimitiveTopology(const pandora::core::PrimitiveTopology topology) {
  using PrimitiveTopology = pandora::core::PrimitiveTopology;

  switch (topology) {
    case PrimitiveTopology::PointList:
      return vk::PrimitiveTopology::ePointList;
    case PrimitiveTopology::LineList:
      return vk::PrimitiveTopology::eLineList;
    case PrimitiveTopology::LineStrip:
      return vk::PrimitiveTopology::eLineStrip;
    case PrimitiveTopology::TriangleList:
      return vk::PrimitiveTopology::eTriangleList;
    case PrimitiveTopology::TriangleStrip:
      return vk::PrimitiveTopology::eTriangleStrip;
    case PrimitiveTopology::TriangleFan:
      return vk::PrimitiveTopology::eTriangleFan;
    case PrimitiveTopology::LineListWithAdjacency:
      return vk::PrimitiveTopology::eLineListWithAdjacency;
    case PrimitiveTopology::LineStripWithAdjacency:
      return vk::PrimitiveTopology::eLineStripWithAdjacency;
    case PrimitiveTopology::TriangleListWithAdjacency:
      return vk::PrimitiveTopology::eTriangleListWithAdjacency;
    case PrimitiveTopology::TriangleStripWithAdjacency:
      return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
    case PrimitiveTopology::PatchList:
      return vk::PrimitiveTopology::ePatchList;
    default:
      return vk::PrimitiveTopology::eTriangleList;
  }
}

vk::PolygonMode vk_helper::getPolygonMode(const pandora::core::PolygonMode polygon_mode) {
  using PolygonMode = pandora::core::PolygonMode;

  switch (polygon_mode) {
    case PolygonMode::Fill:
      return vk::PolygonMode::eFill;
    case PolygonMode::Line:
      return vk::PolygonMode::eLine;
    case PolygonMode::Point:
      return vk::PolygonMode::ePoint;
    default:
      return vk::PolygonMode::eFill;
  }
}

vk::CullModeFlags vk_helper::getCullMode(const pandora::core::CullMode cull_mode) {
  using CullMode = pandora::core::CullMode;

  switch (cull_mode) {
    case CullMode::None:
      return vk::CullModeFlagBits::eNone;
    case CullMode::Front:
      return vk::CullModeFlagBits::eFront;
    case CullMode::Back:
      return vk::CullModeFlagBits::eBack;
    case CullMode::FrontAndBack:
      return vk::CullModeFlagBits::eFrontAndBack;
    default:
      return vk::CullModeFlagBits::eNone;
  }
}

vk::FrontFace vk_helper::getFrontFace(const pandora::core::FrontFace front_face) {
  using FrontFace = pandora::core::FrontFace;

  switch (front_face) {
    case FrontFace::Clockwise:
      return vk::FrontFace::eClockwise;
    case FrontFace::CounterClockwise:
      return vk::FrontFace::eCounterClockwise;
    default:
      return vk::FrontFace::eCounterClockwise;
  }
}

vk::LogicOp vk_helper::getLogicOp(const pandora::core::LogicOp logic_op) {
  using LogicOp = pandora::core::LogicOp;

  switch (logic_op) {
    case LogicOp::Clear:
      return vk::LogicOp::eClear;
    case LogicOp::And:
      return vk::LogicOp::eAnd;
    case LogicOp::AndReverse:
      return vk::LogicOp::eAndReverse;
    case LogicOp::Copy:
      return vk::LogicOp::eCopy;
    case LogicOp::AndInverted:
      return vk::LogicOp::eAndInverted;
    case LogicOp::NoOp:
      return vk::LogicOp::eNoOp;
    case LogicOp::Xor:
      return vk::LogicOp::eXor;
    case LogicOp::Or:
      return vk::LogicOp::eOr;
    case LogicOp::Nor:
      return vk::LogicOp::eNor;
    case LogicOp::Equivalent:
      return vk::LogicOp::eEquivalent;
    case LogicOp::Invert:
      return vk::LogicOp::eInvert;
    case LogicOp::OrReverse:
      return vk::LogicOp::eOrReverse;
    case LogicOp::CopyInverted:
      return vk::LogicOp::eCopyInverted;
    case LogicOp::OrInverted:
      return vk::LogicOp::eOrInverted;
    case LogicOp::Nand:
      return vk::LogicOp::eNand;
    case LogicOp::Set:
      return vk::LogicOp::eSet;
    default:
      return vk::LogicOp::eCopy;
  }
}

vk::DynamicState vk_helper::getDynamicState(const pandora::core::DynamicOption option) {
  using DynamicOption = pandora::core::DynamicOption;

  switch (option) {
    case DynamicOption::Viewport:
      return vk::DynamicState::eViewport;
    case DynamicOption::Scissor:
      return vk::DynamicState::eScissor;
    case DynamicOption::LineWidth:
      return vk::DynamicState::eLineWidth;
    case DynamicOption::DepthBias:
      return vk::DynamicState::eDepthBias;
    case DynamicOption::BlendConstants:
      return vk::DynamicState::eBlendConstants;
    case DynamicOption::DepthBounds:
      return vk::DynamicState::eDepthBounds;
    case DynamicOption::StencilCompareMask:
      return vk::DynamicState::eStencilCompareMask;
    case DynamicOption::StencilWriteMask:
      return vk::DynamicState::eStencilWriteMask;
    case DynamicOption::StencilReference:
      return vk::DynamicState::eStencilReference;
    default:
      return vk::DynamicState::eViewport;
  }
}

vk::VertexInputRate vk_helper::getVertexInputRate(const pandora::core::VertexInputRate input_rate) {
  using VertexInputRate = pandora::core::VertexInputRate;

  switch (input_rate) {
    case VertexInputRate::Vertex:
      return vk::VertexInputRate::eVertex;
    case VertexInputRate::Instance:
      return vk::VertexInputRate::eInstance;
    default:
      return vk::VertexInputRate::eVertex;
  }
}

// Sampler related conversions
vk::Filter vk_helper::getSamplerFilter(const pandora::core::SamplerFilter filter) {
  using SamplerFilter = pandora::core::SamplerFilter;

  switch (filter) {
    case SamplerFilter::Linear:
      return vk::Filter::eLinear;
    case SamplerFilter::Nearest:
      return vk::Filter::eNearest;
    case SamplerFilter::Cubic:
      return vk::Filter::eCubicIMG;
    default:
      return vk::Filter::eLinear;
  }
}

vk::SamplerMipmapMode vk_helper::getSamplerMipmapMode(const pandora::core::SamplerMipmapMode mipmap_mode) {
  using SamplerMipmapMode = pandora::core::SamplerMipmapMode;

  switch (mipmap_mode) {
    case SamplerMipmapMode::Linear:
      return vk::SamplerMipmapMode::eLinear;
    case SamplerMipmapMode::Nearest:
      return vk::SamplerMipmapMode::eNearest;
    default:
      return vk::SamplerMipmapMode::eLinear;
  }
}

vk::SamplerAddressMode vk_helper::getSamplerAddressMode(const pandora::core::SamplerAddressMode address_mode) {
  using SamplerAddressMode = pandora::core::SamplerAddressMode;

  switch (address_mode) {
    case SamplerAddressMode::Repeat:
      return vk::SamplerAddressMode::eRepeat;
    case SamplerAddressMode::MirroredRepeat:
      return vk::SamplerAddressMode::eMirroredRepeat;
    case SamplerAddressMode::ClampToEdge:
      return vk::SamplerAddressMode::eClampToEdge;
    case SamplerAddressMode::ClampToBorder:
      return vk::SamplerAddressMode::eClampToBorder;
    case SamplerAddressMode::MirrorClampToEdge:
      return vk::SamplerAddressMode::eMirrorClampToEdge;
    default:
      return vk::SamplerAddressMode::eRepeat;
  }
}

vk::CompareOp vk_helper::getSamplerCompareOp(const pandora::core::SamplerCompareOp compare_op) {
  using SamplerCompareOp = pandora::core::SamplerCompareOp;

  switch (compare_op) {
    case SamplerCompareOp::Never:
      return vk::CompareOp::eNever;
    case SamplerCompareOp::Less:
      return vk::CompareOp::eLess;
    case SamplerCompareOp::Equal:
      return vk::CompareOp::eEqual;
    case SamplerCompareOp::LessOrEqual:
      return vk::CompareOp::eLessOrEqual;
    case SamplerCompareOp::Greater:
      return vk::CompareOp::eGreater;
    case SamplerCompareOp::NotEqual:
      return vk::CompareOp::eNotEqual;
    case SamplerCompareOp::GreaterOrEqual:
      return vk::CompareOp::eGreaterOrEqual;
    case SamplerCompareOp::Always:
      return vk::CompareOp::eAlways;
    default:
      return vk::CompareOp::eLess;
  }
}

vk::BorderColor vk_helper::getSamplerBorderColor(const pandora::core::SamplerBorderColor border_color) {
  using SamplerBorderColor = pandora::core::SamplerBorderColor;

  switch (border_color) {
    case SamplerBorderColor::FloatTransparentBlack:
      return vk::BorderColor::eFloatTransparentBlack;
    case SamplerBorderColor::IntTransparentBlack:
      return vk::BorderColor::eIntTransparentBlack;
    case SamplerBorderColor::FloatOpaqueBlack:
      return vk::BorderColor::eFloatOpaqueBlack;
    case SamplerBorderColor::IntOpaqueBlack:
      return vk::BorderColor::eIntOpaqueBlack;
    case SamplerBorderColor::FloatOpaqueWhite:
      return vk::BorderColor::eFloatOpaqueWhite;
    case SamplerBorderColor::IntOpaqueWhite:
      return vk::BorderColor::eIntOpaqueWhite;
    default:
      return vk::BorderColor::eFloatTransparentBlack;
  }
}
