#include "square_hl.hpp"

#include <cstdio>
#include <cstring>
#include <print>
#include <span>

namespace samples::highlevel {

SquareHL::SquareHL() {
  auto window_result = plc::ui::Window::create("Square (HL)", 800u, 600u);
  if (!window_result.isOk()) {
    std::println(
        stderr, "SquareHL window error: {}", window_result.error().toString());
    return;
  }
  m_ptrWindow = std::move(window_result).takeValue();
  m_ptrContext =
      std::make_unique<plc::gpu::Context>(m_ptrWindow->getWindowSurface());

  m_ptrWindow->addCallback([&]() {
    if (!m_ptrWindow->isResized()) {
      return;
    }

    m_ptrContext->resetSwapchain();
    constructRenderpass(true);
  });

  m_ptrRenderer = std::make_unique<plh::Renderer>(*m_ptrWindow, *m_ptrContext);
  m_ptrTransfer = std::make_unique<plh::ResourceTransfer>(
      *m_ptrContext, plc::QueueFamilyType::Graphics);

  const auto shader_result = constructShaderResources();
  if (!shader_result.isOk()) {
    std::println(stderr,
                 "SquareHL shader load error: {}",
                 shader_result.error().toString());
    return;
  }

  constructRenderpass();
  constructGraphicPipeline();

  const auto upload_result = uploadGeometry();
  if (!upload_result.isOk()) {
    std::println(
        stderr, "SquareHL upload error: {}", upload_result.error().toString());
    return;
  }

  m_ptrRenderer->setRenderKit(*m_ptrRenderKit);
  m_isInitialized = true;
}

SquareHL::~SquareHL() {
  if (m_ptrContext) {
    m_ptrContext->getPtrDevice()->waitIdle();
  }
}

void SquareHL::run() {
  if (!m_isInitialized) {
    std::println(stderr, "SquareHL is not initialized.");
    return;
  }

  while (m_ptrWindow->update()) {
    auto frame_result = m_ptrRenderer->beginFrame();
    if (!frame_result.isOk()) {
      std::println(stderr,
                   "SquareHL beginFrame error: {}",
                   frame_result.error().toString());
      return;
    }

    auto frame = std::move(frame_result).takeValue();

    const auto record_result = m_ptrRenderer->record(
        frame,
        [&](plc::GraphicCommandBuffer& cmd) { return recordGraphic(cmd); });
    if (!record_result.isOk()) {
      std::println(stderr,
                   "SquareHL record error: {}",
                   record_result.error().toString());
      return;
    }

    const auto end_result = m_ptrRenderer->endFrame(frame);
    if (!end_result.isOk()) {
      std::println(
          stderr, "SquareHL endFrame error: {}", end_result.error().toString());
      return;
    }
  }
}

plc::VoidResult SquareHL::constructShaderResources() {
  plh::ShaderLibrary shader_library(*m_ptrContext);

  PANDORA_TRY_ASSIGN(vertex_shader,
                     shader_library.load("examples/core/square/square.vert"));
  PANDORA_TRY_ASSIGN(fragment_shader,
                     shader_library.load("examples/core/square/square.frag"));

  m_shaderModuleMap["vertex"] = std::move(vertex_shader);
  m_shaderModuleMap["fragment"] = std::move(fragment_shader);

  const auto description_unit =
      plc::gpu::DescriptionUnit(m_shaderModuleMap, {"vertex", "fragment"});

  m_ptrDescriptorSetLayout = std::make_unique<plc::gpu::DescriptorSetLayout>(
      *m_ptrContext, description_unit);
  m_ptrDescriptorSet = std::make_unique<plc::gpu::DescriptorSet>(
      *m_ptrContext, *m_ptrDescriptorSetLayout);

  m_ptrDescriptorSet->updateDescriptorSet(*m_ptrContext, {}, {});

  m_ptrPipeline = std::make_unique<plc::Pipeline>(*m_ptrContext,
                                                  description_unit,
                                                  *m_ptrDescriptorSetLayout,
                                                  plc::PipelineBind::Graphics);

  return plc::ok();
}

void SquareHL::constructRenderpass(bool is_resized) {
  plc::AttachmentList attachment_list;

  const auto backbuffer_attach_index = [&]() {
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

  plc::SubpassGraph subpass_graph;

  plc::SubpassNode subpass_node(plc::PipelineBind::Graphics, 0u);
  subpass_node.attachColor(
      plc::AttachmentReference{}
          .setIndex(backbuffer_attach_index)
          .setLayout(plc::ImageLayout::ColorAttachmentOptimal));
  m_subpassIndexMap["draw"] = subpass_graph.appendNode(subpass_node);

  const auto subpass_edge =
      plc::SubpassEdge{}
          .setDependencyFlag(plc::DependencyFlag::ByRegion)
          .setDstIndex(m_subpassIndexMap.at("draw"))
          .addSrcStage(plc::PipelineStage::ColorAttachmentOutput)
          .addDstStage(plc::PipelineStage::ColorAttachmentOutput)
          .addSrcAccess(plc::AccessFlag::Unknown)
          .addDstAccess(plc::AccessFlag::ColorAttachmentWrite);
  subpass_graph.appendEdge(subpass_edge);

  if (is_resized) {
    m_ptrRenderKit->resetFramebuffer(
        *m_ptrContext,
        attachment_list,
        m_ptrWindow->getWindowSurface()->getWindowSize(),
        true);
  } else {
    m_ptrRenderKit = std::make_unique<plc::RenderKit>(
        *m_ptrContext,
        attachment_list,
        subpass_graph,
        m_ptrWindow->getWindowSurface()->getWindowSize(),
        true);
  }
}

void SquareHL::constructGraphicPipeline() {
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
                                           m_subpassIndexMap.at("draw"));
}

plc::VoidResult SquareHL::uploadGeometry() {
  const std::vector<Vertex> vertices = {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
  };

  const std::vector<uint32_t> indices = {0u, 1u, 2u, 2u, 3u, 0u};

  m_ptrVertexBuffer = std::make_unique<plc::gpu::Buffer>(
      plc::createVertexBuffer(*m_ptrContext, vertices.size() * sizeof(Vertex)));
  m_ptrIndexBuffer = std::make_unique<plc::gpu::Buffer>(
      plc::createIndexBuffer(*m_ptrContext, indices.size() * sizeof(uint32_t)));

  PANDORA_TRY(m_ptrTransfer->uploadBuffer(
      *m_ptrVertexBuffer,
      std::as_bytes(std::span(vertices.data(), vertices.size()))));

  PANDORA_TRY(m_ptrTransfer->uploadBuffer(
      *m_ptrIndexBuffer,
      std::as_bytes(std::span(indices.data(), indices.size()))));

  return plc::ok();
}

plc::VoidResult SquareHL::recordGraphic(plc::GraphicCommandBuffer& cmd) {
  PANDORA_TRY(
      cmd.beginRenderpass(*m_ptrRenderKit,
                          m_ptrWindow->getWindowSurface()->getWindowSize(),
                          plc::SubpassContents::Inline));

  cmd.bindPipeline(*m_ptrPipeline);
  cmd.bindDescriptorSet(*m_ptrPipeline, *m_ptrDescriptorSet);

  static float_t push_timer = 0.0f;
  push_timer += 0.016f;
  cmd.pushConstants(
      *m_ptrPipeline, {plc::ShaderStage::Fragment}, 0u, {push_timer});

  const auto& window_size = m_ptrWindow->getWindowSurface()->getWindowSize();
  cmd.setViewport(plc::gpu_ui::GraphicalSize<float_t>(
                      static_cast<float_t>(window_size.width),
                      static_cast<float_t>(window_size.height)),
                  0.0f,
                  1.0f);
  cmd.setScissor(window_size);

  cmd.bindVertexBuffer(*m_ptrVertexBuffer, 0u);
  cmd.bindIndexBuffer(*m_ptrIndexBuffer, 0u);
  cmd.drawIndexed(6u, 1u, 0u, 0u, 0u);

  cmd.endRenderpass();
  return plc::ok();
}

}  // namespace samples::highlevel
