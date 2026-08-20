# opsec

`opsec` is the OPSEC Express: a tiny terminal privacy meme for when you meant
to type `ls`. It behaves like the classic `sl` gag, but the train is carrying
classified cargo, redactions, and a reminder to check your dotfiles.

## Build locally

On a Debian or Ubuntu machine with a C compiler and GNU Make:

```sh
make
./opsec
```

## Install with Homebrew

Once the formula is accepted into Homebrew/core, install it anywhere with:

```sh
brew install opsec
```

Until then, install it directly from this repository as a tap:

```sh
brew tap yazidears/opsec
brew install yazidears/opsec/opsec
opsec
```

Useful flags:

```text
opsec --once          print one frame and exit
opsec --no-color      disable ANSI colors
opsec --speed 35      make the train faster
opsec --help          show all options
```

The program automatically prints a plain frame when stdout is not a terminal,
so it is safe to use in smoke tests and package checks. Setting `NO_COLOR`
also disables ANSI colors.

## Build the Debian package

On Debian or Ubuntu, install the packaging toolchain and build the package:

```sh
sudo apt update
sudo apt install build-essential debhelper-compat devscripts
dpkg-buildpackage -us -uc -b
sudo apt install ../opsec_0.1.0-1_$(dpkg --print-architecture).deb
```

After that, run it with:

```sh
opsec
```

The package is architecture-aware and has no runtime dependency beyond the
standard C library. The `debian/` directory is ready for a Debian repository,
PPA, or other apt publisher; making `sudo apt install opsec` work for everyone
requires publishing the built package and having users add that repository.

## License

MIT. See [LICENSE](LICENSE).
