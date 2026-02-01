#include "streaming_resources_hl.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <print>
#include <thread>
#include <vector>

namespace samples::highlevel {

StreamingResourcesHL::StreamingResourcesHL()
    : m_startTime(std::chrono::high_resolution_clock::now()),
      m_randomGenerator(std::random_device{}()),
      m_colorDist(0.2f, 1.0f),
      m_currentSemaphoreValue(0u),
      m_triangleSpawnAngle(0.0f),
      m_lastSpawnTime(0.0f) {
  std::println("Initializing StreamingResourcesHL...");

  try {
    std::println("Creating Window...");
    auto window_result =
        plc::ui::Window::create("Streaming Resources (HL)", 800u, 600u);
    if (!window_result.isOk()) {
      std::println(stderr,
                   "StreamingResourcesHL window error: {}",
                   window_result.error().toString());
      return;
    }
    m_ptrWindow = std::move(window_result).takeValue();
    std::println("Window created successfully.");

    std::println("Creating GPU Context...");
    m_ptrContext =
        std::make_unique<plc::gpu::Context>(m_ptrWindow->getWindowSurface());
    std::println("GPU Context created successfully.");

    m_ptrWindow->addCallback([&]() {
      if (!m_ptrWindow->isResized()) {
        return;
      }

      m_ptrContext->resetSwapchain();
      constructRenderpass(true);
    });

    std::println("Creating Command Drivers...");
  } catch (const std::exception& e) {
    std::println(stderr, "Exception in constructor: {}", e.what());
    throw;
  }

  for (size_t idx = 0u; idx < m_ptrContext->getPtrSwapchain()->getImageCount();
       idx += 1u) {
    m_ptrGraphicCommandDriver.push_back(std::make_unique<plc::CommandDriver>(
        m_ptrContext, plc::QueueFamilyType::Graphics));
    m_ptrTransferCommandDriver.push_back(std::make_unique<plc::CommandDriver>(
        m_ptrContext, plc::QueueFamilyType::Transfer));
  }
  std::println("Command Drivers created successfully.");

  m_currentSemaphoreValue = 0u;

  std::println("Creating frame-specific buffers...");
  const size_t frame_count = m_ptrContext->getPtrSwapchain()->getImageCount();
  const size_t buffer_size = sizeof(Vertex) * MAX_TRIANGLES * 3u;
  for (size_t idx = 0u; idx < frame_count; idx += 1u) {
    m_ptrVertexBuffers.push_back(
        plc::createUniqueVertexBuffer(m_ptrContext, buffer_size));
    m_stagingBuffers.push_back(
        plc::createStagingBufferToGPU(m_ptrContext, buffer_size));
  }
  std::println("Buffers created successfully.");

  std::println("Constructing shader resources...");
  const auto shader_result = constructShaderResources();
  if (!shader_result.isOk()) {
    std::println(stderr,
                 "StreamingResourcesHL shader load error: {}",
                 shader_result.error().toString());
    return;
  }
  std::println("Shader resources constructed.");

  std::println("Constructing renderpass...");
  constructRenderpass();
  std::println("Renderpass constructed.");

  std::println("Constructing graphics pipeline...");
  constructGraphicPipeline();
  std::println("Graphics pipeline constructed.");

  spawnNewTriangle(0.0f);
  std::println("Generated initial triangle, total: {}", m_triangleInfos.size());
  std::println("StreamingResourcesHL initialization complete!");

  m_isInitialized = true;
}

StreamingResourcesHL::~StreamingResourcesHL() {
  if (m_ptrContext) {
    m_ptrContext->getPtrDevice()->waitIdle();
  }
}

void StreamingResourcesHL::run() {
  if (!m_isInitialized) {
    std::println(stderr, "StreamingResourcesHL is not initialized.");
    return;
  }
  std::println("🚀 Streaming Resources HL Example Start");

  while (m_ptrWindow->update()) {
    const auto update_result = updateVertexData();
    if (!update_result.isOk()) {
      std::println(stderr,
                   "StreamingResourcesHL update error: {}",
                   update_result.error().toString());
      return;
    }

    const auto graphic_result = setGraphicCommands();
    if (!graphic_result.isOk()) {
      std::println(stderr,
                   "StreamingResourcesHL render error: {}",
                   graphic_result.error().toString());
      return;
    }
  }

  std::println("✅ Streaming Resources HL Example Done");
}

plc::VoidResult StreamingResourcesHL::constructShaderResources() {
  plh::ShaderLibrary shader_library(m_ptrContext);

  PANDORA_TRY_ASSIGN(
      vertex_shader,
      shader_library.load("examples/core/streaming_resources/streaming.vert"));
  PANDORA_TRY_ASSIGN(
      fragment_shader,
      shader_library.load("examples/core/streaming_resources/streaming.frag"));

  m_shaderModuleMap["vertex"] = std::move(vertex_shader);
  m_shaderModuleMap["fragment"] = std::move(fragment_shader);

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"vertex", "fragment"});

  m_ptrDescriptorSetLayout = std::make_unique<plc::gpu::DescriptorSetLayout>(
      m_ptrContext, description_unit);
  m_ptrDescriptorSet = std::make_unique<plc::gpu::DescriptorSet>(
      m_ptrContext, *m_ptrDescriptorSetLayout);

  m_ptrPipeline = std::make_unique<plc::Pipeline>(m_ptrContext,
                                                  description_unit,
                                                  *m_ptrDescriptorSetLayout,
                                                  plc::PipelineBind::Graphics);

  return plc::ok();
}

