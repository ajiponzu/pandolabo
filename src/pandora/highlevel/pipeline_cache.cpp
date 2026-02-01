#include "pandora/highlevel/pipeline_cache.hpp"

namespace pandora::highlevel {

pandora::core::Pipeline& PipelineCache::getOrCreate(
    const std::string& key, const PipelineBuilder& builder) {
  if (auto it = m_cache.find(key); it != m_cache.end()) {
    return *it->second;
  }

  auto created = builder(m_contextOwner.get());
  auto& ref = *created;
  m_cache.emplace(key, std::move(created));
  return ref;
}

}  // namespace pandora::highlevel
