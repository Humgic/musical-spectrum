#include "spectrogram.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <numeric>

Spectrogram::Spectrogram() {}

void Spectrogram::generateSpectrogram(const std::vector<std::vector<double>>& specData,
                                    const std::string& outputPath,
                                    int sampleRate,
                                    const Config& config,
                                    int height, int width) {
    // 生成频谱图
    cv::Mat spectrogramImage = convertToImage(specData, height, width, sampleRate, config);
    
    // 应用颜色映射
    applyColorMap(spectrogramImage);
    
    // 添加音高标注
    int fftSize = (specData[0].size() - 1) * 2;
    addPitchLabels(spectrogramImage, sampleRate, fftSize, config);
    
    cv::imwrite(outputPath, spectrogramImage);
}

cv::Mat Spectrogram::convertToImage(const std::vector<std::vector<double>>& specData,
                                  int height, int width, int sampleRate,
                                  const Config& config) {
    if (specData.empty() || specData[0].empty()) {
        throw std::runtime_error("Empty spectrogram data");
    }
    
    // 创建单通道图像
    cv::Mat image(height, width, CV_8UC1);
    
    // 计算时间范围
    int startFrame = config.start_time * config.samples_per_sec;
    int numFrames = (config.duration < 0) ? 
                   specData.size() - startFrame : 
                   config.duration * config.samples_per_sec;
    numFrames = std::min(numFrames, static_cast<int>(specData.size() - startFrame));
    
    // 找到最大最小值用于归一化
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    
    for (int i = startFrame; i < startFrame + numFrames; ++i) {
        const auto& row = specData[i];
        auto [min, max] = std::minmax_element(row.begin(), row.end());
        minVal = std::min(minVal, *min);
        maxVal = std::max(maxVal, *max);
    }
    
    double range = maxVal - minVal;
    int freqBins = specData[0].size();
    
    // 对数频率映射
    for (int x = 0; x < width; ++x) {
        int timeStep = startFrame + x * numFrames / width;
        
        for (int y = 0; y < height; ++y) {
            double freq = config.min_freq * pow(config.max_freq/config.min_freq, 
                                             (height-1-y)/(double)(height-1));
            int freqBin = round(freq * freqBins / (sampleRate/2.0));
            
            if (freqBin >= freqBins) freqBin = freqBins - 1;
            if (freqBin < 0) freqBin = 0;
            
            double value = specData[timeStep][freqBin];
            double normalizedValue = (value - minVal) / range;
            image.at<uchar>(y, x) = cv::saturate_cast<uchar>(normalizedValue * 255);
        }
    }
    
    return image;
}

std::pair<std::string, int> Spectrogram::getNoteAndOctave(double freq) {
    // A4 = 440Hz
    const double A4_FREQ = 440.0;
    const int A4_MIDI = 69;
    const std::string NOTE_NAMES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    double steps = 12 * log2(freq / A4_FREQ);
    int midiNote = round(steps) + A4_MIDI;
    int octave = (midiNote / 12) - 1;
    int noteIndex = midiNote % 12;
    
    if (noteIndex < 0) noteIndex += 12;
    return {NOTE_NAMES[noteIndex], octave};
}

bool Spectrogram::isWhiteKey(const std::string& note) {
    return note.length() == 1;  // 白键音符名是单个字母（C,D,E,F,G,A,B）
}

void Spectrogram::addPitchLabels(cv::Mat& image, int sampleRate, int fftSize, const Config& config) {
    const int labelWidth = 50;
    cv::Mat withLabels;
    cv::copyMakeBorder(image, withLabels, 0, 0, labelWidth, 0, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    
    // 标注所有白键（C3-C5）
    for (int midiNote = 48; midiNote <= 72; ++midiNote) {  // C3(48) to C5(72)
        double freq = 440.0 * pow(2.0, (midiNote - 69) / 12.0);  // A4(69) = 440Hz
        if (freq >= config.min_freq && freq <= config.max_freq) {
            auto [note, octave] = getNoteAndOctave(freq);
            if (isWhiteKey(note)) {
                int y = round(freqToY(freq, image.rows, config.min_freq, config.max_freq));
                cv::line(withLabels, cv::Point(0, y), cv::Point(labelWidth-5, y), 
                        cv::Scalar(0, 0, 0), 1);
                cv::putText(withLabels, note + std::to_string(octave), 
                           cv::Point(5, y+4), cv::FONT_HERSHEY_SIMPLEX, 0.4, 
                           cv::Scalar(0, 0, 0), 1);
            }
        }
    }
    
    image = withLabels;
}

void Spectrogram::applyColorMap(cv::Mat& image) {
    cv::applyColorMap(image, image, cv::COLORMAP_JET);
}

double Spectrogram::freqToY(double freq, int height, double minFreq, double maxFreq) {
    // 对数映射
    double logFreq = log2(freq);
    double logMin = log2(minFreq);
    double logMax = log2(maxFreq);
    return height * (1.0 - (logFreq - logMin) / (logMax - logMin));
} 