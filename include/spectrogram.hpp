#ifndef SPECTROGRAM_HPP
#define SPECTROGRAM_HPP

#include <vector>
#include <string>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

class Spectrogram {
public:
    struct Config {
        double start_time = 0.0;     // 开始时间（秒）
        double duration = -1.0;      // 持续时间（秒），-1表示直到结束
        int samples_per_sec = 100;   // 每秒采样次数
        double min_freq = 20.0;      // 最低频率（Hz）
        double max_freq = 20000.0;   // 最高频率（Hz）
    };
    
    void generateSpectrogram(const std::vector<std::vector<double>>& specData,
                           const std::string& outputFile,
                           int sampleRate,
                           const Config& config);

private:
#ifdef __APPLE__
    void generateImageCG(const std::vector<std::vector<double>>& data,
                        const std::string& outputFile,
                        int width, int height,
                        double minFreq, double maxFreq,
                        int sampleRate);
#else
    void generateImageStb(const std::vector<std::vector<double>>& data,
                         const std::string& outputFile,
                         int width, int height,
                         double minFreq, double maxFreq,
                         int sampleRate);
#endif

    // 辅助函数
    double freqToY(double freq, int height, double minFreq, double maxFreq);
    std::pair<std::string, int> getNoteAndOctave(double freq);
    bool isWhiteKey(const std::string& note);
};

#endif // SPECTROGRAM_HPP 