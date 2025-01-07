class MusicalSpectrum < Formula
  desc "音频频谱图生成工具"
  homepage "https://github.com/Humgic/musical-spectrum"
  url "https://github.com/Humgic/musical-spectrum/archive/refs/tags/v1.0.0.tar.gz"
  sha256 "d649a447472a0c8d12328e14e7e5164f807174bdfa2b155464883c90009ca6a6"
  license "MIT"
  
  depends_on "cmake" => :build
  depends_on "fftw"
  depends_on "libsndfile"
  depends_on "opencv"

  def install
    # 移除静态链接标志
    inreplace "CMakeLists.txt", "-static-libgcc", ""
    inreplace "CMakeLists.txt", "-static-libstdc++", ""
    
    # Mac 特定的构建设置
    args = std_cmake_args + %w[
      -DCMAKE_FIND_FRAMEWORK=LAST
      -DCMAKE_MACOSX_RPATH=ON
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
    ]

    if Hardware::CPU.arm?
      args << "-DCMAKE_OSX_ARCHITECTURES=arm64"
    else
      args << "-DCMAKE_OSX_ARCHITECTURES=x86_64"
    end
    
    # 直接在源码目录构建
    system "cmake", "-B", "build", *args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    system "#{bin}/spectrum_analyzer"
  end
end 