#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "pandora/core/command_buffer.hpp"

namespace pandora::highlevel {

/// @brief Per-frame context carrying indices and driver reference.
struct FrameContext {
  uint32_t imageIndex = 0u;
  uint32_t frameIndex = 0u;
  std::reference_wrapper<pandora::core::CommandDriver> driver;
  std::vector<pandora::core::SubmitSemaphore> extraWaitSemaphores{};
  std::vector<pandora::core::SubmitSemaphore> extraSignalSemaphores{};

  void begin() {}
  void end() {}
};

}  // namespace pandora::highlevel
