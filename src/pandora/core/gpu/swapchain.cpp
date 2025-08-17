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
  m_frameSyncIndex = (m_frameSyncIndex + 1u) % static_cast<uint32_t>(m_fences.size());
}

void pandora::core::gpu::Swapchain::constructSwapchain(const std::unique_ptr<Device>& ptr_device,
                                                       const std::shared_ptr<gpu_ui::WindowSurface>& ptr_surface) {
  m_imageFormat = DataFormat::R8G8B8A8Srgb;

  {
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3u;

    const auto& vk_surface = ptr_surface->getSurface();

    const auto surface_capabilities = ptr_device->getPhysicalDevice().getSurfaceCapabilitiesKHR(vk_surface.get());
    const auto queue_family_index = ptr_device->getQueueFamilyIndex(pandora::core::QueueFamilyType::Graphics);

    const auto swapchain_info =
        vk::SwapchainCreateInfoKHR{}
            .setSurface(vk_surface.get())
            .setMinImageCount(std::min(surface_capabilities.maxImageCount, MAX_FRAMES_IN_FLIGHT))
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setClipped(true)
            .setImageFormat(vk_helper::getFormat(m_imageFormat))
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setImageExtent(vk_helper::getExtent2D(ptr_surface->getWindowSize()))
            .setImageArrayLayers(1u)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setPreTransform(surface_capabilities.currentTransform)
            .setOldSwapchain(m_ptrSwapchain.get())
            .setQueueFamilyIndices(queue_family_index);

    m_ptrSwapchain = ptr_device->getPtrLogicalDevice()->createSwapchainKHRUnique(swapchain_info);
  }

  m_images = ptr_device->getPtrLogicalDevice()->getSwapchainImagesKHR(m_ptrSwapchain.get());

  {
    auto image_view_info = vk::ImageViewCreateInfo()
                               .setViewType(vk::ImageViewType::e2D)
                               .setFormat(vk_helper::getFormat(m_imageFormat))
                               .setComponents({vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity})
                               .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0u, 1u, 0u, 1u});

    const auto& ptr_vk_device = ptr_device->getPtrLogicalDevice();
    for (const auto& image : m_images) {
      image_view_info.setImage(image);
      m_imageViews.emplace_back(ptr_vk_device->createImageViewUnique(image_view_info));

      m_imageAvailableSemaphores.emplace_back(ptr_vk_device->createSemaphoreUnique({}));
      m_renderFinishedSemaphores.emplace_back(ptr_vk_device->createSemaphoreUnique({}));
      m_fences.emplace_back(ptr_vk_device->createFenceUnique(vk::FenceCreateInfo{vk::FenceCreateFlagBits::eSignaled}));
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

  m_frameSyncIndex = 0u;
  m_imageIndex = 0u;
}
