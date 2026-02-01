#pragma once

#include <chrono>
#include <glm/glm.hpp>
#include <random>
#include <unordered_map>
#include <vector>

#include "pandolabo.hpp"

namespace plc = pandora::core;
namespace plh = pandora::highlevel;

namespace samples::highlevel {

class StreamingResourcesHL {
 private:
  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
  };

  struct TriangleInfo {
    float_t spawnTime;
    size_t vertexStartIndex;
  };

  std::unique_ptr<plc::ui::Window> m_ptrWindow;
  std::unique_ptr<plc::gpu::Context> m_ptrContext;
  std::unique_ptr<plc::RenderKit> m_ptrRenderKit;
  std::unordered_map<std::string, uint32_t> m_subpassIndexMap;

  std::vector<std::unique_ptr<plc::CommandDriver>> m_ptrGraphicCommandDriver;
  std::vector<std::unique_ptr<plc::CommandDriver>> m_ptrTransferCommandDriver;

  std::vector<std::unique_ptr<plc::gpu::Buffer>> m_ptrVertexBuffers;
  std::vector<plc::gpu::Buffer> m_stagingBuffers;
  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::Pipeline> m_ptrPipeline;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  std::unique_ptr<plc::gpu::TimelineSemaphore> m_currentTimelineSemaphore;
  uint64_t m_currentSemaphoreValue;

  std::vector<Vertex> m_activeTriangles;
  std::vector<TriangleInfo> m_triangleInfos;
  float_t m_triangleSpawnAngle;
  float_t m_lastSpawnTime;
  static constexpr size_t MAX_TRIANGLES = 200;
  static constexpr float_t SPAWN_INTERVAL = 0.05f;
  static constexpr float_t TRIANGLE_LIFETIME = 5.0f;

  std::chrono::high_resolution_clock::time_point m_startTime;
  std::mt19937 m_randomGenerator;
  std::uniform_real_distribution<float_t> m_colorDist;

  bool m_isInitialized = false;

 public:
  StreamingResourcesHL();
  ~StreamingResourcesHL();

  void run();

 private:
  plc::VoidResult constructShaderResources();
  void constructRenderpass(bool is_resized = false);
  void constructGraphicPipeline();
  plc::VoidResult updateVertexData();
  plc::VoidResult setGraphicCommands();
  void spawnNewTriangle(float_t currentTime);
  void removeOldTriangles(float_t currentTime);
  std::vector<Vertex> getCurrentTriangles(float_t currentTime);
};

}  // namespace samples::highlevel
