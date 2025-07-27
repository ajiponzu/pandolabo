#include "pandora/core/pipeline.hpp"

#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/renderpass.hpp"

void pandora::core::pipeline::VertexInput::appendBinding(const uint32_t binding,
                                                         const uint32_t stride,
                                                         const VertexInputRate input_rate) {
  vk::VertexInputBindingDescription binding_description;
  binding_description.setBinding(binding);
  binding_description.setStride(stride);
  binding_description.setInputRate(vk_helper::getVertexInputRate(input_rate));

  m_bindings.push_back(binding_description);

  m_info.setVertexBindingDescriptions(m_bindings);
}

void pandora::core::pipeline::VertexInput::appendAttribute(const uint32_t location,
                                                           const uint32_t binding,
                                                           const DataFormat format,
                                                           const uint32_t offset) {
  using vk_helper::getFormat;  // 狭いスコープでのusing宣言

  vk::VertexInputAttributeDescription attribute_description;
  attribute_description.setLocation(location);
  attribute_description.setBinding(binding);
  attribute_description.setFormat(getFormat(format));
  attribute_description.setOffset(offset);

  m_attributes.push_back(attribute_description);

  m_info.setVertexAttributeDescriptions(m_attributes);
}

void pandora::core::pipeline::InputAssembly::setTopology(const PrimitiveTopology topology) {
  m_info.setTopology(vk_helper::getPrimitiveTopology(topology));
}

void pandora::core::pipeline::InputAssembly::setRestart(const bool is_enabled) {
  m_info.setPrimitiveRestartEnable(is_enabled);
}

void pandora::core::pipeline::Tessellation::setPatchControlPoints(const uint32_t count) {
  m_info.setPatchControlPoints(count);
}

void pandora::core::pipeline::ViewportState::setViewport(const gpu_ui::GraphicalSize<float_t>& size,
                                                         const float_t min_depth,
                                                         const float_t max_depth) {
  m_viewport.setX(0.0f);
  m_viewport.setY(0.0f);
  m_viewport.setWidth(size.width);
  m_viewport.setHeight(size.height);
  m_viewport.setMinDepth(min_depth);
  m_viewport.setMaxDepth(max_depth);

  m_info.setViewports(m_viewport);
}

void pandora::core::pipeline::ViewportState::setScissor(const gpu_ui::GraphicalSize<uint32_t>& size) {
  m_scissor.setOffset({0, 0});
  m_scissor.setExtent(vk_helper::getExtent2D(size));

  m_info.setScissors(m_scissor);
}

void pandora::core::pipeline::Rasterization::setDepthBiasEnabled(const bool is_enabled) {
  m_info.setDepthBiasEnable(is_enabled);
}

void pandora::core::pipeline::Rasterization::setDepthBias(const float_t constant_factor,
                                                          const float_t clamp,
                                                          const float_t slope_factor) {
  m_info.setDepthBiasConstantFactor(constant_factor);
  m_info.setDepthBiasClamp(clamp);
  m_info.setDepthBiasSlopeFactor(slope_factor);
}

void pandora::core::pipeline::Rasterization::setRasterizerDiscard(const bool is_enabled) {
  m_info.setRasterizerDiscardEnable(is_enabled);
}

void pandora::core::pipeline::Rasterization::setPolygonMode(const PolygonMode polygon_mode) {
  m_info.setPolygonMode(vk_helper::getPolygonMode(polygon_mode));
}

void pandora::core::pipeline::Rasterization::setCullMode(const CullMode cull_mode) {
  m_info.setCullMode(vk_helper::getCullMode(cull_mode));
}

void pandora::core::pipeline::Rasterization::setFrontFace(const FrontFace front_face) {
  m_info.setFrontFace(vk_helper::getFrontFace(front_face));
}

void pandora::core::pipeline::Rasterization::setLineWidth(const float_t line_width) {
  m_info.setLineWidth(line_width);
}

void pandora::core::pipeline::Multisample::setSampleCount(const std::unique_ptr<gpu::Context>& ptr_context) {
  m_info.setRasterizationSamples(ptr_context->getPtrDevice()->getMaxUsableSampleCount());
}

