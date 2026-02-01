#include "pandora/highlevel/shader_library.hpp"

#include <string>

#include "pandora/core/gpu.hpp"

namespace pandora::highlevel {

pandora::core::Result<pandora::core::gpu::ShaderModule> ShaderLibrary::load(
    std::string_view path) const {
  if (!m_contextOwner.get().isInitialized()) {
    return pandora::core::Error::runtime("Context not initialized")
        .withContext("ShaderLibrary::load");
  }
  const auto spirv_result = pandora::core::io::shader::read(std::string(path));
  if (!spirv_result.isOk()) {
    return spirv_result.error().withContext("ShaderLibrary::load");
  }

  return pandora::core::gpu::ShaderModule(m_contextOwner.get(),
                                          spirv_result.value());
}

}  // namespace pandora::highlevel
