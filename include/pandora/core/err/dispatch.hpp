#pragma once
#include <atomic>
#include <cstdint>
#include <functional>

#include "pandora/core/err/error.hpp"

namespace pandora::core::err {

// Aggregated counters for lightweight telemetry; all atomics to allow lock-free
// updates.
struct ErrorStats {
  std::atomic<uint64_t> total{0};
  std::atomic<uint64_t> fatal{0};
  std::atomic<uint64_t> recoverable{0};
  std::atomic<uint64_t> warning{0};
  std::atomic<uint64_t> note{0};
};

// Expose a reference so callers can periodically snapshot.
ErrorStats& global_error_stats();

// Sink signature: return true to indicate the error was fully handled (optional
// future use).
using ErrorSink = bool (*)(const Error&);

// Register (replace) global sink. Returns previous sink (may be nullptr).
ErrorSink set_error_sink(ErrorSink sink) noexcept;

// Dispatch called by error generation points (e.g., check_vk). Increments stats
// + calls sink if present.
void dispatch_error(const Error& e) noexcept;

}  // namespace pandora::core::err
