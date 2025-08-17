/*
 * buffer.hpp - GPU buffer wrapper for Pandolabo Vulkan C++ wrapper
 *
 * This header contains the Buffer class which manages Vulkan buffer resources.
 * Buffers are used for vertex data, uniform data, storage, and data transfer operations.
 */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../types.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Context;
}

namespace pandora::core::gpu {

/// @brief GPU buffer wrapper class
/// @details GPU buffer is used for simple numbers or matrices for values, vertices, indices, uniforms, etc.
/// This class is also used to transfer data between CPU and GPU.
/// Buffer size unit is 1 byte. For example, if you want a float matrix 4x4,
/// float size is 4 bytes, so 4 * 4 * 4 = 64 bytes are needed.
class Buffer {
 protected:
  vk::UniqueDeviceMemory m_ptrMemory;
  vk::UniqueBuffer m_ptrBuffer;
  size_t m_size = 0u;

 public:
  Buffer() = default;

  /// @brief Construct buffer with specified properties
  /// @param ptr_context GPU context pointer
  /// @param memory_usage Memory usage pattern
  /// @param transfer_type Transfer operation type
  /// @param buffer_usages Buffer usage types
  /// @param size Buffer size in bytes
  Buffer(const std::unique_ptr<Context>& ptr_context,
         const MemoryUsage memory_usage,
         const TransferType transfer_type,
         const std::vector<BufferUsage>& buffer_usages,
         const size_t size);
  ~Buffer();

  // Explicitly delete copy operations to ensure RAII safety
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  /// @brief Move constructor
  /// @param other Buffer to move from
  Buffer(Buffer&& other) noexcept {
    m_ptrBuffer = std::move(other.m_ptrBuffer);
    m_ptrMemory = std::move(other.m_ptrMemory);
    m_size = other.m_size;
  }

  /// @brief Move assignment operator
  /// @param other Buffer to move from
  /// @return Reference to this buffer
  Buffer& operator=(Buffer&& other) noexcept {
    m_ptrBuffer = std::move(other.m_ptrBuffer);
    m_ptrMemory = std::move(other.m_ptrMemory);
    m_size = other.m_size;
    return *this;
  }

  /// @brief Get buffer unique pointer
  /// @return Reference to buffer unique pointer
  const auto& getPtrBuffer() const {
    return m_ptrBuffer;
  }

  /// @brief Get buffer handle
  /// @return Vulkan buffer handle
  const auto& getBuffer() const {
    return m_ptrBuffer.get();
  }

  /// @brief Get buffer size
  /// @return Buffer size in bytes
  auto getSize() const {
    return m_size;
  }

  /// @brief Get virtual address of mapped GPU buffer memory
  /// @details Writing or reading data at this address is directly reflected in GPU memory.
  /// @param ptr_context GPU context pointer
  /// @return Virtual GPU buffer memory address
  void* mapMemory(const std::unique_ptr<Context>& ptr_context) const;

  /// @brief Close GPU memory buffer connection
  /// @param ptr_context GPU context pointer
  void unmapMemory(const std::unique_ptr<Context>& ptr_context) const;
};

}  // namespace pandora::core::gpu
