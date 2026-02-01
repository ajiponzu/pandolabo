#pragma once

#include <functional>
#include <memory>

#include "pandora/core/command_buffer.hpp"
#include "pandora/core/error.hpp"
#include "pandora/core/gpu.hpp"

namespace pandora::highlevel {

/// @brief High-level wrapper for compute command recording and submission.
class ComputeRunner {
 public:
  using RecordFn = std::function<pandora::core::VoidResult(
      pandora::core::ComputeCommandBuffer&)>;

 private:
  std::reference_wrapper<const pandora::core::gpu::Context> m_contextOwner;
  std::unique_ptr<pandora::core::CommandDriver> m_computeDriver;

  pandora::core::CommandDriver& ensureDriver();

 public:
  explicit ComputeRunner(const pandora::core::gpu::Context& context)
      : m_contextOwner(context) {}

  /// @brief Begin recording compute commands.
  [[nodiscard]] pandora::core::Result<pandora::core::ComputeCommandBuffer>
  begin();

  /// @brief Record commands and end the buffer.
  [[nodiscard]] pandora::core::VoidResult record(
      pandora::core::ComputeCommandBuffer& command_buffer,
      const RecordFn& record_fn);

  /// @brief Submit recorded commands.
  [[nodiscard]] pandora::core::VoidResult submit(
      const pandora::core::SubmitSemaphoreGroup& semaphore_group = {});

  /// @brief Wait for compute queue to become idle.
  void queueWaitIdle();
};

}  // namespace pandora::highlevel
