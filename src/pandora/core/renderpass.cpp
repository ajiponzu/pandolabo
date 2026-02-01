#include "pandora/core/renderpass.hpp"

#include "pandora/core/gpu/vk_helper.hpp"

namespace pandora::core {

Renderpass::Renderpass(const gpu::Context& context,
                       const AttachmentList& attachment_list,
                       const SubpassGraph& subpass_graph) {
  m_ptrRenderPass =
      context.getPtrDevice()->getPtrLogicalDevice()->createRenderPassUnique(
          vk::RenderPassCreateInfo()
              .setAttachments(attachment_list.getDescriptions())
              .setSubpasses(subpass_graph.getDescriptions())
              .setDependencies(subpass_graph.getDependencies()));
}

Renderpass::~Renderpass() {}

}  // namespace pandora::core
