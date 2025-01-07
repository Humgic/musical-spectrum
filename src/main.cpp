#include "spectrogram.hpp"
#include <iostream>
#include <string>
#include <sndfile.h>
#include <vector>
#include <cstring>
#include <map>
#include <cmath>
#include <regex>
#include <optional>

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

void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " <输入音频文件> <输出图像文件> [选项]\n"
              << "选项:\n"
              << "  -h            显示此帮助信息\n"
              << "  -b <秒>      开始时间（默认：0.0秒）\n"
              << "  -e <秒>      结束时间\n"
              << "  -d <秒>      持续时间（默认：直到结束）\n"
              << "  -s <n>       每秒采样次数（默认：100）\n"
              << "  -l <音符>    最低音符（默认：20Hz，人耳可听最低频率）\n"
              << "  -u <音符>    最高音符（默认：20kHz，人耳可听最高频率）\n"
              << "\n音符格式示例：C4（中央C）、D#3、Gb5 等\n"
              << "\n注意：开始时间、结束时间、持续时间中只能指定其中两个\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || strcmp(argv[1], "-h") == 0) {
        printUsage(argv[0]);
        return argc < 2 ? 1 : 0;
    }

    if (argc < 3) {
        std::cerr << "错误：需要指定输入和输出文件\n";
        printUsage(argv[0]);
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];
    
    // 默认配置
    Spectrogram::Config config;
    config.min_freq = 20.0;    // 人耳可听最低频率
    config.max_freq = 20000.0; // 人耳可听最高频率
    
    // 用于检查时间参数的计数
    std::optional<double> endTime;
    bool hasStartTime = false;
    bool hasDuration = false;
    bool hasEndTime = false;
    
    // 解析命令行参数
    for (int i = 3; i < argc; i++) {
        try {
            if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
                std::cerr << "无效的选项: " << argv[i] << std::endl;
                return 1;
            }
            
            if (++i >= argc) {
                std::cerr << "选项 " << argv[i-1] << " 需要一个参数" << std::endl;
                return 1;
            }

            switch (argv[i-1][1]) {
                case 'b':
                    config.start_time = std::stod(argv[i]);
                    hasStartTime = true;
                    break;
                case 'e':
                    endTime = std::stod(argv[i]);
                    hasEndTime = true;
                    break;
                case 'd':
                    config.duration = std::stod(argv[i]);
                    hasDuration = true;
                    break;
                case 's':
                    config.samples_per_sec = std::stoi(argv[i]);
                    break;
                case 'l':
                    config.min_freq = noteToFreq(argv[i]);
                    break;
                case 'u':
                    config.max_freq = noteToFreq(argv[i]);
                    break;
                default:
                    std::cerr << "未知选项: -" << argv[i-1][1] << std::endl;
                    printUsage(argv[0]);
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
        }
    }

    // 打开音频文件
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
    SNDFILE* sndFile = sf_open(inputFile, SFM_READ, &sfInfo);
    
    if (!sndFile) {
        std::cerr << "无法打开音频文件: " << inputFile << std::endl;
        return 1;
    }
    
    // 读取音频数据
    std::vector<double> audioData(sfInfo.frames * sfInfo.channels);
    sf_read_double(sndFile, audioData.data(), audioData.size());
    sf_close(sndFile);
    
    // 如果是立体声，转换为单声道
    std::vector<double> monoData;
    if (sfInfo.channels == 2) {
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
    
    // 创建频谱数据
    std::vector<std::vector<double>> specData(numFrames, std::vector<double>(fftSize/2 + 1));
    
    // ... 这里添加FFT计算代码 ...
    
    // 生成频谱图
    Spectrogram spectrogram;
    spectrogram.generateSpectrogram(specData, outputFile, sfInfo.samplerate, config);
    
    std::cout << "频谱图已生成: " << outputFile << std::endl;
    return 0;
} 