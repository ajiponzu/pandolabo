#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "pandolabo.hpp"

namespace plc = pandora::core;
namespace plh = pandora::highlevel;

namespace samples::highlevel {

class BasicCubeHL {
 private:
  struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
  };

  struct CubePosition {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  std::unique_ptr<plc::ui::Window> m_ptrWindow;
  std::unique_ptr<plc::gpu::Context> m_ptrContext;
  std::unique_ptr<plc::RenderKit> m_ptrRenderKit;
  std::unordered_map<std::string, uint32_t> m_subpassIndexMap;

  std::unique_ptr<plh::Renderer> m_ptrRenderer;
  std::unique_ptr<plh::ResourceTransfer> m_ptrTransfer;

  std::unique_ptr<plc::gpu::Buffer> m_ptrVertexBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrIndexBuffer;

  std::unique_ptr<plc::gpu::Buffer> m_ptrUniformBuffer;
  std::unique_ptr<CubePosition> m_ptrCubePosition;
  void* m_ptrCubePositionMapping = nullptr;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::Pipeline> m_ptrPipeline;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  std::unique_ptr<plc::gpu::Image> m_ptrDepthImage;
  std::unique_ptr<plc::gpu::ImageView> m_ptrDepthImageView;

  bool m_isInitialized = false;

 public:
  BasicCubeHL();
  ~BasicCubeHL();

  void run();

 private:
  plc::VoidResult constructShaderResources();
  void constructRenderpass(bool is_resized = false);
  void constructGraphicPipeline();
  plc::VoidResult uploadGeometry();
  plc::VoidResult recordGraphic(plc::GraphicCommandBuffer& cmd);
  void updateUniforms();
};

}  // namespace samples::highlevel