void pandora::core::pipeline::Multisample::setSampleShading(const bool is_enabled) {
  m_info.setSampleShadingEnable(is_enabled);
}

void pandora::core::pipeline::Multisample::setMinSampleShading(const float_t min_sample_shading) {
  m_info.setMinSampleShading(min_sample_shading);
}

void pandora::core::pipeline::DepthStencil::setDepthTest(const bool is_enabled) {
  m_info.setDepthTestEnable(is_enabled);
}

void pandora::core::pipeline::DepthStencil::setDepthWrite(const bool is_enabled) {
  m_info.setDepthWriteEnable(is_enabled);
}

void pandora::core::pipeline::DepthStencil::setDepthCompareOp(const CompareOp compare_op) {
  using vk_helper::getCompareOp;
  m_info.setDepthCompareOp(getCompareOp(compare_op));
}

void pandora::core::pipeline::DepthStencil::setDepthBoundsTest(const bool is_enabled) {
  m_info.setDepthBoundsTestEnable(is_enabled);
}

void pandora::core::pipeline::DepthStencil::setStencilTest(const bool is_enabled) {
  m_info.setStencilTestEnable(is_enabled);
}

void pandora::core::pipeline::DepthStencil::setFrontStencilOp(const StencilOpState& state) {
  using vk_helper::getStencilOpState;
  m_info.setFront(getStencilOpState(state));
}

void pandora::core::pipeline::DepthStencil::setBackStencilOp(const StencilOpState& state) {
  using vk_helper::getStencilOpState;
  m_info.setBack(getStencilOpState(state));
}

void pandora::core::pipeline::ColorBlend::setLogicOp(const bool is_enabled, const LogicOp logic_op) {
  m_info.setLogicOpEnable(is_enabled);
  m_info.setLogicOp(vk_helper::getLogicOp(logic_op));
}

void pandora::core::pipeline::ColorBlend::appendAttachment(const ColorBlendAttachment& attachment) {
  using namespace vk_helper;  // ローカルスコープでusing宣言

  vk::PipelineColorBlendAttachmentState attachment_state;
  attachment_state.setBlendEnable(attachment.is_enabled);
  attachment_state.setSrcColorBlendFactor(getBlendFactor(attachment.src_color));
  attachment_state.setDstColorBlendFactor(getBlendFactor(attachment.dst_color));
  attachment_state.setColorBlendOp(getBlendOp(attachment.color_op));
  attachment_state.setSrcAlphaBlendFactor(getBlendFactor(attachment.src_alpha));
  attachment_state.setDstAlphaBlendFactor(getBlendFactor(attachment.dst_alpha));
  attachment_state.setAlphaBlendOp(getBlendOp(attachment.alpha_op));
  attachment_state.setColorWriteMask(getColorComponent(attachment.color_components));

  m_attachments.push_back(attachment_state);
}

void pandora::core::pipeline::DynamicState::appendState(const DynamicOption option) {
  m_states.push_back(vk_helper::getDynamicState(option));
  m_info.setDynamicStates(m_states);
}

pandora::core::Pipeline::Pipeline(const std::unique_ptr<gpu::Context>& ptr_context,
                                  const gpu::DescriptionUnit& description_unit,
                                  const gpu::DescriptorSetLayout& descriptor_set_layout,
                                  const PipelineBind bind_point) {
  std::vector<vk::PushConstantRange> push_constant_ranges;
  for (const auto& [_, push_constant_range] : description_unit.getPushConstantRangeMap()) {
    push_constant_ranges.emplace_back(
        push_constant_range.stage_flags, push_constant_range.offset, static_cast<uint32_t>(push_constant_range.size));
  }

  vk::PipelineLayoutCreateInfo pipeline_layout_info;
  pipeline_layout_info.setSetLayouts(descriptor_set_layout.getDescriptorSetLayout());
  pipeline_layout_info.setPushConstantRanges(push_constant_ranges);

  m_ptrPipelineLayout =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createPipelineLayoutUnique(pipeline_layout_info);

  m_bindPoint = vk_helper::getPipelineBindPoint(bind_point);
}

