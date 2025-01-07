#include "audio_processor.hpp"
#include "spectrogram.hpp"
#include <iostream>

void printUsage(const char* programName) {
    std::cout << "使用方法: " << programName << " <输入音频文件> <输出图片文件>\n";
    std::cout << "示例: " << programName << " input.wav output.png\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        AudioProcessor audioProcessor;
        if (!audioProcessor.loadAudioFile(argv[1])) {
            std::cerr << "无法加载音频文件: " << argv[1] << std::endl;
            return 1;
        }
        
        // 计算频谱图数据
        auto spectrogramData = audioProcessor.computeSpectrogram(4096, 1024);
        
        // 生成频谱图图像
        Spectrogram spectrogram;
        spectrogram.generateSpectrogram(spectrogramData, argv[2], audioProcessor.getSampleRate());
        
        std::cout << "频谱图已生成: " << argv[2] << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 