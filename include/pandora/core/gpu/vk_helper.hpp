/*
This header file is part of the Hephics project.
This file contains the functions for convert Hephics enum to Vulkan API's enum.
*/

#pragma once

#include "pandora/core/gpu.hpp"
#include "pandora/core/rendering_structures.hpp"
#include "pandora/core/rendering_types.hpp"

namespace vk_helper {

vk::MemoryPropertyFlags getMemoryPropertyFlags(const pandora::core::MemoryUsage memory_usage);

vk::AccessFlagBits getAccessFlagBits(const pandora::core::AccessFlag access_flag);

vk::PipelineStageFlagBits getPipelineStageFlagBits(const pandora::core::PipelineStage stage);

vk::PipelineBindPoint getPipelineBindPoint(const pandora::core::PipelineBind bind_point);

vk::ImageUsageFlagBits getImageUsage(const pandora::core::ImageUsage image_usage);

vk::ImageLayout getImageLayout(const pandora::core::ImageLayout image_layout);

vk::Format getFormat(const pandora::core::DataFormat format);

vk::SampleCountFlagBits getSampleCount(const pandora::core::ImageSampleCount sample_count);

vk::ImageAspectFlags getImageAspectFlags(const pandora::core::ImageAspect image_aspect);

vk::ShaderStageFlags getShaderStageFlagBits(const pandora::core::ShaderStage shader_stage);

vk::Extent2D getExtent2D(const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size);

vk::Extent3D getExtent3D(const pandora::core::gpu_ui::GraphicalSize<uint32_t>& graphical_size);

vk::AttachmentLoadOp getAttachmentLoadOp(const pandora::core::AttachmentLoadOp attachment_load_op);

vk::AttachmentStoreOp getAttachmentStoreOp(const pandora::core::AttachmentStoreOp attachment_store_op);

// Pipeline-related conversions
vk::CompareOp getCompareOp(const pandora::core::CompareOp compare_op);

vk::StencilOp getStencilOp(const pandora::core::StencilOp stencil_op);

vk::StencilOpState getStencilOpState(const pandora::core::StencilOpState& state);

vk::BlendFactor getBlendFactor(const pandora::core::BlendFactor blend_factor);

vk::BlendOp getBlendOp(const pandora::core::BlendOp blend_op);

vk::ColorComponentFlags getColorComponent(const std::vector<pandora::core::ColorComponent>& color_components);

// Command buffer related conversions
vk::CommandBufferUsageFlagBits getCommandBufferUsageFlagBits(const pandora::core::CommandBufferUsage usage_flags);

vk::SubpassContents getSubpassContents(const pandora::core::SubpassContents subpass_contents);

// Subpass related conversions
vk::DependencyFlagBits getDependencyFlag(const pandora::core::DependencyFlag dependency_flag);

// Pipeline state conversions
vk::PrimitiveTopology getPrimitiveTopology(const pandora::core::PrimitiveTopology topology);

vk::PolygonMode getPolygonMode(const pandora::core::PolygonMode polygon_mode);

vk::CullModeFlags getCullMode(const pandora::core::CullMode cull_mode);

vk::FrontFace getFrontFace(const pandora::core::FrontFace front_face);

vk::LogicOp getLogicOp(const pandora::core::LogicOp logic_op);

vk::DynamicState getDynamicState(const pandora::core::DynamicOption option);

vk::VertexInputRate getVertexInputRate(const pandora::core::VertexInputRate input_rate);

// Sampler related conversions
vk::Filter getSamplerFilter(const pandora::core::SamplerFilter filter);

vk::SamplerMipmapMode getSamplerMipmapMode(const pandora::core::SamplerMipmapMode mipmap_mode);

vk::SamplerAddressMode getSamplerAddressMode(const pandora::core::SamplerAddressMode address_mode);

vk::CompareOp getSamplerCompareOp(const pandora::core::SamplerCompareOp compare_op);

vk::BorderColor getSamplerBorderColor(const pandora::core::SamplerBorderColor border_color);

}  // namespace vk_helper
