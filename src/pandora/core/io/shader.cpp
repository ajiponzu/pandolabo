#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <unordered_map>

#include "pandora/core/io.hpp"

static std::pair<std::string, ::EShLanguage> translate_shader_stage(const std::string& shader_code_path) {
  static const std::unordered_map<std::string, std::pair<std::string, ::EShLanguage>> extension_map = {
      {".vert", {"vert", EShLangVertex}},
      {".frag", {"frag", EShLangFragment}},
      {".comp", {"comp", EShLangCompute}},
      {".rgen", {"rgen", EShLangRayGen}},
      {".rmiss", {"rmiss", EShLangMiss}},
      {".rchit", {"rchit", EShLangClosestHit}},
      {".rahit", {"rahit", EShLangAnyHit}}};

  for (const auto& [extension, result] : extension_map) {
    if (shader_code_path.ends_with(extension)) {
      return result;
    }
  }

  throw std::runtime_error("Unknown shader stage");
}

static TBuiltInResource init_t_built_in_resources() {
  TBuiltInResource resources{};

  resources.maxLights = 32;
  resources.maxClipPlanes = 6;
  resources.maxTextureUnits = 32;
  resources.maxTextureCoords = 32;
  resources.maxVertexAttribs = 64;
  resources.maxVertexUniformComponents = 4096;
  resources.maxVaryingFloats = 64;
  resources.maxVertexTextureImageUnits = 32;
  resources.maxCombinedTextureImageUnits = 80;
  resources.maxTextureImageUnits = 32;
  resources.maxFragmentUniformComponents = 4096;
  resources.maxDrawBuffers = 32;
  resources.maxVertexUniformVectors = 128;
  resources.maxVaryingVectors = 8;
  resources.maxFragmentUniformVectors = 16;
  resources.maxVertexOutputVectors = 16;
  resources.maxFragmentInputVectors = 15;
  resources.minProgramTexelOffset = -8;
  resources.maxProgramTexelOffset = 7;
  resources.maxClipDistances = 8;
  resources.maxComputeWorkGroupCountX = 65535;
  resources.maxComputeWorkGroupCountY = 65535;
  resources.maxComputeWorkGroupCountZ = 65535;
  resources.maxComputeWorkGroupSizeX = 1024;
  resources.maxComputeWorkGroupSizeY = 1024;
  resources.maxComputeWorkGroupSizeZ = 64;
  resources.maxComputeUniformComponents = 1024;
  resources.maxComputeTextureImageUnits = 16;
  resources.maxComputeImageUniforms = 8;
  resources.maxComputeAtomicCounters = 8;
  resources.maxComputeAtomicCounterBuffers = 1;
  resources.maxVaryingComponents = 60;
  resources.maxVertexOutputComponents = 64;
  resources.maxGeometryInputComponents = 64;
  resources.maxGeometryOutputComponents = 128;
  resources.maxFragmentInputComponents = 128;
  resources.maxImageUnits = 8;
  resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
  resources.maxCombinedShaderOutputResources = 8;
  resources.maxImageSamples = 0;
  resources.maxVertexImageUniforms = 0;
  resources.maxTessControlImageUniforms = 0;
  resources.maxTessEvaluationImageUniforms = 0;
  resources.maxGeometryImageUniforms = 0;
  resources.maxFragmentImageUniforms = 8;
  resources.maxCombinedImageUniforms = 8;
  resources.maxGeometryTextureImageUnits = 16;
  resources.maxGeometryOutputVertices = 256;
  resources.maxGeometryTotalOutputComponents = 1024;
  resources.maxGeometryUniformComponents = 1024;
  resources.maxGeometryVaryingComponents = 64;
  resources.maxTessControlInputComponents = 128;
  resources.maxTessControlOutputComponents = 128;
  resources.maxTessControlTextureImageUnits = 16;
  resources.maxTessControlUniformComponents = 1024;
  resources.maxTessControlTotalOutputComponents = 4096;
  resources.maxTessEvaluationInputComponents = 128;
  resources.maxTessEvaluationOutputComponents = 128;
  resources.maxTessEvaluationTextureImageUnits = 16;
  resources.maxTessEvaluationUniformComponents = 1024;
  resources.maxTessPatchComponents = 120;
  resources.maxPatchVertices = 32;
  resources.maxTessGenLevel = 64;
  resources.maxViewports = 16;
  resources.maxVertexAtomicCounters = 0;
  resources.maxTessControlAtomicCounters = 0;
  resources.maxTessEvaluationAtomicCounters = 0;
  resources.maxGeometryAtomicCounters = 0;
  resources.maxFragmentAtomicCounters = 8;
  resources.maxCombinedAtomicCounters = 8;
  resources.maxAtomicCounterBindings = 1;
  resources.maxVertexAtomicCounterBuffers = 0;
  resources.maxTessControlAtomicCounterBuffers = 0;
  resources.maxTessEvaluationAtomicCounterBuffers = 0;
  resources.maxGeometryAtomicCounterBuffers = 0;
  resources.maxFragmentAtomicCounterBuffers = 1;
  resources.maxCombinedAtomicCounterBuffers = 1;
  resources.maxAtomicCounterBufferSize = 16384;
  resources.maxTransformFeedbackBuffers = 4;
  resources.maxTransformFeedbackInterleavedComponents = 64;
  resources.maxCullDistances = 8;
  resources.maxCombinedClipAndCullDistances = 8;
  resources.maxSamples = 4;
  resources.maxMeshOutputVerticesNV = 256;
  resources.maxMeshOutputPrimitivesNV = 512;
  resources.maxMeshWorkGroupSizeX_NV = 32;
  resources.maxMeshWorkGroupSizeY_NV = 1;
  resources.maxMeshWorkGroupSizeZ_NV = 1;
  resources.maxTaskWorkGroupSizeX_NV = 32;
  resources.maxTaskWorkGroupSizeY_NV = 1;
  resources.maxTaskWorkGroupSizeZ_NV = 1;
  resources.maxMeshViewCountNV = 4;

  resources.limits.nonInductiveForLoops = 1;
  resources.limits.whileLoops = 1;
  resources.limits.doWhileLoops = 1;
  resources.limits.generalUniformIndexing = 1;
  resources.limits.generalAttributeMatrixVectorIndexing = 1;
  resources.limits.generalVaryingIndexing = 1;
  resources.limits.generalSamplerIndexing = 1;
  resources.limits.generalVariableIndexing = 1;
  resources.limits.generalConstantMatrixVectorIndexing = 1;

  return resources;
}

