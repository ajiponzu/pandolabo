#include "basic_computing.hpp"

#include <cstdio>
#include <mutex>
#include <optional>
#include <print>
#include <shared_mutex>
#include <thread>

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

namespace {

plc::VoidResult set_transfer_secondary_command(
    const plc::TransferCommandBuffer& command_buffer,
    plc::gpu::Buffer& transfered_buffer,
    const std::pair<uint32_t, uint32_t> queue_family_indices,
    plc::gpu::Buffer& staging_buffer) {
  command_buffer.begin();

  command_buffer.copyBuffer(staging_buffer, transfered_buffer);

  // release the ownership of the gpu storage buffer
  PANDORA_TRY_ASSIGN(buffer_barrier,
                     plc::gpu::BufferBarrierBuilder::create()
                         .setBuffer(transfered_buffer)
                         .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
                         .setDstAccessFlags({plc::AccessFlag::ShaderRead,
                                             plc::AccessFlag::ShaderWrite})
                         .setSrcStages({plc::PipelineStage::Transfer})
                         .setDstStages({plc::PipelineStage::Transfer})
                         .setSrcQueueFamilyIndex(queue_family_indices.first)
                         .setDstQueueFamilyIndex(queue_family_indices.second)
                         .build());

  command_buffer.setPipelineBarrier(
      plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));

  command_buffer.end();

  return plc::ok();
}

}  // namespace

