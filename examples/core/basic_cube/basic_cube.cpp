#include "basic_cube.hpp"

#include <glm/gtc/matrix_transform.hpp>

samples::core::BasicCube::BasicCube() {
  m_ptrWindow = std::make_unique<plc::ui::Window>("Basic Cube", 800, 600);
  m_ptrContext = std::make_unique<plc::gpu::Context>(m_ptrWindow->getWindowSurface());

  m_ptrWindow->addCallback([&]() {
    if (!m_ptrWindow->isResized()) {
      return;
    }

    m_ptrContext->resetSwapchain();
    constructRenderpass(true);
  });

  for (size_t idx = 0U; idx < m_ptrContext->getPtrSwapchain()->getImageCount(); idx += 1U) {
    m_ptrGraphicCommandDriver.push_back(
        std::make_unique<plc::CommandDriver>(m_ptrContext, plc::QueueFamilyType::Graphics));
  }
  m_ptrTransferCommandDriver.reset(new plc::CommandDriver(m_ptrContext, plc::QueueFamilyType::Transfer));

  m_ptrCubePosition = std::make_unique<CubePosition>();
  m_ptrCubePosition->model = glm::mat4(1.0f);
  m_ptrCubePosition->view =
      glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  m_ptrCubePosition->proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

  m_ptrUniformBuffer = std::make_unique<plc::gpu::Buffer>(plc::createUniformBuffer(m_ptrContext, sizeof(CubePosition)));
  m_ptrCubePositionMapping = m_ptrUniformBuffer->mapMemory(m_ptrContext);

  constructShaderResources();
  constructRenderpass();
  constructGraphicPipeline();
}

samples::core::BasicCube::~BasicCube() {
  m_ptrContext->getPtrDevice()->waitIdle();
}

void samples::core::BasicCube::run() {
  {
    std::vector<plc::gpu::Buffer> staging_buffers;

    setTransferCommands(staging_buffers);

    plc::gpu::TimelineSemaphore semaphore(m_ptrContext);
    m_ptrTransferCommandDriver->submit(plc::PipelineStage::BottomOfPipe, semaphore);
    m_ptrGraphicCommandDriver.at(0U)->submit(plc::PipelineStage::VertexShader, semaphore);

    semaphore.wait(m_ptrContext);
    m_ptrTransferCommandDriver->queueWaitIdle();
    m_ptrGraphicCommandDriver.at(0U)->queueWaitIdle();

    m_ptrTransferCommandDriver->resetAllCommandPools(m_ptrContext);
    m_ptrGraphicCommandDriver.at(0U)->resetAllCommandPools(m_ptrContext);
  }

  while (m_ptrWindow->update()) {
    static float_t accum_count = 0.0f;
    accum_count += 0.016f;

    m_ptrCubePosition->model = glm::rotate(
        m_ptrCubePosition->model, glm::radians(360 * glm::sin(accum_count) / 100.0f), glm::vec3(0.5f, 1.0f, 0.0f));
    std::memcpy(m_ptrCubePositionMapping, reinterpret_cast<void*>(m_ptrCubePosition.get()), sizeof(CubePosition));

    m_ptrContext->acquireNextImage();
    m_ptrRenderKit->updateIndex(m_ptrContext->getPtrSwapchain()->getImageIndex());

    m_ptrGraphicCommandDriver.at(m_ptrContext->getPtrSwapchain()->getFrameSyncIndex())
        ->resetAllCommandPools(m_ptrContext);
    setGraphicCommands();

    plc::gpu::AcquireImageSemaphore acquire_semaphore(m_ptrContext);
    plc::gpu::RenderSemaphore render_semaphore(m_ptrContext);

    m_ptrGraphicCommandDriver.at(m_ptrContext->getPtrSwapchain()->getFrameSyncIndex())
        ->submit(acquire_semaphore, plc::PipelineStage::ColorAttachmentOutput, render_semaphore);
    m_ptrGraphicCommandDriver.at(m_ptrContext->getPtrSwapchain()->getFrameSyncIndex())
        ->present(m_ptrContext, render_semaphore);

    m_ptrContext->getPtrSwapchain()->updateFrameSyncIndex();
  }

  m_ptrCubePositionMapping = nullptr;
  m_ptrUniformBuffer->unmapMemory(m_ptrContext);
}

