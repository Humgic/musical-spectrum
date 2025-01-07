cmake_minimum_required(VERSION 3.10)
project(spectrum_analyzer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试
enable_testing()

# 查找必要的包
find_package(OpenCV REQUIRED)
find_package(GTest REQUIRED)

# 查找 FFTW3
find_library(FFTW3_LIBRARIES NAMES fftw3 REQUIRED)
find_path(FFTW3_INCLUDE_DIRS fftw3.h REQUIRED)

# 查找 SndFile
find_library(SNDFILE_LIBRARIES NAMES sndfile REQUIRED)
find_path(SNDFILE_INCLUDE_DIRS sndfile.h REQUIRED)

# 添加库目标
add_library(spectrum_lib
    src/spectrogram.cpp
    src/note_utils.cpp
)

target_include_directories(spectrum_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${FFTW3_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
)

target_link_libraries(spectrum_lib
    ${OpenCV_LIBS}
    ${FFTW3_LIBRARIES}
    ${SNDFILE_LIBRARIES}
)

# 添加可执行文件
add_executable(spectrum_analyzer src/main.cpp)
target_link_libraries(spectrum_analyzer PRIVATE spectrum_lib)

# 添加测试
add_executable(spectrum_test tests/test_main.cpp)
target_link_libraries(spectrum_test PRIVATE
    spectrum_lib
    GTest::GTest
    GTest::Main
)

add_test(NAME spectrum_test COMMAND spectrum_test)

# 安装目标
install(TARGETS spectrum_analyzer DESTINATION bin)

# 创建符号链接
add_custom_command(
    TARGET spectrum_analyzer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_FILE:spectrum_analyzer>
        ${CMAKE_BINARY_DIR}/msa
) 