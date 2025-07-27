#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::gpu::Sampler::Sampler(const std::unique_ptr<Context>& ptr_context, const SamplerInfo& sampler_info) {
  using namespace vk_helper;  // ローカルスコープでusing宣言

  vk::SamplerCreateInfo sampler_create_info;
  sampler_create_info.setMagFilter(getSamplerFilter(sampler_info.mag_filter));
  sampler_create_info.setMinFilter(getSamplerFilter(sampler_info.min_filter));
  sampler_create_info.setMipmapMode(getSamplerMipmapMode(sampler_info.mipmap_mode));
  sampler_create_info.setAddressModeU(getSamplerAddressMode(sampler_info.address_mode_u));
  sampler_create_info.setAddressModeV(getSamplerAddressMode(sampler_info.address_mode_v));
  sampler_create_info.setAddressModeW(getSamplerAddressMode(sampler_info.address_mode_w));
  sampler_create_info.setCompareOp(getSamplerCompareOp(sampler_info.compare_op));
  sampler_create_info.setBorderColor(getSamplerBorderColor(sampler_info.border_color));

  sampler_create_info.setAnisotropyEnable(sampler_info.anisotropy_enable);
  sampler_create_info.setMaxAnisotropy(sampler_info.max_anisotropy);
  sampler_create_info.setCompareEnable(sampler_info.compare_enable);
  sampler_create_info.setMipLodBias(sampler_info.mip_lod_bias);
  sampler_create_info.setMinLod(sampler_info.min_lod);
  sampler_create_info.setMaxLod(sampler_info.max_lod);
  sampler_create_info.setUnnormalizedCoordinates(sampler_info.unnormalized_coordinates);

  m_ptrSampler = ptr_context->getPtrDevice()->getPtrLogicalDevice()->createSamplerUnique(sampler_create_info);
}

pandora::core::gpu::Sampler::~Sampler() {}
