#include "pandora/core/module_connection/gpu_ui.hpp"

namespace pandora::core::gpu_ui {

WindowSurface::WindowSurface(GLFWwindow* const ptr_window) {
  m_ptrWindow = ptr_window;

  setWindowSize();
}

WindowSurface::~WindowSurface() {
  m_ptrSurface.release();
  glfwDestroyWindow(m_ptrWindow);
}

void WindowSurface::constructSurface(const vk::UniqueInstance& ptr_instance) {
  VkSurfaceKHR surface{};
  if (glfwCreateWindowSurface(
          VkInstance(ptr_instance.get()), m_ptrWindow, nullptr, &surface)
      != ::VkResult::VK_SUCCESS) {
    m_ptrSurface = vk::UniqueSurfaceKHR(nullptr);

    return;
  }

  m_ptrSurface = vk::UniqueSurfaceKHR(surface, {ptr_instance.get()});
}

void WindowSurface::setWindowSize() {
  int window_width = 0, window_height = 0;
  glfwGetWindowSize(m_ptrWindow, &window_width, &window_height);

  m_windowSize.width = window_width;
  m_windowSize.height = window_height;
  m_windowSize.depth = 0u;
}

}  // namespace pandora::core::gpu_ui
