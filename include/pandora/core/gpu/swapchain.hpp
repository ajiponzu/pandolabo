/*
 * swapchain.hpp - GPU swapchain wrapper for Pandolabo Vulkan C++ wrapper
 *
 * This header contains the Swapchain class which manages Vulkan swapchain
 * operations. The Swapchain class handles image presentation and frame
 * synchronization.
 */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "../error.hpp"
#include "../module_connection/gpu_ui.hpp"
#include "../types.hpp"
#include "fence.hpp"
#include "semaphore.hpp"

// Forward declarations
namespace pandora::core::gpu {
class Device;
}  // namespace pandora::core::gpu

namespace pandora::core::gpu {

/// @brief GPU swapchain wrapper class
/// @details Swapchain is used to present images to the window surface.
/// This wrapper class also contains semaphores and fences for GPU-rendering
/// synchronization.
class Swapchain {
 private:
  vk::UniqueSwapchainKHR m_ptrSwapchain;
  std::vector<vk::Image> m_images;
  std::vector<vk::UniqueImageView> m_imageViews;

  uint32_t m_frameSyncIndex = 0u;
  uint32_t m_imageIndex = 0u;

  std::vector<vk::UniqueSemaphore> m_imageAvailableSemaphores;
  std::vector<vk::UniqueSemaphore> m_renderFinishedSemaphores;
  std::vector<vk::UniqueFence> m_fences;
  DataFormat m_imageFormat{};

 public:
  /// @brief Construct swapchain with device and window surface
  /// @param device GPU device wrapper reference
  /// @param surface GPU-window surface wrapper
  Swapchain(const Device& device,
            const std::shared_ptr<gpu_ui::WindowSurface>& surface);

  // Rule of Five
  ~Swapchain();
  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = default;
  Swapchain& operator=(Swapchain&&) = default;

  /// @brief Recreate swapchain
  /// @details Used when window is resized, moved, etc.
  /// @param device GPU device wrapper reference
  /// @param surface GPU-window surface wrapper
  void resetSwapchain(const Device& device,
                      const std::shared_ptr<gpu_ui::WindowSurface>& surface);

  /// @brief Get swapchain handle
  /// @return Vulkan swapchain handle
  const auto& getSwapchain() const {
    return m_ptrSwapchain.get();
  }

  /// @brief Get current frame synchronization index
  /// @return Frame sync index
  const auto getFrameSyncIndex() const {
    return m_frameSyncIndex;
  }

  /// @brief Get current image index
  /// @return Image index
  const auto getImageIndex() const {
    return m_imageIndex;
  }

  /// @brief Get all swapchain images
  /// @return Vector of swapchain images
  const auto& getImages() const {
    return m_images;
  }

  /// @brief Get all swapchain image views
  /// @return Vector of swapchain image views
  const auto& getImageViews() const {
    return m_imageViews;
  }

  /// @brief Get number of swapchain images
  /// @return Image count
  const auto getImageCount() const {
    return m_images.size();
  }

  /// @brief Get current swapchain image
  /// @return Current swapchain image
  const auto& getImage() const {
    return m_images.at(m_imageIndex);
  }

  /// @brief Get current frame fence
  /// @return Current frame fence
  auto getFence() const {
    return Fence(m_fences.at(m_frameSyncIndex).get());
  }

  /// @brief Get current image available semaphore (for acquire)
  /// @return Image available semaphore
  auto getImageAvailableSemaphore() const {
    return BinarySemaphore(
        m_imageAvailableSemaphores.at(m_frameSyncIndex).get());
  }

  /// @brief Get current render finished semaphore (for presentation)
  /// @return Render finished semaphore
  auto getFinishedSemaphore() const {
    return BinarySemaphore(m_renderFinishedSemaphores.at(m_imageIndex).get());
  }

  /// @brief Get swapchain image format
  /// @return Image format
  auto getImageFormat() const {
    return m_imageFormat;
  }

  /// @brief Acquire image index for updating
  /// @details You must call this function before rendering in each frame.
  /// @param device GPU device wrapper reference
  /// @param timeout Timeout in nanoseconds for image acquisition
  pandora::core::VoidResult updateImageIndex(
      const Device& device,
      uint64_t timeout = std::numeric_limits<uint64_t>::max());

  /// @brief Update frame sync image index
  /// @details You must call this function after presenting rendered image in
  /// each frame.
  void updateFrameSyncIndex();

 private:
  /// @brief Internal swapchain construction
  /// @param device GPU device wrapper reference
  /// @param surface GPU-window surface wrapper
  void constructSwapchain(
      const Device& device,
      const std::shared_ptr<gpu_ui::WindowSurface>& surface);

  /// @brief Clear swapchain resources
  void clear();
};

}  // namespace pandora::core::gpu
