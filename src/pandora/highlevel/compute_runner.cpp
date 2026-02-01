#include "pandora/highlevel/compute_runner.hpp"

namespace pandora::highlevel {

pandora::core::CommandDriver& ComputeRunner::ensureDriver() {
  if (!m_computeDriver) {
    m_computeDriver = std::make_unique<pandora::core::CommandDriver>(
        m_contextOwner.get(), pandora::core::QueueFamilyType::Compute);
  }
  return *m_computeDriver;
}

pandora::core::Result<pandora::core::ComputeCommandBuffer>
ComputeRunner::begin() {
  auto& driver = ensureDriver();
  driver.resetAllCommandPools(m_contextOwner.get());
  auto cmd = driver.getCompute();
  cmd.begin();
  return cmd;
}

pandora::core::VoidResult ComputeRunner::record(
    pandora::core::ComputeCommandBuffer& command_buffer,
    const RecordFn& record_fn) {
  const auto record_result = record_fn(command_buffer);
  if (!record_result.isOk()) {
    return record_result;
  }

  command_buffer.end();
  return pandora::core::ok();
}

pandora::core::VoidResult ComputeRunner::submit(
    const pandora::core::SubmitSemaphoreGroup& semaphore_group) {
  auto& driver = ensureDriver();
  driver.submit(semaphore_group);
  return pandora::core::ok();
}

void ComputeRunner::queueWaitIdle() {
  auto& driver = ensureDriver();
  driver.queueWaitIdle();
}

}  // namespace pandora::highlevel
