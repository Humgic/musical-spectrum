class Spectrum < Formula
  desc "音频频谱图生成工具"
  homepage "https://github.com/Humgic/Spectrum"
  url "https://github.com/Humgic/Spectrum/archive/refs/tags/v1.0.0.tar.gz"
  sha256 "YOUR_TARBALL_SHA256"
  license "MIT"
  
  depends_on "cmake" => :build
  depends_on "fftw"
  depends_on "libsndfile"
  depends_on "opencv"

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    bin.install "build/spectrum_analyzer"
  end

  test do
    system "#{bin}/spectrum_analyzer"
  end
end 