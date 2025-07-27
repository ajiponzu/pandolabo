#include "pandora/core/renderpass.hpp"

pandora::core::RenderKit::RenderKit(const std::unique_ptr<gpu::Context>& ptr_context,
                                    AttachmentList& attachment_list,
                                    const SubpassGraph& subpass_graph,
                                    const gpu_ui::GraphicalSize<uint32_t>& size,
                                    const bool is_presented) {
  m_ptrRenderpass = std::make_unique<Renderpass>(ptr_context, attachment_list, subpass_graph);

  resetFramebuffer(ptr_context, attachment_list, size, is_presented);
}

void pandora::core::RenderKit::resetFramebuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                                                AttachmentList& attachment_list,
                                                const gpu_ui::GraphicalSize<uint32_t>& size,
                                                const bool is_presented) {
  m_framebuffers.clear();

  if (is_presented) {
    const auto backbuffer_count = ptr_context->getPtrSwapchain()->getImageCount();
    for (size_t index = 0U; index < backbuffer_count; index += 1U) {
      attachment_list.setBackbufferAttachment(ptr_context, index);

      m_framebuffers.emplace_back(ptr_context, *m_ptrRenderpass, size, attachment_list);
    }
  } else {
    m_framebuffers.emplace_back(ptr_context, *m_ptrRenderpass, size, attachment_list);
  }

  m_clearValues = attachment_list.getClearValues();
}
