#pragma once

#include <cstdint>
#if __has_include(<source_location>)
  #include <source_location>
#endif
#include <string>
#include <utility>
#include <variant>

namespace pandora::core {

#if defined(__cpp_lib_source_location)
using SourceLocation = std::source_location;
#else
struct SourceLocation {
  const char* file_name = "unknown";
  uint_least32_t line = 0u;
  uint_least32_t column = 0u;
  const char* function_name = "unknown";

  static constexpr SourceLocation current(const char* file = __FILE__,
                                          uint_least32_t line_value = __LINE__,
                                          uint_least32_t column_value = 0u,
                                          const char* function = __func__) {
    return SourceLocation{file, line_value, column_value, function};
  }
};
#endif

/// @brief Error category for Result-based error handling.
enum class ErrorType {
  Runtime = 0,
  Config,
  IO,
  GPU,
  Validation,
  Unknown,
};

/// @brief Structured error information for Result-based APIs.
class Error {
 private:
  ErrorType m_type = ErrorType::Unknown;
  std::string m_message{};
  std::string m_context{};
  SourceLocation m_location = SourceLocation::current();

 public:
  Error() = default;
  Error(ErrorType type,
        std::string message,
        SourceLocation location = SourceLocation::current(),
        std::string context = {})
      : m_type(type),
        m_message(std::move(message)),
        m_context(std::move(context)),
        m_location(location) {}

  static Error runtime(std::string message,
                       SourceLocation location = SourceLocation::current()) {
    return Error(ErrorType::Runtime, std::move(message), location);
  }

  static Error config(std::string message,
                      SourceLocation location = SourceLocation::current()) {
    return Error(ErrorType::Config, std::move(message), location);
  }

  static Error io(std::string message,
                  SourceLocation location = SourceLocation::current()) {
    return Error(ErrorType::IO, std::move(message), location);
  }

  static Error gpu(std::string message,
                   SourceLocation location = SourceLocation::current()) {
    return Error(ErrorType::GPU, std::move(message), location);
  }

  static Error validation(std::string message,
                          SourceLocation location = SourceLocation::current()) {
    return Error(ErrorType::Validation, std::move(message), location);
  }

  static Error unknown(std::string message,
                       SourceLocation location = SourceLocation::current()) {
    return Error(ErrorType::Unknown, std::move(message), location);
  }

  Error withContext(std::string context) const {
    Error copied = *this;
    if (!context.empty()) {
      if (!copied.m_context.empty()) {
        copied.m_context = std::move(context) + ": " + copied.m_context;
      } else {
        copied.m_context = std::move(context);
      }
    }
    return copied;
  }

  ErrorType type() const {
    return m_type;
  }

  const std::string& message() const {
    return m_message;
  }

  const std::string& context() const {
    return m_context;
  }

  const SourceLocation& location() const {
    return m_location;
  }

  std::string toString() const {
    const auto type_name = [this]() -> std::string {
      switch (m_type) {
        case ErrorType::Runtime:
          return "Runtime";
        case ErrorType::Config:
          return "Config";
        case ErrorType::IO:
          return "IO";
        case ErrorType::GPU:
          return "GPU";
        case ErrorType::Validation:
          return "Validation";
        case ErrorType::Unknown:
        default:
          return "Unknown";
      }
    }();

    auto location_text = std::string{m_location.file_name()} + ":"
                         + std::to_string(m_location.line()) + ":"
                         + std::to_string(m_location.column());

    if (m_context.empty()) {
      return type_name + " error: " + m_message + " (" + location_text + ")";
    }
    return type_name + " error: " + m_context + " | " + m_message + " ("
           + location_text + ")";
  }
};

/// @brief Result type for error-aware APIs.
template <typename T>
class Result {
 private:
  std::variant<T, Error> m_storage;

 public:
  Result(const T& value) : m_storage(value) {}
  Result(T&& value) : m_storage(std::move(value)) {}
  Result(const Error& error) : m_storage(error) {}
  Result(Error&& error) : m_storage(std::move(error)) {}

  bool isOk() const {
    return std::holds_alternative<T>(m_storage);
  }

  bool isError() const {
    return std::holds_alternative<Error>(m_storage);
  }

  const T& value() const {
    return std::get<T>(m_storage);
  }

  T& value() {
    return std::get<T>(m_storage);
  }

  T takeValue() {
    return std::move(std::get<T>(m_storage));
  }

  const Error& error() const {
    return std::get<Error>(m_storage);
  }

  Error& error() {
    return std::get<Error>(m_storage);
  }
};

using VoidResult = Result<std::monostate>;

inline VoidResult ok() {
  return VoidResult(std::monostate{});
}

inline Error errorRuntime(std::string message,
                          SourceLocation location = SourceLocation::current()) {
  return Error::runtime(std::move(message), location);
}

inline Error errorConfig(std::string message,
                         SourceLocation location = SourceLocation::current()) {
  return Error::config(std::move(message), location);
}

inline Error errorIo(std::string message,
                     SourceLocation location = SourceLocation::current()) {
  return Error::io(std::move(message), location);
}

inline Error errorGpu(std::string message,
                      SourceLocation location = SourceLocation::current()) {
  return Error::gpu(std::move(message), location);
}

inline Error errorValidation(
    std::string message, SourceLocation location = SourceLocation::current()) {
  return Error::validation(std::move(message), location);
}

inline Error errorUnknown(std::string message,
                          SourceLocation location = SourceLocation::current()) {
  return Error::unknown(std::move(message), location);
}

}  // namespace pandora::core

#define PANDORA_TRY(expr)             \
  do {                                \
    auto _pandora_result = (expr);    \
    if (!_pandora_result.isOk()) {    \
      return _pandora_result.error(); \
    }                                 \
  } while (false)

#define PANDORA_TRY_ASSIGN(lhs, expr)     \
  auto _pandora_result_##lhs = (expr);    \
  if (!_pandora_result_##lhs.isOk()) {    \
    return _pandora_result_##lhs.error(); \
  }                                       \
  auto lhs = std::move(_pandora_result_##lhs).takeValue();
