cmake_minimum_required(VERSION 3.15) # より新しいバージョンを推奨
project(volumen LANGUAGES CXX)

# C++標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 良い習慣

# ビルドタイプ設定
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "ビルドタイプが指定されていないため 'Release' に設定します。")
endif()

# --- 依存関係 ---
# vcpkgを使用することを強く推奨します。
# vcpkgを使う場合、CMake実行時に CMAKE_TOOLCHAIN_FILE を指定します。
# 例: cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkgへのパス]/scripts/buildsystems/vcpkg.cmake
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED) # バージョン指定を推奨

# OpenMPを探す
find_package(OpenMP)

# --- ソースファイル ---
# GLOBは便利ですが、ファイルを追加/削除した際にCMakeの再実行が必要です。
# プロジェクトが大きくなる場合は、明示的にリストすることを検討してください。
file(GLOB_RECURSE SOURCES "src/*.cpp")

# --- 実行ファイル設定 ---
add_executable(volumen ${SOURCES})

# --- インクルードディレクトリ ---
# find_packageが見つけたインクルードパスも追加します。
target_include_directories(volumen PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GLEW_INCLUDE_DIRS}
    ${glfw3_INCLUDE_DIRS}
)

# --- コンパイルオプション ---
target_compile_features(volumen PRIVATE cxx_std_17)

# GLMの実験的機能を有効にする
target_compile_definitions(volumen PRIVATE GLM_ENABLE_EXPERIMENTAL) # <- これを追加

if(MSVC)
    # MSVC (Windows) 用のオプション
    target_compile_options(volumen PRIVATE
        /W2       # 高い警告レベル
        /EHsc     # C++ 例外処理モデル
        /utf-8    # ★★★ ソースと実行文字セットを UTF-8 として扱う ★★★
        "$<$<CONFIG:Release>:/O2>"
        "$<$<CONFIG:Debug>:/Od>"
    )
    target_compile_options(volumen PRIVATE "$<$<CONFIG:Release>:/O2>" "$<$<CONFIG:Debug>:/Od>") # ビルドタイプ毎の最適化
    if(OpenMP_FOUND)
        target_compile_options(volumen PRIVATE /openmp) # MSVC用OpenMPフラグ
    endif()
else()
    # GCC/Clang (Linux) 用のオプション
    target_compile_options(volumen PRIVATE -Wall -Wextra)
    target_compile_options(volumen PRIVATE "$<$<CONFIG:Release>:-O3>" "$<$<CONFIG:Debug>:-g>") # ビルドタイプ毎の最適化/デバッグ情報
    if(OpenMP_FOUND)
        # OpenMP::OpenMP_CXX ターゲットがあればそれを使う (推奨)
        if(TARGET OpenMP::OpenMP_CXX)
            target_link_libraries(volumen PRIVATE OpenMP::OpenMP_CXX)
        else()
            # 古いCMakeや環境ではフラグを直接使う
            target_compile_options(volumen PRIVATE ${OpenMP_CXX_FLAGS})
            target_link_libraries(volumen PRIVATE ${OpenMP_CXX_FLAGS})
        endif()
    endif()
endif()

# --- ライブラリリンク ---
target_link_libraries(volumen PRIVATE
    ${OPENGL_LIBRARIES}
    GLEW::GLEW  # GLEWのインポートされたターゲット
    glfw        # glfw3のfindモジュールは 'glfw' ターゲットを作ることが多い
)
# 注意: find_package がインポートされたターゲットを提供しない場合は、
# ${GLEW_LIBRARIES} や ${glfw3_LIBRARIES} を使う必要があるかもしれません。
# vcpkg を使えば、通常はインポートされたターゲットが提供されます。

# --- 出力ディレクトリ ---
set_target_properties(volumen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# --- Cleanターゲット ---
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMENT "すべてのターゲットをクリーンアップします"
)