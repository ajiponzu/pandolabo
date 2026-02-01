#pragma once

#include <string>
#include <vector>

#include "pandora/core/error.hpp"

namespace pandora::highlevel {

/// @brief Collects multiple errors and converts them into a single Result.
class ErrorCollector {
 private:
  std::vector<pandora::core::Error> m_errors{};

 public:
  /// @brief Add an error to the collector.
  void add(const pandora::core::Error& error) {
    m_errors.push_back(error);
  }

  /// @brief Return aggregated result.
  [[nodiscard]] pandora::core::VoidResult finalize() const;
};

}  // namespace pandora::highlevel
