#include <gtest/gtest.h>
#include "spectrogram.hpp"
#include <cmath>
#include <filesystem>

class SpectrogramTest : public ::testing::Test {
protected:
    Spectrogram spec;
    const double epsilon = 1e-6;  // 浮点数比较的误差范围
    
    void SetUp() override {
        // 在每个测试前运行
    }
    
    void TearDown() override {
        // 在每个测试后运行
    }
};

// 测试音符到频率的转换
TEST_F(SpectrogramTest, NoteToFreqConversion) {
    // A4 = 440Hz
    EXPECT_NEAR(noteToFreq("A4"), 440.0, epsilon);
    
    // C4（中央C）≈ 261.63Hz
    EXPECT_NEAR(noteToFreq("C4"), 261.63, 0.01);
    
    // 测试升降号
    EXPECT_NEAR(noteToFreq("C#4"), 277.18, 0.01);
    EXPECT_NEAR(noteToFreq("Db4"), 277.18, 0.01);
    
    // 测试不同八度
    EXPECT_NEAR(noteToFreq("A3"), 220.0, epsilon);
    EXPECT_NEAR(noteToFreq("A5"), 880.0, epsilon);
}

// 测试频率到Y坐标的映射
TEST_F(SpectrogramTest, FreqToYMapping) {
    const int height = 1000;
    const double minFreq = 20.0;
    const double maxFreq = 20000.0;
    
    // 测试边界值
    EXPECT_NEAR(spec.freqToY(minFreq, height, minFreq, maxFreq), height, epsilon);
    EXPECT_NEAR(spec.freqToY(maxFreq, height, minFreq, maxFreq), 0, epsilon);
    
    // 测试中间值
    double midFreq = sqrt(minFreq * maxFreq);
    EXPECT_NEAR(spec.freqToY(midFreq, height, minFreq, maxFreq), height/2, epsilon);
}

// 测试配置参数验证
TEST_F(SpectrogramTest, ConfigValidation) {
    Spectrogram::Config config;
    
    // 测试默认值
    EXPECT_EQ(config.start_time, 0.0);
    EXPECT_EQ(config.duration, -1.0);
    EXPECT_EQ(config.samples_per_sec, 100);
    EXPECT_EQ(config.min_freq, 20.0);
    EXPECT_EQ(config.max_freq, 20000.0);
    
    // 测试参数设置
    config.start_time = 1.5;
    config.duration = 5.0;
    config.samples_per_sec = 200;
    config.min_freq = noteToFreq("C3");
    config.max_freq = noteToFreq("C6");
    
    EXPECT_EQ(config.start_time, 1.5);
    EXPECT_EQ(config.duration, 5.0);
    EXPECT_EQ(config.samples_per_sec, 200);
    EXPECT_NEAR(config.min_freq, 130.81, 0.01);
    EXPECT_NEAR(config.max_freq, 1046.50, 0.01);
}

// 测试图像生成
TEST_F(SpectrogramTest, ImageGeneration) {
    // 创建测试数据
    const int numFrames = 100;
    const int numBins = 1025;  // fftSize/2 + 1
    std::vector<std::vector<double>> specData(numFrames, std::vector<double>(numBins));
    
    // 生成测试信号（例如：正弦波）
    for (int i = 0; i < numFrames; ++i) {
        for (int j = 0; j < numBins; ++j) {
            specData[i][j] = sin(2 * M_PI * i / numFrames) * sin(2 * M_PI * j / numBins);
        }
    }
    
    // 测试图像生成
    const std::string outputPath = "test_output.png";
    Spectrogram::Config config;
    EXPECT_NO_THROW(spec.generateSpectrogram(specData, outputPath, 44100, config));
    
    // 验证文件是否生成
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    
    // 清理测试文件
    std::filesystem::remove(outputPath);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 