#pragma once

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

// 音符到频率的转换函数
double noteToFreq(const std::string& note);

class Spectrogram {
public:
    struct Config {
        double start_time = 0.0;     // 开始时间（秒）
        double duration = -1.0;      // 持续时间（秒），-1表示直到结束
        int samples_per_sec = 100;   // 每秒采样次数
        double min_freq = 20.0;      // 最低频率（Hz）
        double max_freq = 20000.0;   // 最高频率（Hz）
    };
    
    Spectrogram();
    
    void generateSpectrogram(const std::vector<std::vector<double>>& specData, 
                           const std::string& outputPath,
                           int sampleRate,
                           const Config& config,
                           int height = 2400,
                           int width = 3200);
                           
    double freqToY(double freq, int height, double minFreq, double maxFreq);
                           
private:
    // 基础图像处理函数
    cv::Mat convertToImage(const std::vector<std::vector<double>>& specData,
                          int height, int width, int sampleRate,
                          const Config& config);
    void applyColorMap(cv::Mat& image);
    void addPitchLabels(cv::Mat& image, int sampleRate, int fftSize, const Config& config);
    std::pair<std::string, int> getNoteAndOctave(double freq);
    bool isWhiteKey(const std::string& note);
}; 