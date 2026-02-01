#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "pandora/core/buffer_helpers.hpp"
#include "pandora/core/command_buffer.hpp"
#include "pandora/core/error.hpp"
#include "pandora/core/gpu.hpp"

namespace pandora::highlevel {

/// @brief High-level transfer plan for barriers, copies, and submission.
class TransferPlan {
 private:
  std::reference_wrapper<const pandora::core::gpu::Context> m_contextOwner;
  pandora::core::QueueFamilyType m_queueFamilyType;
  std::unique_ptr<pandora::core::CommandDriver> m_transferDriver;
  std::vector<pandora::core::gpu::BufferBarrier> m_bufferBarriers;
  std::vector<pandora::core::gpu::ImageBarrier> m_imageBarriers;
  std::vector<std::function<void(pandora::core::TransferCommandBuffer&)>>
      m_commands;
  bool m_hasPendingBarriers = false;

  pandora::core::CommandDriver& ensureDriver();
  void commitBarriersInternal();

 public:
  explicit TransferPlan(const pandora::core::gpu::Context& context,
                        pandora::core::QueueFamilyType queue_family_type =
                            pandora::core::QueueFamilyType::Transfer)
      : m_contextOwner(context), m_queueFamilyType(queue_family_type) {}

  /// @brief Add a buffer barrier to the pending batch.
  [[nodiscard]] pandora::core::VoidResult addBufferBarrier(
      const pandora::core::gpu::Buffer& buffer,
      const std::vector<pandora::core::AccessFlag>& src_access,
      const std::vector<pandora::core::AccessFlag>& dst_access,
      const std::vector<pandora::core::PipelineStage>& src_stages,
      const std::vector<pandora::core::PipelineStage>& dst_stages,
      std::optional<uint32_t> src_queue_family = std::nullopt,
      std::optional<uint32_t> dst_queue_family = std::nullopt);

  /// @brief Add an image barrier to the pending batch.
  [[nodiscard]] pandora::core::VoidResult addImageBarrier(
      const pandora::core::gpu::Image& image,
      const pandora::core::ImageViewInfo& view_info,
      pandora::core::ImageLayout old_layout,
      pandora::core::ImageLayout new_layout,
      const std::vector<pandora::core::AccessFlag>& src_access,
      const std::vector<pandora::core::AccessFlag>& dst_access,
      const std::vector<pandora::core::PipelineStage>& src_stages,
      const std::vector<pandora::core::PipelineStage>& dst_stages,
      std::optional<uint32_t> src_queue_family = std::nullopt,
      std::optional<uint32_t> dst_queue_family = std::nullopt);

  /// @brief Add a backbuffer barrier to the pending batch.
  [[nodiscard]] pandora::core::VoidResult addBackbufferBarrier(
      pandora::core::ImageLayout old_layout,
      pandora::core::ImageLayout new_layout,
      const std::vector<pandora::core::AccessFlag>& src_access,
      const std::vector<pandora::core::AccessFlag>& dst_access,
      const std::vector<pandora::core::PipelineStage>& src_stages,
      const std::vector<pandora::core::PipelineStage>& dst_stages,
      std::optional<uint32_t> src_queue_family = std::nullopt,
      std::optional<uint32_t> dst_queue_family = std::nullopt);

  /// @brief Emit a pipeline barrier command for the current pending batch.
  void flushBarriers();

  /// @brief Record a buffer copy command.
  TransferPlan& copyBuffer(const pandora::core::gpu::Buffer& src,
                           const pandora::core::gpu::Buffer& dst);

  /// @brief Record a buffer-to-image copy command.
  TransferPlan& copyBufferToImage(
      const pandora::core::gpu::Buffer& src,
      const pandora::core::gpu::Image& dst,
      pandora::core::ImageLayout dst_layout,
      const pandora::core::ImageViewInfo& view_info);

  /// @brief Record an image-to-buffer copy command.
  TransferPlan& copyImageToBuffer(
      const pandora::core::gpu::Image& src,
      const pandora::core::gpu::Buffer& dst,
      pandora::core::ImageLayout src_layout,
      const pandora::core::ImageViewInfo& view_info);

  /// @brief Submit recorded commands on the transfer queue.
  [[nodiscard]] pandora::core::VoidResult submit(
      const pandora::core::SubmitSemaphoreGroup& semaphore_group = {});

  /// @brief Clear pending barriers and recorded commands.
  void reset();
};

/// @brief Template transfer utility for upload/readback.
class ResourceTransfer {
 private:
  std::reference_wrapper<const pandora::core::gpu::Context> m_contextOwner;
  std::unique_ptr<pandora::core::CommandDriver> m_transferDriver;
  pandora::core::QueueFamilyType m_queueFamilyType =
      pandora::core::QueueFamilyType::Transfer;

  pandora::core::CommandDriver& ensureDriver();

 public:
  explicit ResourceTransfer(const pandora::core::gpu::Context& context,
                            pandora::core::QueueFamilyType queue_family_type =
                                pandora::core::QueueFamilyType::Transfer)
      : m_contextOwner(context), m_queueFamilyType(queue_family_type) {}

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
