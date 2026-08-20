class Opsec < Formula
  desc "Animated operation-security meme for the terminal"
  homepage "https://github.com/yazidears/opsec"
  url "https://github.com/yazidears/opsec/releases/download/v0.1.1/opsec-0.1.1.tar.gz"
  sha256 "e4fba96a1f5ee9ac03ee61e3ac70573725e46427bd6f99337a89ddf611a3a203"
  license "MIT"

  def install
    system "make", "CC=#{ENV.cc}", "CFLAGS=#{ENV.cflags}"
    bin.install "opsec"
  end

  test do
    assert_match "opsec 0.1.1", shell_output("#{bin}/opsec --version")
    assert_match "OPERATION SECURITY", shell_output("#{bin}/opsec --once")
  end
end
