class Spectrum < Formula
  desc "音频频谱分析工具"
  homepage "https://github.com/Humgic/musical-spectrum"
  url "https://github.com/Humgic/musical-spectrum/archive/refs/tags/v2.0.0.tar.gz"
  sha256 "REPLACE_WITH_ACTUAL_SHA256"
  license "MIT"
  head "https://github.com/Humgic/musical-spectrum.git", branch: "main"
  
  depends_on "cmake" => :build
  depends_on "opencv"
  depends_on "fftw"
  depends_on "libsndfile"
  depends_on "googletest" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/spectrum_analyzer", "--version"
  end
end 