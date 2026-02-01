#pragma once

#include "pandolabo.hpp"

namespace plc = pandora::core;
namespace plh = pandora::highlevel;

struct MyImageHL {
  unsigned char* data = nullptr;
  int width = 0;
  int height = 0;
  int channels = 0;
};

namespace samples::highlevel {

class SimpleImageComputingHL {
 private:
  std::unique_ptr<plc::gpu::Context> m_ptrContext;

  std::unique_ptr<plc::gpu::Image> m_ptrImage;
  std::unique_ptr<plc::gpu::Image> m_ptrStorageImage;
  std::unique_ptr<plc::gpu::Buffer> m_ptrUniformBuffer;

  std::unique_ptr<plc::gpu::ImageView> m_ptrImageView;
  std::unique_ptr<plc::gpu::ImageView> m_ptrStorageImageView;

  std::unique_ptr<plc::gpu::Sampler> m_ptrImageSampler;

  std::unique_ptr<plh::ComputeRunner> m_ptrComputeRunner;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;
  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::Pipeline> m_ptrComputePipeline;

  MyImageHL m_image;

  bool m_isInitialized = false;

 public:
  SimpleImageComputingHL();
  ~SimpleImageComputingHL();

  void run();

 private:
  void initializeImageResources();
  plc::VoidResult constructShaderResources();
  plc::VoidResult recordCompute(plc::ComputeCommandBuffer& command_buffer,
                                plc::gpu::Buffer& staging_buffer,
                                plc::gpu::Buffer& result_buffer);
};

}  // namespace samples::highlevel
