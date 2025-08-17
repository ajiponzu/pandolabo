/*
 * command_buffer.hpp - Command buffer and command management for Pandolabo core module
 *
 * This header contains classes for managing GPU command buffers,
 * command execution, and various command types (graphics, compute, transfer).
 */

#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "gpu.hpp"
#include "module_connection/gpu_ui.hpp"
#include "pipeline.hpp"
#include "rendering_structures.hpp"
#include "rendering_types.hpp"
#include "renderpass.hpp"
#include "structures.hpp"
#include "types.hpp"

// Forward declarations
namespace pandora::core {
class RenderKit;
}

namespace pandora::core::gpu {
class Context;
class Buffer;
class Image;
class BufferBarrier;
class ImageBarrier;
class DescriptorSet;
class TimelineSemaphore;
class BinarySemaphore;
}  // namespace pandora::core::gpu

namespace pandora::core {

/// @brief Command buffer begin information configuration
/// Specifies how a command buffer should be started, including usage flags
/// and optional render pass inheritance for secondary command buffers.
class CommandBeginInfo {
 private:
  vk::RenderPass m_renderpass{};    ///< Render pass for inheritance (secondary buffers)
  vk::Framebuffer m_framebuffer{};  ///< Framebuffer for inheritance (secondary buffers)

 public:
  CommandBufferUsage usage = CommandBufferUsage::OneTimeSubmit;  ///< How the command buffer will be used
  uint32_t subpass_index = 0u;                                   ///< Subpass index for inheritance

  // Rule of Zero
  CommandBeginInfo() = default;

  void setRenderPass(const Renderpass& renderpass) {
    m_renderpass = renderpass.getRenderPass();
  }

  void setFramebuffer(const Framebuffer& framebuffer) {
    m_framebuffer = framebuffer.getFrameBuffer();
  }

  const auto& getRenderPass() const {
    return m_renderpass;
  }
  const auto& getFramebuffer() const {
    return m_framebuffer;
  }
};

/// @brief Base command buffer interface for GPU command recording
/// Provides the fundamental interface for recording GPU commands into a Vulkan
/// command buffer. This class serves as the base for specialized command buffer
/// types (transfer, compute, graphics) and manages the underlying Vulkan command buffer.
/// @note This is not an abstract class - it can be instantiated directly for basic operations
class CommandBuffer {
 protected:
  friend class CommandDriver;

  vk::CommandBuffer m_commandBuffer;  ///< Underlying Vulkan command buffer handle
  bool m_isSecondary = false;         ///< Whether this is a secondary command buffer

  /// @brief Protected constructor for derived classes
  /// @param ptr_command_buffer Unique Vulkan command buffer to wrap
  /// @param is_secondary Whether this is a secondary command buffer
  CommandBuffer(const vk::UniqueCommandBuffer& ptr_command_buffer, const bool is_secondary = false)
      : m_commandBuffer(ptr_command_buffer.get()), m_isSecondary(is_secondary) {}

 public:
  // Rule of Five
  CommandBuffer() = default;
  ~CommandBuffer() = default;
  CommandBuffer(const CommandBuffer&) = delete;
  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer(CommandBuffer&&) = default;
  CommandBuffer& operator=(CommandBuffer&&) = default;
  /// @brief Begin recording commands into the command buffer
  /// @param begin_info Configuration for how the command buffer should be started
  /// @note This function must be called before recording any commands
  /// @param command_begin_info Command buffer begin configuration
  void begin(const CommandBeginInfo& command_begin_info = {}) const;

  /// @brief End GPU command recording
  /// This function must be called to finish command buffer recording.
  void end() const;

  /// @brief Set buffer access barrier
  /// @param barrier Buffer barrier configuration
  /// @param src_stage Source pipeline stage
  /// @param dst_stage Destination pipeline stage (wait stage)
  void setPipelineBarrier(const gpu::BufferBarrier& barrier,
                          const PipelineStage src_stage,
                          const PipelineStage dst_stage) const;

  /// @brief Set image access barrier
  /// @param barrier Image barrier configuration
  /// @param src_stage Source pipeline stage
  /// @param dst_stage Destination pipeline stage
  void setPipelineBarrier(const gpu::ImageBarrier& barrier,
                          const PipelineStage src_stage,
                          const PipelineStage dst_stage) const;

  /// @brief Bind pipeline to command buffer
  /// @param pipeline Pipeline to bind for subsequent draw/dispatch commands
  void bindPipeline(const Pipeline& pipeline) const;

  /// @brief Bind descriptor set to pipeline
  /// @param pipeline Pipeline that owns the descriptor set layout
  /// @param descriptor_set Descriptor set containing resource bindings
  void bindDescriptorSet(const Pipeline& pipeline, const gpu::DescriptorSet& descriptor_set) const;

