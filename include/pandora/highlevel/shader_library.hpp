#pragma once

#include <functional>
#include <memory>
#include <string_view>

#include "pandora/core/gpu/shader.hpp"
#include "pandora/core/io.hpp"

namespace pandora::highlevel {

/// @brief Thin wrapper around shader I/O and module creation.
class ShaderLibrary {
 private:
  std::reference_wrapper<const pandora::core::gpu::Context> m_contextOwner;

 public:
  /// @brief Construct with a context owner.
  explicit ShaderLibrary(const pandora::core::gpu::Context& ptr_context)
      : m_contextOwner(ptr_context) {}

  /// @brief Load shader and create a module.
  [[nodiscard]] pandora::core::Result<pandora::core::gpu::ShaderModule> load(
      std::string_view path) const;
};

}  // namespace pandora::highlevel
