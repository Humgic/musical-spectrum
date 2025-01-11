#include <filesystem>
#include <iostream>
#include <string>
#include <sndfile.h>
#include <vector>
#include <cstring>
#include <map>
#include <cmath>
#include <regex>
#include <optional>
#include <algorithm>
#include "version.hpp"
#include "spectrogram.hpp"

namespace fs = std::filesystem;

// 音符到频率的转换函数
double noteToFreq(const std::string& note) {
    static const std::map<std::string, int> noteToSemitone = {
        {"C", 0}, {"D", 2}, {"E", 4}, {"F", 5},
        {"G", 7}, {"A", 9}, {"B", 11}
    };
    
    // 使用正则表达式解析音符格式（例如：C4, D#3, Gb5）
    std::regex notePattern("([A-G])(#|b)?(\\d+)");
    std::smatch matches;
    if (!std::regex_match(note, matches, notePattern)) {
        throw std::runtime_error("无效的音符格式: " + note);
    }
    
    std::string noteName = matches[1];
    std::string accidental = matches[2];
    int octave = std::stoi(matches[3]);
    
    // 计算MIDI音符号
    int semitone = noteToSemitone.at(noteName);
    if (accidental == "#") semitone += 1;
    if (accidental == "b") semitone -= 1;
    
    // MIDI音符到频率的转换（A4 = 440Hz）
    int midiNote = (octave + 1) * 12 + semitone;
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}

void processAudioFile(const std::string& inputFile, const std::string& outputFile, const Spectrogram::Config& config) {
    std::cout << "处理文件: " << inputFile << std::endl;
    
    // 打开音频文件
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
    SNDFILE* sndFile = sf_open(inputFile.c_str(), SFM_READ, &sfInfo);
    
    if (!sndFile) {
        std::cerr << "无法打开音频文件: " << inputFile << std::endl;
        std::cerr << "错误信息: " << sf_strerror(nullptr) << std::endl;
        return;
    }
    
    std::cout << "音频信息: " << std::endl;
    std::cout << "  采样率: " << sfInfo.samplerate << " Hz" << std::endl;
    std::cout << "  声道数: " << sfInfo.channels << std::endl;
    std::cout << "  总帧数: " << sfInfo.frames << std::endl;
    
    // 读取音频数据
    std::vector<double> audioData(sfInfo.frames * sfInfo.channels);
    sf_read_double(sndFile, audioData.data(), audioData.size());
    sf_close(sndFile);
    
    // 如果是立体声，转换为单声道
    std::vector<double> monoData;
    if (sfInfo.channels == 2) {
        std::cout << "转换立体声到单声道..." << std::endl;
        monoData.resize(sfInfo.frames);
        for (sf_count_t i = 0; i < sfInfo.frames; ++i) {
            monoData[i] = (audioData[i*2] + audioData[i*2+1]) / 2.0;
        }
    } else {
        monoData = std::move(audioData);
    }
    
    // 计算频谱图
    const int fftSize = 2048;
    const int hopSize = sfInfo.samplerate / config.samples_per_sec;
    const int numFrames = (monoData.size() - fftSize) / hopSize + 1;
    
    std::cout << "生成频谱图..." << std::endl;
    std::cout << "  FFT大小: " << fftSize << std::endl;
    std::cout << "  跳跃大小: " << hopSize << std::endl;
    std::cout << "  总帧数: " << numFrames << std::endl;
    
    // 创建频谱数据
    std::vector<std::vector<double>> specData(numFrames, std::vector<double>(fftSize/2 + 1));
    
    // ... FFT计算代码 ...
    
    // 生成频谱图
    Spectrogram spectrogram;
    spectrogram.generateSpectrogram(specData, outputFile, sfInfo.samplerate, config);
    
    std::cout << "已生成频谱图: " << outputFile << std::endl;
}