void StreamingResourcesHL::constructRenderpass(const bool is_resized) {
  plc::AttachmentList attachment_list;

  const auto backbuffer_attach_index = [&] {
    const auto attachment_description =
        plc::AttachmentDescription{}
            .setFormat(m_ptrContext->getPtrSwapchain()->getImageFormat())
            .setSamples(plc::ImageSampleCount::v1)
            .setLoadOp(plc::AttachmentLoadOp::Clear)
            .setStoreOp(plc::AttachmentStoreOp::Store)
            .setStencilLoadOp(plc::AttachmentLoadOp::DontCare)
            .setStencilStoreOp(plc::AttachmentStoreOp::DontCare)
            .setLayouts(plc::ImageLayout::Undefined,
                        plc::ImageLayout::PresentSrc);

    return attachment_list.append(
        attachment_description,
        plc::ClearColor{}.setColor(0.1f, 0.1f, 0.1f, 1.0f));
  }();

  plc::SubpassGraph subpass_graph;

  plc::SubpassNode subpass_node(plc::PipelineBind::Graphics, 0u);
  subpass_node.attachColor(
      plc::AttachmentReference{}
          .setIndex(backbuffer_attach_index)
          .setLayout(plc::ImageLayout::ColorAttachmentOptimal));
  m_subpassIndexMap["main"] = subpass_graph.appendNode(subpass_node);

  subpass_graph.appendEdge(
      plc::SubpassEdge{}
          .setDstIndex(m_subpassIndexMap.at("main"))
          .setSrcAccess({plc::AccessFlag::Unknown})
          .setDstAccess({plc::AccessFlag::ColorAttachmentWrite})
          .setSrcStages({plc::PipelineStage::ColorAttachmentOutput})
          .setDstStages({plc::PipelineStage::ColorAttachmentOutput})
          .setDependencyFlag(plc::DependencyFlag::ByRegion));

  if (is_resized) {
    m_ptrRenderKit->resetFramebuffer(
        m_ptrContext,
        attachment_list,
        m_ptrWindow->getWindowSurface()->getWindowSize(),
        true);
  } else {
    m_ptrRenderKit = std::make_unique<plc::RenderKit>(
        m_ptrContext,
        attachment_list,
        subpass_graph,
        m_ptrWindow->getWindowSurface()->getWindowSize(),
        true);
  }
}

