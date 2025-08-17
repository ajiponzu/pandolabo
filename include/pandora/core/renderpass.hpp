/*
 * renderpass.hpp - Renderpass and framebuffer management for Pandolabo core
 * module
 *
 * This header contains classes for managing renderpasses, framebuffers,
 * attachments, and subpass configurations.
 */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "gpu.hpp"
#include "module_connection/gpu_ui.hpp"
#include "rendering_structures.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Context;
class ImageView;
}  // namespace pandora::core::gpu

namespace pandora::core {

/// @brief Attachment list manager for render pass configuration
/// Manages a collection of render targets (color, depth, stencil) and their
/// associated clear values. Provides functionality to add attachments with
/// different clear value types and manage backbuffer attachments for
/// presentation.
class AttachmentList {
 private:
  std::vector<vk::AttachmentDescription>
      m_descriptions;  ///< Vulkan attachment descriptions
  std::vector<vk::ImageView>
      m_attachments;  ///< Image views for each attachment
  std::vector<vk::ClearValue>
      m_clearValues;  ///< Clear values for each attachment
  size_t m_backbufferIndex =
      0u;  ///< Index of the current backbuffer attachment

 public:
  AttachmentList();
  ~AttachmentList();

  /// @brief Add color attachment with image view and clear color
  /// @param description Attachment configuration (format, load/store ops, etc.)
  /// @param image_view Image view to use as the attachment
  /// @param clear_value Color values to clear the attachment with
  /// @return Index of the added attachment
  uint32_t append(const AttachmentDescription& description,
                  const gpu::ImageView& image_view,
                  const ClearColor& clear_value);

  /// @brief Add depth/stencil attachment with image view and clear values
  /// @param description Attachment configuration (format, load/store ops, etc.)
  /// @param image_view Image view to use as the attachment
  /// @param clear_value Depth and stencil values to clear the attachment with
  /// @return Index of the added attachment
  uint32_t append(const AttachmentDescription& description,
                  const gpu::ImageView& image_view,
                  const ClearDepthStencil& clear_value);

  /// @brief Add attachment without specific image view (for backbuffer)
  /// @param description Attachment configuration (format, load/store ops, etc.)
  /// @param clear_value Color values to clear the attachment with
  /// @return Index of the added attachment
  uint32_t append(const AttachmentDescription& description,
                  const ClearColor& clear_value);

  /// @brief Get attachment descriptions for render pass creation
  /// @return Vector of Vulkan attachment descriptions
  const auto& getDescriptions() const {
    return m_descriptions;
  }

  /// @brief Get image views for framebuffer creation
  /// @return Vector of Vulkan image views
  const auto& getAttachments() const {
    return m_attachments;
  }

  /// @brief Get clear values for render pass begin
  /// @return Vector of clear values for each attachment
  const auto& getClearValues() const {
    return m_clearValues;
  }

  /// @brief Set backbuffer attachment for presentation
  /// @param ptr_context GPU context for swapchain access
  /// @param index Swapchain image index to use
  void setBackbufferAttachment(const std::unique_ptr<gpu::Context>& ptr_context,
                               const size_t index);

 private:
  /// @brief Internal method to add attachment with image view
  /// @param description Attachment configuration
  /// @param image_view Image view for the attachment
  /// @return Index of the added attachment
  uint32_t append(const AttachmentDescription& description,
                  const gpu::ImageView& image_view);

  /// @brief Internal method to add attachment description only
  /// @param description Attachment configuration to add
  const void appendDescription(const AttachmentDescription& description);
};

/// @brief Subpass node representing a single rendering subpass
/// Manages attachment references for input, color, resolve, depth/stencil,
/// and preserve attachments within a single subpass of a render pass.
/// Each subpass defines which attachments it reads from, writes to, and
/// preserves.
class SubpassNode {
 private:
  std::vector<vk::AttachmentReference>
      m_inputs;  ///< Input attachments (read-only)
  std::vector<vk::AttachmentReference> m_colors;  ///< Color attachments (write)
  std::vector<vk::AttachmentReference>
      m_resolves;  ///< Resolve attachments for MSAA
  std::shared_ptr<vk::AttachmentReference>
      m_ptrDepthStencil;              ///< Depth/stencil attachment
  std::vector<uint32_t> m_preserves;  ///< Preserved attachment indices

  vk::PipelineBindPoint
      m_bindPoint{};         ///< Pipeline bind point (graphics/compute)
  uint32_t m_viewMask = 0u;  ///< View mask for multiview rendering

