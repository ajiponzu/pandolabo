#include <spirv_cross/spirv_cross.hpp>

#include "pandora/core/gpu.hpp"

class ShaderCompiler : public spirv_cross::Compiler {
 public:
  ShaderCompiler(const std::vector<uint32_t>& spirv_binary) : spirv_cross::Compiler(spirv_binary) {}

  ~ShaderCompiler() {}

  vk::ShaderStageFlagBits getShaderStageFlagBits() const;
  vk::ShaderStageFlags getShaderStageFlags() const;
  std::string getEntryPointName() const {
    return this->get_entry_point().name;
  }

  std::unordered_map<std::string, pandora::core::DescriptorInfo> getDescriptorInfos() const;

  std::unordered_map<std::string, pandora::core::PushConstantRange> getPushConstantRanges() const;
};

vk::ShaderStageFlagBits ShaderCompiler::getShaderStageFlagBits() const {
  const auto& entry_point = this->get_entry_point();

  switch (entry_point.model) {
    case spv::ExecutionModelVertex:
      return vk::ShaderStageFlagBits::eVertex;
    case spv::ExecutionModelFragment:
      return vk::ShaderStageFlagBits::eFragment;
    case spv::ExecutionModelGLCompute:
      return vk::ShaderStageFlagBits::eCompute;
    default:
      return vk::ShaderStageFlagBits::eAll;
  }
}

vk::ShaderStageFlags ShaderCompiler::getShaderStageFlags() const {
  vk::ShaderStageFlags flags;

  const auto& entry_point = this->get_entry_point();
  switch (entry_point.model) {
    case spv::ExecutionModelVertex:
      flags |= vk::ShaderStageFlagBits::eVertex;
      break;
    case spv::ExecutionModelFragment:
      flags |= vk::ShaderStageFlagBits::eFragment;
      break;
    case spv::ExecutionModelGLCompute:
      flags |= vk::ShaderStageFlagBits::eCompute;
      break;
  }

  return flags;
}

static uint32_t get_type_size(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type) {
  if (type.basetype == spirv_cross::SPIRType::Struct) {
    return static_cast<uint32_t>(compiler.get_declared_struct_size(type));
  }

  return 0U;
}

static void set_descriptor_infos(std::unordered_map<std::string, pandora::core::DescriptorInfo>& descriptor_info_map,
                                 const spirv_cross::Compiler& compiler,
                                 const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
                                 const vk::DescriptorType descriptor_type,
                                 const vk::ShaderStageFlags shader_stage_flags) {
  for (const auto& resource : resources) {
    pandora::core::DescriptorInfo descriptor_info;
    descriptor_info.type = descriptor_type;
    descriptor_info.size = get_type_size(compiler, compiler.get_type(resource.base_type_id));
    descriptor_info.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    descriptor_info.stage_flags = shader_stage_flags;

    descriptor_info_map.insert({resource.name, descriptor_info});
  }
}

std::unordered_map<std::string, pandora::core::DescriptorInfo> ShaderCompiler::getDescriptorInfos() const {
  std::unordered_map<std::string, pandora::core::DescriptorInfo> descriptor_info_map;

  const auto resources = this->get_shader_resources();
  const auto shader_stage_flags = getShaderStageFlags();

  set_descriptor_infos(
      descriptor_info_map, *this, resources.uniform_buffers, vk::DescriptorType::eUniformBuffer, shader_stage_flags);
  set_descriptor_infos(
      descriptor_info_map, *this, resources.separate_images, vk::DescriptorType::eSampledImage, shader_stage_flags);
  set_descriptor_infos(
      descriptor_info_map, *this, resources.separate_samplers, vk::DescriptorType::eSampler, shader_stage_flags);
  set_descriptor_infos(descriptor_info_map,
                       *this,
                       resources.sampled_images,
                       vk::DescriptorType::eCombinedImageSampler,
                       shader_stage_flags);
  set_descriptor_infos(
      descriptor_info_map, *this, resources.storage_images, vk::DescriptorType::eStorageImage, shader_stage_flags);
  set_descriptor_infos(
      descriptor_info_map, *this, resources.storage_buffers, vk::DescriptorType::eStorageBuffer, shader_stage_flags);

  return descriptor_info_map;
}

std::unordered_map<std::string, pandora::core::PushConstantRange> ShaderCompiler::getPushConstantRanges() const {
  std::unordered_map<std::string, pandora::core::PushConstantRange> push_constant_range_map;

  const auto& resources = this->get_shader_resources();
  const auto& shader_stage_flags = getShaderStageFlags();

  uint32_t previous_size = 0U;
  for (const auto& resource : resources.push_constant_buffers) {
    pandora::core::PushConstantRange push_constant_range;
    push_constant_range.stage_flags = shader_stage_flags;
    push_constant_range.offset = previous_size;
    push_constant_range.size = this->get_declared_struct_size(this->get_type(resource.base_type_id));

    push_constant_range_map.insert({resource.name, push_constant_range});
    previous_size += static_cast<uint32_t>(push_constant_range.size);
  }

  return push_constant_range_map;
}

pandora::core::gpu::ShaderModule::ShaderModule(const std::unique_ptr<Context>& ptr_context,
                                               const std::vector<uint32_t>& spirv_binary) {
  {
    ShaderCompiler compiler(spirv_binary);

    m_entryPointName = compiler.getEntryPointName();
    m_descriptorInfoMap = compiler.getDescriptorInfos();
    m_pushConstantRangeMap = compiler.getPushConstantRanges();
    m_shaderStageFlag = compiler.getShaderStageFlagBits();
  }

  {
    vk::ShaderModuleCreateInfo shader_module_info{};
    shader_module_info.codeSize = spirv_binary.size() * sizeof(uint32_t);
    shader_module_info.pCode = spirv_binary.data();

    m_ptrShaderModule =
        ptr_context->getPtrDevice()->getPtrLogicalDevice()->createShaderModuleUnique(shader_module_info);
  }
}

pandora::core::gpu::ShaderModule::~ShaderModule() {}
