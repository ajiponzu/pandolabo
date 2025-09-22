/*
 * core.hpp - Pandolabo core module unified header
 *
 * This header provides a unified interface to all core rendering functionality
 * by including all modular core components.
 */

#pragma once

#include "pandora/core/gpu.hpp"
#include "pandora/core/io.hpp"
#include "pandora/core/ui.hpp"

// Core rendering modules
#include "pandora/core/buffer_helpers.hpp"
#include "pandora/core/command_buffer.hpp"
#include "pandora/core/pipeline.hpp"
#include "pandora/core/rendering_structures.hpp"
#include "pandora/core/rendering_types.hpp"
#include "pandora/core/renderpass.hpp"
#include "pandora/core/synchronization.hpp"