 public:
  /// @brief Construct subpass node
  /// @param bind_point Pipeline bind point (graphics, compute, ray tracing)
  /// @param view_mask View mask for multiview rendering (0 for single view)
  SubpassNode(const PipelineBind bind_point, const uint32_t view_mask);
  ~SubpassNode();

  /// @brief Get input attachment references
  /// @return Vector of input attachment references (read-only attachments)
  const auto& getInputs() const {
    return m_inputs;
  }

  /// @brief Get color attachment references
  /// @return Vector of color attachment references (render targets)
  const auto& getColors() const {
    return m_colors;
  }

  /// @brief Get resolve attachment references
  /// @return Vector of resolve attachment references (MSAA resolve targets)
  const auto& getResolves() const {
    return m_resolves;
  }

  /// @brief Get depth/stencil attachment reference
  /// @return Shared pointer to depth/stencil attachment reference (nullptr if
  /// none)
  const auto& getPtrDepthStencil() const {
    return m_ptrDepthStencil;
  }

  /// @brief Get preserve attachment indices
  /// @return Vector of attachment indices to preserve during this subpass
  const auto& getPreserves() const {
    return m_preserves;
  }

  /// @brief Get pipeline bind point
  /// @return Pipeline bind point (graphics, compute, ray tracing)
  const auto& getBindPoint() const {
    return m_bindPoint;
  }

  /// @brief Get view mask for multiview rendering
  /// @return View mask (0 for single view rendering)
  const auto& getViewMask() const {
    return m_viewMask;
  }

  /// @brief Attach input attachment for reading
  /// @param attachment_ref Reference to attachment and layout for reading
  void attachInput(const AttachmentReference& attachment_ref) {
    m_inputs.push_back(convert(attachment_ref));
  }

  /// @brief Attach color attachment for rendering
  /// @param attachment_ref Reference to attachment and layout for color output
  void attachColor(const AttachmentReference& attachment_ref) {
    m_colors.push_back(convert(attachment_ref));
  }

  /// @brief Attach resolve attachment for MSAA resolve
  /// @param attachment_ref Reference to attachment and layout for MSAA resolve
  void attachResolve(const AttachmentReference& attachment_ref) {
    m_resolves.push_back(convert(attachment_ref));
  }

  /// @brief Attach depth/stencil attachment
  /// @param attachment_ref Reference to attachment and layout for depth/stencil
  /// operations
  void attachDepthStencil(const AttachmentReference& attachment_ref) {
    m_ptrDepthStencil =
        std::make_shared<vk::AttachmentReference>(convert(attachment_ref));
  }

  /// @brief Preserve attachment during this subpass
  /// @param attachment_index Index of attachment to preserve (not read or
  /// written)
  void attachPreserve(const uint32_t attachment_index) {
    m_preserves.push_back(attachment_index);
  }

 private:
  /// @brief Convert high-level attachment reference to Vulkan format
  /// @param attachment_ref High-level attachment reference
  /// @return Vulkan attachment reference structure
  vk::AttachmentReference convert(const AttachmentReference& attachment_ref);
};

/// @brief Subpass dependency graph for render pass creation
/// Manages a collection of subpasses and their dependencies, defining
/// the execution order and synchronization requirements between subpasses.
class SubpassGraph {
 private:
  std::vector<vk::SubpassDescription>
      m_descriptions;                ///< Vulkan subpass descriptions
  std::vector<SubpassNode> m_nodes;  ///< High-level subpass nodes
  std::vector<vk::SubpassDependency>
      m_dependencies;  ///< Inter-subpass dependencies

 public:
  SubpassGraph();
  ~SubpassGraph();

  /// @brief Get subpass descriptions for render pass creation
  /// @return Vector of Vulkan subpass descriptions
  const auto& getDescriptions() const {
    return m_descriptions;
  }

  /// @brief Get subpass dependencies for synchronization
  /// @return Vector of Vulkan subpass dependencies
  const auto& getDependencies() const {
    return m_dependencies;
  }

  /// @brief Add subpass node to the graph
  /// @param node Subpass node containing attachment references
  /// @return Index of the added subpass
  uint32_t appendNode(const SubpassNode& node);

  /// @brief Add dependency edge between subpasses
  /// @param edge Dependency specification between subpasses
  void appendEdge(const SubpassEdge& edge);
};

/// @brief Vulkan render pass wrapper
/// Encapsulates a Vulkan render pass object that defines the structure
/// of rendering operations, including attachments, subpasses, and their
/// dependencies.
class Renderpass {
 private:
  vk::UniqueRenderPass m_ptrRenderPass;  ///< Unique Vulkan render pass handle

