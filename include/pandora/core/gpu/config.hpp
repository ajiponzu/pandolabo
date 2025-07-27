/*
 * config.hpp - GPU module configuration header
 *
 * This header defines build configuration macros for the GPU module.
 * This file should be included by all GPU headers that need configuration.
 */

#pragma once

#ifdef _DEBUG
  #define GPU_DEBUG
#endif

// GPU module version constants
constexpr auto PANDOLABO_VK_VERSION = VK_MAKE_VERSION(1, 4, 0);
constexpr auto PANDOLABO_VK_API_VERSION = VK_MAKE_API_VERSION(0, 1, 4, 0);
