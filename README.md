# Spectrum Analyzer

一个高性能的音频频谱分析工具，支持生成频谱图并标注音高。支持批量处理音频文件。

## 功能特点

- 生成高质量的音频频谱图
- 支持多种音频格式（WAV, FLAC, OGG等）
- 可自定义频率范围（支持音符表示，如C4、A#3等）
- 支持时间段选择
- 可调整采样频率
- 自动标注音高刻度
- 支持白键音符标注（C3-C5）
- 支持批量处理整个文件夹的音频文件

## 安装

### 使用 Homebrew 安装（推荐）

```bash
brew tap humgic/spectrum
brew install spectrum
```

### 从源码编译

#### 依赖项

- CMake (>= 3.10)
- OpenCV
- FFTW3
- libsndfile
- Google Test (仅用于测试)

在 Ubuntu/Debian 上安装依赖：

```bash
sudo apt-get update
sudo apt-get install -y \
    cmake \
    libopencv-dev \
    libfftw3-dev \
    libsndfile1-dev \
    libgtest-dev
```

在 macOS 上安装依赖：

```bash
brew install \
    cmake \
    opencv \
    fftw \
    libsndfile \
    googletest
```

#### 编译安装

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## 使用方法

程序提供两个命令名：`spectrum_analyzer`（完整名称）和 `msa`（简短别名）。

### 基本用法

1. 处理单个文件：
```bash
msa <输入音频文件> <输出图像文件> [选项]
```

2. 批量处理文件夹：
```bash
msa <输入文件夹> <输出文件夹> [选项]
```

### 命令行选项

- `-h` 显示帮助信息
- `-b <秒>` 开始时间（默认：0.0秒）
- `-e <秒>` 结束时间
- `-d <秒>` 持续时间（默认：直到结束）
- `-s <n>` 每秒采样次数（默认：100）
- `-l <音符>` 最低音符（默认：20Hz，人耳可听最低频率）
- `-u <音符>` 最高音符（默认：20kHz，人耳可听最高频率）

注意：
1. 开始时间、结束时间、持续时间中只能指定其中两个
2. 当输入为文件夹时，将处理文件夹中所有支持的音频文件
3. 支持的音频格式：WAV、FLAC、OGG 等

### 音符格式

支持标准音符表示法：
- 基本音符：C、D、E、F、G、A、B
- 升降号：# 或 b（例如：C#4、Gb5）
- 八度数字：0-8

例如：
- C4：中央C（261.63 Hz）
- A4：标准音高（440 Hz）
- C#5：升C5（554.37 Hz）

### 使用示例

1. 基本使用：
```bash
msa input.flac output.png
```

2. 指定时间段：
```bash
msa input.flac output.png -b 1.5 -e 6.5
```

3. 指定频率范围：
```bash
msa input.flac output.png -l C3 -u C6
```

4. 批量处理整个文件夹：
```bash
msa input_folder/ output_folder/ -s 150 -l C3 -u C6
```

5. 组合使用：
```bash
msa input_folder/ output_folder/ -b 1.5 -d 5.0 -s 150 -l C3 -u C6
```

## 开发

### 运行测试

```bash
cd build
cmake ..
make -j$(nproc)
./spectrum_test
```

## 版本历史

### v3.0.0
- 添加批量处理功能
- 支持文件夹输入输出
- 改进错误处理
- 优化内存使用
- 升级到 C++17
- 改进代码结构

### v2.0.0
- 添加音符表示法支持
- 添加时间段选择功能
- 添加采样频率调整
- 改进频率范围设置
- 添加单元测试
- 添加简短命令别名 (msa)
- 优化代码结构

### v1.0.0
- 初始版本
- 基本频谱图生成功能
- 支持多种音频格式

## 许可证

MIT License 