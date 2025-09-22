#include "basic_cube.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace samples::core {

BasicCube::BasicCube() {
  m_ptrWindow = std::make_unique<plc::ui::Window>("Basic Cube", 800, 600);
  m_ptrContext =
      std::make_unique<plc::gpu::Context>(m_ptrWindow->getWindowSurface());

  m_ptrWindow->addCallback([&]() {
    if (!m_ptrWindow->isResized()) {
      return;
    }

    m_ptrContext->resetSwapchain();
    constructRenderpass(true);
  });

  for (size_t idx = 0u; idx < m_ptrContext->getPtrSwapchain()->getImageCount();
       idx += 1u) {
    m_ptrGraphicCommandDriver.push_back(std::make_unique<plc::CommandDriver>(
        m_ptrContext, plc::QueueFamilyType::Graphics));
  }
  m_ptrTransferCommandDriver.reset(
      new plc::CommandDriver(m_ptrContext, plc::QueueFamilyType::Transfer));

  m_ptrCubePosition = std::make_unique<CubePosition>();
  m_ptrCubePosition->model = glm::mat4(1.0f);
  m_ptrCubePosition->view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f),
                                        glm::vec3(0.0f, 0.0f, 0.0f),
                                        glm::vec3(0.0f, 1.0f, 0.0f));
  m_ptrCubePosition->proj =
      glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

  m_ptrUniformBuffer = std::make_unique<plc::gpu::Buffer>(
      plc::createUniformBuffer(m_ptrContext, sizeof(CubePosition)));
  m_ptrCubePositionMapping = m_ptrUniformBuffer->mapMemory(m_ptrContext);

  constructShaderResources();
  constructRenderpass();
  constructGraphicPipeline();
}

BasicCube::~BasicCube() {
  m_ptrContext->getPtrDevice()->waitIdle();
}

void BasicCube::run() {
  {
    std::vector<plc::gpu::Buffer> staging_buffers;

    setTransferCommands(staging_buffers);

    plc::gpu::TimelineSemaphore semaphore(m_ptrContext);
    m_ptrTransferCommandDriver->submit(
        {plc::PipelineStage::BottomOfPipe},
        plc::SubmitSemaphoreGroup{}
            .setWaitSemaphores(semaphore.forWait(0u))
            .setSignalSemaphores(semaphore.forSignal(1u)));
    m_ptrGraphicCommandDriver.at(0u)->submit(
        {plc::PipelineStage::VertexShader},
        plc::SubmitSemaphoreGroup{}
            .setWaitSemaphores(semaphore.forWait(1u))
            .setSignalSemaphores(semaphore.forSignal(2u)));

    plc::TimelineSemaphoreDriver{}
        .setSemaphores({semaphore})
        .setValues({2u})
        .wait(m_ptrContext);

    m_ptrTransferCommandDriver->queueWaitIdle();
    m_ptrGraphicCommandDriver.at(0u)->queueWaitIdle();

    m_ptrTransferCommandDriver->resetAllCommandPools(m_ptrContext);
    m_ptrGraphicCommandDriver.at(0u)->resetAllCommandPools(m_ptrContext);
  }

  while (m_ptrWindow->update()) {
    static float_t accum_count = 0.0f;
    accum_count += 0.016f;

    m_ptrCubePosition->model =
        glm::rotate(m_ptrCubePosition->model,
                    glm::radians(360 * glm::sin(accum_count) / 100.0f),
                    glm::vec3(0.5f, 1.0f, 0.0f));
    std::memcpy(m_ptrCubePositionMapping,
                reinterpret_cast<void*>(m_ptrCubePosition.get()),
                sizeof(CubePosition));

    const auto& ptr_swapchain = m_ptrContext->getPtrSwapchain();
    ptr_swapchain->updateImageIndex(m_ptrContext->getPtrDevice());
    m_ptrRenderKit->updateIndex(ptr_swapchain->getImageIndex());

    m_ptrGraphicCommandDriver.at(ptr_swapchain->getFrameSyncIndex())
        ->resetAllCommandPools(m_ptrContext);
    setGraphicCommands();

    const auto image_semaphore = ptr_swapchain->getImageAvailableSemaphore();
    const auto finished_semaphore = ptr_swapchain->getFinishedSemaphore();
    const auto finished_fence = ptr_swapchain->getFence();

    m_ptrGraphicCommandDriver.at(ptr_swapchain->getFrameSyncIndex())
        ->submit({plc::PipelineStage::ColorAttachmentOutput},
                 plc::SubmitSemaphoreGroup{}
                     .setWaitSemaphores(image_semaphore)
                     .setSignalSemaphores(finished_semaphore),
                 finished_fence);
    m_ptrGraphicCommandDriver.at(ptr_swapchain->getFrameSyncIndex())
        ->present(m_ptrContext, finished_semaphore);

    ptr_swapchain->updateFrameSyncIndex();
  }

  m_ptrCubePositionMapping = nullptr;
  m_ptrUniformBuffer->unmapMemory(m_ptrContext);
}

