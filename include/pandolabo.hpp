/*
 * pandolabo.hpp - Main entry point for Pandolabo Vulkan C++ wrapper library
 *
 * This is the primary header file for the Pandolabo library, providing a modern
 * C++ wrapper around Vulkan for graphics and compute applications.
 *
 * Features:
 * - Type-safe Vulkan API wrappers with RAII resource management
 * - High-level abstractions for common graphics operations
 * - GPU buffer and image management with automatic memory handling
 * - Render pass and pipeline configuration utilities
 * - Cross-platform window and input management via GLFW
 * - Compute shader support for GPGPU workloads
 *
 * Usage:
 *   #include <pandolabo.hpp>
 *
 *   // All Pandolabo functionality is available through the pandora namespace
 *   using namespace pandora::core;
 *
 * @author Pandolabo Development Team
 * @version 1.0.0
 */

#pragma once

// Include the core Vulkan wrapper functionality
#include "pandora/core.hpp"

/// @brief Main namespace for all Pandolabo functionality
/// Contains nested namespaces for different modules:
/// - core: Core Vulkan wrapper functionality
/// - core::gpu: Low-level GPU resource management
/// - core::ui: Window and input management
namespace pandora {}  // namespace pandora
