#include "audio_processor.hpp"
#include <cmath>
#include <stdexcept>

AudioProcessor::AudioProcessor() {}

AudioProcessor::~AudioProcessor() {}

bool AudioProcessor::loadAudioFile(const std::string& filename) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    
    if (!file) {
        return false;
    }
    
    sampleRate = sfinfo.samplerate;
    channels = sfinfo.channels;
    
    // 读取所有采样点
    std::vector<float> buffer(sfinfo.frames * sfinfo.channels);
    sf_readf_float(file, buffer.data(), sfinfo.frames);
    
    // 如果是多声道，只取第一个声道
    audioData.resize(sfinfo.frames);
    for (sf_count_t i = 0; i < sfinfo.frames; ++i) {
        audioData[i] = buffer[i * sfinfo.channels];
    }
    
    sf_close(file);
    return true;
}

void AudioProcessor::applyHannWindow(std::vector<double>& window) {
    for (size_t i = 0; i < window.size(); ++i) {
        double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (window.size() - 1)));
        window[i] *= multiplier;
    }
}

std::vector<double> AudioProcessor::computeFFT(const std::vector<double>& window) {
    int n = window.size();
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n);
    
    // Copy input data
    for (int i = 0; i < n; i++) {
        in[i][0] = window[i];
        in[i][1] = 0.0;
    }
    
    fftw_plan plan = fftw_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    
    std::vector<double> magnitudes(n/2 + 1);
    for (int i = 0; i <= n/2; i++) {
        double real = out[i][0];
        double imag = out[i][1];
        magnitudes[i] = sqrt(real*real + imag*imag);
    }
    
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    
    return magnitudes;
}

std::vector<std::vector<double>> AudioProcessor::computeSpectrogram(int windowSize, int hopSize) {
    if (audioData.empty()) {
        throw std::runtime_error("No audio data loaded");
    }
    
    std::vector<std::vector<double>> spectrogram;
    std::vector<double> window(windowSize);
    
    for (size_t i = 0; i + windowSize <= audioData.size(); i += hopSize) {
        // Copy audio data to window
        for (int j = 0; j < windowSize; ++j) {
            window[j] = audioData[i + j];
        }
        
        // Apply Hann window
        applyHannWindow(window);
        
        // Compute FFT and add to spectrogram
        auto magnitudes = computeFFT(window);
        
        // Convert to dB scale
        for (auto& mag : magnitudes) {
            mag = 20 * log10(mag + 1e-6);
        }
        
        spectrogram.push_back(magnitudes);
    }
    
    return spectrogram;
} 