#include <iostream>
#include <ranges>

#include "pandora/core/command_buffer.hpp"
#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/pipeline.hpp"
#include "pandora/core/renderpass.hpp"

pandora::core::CommandDriver::CommandDriver(const std::unique_ptr<gpu::Context>& ptr_context,
                                            const pandora::core::QueueFamilyType queue_family) {
  const auto& ptr_device = ptr_context->getPtrDevice();

  m_queueFamilyType = queue_family;
  m_queueFamilyIndex = ptr_device->getQueueFamilyIndex(queue_family);

  m_queue = ptr_context->getPtrDevice()->getQueue(m_queueFamilyIndex);

  {
    vk::CommandPoolCreateInfo pool_info{{}, m_queueFamilyIndex};
    pool_info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    m_ptrCommandPool = ptr_device->getPtrLogicalDevice()->createCommandPoolUnique(pool_info);
  }

  vk::CommandBufferAllocateInfo alloc_info{m_ptrCommandPool.get(), vk::CommandBufferLevel::ePrimary, 1U};

  m_ptrPrimaryCommandBuffer =
      std::move(ptr_device->getPtrLogicalDevice()->allocateCommandBuffersUnique(alloc_info).front());
}

pandora::core::CommandDriver::~CommandDriver() {}

void pandora::core::CommandDriver::constructSecondary(const std::unique_ptr<gpu::Context>& ptr_context,
                                                      const uint32_t required_secondary_num) {
  const auto& ptr_vk_device = ptr_context->getPtrDevice()->getPtrLogicalDevice();

  for (uint32_t idx = 0U; idx < required_secondary_num; idx += 1U) {
    m_ptrSecondaryCommandPools.push_back(
        ptr_vk_device->createCommandPoolUnique(vk::CommandPoolCreateInfo()
                                                   .setQueueFamilyIndex(m_queueFamilyIndex)
                                                   .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)));
    m_secondaryCommandBuffers.push_back(
        std::move(ptr_vk_device
                      ->allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo()
                                                         .setCommandPool(m_ptrSecondaryCommandPools.back().get())
                                                         .setLevel(vk::CommandBufferLevel::eSecondary)
                                                         .setCommandBufferCount(1U))
                      .front()));
  }
}

void pandora::core::CommandDriver::resetAllCommands() const {
  for (const auto& command_buffer : m_secondaryCommandBuffers) {
    command_buffer->reset(vk::CommandBufferResetFlags());
  }

  m_ptrPrimaryCommandBuffer->reset(vk::CommandBufferResetFlags());
}

void pandora::core::CommandDriver::resetAllCommandPools(const std::unique_ptr<gpu::Context>& ptr_context) const {
  const auto& ptr_vk_device = ptr_context->getPtrDevice()->getPtrLogicalDevice();

  for (const auto& command_pool : m_ptrSecondaryCommandPools) {
    ptr_vk_device->resetCommandPool(command_pool.get());
  }

  ptr_vk_device->resetCommandPool(m_ptrCommandPool.get(), vk::CommandPoolResetFlags());
}

void pandora::core::CommandDriver::mergeSecondaryCommands() const {
  using C = std::ranges::range_value_t<decltype(m_secondaryCommandBuffers)>;

  m_ptrPrimaryCommandBuffer->executeCommands(m_secondaryCommandBuffers
                                             | std::views::transform([](const C& buf) { return buf.get(); })
                                             | std::ranges::to<std::vector<vk::CommandBuffer>>());
}

void pandora::core::CommandDriver::submit(const PipelineStage dst_stage, gpu::TimelineSemaphore& semaphore) const {
  auto submit_info = vk::SubmitInfo()
                         .setPNext(semaphore.getPtrTimelineSubmitInfo())
                         .setCommandBuffers(m_ptrPrimaryCommandBuffer.get())
                         .setWaitSemaphores(semaphore.getSemaphore())
                         .setSignalSemaphores(semaphore.getSemaphore());

  semaphore.setWaitStage(vk_helper::getPipelineStageFlagBits(dst_stage));
  submit_info.setWaitDstStageMask(semaphore.getBackWaitStage());

  m_queue.submit(submit_info);

  semaphore.updateWaitValue();
  semaphore.updateSignalValue();
}

void pandora::core::CommandDriver::submit(gpu::BinarySemaphore& wait_semaphore,
                                          const PipelineStage dst_stage,
                                          gpu::BinarySemaphore& signal_semaphore) const {
  auto submit_info = vk::SubmitInfo()
                         .setCommandBuffers(m_ptrPrimaryCommandBuffer.get())
                         .setWaitSemaphores(wait_semaphore.getSemaphore())
                         .setSignalSemaphores(signal_semaphore.getSemaphore());

  const vk::PipelineStageFlags vk_stage_flags = vk_helper::getPipelineStageFlagBits(dst_stage);
  submit_info.setWaitDstStageMask(vk_stage_flags);

  m_queue.submit(submit_info, signal_semaphore.getFence());
}

void pandora::core::CommandDriver::present(const std::unique_ptr<gpu::Context>& ptr_context,
                                           gpu::BinarySemaphore& wait_semaphore) const {
  if (m_queueFamilyType != pandora::core::QueueFamilyType::Graphics) {
    throw std::runtime_error("Queue family type is not present.");
  }

  const auto& ptr_swapchain = ptr_context->getPtrSwapchain();
  const auto image_index = ptr_swapchain->getImageIndex();

  try {
    static_cast<void>(m_queue.presentKHR(vk::PresentInfoKHR()
                                             .setWaitSemaphores(wait_semaphore.getSemaphore())
                                             .setSwapchains(ptr_swapchain->getSwapchain())
                                             .setImageIndices(image_index)));
  } catch (const vk::SystemError&) {
    throw std::runtime_error("Failed to present image.");
  }
}

pandora::core::GraphicCommandBuffer pandora::core::CommandDriver::getGraphic(
    const std::optional<size_t> secondary_index) const {
  if (secondary_index.has_value()) {
    return GraphicCommandBuffer(m_secondaryCommandBuffers.at(secondary_index.value()), true);
  }

  return GraphicCommandBuffer(m_ptrPrimaryCommandBuffer);
}

pandora::core::ComputeCommandBuffer pandora::core::CommandDriver::getCompute(
    const std::optional<size_t> secondary_index) const {
  if (secondary_index.has_value()) {
    return ComputeCommandBuffer(m_secondaryCommandBuffers.at(secondary_index.value()), true);
  }

  return ComputeCommandBuffer(m_ptrPrimaryCommandBuffer);
}

pandora::core::TransferCommandBuffer pandora::core::CommandDriver::getTransfer(
    const std::optional<size_t> secondary_index) const {
  if (secondary_index.has_value()) {
    return TransferCommandBuffer(m_secondaryCommandBuffers.at(secondary_index.value()), true);
  }

  return TransferCommandBuffer(m_ptrPrimaryCommandBuffer);
}
