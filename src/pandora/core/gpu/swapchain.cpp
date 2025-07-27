#include "pandora/core/gpu.hpp"
#include "pandora/core/gpu/vk_helper.hpp"

pandora::core::gpu::Swapchain::Swapchain(const std::unique_ptr<Device>& ptr_device,
                                         const std::shared_ptr<gpu_ui::WindowSurface>& ptr_surface) {
  constructSwapchain(ptr_device, ptr_surface);
}

pandora::core::gpu::Swapchain::~Swapchain() {
  clear();
}

void pandora::core::gpu::Swapchain::resetSwapchain(const std::unique_ptr<Device>& ptr_device,
                                                   const std::shared_ptr<gpu_ui::WindowSurface>& ptr_surface) {
  clear();
  constructSwapchain(ptr_device, ptr_surface);
}

void pandora::core::gpu::Swapchain::updateImageIndex(const std::unique_ptr<Device>& ptr_device) {
  const auto& ptr_vk_device = ptr_device->getPtrLogicalDevice();

  const auto vk_result =
      ptr_vk_device->waitForFences(m_fences.at(m_frameSyncIndex).get(), VK_TRUE, std::numeric_limits<uint64_t>::max());
  if (vk_result != vk::Result::eSuccess) {
    throw std::runtime_error("Failed to wait for fence.");
  }

  const auto next_image_opt = ptr_vk_device->acquireNextImageKHR(m_ptrSwapchain.get(),
                                                                 std::numeric_limits<uint64_t>::max(),
                                                                 m_imageAvailableSemaphores.at(m_frameSyncIndex).get(),
                                                                 nullptr);

  if (next_image_opt.result != vk::Result::eSuccess && next_image_opt.result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("Failed to acquire next image index.");
  }
  m_imageIndex = next_image_opt.value;

  ptr_vk_device->resetFences(m_fences.at(m_frameSyncIndex).get());
}

void pandora::core::gpu::Swapchain::updateFrameSyncIndex() {
  m_frameSyncIndex = (m_frameSyncIndex + 1U) % static_cast<uint32_t>(m_fences.size());
}

void pandora::core::gpu::Swapchain::constructSwapchain(const std::unique_ptr<Device>& ptr_device,
                                                       const std::shared_ptr<gpu_ui::WindowSurface>& ptr_surface) {
  m_imageFormat = DataFormat::R8G8B8A8Srgb;

  {
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3U;

    const auto& vk_surface = ptr_surface->getSurface();

    const auto surface_capabilities = ptr_device->getPhysicalDevice().getSurfaceCapabilitiesKHR(vk_surface.get());

    auto swapchain_info = vk::SwapchainCreateInfoKHR();
    swapchain_info.setSurface(vk_surface.get());
    swapchain_info.setMinImageCount(std::min(surface_capabilities.maxImageCount, MAX_FRAMES_IN_FLIGHT));
    swapchain_info.setImageSharingMode(vk::SharingMode::eExclusive);
    swapchain_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    swapchain_info.setPresentMode(vk::PresentModeKHR::eFifo);
    swapchain_info.setClipped(true);
    swapchain_info.setImageFormat(vk_helper::getFormat(m_imageFormat));
    swapchain_info.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear);
    swapchain_info.setImageExtent(vk_helper::getExtent2D(ptr_surface->getWindowSize()));
    swapchain_info.setImageArrayLayers(1U);
    swapchain_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    swapchain_info.setPreTransform(surface_capabilities.currentTransform);
    swapchain_info.setOldSwapchain(m_ptrSwapchain.get());

    const auto queue_family_index = ptr_device->getQueueFamilyIndex(pandora::core::QueueFamilyType::Graphics);
    swapchain_info.setQueueFamilyIndices(queue_family_index);

    m_ptrSwapchain = ptr_device->getPtrLogicalDevice()->createSwapchainKHRUnique(swapchain_info);
  }

  m_images = ptr_device->getPtrLogicalDevice()->getSwapchainImagesKHR(m_ptrSwapchain.get());

  {
    vk::ImageViewCreateInfo image_view_info;
    image_view_info.setViewType(vk::ImageViewType::e2D);
    image_view_info.setFormat(vk_helper::getFormat(m_imageFormat));
    image_view_info.setComponents({vk::ComponentSwizzle::eIdentity,
                                   vk::ComponentSwizzle::eIdentity,
                                   vk::ComponentSwizzle::eIdentity,
                                   vk::ComponentSwizzle::eIdentity});
    image_view_info.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0U, 1U, 0U, 1U});

    const auto& ptr_vk_device = ptr_device->getPtrLogicalDevice();
    for (const auto& image : m_images) {
      image_view_info.setImage(image);
      m_imageViews.emplace_back(ptr_vk_device->createImageViewUnique(image_view_info));

      m_imageAvailableSemaphores.emplace_back(ptr_vk_device->createSemaphoreUnique({}));
      m_renderFinishedSemaphores.emplace_back(ptr_vk_device->createSemaphoreUnique({}));
      m_fences.emplace_back(ptr_vk_device->createFenceUnique(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
    }
  }
}

void pandora::core::gpu::Swapchain::clear() {
  /* Notion: this function must not delete swapchain */

  m_imageViews.clear();
  m_images.clear();
  m_renderFinishedSemaphores.clear();
  m_imageAvailableSemaphores.clear();
  m_fences.clear();
}
