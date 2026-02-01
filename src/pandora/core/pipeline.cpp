#include "pandora/core/pipeline.hpp"

#include <ranges>

#include "pandora/core/gpu/vk_helper.hpp"
#include "pandora/core/renderpass.hpp"

namespace pandora::core {

namespace pipeline {
void VertexInput::appendBinding(uint32_t binding,
                                uint32_t stride,
                                VertexInputRate input_rate) {
  m_bindings.push_back(
      vk::VertexInputBindingDescription{}
          .setBinding(binding)
          .setStride(stride)
          .setInputRate(vk_helper::getVertexInputRate(input_rate)));

  m_info.setVertexBindingDescriptions(m_bindings);
}

void VertexInput::appendAttribute(uint32_t location,
                                  uint32_t binding,
                                  DataFormat format,
                                  uint32_t offset) {
  m_attributes.push_back(vk::VertexInputAttributeDescription{}
                             .setLocation(location)
                             .setBinding(binding)
                             .setFormat(vk_helper::getFormat(format))
                             .setOffset(offset));

  m_info.setVertexAttributeDescriptions(m_attributes);
}

void InputAssembly::setTopology(PrimitiveTopology topology) {
  m_info.setTopology(vk_helper::getPrimitiveTopology(topology));
}

void InputAssembly::setRestart(bool is_enabled) {
  m_info.setPrimitiveRestartEnable(is_enabled);
}

void Tessellation::setPatchControlPoints(uint32_t count) {
  m_info.setPatchControlPoints(count);
}

void ViewportState::setViewport(const gpu_ui::GraphicalSize<float_t>& size,
                                float_t min_depth,
                                float_t max_depth) {
  m_viewport.setX(0.0f)
      .setY(0.0f)
      .setWidth(size.width)
      .setHeight(size.height)
      .setMinDepth(min_depth)
      .setMaxDepth(max_depth);

  m_info.setViewports(m_viewport);
}

void ViewportState::setScissor(const gpu_ui::GraphicalSize<uint32_t>& size) {
  m_scissor.setOffset({0, 0}).setExtent(vk_helper::getExtent2D(size));

  m_info.setScissors(m_scissor);
}

void Rasterization::setDepthBiasEnabled(bool is_enabled) {
  m_info.setDepthBiasEnable(is_enabled);
}

void Rasterization::setDepthBias(float_t constant_factor,
                                 float_t clamp,
                                 float_t slope_factor) {
  m_info.setDepthBiasConstantFactor(constant_factor)
      .setDepthBiasClamp(clamp)
      .setDepthBiasSlopeFactor(slope_factor);
}

void Rasterization::setRasterizerDiscard(bool is_enabled) {
  m_info.setRasterizerDiscardEnable(is_enabled);
}

void Rasterization::setPolygonMode(PolygonMode polygon_mode) {
  m_info.setPolygonMode(vk_helper::getPolygonMode(polygon_mode));
}

void Rasterization::setCullMode(CullMode cull_mode) {
  m_info.setCullMode(vk_helper::getCullMode(cull_mode));
}

void Rasterization::setFrontFace(FrontFace front_face) {
  m_info.setFrontFace(vk_helper::getFrontFace(front_face));
}

void Rasterization::setLineWidth(float_t line_width) {
  m_info.setLineWidth(line_width);
}

void Multisample::setSampleCount(const gpu::Context& ptr_context) {
  m_info.setRasterizationSamples(
      ptr_context.getPtrDevice()->getMaxUsableSampleCount());
}

void Multisample::setSampleShading(bool is_enabled) {
  m_info.setSampleShadingEnable(is_enabled);
}

void Multisample::setMinSampleShading(float_t min_sample_shading) {
  m_info.setMinSampleShading(min_sample_shading);
}

void DepthStencil::setDepthTest(bool is_enabled) {
  m_info.setDepthTestEnable(is_enabled);
}

void DepthStencil::setDepthWrite(bool is_enabled) {
  m_info.setDepthWriteEnable(is_enabled);
}

void DepthStencil::setDepthCompareOp(CompareOp compare_op) {
  m_info.setDepthCompareOp(vk_helper::getCompareOp(compare_op));
}

void DepthStencil::setDepthBoundsTest(bool is_enabled) {
  m_info.setDepthBoundsTestEnable(is_enabled);
}

void DepthStencil::setStencilTest(bool is_enabled) {
  m_info.setStencilTestEnable(is_enabled);
}

void DepthStencil::setFrontStencilOp(const StencilOpState& state) {
  m_info.setFront(vk_helper::getStencilOpState(state));
}

void DepthStencil::setBackStencilOp(const StencilOpState& state) {
  m_info.setBack(vk_helper::getStencilOpState(state));
}

void ColorBlend::setLogicOp(bool is_enabled, LogicOp logic_op) {
  m_info.setLogicOpEnable(is_enabled);
  m_info.setLogicOp(vk_helper::getLogicOp(logic_op));
}

void ColorBlend::appendAttachment(const ColorBlendAttachment& attachment) {
  using namespace vk_helper;

  m_attachments.push_back(
      vk::PipelineColorBlendAttachmentState{}
          .setBlendEnable(attachment.is_enabled)
          .setSrcColorBlendFactor(getBlendFactor(attachment.src_color))
          .setDstColorBlendFactor(getBlendFactor(attachment.dst_color))
          .setColorBlendOp(getBlendOp(attachment.color_op))
          .setSrcAlphaBlendFactor(getBlendFactor(attachment.src_alpha))
          .setDstAlphaBlendFactor(getBlendFactor(attachment.dst_alpha))
          .setAlphaBlendOp(getBlendOp(attachment.alpha_op))
          .setColorWriteMask(getColorComponent(attachment.color_components)));
}

void DynamicState::appendState(DynamicOption option) {
  m_states.push_back(vk_helper::getDynamicState(option));
  m_info.setDynamicStates(m_states);
}

}  // namespace pipeline

Pipeline::Pipeline(const gpu::Context& ptr_context,
                   const gpu::DescriptionUnit& description_unit,
                   const gpu::DescriptorSetLayout& descriptor_set_layout,
                   PipelineBind bind_point) {
  using P = std::ranges::range_value_t<
      decltype(description_unit.getPushConstantRangeMap()
               | std::views::values)>;
  const auto push_constant_ranges =
      description_unit.getPushConstantRangeMap() | std::views::values
      | std::views::transform([](const P& x) {
          return vk::PushConstantRange{}
              .setStageFlags(x.stage_flags)
              .setOffset(x.offset)
              .setSize(static_cast<uint32_t>(x.size));
        })
      | std::ranges::to<std::vector<vk::PushConstantRange>>();

  m_ptrPipelineLayout =
      ptr_context.getPtrDevice()
          ->getPtrLogicalDevice()
          ->createPipelineLayoutUnique(
              vk::PipelineLayoutCreateInfo{}
                  .setSetLayouts(descriptor_set_layout.getDescriptorSetLayout())
                  .setPushConstantRanges(push_constant_ranges));

  m_bindPoint = vk_helper::getPipelineBindPoint(bind_point);
}

Pipeline::~Pipeline() {}

void Pipeline::constructComputePipeline(
    const gpu::Context& ptr_context, const gpu::ShaderModule& shader_module) {
  m_queueFamilyType = QueueFamilyType::Compute;

  const auto compute_pipeline_info =
      vk::ComputePipelineCreateInfo{}
          .setLayout(m_ptrPipelineLayout.get())
          .setStage(vk::PipelineShaderStageCreateInfo{}
                        .setStage(vk::ShaderStageFlagBits::eCompute)
                        .setModule(shader_module.getModule())
                        .setPName(shader_module.getEntryPointName().c_str()));

  m_ptrPipeline =
      ptr_context.getPtrDevice()
          ->getPtrLogicalDevice()
          ->createComputePipelineUnique(nullptr, compute_pipeline_info)
          .value;
}

void Pipeline::constructGraphicsPipeline(
    const gpu::Context& ptr_context,
    const std::unordered_map<std::string, gpu::ShaderModule>& shader_module_map,
    const std::vector<std::string>& module_keys,
    pipeline::GraphicInfo& ptr_graphic_info,
    const Renderpass& render_pass,
    uint32_t subpass_index) {
  m_queueFamilyType = QueueFamilyType::Graphics;
  vk::GraphicsPipelineCreateInfo pipeline_info{};

  using M = std::ranges::range_value_t<decltype(module_keys)>;
  const auto shader_stage_infos =
      module_keys | std::views::transform([&shader_module_map](const M& x) {
        const auto& shader_module = shader_module_map.at(x);

        return vk::PipelineShaderStageCreateInfo{}
            .setStage(shader_module.getShaderStageFlag())
            .setModule(shader_module.getModule())
            .setPName(shader_module.getEntryPointName().c_str());
      })
      | std::ranges::to<std::vector<vk::PipelineShaderStageCreateInfo>>();

  {
    auto& vertex_input = ptr_graphic_info.vertex_input;
    vertex_input.m_info.setVertexBindingDescriptions(vertex_input.m_bindings)
        .setVertexAttributeDescriptions(vertex_input.m_attributes);

    pipeline_info.setPVertexInputState(&(vertex_input.m_info));
  }

  {
    auto& color_blend = ptr_graphic_info.color_blend;
    color_blend.m_info.setAttachments(color_blend.m_attachments);

    pipeline_info.setPColorBlendState(&(color_blend.m_info));
  }

  {
    auto& dynamic_state = ptr_graphic_info.dynamic_state;
    dynamic_state.m_info.setDynamicStates(dynamic_state.m_states);

    pipeline_info.setPDynamicState(&(dynamic_state.m_info));
  }

  pipeline_info.setStages(shader_stage_infos)
      .setPInputAssemblyState(&(ptr_graphic_info.input_assembly.m_info))
      .setPTessellationState(&(ptr_graphic_info.tessellation.m_info))
      .setPViewportState(&(ptr_graphic_info.viewport_state.m_info))
      .setPRasterizationState(&(ptr_graphic_info.rasterization.m_info))
      .setPMultisampleState(&(ptr_graphic_info.multisample.m_info))
      .setPDepthStencilState(&(ptr_graphic_info.depth_stencil.m_info))
      .setLayout(m_ptrPipelineLayout.get())
      .setRenderPass(render_pass.getRenderPass())
      .setSubpass(subpass_index);

  m_ptrPipeline = ptr_context.getPtrDevice()
                      ->getPtrLogicalDevice()
                      ->createGraphicsPipelineUnique(nullptr, pipeline_info)
                      .value;
}

}  // namespace pandora::core
