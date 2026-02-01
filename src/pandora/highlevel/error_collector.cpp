#include "pandora/highlevel/error_collector.hpp"

#include <format>

namespace pandora::highlevel {

pandora::core::VoidResult ErrorCollector::finalize() const {
  if (m_errors.empty()) {
    return pandora::core::ok();
  }

  std::string detail;
  detail.reserve(128u * m_errors.size());
  for (size_t idx = 0u; const auto& error : m_errors) {
    if (idx != 0u) {
      detail += " | ";
    }
    detail += error.toString();
    idx += 1u;
  }

  return pandora::core::Error::runtime(
      std::format("Collected {} error(s): {}", m_errors.size(), detail));
}

}  // namespace pandora::highlevel
