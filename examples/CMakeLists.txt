cmake_minimum_required(VERSION 3.16)

# 親プロジェクトで定義された変数を使用
if(NOT DEFINED PROJECT_NAME)
    project(pandolabo_examples)

    # ライブラリを探す
    find_package(pandolabo REQUIRED)
else()
    # 親プロジェクトの一部として実行される場合
    set(LIB_TARGET ${PROJECT_NAME})
endif()

# 例のターゲット作成
# すべてのC++ソースファイルを自動検出
file(GLOB_RECURSE EXAMPLE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# すべてのヘッダーファイルを自動検出
file(GLOB_RECURSE EXAMPLE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# デバッグ: 検出されたファイルを表示
message(STATUS "検出されたソースファイル:")
foreach(source ${EXAMPLE_SOURCES})
    message(STATUS "  ${source}")
endforeach()

message(STATUS "検出されたヘッダーファイル:")
foreach(header ${EXAMPLE_HEADERS})
    message(STATUS "  ${header}")
endforeach()

add_executable(basic_usage ${EXAMPLE_SOURCES} ${EXAMPLE_HEADERS})

# WindowsでC++23を使用する場合、特別な設定
if(WIN32 AND DEFINED PANDOLABO_CXX_STANDARD AND PANDOLABO_CXX_STANDARD STREQUAL "23")
    # 明示的にコンパイラフラグのみ使用（CXX_STANDARDプロパティは設定しない）
    target_compile_options(basic_usage PRIVATE /std:c++23preview)
    message(STATUS "Applied C++23 preview settings to basic_usage target")
endif()

# プラットフォーム別設定
if(WIN32)
    # Windows: MSVC標準設定（追加設定不要）
    message(STATUS "Building example with MSVC on Windows")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Linux/macOS: Clang最適化設定
    target_compile_options(basic_usage PRIVATE -O2 -Wall -Wextra)
    message(STATUS "Building example with Clang on ${CMAKE_SYSTEM_NAME}")
endif()

# ライブラリとのリンク
if(DEFINED LIB_TARGET)
    # 親プロジェクトの一部として実行される場合
    target_link_libraries(basic_usage PRIVATE ${LIB_TARGET})
    target_include_directories(basic_usage PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    # 独立して実行される場合
    target_link_libraries(basic_usage PRIVATE pandolabo::pandolabo)
    target_include_directories(basic_usage PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
