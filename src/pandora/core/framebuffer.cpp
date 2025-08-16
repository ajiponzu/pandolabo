#include "pandora/core/renderpass.hpp"

pandora::core::Framebuffer::Framebuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                                        const Renderpass& render_pass,
                                        const gpu_ui::GraphicalSize<uint32_t>& size,
                                        const AttachmentList& attachments) {
  m_ptrFramebuffer = ptr_context->getPtrDevice()->getPtrLogicalDevice()->createFramebufferUnique(
      vk::FramebufferCreateInfo()
          .setRenderPass(render_pass.getRenderPass())
          .setWidth(size.width)
          .setHeight(size.height)
          .setLayers(1U)
          .setAttachments(attachments.getAttachments()));
}

pandora::core::Framebuffer::~Framebuffer() {}
