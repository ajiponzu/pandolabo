#pragma once
#include <cstdint>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>

namespace pandora::core::err {

// Domains roughly partition subsystems; keep values stable for
// logging/telemetry.
enum class Domain : std::uint16_t {
  gpu = 1,
  gpu_swapchain = 2,
  gpu_validation = 3,
  gpu_shader = 4,
  gpu_pipeline = 5,
  memory = 10,
  internal = 100,
};

// Broad category for operational reaction.
enum class Severity : std::uint8_t {
  note = 0,
  warning = 1,
  recoverable = 2,
  fatal = 3
};

// Fine grained code (extend cautiously; stable numeric values aid external
// tooling).
enum class Code : std::uint16_t {
  ok = 0,
  unknown = 1,
  // Generic
  invalid_argument = 10,
  invalid_state = 11,
  timeout = 12,
  retry_later = 13,
  out_of_memory = 14,
  device_lost = 15,
  swapchain_out_of_date = 16,
  shader_compile_failed = 17,
  note = 18,
};

struct Error final {
  Domain domain{Domain::internal};
  Code code{Code::unknown};
  Severity severity{Severity::recoverable};
  std::string message;            // Human readable summary (short)
  std::uint32_t native_code = 0;  // Original API status (e.g. VkResult numeric)
  std::uint64_t context_id = 0;   // User supplied correlation id
  std::source_location where = std::source_location::current();
};

class ErrorException : public std::runtime_error {
 public:
  explicit ErrorException(const Error& e)
      : std::runtime_error(e.message), error_(e) {}
  const Error& error() const noexcept {
    return error_;
  }

 private:
  Error error_;
};

[[noreturn]] inline void throw_error(const Error& e) {
  throw ErrorException(e);
}

inline std::string_view to_string(Domain d) {
  switch (d) {
    case Domain::gpu:
      return "gpu";
    case Domain::gpu_swapchain:
      return "gpu_swapchain";
    case Domain::gpu_validation:
      return "gpu_validation";
    case Domain::gpu_shader:
      return "gpu_shader";
    case Domain::gpu_pipeline:
      return "gpu_pipeline";
    case Domain::memory:
      return "memory";
    case Domain::internal:
      return "internal";
  }
  return "unknown";
}

inline std::string_view to_string(Severity s) {
  switch (s) {
    case Severity::note:
      return "note";
    case Severity::warning:
      return "warning";
    case Severity::recoverable:
      return "recoverable";
    case Severity::fatal:
      return "fatal";
  }
  return "?";
}

inline std::string_view to_string(Code c) {
  switch (c) {
    case Code::ok:
      return "ok";
    case Code::unknown:
      return "unknown";
    case Code::invalid_argument:
      return "invalid_argument";
    case Code::invalid_state:
      return "invalid_state";
    case Code::timeout:
      return "timeout";
    case Code::retry_later:
      return "retry_later";
    case Code::out_of_memory:
      return "out_of_memory";
    case Code::device_lost:
      return "device_lost";
    case Code::swapchain_out_of_date:
      return "swapchain_out_of_date";
    case Code::shader_compile_failed:
      return "shader_compile_failed";
    case Code::note:
      return "note";
  }
  return "?";
}

}  // namespace pandora::core::err