void StreamingResourcesHL::constructGraphicPipeline() {
  const auto ptr_graphic_info =
      plc::pipeline::GraphicInfoBuilder::create()
          .setVertexInput(
              plc::pipeline::VertexInput{}
                  .addBinding(0u, sizeof(Vertex), plc::VertexInputRate::Vertex)
                  .addAttribute(0u,
                                0u,
                                plc::DataFormat::R32G32Sfloat,
                                offsetof(Vertex, pos))
                  .addAttribute(1u,
                                0u,
                                plc::DataFormat::R32G32B32Sfloat,
                                offsetof(Vertex, color)))
          .setInputAssembly(
              plc::pipeline::InputAssembly{}
                  .withTopology(plc::PrimitiveTopology::TriangleList)
                  .withRestart(false))
          .setViewportState(plc::pipeline::ViewportState{}
                                .withScissor({800u, 600u})
                                .withViewport({800.0f, 600.0f}, 0.0f, 1.0f))
          .setRasterization(plc::pipeline::Rasterization{}
                                .withPolygonMode(plc::PolygonMode::Fill)
                                .withCullMode(plc::CullMode::Back)
                                .withFrontFace(plc::FrontFace::Clockwise)
                                .withLineWidth(1.0f))
          .setColorBlend(
              plc::pipeline::ColorBlend{}
                  .withLogicOp(false, plc::LogicOp::Copy)
                  .addAttachment(plc::ColorBlendAttachment{}.setColorComponents(
                      {plc::ColorComponent::R,
                       plc::ColorComponent::G,
                       plc::ColorComponent::B,
                       plc::ColorComponent::A})))
          .setDynamicState(plc::pipeline::DynamicState{}
                               .addState(plc::DynamicOption::Viewport)
                               .addState(plc::DynamicOption::Scissor))
          .build();

  m_ptrPipeline->constructGraphicsPipeline(m_ptrContext,
                                           m_shaderModuleMap,
                                           {"vertex", "fragment"},
                                           ptr_graphic_info,
                                           m_ptrRenderKit->getRenderpass(),
                                           m_subpassIndexMap.at("main"));
}

plc::VoidResult StreamingResourcesHL::updateVertexData() {
  auto current_time = std::chrono::high_resolution_clock::now();
  auto time =
      std::chrono::duration<float_t>(current_time - m_startTime).count();

  if (time - m_lastSpawnTime > SPAWN_INTERVAL) {
    spawnNewTriangle(time);
    m_lastSpawnTime = time;
    std::println("Spawned triangle, total: {}", m_triangleInfos.size());
  }

  removeOldTriangles(time);

  auto vertices = getCurrentTriangles(time);
  std::println("Current vertices: {} ({} triangles)",
               vertices.size(),
               vertices.size() / 3u);

  if (vertices.empty()) {
    return plc::ok();
  }

  const auto& ptr_swapchain = m_ptrContext->getPtrSwapchain();
  const uint32_t frame_index = ptr_swapchain->getFrameSyncIndex();

  auto& staging_buffer = m_stagingBuffers[frame_index];
  auto& vertex_buffer = m_ptrVertexBuffers[frame_index];
  auto& transfer_driver = m_ptrTransferCommandDriver[frame_index];

  size_t vertex_data_size = vertices.size() * sizeof(Vertex);
  auto mapped_address = staging_buffer.mapMemory(m_ptrContext);
  std::memcpy(mapped_address, vertices.data(), vertex_data_size);
  staging_buffer.unmapMemory(m_ptrContext);

  if (!m_currentTimelineSemaphore) {
    m_currentTimelineSemaphore =
        std::make_unique<plc::gpu::TimelineSemaphore>(m_ptrContext);
    m_currentSemaphoreValue = 0u;
  }

  constexpr uint64_t TIMELINE_RESET_THRESHOLD = 1000000u;
  if (m_currentSemaphoreValue > TIMELINE_RESET_THRESHOLD) {
    std::println(
        "Timeline Semaphore value ({}) exceeded threshold, recreating...",
        m_currentSemaphoreValue);
    m_currentTimelineSemaphore =
        std::make_unique<plc::gpu::TimelineSemaphore>(m_ptrContext);
    m_currentSemaphoreValue = 0u;
  }

  m_currentSemaphoreValue += 1u;

  const auto command_buffer = transfer_driver->getTransfer();
  command_buffer.begin();
  command_buffer.copyBuffer(staging_buffer, *vertex_buffer);

  const auto queue_family_indices = std::make_pair(
      transfer_driver->getQueueFamilyIndex(),
      m_ptrGraphicCommandDriver.at(frame_index)->getQueueFamilyIndex());

  PANDORA_TRY_ASSIGN(buffer_barrier,
                     plc::gpu::BufferBarrierBuilder::create()
                         .setBuffer(*vertex_buffer)
                         .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
                         .setDstAccessFlags({plc::AccessFlag::TransferRead})
                         .setSrcStages({plc::PipelineStage::Transfer})
                         .setDstStages({plc::PipelineStage::Transfer})
                         .setSrcQueueFamilyIndex(queue_family_indices.first)
                         .setDstQueueFamilyIndex(queue_family_indices.second)
                         .build());

  command_buffer.setPipelineBarrier(
      plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));

  command_buffer.end();

  transfer_driver->submit(
      plc::SubmitSemaphoreGroup{}
          .setWaitSemaphores({plc::SubmitSemaphore{}
                                  .setSemaphore(*m_currentTimelineSemaphore)
                                  .setValue(0u)
                                  .setStageMask(plc::PipelineStage::Transfer)})
          .setSignalSemaphores(
              {plc::SubmitSemaphore{}
                   .setSemaphore(*m_currentTimelineSemaphore)
                   .setValue(m_currentSemaphoreValue)
                   .setStageMask(plc::PipelineStage::Transfer)}));

  return plc::ok();
}

