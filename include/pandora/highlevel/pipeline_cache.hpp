#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "pandora/core/pipeline.hpp"

namespace pandora::highlevel {

/// @brief Thin cache for graphics/compute pipelines.
class PipelineCache {
 public:
  using PipelineBuilder =
      std::function<std::unique_ptr<pandora::core::Pipeline>(
          const std::unique_ptr<pandora::core::gpu::Context>&)>;

 private:
  std::reference_wrapper<const std::unique_ptr<pandora::core::gpu::Context>>
      m_contextOwner;
  std::unordered_map<std::string, std::unique_ptr<pandora::core::Pipeline>>
      m_cache;

 public:
  explicit PipelineCache(
      const std::unique_ptr<pandora::core::gpu::Context>& ptr_context)
      : m_contextOwner(ptr_context) {}

  /// @brief Get cached pipeline or create it using builder.
  pandora::core::Pipeline& getOrCreate(const std::string& key,
                                       const PipelineBuilder& builder);
};

}  // namespace pandora::highlevel