pandora::core::Pipeline::~Pipeline() {}

void pandora::core::Pipeline::constructComputePipeline(const std::unique_ptr<gpu::Context>& ptr_context,
                                                       const gpu::ShaderModule& shader_module) {
  m_queueFamilyType = QueueFamilyType::Compute;

  vk::PipelineShaderStageCreateInfo shader_stage_info;
  shader_stage_info.setStage(vk::ShaderStageFlagBits::eCompute);
  shader_stage_info.setModule(shader_module.getModule());
  shader_stage_info.setPName(shader_module.getEntryPointName().c_str());

  vk::ComputePipelineCreateInfo compute_pipeline_info;
  compute_pipeline_info.setLayout(m_ptrPipelineLayout.get());
  compute_pipeline_info.setStage(shader_stage_info);

  m_ptrPipeline = ptr_context->getPtrDevice()
                      ->getPtrLogicalDevice()
                      ->createComputePipelineUnique(nullptr, compute_pipeline_info)
                      .value;
}

void pandora::core::Pipeline::constructGraphicsPipeline(
    const std::unique_ptr<gpu::Context>& ptr_context,
    const std::unordered_map<std::string, gpu::ShaderModule>& shader_module_map,
    const std::vector<std::string>& module_keys,
    const std::unique_ptr<pipeline::GraphicInfo>& ptr_graphic_info,
    const Renderpass& render_pass,
    const uint32_t subpass_index) {
  m_queueFamilyType = QueueFamilyType::Graphics;

  std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_infos;
  for (const auto& module_key : module_keys) {
    const auto& shader_module = shader_module_map.at(module_key);

    vk::PipelineShaderStageCreateInfo shader_stage_info;
    shader_stage_info.setStage(shader_module.getShaderStageFlag());
    shader_stage_info.setModule(shader_module.getModule());
    shader_stage_info.setPName(shader_module.getEntryPointName().c_str());
    shader_stage_infos.emplace_back(shader_stage_info);
  }

  vk::GraphicsPipelineCreateInfo pipeline_info;
  pipeline_info.setStages(shader_stage_infos);

  {
    auto& vertex_input = ptr_graphic_info->vertex_input;
    vertex_input.m_info.setVertexBindingDescriptions(vertex_input.m_bindings);
    vertex_input.m_info.setVertexAttributeDescriptions(vertex_input.m_attributes);

    pipeline_info.setPVertexInputState(&(vertex_input.m_info));
  }

  {
    pipeline_info.setPInputAssemblyState(&(ptr_graphic_info->input_assembly.m_info));
  }

  {
    pipeline_info.setPTessellationState(&(ptr_graphic_info->tessellation.m_info));
  }

  {
    pipeline_info.setPViewportState(&(ptr_graphic_info->viewport_state.m_info));
  }

  {
    pipeline_info.setPRasterizationState(&(ptr_graphic_info->rasterization.m_info));
  }

  {
    pipeline_info.setPMultisampleState(&(ptr_graphic_info->multisample.m_info));
  }

  {
    pipeline_info.setPDepthStencilState(&(ptr_graphic_info->depth_stencil.m_info));
  }

  {
    auto& color_blend = ptr_graphic_info->color_blend;
    color_blend.m_info.setAttachments(color_blend.m_attachments);

    pipeline_info.setPColorBlendState(&(color_blend.m_info));
  }

  {
    auto& dynamic_state = ptr_graphic_info->dynamic_state;
    dynamic_state.m_info.setDynamicStates(dynamic_state.m_states);

    pipeline_info.setPDynamicState(&(dynamic_state.m_info));
  }

  pipeline_info.setLayout(m_ptrPipelineLayout.get());
  pipeline_info.setRenderPass(render_pass.getRenderPass());
  pipeline_info.setSubpass(subpass_index);

  m_ptrPipeline =
      ptr_context->getPtrDevice()->getPtrLogicalDevice()->createGraphicsPipelineUnique(nullptr, pipeline_info).value;
}
