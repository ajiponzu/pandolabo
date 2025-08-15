/*
 * ui.hpp - User interface abstractions for Pandolabo
 *
 * This header provides user interface functionality including:
 * - GLFW window management and event handling
 * - Keyboard and mouse input processing
 * - Integration with GPU surface creation for rendering
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "module_connection/gpu_ui.hpp"

namespace pandora::core {

/// @brief Keyboard key codes mapped from GLFW constants
/// This enum provides a type-safe wrapper around GLFW key codes
/// for consistent input handling across the application
enum class KeyCode : int32_t {
  Space = GLFW_KEY_SPACE,
  Apostrophe = GLFW_KEY_APOSTROPHE,
  Comma = GLFW_KEY_COMMA,
  Minus = GLFW_KEY_MINUS,
  Period = GLFW_KEY_PERIOD,
  Slash = GLFW_KEY_SLASH,
  Num0 = GLFW_KEY_0,
  Num1 = GLFW_KEY_1,
  Num2 = GLFW_KEY_2,
  Num3 = GLFW_KEY_3,
  Num4 = GLFW_KEY_4,
  Num5 = GLFW_KEY_5,
  Num6 = GLFW_KEY_6,
  Num7 = GLFW_KEY_7,
  Num8 = GLFW_KEY_8,
  Num9 = GLFW_KEY_9,
  Semicolon = GLFW_KEY_SEMICOLON,
  Equal = GLFW_KEY_EQUAL,
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  J = GLFW_KEY_J,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  LeftBracket = GLFW_KEY_LEFT_BRACKET,
  Backslash = GLFW_KEY_BACKSLASH,
  RightBracket = GLFW_KEY_RIGHT_BRACKET,
  GraveAccent = GLFW_KEY_GRAVE_ACCENT,
  World1 = GLFW_KEY_WORLD_1,
  World2 = GLFW_KEY_WORLD_2,
  Escape = GLFW_KEY_ESCAPE,
  Enter = GLFW_KEY_ENTER,
  Tab = GLFW_KEY_TAB,
  Backspace = GLFW_KEY_BACKSPACE,
  Insert = GLFW_KEY_INSERT,
  Delete = GLFW_KEY_DELETE,
  Right = GLFW_KEY_RIGHT,
  Left = GLFW_KEY_LEFT,
  Down = GLFW_KEY_DOWN,
  Up = GLFW_KEY_UP,
  PageUp = GLFW_KEY_PAGE_UP,
  PageDown = GLFW_KEY_PAGE_DOWN,
  Home = GLFW_KEY_HOME,
  End = GLFW_KEY_END,
  CapsLock = GLFW_KEY_CAPS_LOCK,
  ScrollLock = GLFW_KEY_SCROLL_LOCK,
  NumLock = GLFW_KEY_NUM_LOCK,
  PrintScreen = GLFW_KEY_PRINT_SCREEN,
  Pause = GLFW_KEY_PAUSE,
  F1 = GLFW_KEY_F1,
  F2 = GLFW_KEY_F2,
  F3 = GLFW_KEY_F3,
  F4 = GLFW_KEY_F4,
  F5 = GLFW_KEY_F5,
  F6 = GLFW_KEY_F6,
  F7 = GLFW_KEY_F7,
  F8 = GLFW_KEY_F8,
};

constexpr auto convertKeyCodeToInt(KeyCode key) {
  return static_cast<int32_t>(key);
}

constexpr auto convertIntToKeyCode(int32_t key) {
  return static_cast<KeyCode>(key);
}

/// @brief Mouse state structure containing position and scroll information
/// Used to track current mouse cursor position and scroll wheel state
struct Mouse {
  double pos_x = 0.0;     ///< Current X coordinate of mouse cursor
  double pos_y = 0.0;     ///< Current Y coordinate of mouse cursor
  double scroll_x = 0.0;  ///< Horizontal scroll wheel delta
  double scroll_y = 0.0;  ///< Vertical scroll wheel delta
};

namespace ui {

/// @brief GLFW window wrapper class for user interface management
/// This class manages window creation, event handling, and input processing
/// using the GLFW API. It provides a high-level interface for:
/// - Window lifecycle management (creation, destruction, resizing)
/// - Keyboard input tracking and event handling
/// - Mouse position and scroll tracking
/// - Integration with Vulkan surface creation for rendering
class Window {
 private:
  static std::unordered_map<uint64_t, std::unordered_set<int32_t>> s_inputKeySetMap;
  static std::unordered_map<uint64_t, Mouse> s_mouseMap;
  static std::unordered_map<uint64_t, bool> s_resizedBoolMap;

  std::shared_ptr<gpu_ui::WindowSurface> m_ptrWindowSurface;
  GLFWwindow* m_ptrWindow;
  std::vector<std::function<void()>> m_callbacks;

 private:
  static auto convertWindowPtr(const GLFWwindow* ptr_window) {
    return reinterpret_cast<uint64_t>(ptr_window);
  }

 public:
  static void insertInputKey(const GLFWwindow* ptr_window, const int32_t key) {
    s_inputKeySetMap[convertWindowPtr(ptr_window)].insert(key);
  }
  static void eraseInputKey(const GLFWwindow* ptr_window, const int32_t key) {
    s_inputKeySetMap[convertWindowPtr(ptr_window)].erase(key);
  }
  static void setMousePos(const GLFWwindow* ptr_window, const double x, const double y) {
    s_mouseMap[convertWindowPtr(ptr_window)].pos_x = x;
    s_mouseMap[convertWindowPtr(ptr_window)].pos_y = y;
  }
  static void setMouseScroll(const GLFWwindow* ptr_window, const double x, const double y) {
    s_mouseMap[convertWindowPtr(ptr_window)].scroll_x = x;
    s_mouseMap[convertWindowPtr(ptr_window)].scroll_y = y;
  }
  static void setResizedBool(const GLFWwindow* ptr_window, const bool is_resized) {
    s_resizedBoolMap[convertWindowPtr(ptr_window)] = is_resized;
  }

  Window(const std::string& title, const int32_t width, const int32_t height);

  // Rule of Five
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&) = default;
  Window& operator=(Window&&) = default;

  const auto& getMouse() const {
    return s_mouseMap.at(convertWindowPtr(m_ptrWindow));
  }
  const auto& getWindowSurface() const {
    return m_ptrWindowSurface;
  }
  const auto isResized() const {
    return s_resizedBoolMap.at(convertWindowPtr(m_ptrWindow));
  }

  /// @brief Update window and input
  /// @return True if window should continue running, false if should close
  bool update();

  /// @brief Add callback function for input handling
  /// You can add reference binding lambda functions for custom input processing.
  /// @param callback Function or function object to be called during input processing
  void addCallback(std::function<void()> callback) {
    m_callbacks.emplace_back(callback);
  }

  bool findInputKey(KeyCode key) const {
    return s_inputKeySetMap.at(convertWindowPtr(m_ptrWindow)).contains(convertKeyCodeToInt(key));
  }
};

}  // namespace ui

}  // namespace pandora::core