 public:
  /// @brief Construct render pass from attachments and subpass graph
  /// @param ptr_context GPU context for device access
  /// @param attachment_list Collection of attachments used in the render pass
  /// @param subpass_graph Subpass definitions and dependencies
  Renderpass(const std::unique_ptr<gpu::Context>& ptr_context,
             const AttachmentList& attachment_list,
             const SubpassGraph& subpass_graph);

  // Rule of Five
  ~Renderpass();
  Renderpass(const Renderpass&) = delete;
  Renderpass& operator=(const Renderpass&) = delete;
  Renderpass(Renderpass&&) = default;
  Renderpass& operator=(Renderpass&&) = default;

  /// @brief Get underlying Vulkan render pass handle
  /// @return Vulkan render pass handle
  const auto& getRenderPass() const {
    return m_ptrRenderPass.get();
  }
};

/// @brief Vulkan framebuffer wrapper
/// Represents a collection of image views that serve as render targets
/// for a specific render pass configuration.
class Framebuffer {
 private:
  vk::UniqueFramebuffer m_ptrFramebuffer;  ///< Unique Vulkan framebuffer handle

 public:
  /// @brief Construct framebuffer for a render pass
  /// @param ptr_context GPU context for device access
  /// @param render_pass Compatible render pass
  /// @param size Framebuffer dimensions
  /// @param attachments Attachment list containing image views
  Framebuffer(const std::unique_ptr<gpu::Context>& ptr_context,
              const Renderpass& render_pass,
              const gpu_ui::GraphicalSize<uint32_t>& size,
              const AttachmentList& attachments);

  // Rule of Five
  ~Framebuffer();
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;
  Framebuffer(Framebuffer&&) = default;
  Framebuffer& operator=(Framebuffer&&) = default;

  /// @brief Get underlying Vulkan framebuffer handle
  /// @return Vulkan framebuffer handle
  const auto& getFrameBuffer() const {
    return m_ptrFramebuffer.get();
  }
};

/// @brief Complete rendering kit combining render pass and framebuffers
/// Provides a high-level interface for managing render passes, multiple
/// framebuffers (for double/triple buffering), and their associated clear
/// values. Handles swapchain integration for presentation and automatic
/// framebuffer management.
class RenderKit {
 private:
  std::unique_ptr<Renderpass> m_ptrRenderpass;  ///< Managed render pass
  std::vector<Framebuffer>
      m_framebuffers;  ///< Framebuffers for each swapchain image
  std::vector<vk::ClearValue> m_clearValues;  ///< Clear values for attachments
  size_t m_currentIndex = 0u;  ///< Currently active framebuffer index

 public:
  /// @brief Construct render kit with render pass and framebuffers
  /// @param ptr_context GPU context for device access
  /// @param attachment_list Attachments for the render pass
  /// @param subpass_graph Subpass configuration
  /// @param size Framebuffer dimensions
  /// @param is_presented Whether this render kit is used for presentation
  RenderKit(const std::unique_ptr<gpu::Context>& ptr_context,
            AttachmentList& attachment_list,
            const SubpassGraph& subpass_graph,
            const gpu_ui::GraphicalSize<uint32_t>& size,
            const bool is_presented);

  // Rule of Five
  ~RenderKit() = default;
  RenderKit(const RenderKit&) = delete;
  RenderKit& operator=(const RenderKit&) = delete;
  RenderKit(RenderKit&&) = default;
  RenderKit& operator=(RenderKit&&) = default;

  /// @brief Get the render pass
  /// @return Reference to the managed render pass
  const auto& getRenderpass() const {
    return *m_ptrRenderpass;
  }

  /// @brief Get the current framebuffer
  /// @return Reference to the currently active framebuffer
  const auto& getFramebuffer() const {
    return m_framebuffers.at(m_currentIndex);
  }

  /// @brief Get clear values for render pass begin
  /// @return Vector of clear values for all attachments
  const auto& getClearValues() const {
    return m_clearValues;
  }

  /// @brief Update current framebuffer index
  /// @param index New framebuffer index to use (should match swapchain image
  /// index)
  void updateIndex(const size_t index) {
    m_currentIndex = index;
  }

  /// @brief Reset and recreate framebuffers (e.g., after window resize)
  /// @param ptr_context GPU context for device access
  /// @param attachment_list Updated attachment list
  /// @param size New framebuffer dimensions
  /// @param is_presented Whether this render kit is used for presentation
  void resetFramebuffer(const std::unique_ptr<gpu::Context>& ptr_context,
                        AttachmentList& attachment_list,
                        const gpu_ui::GraphicalSize<uint32_t>& size,
                        const bool is_presented);
};

}  // namespace pandora::core