  /// @brief Register push constants to pipeline
  /// Push constants provide a fast way to pass small amounts of data to shaders.
  /// @param pipeline Pipeline to receive push constants
  /// @param dst_stages Shader stages that will use the push constants
  /// @param offset Byte offset into push constant range
  /// @param data Push constant data as float values
  void pushConstants(const Pipeline& pipeline,
                     const std::vector<ShaderStage>& dst_stages,
                     const uint32_t offset,
                     const std::vector<float_t>& data) const;

  /// @brief Reset GPU command buffer
  /// Clears all recorded commands, preparing the buffer for new recording.
  void resetCommands() const;
};

/// @brief Transfer command buffer for data transfer operations
/// This class provides an interface for GPU transfer commands including buffer-to-buffer
/// copies, buffer-to-image transfers, and mipmap generation operations.
class TransferCommandBuffer : public CommandBuffer {
 protected:
  friend class CommandDriver;

  TransferCommandBuffer(const vk::UniqueCommandBuffer& ptr_command_buffer, const bool is_secondary = false)
      : CommandBuffer(ptr_command_buffer, is_secondary) {}

 public:
  // Rule of Five
  TransferCommandBuffer() = default;
  ~TransferCommandBuffer() = default;
  TransferCommandBuffer(const TransferCommandBuffer&) = delete;
  TransferCommandBuffer& operator=(const TransferCommandBuffer&) = delete;
  TransferCommandBuffer(TransferCommandBuffer&&) = default;
  TransferCommandBuffer& operator=(TransferCommandBuffer&&) = default;
  /// @brief Copy data between buffers
  /// Performs buffer-to-buffer copy operations, typically used for transferring
  /// data from CPU staging buffers to GPU device-local buffers.
  /// @param staging_buffer Source buffer containing data (typically CPU-accessible)
  /// @param dst_buffer Destination buffer (typically GPU device-local)
  void copyBuffer(const gpu::Buffer& staging_buffer, const gpu::Buffer& dst_buffer) const;

  /// @brief Copy CPU staging buffer data to GPU image
  /// @param buffer CPU buffer containing image data
  /// @param image GPU local image destination
  /// @param image_layout GPU image layout (TransferDstOptimal recommended)
  /// @param image_view_info Image subresource information
  void copyBufferToImage(const gpu::Buffer& buffer,
                         const gpu::Image& image,
                         const ImageLayout image_layout,
                         const ImageViewInfo& image_view_info) const;

  /// @brief Copy GPU image data to CPU staging buffer
  /// @param image GPU local image source
  /// @param buffer CPU staging buffer destination
  /// @param image_layout GPU image layout (TransferSrcOptimal recommended)
  /// @param image_view_info Image subresource information
  void copyImageToBuffer(const gpu::Image& image,
                         const gpu::Buffer& buffer,
                         const ImageLayout image_layout,
                         const ImageViewInfo& image_view_info) const;

  /// @brief Generate mipmaps for GPU image
  /// @param image Image to generate mipmaps for
  /// @param dst_stage Pipeline stage that will use the mipmapped image
  void setMipmaps(const gpu::Image& image, const PipelineStage dst_stage) const;

  /// @brief Transfer mipmap image ownership to another queue family
  /// Source owner command buffer must call this function for queue family ownership transfer.
  /// @param image Mipmapped image to transfer
  /// @param src_stage Source pipeline stage
  /// @param dst_stage Destination pipeline stage
  /// @param queue_family_index Queue family indices (first: source, second: destination)
  void transferMipmapImages(const gpu::Image& image,
                            const PipelineStage src_stage,
                            const PipelineStage dst_stage,
                            std::pair<uint32_t, uint32_t> queue_family_index) const;

  /// @brief Acquire mipmap image ownership from another queue family
  /// Destination owner command buffer must call this function for queue family ownership transfer.
  /// @param image Mipmapped image to acquire
  /// @param src_stage Source pipeline stage
  /// @param dst_stage Destination pipeline stage
  /// @param queue_family_index Queue family indices (first: source, second: destination)
  void acquireMipmapImages(const gpu::Image& image,
                           const PipelineStage src_stage,
                           const PipelineStage dst_stage,
                           std::pair<uint32_t, uint32_t> queue_family_index) const;
};

/// @brief Compute command buffer for compute shader operations
/// This class provides an interface for GPU compute commands, extending transfer
/// capabilities with compute shader execution functionality.
class ComputeCommandBuffer : public TransferCommandBuffer {
 protected:
  friend class CommandDriver;

