#pragma once

#include <memory>
#include <string>
#include <vector>

#include "pandora/core.hpp"

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

namespace samples {
namespace core {

class BasicComputing {
 private:
  std::unique_ptr<plc::gpu::Context> m_ptrContext;

  std::unique_ptr<plc::CommandDriver> m_ptrComputeCommandDriver;
  std::unique_ptr<plc::CommandDriver> m_ptrTransferCommandDriver;

  std::unique_ptr<plc::gpu::Buffer> m_ptrUniformBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrInputStorageBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrOutputStorageBuffer;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::Pipeline> m_ptrComputePipeline;

 public:
  BasicComputing();

  ~BasicComputing();

  void run();

 private:
  plc::VoidResult setTransferCommands(
      std::vector<plc::gpu::Buffer>& staging_buffers);

  plc::VoidResult constructShaderResources();
  plc::VoidResult setComputeCommands(plc::gpu::Buffer& staging_buffer);
};

}  // namespace core
}  // namespace samples
