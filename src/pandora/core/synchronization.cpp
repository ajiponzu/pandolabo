#include "pandora/core/synchronization.hpp"

#include <ranges>

#include "pandora/core/gpu/vk_helper.hpp"

namespace pandora::core {

WaitedFences::WaitedFences(const std::vector<gpu::Fence>& fences) {
  m_fences =
      fences
      | std::views::transform([](const gpu::Fence& f) { return f.getFence(); })
      | std::ranges::to<std::vector>();
}

bool WaitedFences::wait(const std::unique_ptr<gpu::Context>& ptr_context,
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

TimelineSemaphoreDriver& TimelineSemaphoreDriver::setSemaphores(
    const std::vector<std::reference_wrapper<gpu::TimelineSemaphore>>&
        semaphores) {
  m_semaphores = semaphores
                 | std::views::transform([](const gpu::TimelineSemaphore& sem) {
                     return sem.getTimelineSemaphore();
                   })
                 | std::ranges::to<std::vector>();

  return *this;
}

TimelineSemaphoreDriver& TimelineSemaphoreDriver::setValues(
    const std::vector<uint64_t>& values) {
  m_values = values;

  return *this;
}

bool TimelineSemaphoreDriver::wait(
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

void TimelineSemaphoreDriver::signal(
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

void SubmitSemaphoreGroup::setWaitStages(
    const std::vector<PipelineStage>& stages) const {
  m_waitStages = stages | std::views::transform([](const PipelineStage& stage) {
                   return static_cast<vk::PipelineStageFlags>(
                       vk_helper::getPipelineStageFlagBits(stage));
                 })
                 | std::ranges::to<std::vector>();
}

}  // namespace pandora::core