  ComputeCommandBuffer(const vk::UniqueCommandBuffer& ptr_command_buffer, const bool is_secondary = false)
      : TransferCommandBuffer(ptr_command_buffer, is_secondary) {}

 public:
  // Rule of Five
  ComputeCommandBuffer() = default;
  ~ComputeCommandBuffer() = default;
  ComputeCommandBuffer(const ComputeCommandBuffer&) = delete;
  ComputeCommandBuffer& operator=(const ComputeCommandBuffer&) = delete;
  ComputeCommandBuffer(ComputeCommandBuffer&&) = default;
  ComputeCommandBuffer& operator=(ComputeCommandBuffer&&) = default;
  /// @brief Execute compute shader with specified work group size
  /// @param work_group_size Work group dimensions calculated from resource size
  ///                       and local_size declarations in the compute shader
  void compute(const ComputeWorkGroupSize& work_group_size) const;
};

/// @brief Graphics command buffer for rendering operations
/// This class provides a comprehensive interface for GPU graphics commands,
/// including render pass management, drawing operations, and dynamic state updates.
class GraphicCommandBuffer : public ComputeCommandBuffer {
 protected:
  friend class CommandDriver;

  GraphicCommandBuffer(const vk::UniqueCommandBuffer& ptr_command_buffer, const bool is_secondary = false)
      : ComputeCommandBuffer(ptr_command_buffer, is_secondary) {}

 public:
  // Rule of Five
  GraphicCommandBuffer() = default;
  ~GraphicCommandBuffer() = default;
  GraphicCommandBuffer(const GraphicCommandBuffer&) = delete;
  GraphicCommandBuffer& operator=(const GraphicCommandBuffer&) = delete;
  GraphicCommandBuffer(GraphicCommandBuffer&&) = default;
  GraphicCommandBuffer& operator=(GraphicCommandBuffer&&) = default;
  /* Dynamic state setters */
  /// @brief Set scissor rectangle for clipping
  /// @param size Scissor rectangle dimensions
  void setScissor(const gpu_ui::GraphicalSize<uint32_t>& size) const;

  /// @brief Set viewport transformation parameters
  /// @param size Viewport dimensions
  /// @param min_depth Minimum depth value (typically 0.0)
  /// @param max_depth Maximum depth value (typically 1.0)
  void setViewport(const gpu_ui::GraphicalSize<float_t>& size, const float_t min_depth, const float_t max_depth) const;
  /* End: dynamic state setters */

  /// @brief Bind vertex buffer for vertex input
  /// @param buffer Vertex buffer containing vertex data
  /// @param offset Byte offset into the buffer
  void bindVertexBuffer(const gpu::Buffer& buffer, const uint32_t& offset) const;

  /// @brief Bind index buffer for indexed drawing
  /// @param buffer Index buffer containing index data
  /// @param offset Byte offset into the buffer
  void bindIndexBuffer(const gpu::Buffer& buffer, const uint32_t& offset) const;

  /// @brief Draw vertices without indexing
  /// @param vertex_count Number of vertices to draw
  /// @param instance_count Number of instances to draw
  /// @param first_vertex First vertex to draw
  /// @param first_instance First instance to draw
  void draw(const uint32_t vertex_count,
            const uint32_t instance_count,
            const uint32_t first_vertex,
            const uint32_t first_instance) const;

  /// @brief Draw indexed vertices
  /// @param index_count Number of indices to draw
  /// @param instance_count Number of instances to draw
  /// @param first_index First index to draw
  /// @param vertex_offset Offset added to vertex indices
  /// @param first_instance First instance to draw
  void drawIndexed(const uint32_t index_count,
                   const uint32_t instance_count,
                   const uint32_t first_index,
                   const int32_t vertex_offset,
                   const uint32_t first_instance) const;

  /// @brief Begin render pass execution
  /// @param render_kit Render kit containing render pass and framebuffer
  /// @param render_area Rendering area dimensions
  /// @param subpass_contents How subpass commands are provided
  void beginRenderpass(const RenderKit& render_kit,
                       const gpu_ui::GraphicalSize<uint32_t>& render_area,
                       const SubpassContents subpass_contents) const;

  /// @brief End current render pass
  void endRenderpass() const;

