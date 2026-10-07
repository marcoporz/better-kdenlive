# Installing Better Kdenlive on other distributions

> **Status: untested.** Only Fedora 44 has been tested (see the main README). This page
> documents the method for other distributions. If you try it, please open an issue
> saying what worked and what did not, so this page can be corrected.

The procedure is always the same, only the package names change:

1. Install a compiler toolchain, CMake and Ninja.
2. Install Kdenlive's **build dependencies** (the `-devel` / `-dev` packages).
3. Build Better Kdenlive into a prefix inside your home directory.
4. Build the two effect plugins (Floating and Transform + Motion Blur).
5. Run it with its own config folders.

Nothing in steps 1 to 3 and 5 touches your normal Kdenlive.

## Before you start: check your versions

Better Kdenlive is based on Kdenlive 26.08.1, which needs recent **Qt 6**, **KDE
Frameworks 6** and **MLT 7**. Distributions with older libraries cannot build it, and CMake
stops with a message like `Could not find a package configuration file provided by "KF6..."`
or reports a version that is too old. Check MLT first:

~~~
pkg-config --modversion mlt-framework-7
~~~

If the version is much lower than 7.38 (the plugins want at least 7.38), use a newer
distribution release, or a rolling one.

## How to find the missing packages when CMake complains

CMake names the missing package. Search your package manager for it, install the
development package, and run the `cmake -B build ...` line again. Repeat until it ends with
`Build files have been written`.

- openSUSE: `zypper se -s kf6` or `zypper se -s qt6 | grep devel`
- Debian / Ubuntu: `apt search kf6 | grep dev`, or `apt-file search <FileNameFromCMake>`
- Arch: `pacman -Ss kf6`

## openSUSE Slowroll and Tumbleweed

Slowroll follows Tumbleweed with a delay, so its libraries can be older than Tumbleweed's.
Check the versions (see above) before a long build.

Basic tools:

~~~
sudo zypper install git cmake ninja gcc-c++ extra-cmake-modules
~~~

Build dependencies of Kdenlive. This asks zypper for the build requirements of the distribution's
own `kdenlive` package. It needs a **source repository** to be enabled:

~~~
zypper lr -u | grep -i source
sudo zypper mr -e <alias-of-the-source-repo>
sudo zypper source-install --build-deps-only kdenlive
~~~

I do not know the name of the source repository on Slowroll: take the alias from the first
command. You can disable it again afterwards with `sudo zypper mr -d <alias>`.
If you do not want to enable source repositories, install the packages that CMake asks for
one by one (see the section above): they are named like `qt6-...-devel` and `kf6-...-devel`.

Then follow the build steps of the main README, with these differences:

- Configure with the same `cmake -B build -G Ninja ...` line.
- openSUSE puts 64-bit libraries in `lib64`; the run script already covers `lib` and `lib64`.
- The MLT tool is called `melt` on openSUSE (try `melt-7` if it is not found).

Effect plugins:

~~~
sudo zypper install libmlt-devel qt6-base-devel libX11-devel cmake gcc-c++ pkgconf-pkg-config
~~~

Then clone and run `./install.sh` in both plugin repositories (see the main README, step B).

Codecs: the codecs in openSUSE's own ffmpeg are limited, so exporting H.264 or H.265 may fail
with a message about an unsupported codec. The usual fix is to switch to ffmpeg from the Packman
repository; see openSUSE's multimedia documentation for the current instructions.

## Debian, Ubuntu and derivatives

Use a recent release: Kdenlive 26.08 will probably not build on an older LTS, because its Qt 6 and
KDE Frameworks 6 are too old.

~~~
sudo apt install git cmake ninja-build g++ extra-cmake-modules
sudo apt build-dep kdenlive
~~~

`apt build-dep` needs `deb-src` lines enabled in your sources (on Ubuntu: Software & Updates,
tick "Source code"). If the dependencies of the packaged Kdenlive differ from what 26.08 needs,
CMake will name the missing ones.

Effect plugins:

~~~
sudo apt install libmlt-dev qt6-base-dev libx11-dev cmake g++ pkg-config
~~~

Library folders: Debian-based systems usually install into `lib/x86_64-linux-gnu`, which is not in
the run script's `LD_LIBRARY_PATH`. If the program does not start with "cannot open shared object
file", add that folder to `LD_LIBRARY_PATH` in `extras/run-better-kdenlive.sh`, and if it also
cannot find Qt plugins, set `QT_PLUGIN_PATH` to the `plugins` folder inside the install prefix.
The MLT tool may be called `melt` or `melt-7`.

## Arch Linux and derivatives

Arch has no `build-dep` command. Installing the packaged Kdenlive brings in the libraries it
needs, and Arch normally ships development headers together with the libraries:

~~~
sudo pacman -S --needed base-devel git cmake ninja extra-cmake-modules kdenlive
~~~

The installed `kdenlive` package is not used and does not conflict with Better Kdenlive, which
lives in its own prefix. If CMake still reports missing packages, install them as described above.

Effect plugins:

~~~
sudo pacman -S mlt qt6-base libx11 cmake gcc pkgconf
~~~

## Other distributions

The steps do not change. Install the toolchain, CMake and Ninja, find the development packages
CMake asks for, and build into your home directory. Flatpak, AppImage and Snap builds of
Kdenlive bundle their own MLT and cannot use the effect plugins. Windows and macOS are not
supported.

## If it works (or not)

Please report the distribution and version, the output of `pkg-config --modversion mlt-framework-7`,
and the first error from the build, in an issue on this repository.
