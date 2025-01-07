#pragma once

#include <vector>
#include <string>
#include <sndfile.h>
#include <fftw3.h>

class AudioProcessor {
public:
    AudioProcessor();
    ~AudioProcessor();
    
    bool loadAudioFile(const std::string& filename);
    std::vector<std::vector<double>> computeSpectrogram(int windowSize = 2048, int hopSize = 512);
    int getSampleRate() const { return sampleRate; }

private:
    std::vector<float> audioData;
    int sampleRate;
    int channels;
    
    std::vector<double> computeFFT(const std::vector<double>& window);
    void applyHannWindow(std::vector<double>& window);
}; 