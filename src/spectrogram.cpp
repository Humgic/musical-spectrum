#include "spectrogram.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

Spectrogram::Spectrogram() {}

void Spectrogram::generateSpectrogram(const std::vector<std::vector<double>>& specData,
                                    const std::string& outputPath,
                                    int sampleRate,
                                    int height, int width) {
    cv::Mat spectrogramImage = convertToImage(specData, height, width, sampleRate);
    applyColorMap(spectrogramImage);
    
    // 添加音高标注
    int fftSize = (specData[0].size() - 1) * 2;
    addPitchLabels(spectrogramImage, sampleRate, fftSize);
    
    cv::imwrite(outputPath, spectrogramImage);
}

double Spectrogram::freqToY(double freq, int height, double minFreq, double maxFreq) {
    // 对数映射
    double logFreq = log2(freq);
    double logMin = log2(minFreq);
    double logMax = log2(maxFreq);
    return height * (1.0 - (logFreq - logMin) / (logMax - logMin));
}

std::string Spectrogram::getNoteName(double freq) {
    // A4 = 440Hz
    const double A4_FREQ = 440.0;
    const int A4_MIDI = 69;
    const std::string NOTE_NAMES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    double steps = 12 * log2(freq / A4_FREQ);
    int midiNote = round(steps) + A4_MIDI;
    int octave = (midiNote / 12) - 1;
    int noteIndex = midiNote % 12;
    
    if (noteIndex < 0) noteIndex += 12;
    return NOTE_NAMES[noteIndex] + std::to_string(octave);
}

void Spectrogram::addPitchLabels(cv::Mat& image, int sampleRate, int fftSize) {
    const int labelWidth = 50;
    cv::Mat withLabels;
    cv::copyMakeBorder(image, withLabels, 0, 0, labelWidth, 0, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
    
    // 频率范围：20Hz到20kHz（人耳可听范围）
    const double minFreq = 20.0;
    const double maxFreq = 20000.0;
    
    // 标注八度音阶
    for (int octave = 0; octave <= 8; ++octave) {
        double freq = 440.0 * pow(2.0, (octave * 12 - 57) / 12.0); // C频率
        if (freq >= minFreq && freq <= maxFreq) {
            int y = round(freqToY(freq, image.rows, minFreq, maxFreq));
            cv::line(withLabels, cv::Point(0, y), cv::Point(labelWidth-5, y), cv::Scalar(0, 0, 0), 1);
            cv::putText(withLabels, "C" + std::to_string(octave), 
                       cv::Point(5, y+4), cv::FONT_HERSHEY_SIMPLEX, 0.4, 
                       cv::Scalar(0, 0, 0), 1);
        }
    }
    
    image = withLabels;
}

cv::Mat Spectrogram::convertToImage(const std::vector<std::vector<double>>& specData,
                                  int height, int width, int sampleRate) {
    if (specData.empty() || specData[0].empty()) {
        throw std::runtime_error("Empty spectrogram data");
    }
    
    // 找到最大最小值用于归一化
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    
    for (const auto& row : specData) {
        auto [min, max] = std::minmax_element(row.begin(), row.end());
        minVal = std::min(minVal, *min);
        maxVal = std::max(maxVal, *max);
    }
    
    // 创建图像
    cv::Mat image(height, width, CV_8UC1);
    
    double range = maxVal - minVal;
    int timeSteps = specData.size();
    int freqBins = specData[0].size();
    
    // 频率范围（Hz）
    const double minFreq = 20.0;
    const double maxFreq = 20000.0;
    
    // 对数频率映射
    for (int x = 0; x < width; ++x) {
        int timeStep = x * (timeSteps - 1) / (width - 1);
        
        for (int y = 0; y < height; ++y) {
            // 将y坐标映射到对数频率
            double freq = minFreq * pow(maxFreq/minFreq, (height-1-y)/(double)(height-1));
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

void Spectrogram::applyColorMap(cv::Mat& image) {
    cv::applyColorMap(image, image, cv::COLORMAP_JET);
} 