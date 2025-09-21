# テスト用CMakeLists.txt

# Catch2パッケージを検索
find_package(Catch2 REQUIRED)

# テストファイルを収集
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# テスト実行ファイルを作成
add_executable(tests ${TEST_SOURCES})

# WindowsでC++23を使用する場合、特別な設定
if(DEFINED PANDOLABO_CXX_STANDARD AND PANDOLABO_CXX_STANDARD STREQUAL "23")
    # 明示的にコンパイラフラグのみ使用（CXX_STANDARDプロパティは設定しない）
    target_compile_options(tests PRIVATE /std:c++23preview)
    message(STATUS "Applied C++23 preview settings to tests target")
else()
    # C++20標準を設定
    target_compile_features(tests PRIVATE cxx_std_20)
endif()

# 最適化設定（MSVC）
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
    target_compile_options(tests PRIVATE
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od>
    )
    message(STATUS "Building tests with MSVC optimization")
endif()

# Catch2ライブラリをリンク
target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    ${PROJECT_NAME}  # メインライブラリもリンク
)

# インクルードディレクトリを追加
target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# テスト登録
include(CTest)
if(TARGET Catch2::Catch2WithMain)
    # Catch2の自動テスト発見を使用
    if(COMMAND catch_discover_tests)
        catch_discover_tests(tests)
    else()
        add_test(NAME unit_tests COMMAND tests)
    endif()
endif()