plc::VoidResult StreamingResourcesHL::setGraphicCommands() {
  const auto& ptr_swapchain = m_ptrContext->getPtrSwapchain();
  const auto update_result =
      ptr_swapchain->updateImageIndex(m_ptrContext->getPtrDevice());
  if (!update_result.isOk()) {
    return update_result.error();
  }
  m_ptrRenderKit->updateIndex(ptr_swapchain->getImageIndex());

  const uint32_t frame_index = ptr_swapchain->getFrameSyncIndex();
  auto& frame_vertex_buffer = m_ptrVertexBuffers[frame_index];

  auto current_time = std::chrono::high_resolution_clock::now();
  auto time =
      std::chrono::duration<float_t>(current_time - m_startTime).count();
  auto vertices = getCurrentTriangles(time);

  size_t vertex_count = vertices.size();

  const auto command_buffer =
      m_ptrGraphicCommandDriver.at(frame_index)->getGraphic();

  command_buffer.begin();

  const auto queue_family_indices = std::make_pair(
      m_ptrTransferCommandDriver.at(frame_index)->getQueueFamilyIndex(),
      m_ptrGraphicCommandDriver.at(frame_index)->getQueueFamilyIndex());

  PANDORA_TRY_ASSIGN(
      buffer_barrier,
      plc::gpu::BufferBarrierBuilder::create()
          .setBuffer(*frame_vertex_buffer)
          .setSrcAccessFlags({plc::AccessFlag::TransferWrite})
          .setDstAccessFlags({plc::AccessFlag::VertexAttributeRead})
          .setSrcStages({plc::PipelineStage::Transfer})
          .setDstStages({plc::PipelineStage::VertexAttributeInput})
          .setSrcQueueFamilyIndex(queue_family_indices.first)
          .setDstQueueFamilyIndex(queue_family_indices.second)
          .build());

  command_buffer.setPipelineBarrier(
      plc::BarrierDependency{}.setBufferBarriers({buffer_barrier}));

  PANDORA_TRY(command_buffer.beginRenderpass(
      *m_ptrRenderKit,
      m_ptrWindow->getWindowSurface()->getWindowSize(),
      plc::SubpassContents::Inline));

  command_buffer.bindPipeline(*m_ptrPipeline);
  command_buffer.bindDescriptorSet(*m_ptrPipeline, *m_ptrDescriptorSet);

  const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();
  command_buffer.setViewport(plc::gpu_ui::GraphicalSize<float_t>(
                                 static_cast<float_t>(window_size.width),
                                 static_cast<float_t>(window_size.height)),
                             0.0f,
                             1.0f);
  command_buffer.setScissor(window_size);

  command_buffer.bindVertexBuffer(*frame_vertex_buffer, 0u);
  command_buffer.draw(static_cast<uint32_t>(vertex_count), 1u, 0u, 0u);

  command_buffer.endRenderpass();
  command_buffer.end();

  const auto image_semaphore = ptr_swapchain->getImageAvailableSemaphore();
  const auto finished_semaphore = ptr_swapchain->getFinishedSemaphore();
  const auto finished_fence = ptr_swapchain->getFence();

  if (m_currentTimelineSemaphore) {
    m_ptrGraphicCommandDriver.at(frame_index)
        ->submit(plc::SubmitSemaphoreGroup{}
                     .setWaitSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(*m_currentTimelineSemaphore)
                              .setValue(m_currentSemaphoreValue)
                              .setStageMask(
                                  plc::PipelineStage::VertexAttributeInput),
                          plc::SubmitSemaphore{}
                              .setSemaphore(image_semaphore)
                              .setStageMask(
                                  plc::PipelineStage::ColorAttachmentOutput)})
                     .setSignalSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(finished_semaphore)
                              .setStageMask(plc::PipelineStage::AllGraphics)}),
                 finished_fence);

    plc::TimelineSemaphoreDriver{}
        .setSemaphores({*m_currentTimelineSemaphore})
        .setValues({m_currentSemaphoreValue})
        .wait(m_ptrContext);
  } else {
    m_ptrGraphicCommandDriver.at(frame_index)
        ->submit(plc::SubmitSemaphoreGroup{}
                     .setWaitSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(image_semaphore)
                              .setValue(0u)
                              .setStageMask(
                                  plc::PipelineStage::ColorAttachmentOutput)})
                     .setSignalSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(finished_semaphore)
                              .setValue(0u)
                              .setStageMask(plc::PipelineStage::AllGraphics)}),
                 finished_fence);
  }

  const auto present_result = m_ptrGraphicCommandDriver.at(frame_index)
                                  ->present(m_ptrContext, finished_semaphore);
  if (!present_result.isOk()) {
    return present_result.error();
  }

  ptr_swapchain->updateFrameSyncIndex();

  return plc::ok();
}

