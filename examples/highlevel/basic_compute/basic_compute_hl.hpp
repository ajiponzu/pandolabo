#pragma once

#include <memory>
#include <vector>

#include "pandolabo.hpp"

namespace plc = pandora::core;
namespace plh = pandora::highlevel;

namespace samples::highlevel {

class BasicComputeHL {
 private:
  std::unique_ptr<plc::gpu::Context> m_ptrContext;

  std::unique_ptr<plh::ComputeRunner> m_ptrComputeRunner;
  std::unique_ptr<plh::ResourceTransfer> m_ptrTransfer;

  std::unique_ptr<plc::gpu::Buffer> m_ptrUniformBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrInputStorageBuffer;
  std::unique_ptr<plc::gpu::Buffer> m_ptrOutputStorageBuffer;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::Pipeline> m_ptrComputePipeline;

  bool m_isInitialized = false;

 public:
  BasicComputeHL();
  ~BasicComputeHL();

  void run();

 private:
  plc::VoidResult constructShaderResources();
  plc::VoidResult recordCompute(plc::ComputeCommandBuffer& command_buffer);
};

}  // namespace samples::highlevel
