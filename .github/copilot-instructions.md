# Pandolabo - GitHub Copilot Instructions

This file provides guidance for GitHub Copilot when working with the Pandolabo project.

## 補足

- ちなみに，回答は日本語でお願いします．ただし，コード内のコメントは英語で記述してください
- std::cout 系ではなく，std::print 系を使用してください．
- 入出力系含め，できる限り新しい c++の機能を使用してください
- 実装の前に，必ず方針を説明し同意を求めてください

## Project Overview

**Pandolabo** is a modern C++23 Vulkan graphics library designed for Windows development with MSVC. This is a personal learning project with a focus on modern C++ techniques and Vulkan graphics programming.

### Key Technologies

- **Language**: C++23 (MSVC with `/std:c++23preview`)
- **Graphics API**: Vulkan 1.4+ with Vulkan.hpp
- **Platform**: Windows-only (Win10/Win11)
- **Compiler**: MSVC (Visual Studio 2022)
- **Build System**: CMake 3.16+ with Conan 2.x
- **Development Environment**: VS Code (primary)

### Dependencies

- **Vulkan.hpp**: Modern C++ Vulkan bindings
- **GLFW**: Window management and input
- **glslang**: Shader compilation
- **SPIRV-Tools/SPIRV-Cross**: Shader processing
- **nlohmann/json**: JSON handling
- **Catch2**: Unit testing framework

## Project Structure

```
pandolabo/
├── include/pandolabo.hpp          # Main public header
├── include/pandora/               # Core library headers
│   ├── core.hpp                  # Unified core header
│   ├── core/error.hpp            # Error handling system
│   └── core/                     # Modular components
│       ├── gpu/                  # GPU management (Context, Device, etc.)
│       ├── io/                   # I/O utilities
│       └── ui/                   # Window and input management
├── src/pandora/core/             # Implementation files
├── examples/                     # Usage examples and samples (or integration tests)
├── tests/                        # Unit tests
├── scripts/build.ps1             # Build automation script
└── docs/                         # Documentation
```

## Coding Standards

### C++ Style Guidelines

1. **Modern C++23 Features**

   - Use `std::format` for string formatting
   - Prefer `std::source_location` for debugging
   - Use concepts and ranges where appropriate
   - Leverage `std::expected` patterns (custom Result<T> implementation)

2. **Naming Conventions**

   ```cpp
   // Namespaces: lowercase with underscores
   namespace pandora::core::gpu {}

   // Classes: PascalCase
   class GpuContext {}

   // Functions/Variables: camelCase
   void initializeRenderer() {}

   // Constants: UPPER_SNAKE_CASE
   const int MAX_BUFFER_SIZE = 1024;

   // Private members: m_ prefix
   class Example {
   private:
       int m_memberVariable;
       std::unique_ptr<Device> m_ptrDevice;

      // camelCase method
       void run();
   };
   ```

3. **File Organization**

   ```cpp
   // Header files (.hpp)
   #pragma once

   #include <vulkan/vulkan.hpp>

   #include "pandora/core/other_headers.hpp"

   namespace pandora::core {
   // Content
   }
   ```

### Error Handling

This project implements a Golang-inspired error handling system:

```cpp
// Use Result<T> for operations that can fail
Result<std::unique_ptr<Context>> createContext() {
    try {
        auto context = std::make_unique<Context>();
        if (!context->isInitialized()) {
            return PANDORA_ERROR_CONFIG("Context initialization failed");
        }
        return std::move(context);
    } catch (const std::exception& e) {
        return PANDORA_ERROR_RUNTIME(e.what()).wrap("Context creation failed");
    }
}

// Error propagation with PANDORA_TRY macro
VoidResult initializeSystem() {
    PANDORA_TRY(setupVulkan().wrapError("Vulkan setup failed"));
    PANDORA_TRY(createWindow().wrapError("Window creation failed"));
    return std::monostate{};
}

// Behavioral error checking
void handleError(const Error& error) {
    if (error.isTemporary()) {
        // Retry logic
    } else if (error.isCritical()) {
        // Emergency shutdown
    }
}
```

### GPU Programming Patterns

1. **RAII Resource Management**

   ```cpp
   class Buffer {
   private:
       vk::UniqueBuffer m_buffer;
       vk::UniqueDeviceMemory m_memory;
   public:
       // Automatic cleanup through RAII
   };
   ```

2. **Vulkan.hpp Usage**

   ```cpp
   // Prefer Vulkan.hpp over C API
   vk::CommandBufferBeginInfo beginInfo{};
   commandBuffer.begin(beginInfo);

   // Use unique handles for automatic cleanup
   vk::UniqueInstance instance = vk::createInstanceUnique(createInfo);
   ```

