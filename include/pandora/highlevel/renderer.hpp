#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "pandora/core/command_buffer.hpp"
#include "pandora/core/error.hpp"
#include "pandora/core/renderpass.hpp"
#include "pandora/core/ui.hpp"
#include "pandora/highlevel/frame_context.hpp"

namespace pandora::highlevel {

/// @brief High-level renderer wrapper for frame orchestration.
class Renderer {
 public:
  using RecordFn = std::function<pandora::core::VoidResult(
      pandora::core::GraphicCommandBuffer&)>;

 private:
  std::reference_wrapper<const pandora::core::ui::Window> m_windowOwner;
  std::reference_wrapper<const pandora::core::gpu::Context> m_contextOwner;
  std::vector<std::unique_ptr<pandora::core::CommandDriver>> m_graphicDrivers;
  std::optional<std::reference_wrapper<pandora::core::RenderKit>> m_renderKit;

 public:
  Renderer(const pandora::core::ui::Window& window,
           const pandora::core::gpu::Context& context);

  /// @brief Set render kit used for render pass begin/end convenience.
  void setRenderKit(pandora::core::RenderKit& render_kit) {
    m_renderKit = render_kit;
  }

  /// @brief Acquire image and build per-frame context.
  [[nodiscard]] pandora::core::Result<FrameContext> beginFrame();

  /// @brief Record commands on the frame's command buffer.
  [[nodiscard]] pandora::core::VoidResult record(FrameContext& frame,
                                                 const RecordFn& record_fn);

  /// @brief Submit recorded commands and present.
  [[nodiscard]] pandora::core::VoidResult endFrame(FrameContext& frame);
};

}  // namespace pandora::highlevel
