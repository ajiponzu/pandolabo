#include "pandora/core/gpu.hpp"

pandora::core::gpu::TimelineSemaphore::TimelineSemaphore(
    const std::unique_ptr<Context>& ptr_context) {
  const auto semaphore_type_info =
      vk::SemaphoreTypeCreateInfo{}.setSemaphoreType(
          vk::SemaphoreType::eTimeline);
  const auto semaphore_info =
      vk::SemaphoreCreateInfo{}.setPNext(&semaphore_type_info);

  m_ptrSemaphore =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createSemaphoreUnique(
          semaphore_info);

  m_timelineSubmitInfo.setWaitSemaphoreValues(m_waitValue)
      .setSignalSemaphoreValues(m_signalValue);
}

pandora::core::gpu::TimelineSemaphore::~TimelineSemaphore() {}

void pandora::core::gpu::TimelineSemaphore::wait(
    const std::unique_ptr<Context>& ptr_context) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  const auto semaphore_wait_info = vk::SemaphoreWaitInfo{}
                                       .setSemaphores(m_ptrSemaphore.get())
                                       .setValues(m_waitValue);
  const auto vk_result = ptr_vk_device->waitSemaphores(
      semaphore_wait_info, std::numeric_limits<uint64_t>::max());
  if (vk_result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to wait for semaphore");
  }

  // Recycle this semaphore values
  m_signalValue = 1u;
  m_waitValue = 0u;
  m_waitStages.clear();

  m_timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfoKHR{}
                             .setWaitSemaphoreValues(m_waitValue)
                             .setSignalSemaphoreValues(m_signalValue);

  const auto semaphore_type_info =
      vk::SemaphoreTypeCreateInfo{}.setSemaphoreType(
          vk::SemaphoreType::eTimeline);
  const auto semaphore_info =
      vk::SemaphoreCreateInfo{}.setPNext(&semaphore_type_info);

  m_ptrSemaphore = ptr_vk_device->createSemaphoreUnique(semaphore_info);
}

pandora::core::gpu::SolidBinarySemaphore::SolidBinarySemaphore(
    const std::unique_ptr<Context>& ptr_context) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  m_ptrSemaphore = ptr_vk_device->createSemaphoreUnique({});
  m_ptrFence = ptr_vk_device->createFenceUnique(
      vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
}

pandora::core::gpu::SolidBinarySemaphore::~SolidBinarySemaphore() {}

pandora::core::gpu::BinarySemaphore
pandora::core::gpu::SolidBinarySemaphore::getSemaphore() const {
  BinarySemaphore semaphore{};
  semaphore.m_semaphore = m_ptrSemaphore.get();
  semaphore.m_fence = m_ptrFence.get();

  return semaphore;
}

void pandora::core::gpu::SolidBinarySemaphore::wait(
    const std::unique_ptr<Context>& ptr_context) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  const auto vk_result = ptr_vk_device->waitForFences(
      m_ptrFence.get(), VK_TRUE, std::numeric_limits<uint64_t>::max());
  if (vk_result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to wait for fence.");
  }
}

pandora::core::gpu::AcquireImageSemaphore::AcquireImageSemaphore(
    const std::unique_ptr<Context>& ptr_context) {
  m_semaphore = ptr_context->getPtrSwapchain()->getImageSemaphore();
  m_fence = ptr_context->getPtrSwapchain()->getFence();
}

pandora::core::gpu::AcquireImageSemaphore::~AcquireImageSemaphore() {}

pandora::core::gpu::RenderSemaphore::RenderSemaphore(
    const std::unique_ptr<Context>& ptr_context) {
  m_semaphore = ptr_context->getPtrSwapchain()->getFinishedSemaphore();
  m_fence = nullptr;
}

pandora::core::gpu::RenderSemaphore::~RenderSemaphore() {}
