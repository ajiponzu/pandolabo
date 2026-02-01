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

#include "gpu.hpp"
#include "types.hpp"

namespace pandora::core {

namespace gpu {
class Context;
class Buffer;
class ShaderModule;
}  // namespace gpu

/* Buffer creation helper functions */

/// @brief Create staging buffer for CPU to GPU data transfer
/// Creates a buffer optimized for transferring data from CPU memory to GPU
/// memory.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for CPU-to-GPU transfer
gpu::Buffer createStagingBufferToGPU(const gpu::Context& context, size_t size);

/// @brief Create staging buffer for CPU to GPU data transfer (unique_ptr
/// version) Provides explicit ownership management for cases where heap
/// allocation is needed.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for CPU-to-GPU transfer
std::unique_ptr<gpu::Buffer> createUniqueStagingBufferToGPU(
    const gpu::Context& context, size_t size);

/// @brief Create staging buffer for GPU to CPU data transfer
/// Creates a buffer optimized for transferring data from GPU memory to CPU
/// memory.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for GPU-to-CPU transfer
gpu::Buffer createStagingBufferFromGPU(const gpu::Context& context,
                                       size_t size);

/// @brief Create staging buffer for GPU to CPU data transfer (unique_ptr
/// version) Provides explicit ownership management for cases where heap
/// allocation is needed.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for GPU-to-CPU transfer
std::unique_ptr<gpu::Buffer> createUniqueStagingBufferFromGPU(
    const gpu::Context& context, size_t size);

/// @brief Create storage buffer for compute shader operations
/// Creates a buffer that can be read and written by compute shaders.
/// @param context GPU context for device access
/// @param transfer_type Transfer capabilities needed for this buffer
/// @param size Buffer size in bytes
/// @return Buffer object configured for storage operations
gpu::Buffer createStorageBuffer(const gpu::Context& context,
                                TransferType transfer_type,
                                size_t size);

/// @brief Create storage buffer for compute shader operations (unique_ptr
/// version) Provides explicit ownership management for cases where heap
/// allocation is needed.
/// @param context GPU context for device access
/// @param transfer_type Transfer capabilities needed for this buffer
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for storage operations
std::unique_ptr<gpu::Buffer> createUniqueStorageBuffer(
    const gpu::Context& context, TransferType transfer_type, size_t size);

/// @brief Create uniform buffer for shader uniform data
/// Creates a buffer optimized for small, frequently updated uniform data.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for uniform data
gpu::Buffer createUniformBuffer(const gpu::Context& context, size_t size);

/// @brief Create uniform buffer for shader uniform data (unique_ptr version)
/// Provides explicit ownership management for cases where heap allocation is
/// needed.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for uniform data
std::unique_ptr<gpu::Buffer> createUniqueUniformBuffer(
    const gpu::Context& context, size_t size);

/// @brief Create vertex buffer for vertex data
/// Creates a buffer optimized for storing vertex attribute data.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for vertex data
gpu::Buffer createVertexBuffer(const gpu::Context& context, size_t size);

/// @brief Create vertex buffer for vertex data (unique_ptr version)
/// Provides explicit ownership management for cases where heap allocation is
/// needed.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for vertex data
std::unique_ptr<gpu::Buffer> createUniqueVertexBuffer(
    const gpu::Context& context, size_t size);

/// @brief Create index buffer for indexed rendering
/// Creates a buffer optimized for storing vertex indices for indexed drawing.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Buffer object configured for index data
gpu::Buffer createIndexBuffer(const gpu::Context& context, size_t size);

/// @brief Create index buffer for indexed rendering (unique_ptr version)
/// Provides explicit ownership management for cases where heap allocation is
/// needed.
/// @param context GPU context for device access
/// @param size Buffer size in bytes
/// @return Unique pointer to buffer object configured for index data
std::unique_ptr<gpu::Buffer> createUniqueIndexBuffer(
    const gpu::Context& context, size_t size);

/* End: Buffer creation helper functions */

/// @brief Type alias for shader module collections
/// Maps shader names to their corresponding shader module objects
using ShaderModuleMap = std::unordered_map<std::string, gpu::ShaderModule>;

}  // namespace pandora::core
