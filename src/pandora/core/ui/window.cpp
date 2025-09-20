#include <iostream>

#include "pandora/core/ui.hpp"

std::unordered_map<uint64_t, std::unordered_set<int32_t>>
    pandora::core::ui::Window::s_inputKeySetMap;
std::unordered_map<uint64_t, pandora::core::Mouse>
    pandora::core::ui::Window::s_mouseMap;
std::unordered_map<uint64_t, bool> pandora::core::ui::Window::s_resizedBoolMap;

namespace {

#ifdef _DEBUG
void error_callback(int error, const char* description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}
#endif

void callback_input_key(
    GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
  using namespace pandora::core::ui;

  if (action == GLFW_PRESS) {
    Window::insertInputKey(window, key);
  } else if (action == GLFW_RELEASE) {
    Window::eraseInputKey(window, key);
  }
}

void callback_cursor_pos(GLFWwindow* window, double x, double y) {
  using namespace pandora::core::ui;

  Window::setMousePos(window, x, y);
}

void callback_scroll(GLFWwindow* window, double x, double y) {
  using namespace pandora::core::ui;

  Window::setMouseScroll(window, x, y);
}

void callback_resized(GLFWwindow* window, int /*width*/, int /*height*/) {
  using namespace pandora::core::ui;

  Window::setResizedBool(window, true);
}

}  // namespace

pandora::core::ui::Window::Window(const std::string& title,
                                  const int32_t width,
                                  const int32_t height) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW.");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_ptrWindow =
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (m_ptrWindow == nullptr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window.");
  }

  glfwMakeContextCurrent(m_ptrWindow);

#ifdef _DEBUG
  glfwSetErrorCallback(error_callback);
#endif

  glfwSetKeyCallback(m_ptrWindow, callback_input_key);
  glfwSetCursorPosCallback(m_ptrWindow, callback_cursor_pos);
  glfwSetScrollCallback(m_ptrWindow, callback_scroll);
  glfwSetWindowSizeCallback(m_ptrWindow, callback_resized);

  m_ptrWindowSurface = std::make_shared<gpu_ui::WindowSurface>(m_ptrWindow);
}

pandora::core::ui::Window::~Window() {
  glfwDestroyWindow(m_ptrWindow);
  glfwTerminate();

#ifdef _DEBUG
  glfwSetErrorCallback(nullptr);
#endif
}

bool pandora::core::ui::Window::update() {
  setResizedBool(m_ptrWindow, false);

  glfwPollEvents();

  for (auto& callback : m_callbacks) {
    callback();
  }

  return !glfwWindowShouldClose(m_ptrWindow);
}
