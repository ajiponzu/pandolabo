#include "square.hpp"

samples::core::Square::Square() {
  m_ptrWindow = std::make_unique<plc::ui::Window>("Square", 800U, 600U);
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

  constructShaderResources();
  constructRenderpass();
  constructGraphicPipeline();
}

samples::core::Square::~Square() {
  m_ptrContext->getPtrDevice()->waitIdle();
}

void samples::core::Square::run() {
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
}

void samples::core::Square::constructShaderResources() {
  {
    const auto spirv_binary = plc::io::shader::read("examples/core/square/square.vert");

    m_shaderModuleMap["vertex"] = plc::gpu::ShaderModule(m_ptrContext, spirv_binary);
  }
  {
    const auto spirv_binary = plc::io::shader::read("examples/core/square/square.frag");

    m_shaderModuleMap["fragment"] = plc::gpu::ShaderModule(m_ptrContext, spirv_binary);
  }

  const auto description_unit = plc::gpu::DescriptionUnit(m_shaderModuleMap, {"vertex", "fragment"});

  m_ptrDescriptorSetLayout.reset(new plc::gpu::DescriptorSetLayout(m_ptrContext, description_unit));
  m_ptrDescriptorSet.reset(new plc::gpu::DescriptorSet(m_ptrContext, *m_ptrDescriptorSetLayout));

  m_ptrDescriptorSet->updateDescriptorSet(m_ptrContext, {}, {});

  m_ptrPipeline.reset(
      new plc::Pipeline(m_ptrContext, description_unit, *m_ptrDescriptorSetLayout, plc::PipelineBind::Graphics));
}

void samples::core::Square::constructRenderpass(const bool is_resized) {
  plc::AttachmentList attachment_list;

  const auto backbuffer_attach_index = [&] {
    plc::AttachmentDescription attachment_description;
    attachment_description.format = m_ptrContext->getPtrSwapchain()->getImageFormat();
    attachment_description.samples = plc::ImageSampleCount::v1;
    attachment_description.load_op = plc::AttachmentLoadOp::Clear;
    attachment_description.store_op = plc::AttachmentStoreOp::Store;
    attachment_description.stencil_load_op = plc::AttachmentLoadOp::DontCare;
    attachment_description.stencil_store_op = plc::AttachmentStoreOp::DontCare;
    attachment_description.initial_layout = plc::ImageLayout::Undefined;
    attachment_description.final_layout = plc::ImageLayout::PresentSrc;

    return attachment_list.append(attachment_description, plc::ClearColor({0.0F, 0.0F, 0.0F, 1.0F}));
  }();

  plc::SubpassGraph subpass_graph;

  plc::SubpassNode subpass_node(plc::PipelineBind::Graphics, 0U);
  subpass_node.attachColor(plc::AttachmentReference(backbuffer_attach_index, plc::ImageLayout::ColorAttachmentOptimal));
  m_supassIndexMap["draw"] = subpass_graph.appendNode(subpass_node);

  plc::SubpassEdge subpass_edge;
  subpass_edge.dependency_flag = plc::DependencyFlag::ByRegion;
  // First subpass index must be ~0U (not equal 0U).
  // SubpassEdge struct's src_index is initialized ~0U.
  // subpass_edge.src_index = ~0U;
  subpass_edge.dst_index = m_supassIndexMap.at("draw");
  subpass_edge.src_stages.push_back(plc::PipelineStage::ColorAttachmentOutput);
  subpass_edge.dst_stages.push_back(plc::PipelineStage::ColorAttachmentOutput);
  subpass_edge.src_access.push_back(plc::AccessFlag::Unknown);
  subpass_edge.dst_access.push_back(plc::AccessFlag::ColorAttachmentWrite);
  subpass_graph.appendEdge(subpass_edge);

  if (is_resized) {
    m_ptrRenderKit->resetFramebuffer(
        m_ptrContext, attachment_list, m_ptrWindow->getWindowSurface()->getWindowSize(), true);
  } else {
    m_ptrRenderKit.reset(new plc::RenderKit(
        m_ptrContext, attachment_list, subpass_graph, m_ptrWindow->getWindowSurface()->getWindowSize(), true));
  }
}

void samples::core::Square::constructGraphicPipeline() {
  const auto ptr_graphic_info = std::make_unique<plc::pipeline::GraphicInfo>();

  {
    ptr_graphic_info->vertex_input.appendBinding(0U, sizeof(Vertex), plc::VertexInputRate::Vertex);
    ptr_graphic_info->vertex_input.appendAttribute(0U, 0U, plc::DataFormat::R32G32Sfloat, offsetof(Vertex, pos));
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

void samples::core::Square::setTransferCommands(std::vector<plc::gpu::Buffer>& staging_buffers) {
  const auto queue_family_indices = std::make_pair(m_ptrTransferCommandDriver->getQueueFamilyIndex(),
                                                   m_ptrGraphicCommandDriver.at(0)->getQueueFamilyIndex());

  {
    const auto command_buffer = m_ptrTransferCommandDriver->getTransfer();
    command_buffer.begin();

    {
      const std::vector<Vertex> vertices = {
          {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
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
      const std::vector<uint32_t> indices = {0U, 1U, 2U, 2U, 3U, 0U};

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

void samples::core::Square::setGraphicCommands() {
  const auto command_buffer =
      m_ptrGraphicCommandDriver.at(m_ptrContext->getPtrSwapchain()->getFrameSyncIndex())->getGraphic();
  command_buffer.begin();

  command_buffer.beginRenderpass(
      *m_ptrRenderKit, m_ptrWindow->getWindowSurface()->getWindowSize(), plc::SubpassContents::Inline);

  command_buffer.bindPipeline(*m_ptrPipeline);

  command_buffer.bindDescriptorSet(*m_ptrPipeline, *m_ptrDescriptorSet);

  static float_t push_timer = 0.0f;
  push_timer += 0.016f;
  command_buffer.pushConstants(*m_ptrPipeline, {plc::ShaderStage::Fragment}, 0U, {push_timer});

  const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();
  command_buffer.setViewport(plc::gpu_ui::GraphicalSize<float_t>(static_cast<float_t>(window_size.width),
                                                                 static_cast<float_t>(window_size.height)),
                             0.0f,
                             1.0f);
  command_buffer.setScissor(window_size);

  command_buffer.bindVertexBuffer(*m_ptrVertexBuffer, 0U);
  command_buffer.bindIndexBuffer(*m_ptrIndexBuffer, 0U);

  command_buffer.drawIndexed(6U, 1U, 0U, 0, 0U);

  command_buffer.endRenderpass();

  command_buffer.end();
}
