cmake_minimum_required(VERSION 3.16)

# 親プロジェクトで定義された変数を使用
if(NOT DEFINED PROJECT_NAME)
    project(pandolabo_examples)
    find_package(pandolabo REQUIRED)
    set(LIB_TARGET pandolabo::pandolabo)
else()
    set(LIB_TARGET ${PROJECT_NAME})
endif()

# glm is used only by examples (e.g., BasicCube)
find_package(glm REQUIRED)

function(pdl_add_example name)
    cmake_parse_arguments(PDL "" "" "SOURCES;INCLUDES" ${ARGN})
    add_executable(${name} ${PDL_SOURCES})

    if(DEFINED PANDOLABO_CXX_STANDARD AND PANDOLABO_CXX_STANDARD STREQUAL "23")
        target_compile_options(${name} PRIVATE /std:c++23preview)
    else()
        target_compile_features(${name} PRIVATE cxx_std_20)
    endif()

    target_compile_options(${name} PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od>
    )

    target_link_libraries(${name} PRIVATE ${LIB_TARGET} glm::glm)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${PDL_INCLUDES})
endfunction()

# すべての個別Exampleをまとめる仮想ターゲット
add_custom_target(examples_all)

# 個別実行ターゲット
pdl_add_example(example_basic_compute
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/basic_compute/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/basic_compute/basic_computing.cpp
)
add_dependencies(examples_all example_basic_compute)

pdl_add_example(example_hl_basic_compute
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/basic_compute/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/basic_compute/basic_compute_hl.cpp
)
add_dependencies(examples_all example_hl_basic_compute)

pdl_add_example(example_basic_cube
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/basic_cube/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/basic_cube/basic_cube.cpp
)
add_dependencies(examples_all example_basic_cube)

pdl_add_example(example_hl_basic_cube
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/basic_cube/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/basic_cube/basic_cube_hl.cpp
)
add_dependencies(examples_all example_hl_basic_cube)

pdl_add_example(example_simple_image
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/computing_image/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/computing_image/simple_image_computing.cpp
)
add_dependencies(examples_all example_simple_image)

pdl_add_example(example_hl_simple_image
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/computing_image/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/computing_image/simple_image_computing_hl.cpp
)
add_dependencies(examples_all example_hl_simple_image)

pdl_add_example(example_square
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/square/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/square/square.cpp
)
add_dependencies(examples_all example_square)

pdl_add_example(example_hl_square
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/square/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/square/square_hl.cpp
)
add_dependencies(examples_all example_hl_square)

pdl_add_example(example_streaming_resources
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/streaming_resources/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/streaming_resources/streaming_resources.cpp
)
add_dependencies(examples_all example_streaming_resources)

pdl_add_example(example_hl_streaming_resources
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/streaming_resources/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/highlevel/streaming_resources/streaming_resources_hl.cpp
)
add_dependencies(examples_all example_hl_streaming_resources)
