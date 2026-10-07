# Better Kdenlive

An **unofficial, personal fork** of [Kdenlive](https://kdenlive.org) (based on **v26.08.1**)
that adds a few features I missed coming from Premiere Pro.

> **Not affiliated with KDE or the Kdenlive project.** "Kdenlive" is their name and work;
> this fork only adds a handful of changes on top. Please do not report bugs of this
> fork to the official Kdenlive bug tracker.

## What is added

- **Caption animations** (subtitle editor): a menu with ready-made animations for
  captions (Fade, Pop-in, Bounce, Karaoke word-by-word, Remove animation), plus an
  **"Apply to all subtitles"** option to animate a whole video in one click.
  Karaoke timing is estimated from word length, not from real word timestamps.
  Bulk changes are not guaranteed to be undoable: save your project first.
- **Adjustment layer**: *Add Adjustment Layer* in the Bin creates a transparent clip.
  Put effects on it, then right-click the clip and choose **Apply Effects to Tracks
  Below**. The effects are copied to the video tracks below, limited to the clip's
  range (keyframed effects work). Deleting the clip removes them again; applying twice
  replaces the previous copy; undo/redo is supported and the link survives saving and
  reopening the project.
- **Floating** and **Transform + Motion Blur** effect definitions (see requirements below).

## Known limitations

- The adjustment layer is a *copy*, not a live link: after moving or resizing the clip,
  or changing an effect parameter, press **Apply Effects to Tracks Below** again.
- Duplicating (copy/paste) the adjustment layer clip shares its effects. Create a new
  one from the Bin for each use.
- Built-in effects such as Transform exist once per track, so two adjustment layers
  using Transform on the same track overwrite each other.
- Only video tracks below the clip are affected.
- Tested only on Fedora 44 (MLT 7.40, Qt 6.11), building with the system MLT.

## Extra requirements for the two effects

The XML definitions are included, but the effects themselves are MLT plugins that must be
compiled and installed separately (they need the system MLT, not Flatpak/AppImage):

- Floating: https://github.com/marcoporz/floating-shaking-kdenlive-effect
- Transform + Motion Blur (shutter angle): https://github.com/marcoporz/kdenlive-shutter-angle-motionblur-effect
  (based on the unmerged MLT pull request mltframework/mlt#1301)

Both have an `install.sh`. Without the plugins the effects show up but do nothing.
You can ignore the step that copies the XML into `~/.local/share/kdenlive/effects`,
since the definitions are already built in here.

## Building (Fedora)

~~~
git clone https://github.com/marcoporz/better-kdenlive.git
cd better-kdenlive
git checkout better-kdenlive
~~~

Dependencies. If you use the full ffmpeg from RPM Fusion, `builddep` conflicts with the
`ffmpeg-free-devel` packages; this worked:

~~~
sudo dnf install ffmpeg-devel
sudo dnf builddep kdenlive --skip-unavailable --exclude=ffmpeg-free-devel --exclude=libavcodec-free-devel
~~~

Build and install into your home directory (no `sudo`, nothing system-wide is touched):

~~~
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=$HOME/better-kdenlive-install
cmake --build build
cmake --install build
~~~

Run it with its own config, data and cache, so it stays separate from any normal Kdenlive:

~~~
BK_DIR=$HOME/better-kdenlive-install extras/run-better-kdenlive.sh
~~~

Because it uses its own data folder, fonts in `~/.local/share/fonts` are not seen
automatically: `ln -s ~/.local/share/fonts $BK_DIR/data/fonts` fixes that.

## License

Same as Kdenlive (GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL, see `LICENSES/`).
The two plugin repositories linked above are LGPL-2.1-or-later.

## Credits

Kdenlive is developed by the Kdenlive team and contributors. This fork adds the changes
described above on top of their work.
