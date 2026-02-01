#include "pandora/core/synchronization.hpp"

#include <ranges>

#include "pandora/core/gpu/vk_helper.hpp"

namespace pandora::core {

BarrierDependency& BarrierDependency::setMemoryBarriers(
    const std::vector<std::reference_wrapper<const gpu::MemoryBarrier>>&
        barriers) {
  m_memoryBarriers =
      barriers | std::views::transform([](const gpu::MemoryBarrier& barrier) {
        return barrier.getBarrier();
      })
      | std::ranges::to<std::vector>();

  m_dependencyInfo.setMemoryBarriers(m_memoryBarriers);

  return *this;
}

BarrierDependency& BarrierDependency::setBufferBarriers(
    const std::vector<std::reference_wrapper<const gpu::BufferBarrier>>&
        barriers) {
  m_bufferBarriers =
      barriers | std::views::transform([](const gpu::BufferBarrier& barrier) {
        return barrier.getBarrier();
      })
      | std::ranges::to<std::vector>();

  m_dependencyInfo.setBufferMemoryBarriers(m_bufferBarriers);

  return *this;
}

BarrierDependency& BarrierDependency::setImageBarriers(
    const std::vector<std::reference_wrapper<const gpu::ImageBarrier>>&
        barriers) {
  m_imageBarriers =
      barriers | std::views::transform([](const gpu::ImageBarrier& barrier) {
        return barrier.getBarrier();
      })
      | std::ranges::to<std::vector>();

  m_dependencyInfo.setImageMemoryBarriers(m_imageBarriers);

  return *this;
}

WaitedFences::WaitedFences(const std::vector<gpu::Fence>& fences) {
  m_fences =
      fences
      | std::views::transform([](const gpu::Fence& f) { return f.getFence(); })
      | std::ranges::to<std::vector>();
}

bool WaitedFences::wait(const gpu::Context& context,
                        uint64_t timeout,
                        bool is_reset) {
  const auto& ptr_vk_device = context.getPtrDevice()->getPtrLogicalDevice();

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
                     return sem.getSemaphore();
                   })
                 | std::ranges::to<std::vector>();

  return *this;
}

TimelineSemaphoreDriver& TimelineSemaphoreDriver::setValues(
    const std::vector<uint64_t>& values) {
  m_values = values;

  return *this;
}

bool TimelineSemaphoreDriver::wait(const gpu::Context& context,
                                   uint64_t timeout) {
  const auto& ptr_vk_device = context.getPtrDevice()->getPtrLogicalDevice();

  const auto semaphore_wait_info =
      vk::SemaphoreWaitInfo{}.setSemaphores(m_semaphores).setValues(m_values);
  const auto vk_result =
      ptr_vk_device->waitSemaphores(semaphore_wait_info, timeout);
  if (vk_result != vk::Result::eSuccess) {
    return false;
  }

  return true;
}

void TimelineSemaphoreDriver::signal(const gpu::Context& context) {
  const auto& ptr_vk_device = context.getPtrDevice()->getPtrLogicalDevice();

  for (const auto& [semaphore, value] :
       std::ranges::views::zip(m_semaphores, m_values)) {
    const auto semaphore_signal_info =
        vk::SemaphoreSignalInfo{}.setSemaphore(semaphore).setValue(value);
    ptr_vk_device->signalSemaphore(semaphore_signal_info);
  }
}

SubmitSemaphoreGroup& SubmitSemaphoreGroup::setWaitSemaphores(
    const std::vector<SubmitSemaphore>& semaphores) {
  m_waitSemaphores = semaphores
                     | std::views::transform([](const SubmitSemaphore& sem) {
                         return sem.getSemaphoreSubmitInfo();
                       })
                     | std::ranges::to<std::vector>();

  return *this;
}

SubmitSemaphoreGroup& SubmitSemaphoreGroup::setSignalSemaphores(
    const std::vector<SubmitSemaphore>& semaphores) {
  m_signalSemaphores = semaphores
                       | std::views::transform([](const SubmitSemaphore& sem) {
                           return sem.getSemaphoreSubmitInfo();
                         })
                       | std::ranges::to<std::vector>();

  return *this;
}

}  // namespace pandora::core
