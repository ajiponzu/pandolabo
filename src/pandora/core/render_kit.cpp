#include "pandora/core/gpu.hpp"
#include "pandora/core/renderpass.hpp"

namespace pandora::core {

RenderKit::RenderKit(const gpu::Context& context,
                     AttachmentList& attachment_list,
                     const SubpassGraph& subpass_graph,
                     const gpu_ui::GraphicalSize<uint32_t>& size,
                     bool is_presented) {
  m_ptrRenderpass =
      std::make_unique<Renderpass>(context, attachment_list, subpass_graph);

  resetFramebuffer(context, attachment_list, size, is_presented);
}

void RenderKit::resetFramebuffer(const gpu::Context& context,
                                 AttachmentList& attachment_list,
                                 const gpu_ui::GraphicalSize<uint32_t>& size,
                                 bool is_presented) {
  m_framebuffers.clear();

  if (is_presented) {
    const auto backbuffer_count = context.getPtrSwapchain()->getImageCount();
    for (size_t index = 0u; index < backbuffer_count; index += 1u) {
      attachment_list.setBackbufferAttachment(context, index);

      m_framebuffers.emplace_back(
          context, *m_ptrRenderpass, size, attachment_list);
    }
  } else {
    m_framebuffers.emplace_back(
        context, *m_ptrRenderpass, size, attachment_list);
  }

  m_clearValues = attachment_list.getClearValues();
}

}  // namespace pandora::core
