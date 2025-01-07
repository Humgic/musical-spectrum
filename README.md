# Musical Spectrum Analyzer

一个强大的音频频谱图生成工具，支持多种音频格式（WAV、FLAC、OGG等）。

## 特性

- 支持多种音频格式（WAV、FLAC、OGG、AIFF等）
- 对数频率轴显示
- 标准音高标注（C0-C8）
- 高分辨率频谱图输出
- 完整的人耳可听频率范围（20Hz-20kHz）

## 安装

使用 Homebrew 安装：

```bash
brew tap Humgic/musical-spectrum
brew install musical-spectrum
```

## 使用方法

```bash
spectrum_analyzer input.flac output.png
```

## 示例输出

频谱图将显示：
- X轴：时间
- Y轴：频率（带音高标注）
- 颜色：能量强度（蓝色为低，红色为高）

## 依赖

- FFTW3
- libsndfile
- OpenCV

## 许可证

MIT License 