void samples::core::BasicCube::constructShaderResources() {
  {
    const auto spirv_binary = plc::io::shader::read("examples/core/basic_cube/cube.vert");

    m_shaderModuleMap["vertex"] = plc::gpu::ShaderModule(m_ptrContext, spirv_binary);
  }
  {
    const auto spirv_binary = plc::io::shader::read("examples/core/basic_cube/cube.frag");

    m_shaderModuleMap["fragment"] = plc::gpu::ShaderModule(m_ptrContext, spirv_binary);
  }

  const auto description_unit = plc::gpu::DescriptionUnit(m_shaderModuleMap, {"vertex", "fragment"});

  m_ptrDescriptorSetLayout.reset(new plc::gpu::DescriptorSetLayout(m_ptrContext, description_unit));
  m_ptrDescriptorSet.reset(new plc::gpu::DescriptorSet(m_ptrContext, *m_ptrDescriptorSetLayout));

  const auto buffer_descriptions = std::vector<plc::gpu::BufferDescription>{
      {description_unit.getDescriptorInfoMap().at("CubePosition"), *m_ptrUniformBuffer}};

  m_ptrDescriptorSet->updateDescriptorSet(m_ptrContext, buffer_descriptions, {});

  m_ptrPipeline.reset(
      new plc::Pipeline(m_ptrContext, description_unit, *m_ptrDescriptorSetLayout, plc::PipelineBind::Graphics));
}

void samples::core::BasicCube::constructRenderpass(const bool is_resized) {
  {
    const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();

    const auto image_sub_info = plc::ImageSubInfo{}
                                    .setSize(window_size.width, window_size.height, 1U)
                                    .setMipLevels(1U)
                                    .setArrayLayers(1U)
                                    .setSamples(plc::ImageSampleCount::v1)
                                    .setFormat(plc::DataFormat::DepthSfloatStencilUint)
                                    .setDimension(plc::ImageDimension::v2D);

    m_ptrDepthImage.reset(new plc::gpu::Image(m_ptrContext,
                                              plc::MemoryUsage::GpuOnly,
                                              plc::TransferType::Unknown,
                                              {plc::ImageUsage::DepthStencilAttachment},
                                              image_sub_info));

    const auto image_view_info = plc::ImageViewInfo{}
                                     .setAspect(plc::ImageAspect::DepthStencil)
                                     .setArrayRange(0U, image_sub_info.array_layers)
                                     .setMipRange(0U, image_sub_info.mip_levels);

    m_ptrDepthImageView.reset(new plc::gpu::ImageView(m_ptrContext, *m_ptrDepthImage, image_view_info));
  }

  plc::AttachmentList attachment_list;

  const auto backbuffer_attach_index = [&] {
    const auto attachment_description = plc::AttachmentDescription{}
                                            .setFormat(m_ptrContext->getPtrSwapchain()->getImageFormat())
                                            .setSamples(plc::ImageSampleCount::v1)
                                            .setLoadOp(plc::AttachmentLoadOp::Clear)
                                            .setStoreOp(plc::AttachmentStoreOp::Store)
                                            .setStencilLoadOp(plc::AttachmentLoadOp::DontCare)
                                            .setStencilStoreOp(plc::AttachmentStoreOp::DontCare)
                                            .setLayouts(plc::ImageLayout::Undefined, plc::ImageLayout::PresentSrc);

    return attachment_list.append(attachment_description, plc::ClearColor{}.setColor(0.0f, 0.0f, 0.0f, 1.0f));
  }();

  const auto depth_attach_index = [&] {
    const auto attachment_description =
        plc::AttachmentDescription{}
            .setFormat(plc::DataFormat::DepthSfloatStencilUint)
            .setSamples(plc::ImageSampleCount::v1)
            .setLoadOp(plc::AttachmentLoadOp::Clear)
            .setStoreOp(plc::AttachmentStoreOp::DontCare)
            .setStencilLoadOp(plc::AttachmentLoadOp::DontCare)
            .setStencilStoreOp(plc::AttachmentStoreOp::DontCare)
            .setLayouts(plc::ImageLayout::Undefined, plc::ImageLayout::DepthStencilAttachmentOptimal);

    return attachment_list.append(
        attachment_description, *m_ptrDepthImageView, plc::ClearDepthStencil{}.setValues(1.0f, 0U));
  }();

  plc::SubpassGraph subpass_graph;

  {
    plc::SubpassNode subpass_node(plc::PipelineBind::Graphics, 0U);
    subpass_node.attachColor(plc::AttachmentReference{}
                                 .setIndex(backbuffer_attach_index)
                                 .setLayout(plc::ImageLayout::ColorAttachmentOptimal));
    subpass_node.attachDepthStencil(plc::AttachmentReference{}
                                        .setIndex(depth_attach_index)
                                        .setLayout(plc::ImageLayout::DepthStencilAttachmentOptimal));
    m_supassIndexMap["draw"] = subpass_graph.appendNode(subpass_node);

    const auto subpass_edge = plc::SubpassEdge{}
                                  .setDependencyFlag(plc::DependencyFlag::ByRegion)
                                  .setDstIndex(m_supassIndexMap.at("draw"))
                                  .addSrcStage(plc::PipelineStage::ColorAttachmentOutput)
                                  .addDstStage(plc::PipelineStage::ColorAttachmentOutput)
                                  .addSrcAccess(plc::AccessFlag::Unknown)
                                  .addDstAccess(plc::AccessFlag::ColorAttachmentWrite);
    subpass_graph.appendEdge(subpass_edge);
  }

  if (is_resized) {
    m_ptrRenderKit->resetFramebuffer(
        m_ptrContext, attachment_list, m_ptrWindow->getWindowSurface()->getWindowSize(), true);
  } else {
    m_ptrRenderKit.reset(new plc::RenderKit(
        m_ptrContext, attachment_list, subpass_graph, m_ptrWindow->getWindowSurface()->getWindowSize(), true));
  }
}

