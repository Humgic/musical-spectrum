#ifndef NOTE_UTILS_HPP
#define NOTE_UTILS_HPP

#include <string>

// 将音符字符串转换为频率（Hz）
// 例如：A4 = 440Hz, C4 ≈ 261.63Hz
double noteToFreq(const std::string& note);

#endif // NOTE_UTILS_HPP 