#include "pandora/core/module_connection/gpu_ui.hpp"

namespace pandora::core::gpu_ui {

WindowSurface::WindowSurface(GLFWwindow& window) : m_window(window) {
  setWindowSize();
}

void WindowSurface::constructSurface(const vk::UniqueInstance& instance) {
  VkSurfaceKHR surface{};
  if (glfwCreateWindowSurface(
          VkInstance(instance.get()), &m_window.get(), nullptr, &surface)
      != ::VkResult::VK_SUCCESS) {
    m_ptrSurface = vk::UniqueSurfaceKHR(nullptr);

    return;
  }

  m_ptrSurface = vk::UniqueSurfaceKHR(surface, {instance.get()});
}

void WindowSurface::destroySurface() {
  m_ptrSurface.reset();
}

void WindowSurface::setWindowSize() {
  int window_width = 0, window_height = 0;
  glfwGetWindowSize(&m_window.get(), &window_width, &window_height);

  m_windowSize.width = window_width;
  m_windowSize.height = window_height;
  m_windowSize.depth = 0u;
}

}  // namespace pandora::core::gpu_ui
