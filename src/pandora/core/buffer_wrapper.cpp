#include "pandora/core/buffer_helpers.hpp"

namespace pandora::core {

gpu::Buffer createStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::CpuToGpu,
                     TransferType::TransferSrc,
                     std::vector<BufferUsage>{BufferUsage::StagingBuffer},
                     size);
}

std::unique_ptr<gpu::Buffer> createUniqueStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::CpuToGpu,
      TransferType::TransferSrc,
      std::vector<BufferUsage>{BufferUsage::StagingBuffer},
      size);
}

gpu::Buffer createStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuToCpu,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::StagingBuffer},
                     size);
}

std::unique_ptr<gpu::Buffer> createUniqueStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuToCpu,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::StagingBuffer},
      size);
}

gpu::Buffer createStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     transfer_type,
                     std::vector<BufferUsage>{BufferUsage::StorageBuffer},
                     size);
}

std::unique_ptr<gpu::Buffer> createUniqueStorageBuffer(
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

gpu::Buffer createUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::CpuToGpu,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::UniformBuffer},
                     size);
}

std::unique_ptr<gpu::Buffer> createUniqueUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::CpuToGpu,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::UniformBuffer},
      size);
}

gpu::Buffer createVertexBuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                               size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::VertexBuffer},
                     size);
}

std::unique_ptr<gpu::Buffer> createUniqueVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuOnly,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::VertexBuffer},
      size);
}

gpu::Buffer createIndexBuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                              size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::IndexBuffer},
                     size);
}

std::unique_ptr<gpu::Buffer> createUniqueIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return std::make_unique<gpu::Buffer>(
      ptr_context,
      MemoryUsage::GpuOnly,
      TransferType::TransferDst,
      std::vector<BufferUsage>{BufferUsage::IndexBuffer},
      size);
}

}  // namespace pandora::core
