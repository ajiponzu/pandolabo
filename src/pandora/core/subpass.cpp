#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/renderpass.hpp"

namespace {
// Local helpers for render pass (old flag types)
vk::PipelineStageFlagBits getOldPipelineStageFlagBits(
    pandora::core::PipelineStage stage) {
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
      return vk::PipelineStageFlagBits{0u};
  }
}

vk::AccessFlagBits getOldAccessFlagBits(pandora::core::AccessFlag access) {
  switch (access) {
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
      return vk::AccessFlagBits{0u};
  }
}
}  // namespace

namespace pandora::core {

SubpassNode::SubpassNode(const PipelineBind bind_point, uint32_t view_mask) {
  m_bindPoint = vk_helper::getPipelineBindPoint(bind_point);
  m_viewMask = view_mask;
}

SubpassNode::~SubpassNode() {}

vk::AttachmentReference SubpassNode::convert(
    const AttachmentReference& attachment_ref) {
  return vk::AttachmentReference{}
      .setAttachment(attachment_ref.index)
      .setLayout(vk_helper::getImageLayout(attachment_ref.layout));
}

SubpassGraph::SubpassGraph() {}

SubpassGraph::~SubpassGraph() {}

uint32_t SubpassGraph::appendNode(const SubpassNode& node) {
  m_nodes.push_back(node);

  {
    const auto& subpass_node = m_nodes.back();

    vk::SubpassDescription description{};
    description.setPipelineBindPoint(subpass_node.getBindPoint());

    if (!subpass_node.getInputs().empty()) {
      description.setInputAttachments(subpass_node.getInputs());
    }
    if (!subpass_node.getColors().empty()) {
      description.setColorAttachments(subpass_node.getColors());
    }
    if (!subpass_node.getResolves().empty()) {
      description.setResolveAttachments(subpass_node.getResolves());
    }
    if (!subpass_node.getPreserves().empty()) {
      description.setPreserveAttachments(subpass_node.getPreserves());
    }

    m_descriptions.push_back(description);
  }

  return static_cast<uint32_t>(m_descriptions.size()) - 1u;
}

void SubpassGraph::appendEdge(const SubpassEdge& edge) {
  using namespace vk_helper;

  vk::SubpassDependency dependency{};
  dependency.setSrcSubpass(edge.src_index).setDstSubpass(edge.dst_index);

  for (const auto& stage : edge.src_stages) {
    dependency.srcStageMask |= getOldPipelineStageFlagBits(stage);
  }
  for (const auto& stage : edge.dst_stages) {
    dependency.dstStageMask |= getOldPipelineStageFlagBits(stage);
  }

  for (const auto& access : edge.src_access) {
    dependency.srcAccessMask |= getOldAccessFlagBits(access);
  }
  for (const auto& access : edge.dst_access) {
    dependency.dstAccessMask |= getOldAccessFlagBits(access);
  }

  dependency.setDependencyFlags(getDependencyFlag(edge.dependency_flag));

  m_dependencies.push_back(dependency);
}

}  // namespace pandora::core