void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " <输入音频文件/文件夹> <输出图像文件/文件夹> [选项]\n"
              << "选项:\n"
              << "  -h            显示此帮助信息\n"
              << "  -b <秒>      开始时间（默认：0.0秒）\n"
              << "  -e <秒>      结束时间\n"
              << "  -d <秒>      持续时间（默认：直到结束）\n"
              << "  -s <n>       每秒采样次数（默认：100）\n"
              << "  -l <音符>    最低音符（默认：20Hz，人耳可听最低频率）\n"
              << "  -u <音符>    最高音符（默认：20kHz，人耳可听最高频率）\n"
              << "\n音符格式示例：C4（中央C）、D#3、Gb5 等\n"
              << "\n注意：\n"
              << "1. 开始时间、结束时间、持续时间中只能指定其中两个\n"
              << "2. 当输入为文件夹时，将处理文件夹中所有支持的音频文件\n"
              << "   支持的格式：WAV, FLAC, OGG 等\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "Debug: Program started with " << argc << " arguments" << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "Debug: argv[" << i << "] = " << argv[i] << std::endl;
    }
    
    // 获取程序名称（不包含路径）
    std::string programName = fs::path(argv[0]).filename().string();
    std::cout << "Debug: Program name = " << programName << std::endl;
    
    if (argc > 1 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
        std::cout << "Debug: Showing version info" << std::endl;
        std::cout << "Debug: SPECTRUM_VERSION = " << SPECTRUM_VERSION << std::endl;
        std::cout << "Debug: SPECTRUM_VERSION_MAJOR = " << SPECTRUM_VERSION_MAJOR << std::endl;
        std::cout << "Debug: SPECTRUM_VERSION_MINOR = " << SPECTRUM_VERSION_MINOR << std::endl;
        std::cout << "Debug: SPECTRUM_VERSION_PATCH = " << SPECTRUM_VERSION_PATCH << std::endl;
        std::cout << "Musical Spectrum Analyzer version " << SPECTRUM_VERSION << std::endl;
        std::cout.flush();
        return 0;
    }

    if (argc < 3 || (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help"))) {
        std::cout << "用法: " << programName << " <输入文件/目录> <输出目录> [选项]" << std::endl;
        std::cout << "选项:" << std::endl;
        std::cout << "  -h                            显示此帮助信息" << std::endl;
        std::cout << "  -b <秒>                       开始时间（默认：0.0秒）" << std::endl;
        std::cout << "  -e <秒>                       结束时间" << std::endl;
        std::cout << "  -d <秒>                       持续时间（默认：直到结束）" << std::endl;
        std::cout << "  -s <n>                        每秒采样次数（默认：100）" << std::endl;
        std::cout << "  -l <音符>                     最低音符（默认：20Hz）" << std::endl;
        std::cout << "  -u <音符>                     最高音符（默认：20kHz）" << std::endl;
        std::cout << "\n音符格式示例：C4（中央C）、D#3、Gb5 等\n";
        std::cout << "\n支持的音频格式：WAV, FLAC, OGG 等\n";
        std::cout << "\n注意：开始时间、结束时间、持续时间中只能指定其中两个\n";
        std::cout.flush();
        return argc < 2 ? 1 : 0;
    }

    // 解析输入和输出路径
    std::string inputPath = argv[1];
    std::string outputPath = argv[2];
    
    std::cout << "输入路径: " << inputPath << std::endl;
    std::cout << "输出路径: " << outputPath << std::endl;

    // 配置选项
    Spectrogram::Config config;
    std::optional<double> endTime;
    bool hasStartTime = false;
    bool hasDuration = false;
    bool hasEndTime = false;
    
    // 解析命令行选项
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            std::cerr << "错误：选项 " << arg << " 需要一个参数" << std::endl;
            return 1;
        }
        
        try {
            if (arg == "-s") {
                config.samples_per_sec = std::stoi(argv[++i]);
                std::cout << "设置每秒采样数为: " << config.samples_per_sec << std::endl;
            }
            else if (arg == "-l") {
                try {
                    config.min_freq = noteToFreq(argv[++i]);
                    std::cout << "设置最低频率为: " << config.min_freq << " Hz" << std::endl;
                } catch (const std::exception& e) {
                    config.min_freq = std::stod(argv[i]);
                    std::cout << "设置最低频率为: " << config.min_freq << " Hz" << std::endl;
                }
            }
            else if (arg == "-u") {
                try {
                    config.max_freq = noteToFreq(argv[++i]);
                    std::cout << "设置最高频率为: " << config.max_freq << " Hz" << std::endl;
                } catch (const std::exception& e) {
                    config.max_freq = std::stod(argv[i]);
                    std::cout << "设置最高频率为: " << config.max_freq << " Hz" << std::endl;
                }
            }
            else if (arg == "-b") {
                config.start_time = std::stod(argv[++i]);
                hasStartTime = true;
                std::cout << "设置开始时间为: " << config.start_time << " 秒" << std::endl;
            }
            else if (arg == "-e") {
                endTime = std::stod(argv[++i]);
                hasEndTime = true;
                std::cout << "设置结束时间为: " << endTime.value() << " 秒" << std::endl;
            }
            else if (arg == "-d") {
                config.duration = std::stod(argv[++i]);
                hasDuration = true;
                std::cout << "设置持续时间为: " << config.duration << " 秒" << std::endl;
            }
            else if (arg == "-h") {
                // 帮助信息已经在前面处理过了
                return 0;
            }
            else {
                std::cerr << "未知选项: " << arg << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "参数错误: " << e.what() << std::endl;
            return 1;
        }
    }

    // 检查时间参数的组合
    int timeParamsCount = hasStartTime + hasDuration + hasEndTime;
    if (timeParamsCount > 2) {
        std::cerr << "错误：开始时间、结束时间、持续时间只能指定其中两个\n";
        return 1;
    }

    // 根据指定的时间参数计算持续时间
    if (hasEndTime) {
        if (endTime.value() <= config.start_time) {
            std::cerr << "错误：结束时间必须大于开始时间\n";
            return 1;
        }
        if (!hasDuration) {
            config.duration = endTime.value() - config.start_time;
            std::cout << "计算得到持续时间为: " << config.duration << " 秒" << std::endl;
        }
    }

    // 创建输出目录
    try {
        fs::create_directories(outputPath);
        std::cout << "已创建输出目录: " << outputPath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "创建输出目录失败: " << e.what() << std::endl;
        return 1;
    }

    // 处理输入
    if (fs::is_directory(inputPath)) {
        std::cout << "处理目录: " << inputPath << std::endl;
        for (const auto& entry : fs::directory_iterator(inputPath)) {
            if (entry.path().extension() == ".wav" || entry.path().extension() == ".mp3") {
                std::string outputFile = (fs::path(outputPath) / entry.path().filename()).string();
                outputFile = outputFile.substr(0, outputFile.find_last_of('.')) + ".png";
                processAudioFile(entry.path().string(), outputFile, config);
            }
        }
    } else {
        std::cout << "处理单个文件: " << inputPath << std::endl;
        std::string outputFile = (fs::path(outputPath) / fs::path(inputPath).filename()).string();
        outputFile = outputFile.substr(0, outputFile.find_last_of('.')) + ".png";
        processAudioFile(inputPath, outputFile, config);
    }

    return 0;
} 