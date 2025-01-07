cmake_minimum_required(VERSION 3.10)
project(spectrum_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的库
find_library(FFTW3_LIBRARIES NAMES fftw3 REQUIRED)
find_path(FFTW3_INCLUDE_DIRS fftw3.h REQUIRED)

find_library(SNDFILE_LIBRARIES NAMES sndfile REQUIRED)
find_path(SNDFILE_INCLUDE_DIRS sndfile.h REQUIRED)

# OpenCV 组件
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

add_executable(spectrum_analyzer src/main.cpp src/audio_processor.cpp src/spectrogram.cpp)

target_include_directories(spectrum_analyzer PRIVATE 
    ${FFTW3_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    include
)

target_link_libraries(spectrum_analyzer PRIVATE 
    ${FFTW3_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    ${OpenCV_LIBS}
    -static-libgcc 
    -static-libstdc++
) 