static std::vector<uint32_t> compile_shader(const ::EShLanguage& shader_stage, const std::string& shader_code) {
  glslang::InitializeProcess();

  std::vector shader_c_strings = {shader_code.data()};

  glslang::TShader shader(shader_stage);
  shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_5);
  shader.setStrings(shader_c_strings.data(), static_cast<int32_t>(shader_c_strings.size()));

  EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
  const auto t_built_in_resources = init_t_built_in_resources();
  if (!shader.parse(&t_built_in_resources, 100, false, messages)) {
    throw std::runtime_error(shader_code + "\n" + shader.getInfoLog());
  }

  glslang::TProgram program{};
  program.addShader(&shader);

  if (!program.link(messages)) {
    throw std::runtime_error(shader_code + "\n" + shader.getInfoLog());
  }

  std::vector<uint32_t> shader_binary;
  glslang::GlslangToSpv(*program.getIntermediate(shader_stage), shader_binary);
  glslang::FinalizeProcess();

  return shader_binary;
}

std::vector<uint32_t> pandora::core::io::shader::readText(const std::string& file_path) {
  const auto& [shader_type, stage] = translate_shader_stage(file_path);

  std::stringstream shader_code{};
  std::ifstream input_file(file_path);
  shader_code << input_file.rdbuf();
  input_file.close();

  return compile_shader(stage, shader_code.str());
}

std::vector<uint32_t> pandora::core::io::shader::readBinary(const std::string& file_path) {
  std::ifstream file(file_path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    std::println("Failed to open file: {}", file_path);

    return std::vector<uint32_t>();
  }

  size_t file_size = (size_t)file.tellg();
  std::vector<uint32_t> shader_binary(file_size / sizeof(uint32_t));

  file.seekg(0);
  file.read(reinterpret_cast<char*>(shader_binary.data()), file_size);

  return shader_binary;
}

std::vector<uint32_t> pandora::core::io::shader::read(const std::string& file_path) {
  if (file_path.ends_with(".spv")) {
    return readBinary(file_path);
  } else {
    return readText(file_path);
  }
}

void pandora::core::io::shader::write(const std::string& file_path, const std::vector<uint32_t>& shader_binary) {
  std::ofstream output_file(file_path, std::ios::binary);

  output_file.write(reinterpret_cast<const char*>(shader_binary.data()), shader_binary.size() * sizeof(uint32_t));
}
