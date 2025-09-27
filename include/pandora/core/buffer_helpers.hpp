/*
 * buffer_helpers.hpp - Buffer creation helper functions for Pandolabo core
 * module
 *
 * This header contains utility functions for creating various types of GPU
 * buffers including staging, storage, uniform, vertex, and index buffers.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "err.hpp"
#include "gpu.hpp"
#include "types.hpp"

namespace pandora::core {

namespace gpu {
class Context;
class Buffer;
class ShaderModule;
}  // namespace gpu

/* Buffer creation helper functions */

#include "pandora/core/err/result.hpp"

// New Result-based API (non-throw path friendly). Returns unique_ptr ownership.
// Naming: makeXBuffer() to avoid collision with existing createXBuffer.
// Policy: does NOT throw when PLB_NO_EXCEPTIONS is defined; instead returns
// unexpected(Error).

namespace detail {

// Internal generic builder used by all public helpers.
err::Result<std::unique_ptr<gpu::Buffer>> makeBuffer(
    const std::unique_ptr<gpu::Context>& ctx,
    MemoryUsage usage,
    TransferType transfer,
    std::vector<BufferUsage> buffer_usages,
    size_t size,
    const char* category);
}  // namespace detail

/// @brief Create staging buffer for CPU to GPU data transfer
/// Creates a buffer optimized for transferring data from CPU memory to GPU
/// memory.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for CPU-to-GPU transfer
[[deprecated("Use makeStagingBufferToGPU (Result-returning) instead")]]
gpu::Buffer createStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

err::Result<std::unique_ptr<gpu::Buffer>> makeStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create staging buffer for CPU to GPU data transfer (unique_ptr
/// version) Provides explicit ownership management for cases where heap
/// allocation is needed.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for CPU-to-GPU transfer
[[deprecated("Use makeStagingBufferToGPU returning Result instead")]]
std::unique_ptr<gpu::Buffer> createUniqueStagingBufferToGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create staging buffer for GPU to CPU data transfer
/// Creates a buffer optimized for transferring data from GPU memory to CPU
/// memory.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for GPU-to-CPU transfer
[[deprecated("Use makeStagingBufferFromGPU (Result-returning) instead")]]
gpu::Buffer createStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

err::Result<std::unique_ptr<gpu::Buffer>> makeStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create staging buffer for GPU to CPU data transfer (unique_ptr
/// version) Provides explicit ownership management for cases where heap
/// allocation is needed.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for GPU-to-CPU transfer
[[deprecated("Use makeStagingBufferFromGPU returning Result instead")]]
std::unique_ptr<gpu::Buffer> createUniqueStagingBufferFromGPU(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create storage buffer for compute shader operations
/// Creates a buffer that can be read and written by compute shaders.
/// @param ptr_context GPU context for device access
/// @param transfer_type Transfer capabilities needed for this buffer
/// @param size Buffer size in bytes
/// @return Buffer object configured for storage operations
[[deprecated("Use makeStorageBuffer (Result-returning) instead")]]
gpu::Buffer createStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size);

err::Result<std::unique_ptr<gpu::Buffer>> makeStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size);

/// @brief Create storage buffer for compute shader operations (unique_ptr
/// version) Provides explicit ownership management for cases where heap
/// allocation is needed.
/// @param ptr_context GPU context for device access
/// @param transfer_type Transfer capabilities needed for this buffer
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for storage operations
[[deprecated("Use makeStorageBuffer returning Result instead")]]
std::unique_ptr<gpu::Buffer> createUniqueStorageBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context,
    TransferType transfer_type,
    size_t size);

/// @brief Create uniform buffer for shader uniform data
/// Creates a buffer optimized for small, frequently updated uniform data.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for uniform data
[[deprecated("Use makeUniformBuffer (Result-returning) instead")]]
gpu::Buffer createUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

err::Result<std::unique_ptr<gpu::Buffer>> makeUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create uniform buffer for shader uniform data (unique_ptr version)
/// Provides explicit ownership management for cases where heap allocation is
/// needed.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for uniform data
[[deprecated("Use makeUniformBuffer returning Result instead")]]
std::unique_ptr<gpu::Buffer> createUniqueUniformBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create vertex buffer for vertex data
/// Creates a buffer optimized for storing vertex attribute data.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for vertex data
[[deprecated("Use makeVertexBuffer (Result-returning) instead")]]
gpu::Buffer createVertexBuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                               size_t size);

err::Result<std::unique_ptr<gpu::Buffer>> makeVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create vertex buffer for vertex data (unique_ptr version)
/// Provides explicit ownership management for cases where heap allocation is
/// needed.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for vertex data
[[deprecated("Use makeVertexBuffer returning Result instead")]]
std::unique_ptr<gpu::Buffer> createUniqueVertexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create index buffer for indexed rendering
/// Creates a buffer optimized for storing vertex indices for indexed drawing.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for index data
[[deprecated("Use makeIndexBuffer (Result-returning) instead")]]
gpu::Buffer createIndexBuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                              size_t size);

err::Result<std::unique_ptr<gpu::Buffer>> makeIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/// @brief Create index buffer for indexed rendering (unique_ptr version)
/// Provides explicit ownership management for cases where heap allocation is
/// needed.
/// @param ptr_context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for index data
[[deprecated("Use makeIndexBuffer returning Result instead")]]
std::unique_ptr<gpu::Buffer> createUniqueIndexBuffer(
    const std::unique_ptr<gpu::Context>& ptr_context, size_t size);

/* End: Buffer creation helper functions */

/// @brief Type alias for shader module collections
/// Maps shader names to their corresponding shader module objects
using ShaderModuleMap = std::unordered_map<std::string, gpu::ShaderModule>;

}  // namespace pandora::core
