#include "pandora/core/buffer_helpers.hpp"

pandora::core::gpu::Buffer pandora::core::createStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::CpuToGpu,
                     TransferType::TransferSrc,
                     std::vector<BufferUsage>{BufferUsage::StagingBuffer},
                     size);
}

std::unique_ptr<pandora::core::gpu::Buffer>
pandora::core::createUniqueStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::CpuToGpu,
      TransferType::TransferSrc,
      std::vector<BufferUsage>{BufferUsage::StagingBuffer},
      size);
}

pandora::core::gpu::Buffer pandora::core::createStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuToCpu,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::StagingBuffer},
                     size);
}

std::unique_ptr<pandora::core::gpu::Buffer>
pandora::core::createUniqueStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuToCpu,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::StagingBuffer},
      size);
}

pandora::core::gpu::Buffer pandora::core::createStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     transfer_type,
                     std::vector<BufferUsage>{BufferUsage::StorageBuffer},
                     size);
}

std::unique_ptr<pandora::core::gpu::Buffer>
pandora::core::createUniqueStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuOnly,
      transfer_type,
      std::vector<BufferUsage>{BufferUsage::StorageBuffer},
      size);
}

pandora::core::gpu::Buffer pandora::core::createUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::CpuToGpu,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::UniformBuffer},
                     size);
}

std::unique_ptr<pandora::core::gpu::Buffer>
pandora::core::createUniqueUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::CpuToGpu,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::UniformBuffer},
      size);
}

pandora::core::gpu::Buffer pandora::core::createVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::VertexBuffer},
                     size);
}

std::unique_ptr<pandora::core::gpu::Buffer>
pandora::core::createUniqueVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuOnly,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::VertexBuffer},
      size);
}

pandora::core::gpu::Buffer pandora::core::createIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::IndexBuffer},
                     size);
}

std::unique_ptr<pandora::core::gpu::Buffer>
pandora::core::createUniqueIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuOnly,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::IndexBuffer},
      size);
}
