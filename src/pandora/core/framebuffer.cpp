#include "pandora/core/renderpass.hpp"

pandora::core::Framebuffer::Framebuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                                        const Renderpass& render_pass,
                                        const gpu_ui::GraphicalSize<uint32_t>& size,
                                        const AttachmentList& attachments) {
  vk::FramebufferCreateInfo framebuffer_info;
  framebuffer_info.setRenderPass(render_pass.getRenderPass());
  framebuffer_info.setWidth(size.width);
  framebuffer_info.setHeight(size.height);
  framebuffer_info.setLayers(1U);
  framebuffer_info.setAttachments(attachments.getAttachments());

  m_ptrFramebuffer = ptr_context->getPtrDevice()->getPtrLogicalDevice()->createFramebufferUnique(framebuffer_info);
}

pandora::core::Framebuffer::~Framebuffer() {}