3. **Graphics Pipeline Setup**
   ```cpp
   // Use builder patterns for complex objects
   auto pipeline = PipelineBuilder()
       .setShaders(vertexShader, fragmentShader)
       .setViewport(viewport)
       .setRenderPass(renderPass)
       .build();
   ```

## Development Workflow

### Build Commands

```powershell
# Setup environment
.\scripts\build.ps1 setup

# Build project
.\scripts\build.ps1 build -Configuration Debug
.\scripts\build.ps1 build -Configuration Release

# Run tests
.\scripts\build.ps1 test

# Run examples
.\scripts\build.ps1 examples
.\scripts\build.ps1 run -Example basic_cube

# Format code
.\scripts\build.ps1 format

# Generate VS Code configuration
.\scripts\build.ps1 vscode -Configuration Debug
```

### Testing Guidelines

1. **Use Catch2 for unit tests**

   ```cpp
   #include <catch2/catch_test_macros.hpp>
   #include <pandolabo.hpp>

   TEST_CASE("Context creation", "[gpu][context]") {
       auto result = pandora::core::gpu::createGpuContext();
       REQUIRE(result.isOk());
   }
   ```

2. **GPU tests are optional**
   - Set `PANDOLABO_ENABLE_GPU_TESTS=1` to enable GPU-dependent tests
   - Default tests should run without GPU

### Documentation

1. **Doxygen-style comments**

   ```cpp
   /// @brief Create a new GPU context
   /// @param windowSurface Optional window surface for presentation
   /// @return Result containing Context or Error
   [[nodiscard]] Result<std::unique_ptr<Context>> createContext(
       std::shared_ptr<WindowSurface> windowSurface = nullptr);
   ```

2. **Include usage examples in headers**
   ```cpp
   /**
    * @example
    * auto context = createContext();
    * if (context.isOk()) {
    *     // Use context
    * }
    */
   ```

## Common Patterns to Generate

### 1. GPU Resource Classes

```cpp
class GpuResource {
private:
    vk::UniqueHandle m_handle;
    std::shared_ptr<Device> m_device;

public:
    explicit GpuResource(std::shared_ptr<Device> device);
    ~GpuResource() = default;

    // Rule of Five
    GpuResource(const GpuResource&) = delete;
    GpuResource& operator=(const GpuResource&) = delete;
    GpuResource(GpuResource&&) = default;
    GpuResource& operator=(GpuResource&&) = default;

    [[nodiscard]] const auto& getHandle() const { return m_handle; }
};
```

### 2. Error-Safe Operations

```cpp
Result<BufferData> loadBufferData(const std::string& filename) {
    auto fileResult = readFile(filename);
    PANDORA_TRY(fileResult.wrapError("Failed to read buffer file"));

    auto parseResult = parseBufferFormat(fileResult.value());
    return parseResult.wrapError("Failed to parse buffer data");
}
```

### 3. Builder Patterns

```cpp
class PipelineBuilder {
private:
    PipelineCreateInfo m_createInfo;

public:
    PipelineBuilder& setVertexShader(const Shader& shader);
    PipelineBuilder& setFragmentShader(const Shader& shader);
    PipelineBuilder& setViewport(const vk::Viewport& viewport);

    [[nodiscard]] Result<std::unique_ptr<Pipeline>> build();
};
```

## Project-Specific Considerations

1. **Windows-Only Development**

   - Use Windows-specific paths with backslashes when needed
   - MSVC-specific compiler flags and features are acceptable
   - PowerShell scripts for automation

2. **Personal Learning Project**

   - Code quality over performance optimization
   - Educational comments are welcome
   - Experimental C++23 features encouraged

3. **Self-Documentation Focus**

   - Include reasoning for design decisions
   - Add TODO comments for future improvements
   - Link to relevant Vulkan specification sections

4. **GitHub Copilot Assistance**
   - This project welcomes AI-assisted development
   - Document AI-generated sections when significant
   - Focus on learning modern C++ patterns

## Debugging and Development

### VS Code Integration

- Use generated `.vscode/` configuration files
- Debug configurations for both Debug and Release builds
- Integrated task runner for build commands

### Common Debug Scenarios

1. **Vulkan Validation Layers**: Enabled in Debug builds with `GPU_DEBUG`
2. **Memory Leaks**: RAII patterns should prevent most issues
3. **GPU Errors**: Check Vulkan validation layer output

### Performance Considerations

- Profile GPU operations with tools like RenderDoc
- Use Release builds for performance testing
- Memory allocation patterns matter for GPU resources

---

_This file is maintained as part of the Pandolabo project documentation. Update it when significant architectural changes are made._