  /// @brief Advance to next subpass
  /// @param subpass_contents How next subpass commands are provided
  void nextSubpass(const SubpassContents subpass_contents) const;
};

/// @brief Command driver for managing command buffers and execution
/// This class provides a comprehensive interface for command buffer management,
/// including primary and secondary command buffers, multi-threading support,
/// and command submission with synchronization primitives.
class CommandDriver {
 private:
  vk::Queue m_queue;                                               ///< Vulkan queue for command submission
  vk::UniqueCommandPool m_ptrCommandPool;                          ///< Primary command pool
  vk::UniqueCommandBuffer m_ptrPrimaryCommandBuffer;               ///< Primary command buffer
  std::vector<vk::UniqueCommandPool> m_ptrSecondaryCommandPools;   ///< Secondary command pools
  std::vector<vk::UniqueCommandBuffer> m_secondaryCommandBuffers;  ///< Secondary command buffers

  QueueFamilyType m_queueFamilyType;  ///< Queue family type (graphics, compute, transfer)
  uint32_t m_queueFamilyIndex;        ///< Queue family index

 public:
  /// @brief Construct command driver for specified queue family
  /// @param ptr_context Vulkan context for device operations
  /// @param queue_family Queue family type to use for commands
  CommandDriver(const std::unique_ptr<gpu::Context>& ptr_context, const QueueFamilyType queue_family);

  // Rule of Five
  ~CommandDriver();
  CommandDriver(const CommandDriver&) = delete;
  CommandDriver& operator=(const CommandDriver&) = delete;
  CommandDriver(CommandDriver&&) = default;
  CommandDriver& operator=(CommandDriver&&) = default;

  /// @brief Destroy all secondary command buffers
  void destroySecondary() {
    m_secondaryCommandBuffers.clear();
  }

  /// @brief Allocate secondary command buffers for multi-threading
  /// Secondary command buffers enable parallel command recording across multiple threads.
  /// @param ptr_context Vulkan context for device operations
  /// @param required_secondary_num Number of secondary command buffers to allocate
  void constructSecondary(const std::unique_ptr<gpu::Context>& ptr_context, const uint32_t required_secondary_num = 1u);

  /// @brief Reset all command buffers to initial state
  void resetAllCommands() const;

  /// @brief Reset all command pools
  /// @param ptr_context Vulkan context for device operations
  void resetAllCommandPools(const std::unique_ptr<gpu::Context>& ptr_context) const;

  /// @brief Integrate secondary commands into primary command buffer
  /// If secondary command buffers are used, this function must be called
  /// before the primary command buffer's end() command.
  void mergeSecondaryCommands() const;

  /// @brief Submit GPU commands with timeline semaphore synchronization
  /// @param dst_stage Pipeline stage to wait for
  /// @param semaphore Timeline semaphore for synchronization
  void submit(const PipelineStage dst_stage, gpu::TimelineSemaphore& semaphore) const;

  /// @brief Submit GPU commands for graphics with binary semaphore synchronization
  /// @param wait_semaphore Semaphore to wait on before execution
  /// @param dst_stage Pipeline stage to wait for
  /// @param signal_semaphore Semaphore to signal after completion
  void submit(gpu::BinarySemaphore& wait_semaphore,
              const PipelineStage dst_stage,
              gpu::BinarySemaphore& signal_semaphore) const;

  /// @brief Present rendered image to display
  /// @param ptr_context Vulkan context for presentation
  /// @param wait_semaphore Semaphore to wait for before presentation
  void present(const std::unique_ptr<gpu::Context>& ptr_context, gpu::BinarySemaphore& wait_semaphore) const;

  /// @brief Wait for all operations on this queue to complete
  void queueWaitIdle() const {
    m_queue.waitIdle();
  }

  /// @brief Get primary command buffer
  /// @return Primary command buffer wrapper
  CommandBuffer getPrimary() const {
    return CommandBuffer(m_ptrPrimaryCommandBuffer);
  }

  /// @brief Get graphics command buffer
  /// @param secondary_index Optional secondary buffer index for multi-threading
  /// @return Graphics command buffer wrapper
  GraphicCommandBuffer getGraphic(const std::optional<size_t> secondary_index = std::nullopt) const;

  /// @brief Get compute command buffer
  /// @param secondary_index Optional secondary buffer index for multi-threading
  /// @return Compute command buffer wrapper
  ComputeCommandBuffer getCompute(const std::optional<size_t> secondary_index = std::nullopt) const;

  /// @brief Get transfer command buffer
  /// @param secondary_index Optional secondary buffer index for multi-threading
  /// @return Transfer command buffer wrapper
  TransferCommandBuffer getTransfer(const std::optional<size_t> secondary_index = std::nullopt) const;

  /// @brief Get queue family type
  /// @return Queue family type used by this driver
  const auto& getQueueFamilyType() const {
    return m_queueFamilyType;
  }

  /// @brief Get queue family index
  /// @return Queue family index used by this driver
  const auto& getQueueFamilyIndex() const {
    return m_queueFamilyIndex;
  }
};

}  // namespace pandora::core
