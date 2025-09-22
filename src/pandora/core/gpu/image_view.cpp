#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

namespace {

vk::ImageViewType get_image_view_type(
    const pandora::core::ImageDimension image_dimension) {
  switch (image_dimension) {
    using enum pandora::core::ImageDimension;
    using enum vk::ImageViewType;

    case v1D:
      return e1D;
    case v2D:
      return e2D;
    case v3D:
      return e3D;
    default:
      return vk::ImageViewType(0u);
  }
}

}  // namespace

namespace pandora::core::gpu {

ImageView::ImageView(const std::unique_ptr<Context>& ptr_context,
                     const Image& image,
                     const ImageViewInfo& image_view_info) {
  const auto create_info =
      vk::ImageViewCreateInfo()
          .setSubresourceRange(
              vk::ImageSubresourceRange{}
                  .setAspectMask(
                      vk_helper::getImageAspectFlags(image_view_info.aspect))
                  .setBaseMipLevel(image_view_info.base_mip_level)
                  .setLevelCount(image_view_info.mip_levels)
                  .setBaseArrayLayer(image_view_info.base_array_layer)
                  .setLayerCount(image_view_info.array_layers))
          .setComponents(vk::ComponentMapping{}
                             .setR(vk::ComponentSwizzle::eIdentity)
                             .setG(vk::ComponentSwizzle::eIdentity)
                             .setB(vk::ComponentSwizzle::eIdentity)
                             .setA(vk::ComponentSwizzle::eIdentity))
          .setViewType(get_image_view_type(image.getDimension()))
          .setFormat(image.getFormat())
          .setImage(image.getImage());

  m_ptrImageView =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createImageViewUnique(
          create_info);
  m_ptrImageViewInfo = std::make_unique<ImageViewInfo>(image_view_info);
}

ImageView::~ImageView() {}

}  // namespace pandora::core::gpu
