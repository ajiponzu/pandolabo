#pragma once

#include <glm/glm.hpp>

#include "pandora/core.hpp"

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

namespace samples {
namespace core {
class Square {
 private:
  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };

  std::unique_ptr<plc::ui::Window> m_ptrWindow;
  std::unique_ptr<plc::gpu::Context> m_ptrContext;
  std::unique_ptr<plc::RenderKit> m_ptrRenderKit;
  std::unordered_map<std::string, uint32_t> m_supassIndexMap;

  std::vector<std::unique_ptr<plc::CommandDriver>> m_ptrGraphicCommandDriver;
  std::unique_ptr<plc::CommandDriver> m_ptrTransferCommandDriver;

  std::unique_ptr<plc::gpu::Buffer> m_ptrVertexBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrIndexBuffer;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::Pipeline> m_ptrPipeline;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

 public:
  Square();
  ~Square();

  void run();

 private:
  void constructShaderResources();
  void constructRenderpass(const bool is_resized = false);
  void constructGraphicPipeline();
  void setTransferCommands(std::vector<plc::gpu::Buffer>& staging_buffers);
  void setGraphicCommands();
};
}  // namespace core
}  // namespace samples
