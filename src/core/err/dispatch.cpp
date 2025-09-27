#include "pandora/core/err/dispatch.hpp"

namespace pandora::core::err {
namespace {
std::atomic<ErrorSink> g_sink{nullptr};
ErrorStats g_stats;  // zero-initialized
}  // namespace

ErrorStats& global_error_stats() {
  return g_stats;
}

ErrorSink set_error_sink(ErrorSink sink) noexcept {
  return g_sink.exchange(sink, std::memory_order_acq_rel);
}

void dispatch_error(const Error& e) noexcept {
  g_stats.total.fetch_add(1, std::memory_order_relaxed);
  switch (e.severity) {
    case Severity::fatal:
      g_stats.fatal.fetch_add(1, std::memory_order_relaxed);
      break;
    case Severity::recoverable:
      g_stats.recoverable.fetch_add(1, std::memory_order_relaxed);
      break;
    case Severity::warning:
      g_stats.warning.fetch_add(1, std::memory_order_relaxed);
      break;
    case Severity::note:
      g_stats.note.fetch_add(1, std::memory_order_relaxed);
      break;
  }
  if (auto sink = g_sink.load(std::memory_order_acquire)) {
    // Ignore return value for now; future: conditional suppression/escalation.
    (void)sink(e);
  }
}

}  // namespace pandora::core::err
