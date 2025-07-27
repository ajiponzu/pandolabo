#pragma once

#include "pandora/core.hpp"

// Namespace alias for cleaner code in examples
namespace plc = pandora::core;

struct MyImage {
  unsigned char* data = nullptr;
  int width = 0;
  int height = 0;
  int channels = 0;
};

namespace samples {
namespace core {
class SimpleImageComputing {
 private:
  std::unique_ptr<plc::gpu::Context> m_ptrContext;

  std::unique_ptr<plc::gpu::Image> m_ptrImage;
  std::unique_ptr<plc::gpu::Image> m_ptrStorageImage;
  std::unique_ptr<plc::gpu::Buffer> m_ptrUniformBuffer;

  std::unique_ptr<plc::gpu::ImageView> m_ptrImageView;
  std::unique_ptr<plc::gpu::ImageView> m_ptrStorageImageView;

  std::unique_ptr<plc::gpu::Sampler> m_ptrImageSampler;

  std::unique_ptr<plc::CommandDriver> m_ptrComputeCommandDriver;
  std::unique_ptr<plc::CommandDriver> m_ptrTransferCommandDriver;

  plc::ShaderModuleMap m_shaderModuleMap;

  std::unique_ptr<plc::gpu::DescriptorSetLayout> m_ptrDescriptorSetLayout;

  std::unique_ptr<plc::gpu::DescriptorSet> m_ptrDescriptorSet;
  std::unique_ptr<plc::Pipeline> m_ptrComputePipeline;

  MyImage m_image;

 public:
  SimpleImageComputing();
  ~SimpleImageComputing();

  void run();

 private:
  void initializeImageResources();
  void constructShaderResources();

  void setTransferCommands(std::vector<plc::gpu::Buffer>& staging_buffers);

  void setComputeCommands(const plc::gpu::Buffer& staging_buffer);
};

}  // namespace core
}  // namespace samples
