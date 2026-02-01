#include "pandora/highlevel/shader_library.hpp"

#include <string>

namespace pandora::highlevel {

pandora::core::Result<pandora::core::gpu::ShaderModule> ShaderLibrary::load(
    std::string_view path) const {
  const auto spirv_result = pandora::core::io::shader::read(std::string(path));
  if (!spirv_result.isOk()) {
    return spirv_result.error().withContext("ShaderLibrary::load");
  }

  return pandora::core::gpu::ShaderModule(m_contextOwner.get(),
                                          spirv_result.value());
}

}  // namespace pandora::highlevel
