/* Utility: Vulkan enum conversion helpers for Pandolabo */

#pragma once

#include "pandora/core/gpu.hpp"
#include "pandora/core/rendering_structures.hpp"
#include "pandora/core/rendering_types.hpp"

namespace vk_helper {

vk::MemoryPropertyFlags getMemoryPropertyFlags(
    pandora::core::MemoryUsage memory_usage);

vk::AccessFlagBits getAccessFlagBits(pandora::core::AccessFlag access_flag);

vk::PipelineStageFlagBits getPipelineStageFlagBits(
    pandora::core::PipelineStage stage);

vk::PipelineBindPoint getPipelineBindPoint(
    pandora::core::PipelineBind bind_point);

vk::ImageUsageFlagBits getImageUsage(pandora::core::ImageUsage image_usage);

vk::ImageLayout getImageLayout(pandora::core::ImageLayout image_layout);

vk::Format getFormat(pandora::core::DataFormat format);

vk::SampleCountFlagBits getSampleCount(
    pandora::core::ImageSampleCount sample_count);

vk::ImageAspectFlags getImageAspectFlags(
    pandora::core::ImageAspect image_aspect);

vk::ShaderStageFlags getShaderStageFlagBits(
    pandora::core::ShaderStage shader_stage);

vk::Extent2D getExtent2D(
    const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size);

vk::Extent3D getExtent3D(
    const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size);

vk::AttachmentLoadOp getAttachmentLoadOp(
    pandora::core::AttachmentLoadOp attachment_load_op);

vk::AttachmentStoreOp getAttachmentStoreOp(
    pandora::core::AttachmentStoreOp attachment_store_op);

// Pipeline-related conversions
vk::CompareOp getCompareOp(pandora::core::CompareOp compare_op);

vk::StencilOp getStencilOp(pandora::core::StencilOp stencil_op);

vk::StencilOpState getStencilOpState(
    const pandora::core::StencilOpState& state);

vk::BlendFactor getBlendFactor(pandora::core::BlendFactor blend_factor);

vk::BlendOp getBlendOp(pandora::core::BlendOp blend_op);

vk::ColorComponentFlags getColorComponent(
    const std::vector<pandora::core::ColorComponent>& color_components);

// Command buffer related conversions
vk::CommandBufferUsageFlagBits getCommandBufferUsageFlagBits(
    pandora::core::CommandBufferUsage usage_flags);

vk::SubpassContents getSubpassContents(
    pandora::core::SubpassContents subpass_contents);

// Subpass related conversions
vk::DependencyFlagBits getDependencyFlag(
    pandora::core::DependencyFlag dependency_flag);

// Pipeline state conversions
vk::PrimitiveTopology getPrimitiveTopology(
    pandora::core::PrimitiveTopology topology);

vk::PolygonMode getPolygonMode(pandora::core::PolygonMode polygon_mode);

vk::CullModeFlags getCullMode(pandora::core::CullMode cull_mode);

vk::FrontFace getFrontFace(pandora::core::FrontFace front_face);

vk::LogicOp getLogicOp(pandora::core::LogicOp logic_op);

vk::DynamicState getDynamicState(pandora::core::DynamicOption option);

vk::VertexInputRate getVertexInputRate(
    pandora::core::VertexInputRate input_rate);

// Sampler related conversions
vk::Filter getSamplerFilter(pandora::core::SamplerFilter filter);

vk::SamplerMipmapMode getSamplerMipmapMode(
    pandora::core::SamplerMipmapMode mipmap_mode);

vk::SamplerAddressMode getSamplerAddressMode(
    pandora::core::SamplerAddressMode address_mode);

vk::CompareOp getSamplerCompareOp(pandora::core::SamplerCompareOp compare_op);

vk::BorderColor getSamplerBorderColor(
    pandora::core::SamplerBorderColor border_color);

}  // namespace vk_helper
