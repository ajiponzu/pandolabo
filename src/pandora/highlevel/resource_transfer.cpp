#include "pandora/highlevel/resource_transfer.hpp"

#include <cstring>

namespace pandora::highlevel {

pandora::core::CommandDriver& ResourceTransfer::ensureDriver() {
  if (!m_transferDriver) {
    m_transferDriver = std::make_unique<pandora::core::CommandDriver>(
        m_contextOwner.get(), m_queueFamilyType);
  }
  return *m_transferDriver;
}

pandora::core::VoidResult ResourceTransfer::uploadBuffer(
    pandora::core::gpu::Buffer& dst, std::span<const std::byte> data) {
  if (!m_contextOwner.get().isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ResourceTransfer::uploadBuffer");
  }
  if (data.size_bytes() > dst.getSize()) {
    return pandora::core::Error::validation(
               "Upload size exceeds destination buffer size")
        .withContext("ResourceTransfer::uploadBuffer");
  }

  auto staging = pandora::core::createStagingBufferToGPU(m_contextOwner.get(),
                                                         data.size_bytes());

  auto* mapped = staging.mapMemory(m_contextOwner.get());
  std::memcpy(mapped, data.data(), data.size_bytes());
  staging.unmapMemory(m_contextOwner.get());

  auto& driver = ensureDriver();
  driver.resetAllCommandPools(m_contextOwner.get());
  auto cmd = driver.getTransfer();
  cmd.begin();
  cmd.copyBuffer(staging, dst);
  cmd.end();

  driver.submit(pandora::core::SubmitSemaphoreGroup{});
  driver.queueWaitIdle();

  return pandora::core::ok();
}

pandora::core::VoidResult ResourceTransfer::uploadImage(
    pandora::core::gpu::Image& dst,
    const pandora::core::ImageViewInfo& view_info,
    std::span<const std::byte> data) {
  if (!m_contextOwner.get().isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ResourceTransfer::uploadImage");
  }
  auto staging = pandora::core::createStagingBufferToGPU(m_contextOwner.get(),
                                                         data.size_bytes());

  auto* mapped = staging.mapMemory(m_contextOwner.get());
  std::memcpy(mapped, data.data(), data.size_bytes());
  staging.unmapMemory(m_contextOwner.get());

  auto& driver = ensureDriver();
  driver.resetAllCommandPools(m_contextOwner.get());
  auto cmd = driver.getTransfer();
  cmd.begin();
  cmd.copyBufferToImage(
      staging, dst, pandora::core::ImageLayout::TransferDstOptimal, view_info);
  cmd.end();

  driver.submit(pandora::core::SubmitSemaphoreGroup{});
  driver.queueWaitIdle();

  return pandora::core::ok();
}

pandora::core::VoidResult ResourceTransfer::readbackBuffer(
    pandora::core::gpu::Buffer& src, std::span<std::byte> out) {
  if (!m_contextOwner.get().isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ResourceTransfer::readbackBuffer");
  }
  if (out.size_bytes() > src.getSize()) {
    return pandora::core::Error::validation(
               "Readback size exceeds source buffer size")
        .withContext("ResourceTransfer::readbackBuffer");
  }

  auto staging = pandora::core::createStagingBufferFromGPU(m_contextOwner.get(),
                                                           out.size_bytes());

  auto& driver = ensureDriver();
  driver.resetAllCommandPools(m_contextOwner.get());
  auto cmd = driver.getTransfer();
  cmd.begin();
  cmd.copyBuffer(src, staging);
  cmd.end();

  driver.submit(pandora::core::SubmitSemaphoreGroup{});
  driver.queueWaitIdle();

  const auto* mapped = staging.mapMemory(m_contextOwner.get());
  std::memcpy(out.data(), mapped, out.size_bytes());
  staging.unmapMemory(m_contextOwner.get());

  return pandora::core::ok();
}

}  // namespace pandora::highlevel
