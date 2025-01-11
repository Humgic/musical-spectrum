class Spectrum < Formula
  desc "音频频谱分析工具"
  homepage "https://github.com/Humgic/musical-spectrum"
  url "https://github.com/Humgic/musical-spectrum/archive/refs/tags/v3.1.0.tar.gz"
  sha256 "05bc65c2776c8deeeaf26836a9ed80104d940e15b87a903722f655a1df1bae44"
  license "MIT"
  head "https://github.com/Humgic/musical-spectrum.git", branch: "main"
  
  depends_on "cmake" => :build
  depends_on "fftw"
  depends_on "libsndfile"
  depends_on "googletest" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args,
           "-DUSE_CORE_GRAPHICS=ON"
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/spectrum_analyzer", "--version"
  end
end 