cmake_minimum_required(VERSION 3.10)
project(spectrum_analyzer VERSION 3.0.0)

# 配置版本信息
configure_file(
    ${CMAKE_SOURCE_DIR}/include/version.hpp.in
    ${CMAKE_BINARY_DIR}/include/version.hpp
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS特定配置
if(APPLE)
    # 添加Homebrew安装路径
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt" "/opt/homebrew/opt")
    # 确保能找到dylib
    set(CMAKE_MACOSX_RPATH ON)
    # 添加Core Graphics框架
    find_library(CORE_GRAPHICS CoreGraphics REQUIRED)
    find_library(COCOA Cocoa REQUIRED)
endif()

# 启用测试
enable_testing()

# 查找必要的包
find_package(GTest REQUIRED)

# 查找 FFTW3
if(APPLE)
    find_library(FFTW3_LIBRARIES NAMES fftw3 PATHS /usr/local/opt/fftw/lib /opt/homebrew/opt/fftw/lib REQUIRED)
    find_path(FFTW3_INCLUDE_DIRS fftw3.h PATHS /usr/local/opt/fftw/include /opt/homebrew/opt/fftw/include REQUIRED)
else()
    find_library(FFTW3_LIBRARIES NAMES fftw3 REQUIRED)
    find_path(FFTW3_INCLUDE_DIRS fftw3.h REQUIRED)
endif()

# 查找 SndFile
if(APPLE)
    find_library(SNDFILE_LIBRARIES NAMES sndfile PATHS /usr/local/opt/libsndfile/lib /opt/homebrew/opt/libsndfile/lib REQUIRED)
    find_path(SNDFILE_INCLUDE_DIRS sndfile.h PATHS /usr/local/opt/libsndfile/include /opt/homebrew/opt/libsndfile/include REQUIRED)
else()
    find_library(SNDFILE_LIBRARIES NAMES sndfile REQUIRED)
    find_path(SNDFILE_INCLUDE_DIRS sndfile.h REQUIRED)
endif()

# 添加库目标
add_library(spectrum_lib
    src/spectrogram.cpp
    src/note_utils.cpp
)

target_include_directories(spectrum_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${FFTW3_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
)

target_link_libraries(spectrum_lib
    ${FFTW3_LIBRARIES}
    ${SNDFILE_LIBRARIES}
)

if(APPLE)
    target_link_libraries(spectrum_lib
        ${CORE_GRAPHICS}
        ${COCOA}
    )
    target_compile_definitions(spectrum_lib PRIVATE USE_CORE_GRAPHICS)
endif()

# 添加可执行文件
add_executable(spectrum_analyzer src/main.cpp)
target_include_directories(spectrum_analyzer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)
target_link_libraries(spectrum_analyzer PRIVATE spectrum_lib)

# 在macOS上链接stdc++fs库（如果需要）
if(APPLE)
    target_link_libraries(spectrum_analyzer PRIVATE stdc++fs)
endif()

# 添加测试
add_executable(spectrum_test tests/test_main.cpp)
target_link_libraries(spectrum_test PRIVATE
    spectrum_lib
    GTest::GTest
    GTest::Main
)

add_test(NAME spectrum_test COMMAND spectrum_test)

# 创建开发环境中的符号链接
add_custom_command(
    TARGET spectrum_analyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:spectrum_analyzer>
        ${CMAKE_BINARY_DIR}/msa
)

# 安装目标
install(TARGETS spectrum_analyzer
        RUNTIME DESTINATION bin)

# 创建安装环境中的符号链接
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \
        \${CMAKE_INSTALL_PREFIX}/bin/spectrum_analyzer \
        \${CMAKE_INSTALL_PREFIX}/bin/msa)") 