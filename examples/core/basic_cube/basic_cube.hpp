#pragma once
#include <glm/glm.hpp>
#include <pandora/core.hpp>

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

namespace samples {
namespace core {
class BasicCube {
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
  std::unordered_map<std::string, uint32_t> m_supassIndexMap;

  std::vector<std::unique_ptr<plc::CommandDriver>> m_ptrGraphicCommandDriver;
  std::unique_ptr<plc::CommandDriver> m_ptrTransferCommandDriver;

  std::unique_ptr<plc::gpu::Buffer> m_ptrVertexBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrIndexBuffer;

  std::unique_ptr<plc::gpu::Buffer> m_ptrUniformBuffer;
  std::unique_ptr<CubePosition> m_ptrCubePosition;
  void* m_ptrCubePositionMapping;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::Pipeline> m_ptrPipeline;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  std::unique_ptr<plc::gpu::Image> m_ptrDepthImage;
  std::unique_ptr<plc::gpu::ImageView> m_ptrDepthImageView;

 public:
  BasicCube();
  ~BasicCube();

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