namespace samples::core {

BasicComputing::BasicComputing() {
  m_ptrContext = std::make_unique<plc::gpu::Context>(nullptr);

  m_ptrComputeCommandDriver = std::make_unique<plc::CommandDriver>(
      *m_ptrContext, plc::QueueFamilyType::Compute);

  m_ptrTransferCommandDriver = std::make_unique<plc::CommandDriver>(
      *m_ptrContext, plc::QueueFamilyType::Transfer);

  m_ptrUniformBuffer =
      plc::createUniqueUniformBuffer(*m_ptrContext, sizeof(float_t));
  const auto uniform_mapped_address =
      m_ptrUniformBuffer->mapMemory(*m_ptrContext);
  std::fill_n(reinterpret_cast<float_t*>(uniform_mapped_address),
              m_ptrUniformBuffer->getSize() / sizeof(float_t),
              3.14f);
  m_ptrUniformBuffer->unmapMemory(*m_ptrContext);

  m_ptrInputStorageBuffer = plc::createUniqueStorageBuffer(
      *m_ptrContext, plc::TransferType::TransferDst, sizeof(uint32_t) * 1024u);

  m_ptrOutputStorageBuffer =
      plc::createUniqueStorageBuffer(*m_ptrContext,
                                     plc::TransferType::TransferSrcDst,
                                     sizeof(uint32_t) * 1024u);
}

BasicComputing::~BasicComputing() {
  m_ptrContext->getPtrDevice()->waitIdle();
}

void BasicComputing::run() {
  try {
    auto result_buffer = plc::createStagingBufferFromGPU(
        *m_ptrContext, m_ptrOutputStorageBuffer->getSize());

    {
      std::vector<plc::gpu::Buffer> staging_buffers;

      const auto shader_result = constructShaderResources();
      if (!shader_result.isOk()) {
        std::println(stderr,
                     "BasicComputing shader load error: {}",
                     shader_result.error().toString());
        return;
      }
      const auto transfer_result = setTransferCommands(staging_buffers);
      if (!transfer_result.isOk()) {
        std::println(stderr,
                     "BasicComputing transfer error: {}",
                     transfer_result.error().toString());
        return;
      }
      const auto compute_result = setComputeCommands(result_buffer);
      if (!compute_result.isOk()) {
        std::println(stderr,
                     "BasicComputing compute error: {}",
                     compute_result.error().toString());
        return;
      }

      plc::gpu::TimelineSemaphore semaphore(*m_ptrContext);

      m_ptrTransferCommandDriver->submit(
          plc::SubmitSemaphoreGroup{}
              .setWaitSemaphores({
                  plc::SubmitSemaphore{}
                      .setSemaphore(semaphore)
                      .setValue(0u)
                      .setStageMask(plc::PipelineStage::Transfer),
              })
              .setSignalSemaphores({
                  plc::SubmitSemaphore{}
                      .setSemaphore(semaphore)
                      .setValue(1u)
                      .setStageMask(plc::PipelineStage::Transfer),
              }));
      m_ptrComputeCommandDriver->submit(
          plc::SubmitSemaphoreGroup{}
              .setWaitSemaphores({
                  plc::SubmitSemaphore{}
                      .setSemaphore(semaphore)
                      .setValue(1u)
                      .setStageMask(plc::PipelineStage::Transfer),
              })
              .setSignalSemaphores({
                  plc::SubmitSemaphore{}
                      .setSemaphore(semaphore)
                      .setValue(2u)
                      .setStageMask(plc::PipelineStage::AllCommands),
              }));

      plc::TimelineSemaphoreDriver{}
          .setSemaphores({semaphore})
          .setValues({2u})
          .wait(*m_ptrContext);
    }

    {
      std::vector<uint32_t> result(result_buffer.getSize() / sizeof(uint32_t));
      const auto result_mapped_address = result_buffer.mapMemory(*m_ptrContext);
      std::memcpy(reinterpret_cast<void*>(result.data()),
                  result_mapped_address,
                  result_buffer.getSize());
      result_buffer.unmapMemory(*m_ptrContext);

      for (size_t idx = 0u; const auto& item : result) {
        std::println(stdout, "idx[{}]: {}", idx, item);
      }
    }

    m_ptrComputeCommandDriver->resetAllCommandPools(*m_ptrContext);
    m_ptrTransferCommandDriver->resetAllCommandPools(*m_ptrContext);
  } catch (const std::exception& e) {
    std::println(stderr, "Exception caught: {}", e.what());
    throw;
  } catch (...) {
    std::println(stderr, "Unknown exception caught");
    throw;
  }
}

plc::VoidResult BasicComputing::setTransferCommands(
    std::vector<plc::gpu::Buffer>& staging_buffers) {
  std::shared_mutex mutex;
  std::mutex error_mutex;
  std::optional<plc::Error> thread_error;

  m_ptrTransferCommandDriver->constructSecondary(*m_ptrContext, 2);

  staging_buffers.push_back(plc::createStagingBufferToGPU(
      *m_ptrContext, m_ptrInputStorageBuffer->getSize()));
  staging_buffers.push_back(plc::createStagingBufferToGPU(
      *m_ptrContext, m_ptrOutputStorageBuffer->getSize()));

  const auto src_queue_family_index =
      m_ptrContext->getPtrDevice()->getQueueFamilyIndex(
          plc::QueueFamilyType::Transfer);
  const auto dst_queue_family_index =
      m_ptrContext->getPtrDevice()->getQueueFamilyIndex(
          plc::QueueFamilyType::Compute);

  // multi-threading
  std::thread transfer_thread0([&]() {
    const size_t thread_index = 0u;

    std::unique_lock<std::shared_mutex> buffer_lock(mutex);
    const auto command_buffer =
        m_ptrTransferCommandDriver->getTransfer(thread_index);
    buffer_lock.unlock();

    buffer_lock.lock();
    auto& staging_buffer = staging_buffers[thread_index];
    const auto mapped_address = staging_buffer.mapMemory(*m_ptrContext);
    buffer_lock.unlock();

    std::fill_n(reinterpret_cast<uint32_t*>(mapped_address),
                staging_buffer.getSize() / sizeof(uint32_t),
                5U);

    buffer_lock.lock();
    staging_buffer.unmapMemory(*m_ptrContext);
    buffer_lock.unlock();

    const auto result = set_transfer_secondary_command(
        command_buffer,
        *m_ptrInputStorageBuffer,
        {src_queue_family_index, dst_queue_family_index},
        staging_buffer);
    if (!result.isOk()) {
      std::scoped_lock error_lock(error_mutex);
      if (!thread_error.has_value()) {
        thread_error = result.error();
      }
    }
  });

  std::thread transfer_thread1([&]() {
    const size_t thread_index = 1u;

    std::unique_lock<std::shared_mutex> buffer_lock(mutex);
    const auto command_buffer =
        m_ptrTransferCommandDriver->getTransfer(thread_index);
    buffer_lock.unlock();

    buffer_lock.lock();
    auto& staging_buffer = staging_buffers[thread_index];
    const auto mapped_address = staging_buffer.mapMemory(*m_ptrContext);
    buffer_lock.unlock();

    std::fill_n(reinterpret_cast<uint32_t*>(mapped_address),
                staging_buffer.getSize() / sizeof(uint32_t),
                5U);

    buffer_lock.lock();
    staging_buffer.unmapMemory(*m_ptrContext);
    buffer_lock.unlock();

    const auto result = set_transfer_secondary_command(
        command_buffer,
        *m_ptrOutputStorageBuffer,
        {src_queue_family_index, dst_queue_family_index},
        staging_buffer);
    if (!result.isOk()) {
      std::scoped_lock error_lock(error_mutex);
      if (!thread_error.has_value()) {
        thread_error = result.error();
      }
    }
  });

  const auto primary_command_buffer = m_ptrTransferCommandDriver->getPrimary();

  primary_command_buffer.begin();

  // wait for the completion of the secondary command
  transfer_thread0.join();
  transfer_thread1.join();

  // merge secondary command
  m_ptrTransferCommandDriver->mergeSecondaryCommands();

  primary_command_buffer.end();

  if (thread_error.has_value()) {
    return *thread_error;
  }

  return plc::ok();
}

plc::VoidResult BasicComputing::constructShaderResources() {
  PANDORA_TRY_ASSIGN(
      spirv_binary,
      plc::io::shader::read("examples/core/basic_compute/basic.comp"));

  m_shaderModuleMap["compute"] =
      plc::gpu::ShaderModule(*m_ptrContext, spirv_binary);

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"compute"});

  m_ptrDescriptorSetLayout = std::make_unique<plc::gpu::DescriptorSetLayout>(
      *m_ptrContext, description_unit);

  m_ptrDescriptorSet = std::make_unique<plc::gpu::DescriptorSet>(
      *m_ptrContext, *m_ptrDescriptorSetLayout);

  std::vector<plc::gpu::BufferDescription> buffer_descriptions;
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("UniformNumber"),
      *m_ptrUniformBuffer);
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("Output"),
      *m_ptrOutputStorageBuffer);
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("Input"),
      *m_ptrInputStorageBuffer);

  m_ptrDescriptorSet->updateDescriptorSet(
      *m_ptrContext, buffer_descriptions, {});

  m_ptrComputePipeline =
      std::make_unique<plc::Pipeline>(*m_ptrContext,
                                      description_unit,
                                      *m_ptrDescriptorSetLayout,
                                      plc::PipelineBind::Compute);
  m_ptrComputePipeline->constructComputePipeline(
      *m_ptrContext, m_shaderModuleMap.at("compute"));

  return plc::ok();
}

