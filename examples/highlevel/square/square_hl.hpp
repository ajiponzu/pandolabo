#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "pandolabo.hpp"

namespace plc = pandora::core;
namespace plh = pandora::highlevel;

namespace samples::highlevel {

class SquareHL {
 private:
  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };

  std::unique_ptr<plc::ui::Window> m_ptrWindow;
  std::unique_ptr<plc::gpu::Context> m_ptrContext;
  std::unique_ptr<plc::RenderKit> m_ptrRenderKit;
  std::unordered_map<std::string, uint32_t> m_subpassIndexMap;

  std::unique_ptr<plh::Renderer> m_ptrRenderer;
  std::unique_ptr<plh::ResourceTransfer> m_ptrTransfer;

  std::unique_ptr<plc::gpu::Buffer> m_ptrVertexBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrIndexBuffer;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::Pipeline> m_ptrPipeline;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  bool m_isInitialized = false;

 public:
  SquareHL();
  ~SquareHL();

  void run();

 private:
  plc::VoidResult constructShaderResources();
  void constructRenderpass(bool is_resized = false);
  void constructGraphicPipeline();
  plc::VoidResult uploadGeometry();
  plc::VoidResult recordGraphic(plc::GraphicCommandBuffer& command_buffer);
};

}  // namespace samples::highlevel
