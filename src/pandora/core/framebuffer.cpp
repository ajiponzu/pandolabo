#include "pandora/core/gpu.hpp"
#include "pandora/core/renderpass.hpp"

namespace pandora::core {

Framebuffer::Framebuffer(const gpu::Context& ptr_context,
                         const Renderpass& render_pass,
                         const gpu_ui::GraphicalSize<uint32_t>& size,
                         const AttachmentList& attachments) {
  m_ptrFramebuffer = ptr_context.getPtrDevice()
                         ->getPtrLogicalDevice()
                         ->createFramebufferUnique(
                             vk::FramebufferCreateInfo()
                                 .setRenderPass(render_pass.getRenderPass())
                                 .setWidth(size.width)
                                 .setHeight(size.height)
                                 .setLayers(1u)
                                 .setAttachments(attachments.getAttachments()));
}

Framebuffer::~Framebuffer() {}

}  // namespace pandora::core