plc::VoidResult BasicComputing::setComputeCommands(
    plc::gpu::Buffer& staging_buffer) {
  const auto command_buffer = m_ptrComputeCommandDriver->getCompute();

  command_buffer.begin();

  {
    // acquire the ownership of the gpu storage buffer
    // acrquire barrier parameters are same as the release barrier.
    PANDORA_TRY_ASSIGN(
        buffer_barrier,
        plc::gpu::BufferBarrierBuilder::create()
            .setBuffer(*m_ptrInputStorageBuffer)
            .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
            .setDstAccessFlags(
                {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
            .setSrcStages({plc::PipelineStage::Transfer})
            .setDstStages({plc::PipelineStage::ComputeShader})
            .setSrcQueueFamilyIndex(
                m_ptrTransferCommandDriver->getQueueFamilyIndex())
            .setDstQueueFamilyIndex(
                m_ptrComputeCommandDriver->getQueueFamilyIndex())
            .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));
  }

  {
    // acquire the ownership of the gpu storage buffer
    PANDORA_TRY_ASSIGN(
        buffer_barrier,
        plc::gpu::BufferBarrierBuilder::create()
            .setBuffer(*m_ptrOutputStorageBuffer)
            .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
            .setDstAccessFlags(
                {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
            .setSrcStages({plc::PipelineStage::Transfer})
            .setDstStages({plc::PipelineStage::ComputeShader})
            .setSrcQueueFamilyIndex(
                m_ptrTransferCommandDriver->getQueueFamilyIndex())
            .setDstQueueFamilyIndex(
                m_ptrComputeCommandDriver->getQueueFamilyIndex())
            .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));
  }

  command_buffer.bindPipeline(*m_ptrComputePipeline);
  command_buffer.bindDescriptorSet(*m_ptrComputePipeline, *m_ptrDescriptorSet);
  command_buffer.compute(plc::ComputeWorkGroupSize{4u, 1u, 1u});

  {
    PANDORA_TRY_ASSIGN(
        buffer_barrier,
        plc::gpu::BufferBarrierBuilder::create()
            .setBuffer(*m_ptrOutputStorageBuffer)
            .setSrcAccessFlags(
                {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
            .setDstAccessFlags({plc::AccessFlag::TransferRead})
            .setSrcStages({plc::PipelineStage::ComputeShader})
            .setDstStages({plc::PipelineStage::Transfer})
            .setSrcQueueFamilyIndex(
                m_ptrTransferCommandDriver->getQueueFamilyIndex())
            .setDstQueueFamilyIndex(
                m_ptrComputeCommandDriver->getQueueFamilyIndex())
            .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));
  }

  command_buffer.copyBuffer(*m_ptrOutputStorageBuffer, staging_buffer);

  {
    PANDORA_TRY_ASSIGN(
        buffer_barrier,
        plc::gpu::BufferBarrierBuilder::create()
            .setBuffer(*m_ptrOutputStorageBuffer)
            .setSrcAccessFlags({plc::AccessFlag::TransferRead})
            .setDstAccessFlags({plc::AccessFlag::Unknown})
            .setSrcStages({plc::PipelineStage::ComputeShader})
            .setDstStages({plc::PipelineStage::Transfer})
            .setSrcQueueFamilyIndex(
                m_ptrTransferCommandDriver->getQueueFamilyIndex())
            .setDstQueueFamilyIndex(
                m_ptrComputeCommandDriver->getQueueFamilyIndex())
            .build());

    command_buffer.setPipelineBarrier(
        plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));
  }

  command_buffer.end();

  return plc::ok();
}

}  // namespace samples::core
