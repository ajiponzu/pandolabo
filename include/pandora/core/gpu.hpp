/*
 * gpu.hpp - Pandolabo GPU module unified header
 *
 * This header provides a unified interface to all GPU functionality
 * by including all modular GPU components.
 */

#pragma once

// Core types and structures
#include "structures.hpp"
#include "types.hpp"

// GPU modules
#include "gpu/buffer.hpp"
#include "gpu/config.hpp"
#include "gpu/context.hpp"
#include "gpu/debug.hpp"
#include "gpu/descriptor.hpp"
#include "gpu/device.hpp"
#include "gpu/image.hpp"
#include "gpu/shader.hpp"
#include "gpu/swapchain.hpp"
#include "gpu/synchronization.hpp"
