#include <ranges>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

namespace pandora::core::gpu {

TimelineSemaphore::TimelineSemaphore(
    const std::unique_ptr<Context>& ptr_context) {
  const auto semaphore_type_info =
      vk::SemaphoreTypeCreateInfo{}.setSemaphoreType(
          vk::SemaphoreType::eTimeline);
  const auto semaphore_info =
      vk::SemaphoreCreateInfo{}.setPNext(&semaphore_type_info);

  m_ptrSemaphore =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createSemaphoreUnique(
          semaphore_info);
}

TimelineSemaphore::~TimelineSemaphore() {}

}  // namespace pandora::core::gpu
