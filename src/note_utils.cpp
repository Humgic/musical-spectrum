#include "note_utils.hpp"
#include <map>
#include <regex>
#include <cmath>
#include <stdexcept>

double noteToFreq(const std::string& note) {
    static const std::map<std::string, int> noteToSemitone = {
        {"C", 0}, {"D", 2}, {"E", 4}, {"F", 5},
        {"G", 7}, {"A", 9}, {"B", 11}
    };
    
    // 使用正则表达式解析音符格式（例如：C4, D#3, Gb5）
    std::regex notePattern("([A-G])(#|b)?(\\d+)");
    std::smatch matches;
    if (!std::regex_match(note, matches, notePattern)) {
        throw std::runtime_error("无效的音符格式: " + note);
    }
    
    std::string noteName = matches[1];
    std::string accidental = matches[2];
    int octave = std::stoi(matches[3]);
    
    // 计算MIDI音符号
    int semitone = noteToSemitone.at(noteName);
    if (accidental == "#") semitone += 1;
    if (accidental == "b") semitone -= 1;
    
    // MIDI音符到频率的转换（A4 = 440Hz）
    int midiNote = (octave + 1) * 12 + semitone;
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
} 