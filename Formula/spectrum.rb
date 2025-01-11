class Spectrum < Formula
  desc "音频频谱分析工具"
  homepage "https://github.com/Humgic/musical-spectrum"
  url "https://github.com/Humgic/musical-spectrum/archive/refs/tags/v3.0.0.tar.gz"
  sha256 "054bce5e0a2917add3487e29c0f34d35a59753c88fd6ec4d069861c0f565a98f"
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