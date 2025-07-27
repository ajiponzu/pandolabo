cmake_minimum_required(VERSION 3.16)

# プラットフォーム別コンパイラー設定
if(WIN32)
    # Windows: MSVCを強制的に使用
    message(STATUS "Windows detected - forcing MSVC compiler")
    # Clang設定をスキップしてMSVCを使用
else()
    # Linux/macOS: Clangコンパイラを優先設定
    find_program(CLANG_CXX_COMPILER NAMES clang++ clang++-18 clang++-17 clang++-16 clang++-15)
    if(CLANG_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER ${CLANG_CXX_COMPILER})
        message(STATUS "Using Clang C++ compiler: ${CLANG_CXX_COMPILER}")
    else()
        message(WARNING "Clang not found on non-Windows platform, falling back to default compiler")
    endif()

    find_program(CLANG_C_COMPILER NAMES clang clang-18 clang-17 clang-16 clang-15)
    if(CLANG_C_COMPILER)
        set(CMAKE_C_COMPILER ${CLANG_C_COMPILER})
        message(STATUS "Using Clang C compiler: ${CLANG_C_COMPILER}")
    else()
        message(STATUS "Clang not found, falling back to default C compiler")
    endif()
endif()

project(pandolabo VERSION 1.0.0 LANGUAGES CXX)

# プラットフォーム別C++標準設定
if(WIN32)
    # Windows: MSVC + C++20 (安定版)
    if(NOT MSVC)
        message(FATAL_ERROR "Windows builds require MSVC compiler. Please use Visual Studio or MSVC tools.")
    endif()
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    message(STATUS "Using MSVC with C++20 on Windows")
else()
    # Linux/macOS: Clang + 最新C++バージョン
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # C++標準をユーザーが指定可能にする（デフォルトは26）
        set(PANDOLABO_CXX_STANDARD "26" CACHE STRING "C++ standard version (20, 23, 26)")
        set_property(CACHE PANDOLABO_CXX_STANDARD PROPERTY STRINGS "20" "23" "26")

        # C++標準の設定（最低C++20、それ以上も可能）
        set(CMAKE_CXX_STANDARD ${PANDOLABO_CXX_STANDARD})
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
        message(STATUS "Using Clang with C++${PANDOLABO_CXX_STANDARD} on ${CMAKE_SYSTEM_NAME}")
    else()
        message(WARNING "Non-Windows platforms should preferably use Clang")
        set(CMAKE_CXX_STANDARD 20)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
endif()

# コンパイラ設定
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# デバッグビルドでのシンボル情報
set(CMAKE_DEBUG_POSTFIX "d")

# VSCode IntelliSense用のコンパイルコマンドを生成（Clangの場合のみ）
if(NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Enabling compile_commands.json generation for Clang on non-Windows platform")
else()
    message(STATUS "Skipping compile_commands.json generation (MSVC on Windows uses different IntelliSense method)")
endif()

# Conan統合の設定
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/conan_toolchain.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/build/conan_toolchain.cmake)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/conan_deps.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/build/conan_deps.cmake)
endif()

# パッケージの検索
find_package(glslang REQUIRED)
find_package(spirv-cross REQUIRED)
find_package(SPIRV-Tools REQUIRED)
find_package(SPIRV-Headers REQUIRED)
find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(glfw3 REQUIRED)

# テストオプション
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# インクルードディレクトリ
include_directories(include)

# ソースファイルの取得
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# 静的ライブラリの作成
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# インクルードディレクトリの公開
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# C++標準の設定をターゲットに適用（最低C++20、自動検出でC++26も可能）
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Vulkan.hpp動的ディスパッチローダー設定
target_compile_definitions(${PROJECT_NAME} PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

# 外部ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glslang::_glslang-do-not-use
        glslang::glslang
        glslang::SPIRV
        glslang::glslang-default-resource-limits
        glslang::SPVRemapper
        glslang::MachineIndependent
        glslang::OSDependent
        glslang::GenericCodeGen
        spirv-cross::spirv-cross
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-reflect
        spirv-tools::spirv-tools
        SPIRV-Headers::SPIRV-Headers
        Vulkan::Loader
        vulkan-headers::vulkan-headers
        nlohmann_json::nlohmann_json
        glfw
)

# Conanパッケージのインクルードディレクトリを手動で追加
# CMakeDepsが正しく動作しない場合の対策
target_include_directories(${PROJECT_NAME} PUBLIC
    "C:/Users/cpjvm/.conan2/p/vulkae153e7dd6b61a/p/include"
    "C:/Users/cpjvm/.conan2/p/b/spirv51f92ea85c2f6/p/include"
    "C:/Users/cpjvm/.conan2/p/b/glsla3085496846f77/b/src"
)

