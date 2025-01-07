#pragma once

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

class Spectrogram {
public:
    Spectrogram();
    
    void generateSpectrogram(const std::vector<std::vector<double>>& specData, 
                           const std::string& outputPath,
                           int sampleRate,
                           int height = 1200,
                           int width = 1600);
                           
private:
    cv::Mat convertToImage(const std::vector<std::vector<double>>& specData,
                          int height, int width, int sampleRate);
    void applyColorMap(cv::Mat& image);
    void addPitchLabels(cv::Mat& image, int sampleRate, int fftSize);
    double freqToY(double freq, int height, double minFreq, double maxFreq);
    std::string getNoteName(double freq);
}; 