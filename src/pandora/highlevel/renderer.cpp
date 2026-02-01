#include "pandora/highlevel/renderer.hpp"

namespace pandora::highlevel {

Renderer::Renderer(const pandora::core::ui::Window& window,
                   const pandora::core::gpu::Context& context)
    : m_windowOwner(window), m_contextOwner(context) {
  const auto& swapchain = m_contextOwner.get().getPtrSwapchain();
  if (!swapchain) {
    return;
  }

  m_graphicDrivers.reserve(swapchain->getImageCount());
  for (size_t idx = 0u; idx < swapchain->getImageCount(); idx += 1u) {
    m_graphicDrivers.push_back(std::make_unique<pandora::core::CommandDriver>(
        m_contextOwner.get(), pandora::core::QueueFamilyType::Graphics));
  }
}

pandora::core::Result<FrameContext> Renderer::beginFrame() {
  const auto& context = m_contextOwner.get();
  const auto& ptr_swapchain = context.getPtrSwapchain();
  if (!ptr_swapchain) {
    return pandora::core::Error::runtime("Swapchain not initialized");
  }

  const auto update_result =
      ptr_swapchain->updateImageIndex(*context.getPtrDevice());
  if (!update_result.isOk()) {
    return update_result.error().withContext("Renderer::beginFrame");
  }

  if (m_renderKit.has_value()) {
    m_renderKit->get().updateIndex(ptr_swapchain->getImageIndex());
  }

  const auto frame_index = ptr_swapchain->getFrameSyncIndex();
  const auto image_index = ptr_swapchain->getImageIndex();
  auto& driver = *m_graphicDrivers.at(frame_index);
  driver.resetAllCommandPools(context);

  return FrameContext{image_index, frame_index, driver};
}

pandora::core::VoidResult Renderer::record(FrameContext& frame,
                                           const RecordFn& record_fn) {
  auto command_buffer = frame.driver.get().getGraphic();
  command_buffer.begin();

  const auto record_result = record_fn(command_buffer);
  if (!record_result.isOk()) {
    return record_result;
  }

  command_buffer.end();
  return pandora::core::ok();
}

pandora::core::VoidResult Renderer::endFrame(FrameContext& frame) {
  const auto& context = m_contextOwner.get();
  const auto& ptr_swapchain = context.getPtrSwapchain();

  const auto image_semaphore = ptr_swapchain->getImageAvailableSemaphore();
  const auto finished_semaphore = ptr_swapchain->getFinishedSemaphore();
  const auto finished_fence = ptr_swapchain->getFence();

  auto wait_semaphores = frame.extraWaitSemaphores;
  wait_semaphores.push_back(
      pandora::core::SubmitSemaphore{}
          .setSemaphore(image_semaphore)
          .setStageMask(pandora::core::PipelineStage::ColorAttachmentOutput));

  auto signal_semaphores = frame.extraSignalSemaphores;
  signal_semaphores.push_back(
      pandora::core::SubmitSemaphore{}
          .setSemaphore(finished_semaphore)
          .setStageMask(pandora::core::PipelineStage::AllGraphics));

  frame.driver.get().submit(pandora::core::SubmitSemaphoreGroup{}
                                .setWaitSemaphores(wait_semaphores)
                                .setSignalSemaphores(signal_semaphores),
                            finished_fence);

  const auto present_result =
      frame.driver.get().present(context, finished_semaphore);
  if (!present_result.isOk()) {
    return present_result.error().withContext("Renderer::endFrame");
  }

  ptr_swapchain->updateFrameSyncIndex();
  return pandora::core::ok();
}

}  // namespace pandora::highlevel
