#include "basic_compute_hl.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <print>
#include <span>

namespace samples::highlevel {

BasicComputeHL::BasicComputeHL() {
  m_ptrContext = std::make_unique<plc::gpu::Context>(nullptr);

  m_ptrComputeRunner = std::make_unique<plh::ComputeRunner>(m_ptrContext);
  m_ptrTransfer = std::make_unique<plh::ResourceTransfer>(
      m_ptrContext, plc::QueueFamilyType::Compute);

  m_ptrUniformBuffer =
      plc::createUniqueUniformBuffer(m_ptrContext, sizeof(float_t));
  const auto uniform_mapped_address =
      m_ptrUniformBuffer->mapMemory(m_ptrContext);
  std::fill_n(reinterpret_cast<float_t*>(uniform_mapped_address),
              m_ptrUniformBuffer->getSize() / sizeof(float_t),
              3.14f);
  m_ptrUniformBuffer->unmapMemory(m_ptrContext);

  m_ptrInputStorageBuffer = plc::createUniqueStorageBuffer(
      m_ptrContext, plc::TransferType::TransferDst, sizeof(uint32_t) * 1024u);
  m_ptrOutputStorageBuffer =
      plc::createUniqueStorageBuffer(m_ptrContext,
                                     plc::TransferType::TransferSrcDst,
                                     sizeof(uint32_t) * 1024u);

  const auto shader_result = constructShaderResources();
  if (!shader_result.isOk()) {
    std::println(stderr,
                 "BasicComputeHL shader load error: {}",
                 shader_result.error().toString());
    return;
  }

  m_isInitialized = true;
}

BasicComputeHL::~BasicComputeHL() {
  if (m_ptrContext) {
    m_ptrContext->getPtrDevice()->waitIdle();
  }
}

void BasicComputeHL::run() {
  if (!m_isInitialized) {
    std::println(stderr, "BasicComputeHL is not initialized.");
    return;
  }

  std::vector<uint32_t> input(m_ptrInputStorageBuffer->getSize()
                              / sizeof(uint32_t));
  std::fill(input.begin(), input.end(), 5u);

  const auto upload_result = m_ptrTransfer->uploadBuffer(
      *m_ptrInputStorageBuffer,
      std::as_bytes(std::span(input.data(), input.size())));
  if (!upload_result.isOk()) {
    std::println(stderr,
                 "BasicComputeHL upload error: {}",
                 upload_result.error().toString());
    return;
  }

  auto cmd_result = m_ptrComputeRunner->begin();
  if (!cmd_result.isOk()) {
    std::println(stderr,
                 "BasicComputeHL begin error: {}",
                 cmd_result.error().toString());
    return;
  }
  auto cmd = std::move(cmd_result).takeValue();

  const auto record_result = m_ptrComputeRunner->record(
      cmd,
      [&](plc::ComputeCommandBuffer& buffer) { return recordCompute(buffer); });
  if (!record_result.isOk()) {
    std::println(stderr,
                 "BasicComputeHL record error: {}",
                 record_result.error().toString());
    return;
  }

  const auto submit_result = m_ptrComputeRunner->submit();
  if (!submit_result.isOk()) {
    std::println(stderr,
                 "BasicComputeHL submit error: {}",
                 submit_result.error().toString());
    return;
  }
  m_ptrComputeRunner->queueWaitIdle();

  std::vector<std::byte> readback(m_ptrOutputStorageBuffer->getSize());
  const auto read_result =
      m_ptrTransfer->readbackBuffer(*m_ptrOutputStorageBuffer, readback);
  if (!read_result.isOk()) {
    std::println(stderr,
                 "BasicComputeHL readback error: {}",
                 read_result.error().toString());
    return;
  }

  const auto* result = reinterpret_cast<const uint32_t*>(readback.data());
  for (size_t idx = 0u; idx < readback.size() / sizeof(uint32_t); idx += 1u) {
    std::println(stdout, "idx[{}]: {}", idx, result[idx]);
  }
}

plc::VoidResult BasicComputeHL::constructShaderResources() {
  plh::ShaderLibrary shader_library(m_ptrContext);

  PANDORA_TRY_ASSIGN(
      spirv_binary,
      shader_library.load("examples/core/basic_compute/basic.comp"));

  m_shaderModuleMap["compute"] = std::move(spirv_binary);

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"compute"});

  m_ptrDescriptorSetLayout = std::make_unique<plc::gpu::DescriptorSetLayout>(
      m_ptrContext, description_unit);
  m_ptrDescriptorSet = std::make_unique<plc::gpu::DescriptorSet>(
      m_ptrContext, *m_ptrDescriptorSetLayout);

  std::vector<plc::gpu::BufferDescription> buffer_descriptions;
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("UniformNumber"),
      *m_ptrUniformBuffer);
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("Output"),
      *m_ptrOutputStorageBuffer);
  buffer_descriptions.emplace_back(
      description_unit.getDescriptorInfoMap().at("Input"),
      *m_ptrInputStorageBuffer);

  m_ptrDescriptorSet->updateDescriptorSet(
      m_ptrContext, buffer_descriptions, {});

  m_ptrComputePipeline =
      std::make_unique<plc::Pipeline>(m_ptrContext,
                                      description_unit,
                                      *m_ptrDescriptorSetLayout,
                                      plc::PipelineBind::Compute);
  m_ptrComputePipeline->constructComputePipeline(
      m_ptrContext, m_shaderModuleMap.at("compute"));

  return plc::ok();
}

plc::VoidResult BasicComputeHL::recordCompute(
    plc::ComputeCommandBuffer& command_buffer) {
  command_buffer.bindPipeline(*m_ptrComputePipeline);
  command_buffer.bindDescriptorSet(*m_ptrComputePipeline, *m_ptrDescriptorSet);
  command_buffer.compute(plc::ComputeWorkGroupSize{4u, 1u, 1u});
  return plc::ok();
}

}  // namespace samples::highlevel
