#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::gpu::Sampler::Sampler(
    const std::unique_ptr<Context>& ptr_context,
    const SamplerInfo& sampler_info) {
  using namespace vk_helper;

  const auto sampler_create_info =
      vk::SamplerCreateInfo{}
          .setMagFilter(getSamplerFilter(sampler_info.mag_filter))
          .setMinFilter(getSamplerFilter(sampler_info.min_filter))
          .setMipmapMode(getSamplerMipmapMode(sampler_info.mipmap_mode))
          .setAddressModeU(getSamplerAddressMode(sampler_info.address_mode_u))
          .setAddressModeV(getSamplerAddressMode(sampler_info.address_mode_v))
          .setAddressModeW(getSamplerAddressMode(sampler_info.address_mode_w))
          .setCompareOp(getSamplerCompareOp(sampler_info.compare_op))
          .setBorderColor(getSamplerBorderColor(sampler_info.border_color))
          .setAnisotropyEnable(sampler_info.anisotropy_enable)
          .setMaxAnisotropy(sampler_info.max_anisotropy)
          .setCompareEnable(sampler_info.compare_enable)
          .setMipLodBias(sampler_info.mip_lod_bias)
          .setMinLod(sampler_info.min_lod)
          .setMaxLod(sampler_info.max_lod)
          .setUnnormalizedCoordinates(sampler_info.unnormalized_coordinates);

  m_ptrSampler =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createSamplerUnique(
          sampler_create_info);
}

pandora::core::gpu::Sampler::~Sampler() {}
