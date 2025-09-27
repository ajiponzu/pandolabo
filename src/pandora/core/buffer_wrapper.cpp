#include "pandora/core/buffer_helpers.hpp"

namespace pandora::core::detail {

err::Result<std::unique_ptr<gpu::Buffer>> makeBuffer(
    const std::unique_ptr<gpu::Context>& ctx,
    MemoryUsage usage,
    TransferType transfer,
    std::vector<BufferUsage> buffer_usages,
    size_t size,
    const char* category) {
  if (!ctx) {
    return std::unexpected(err::Error{err::Domain::gpu,
                                      err::Code::invalid_state,
                                      err::Severity::fatal,
                                      "null context in makeBuffer"});
  }
  try {
    auto ptr = std::make_unique<gpu::Buffer>(
        ctx, usage, transfer, buffer_usages, size);
    return ptr;
  } catch (const vk::SystemError& se) {
    err::Error e{
        err::Domain::gpu,
        err::Code::unknown,
        err::Severity::recoverable,
        std::string("vk::SystemError in ") + category + ": " + se.what()};
    ::pandora::core::err::dispatch_error(e);
    return std::unexpected(e);
  } catch (const std::exception& ex) {
    err::Error e{err::Domain::gpu,
                 err::Code::unknown,
                 err::Severity::recoverable,
                 std::string("exception in ") + category + ": " + ex.what()};
    err::dispatch_error(e);

    return std::unexpected(e);
  }
}
}  // namespace pandora::core::detail

namespace pandora::core {

gpu::Buffer createStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  // Legacy path (may throw). Prefer makeStagingBufferToGPU.
  return gpu::Buffer(ptr_context,
                     MemoryUsage::CpuToGpu,
                     TransferType::TransferSrc,
                     std::vector<BufferUsage>{BufferUsage::StagingBuffer},
                     size);
}

err::Result<std::unique_ptr<gpu::Buffer>> makeStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return detail::makeBuffer(ptr_context,
                            MemoryUsage::CpuToGpu,
                            TransferType::TransferSrc,
                            {BufferUsage::StagingBuffer},
                            size,
                            "makeStagingBufferToGPU");
}

std::unique_ptr<gpu::Buffer> createUniqueStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  auto r = makeStagingBufferToGPU(ptr_context, size);
  if (!r) {
#if defined(PLB_NO_EXCEPTIONS)
    return {};
#else
    ::pandora::core::err::throw_error(r.error());
#endif
  }
  return std::move(r.value());
}

gpu::Buffer createStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuToCpu,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::StagingBuffer},
                     size);
}

err::Result<std::unique_ptr<gpu::Buffer>> makeStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return detail::makeBuffer(ptr_context,
                            MemoryUsage::GpuToCpu,
                            TransferType::TransferDst,
                            {BufferUsage::StagingBuffer},
                            size,
                            "makeStagingBufferFromGPU");
}

std::unique_ptr<gpu::Buffer> createUniqueStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  auto r = makeStagingBufferFromGPU(ptr_context, size);
  if (!r) {
#if defined(PLB_NO_EXCEPTIONS)
    return {};
#else
    ::pandora::core::err::throw_error(r.error());
#endif
  }
  return std::move(r.value());
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

err::Result<std::unique_ptr<gpu::Buffer>> makeStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size) {
  return detail::makeBuffer(ptr_context,
                            MemoryUsage::GpuOnly,
                            transfer_type,
                            {BufferUsage::StorageBuffer},
                            size,
                            "makeStorageBuffer");
}

std::unique_ptr<gpu::Buffer> createUniqueStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size) {
  auto r = makeStorageBuffer(ptr_context, transfer_type, size);
  if (!r) {
#if defined(PLB_NO_EXCEPTIONS)
    return {};
#else
    ::pandora::core::err::throw_error(r.error());
#endif
  }
  return std::move(r.value());
}

gpu::Buffer createUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::CpuToGpu,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::UniformBuffer},
                     size);
}

err::Result<std::unique_ptr<gpu::Buffer>> makeUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return detail::makeBuffer(ptr_context,
                            MemoryUsage::CpuToGpu,
                            TransferType::TransferDst,
                            {BufferUsage::UniformBuffer},
                            size,
                            "makeUniformBuffer");
}

std::unique_ptr<gpu::Buffer> createUniqueUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  auto r = makeUniformBuffer(ptr_context, size);
  if (!r) {
#if defined(PLB_NO_EXCEPTIONS)
    return {};
#else
    ::pandora::core::err::throw_error(r.error());
#endif
  }
  return std::move(r.value());
}

gpu::Buffer createVertexBuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                               size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::VertexBuffer},
                     size);
}

err::Result<std::unique_ptr<gpu::Buffer>> makeVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return detail::makeBuffer(ptr_context,
                            MemoryUsage::GpuOnly,
                            TransferType::TransferDst,
                            {BufferUsage::VertexBuffer},
                            size,
                            "makeVertexBuffer");
}

std::unique_ptr<gpu::Buffer> createUniqueVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  auto r = makeVertexBuffer(ptr_context, size);
  if (!r) {
#if defined(PLB_NO_EXCEPTIONS)
    return {};
#else
    ::pandora::core::err::throw_error(r.error());
#endif
  }
  return std::move(r.value());
}

gpu::Buffer createIndexBuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                              size_t size) {
  return gpu::Buffer(ptr_context,
                     MemoryUsage::GpuOnly,
                     TransferType::TransferDst,
                     std::vector<BufferUsage>{BufferUsage::IndexBuffer},
                     size);
}

err::Result<std::unique_ptr<gpu::Buffer>> makeIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  return detail::makeBuffer(ptr_context,
                            MemoryUsage::GpuOnly,
                            TransferType::TransferDst,
                            {BufferUsage::IndexBuffer},
                            size,
                            "makeIndexBuffer");
}

std::unique_ptr<gpu::Buffer> createUniqueIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size) {
  auto r = makeIndexBuffer(ptr_context, size);
  if (!r) {
#if defined(PLB_NO_EXCEPTIONS)
    return {};
#else
    ::pandora::core::err::throw_error(r.error());
#endif
  }
  return std::move(r.value());
}

}  // namespace pandora::core