void BasicCube::constructShaderResources() {
  {
    const auto spirv_binary =
        plc::io::shader::read("examples/core/basic_cube/cube.vert");

    m_shaderModuleMap["vertex"] =
        plc::gpu::ShaderModule(m_ptrContext, spirv_binary);
  }
  {
    const auto spirv_binary =
        plc::io::shader::read("examples/core/basic_cube/cube.frag");

    m_shaderModuleMap["fragment"] =
        plc::gpu::ShaderModule(m_ptrContext, spirv_binary);
  }

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"vertex", "fragment"});

  m_ptrDescriptorSetLayout.reset(
      new plc::gpu::DescriptorSetLayout(m_ptrContext, description_unit));
  m_ptrDescriptorSet.reset(
      new plc::gpu::DescriptorSet(m_ptrContext, *m_ptrDescriptorSetLayout));

  const auto buffer_descriptions = std::vector<plc::gpu::BufferDescription>{
      {description_unit.getDescriptorInfoMap().at("CubePosition"),
       *m_ptrUniformBuffer}};

  m_ptrDescriptorSet->updateDescriptorSet(
      m_ptrContext, buffer_descriptions, {});

  m_ptrPipeline.reset(new plc::Pipeline(m_ptrContext,
                                        description_unit,
                                        *m_ptrDescriptorSetLayout,
                                        plc::PipelineBind::Graphics));
}

void BasicCube::constructRenderpass(const bool is_resized) {
  {
    const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();

    const auto image_sub_info =
        plc::ImageSubInfo{}
            .setSize(window_size.width, window_size.height, 1u)
            .setMipLevels(1u)
            .setArrayLayers(1u)
            .setSamples(plc::ImageSampleCount::v1)
            .setFormat(plc::DataFormat::DepthSfloatStencilUint)
            .setDimension(plc::ImageDimension::v2D);

    m_ptrDepthImage.reset(
        new plc::gpu::Image(m_ptrContext,
                            plc::MemoryUsage::GpuOnly,
                            plc::TransferType::Unknown,
                            {plc::ImageUsage::DepthStencilAttachment},
                            image_sub_info));

    const auto image_view_info =
        plc::ImageViewInfo{}
            .setAspect(plc::ImageAspect::DepthStencil)
            .setArrayRange(0u, image_sub_info.array_layers)
            .setMipRange(0u, image_sub_info.mip_levels);

    m_ptrDepthImageView.reset(new plc::gpu::ImageView(
        m_ptrContext, *m_ptrDepthImage, image_view_info));
  }

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
        plc::ClearColor{}.setColor(0.0f, 0.0f, 0.0f, 1.0f));
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
            .setLayouts(plc::ImageLayout::Undefined,
                        plc::ImageLayout::DepthStencilAttachmentOptimal);

    return attachment_list.append(attachment_description,
                                  *m_ptrDepthImageView,
                                  plc::ClearDepthStencil{}.setValues(1.0f, 0u));
  }();

  plc::SubpassGraph subpass_graph;

  {
    plc::SubpassNode subpass_node(plc::PipelineBind::Graphics, 0u);
    subpass_node.attachColor(
        plc::AttachmentReference{}
            .setIndex(backbuffer_attach_index)
            .setLayout(plc::ImageLayout::ColorAttachmentOptimal));
    subpass_node.attachDepthStencil(
        plc::AttachmentReference{}
            .setIndex(depth_attach_index)
            .setLayout(plc::ImageLayout::DepthStencilAttachmentOptimal));
    m_supassIndexMap["draw"] = subpass_graph.appendNode(subpass_node);

    const auto subpass_edge =
        plc::SubpassEdge{}
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
        m_ptrContext,
        attachment_list,
        m_ptrWindow->getWindowSurface()->getWindowSize(),
        true);
  } else {
    m_ptrRenderKit.reset(
        new plc::RenderKit(m_ptrContext,
                           attachment_list,
                           subpass_graph,
                           m_ptrWindow->getWindowSurface()->getWindowSize(),
                           true));
  }
}

