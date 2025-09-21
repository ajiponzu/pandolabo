cmake_minimum_required(VERSION 3.16)

if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only. Please build on Windows with MSVC.")
endif()

project(pandolabo VERSION 1.0.0 LANGUAGES CXX)

# Windows: MSVC + C++標準バージョン
if(NOT MSVC)
    message(FATAL_ERROR "Windows builds require MSVC compiler. Please use Visual Studio or MSVC tools.")
endif()

# C++標準をユーザーが指定可能にする（デフォルトは23）
set(PANDOLABO_CXX_STANDARD "23" CACHE STRING "C++ standard version (20, 23)")
set_property(CACHE PANDOLABO_CXX_STANDARD PROPERTY STRINGS "20" "23")

# C++23プレビューサポート
if(PANDOLABO_CXX_STANDARD STREQUAL "23")
    add_compile_options(/std:c++23preview)
    message(STATUS "Using MSVC with C++23 preview on Windows")
else()
    set(CMAKE_CXX_STANDARD ${PANDOLABO_CXX_STANDARD})
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    message(STATUS "Using MSVC with C++${PANDOLABO_CXX_STANDARD} on Windows")
endif()

# コンパイラ設定（MSVC）
add_compile_options(/W4)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# リリース/デバッグ最適化設定
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /RTC1")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /RTC1")
message(STATUS "MSVC optimization flags configured - Release: /O2 /Ob2, Debug: /Od")

# デバッグビルドでのシンボル情報
set(CMAKE_DEBUG_POSTFIX "d")

# デフォルトビルドタイプの設定
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
    message(STATUS "Setting default build type to Release")
endif()

# compile_commands.json はWindowsでは生成しない
message(STATUS "Windows/MSVC build: skipping compile_commands.json generation")

# Conan統合の設定 - toolchainで指定された場合は自動インクルードをスキップ
if(NOT CMAKE_TOOLCHAIN_FILE OR NOT CMAKE_TOOLCHAIN_FILE MATCHES "conan_toolchain")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conan/build/generators/conan_toolchain.cmake")
        include(${CMAKE_CURRENT_SOURCE_DIR}/conan/build/generators/conan_toolchain.cmake)
        message(STATUS "Using default Conan toolchain (Release)")
    endif()
else()
    message(STATUS "Using external Conan toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/conan_deps.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/build/conan_deps.cmake)
endif()

# WindowsでC++23を使用する場合、ConanのCMAKE_CXX_STANDARD設定をオーバーライド
if(WIN32 AND PANDOLABO_CXX_STANDARD STREQUAL "23")
    # ConanのCMAKE_CXX_STANDARD 23設定をクリアしてstdcpplatest防止
    unset(CMAKE_CXX_STANDARD)
    unset(CMAKE_CXX_STANDARD_REQUIRED)
    unset(CMAKE_CXX_EXTENSIONS)
    message(STATUS "Overriding Conan C++ standard setting for C++23 preview")
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
# ライブラリターゲットの追加
add_library(${PROJECT_NAME} ${SOURCES})

# WindowsでC++23を使用する場合、特別な設定
if(WIN32 AND PANDOLABO_CXX_STANDARD STREQUAL "23")
    # ターゲットのC++標準設定をクリアして、明示的にコンパイラフラグのみ使用
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++23preview)
    message(STATUS "Applied C++23 preview settings to ${PROJECT_NAME} target")
else()
    # 他の場合は通常のCMAKE_CXX_STANDARD設定を使用
    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD ${PANDOLABO_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    # C++標準の設定をターゲットに適用（最低C++20、自動検出でC++26も可能）
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
endif()

# インクルードディレクトリの公開
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Vulkan.hpp動的ディスパッチローダー設定
target_compile_definitions(${PROJECT_NAME} PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

# 外部ライブラリのリンク
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        # 公開ヘッダや実行時に利用者側へ伝播すべき最低限の依存
        Vulkan::Loader
        Vulkan::Headers
        vulkan-headers::vulkan-headers
        glfw
    PRIVATE
        # 内部実装でのみ利用する依存（ヘッダ公開は不要）
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
        nlohmann_json::nlohmann_json
)

## Conan/CMakeDeps/Toolchain に一本化
# 依存のインクルードパスはCMakeDepsのパッケージターゲット経由で伝播させる。
# ユーザー環境に依存するハードコードされたinclude検索は削除。

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
