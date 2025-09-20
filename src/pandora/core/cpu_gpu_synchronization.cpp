#include <ranges>

#include "pandora/core/synchronization_helpers.hpp"

bool pandora::core::WaitedFences::wait(
    const std::unique_ptr<gpu::Context>& ptr_context,
    uint64_t timeout,
    bool is_reset) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  const auto vk_result =
      ptr_vk_device->waitForFences(m_fences, VK_TRUE, timeout);
  if (vk_result != vk::Result::eSuccess) {
    return false;
  }

  if (is_reset) {
    ptr_vk_device->resetFences(m_fences);
  }

  return true;
}

pandora::core::TimelineSemaphoreDriver&
pandora::core::TimelineSemaphoreDriver::setSemaphores(
    const std::vector<std::reference_wrapper<gpu::TimelineSemaphore>>&
        semaphores) {
  m_semaphores = semaphores
                 | std::views::transform([](const gpu::TimelineSemaphore& sem) {
                     return sem.getTimelineSemaphore();
                   })
                 | std::ranges::to<std::vector>();

  return *this;
}

pandora::core::TimelineSemaphoreDriver&
pandora::core::TimelineSemaphoreDriver::setValues(
    const std::vector<uint64_t>& values) {
  m_values = values;

  return *this;
}

bool pandora::core::TimelineSemaphoreDriver::wait(
    const std::unique_ptr<gpu::Context>& ptr_context, uint64_t timeout) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  const auto semaphore_wait_info =
      vk::SemaphoreWaitInfo{}.setSemaphores(m_semaphores).setValues(m_values);
  const auto vk_result =
      ptr_vk_device->waitSemaphores(semaphore_wait_info, timeout);
  if (vk_result != vk::Result::eSuccess) {
    return false;
  }

  return true;
}

void pandora::core::TimelineSemaphoreDriver::signal(
    const std::unique_ptr<gpu::Context>& ptr_context) {
  const auto& ptr_vk_device =
      ptr_context->getPtrDevice()->getPtrLogicalDevice();

  for (const auto& [semaphore, value] :
       std::ranges::views::zip(m_semaphores, m_values)) {
    const auto semaphore_signal_info =
        vk::SemaphoreSignalInfo{}.setSemaphore(semaphore).setValue(value);
    ptr_vk_device->signalSemaphore(semaphore_signal_info);
  }
}
