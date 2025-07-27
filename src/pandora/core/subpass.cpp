#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/renderpass.hpp"

pandora::core::SubpassNode::SubpassNode(const PipelineBind bind_point, const uint32_t view_mask) {
  m_bindPoint = vk_helper::getPipelineBindPoint(bind_point);
  m_viewMask = view_mask;
}

pandora::core::SubpassNode::~SubpassNode() {}

vk::AttachmentReference pandora::core::SubpassNode::convert(const AttachmentReference& attachment_ref) {
  using vk_helper::getImageLayout;

  vk::AttachmentReference reference;
  reference.setAttachment(attachment_ref.index);
  reference.setLayout(getImageLayout(attachment_ref.layout));

  return reference;
}

pandora::core::SubpassGraph::SubpassGraph() {}

pandora::core::SubpassGraph::~SubpassGraph() {}

uint32_t pandora::core::SubpassGraph::appendNode(const SubpassNode& node) {
  m_nodes.push_back(node);

  {
    const auto& subpass_node = m_nodes.back();

    vk::SubpassDescription description;
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

  return static_cast<uint32_t>(m_descriptions.size()) - 1U;
}

void pandora::core::SubpassGraph::appendEdge(const SubpassEdge& edge) {
  using namespace vk_helper;  // ローカルスコープでusing宣言

  vk::SubpassDependency dependency;
  dependency.setSrcSubpass(edge.src_index);
  dependency.setDstSubpass(edge.dst_index);

  for (const auto& stage : edge.src_stages) {
    dependency.srcStageMask |= getPipelineStageFlagBits(stage);
  }
  for (const auto& stage : edge.dst_stages) {
    dependency.dstStageMask |= getPipelineStageFlagBits(stage);
  }

  for (const auto& access : edge.src_access) {
    dependency.srcAccessMask |= getAccessFlagBits(access);
  }
  for (const auto& access : edge.dst_access) {
    dependency.dstAccessMask |= getAccessFlagBits(access);
  }

  dependency.setDependencyFlags(getDependencyFlag(edge.dependency_flag));

  m_dependencies.push_back(dependency);
}
