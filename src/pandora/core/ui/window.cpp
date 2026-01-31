#include "pandora/core/ui.hpp"

#ifdef _DEBUG
  #include <print>
#endif

std::unordered_map<uint64_t, std::unordered_set<int32_t>>
    pandora::core::ui::Window::s_inputKeySetMap;
std::unordered_map<uint64_t, pandora::core::Mouse>
    pandora::core::ui::Window::s_mouseMap;
std::unordered_map<uint64_t, bool> pandora::core::ui::Window::s_resizedBoolMap;

namespace {

#ifdef _DEBUG
void error_callback(int error, const char* description) {
  std::println(stderr, "GLFW Error {}: {}", error, description);
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

namespace pandora::core::ui {

Window::Window(const std::string& title, int32_t width, int32_t height) {
  if (!glfwInit()) {
    m_initError = errorRuntime("Failed to initialize GLFW.");
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_ptrWindow.reset(
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr));
  if (m_ptrWindow == nullptr) {
    glfwTerminate();
    m_initError = errorRuntime("Failed to create GLFW window.");
    return;
  }

  glfwMakeContextCurrent(m_ptrWindow.get());

#ifdef _DEBUG
  glfwSetErrorCallback(error_callback);
#endif

  glfwSetKeyCallback(m_ptrWindow.get(), callback_input_key);
  glfwSetCursorPosCallback(m_ptrWindow.get(), callback_cursor_pos);
  glfwSetScrollCallback(m_ptrWindow.get(), callback_scroll);
  glfwSetWindowSizeCallback(m_ptrWindow.get(), callback_resized);

  m_ptrWindowSurface = std::make_shared<gpu_ui::WindowSurface>(*m_ptrWindow);
  m_isInitialized = true;
}

Window::~Window() {
  if (m_isInitialized) {
    m_ptrWindow.reset();
    glfwTerminate();
  }

#ifdef _DEBUG
  glfwSetErrorCallback(nullptr);
#endif
}

Result<std::unique_ptr<Window>> Window::create(const std::string& title,
                                               int32_t width,
                                               int32_t height) {
  auto window = std::make_unique<Window>(title, width, height);
  const auto init_result = window->getInitResult();
  if (!init_result.isOk()) {
    return init_result.error();
  }
  return window;
}

bool Window::update() {
  if (!m_isInitialized) {
    return false;
  }
  setResizedBool(m_ptrWindow.get(), false);

  glfwPollEvents();

  for (auto& callback : m_callbacks) {
    callback();
  }

  return !glfwWindowShouldClose(m_ptrWindow.get());
}

}  // namespace pandora::core::ui