void StreamingResourcesHL::spawnNewTriangle(float_t currentTime) {
  if (m_triangleInfos.size() >= MAX_TRIANGLES) {
    return;
  }

  TriangleInfo triangleInfo;
  triangleInfo.spawnTime = currentTime;
  triangleInfo.vertexStartIndex = m_activeTriangles.size();
  m_triangleInfos.push_back(triangleInfo);

  auto radius = 0.15f;
  auto angleStep = 2.0f * std::numbers::pi_v<float_t> / 3.0f;

  glm::vec3 color = {m_colorDist(m_randomGenerator),
                     m_colorDist(m_randomGenerator),
                     m_colorDist(m_randomGenerator)};

  for (int i = 0; i < 3; i += 1) {
    auto angle = m_triangleSpawnAngle + (i * angleStep);
    glm::vec2 pos = {std::cos(angle) * radius, std::sin(angle) * radius};
    Vertex vertex;
    vertex.pos = pos;
    vertex.color = color;
    m_activeTriangles.push_back(vertex);
  }

  m_triangleSpawnAngle += std::numbers::pi_v<float_t> / 6.0f;
  if (m_triangleSpawnAngle >= 2.0f * std::numbers::pi_v<float_t>) {
    m_triangleSpawnAngle -= 2.0f * std::numbers::pi_v<float_t>;
  }
}

void StreamingResourcesHL::removeOldTriangles(float_t currentTime) {
  auto triangleIt = m_triangleInfos.begin();
  while (triangleIt != m_triangleInfos.end()) {
    if ((currentTime - triangleIt->spawnTime) > TRIANGLE_LIFETIME) {
      size_t startIndex = triangleIt->vertexStartIndex;
      m_activeTriangles.erase(m_activeTriangles.begin() + startIndex,
                              m_activeTriangles.begin() + startIndex + 3u);

      for (auto& info : m_triangleInfos) {
        if (info.vertexStartIndex > startIndex) {
          info.vertexStartIndex -= 3u;
        }
      }

      triangleIt = m_triangleInfos.erase(triangleIt);
    } else {
      ++triangleIt;
    }
  }
}

std::vector<StreamingResourcesHL::Vertex>
StreamingResourcesHL::getCurrentTriangles(float_t currentTime) {
  std::vector<Vertex> result;
  result.reserve(m_activeTriangles.size());

  for (size_t i = 0; i < m_triangleInfos.size(); i += 1) {
    const auto& triangleInfo = m_triangleInfos[i];
    auto age = currentTime - triangleInfo.spawnTime;
    auto alpha = 1.0f - (age / TRIANGLE_LIFETIME);
    alpha = std::max(0.0f, std::min(1.0f, alpha));

    for (int j = 0; j < 3; j += 1) {
      size_t vertexIndex = triangleInfo.vertexStartIndex + j;
      if (vertexIndex < m_activeTriangles.size()) {
        Vertex vertex = m_activeTriangles[vertexIndex];
        vertex.color *= alpha;
        result.push_back(vertex);
      }
    }
  }

  return result;
}

}  // namespace samples::highlevel