# Conanキャッシュから動的にパスを探す
file(GLOB CONAN_VULKAN_HEADERS "C:/Users/cpjvm/.conan2/p/vulka*/p/include")
file(GLOB CONAN_SPIRV_CROSS "C:/Users/cpjvm/.conan2/p/b/spirv*/p/include")
file(GLOB CONAN_GLSLANG "C:/Users/cpjvm/.conan2/p/b/glsla*/b/src")

foreach(VULKAN_INCLUDE ${CONAN_VULKAN_HEADERS})
    if(EXISTS "${VULKAN_INCLUDE}/vulkan/vulkan.hpp")
        target_include_directories(${PROJECT_NAME} PUBLIC ${VULKAN_INCLUDE})
        message(STATUS "Found Vulkan headers: ${VULKAN_INCLUDE}")
    endif()
endforeach()

foreach(SPIRV_INCLUDE ${CONAN_SPIRV_CROSS})
    if(EXISTS "${SPIRV_INCLUDE}/spirv_cross/spirv_cross.hpp")
        target_include_directories(${PROJECT_NAME} PUBLIC ${SPIRV_INCLUDE})
        message(STATUS "Found SPIRV-Cross headers: ${SPIRV_INCLUDE}")
    endif()
endforeach()

foreach(GLSLANG_INCLUDE ${CONAN_GLSLANG})
    if(EXISTS "${GLSLANG_INCLUDE}/glslang/Public/ShaderLang.h")
        target_include_directories(${PROJECT_NAME} PUBLIC ${GLSLANG_INCLUDE})
        message(STATUS "Found glslang headers: ${GLSLANG_INCLUDE}")
    endif()
endforeach()

# Glslangの追加パスを探す
file(GLOB CONAN_GLSLANG_INCLUDES "C:/Users/cpjvm/.conan2/p/*/p/include")
foreach(GLSLANG_INCLUDE ${CONAN_GLSLANG_INCLUDES})
    if(EXISTS "${GLSLANG_INCLUDE}/glslang/SPIRV/GlslangToSpv.h")
        target_include_directories(${PROJECT_NAME} PUBLIC ${GLSLANG_INCLUDE})
        message(STATUS "Found glslang SPIRV headers: ${GLSLANG_INCLUDE}")
    endif()
endforeach()

# Catch2のパスを探す
file(GLOB CONAN_CATCH2_INCLUDES "C:/Users/cpjvm/.conan2/p/*/p/include")
foreach(CATCH2_INCLUDE ${CONAN_CATCH2_INCLUDES})
    if(EXISTS "${CATCH2_INCLUDE}/catch2/catch_test_macros.hpp")
        target_include_directories(${PROJECT_NAME} PUBLIC ${CATCH2_INCLUDE})
        message(STATUS "Found Catch2 headers: ${CATCH2_INCLUDE}")
    endif()
endforeach()

# Conanパッケージのインクルードディレクトリを取得
# CMakeDepsが正しく動作しない場合のフォールバック
if(NOT TARGET vulkan-headers::vulkan-headers OR NOT TARGET glfw)
    # Conanホームディレクトリを動的に取得
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env .venv/Scripts/conan config home
        OUTPUT_VARIABLE CONAN_HOME
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    if(CONAN_HOME)
        file(GLOB CONAN_PACKAGES "${CONAN_HOME}/p/*/p/include")
        file(GLOB CONAN_BUILD_PACKAGES "${CONAN_HOME}/p/*/s/*/include")

        # パッケージディレクトリからヘッダを探索
        foreach(PACKAGE_INCLUDE ${CONAN_PACKAGES})
            if(EXISTS "${PACKAGE_INCLUDE}/vulkan/vulkan.hpp")
                target_include_directories(${PROJECT_NAME} PUBLIC ${PACKAGE_INCLUDE})
                message(STATUS "Found Vulkan headers: ${PACKAGE_INCLUDE}")
            endif()
            if(EXISTS "${PACKAGE_INCLUDE}/GLFW/glfw3.h")
                target_include_directories(${PROJECT_NAME} PUBLIC ${PACKAGE_INCLUDE})
                message(STATUS "Found GLFW headers: ${PACKAGE_INCLUDE}")
            endif()
        endforeach()

        # ビルドソースディレクトリからも探索
        foreach(PACKAGE_INCLUDE ${CONAN_BUILD_PACKAGES})
            if(EXISTS "${PACKAGE_INCLUDE}/GLFW/glfw3.h")
                target_include_directories(${PROJECT_NAME} PUBLIC ${PACKAGE_INCLUDE})
                message(STATUS "Found GLFW headers (build source): ${PACKAGE_INCLUDE}")
            endif()
        endforeach()
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# インストール設定
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# パッケージ設定ファイルの生成
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
)

# CPack設定（パッケージ配布用）
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ Static Library")
set(CPACK_PACKAGE_VENDOR "Your Company")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Windows用設定
set(CPACK_GENERATOR "ZIP;NSIS")
set(CPACK_NSIS_DISPLAY_NAME ${PROJECT_NAME})

# Linux用設定
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
