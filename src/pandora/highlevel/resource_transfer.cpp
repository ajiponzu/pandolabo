#include "pandora/highlevel/resource_transfer.hpp"

#include <cstring>
#include <memory>

namespace {

void writeToStagingBuffer(pandora::core::gpu::Buffer& staging,
                          const pandora::core::gpu::Context& context,
                          std::span<const std::byte> data) {
  auto* mapped = staging.mapMemory(context);
  std::memcpy(mapped, data.data(), data.size_bytes());
  staging.unmapMemory(context);
}

void readFromStagingBuffer(const pandora::core::gpu::Buffer& staging,
                           const pandora::core::gpu::Context& context,
                           std::span<std::byte> out) {
  const auto* mapped = staging.mapMemory(context);
  std::memcpy(out.data(), mapped, out.size_bytes());
  staging.unmapMemory(context);
}

template <typename RecordFn>
void submitTransfer(pandora::core::CommandDriver& driver,
                    const pandora::core::gpu::Context& context,
                    RecordFn&& record_fn) {
  driver.resetAllCommandPools(context);
  auto cmd = driver.getTransfer();
  cmd.begin();
  record_fn(cmd);
  cmd.end();

  driver.submit(pandora::core::SubmitSemaphoreGroup{});
  driver.queueWaitIdle();
}

}  // namespace