void BasicCube::constructGraphicPipeline() {
  // Create GraphicInfo using builder pattern
  const auto ptr_graphic_info =
      plc::pipeline::GraphicInfoBuilder::create()
          .setVertexInput(
              plc::pipeline::VertexInput{}
                  .addBinding(0u, sizeof(Vertex), plc::VertexInputRate::Vertex)
                  .addAttribute(0u,
                                0u,
                                plc::DataFormat::R32G32B32Sfloat,
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
                                           m_supassIndexMap.at("draw"));
}

void BasicCube::setTransferCommands(
    std::vector<plc::gpu::Buffer>& staging_buffers) {
  const auto queue_family_indices =
      std::make_pair(m_ptrTransferCommandDriver->getQueueFamilyIndex(),
                     m_ptrGraphicCommandDriver.at(0)->getQueueFamilyIndex());

  {
    const auto command_buffer = m_ptrTransferCommandDriver->getTransfer();
    command_buffer.begin();

    {
      const std::vector<Vertex> vertices = {
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.5f, 0.0f}},
          {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.5f}},
          {{-0.5f, -0.5f, -0.5f}, {0.5f, 0.0f, 0.0f}},
          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},

          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

          {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
          {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},

          {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
          {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
          {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},

          {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},

          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      };

      m_ptrVertexBuffer =
          std::make_unique<plc::gpu::Buffer>(plc::createVertexBuffer(
              m_ptrContext, vertices.size() * sizeof(Vertex)));

      auto staging_buffer = plc::createStagingBufferToGPU(
          m_ptrContext, vertices.size() * sizeof(Vertex));
      auto mapped_address = staging_buffer.mapMemory(m_ptrContext);
      std::memcpy(mapped_address, vertices.data(), staging_buffer.getSize());
      staging_buffer.unmapMemory(m_ptrContext);

      staging_buffers.push_back(std::move(staging_buffer));

      command_buffer.copyBuffer(staging_buffers.at(0u), *m_ptrVertexBuffer);

      const auto buffer_barrier =
          plc::gpu::BufferBarrierBuilder::create()
              .setBuffer(*m_ptrVertexBuffer)
              .setPriorityAccessFlags({plc::AccessFlag::TransferWrite})
              .setWaitAccessFlags(
                  {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
              .setSrcQueueFamilyIndex(queue_family_indices.first)
              .setDstQueueFamilyIndex(queue_family_indices.second)
              .build();

      command_buffer.setPipelineBarrier(buffer_barrier,
                                        plc::PipelineStage::Transfer,
                                        plc::PipelineStage::BottomOfPipe);
    }

    {
      const std::vector<uint32_t> indices = {
          0u,  1u,  2u,  3u,  2u,  1u,  4u,  5u,  6u,  7u,  6u,  5u,
          8u,  9u,  10u, 11u, 10u, 9u,  12u, 13u, 14u, 15u, 14u, 13u,
          16u, 17u, 18u, 19u, 18u, 17u, 20u, 21u, 22u, 23u, 22u, 21u,
      };

      m_ptrIndexBuffer =
          std::make_unique<plc::gpu::Buffer>(plc::createIndexBuffer(
              m_ptrContext, indices.size() * sizeof(uint32_t)));

      auto staging_buffer = plc::createStagingBufferToGPU(
          m_ptrContext, indices.size() * sizeof(uint32_t));
      auto mapped_address = staging_buffer.mapMemory(m_ptrContext);
      std::memcpy(mapped_address, indices.data(), staging_buffer.getSize());
      staging_buffer.unmapMemory(m_ptrContext);

      staging_buffers.push_back(std::move(staging_buffer));

      command_buffer.copyBuffer(staging_buffers.at(1u), *m_ptrIndexBuffer);

      const auto buffer_barrier =
          plc::gpu::BufferBarrierBuilder::create()
              .setBuffer(*m_ptrIndexBuffer)
              .setPriorityAccessFlags({plc::AccessFlag::TransferWrite})
              .setWaitAccessFlags(
                  {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
              .setSrcQueueFamilyIndex(queue_family_indices.first)
              .setDstQueueFamilyIndex(queue_family_indices.second)
              .build();

      command_buffer.setPipelineBarrier(buffer_barrier,
                                        plc::PipelineStage::Transfer,
                                        plc::PipelineStage::BottomOfPipe);
    }

    command_buffer.end();
  }

  {
    const auto command_buffer = m_ptrGraphicCommandDriver.at(0)->getGraphic();
    command_buffer.begin();

    {
      const auto buffer_barrier =
          plc::gpu::BufferBarrierBuilder::create()
              .setBuffer(*m_ptrVertexBuffer)
              .setPriorityAccessFlags({plc::AccessFlag::TransferWrite})
              .setWaitAccessFlags(
                  {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
              .setSrcQueueFamilyIndex(queue_family_indices.first)
              .setDstQueueFamilyIndex(queue_family_indices.second)
              .build();

      command_buffer.setPipelineBarrier(buffer_barrier,
                                        plc::PipelineStage::BottomOfPipe,
                                        plc::PipelineStage::VertexShader);
    }

    {
      const auto buffer_barrier =
          plc::gpu::BufferBarrierBuilder::create()
              .setBuffer(*m_ptrIndexBuffer)
              .setPriorityAccessFlags({plc::AccessFlag::TransferWrite})
              .setWaitAccessFlags(
                  {plc::AccessFlag::ShaderRead, plc::AccessFlag::ShaderWrite})
              .setSrcQueueFamilyIndex(queue_family_indices.first)
              .setDstQueueFamilyIndex(queue_family_indices.second)
              .build();

      command_buffer.setPipelineBarrier(buffer_barrier,
                                        plc::PipelineStage::BottomOfPipe,
                                        plc::PipelineStage::VertexShader);
    }

    command_buffer.end();
  }
}

void BasicCube::setGraphicCommands() {
  const auto command_buffer =
      m_ptrGraphicCommandDriver
          .at(m_ptrContext->getPtrSwapchain()->getFrameSyncIndex())
          ->getGraphic();
  command_buffer.begin();

  command_buffer.beginRenderpass(
      *m_ptrRenderKit,
      m_ptrWindow->getWindowSurface()->getWindowSize(),
      plc::SubpassContents::Inline);

  command_buffer.bindPipeline(*m_ptrPipeline);

  command_buffer.bindDescriptorSet(*m_ptrPipeline, *m_ptrDescriptorSet);

  const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();
  command_buffer.setViewport(plc::gpu_ui::GraphicalSize<float_t>(
                                 static_cast<float_t>(window_size.width),
                                 static_cast<float_t>(window_size.height)),
                             0.0f,
                             1.0f);
  command_buffer.setScissor(window_size);

  command_buffer.bindVertexBuffer(*m_ptrVertexBuffer, 0u);
  command_buffer.bindIndexBuffer(*m_ptrIndexBuffer, 0u);

  command_buffer.drawIndexed(36U, 1u, 0u, 0u, 0u);

  command_buffer.endRenderpass();

  command_buffer.end();
}

}  // namespace samples::core
