#include "streaming_resources.hpp"

#include <algorithm>
#include <cstdio>
#include <numbers>
#include <print>
#include <thread>
#include <vector>

namespace samples::core {

StreamingResources::StreamingResources()
    : m_startTime(std::chrono::high_resolution_clock::now()),
      m_randomGenerator(std::random_device{}()),
      m_colorDist(0.2f, 1.0f),
      m_currentSemaphoreValue(0u),
      m_triangleSpawnAngle(0.0f),
      m_lastSpawnTime(0.0f) {
  std::println("Initializing StreamingResources...");

  try {
    std::println("Creating Window...");
    auto window_result =
        plc::ui::Window::create("Streaming Resources", 800u, 600u);
    if (!window_result.isOk()) {
      std::println(stderr,
                   "StreamingResources window error: {}",
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
        *m_ptrContext, plc::QueueFamilyType::Graphics));
    m_ptrTransferCommandDriver.push_back(std::make_unique<plc::CommandDriver>(
        *m_ptrContext, plc::QueueFamilyType::Transfer));
  }
  std::println("Command Drivers created successfully.");

  // Initialize Timeline Semaphore management variables
  m_currentSemaphoreValue = 0u;
  // Timeline Semaphore will be created on first use

  // Create frame-specific vertex buffers and staging buffers (increased size
  // for multiple triangles)
  std::println("Creating frame-specific buffers...");
  const size_t frame_count = m_ptrContext->getPtrSwapchain()->getImageCount();
  const size_t buffer_size =
      sizeof(Vertex) * MAX_TRIANGLES * 3u;  // 3 vertices per triangle
  for (size_t idx = 0u; idx < frame_count; idx += 1u) {
    m_ptrVertexBuffers.push_back(
        plc::createUniqueVertexBuffer(*m_ptrContext, buffer_size));
    m_stagingBuffers.push_back(
        plc::createStagingBufferToGPU(*m_ptrContext, buffer_size));
  }
  std::println("Buffers created successfully.");

  std::println("Constructing shader resources...");
  const auto shader_result = constructShaderResources();
  if (!shader_result.isOk()) {
    std::println(stderr,
                 "StreamingResources shader load error: {}",
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

  // Generate the first triangle immediately
  spawnNewTriangle(0.0f);
  std::println("Generated initial triangle, total: {}", m_triangleInfos.size());
  std::println("StreamingResources initialization complete!");

  m_isInitialized = true;
}

StreamingResources::~StreamingResources() {
  m_ptrContext->getPtrDevice()->waitIdle();
}

void StreamingResources::run() {
  if (!m_isInitialized) {
    std::println(stderr, "StreamingResources is not initialized.");
    return;
  }
  std::println("🚀 Streaming Resources Example 開始");
  std::println("動的三角形生成とストリーミングバッファリング");
  std::println("回転方向に新しい三角形を出力し、古い三角形を消去");

  while (m_ptrWindow->update()) {
    // const auto frame_index =
    //     m_ptrContext->getPtrSwapchain()->getFrameSyncIndex();

    // Reset command pools AFTER rendering completion (simultaneousUse=false)
    // m_ptrGraphicCommandDriver.at(frame_index)
    //     ->resetAllCommandPools(m_ptrContext);

    // Also reset transfer command pool for the next frame to prevent reuse
    // issues
    // m_ptrTransferCommandDriver.at(frame_index)
    //     ->resetAllCommandPools(m_ptrContext);

    const auto update_result = updateVertexData();
    if (!update_result.isOk()) {
      std::println(stderr,
                   "StreamingResources update error: {}",
                   update_result.error().toString());
      return;
    }

    const auto graphic_result = setGraphicCommands();
    if (!graphic_result.isOk()) {
      std::println(stderr,
                   "StreamingResources render error: {}",
                   graphic_result.error().toString());
      return;
    }
  }

  std::println("✅ Streaming Resources Example 完了");
}

plc::VoidResult StreamingResources::constructShaderResources() {
  {
    PANDORA_TRY_ASSIGN(spirv_binary,
                       plc::io::shader::read(
                           "examples/core/streaming_resources/streaming.vert"));

    m_shaderModuleMap["vertex"] =
        plc::gpu::ShaderModule(*m_ptrContext, spirv_binary);
  }
  {
    PANDORA_TRY_ASSIGN(spirv_binary,
                       plc::io::shader::read(
                           "examples/core/streaming_resources/streaming.frag"));

    m_shaderModuleMap["fragment"] =
        plc::gpu::ShaderModule(*m_ptrContext, spirv_binary);
  }

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"vertex", "fragment"});

  m_ptrDescriptorSetLayout.reset(
      new plc::gpu::DescriptorSetLayout(*m_ptrContext, description_unit));
  m_ptrDescriptorSet.reset(
      new plc::gpu::DescriptorSet(*m_ptrContext, *m_ptrDescriptorSetLayout));

  m_ptrPipeline.reset(new plc::Pipeline(*m_ptrContext,
                                        description_unit,
                                        *m_ptrDescriptorSetLayout,
                                        plc::PipelineBind::Graphics));

  return plc::ok();
}

void StreamingResources::constructRenderpass(const bool is_resized) {
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
  m_supassIndexMap["main"] = subpass_graph.appendNode(subpass_node);

  subpass_graph.appendEdge(
      plc::SubpassEdge{}
          .setDstIndex(m_supassIndexMap.at("main"))
          .setSrcAccess({plc::AccessFlag::Unknown})
          .setDstAccess({plc::AccessFlag::ColorAttachmentWrite})
          .setSrcStages({plc::PipelineStage::ColorAttachmentOutput})
          .setDstStages({plc::PipelineStage::ColorAttachmentOutput})
          .setDependencyFlag(plc::DependencyFlag::ByRegion));

  if (is_resized) {
    m_ptrRenderKit->resetFramebuffer(
        *m_ptrContext,
        attachment_list,
        m_ptrWindow->getWindowSurface()->getWindowSize(),
        true);
  } else {
    m_ptrRenderKit.reset(
        new plc::RenderKit(*m_ptrContext,
                           attachment_list,
                           subpass_graph,
                           m_ptrWindow->getWindowSurface()->getWindowSize(),
                           true));
  }
}

void StreamingResources::constructGraphicPipeline() {
  // Create GraphicInfo using builder pattern
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

  m_ptrPipeline->constructGraphicsPipeline(*m_ptrContext,
                                           m_shaderModuleMap,
                                           {"vertex", "fragment"},
                                           *ptr_graphic_info,
                                           m_ptrRenderKit->getRenderpass(),
                                           m_supassIndexMap.at("main"));
}

plc::VoidResult StreamingResources::updateVertexData() {
  auto current_time = std::chrono::high_resolution_clock::now();
  auto time =
      std::chrono::duration<float_t>(current_time - m_startTime).count();

  // Spawn new triangles periodically
  if (time - m_lastSpawnTime > SPAWN_INTERVAL) {
    spawnNewTriangle(time);
    m_lastSpawnTime = time;
    std::println("Spawned triangle, total: {}", m_triangleInfos.size());
  }

  // Remove old triangles
  removeOldTriangles(time);

  // Get current active triangles
  auto vertices = getCurrentTriangles(time);
  std::println("Current vertices: {} ({} triangles)",
               vertices.size(),
               vertices.size() / 3u);

  // Skip if no triangles to render
  if (vertices.empty()) {
    return plc::ok();
  }

  // Get frame-specific resources to avoid contention
  const auto& ptr_swapchain = m_ptrContext->getPtrSwapchain();
  const uint32_t frame_index = ptr_swapchain->getFrameSyncIndex();

  // Use frame-specific staging buffer, vertex buffer, and transfer command
  // driver
  auto& staging_buffer = m_stagingBuffers[frame_index];
  auto& vertex_buffer = m_ptrVertexBuffers[frame_index];
  auto& transfer_driver = m_ptrTransferCommandDriver[frame_index];

  // Copy vertex data to staging buffer
  size_t vertex_data_size = vertices.size() * sizeof(Vertex);
  auto mapped_address = staging_buffer.mapMemory(*m_ptrContext);
  std::memcpy(mapped_address, vertices.data(), vertex_data_size);
  staging_buffer.unmapMemory(*m_ptrContext);

  // Create Timeline Semaphore if needed, or reuse existing one
  if (!m_currentTimelineSemaphore) {
    m_currentTimelineSemaphore =
        std::make_unique<plc::gpu::TimelineSemaphore>(*m_ptrContext);
    m_currentSemaphoreValue = 0u;  // Initial state
  }

  // Check if timeline value is getting too large and needs reset
  constexpr uint64_t TIMELINE_RESET_THRESHOLD = 1000000u;  // 1 million
  if (m_currentSemaphoreValue > TIMELINE_RESET_THRESHOLD) {
    std::println(
        "Timeline Semaphore value ({}) exceeded threshold, recreating...",
        m_currentSemaphoreValue);
    m_currentTimelineSemaphore =
        std::make_unique<plc::gpu::TimelineSemaphore>(*m_ptrContext);
    m_currentSemaphoreValue = 0u;  // Reset to initial state
  }

  // Increment timeline value for this frame
  m_currentSemaphoreValue += 1u;

  // Use transfer command to copy from staging to vertex buffer
  const auto command_buffer = transfer_driver->getTransfer();
  command_buffer.begin();
  command_buffer.copyBuffer(staging_buffer, *vertex_buffer);

  // Add buffer barrier to ensure transfer is complete before graphics reads
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

  // Submit transfer command with Timeline Semaphore synchronization
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

  // Do NOT wait here - let Graphics queue wait for the Timeline Semaphore
  // The Graphics operation in setGraphicCommands() will wait for this signal

  return plc::ok();
}

void StreamingResources::spawnNewTriangle(float_t currentTime) {
  if (m_triangleInfos.size() >= MAX_TRIANGLES) {
    return;  // Maximum triangles reached
  }

  // Record triangle info
  TriangleInfo triangleInfo;
  triangleInfo.spawnTime = currentTime;
  triangleInfo.vertexStartIndex = m_activeTriangles.size();
  m_triangleInfos.push_back(triangleInfo);

  // Create a new triangle at the current spawn angle
  auto radius = 0.15f;
  auto angleStep =
      2.0f * std::numbers::pi_v<float_t> / 3.0f;  // 120 degrees = 2π/3

  glm::vec3 color = {m_colorDist(m_randomGenerator),
                     m_colorDist(m_randomGenerator),
                     m_colorDist(m_randomGenerator)};

  // Generate 3 vertices for the triangle
  for (int i = 0; i < 3; i += 1) {
    auto angle = m_triangleSpawnAngle + (i * angleStep);
    glm::vec2 pos = {std::cos(angle) * radius, std::sin(angle) * radius};
    Vertex vertex;
    vertex.pos = pos;
    vertex.color = color;
    m_activeTriangles.push_back(vertex);
  }

  // Update spawn angle for next triangle (rotate by 30 degrees)
  m_triangleSpawnAngle +=
      std::numbers::pi_v<float_t> / 6.0f;  // 30 degrees = π/6
  if (m_triangleSpawnAngle >= 2.0f * std::numbers::pi_v<float_t>) {
    m_triangleSpawnAngle -= 2.0f * std::numbers::pi_v<float_t>;
  }
}

void StreamingResources::removeOldTriangles(float_t currentTime) {
  // Remove old triangles and their corresponding vertices
  auto triangleIt = m_triangleInfos.begin();
  while (triangleIt != m_triangleInfos.end()) {
    if ((currentTime - triangleIt->spawnTime) > TRIANGLE_LIFETIME) {
      // Remove the 3 vertices for this triangle
      size_t startIndex = triangleIt->vertexStartIndex;
      m_activeTriangles.erase(m_activeTriangles.begin() + startIndex,
                              m_activeTriangles.begin() + startIndex + 3u);

      // Update indices for remaining triangles
      for (auto& info : m_triangleInfos) {
        if (info.vertexStartIndex > startIndex) {
          info.vertexStartIndex -= 3u;
        }
      }

      // Remove triangle info
      triangleIt = m_triangleInfos.erase(triangleIt);
    } else {
      ++triangleIt;
    }
  }
}

std::vector<StreamingResources::Vertex> StreamingResources::getCurrentTriangles(
    float_t currentTime) {
  // Apply fading effect based on triangle age
  std::vector<Vertex> result;
  result.reserve(m_activeTriangles.size());

  for (size_t i = 0; i < m_triangleInfos.size(); i += 1) {
    const auto& triangleInfo = m_triangleInfos[i];
    auto age = currentTime - triangleInfo.spawnTime;
    auto alpha = 1.0f - (age / TRIANGLE_LIFETIME);
    alpha = std::max(0.0f, std::min(1.0f, alpha));

    // Add the 3 vertices for this triangle with fading
    for (int j = 0; j < 3; j += 1) {
      size_t vertexIndex = triangleInfo.vertexStartIndex + j;
      if (vertexIndex < m_activeTriangles.size()) {
        Vertex vertex = m_activeTriangles[vertexIndex];
        vertex.color *= alpha;  // Fade by modifying color intensity
        result.push_back(vertex);
      }
    }
  }

  return result;
}

plc::VoidResult StreamingResources::setGraphicCommands() {
  const auto& ptr_swapchain = m_ptrContext->getPtrSwapchain();
  const auto update_result =
      ptr_swapchain->updateImageIndex(*m_ptrContext->getPtrDevice());
  if (!update_result.isOk()) {
    return update_result.error();
  }
  m_ptrRenderKit->updateIndex(ptr_swapchain->getImageIndex());

  const uint32_t frame_index = ptr_swapchain->getFrameSyncIndex();
  auto& frame_vertex_buffer = m_ptrVertexBuffers[frame_index];

  // Get current triangles (same logic as updateVertexData)
  auto current_time = std::chrono::high_resolution_clock::now();
  auto time =
      std::chrono::duration<float_t>(current_time - m_startTime).count();
  auto vertices = getCurrentTriangles(time);

  size_t vertex_count = vertices.size();

  // DON'T reset command pools here - move to after rendering completion
  // m_ptrGraphicCommandDriver.at(frame_index)->resetAllCommandPools(m_ptrContext);

  const auto command_buffer =
      m_ptrGraphicCommandDriver.at(frame_index)->getGraphic();

  command_buffer.begin();

  // Add buffer barrier to acquire ownership from transfer queue
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

  // Include Timeline Semaphore to wait for transfer completion if available
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
                              // Wait at CAO to cover beginRenderPass transition
                              .setStageMask(
                                  plc::PipelineStage::ColorAttachmentOutput)})
                     .setSignalSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(finished_semaphore)
                              .setStageMask(plc::PipelineStage::AllGraphics)}),
                 finished_fence);

    // Wait for Timeline Semaphore completion to ensure proper synchronization
    plc::TimelineSemaphoreDriver{}
        .setSemaphores({*m_currentTimelineSemaphore})
        .setValues({m_currentSemaphoreValue})
        .wait(*m_ptrContext);
  } else {
    // No Timeline Semaphore available, just use image semaphore
    m_ptrGraphicCommandDriver.at(frame_index)
        ->submit(plc::SubmitSemaphoreGroup{}
                     .setWaitSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(image_semaphore)
                              .setValue(0u)
                              // Wait at CAO to cover beginRenderPass transition
                              .setStageMask(
                                  plc::PipelineStage::ColorAttachmentOutput)})
                     .setSignalSemaphores(
                         {plc::SubmitSemaphore{}
                              .setSemaphore(finished_semaphore)
                              .setValue(0u)
                              .setStageMask(plc::PipelineStage::AllGraphics)}),
                 finished_fence);
  }

  // Present after all rendering operations are complete
  const auto present_result = m_ptrGraphicCommandDriver.at(frame_index)
                                  ->present(*m_ptrContext, finished_semaphore);
  if (!present_result.isOk()) {
    return present_result.error();
  }

  ptr_swapchain->updateFrameSyncIndex();

  return plc::ok();
}

}  // namespace samples::core