void samples::core::BasicCube::constructGraphicPipeline() {
  const auto ptr_graphic_info = std::make_unique<plc::pipeline::GraphicInfo>();

  {
    ptr_graphic_info->vertex_input.appendBinding(0U, sizeof(Vertex), plc::VertexInputRate::Vertex);
    ptr_graphic_info->vertex_input.appendAttribute(0U, 0U, plc::DataFormat::R32G32B32Sfloat, offsetof(Vertex, pos));
    ptr_graphic_info->vertex_input.appendAttribute(1U, 0U, plc::DataFormat::R32G32B32Sfloat, offsetof(Vertex, color));
  }

  {
    ptr_graphic_info->input_assembly.setTopology(plc::PrimitiveTopology::TriangleList);
    ptr_graphic_info->input_assembly.setRestart(false);
  }

  {
    ptr_graphic_info->viewport_state.setScissor({800U, 600U});
    ptr_graphic_info->viewport_state.setViewport({800.0f, 600.0f}, 0.0f, 1.0f);
  }

  {
    ptr_graphic_info->rasterization.setPolygonMode(plc::PolygonMode::Fill);
    ptr_graphic_info->rasterization.setCullMode(plc::CullMode::Back);
    ptr_graphic_info->rasterization.setFrontFace(plc::FrontFace::Clockwise);
    ptr_graphic_info->rasterization.setLineWidth(1.0f);
  }

  {
    plc::ColorBlendAttachment color_blend_attachment;
    color_blend_attachment.color_components = {
        plc::ColorComponent::R, plc::ColorComponent::G, plc::ColorComponent::B, plc::ColorComponent::A};
    ptr_graphic_info->color_blend.setLogicOp(false, plc::LogicOp::Copy);
    ptr_graphic_info->color_blend.appendAttachment(color_blend_attachment);
  }

  {
    ptr_graphic_info->dynamic_state.appendState(plc::DynamicOption::Viewport);
    ptr_graphic_info->dynamic_state.appendState(plc::DynamicOption::Scissor);
  }

  m_ptrPipeline->constructGraphicsPipeline(m_ptrContext,
                                           m_shaderModuleMap,
                                           {"vertex", "fragment"},
                                           ptr_graphic_info,
                                           m_ptrRenderKit->getRenderpass(),
                                           m_supassIndexMap.at("draw"));
}

