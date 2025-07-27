/*
 * io.hpp - Input/output operations for Pandolabo core module
 *
 * This header provides file I/O functionality for asset management including
 * shader code reading, compilation, and SPIR-V binary handling.
 */

#include <string>
#include <vector>

#pragma once

/// @brief Shader input/output operations namespace
/// Provides functionality for reading, compiling, and writing shader code.
/// Supports both GLSL source files and pre-compiled SPIR-V binaries.
namespace pandora::core::io::shader {

/// @brief Read and compile GLSL shader source to SPIR-V binary
/// Reads GLSL source code from file and compiles it to SPIR-V binary format.
/// Supports various shader types (.vert, .frag, .comp, etc.)
/// @param file_path Path to GLSL source file
/// @return SPIR-V binary data as vector of 32-bit words
std::vector<uint32_t> readText(const std::string& file_path);

/// @brief Read pre-compiled SPIR-V binary from file
/// Loads SPIR-V binary data directly from a .spv file without compilation.
/// @param file_path Path to SPIR-V binary file (.spv)
/// @return SPIR-V binary data as vector of 32-bit words
std::vector<uint32_t> readBinary(const std::string& file_path);

/// @brief Read shader with automatic format detection
/// Automatically detects file format based on extension and reads accordingly:
/// - .spv files: Read as pre-compiled SPIR-V binary
/// - .vert, .frag, .comp, etc.: Compile GLSL source to SPIR-V
/// @param file_path Path to shader file with appropriate extension
/// @return SPIR-V binary data as vector of 32-bit words
std::vector<uint32_t> read(const std::string& file_path);

/// @brief Write SPIR-V binary to file
/// Saves SPIR-V binary data to a file for later use or distribution.
/// @param file_path Output file path for the binary
/// @param shader_binary SPIR-V binary data to write
void write(const std::string& file_path, const std::vector<uint32_t>& shader_binary);

}  // namespace pandora::core::io::shader
