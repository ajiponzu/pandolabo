/*
This header file contains a module connecting connect gpu.hpp and ui.hpp.
*/

#pragma once

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <concepts>
#include <functional>
#include <type_traits>

namespace pandora::core::gpu_ui {

template <typename T>
concept NumberConcept = std::is_arithmetic_v<T>;

template <NumberConcept T>
struct GraphicalSize {
  T width;
  T height;
  T depth;
};

/// @brief Vulkan surface wrapper for GPU-UI connection
/// This class connects Vulkan rendering with GLFW window management,
/// providing the surface interface needed for presentation.
class WindowSurface {
 private:
  std::reference_wrapper<GLFWwindow>
      m_window;                            ///< Non-owning GLFW window handle
  vk::UniqueSurfaceKHR m_ptrSurface;       ///< Vulkan surface for presentation
  GraphicalSize<uint32_t> m_windowSize{};  ///< Window dimensions

 public:
  explicit WindowSurface(GLFWwindow& window);
  ~WindowSurface() = default;

  auto getWindow() const {
    return &m_window.get();
  };
  const auto& getSurface() const {
    return m_ptrSurface;
  };
  const auto& getWindowSize() const {
    return m_windowSize;
  }

  void constructSurface(const vk::UniqueInstance& instance);
  void destroySurface();

  void setWindowSize();
};

}  // namespace pandora::core::gpu_ui