void samples::core::BasicCube::setTransferCommands(std::vector<plc::gpu::Buffer>& staging_buffers) {
  const auto queue_family_indices = std::make_pair(m_ptrTransferCommandDriver->getQueueFamilyIndex(),
                                                   m_ptrGraphicCommandDriver.at(0)->getQueueFamilyIndex());

  {
    const auto command_buffer = m_ptrTransferCommandDriver->getTransfer();
    command_buffer.begin();

    {
      const std::vector<Vertex> vertices = {
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.5f, 0.0f}},  {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.5f}},
          {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},

          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},   {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

          {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},   {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},  {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},

          {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},   {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},

          {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},   {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},  {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},

          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},  {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      };

      m_ptrVertexBuffer =
          std::make_unique<plc::gpu::Buffer>(plc::createVertexBuffer(m_ptrContext, vertices.size() * sizeof(Vertex)));

      auto staging_buffer = plc::createStagingBufferToGPU(m_ptrContext, vertices.size() * sizeof(Vertex));
      auto mapped_address = staging_buffer.mapMemory(m_ptrContext);
      std::memcpy(mapped_address, vertices.data(), staging_buffer.getSize());
      staging_buffer.unmapMemory(m_ptrContext);

      staging_buffers.push_back(std::move(staging_buffer));

      command_buffer.copyBuffer(staging_buffers.at(0U), *m_ptrVertexBuffer);

      auto buffer_barrier = plc::gpu::BufferBarrier(*m_ptrVertexBuffer,
                                                    {plc::AccessFlag::TransferWrite},
                                                    {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite});
      buffer_barrier.setSrcQueueFamilyIndex(queue_family_indices.first);
      buffer_barrier.setDstQueueFamilyIndex(queue_family_indices.second);

      command_buffer.setPipelineBarrier(buffer_barrier, plc::PipelineStage::Transfer, plc::PipelineStage::BottomOfPipe);
    }

    {
      const std::vector<uint32_t> indices = {
          0U,  1U,  2U,  3U,  2U,  1U,  4U,  5U,  6U,  7U,  6U,  5U,  8U,  9U,  10U, 11U, 10U, 9U,
          12U, 13U, 14U, 15U, 14U, 13U, 16U, 17U, 18U, 19U, 18U, 17U, 20U, 21U, 22U, 23U, 22U, 21U,
      };

      m_ptrIndexBuffer =
          std::make_unique<plc::gpu::Buffer>(plc::createIndexBuffer(m_ptrContext, indices.size() * sizeof(uint32_t)));

      auto staging_buffer = plc::createStagingBufferToGPU(m_ptrContext, indices.size() * sizeof(uint32_t));
      auto mapped_address = staging_buffer.mapMemory(m_ptrContext);
      std::memcpy(mapped_address, indices.data(), staging_buffer.getSize());
      staging_buffer.unmapMemory(m_ptrContext);

      staging_buffers.push_back(std::move(staging_buffer));

      command_buffer.copyBuffer(staging_buffers.at(1U), *m_ptrIndexBuffer);

      auto buffer_barrier = plc::gpu::BufferBarrier(*m_ptrIndexBuffer,
                                                    {plc::AccessFlag::TransferWrite},
                                                    {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite});
      buffer_barrier.setSrcQueueFamilyIndex(queue_family_indices.first);
      buffer_barrier.setDstQueueFamilyIndex(queue_family_indices.second);

      command_buffer.setPipelineBarrier(buffer_barrier, plc::PipelineStage::Transfer, plc::PipelineStage::BottomOfPipe);
    }

    command_buffer.end();
  }

  {
    const auto command_buffer = m_ptrGraphicCommandDriver.at(0)->getGraphic();
    command_buffer.begin();

    {
      auto buffer_barrier = plc::gpu::BufferBarrier(*m_ptrVertexBuffer,
                                                    {plc::AccessFlag::TransferWrite},
                                                    {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite});
      buffer_barrier.setSrcQueueFamilyIndex(queue_family_indices.first);
      buffer_barrier.setDstQueueFamilyIndex(queue_family_indices.second);

      command_buffer.setPipelineBarrier(
          buffer_barrier, plc::PipelineStage::BottomOfPipe, plc::PipelineStage::VertexShader);
    }

    {
      auto buffer_barrier = plc::gpu::BufferBarrier(*m_ptrIndexBuffer,
                                                    {plc::AccessFlag::TransferWrite},
                                                    {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite});
      buffer_barrier.setSrcQueueFamilyIndex(queue_family_indices.first);
      buffer_barrier.setDstQueueFamilyIndex(queue_family_indices.second);

      command_buffer.setPipelineBarrier(
          buffer_barrier, plc::PipelineStage::BottomOfPipe, plc::PipelineStage::VertexShader);
    }

    command_buffer.end();
  }
}

void samples::core::BasicCube::setGraphicCommands() {
  const auto command_buffer =
      m_ptrGraphicCommandDriver.at(m_ptrContext->getPtrSwapchain()->getFrameSyncIndex())->getGraphic();
  command_buffer.begin();

  command_buffer.beginRenderpass(
      *m_ptrRenderKit, m_ptrWindow->getWindowSurface()->getWindowSize(), plc::SubpassContents::Inline);

  command_buffer.bindPipeline(*m_ptrPipeline);

  command_buffer.bindDescriptorSet(*m_ptrPipeline, *m_ptrDescriptorSet);

  const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();
  command_buffer.setViewport(plc::gpu_ui::GraphicalSize<float_t>(static_cast<float_t>(window_size.width),
                                                                 static_cast<float_t>(window_size.height)),
                             0.0f,
                             1.0f);
  command_buffer.setScissor(window_size);

  command_buffer.bindVertexBuffer(*m_ptrVertexBuffer, 0U);
  command_buffer.bindIndexBuffer(*m_ptrIndexBuffer, 0U);

  command_buffer.drawIndexed(36U, 1U, 0U, 0U, 0U);

  command_buffer.endRenderpass();

  command_buffer.end();
}
