#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/renderpass.hpp"

pandora::core::AttachmentList::AttachmentList() {}

pandora::core::AttachmentList::~AttachmentList() {}

uint32_t pandora::core::AttachmentList::append(const AttachmentDescription& description,
                                               const gpu::ImageView& image_view,
                                               const ClearColor& clear_value) {
  m_clearValues.emplace_back(vk::ClearColorValue(clear_value.color));

  return append(description, image_view);
}

uint32_t pandora::core::AttachmentList::append(const AttachmentDescription& description,
                                               const gpu::ImageView& image_view,
                                               const ClearDepthStencil& clear_value) {
  m_clearValues.emplace_back(vk::ClearDepthStencilValue(clear_value.depth, clear_value.stencil));

  return append(description, image_view);
}

uint32_t pandora::core::AttachmentList::append(const AttachmentDescription& description,
                                               const ClearColor& clear_value) {
  m_clearValues.push_back(vk::ClearColorValue(clear_value.color));

  appendDescription(description);

  m_attachments.push_back(vk::ImageView());

  return static_cast<uint32_t>(m_attachments.size()) - 1u;
}

void pandora::core::AttachmentList::setBackbufferAttachment(const std::unique_ptr<gpu::Context>& ptr_context,
                                                            const size_t index) {
  m_attachments[m_backbufferIndex] = ptr_context->getPtrSwapchain()->getImageViews().at(index).get();
}

uint32_t pandora::core::AttachmentList::append(const AttachmentDescription& description,
                                               const gpu::ImageView& image_view) {
  appendDescription(description);

  m_attachments.push_back(image_view.getImageView());

  return static_cast<uint32_t>(m_attachments.size()) - 1u;
}

const void pandora::core::AttachmentList::appendDescription(const AttachmentDescription& description) {
  using namespace vk_helper;

  const auto vk_description = vk::AttachmentDescription{}
                                  .setFormat(getFormat(description.format))
                                  .setSamples(getSampleCount(description.samples))
                                  .setLoadOp(getAttachmentLoadOp(description.load_op))
                                  .setStoreOp(getAttachmentStoreOp(description.store_op))
                                  .setStencilLoadOp(getAttachmentLoadOp(description.stencil_load_op))
                                  .setStencilStoreOp(getAttachmentStoreOp(description.stencil_store_op))
                                  .setInitialLayout(getImageLayout(description.initial_layout))
                                  .setFinalLayout(getImageLayout(description.final_layout));

  m_descriptions.push_back(vk_description);
}
