#include <gtest/gtest.h>
#include "spectrogram.hpp"
#include "note_utils.hpp"

// 测试音符到频率的转换
TEST(SpectrogramTest, NoteToFreqConversion) {
    const double epsilon = 0.01;
    
    // 测试标准音高 A4 = 440Hz
    EXPECT_NEAR(noteToFreq("A4"), 440.0, epsilon);
    
    // 测试中央C（C4）
    EXPECT_NEAR(noteToFreq("C4"), 261.63, 0.01);
    
    // 测试升降号
    EXPECT_NEAR(noteToFreq("C#4"), 277.18, 0.01);
    EXPECT_NEAR(noteToFreq("Db4"), 277.18, 0.01);
    
    // 测试不同八度
    EXPECT_NEAR(noteToFreq("A3"), 220.0, epsilon);
    EXPECT_NEAR(noteToFreq("A5"), 880.0, epsilon);
}

// 测试频谱图生成的基本功能
TEST(SpectrogramTest, BasicGeneration) {
    Spectrogram spec;
    Spectrogram::Config config;
    
    // 测试默认配置
    EXPECT_EQ(config.start_time, 0.0);
    EXPECT_EQ(config.samples_per_sec, 100);
    EXPECT_NEAR(config.min_freq, 20.0, 0.01);
    EXPECT_NEAR(config.max_freq, 20000.0, 0.01);
    
    // 测试配置修改
    config.min_freq = noteToFreq("C3");
    config.max_freq = noteToFreq("C6");
    EXPECT_GT(config.max_freq, config.min_freq);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 