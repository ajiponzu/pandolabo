# テスト用CMakeLists.txt

# Catch2パッケージを検索
find_package(Catch2 REQUIRED)

# テストファイルを収集
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# テスト実行ファイルを作成
add_executable(tests ${TEST_SOURCES})

# C++20標準を設定
target_compile_features(tests PRIVATE cxx_std_20)

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
