#pragma once

#include <chrono>
#include <glm/glm.hpp>
#include <random>

#include "pandora/core.hpp"

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

namespace samples {
namespace core {

class StreamingResources {
 private:
  struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    // Note: spawnTime removed to match pipeline expectations
    // Age tracking will be handled separately
  };

  struct TriangleInfo {
    float_t spawnTime;
    size_t vertexStartIndex;  // Index in m_activeTriangles where this triangle
                              // starts
  };

  std::unique_ptr<plc::ui::Window> m_ptrWindow;
  std::unique_ptr<plc::gpu::Context> m_ptrContext;
  std::unique_ptr<plc::RenderKit> m_ptrRenderKit;
  std::unordered_map<std::string, uint32_t> m_supassIndexMap;

  std::vector<std::unique_ptr<plc::CommandDriver>> m_ptrGraphicCommandDriver;
  std::vector<std::unique_ptr<plc::CommandDriver>> m_ptrTransferCommandDriver;

  // Frame-specific vertex buffers to avoid resource contention
  std::vector<std::unique_ptr<plc::gpu::Buffer>> m_ptrVertexBuffers;
  std::vector<plc::gpu::Buffer> m_stagingBuffers;
  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::Pipeline> m_ptrPipeline;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  // Timeline semaphore for streaming synchronization (reused for safety)
  std::unique_ptr<plc::gpu::TimelineSemaphore> m_currentTimelineSemaphore;
  uint64_t m_currentSemaphoreValue;

  // Triangle generation state
  std::vector<Vertex> m_activeTriangles;
  std::vector<TriangleInfo> m_triangleInfos;
  float_t m_triangleSpawnAngle;
  float_t m_lastSpawnTime;
  static constexpr size_t MAX_TRIANGLES = 200;
  static constexpr float_t SPAWN_INTERVAL = 0.05f;    // seconds
  static constexpr float_t TRIANGLE_LIFETIME = 5.0f;  // seconds

  // Animation state
  std::chrono::high_resolution_clock::time_point m_startTime;
  std::mt19937 m_randomGenerator;
  std::uniform_real_distribution<float_t> m_colorDist;

  bool m_isInitialized = false;

 public:
  StreamingResources();
  ~StreamingResources();

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

}  // namespace core
}  // namespace samples
