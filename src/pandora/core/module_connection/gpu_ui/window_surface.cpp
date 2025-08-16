#include "pandora/core/module_connection/gpu_ui.hpp"

pandora::core::gpu_ui::WindowSurface::WindowSurface(GLFWwindow* const ptr_window) {
  m_ptrWindow = ptr_window;

  setWindowSize();
}

pandora::core::gpu_ui::WindowSurface::~WindowSurface() {
  m_ptrSurface.release();
  glfwDestroyWindow(m_ptrWindow);
}

void pandora::core::gpu_ui::WindowSurface::constructSurface(const vk::UniqueInstance& ptr_instance) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(VkInstance(ptr_instance.get()), m_ptrWindow, nullptr, &surface)
      != ::VkResult::VK_SUCCESS) {
    m_ptrSurface = vk::UniqueSurfaceKHR(nullptr);

    return;
  }

  m_ptrSurface = vk::UniqueSurfaceKHR(surface, {ptr_instance.get()});
}

void pandora::core::gpu_ui::WindowSurface::setWindowSize() {
  int window_width, window_height;
  glfwGetWindowSize(m_ptrWindow, &window_width, &window_height);

  m_windowSize.width = window_width;
  m_windowSize.height = window_height;
  m_windowSize.depth = 0U;
}
