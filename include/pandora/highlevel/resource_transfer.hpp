#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <span>

#include "pandora/core/buffer_helpers.hpp"
#include "pandora/core/command_buffer.hpp"
#include "pandora/core/error.hpp"
#include "pandora/core/gpu.hpp"

namespace pandora::highlevel {

/// @brief Template transfer utility for upload/readback.
class ResourceTransfer {
 private:
  std::reference_wrapper<const std::unique_ptr<pandora::core::gpu::Context>>
      m_contextOwner;
  std::unique_ptr<pandora::core::CommandDriver> m_transferDriver;
  pandora::core::QueueFamilyType m_queueFamilyType =
      pandora::core::QueueFamilyType::Transfer;

  pandora::core::CommandDriver& ensureDriver();

 public:
  explicit ResourceTransfer(
      const std::unique_ptr<pandora::core::gpu::Context>& ptr_context,
      pandora::core::QueueFamilyType queue_family_type =
          pandora::core::QueueFamilyType::Transfer)
      : m_contextOwner(ptr_context), m_queueFamilyType(queue_family_type) {}

  /// @brief Upload data to a GPU buffer via staging.
  [[nodiscard]] pandora::core::VoidResult uploadBuffer(
      pandora::core::gpu::Buffer& dst, std::span<const std::byte> data);

  /// @brief Upload data to a GPU image via staging.
  [[nodiscard]] pandora::core::VoidResult uploadImage(
      pandora::core::gpu::Image& dst,
      const pandora::core::ImageViewInfo& view_info,
      std::span<const std::byte> data);

  /// @brief Read back buffer data into CPU memory.
  [[nodiscard]] pandora::core::VoidResult readbackBuffer(
      pandora::core::gpu::Buffer& src, std::span<std::byte> out);
};

}  // namespace pandora::highlevel
