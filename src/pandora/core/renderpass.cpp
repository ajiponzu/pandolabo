#include "pandora/core/renderpass.hpp"

#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::Renderpass::Renderpass(const std::unique_ptr<gpu::Context>& ptr_context,
                                      const AttachmentList& attachment_list,
                                      const SubpassGraph& subpass_graph) {
  m_ptrRenderPass = ptr_context->getPtrDevice()->getPtrLogicalDevice()->createRenderPassUnique(vk::RenderPassCreateInfo(
      {}, attachment_list.getDescriptions(), subpass_graph.getDescriptions(), subpass_graph.getDependencies()));
}

pandora::core::Renderpass::~Renderpass() {}