namespace pandora::highlevel {

pandora::core::CommandDriver& TransferPlan::ensureDriver() {
  if (!m_transferDriver) {
    m_transferDriver = std::make_unique<pandora::core::CommandDriver>(
        m_contextOwner.get(), m_queueFamilyType);
  }
  return *m_transferDriver;
}

void TransferPlan::commitBarriersInternal() {
  if (!m_hasPendingBarriers) {
    return;
  }

  pandora::core::BarrierDependency dependency{};
  if (!m_bufferBarriers.empty()) {
    std::vector<std::reference_wrapper<const pandora::core::gpu::BufferBarrier>>
        buffer_refs;
    buffer_refs.reserve(m_bufferBarriers.size());
    for (const auto& barrier : m_bufferBarriers) {
      buffer_refs.emplace_back(barrier);
    }
    dependency.setBufferBarriers(buffer_refs);
  }

  if (!m_imageBarriers.empty()) {
    std::vector<std::reference_wrapper<const pandora::core::gpu::ImageBarrier>>
        image_refs;
    image_refs.reserve(m_imageBarriers.size());
    for (const auto& barrier : m_imageBarriers) {
      image_refs.emplace_back(barrier);
    }
    dependency.setImageBarriers(image_refs);
  }

  auto dependency_ptr =
      std::make_shared<pandora::core::BarrierDependency>(std::move(dependency));
  m_commands.emplace_back(
      [dependency_ptr](pandora::core::TransferCommandBuffer& cmd) {
        cmd.setPipelineBarrier(*dependency_ptr);
      });

  m_bufferBarriers.clear();
  m_imageBarriers.clear();
  m_hasPendingBarriers = false;
}

pandora::core::VoidResult TransferPlan::addBufferBarrier(
    const pandora::core::gpu::Buffer& buffer,
    const std::vector<pandora::core::AccessFlag>& src_access,
    const std::vector<pandora::core::AccessFlag>& dst_access,
    const std::vector<pandora::core::PipelineStage>& src_stages,
    const std::vector<pandora::core::PipelineStage>& dst_stages,
    std::optional<uint32_t> src_queue_family,
    std::optional<uint32_t> dst_queue_family) {
  auto builder = pandora::core::gpu::BufferBarrierBuilder::create()
                     .setBuffer(buffer)
                     .setSrcAccessFlags(src_access)
                     .setDstAccessFlags(dst_access)
                     .setSrcStages(src_stages)
                     .setDstStages(dst_stages);
  if (src_queue_family.has_value()) {
    builder.setSrcQueueFamilyIndex(src_queue_family.value());
  }
  if (dst_queue_family.has_value()) {
    builder.setDstQueueFamilyIndex(dst_queue_family.value());
  }

  PANDORA_TRY_ASSIGN(barrier, builder.build());
  m_bufferBarriers.emplace_back(std::move(barrier));
  m_hasPendingBarriers = true;
  return pandora::core::ok();
}

pandora::core::VoidResult TransferPlan::addImageBarrier(
    const pandora::core::gpu::Image& image,
    const pandora::core::ImageViewInfo& view_info,
    pandora::core::ImageLayout old_layout,
    pandora::core::ImageLayout new_layout,
    const std::vector<pandora::core::AccessFlag>& src_access,
    const std::vector<pandora::core::AccessFlag>& dst_access,
    const std::vector<pandora::core::PipelineStage>& src_stages,
    const std::vector<pandora::core::PipelineStage>& dst_stages,
    std::optional<uint32_t> src_queue_family,
    std::optional<uint32_t> dst_queue_family) {
  auto builder = pandora::core::gpu::ImageBarrierBuilder::create()
                     .setImage(image)
                     .setImageViewInfo(view_info)
                     .setOldLayout(old_layout)
                     .setNewLayout(new_layout)
                     .setSrcAccessFlags(src_access)
                     .setDstAccessFlags(dst_access)
                     .setSrcStages(src_stages)
                     .setDstStages(dst_stages);
  if (src_queue_family.has_value()) {
    builder.setSrcQueueFamilyIndex(src_queue_family.value());
  }
  if (dst_queue_family.has_value()) {
    builder.setDstQueueFamilyIndex(dst_queue_family.value());
  }

  PANDORA_TRY_ASSIGN(barrier, builder.build());
  m_imageBarriers.emplace_back(std::move(barrier));
  m_hasPendingBarriers = true;
  return pandora::core::ok();
}

pandora::core::VoidResult TransferPlan::addBackbufferBarrier(
    pandora::core::ImageLayout old_layout,
    pandora::core::ImageLayout new_layout,
    const std::vector<pandora::core::AccessFlag>& src_access,
    const std::vector<pandora::core::AccessFlag>& dst_access,
    const std::vector<pandora::core::PipelineStage>& src_stages,
    const std::vector<pandora::core::PipelineStage>& dst_stages,
    std::optional<uint32_t> src_queue_family,
    std::optional<uint32_t> dst_queue_family) {
  auto builder = pandora::core::gpu::ImageBarrierBuilder::create()
                     .setOldLayout(old_layout)
                     .setNewLayout(new_layout)
                     .setSrcAccessFlags(src_access)
                     .setDstAccessFlags(dst_access)
                     .setSrcStages(src_stages)
                     .setDstStages(dst_stages);
  if (src_queue_family.has_value()) {
    builder.setSrcQueueFamilyIndex(src_queue_family.value());
  }
  if (dst_queue_family.has_value()) {
    builder.setDstQueueFamilyIndex(dst_queue_family.value());
  }

  PANDORA_TRY_ASSIGN(barrier, builder.build(m_contextOwner.get()));
  m_imageBarriers.emplace_back(std::move(barrier));
  m_hasPendingBarriers = true;
  return pandora::core::ok();
}

void TransferPlan::flushBarriers() {
  commitBarriersInternal();
}

TransferPlan& TransferPlan::copyBuffer(const pandora::core::gpu::Buffer& src,
                                       const pandora::core::gpu::Buffer& dst) {
  commitBarriersInternal();
  m_commands.emplace_back(
      [&src, &dst](pandora::core::TransferCommandBuffer& cmd) {
        cmd.copyBuffer(src, dst);
      });
  return *this;
}

TransferPlan& TransferPlan::copyBufferToImage(
    const pandora::core::gpu::Buffer& src,
    const pandora::core::gpu::Image& dst,
    pandora::core::ImageLayout dst_layout,
    const pandora::core::ImageViewInfo& view_info) {
  commitBarriersInternal();
  m_commands.emplace_back([&src, &dst, dst_layout, &view_info](
                              pandora::core::TransferCommandBuffer& cmd) {
    cmd.copyBufferToImage(src, dst, dst_layout, view_info);
  });
  return *this;
}

TransferPlan& TransferPlan::copyImageToBuffer(
    const pandora::core::gpu::Image& src,
    const pandora::core::gpu::Buffer& dst,
    pandora::core::ImageLayout src_layout,
    const pandora::core::ImageViewInfo& view_info) {
  commitBarriersInternal();
  m_commands.emplace_back([&src, &dst, src_layout, &view_info](
                              pandora::core::TransferCommandBuffer& cmd) {
    cmd.copyImageToBuffer(src, dst, src_layout, view_info);
  });
  return *this;
}

pandora::core::VoidResult TransferPlan::submit(
    const pandora::core::SubmitSemaphoreGroup& semaphore_group) {
  const auto& context = m_contextOwner.get();
  if (!context.isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("TransferPlan::submit");
  }

  commitBarriersInternal();

  if (m_commands.empty()) {
    return pandora::core::ok();
  }

  auto& driver = ensureDriver();
  driver.resetAllCommandPools(context);
  auto cmd = driver.getTransfer();
  cmd.begin();
  for (auto& record : m_commands) {
    record(cmd);
  }
  cmd.end();

  driver.submit(semaphore_group);
  return pandora::core::ok();
}

void TransferPlan::reset() {
  m_bufferBarriers.clear();
  m_imageBarriers.clear();
  m_commands.clear();
  m_hasPendingBarriers = false;
}

pandora::core::CommandDriver& ResourceTransfer::ensureDriver() {
  if (!m_transferDriver) {
    m_transferDriver = std::make_unique<pandora::core::CommandDriver>(
        m_contextOwner.get(), m_queueFamilyType);
  }
  return *m_transferDriver;
}

pandora::core::VoidResult ResourceTransfer::uploadBuffer(
    pandora::core::gpu::Buffer& dst, std::span<const std::byte> data) {
  const auto& context = m_contextOwner.get();
  if (!context.isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ResourceTransfer::uploadBuffer");
  }
  if (data.size_bytes() > dst.getSize()) {
    return pandora::core::Error::validation(
               "Upload size exceeds destination buffer size")
        .withContext("ResourceTransfer::uploadBuffer");
  }

  auto staging =
      pandora::core::createStagingBufferToGPU(context, data.size_bytes());
  writeToStagingBuffer(staging, context, data);

  auto& driver = ensureDriver();
  submitTransfer(
      driver, context, [&](pandora::core::TransferCommandBuffer& cmd) {
        cmd.copyBuffer(staging, dst);
      });

  return pandora::core::ok();
}

pandora::core::VoidResult ResourceTransfer::uploadImage(
    pandora::core::gpu::Image& dst,
    const pandora::core::ImageViewInfo& view_info,
    std::span<const std::byte> data) {
  const auto& context = m_contextOwner.get();
  if (!context.isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ResourceTransfer::uploadImage");
  }
  auto staging =
      pandora::core::createStagingBufferToGPU(context, data.size_bytes());
  writeToStagingBuffer(staging, context, data);

  auto& driver = ensureDriver();
  submitTransfer(
      driver, context, [&](pandora::core::TransferCommandBuffer& cmd) {
        cmd.copyBufferToImage(staging,
                              dst,
                              pandora::core::ImageLayout::TransferDstOptimal,
                              view_info);
      });

  return pandora::core::ok();
}

pandora::core::VoidResult ResourceTransfer::readbackBuffer(
    pandora::core::gpu::Buffer& src, std::span<std::byte> out) {
  const auto& context = m_contextOwner.get();
  if (!context.isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ResourceTransfer::readbackBuffer");
  }
  if (out.size_bytes() > src.getSize()) {
    return pandora::core::Error::validation(
               "Readback size exceeds source buffer size")
        .withContext("ResourceTransfer::readbackBuffer");
  }

  auto staging =
      pandora::core::createStagingBufferFromGPU(context, out.size_bytes());

  auto& driver = ensureDriver();
  submitTransfer(
      driver, context, [&](pandora::core::TransferCommandBuffer& cmd) {
        cmd.copyBuffer(src, staging);
      });

  readFromStagingBuffer(staging, context, out);

  return pandora::core::ok();
}

}  // namespace pandora::highlevel
