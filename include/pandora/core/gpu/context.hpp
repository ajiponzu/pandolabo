/*
 * context.hpp - GPU context wrapper for Pandolabo Vulkan C++ wrapper
 *
 * This header contains the Context class which is the main GPU handler.
 * The Context class manages Vulkan instance, device, window surface, and
 * swapchain.
 */

#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

#include "../module_connection/gpu_ui.hpp"
#include "config.hpp"
#include "device.hpp"
#include "swapchain.hpp"

#ifdef GPU_DEBUG
  #include "debug.hpp"
#endif

namespace pandora::core::gpu {

/// @brief Main GPU handler class
/// @details This class contains Vulkan instance, GPU device, window surface,
/// and swapchain. This is the core of the Pandolabo project.
class Context {
 private:
#ifdef GPU_DEBUG
  std::unique_ptr<debug::Messenger> m_ptrMessenger;
#endif

  vk::UniqueInstance m_ptrInstance;
  std::shared_ptr<gpu_ui::WindowSurface> m_ptrWindowSurface;
  std::unique_ptr<Device> m_ptrDevice;
  std::unique_ptr<Swapchain> m_ptrSwapchain;

  bool m_isInitialized = false;

 public:
  /// @brief Construct Context with optional window surface
  /// @param ptr_window_surface Window surface for presentation (optional)
  Context(std::shared_ptr<gpu_ui::WindowSurface> ptr_window_surface = nullptr);

  // Rule of Five
  ~Context();
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
  Context(Context&&) = default;
  Context& operator=(Context&&) = default;

  /// @brief Get Vulkan instance
  /// @return Reference to Vulkan instance
  const auto& getInstance() const {
    return m_ptrInstance;
  }

  /// @brief Get window surface pointer
  /// @return Shared pointer to window surface
  const auto& getPtrWindowSurface() const {
    return m_ptrWindowSurface;
  }

  /// @brief Get device pointer
  /// @return Unique pointer to device
  const auto& getPtrDevice() const {
    return m_ptrDevice;
  }

  /// @brief Get swapchain pointer
  /// @return Unique pointer to swapchain
  const auto& getPtrSwapchain() const {
    return m_ptrSwapchain;
  }

  /// @brief Acquire the next image from the swapchain
  void acquireNextImage();

  /// @brief Reset the swapchain (e.g., after window resize)
  void resetSwapchain();

  /// @brief Check if the context is initialized
  /// @return True if initialization is complete
  bool isInitialized() const {
    return m_isInitialized;
  }
};

}  // namespace pandora::core::gpu
