#include "spectrogram.hpp"
#include <cmath>
#include <algorithm>

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#else
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

void Spectrogram::generateSpectrogram(const std::vector<std::vector<double>>& specData,
                                    const std::string& outputFile,
                                    int sampleRate,
                                    const Config& config) {
    const int width = 3200;  // 默认宽度
    const int height = 2400; // 默认高度

#ifdef __APPLE__
    generateImageCG(specData, outputFile, width, height, config.min_freq, config.max_freq, sampleRate);
#else
    generateImageStb(specData, outputFile, width, height, config.min_freq, config.max_freq, sampleRate);
#endif
}

#ifdef __APPLE__
void Spectrogram::generateImageCG(const std::vector<std::vector<double>>& data,
                                 const std::string& outputFile,
                                 int width, int height,
                                 double minFreq, double maxFreq,
                                 int sampleRate) {
    // 创建颜色空间
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    
    // 创建上下文
    CGContextRef context = CGBitmapContextCreate(nullptr,
                                               width, height,
                                               8, // bits per component
                                               width * 4, // bytes per row
                                               colorSpace,
                                               kCGImageAlphaPremultipliedLast);
    
    // 设置背景为黑色
    CGContextSetRGBFillColor(context, 0, 0, 0, 1);
    CGContextFillRect(context, CGRectMake(0, 0, width, height));
    
    // 绘制频谱数据
    for (size_t x = 0; x < data.size() && x < width; ++x) {
        for (size_t y = 0; y < data[x].size(); ++y) {
            double freq = y * sampleRate / 2.0 / data[x].size();
            if (freq < minFreq || freq > maxFreq) continue;
            
            int pixelY = height - 1 - static_cast<int>(freqToY(freq, height, minFreq, maxFreq));
            if (pixelY < 0 || pixelY >= height) continue;
            
            // 将频谱数据转换为颜色
            double intensity = data[x][y];
            double hue = 0.7 * (1.0 - std::min(1.0, intensity)); // 从蓝到红
            
            // HSV to RGB 转换
            double h = hue * 6.0;
            double s = 1.0;
            double v = std::min(1.0, intensity * 2.0);
            
            double r, g, b;
            int i = static_cast<int>(h);
            double f = h - i;
            double p = v * (1 - s);
            double q = v * (1 - s * f);
            double t = v * (1 - s * (1 - f));
            
            switch (i % 6) {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                case 5: r = v; g = p; b = q; break;
                default: r = g = b = 0; break;
            }
            
            CGContextSetRGBFillColor(context, r, g, b, 1);
            CGContextFillRect(context, CGRectMake(x, pixelY, 1, 1));
        }
    }
    
    // 添加音符标注
    CGContextSetRGBStrokeColor(context, 1, 1, 1, 0.5); // 白色，半透明
    CGContextSetLineWidth(context, 1);
    
    // 设置字体
    CGContextSelectFont(context, "Helvetica", 12, kCGEncodingMacRoman);
    CGContextSetRGBFillColor(context, 1, 1, 1, 1);
    CGContextSetTextDrawingMode(context, kCGTextFill);
    
    // 绘制音符刻度
    for (int octave = 1; octave <= 8; ++octave) {
        const char* notes[] = {"C", "D", "E", "F", "G", "A", "B"};
        for (const char* note : notes) {
            std::string noteStr = std::string(note) + std::to_string(octave);
            double freq = 440.0 * std::pow(2.0, (getNoteAndOctave(noteStr).first - 69) / 12.0);
            if (freq >= minFreq && freq <= maxFreq) {
                int y = height - 1 - static_cast<int>(freqToY(freq, height, minFreq, maxFreq));
                
                // 绘制横线
                CGContextMoveToPoint(context, 0, y);
                CGContextAddLineToPoint(context, width, y);
                CGContextStrokePath(context);
                
                // 绘制文本
                CGContextSaveGState(context);
                CGContextTranslateCTM(context, 5, y - 6);
                CGContextShowText(context, noteStr.c_str(), noteStr.length());
                CGContextRestoreGState(context);
            }
        }
    }
    
    // 创建图像
    CGImageRef image = CGBitmapContextCreateImage(context);
    
    // 创建URL
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(nullptr,
                                                         (const UInt8*)outputFile.c_str(),
                                                         outputFile.length(),
                                                         false);
    
    // 创建图像目标
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url,
                                                                       kUTTypePNG,
                                                                       1,
                                                                       nullptr);
    
    // 添加图像到目标
    CGImageDestinationAddImage(destination, image, nullptr);
    
    // 完成图像写入
    CGImageDestinationFinalize(destination);
    
    // 清理资源
    CFRelease(destination);
    CFRelease(url);
    CGImageRelease(image);
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
}
#else
void Spectrogram::generateImageStb(const std::vector<std::vector<double>>& data,
                                  const std::string& outputFile,
                                  int width, int height,
                                  double minFreq, double maxFreq,
                                  int sampleRate) {
    // 创建图像数据
    std::vector<unsigned char> imageData(width * height * 3, 0);
    
    // 绘制频谱数据
    for (size_t x = 0; x < data.size() && x < width; ++x) {
        for (size_t y = 0; y < data[x].size(); ++y) {
            double freq = y * sampleRate / 2.0 / data[x].size();
            if (freq < minFreq || freq > maxFreq) continue;
            
            int pixelY = height - 1 - static_cast<int>(freqToY(freq, height, minFreq, maxFreq));
            if (pixelY < 0 || pixelY >= height) continue;
            
            // 将频谱数据转换为颜色
            double intensity = data[x][y];
            double hue = 0.7 * (1.0 - std::min(1.0, intensity)); // 从蓝到红
            
            // HSV to RGB 转换
            double h = hue * 6.0;
            double s = 1.0;
            double v = std::min(1.0, intensity * 2.0);
            
            double r, g, b;
            int i = static_cast<int>(h);
            double f = h - i;
            double p = v * (1 - s);
            double q = v * (1 - s * f);
            double t = v * (1 - s * (1 - f));
            
            switch (i % 6) {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                case 5: r = v; g = p; b = q; break;
                default: r = g = b = 0; break;
            }
            
            int idx = (pixelY * width + x) * 3;
            imageData[idx] = static_cast<unsigned char>(r * 255);
            imageData[idx + 1] = static_cast<unsigned char>(g * 255);
            imageData[idx + 2] = static_cast<unsigned char>(b * 255);
        }
    }
    
    // 保存图像
    stbi_write_png(outputFile.c_str(), width, height, 3, imageData.data(), width * 3);
}
#endif

double Spectrogram::freqToY(double freq, int height, double minFreq, double maxFreq) {
    // 使用对数刻度
    double logMin = std::log2(minFreq);
    double logMax = std::log2(maxFreq);
    double logFreq = std::log2(freq);
    return height * (logFreq - logMin) / (logMax - logMin);
}

std::pair<std::string, int> Spectrogram::getNoteAndOctave(double freq) {
    // MIDI音符号到音符名的转换
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    // 计算MIDI音符号
    double midiNote = 69 + 12 * std::log2(freq / 440.0);
    int noteNumber = static_cast<int>(std::round(midiNote));
    
    // 计算八度和音符
    int octave = (noteNumber / 12) - 1;
    int noteIndex = noteNumber % 12;
    if (noteIndex < 0) noteIndex += 12;
    
    return {noteNames[noteIndex], octave};
}

bool Spectrogram::isWhiteKey(const std::string& note) {
    return note.length() == 1; // 没有升降号的音符是白键
} 