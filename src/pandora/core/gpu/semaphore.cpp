#include <ranges>

#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

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
}

pandora::core::gpu::TimelineSemaphore::~TimelineSemaphore() {}

void pandora::core::gpu::SubmitSemaphoreGroup::setWaitStages(
    const std::vector<PipelineStage>& stages) const {
  m_waitStages = stages | std::views::transform([](const PipelineStage& stage) {
                   return static_cast<vk::PipelineStageFlags>(
                       vk_helper::getPipelineStageFlagBits(stage));
                 })
                 | std::ranges::to<std::vector>();
}
