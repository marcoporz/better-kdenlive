# Better Kdenlive

An **unofficial, personal fork of [Kdenlive](https://kdenlive.org) 26.08.1** that adds a few
things I missed coming from Premiere Pro: an **adjustment layer**, **animated captions** for
short-form video, and two motion effects, **Floating** and **Transform + Motion Blur**.

> **Not affiliated with KDE or the Kdenlive project.** "Kdenlive" is their name and their work;
> this fork only adds a few changes on top. Please do not report problems of this fork to the
> official Kdenlive bug tracker. Use this repository's issues instead.

## Contents

1. [What is added](#what-is-added)
2. [How to use each feature](#how-to-use-each-feature)
3. [Installation on Fedora (tested)](#installation-on-fedora-tested)
4. [Other distributions](#other-distributions)
5. [Troubleshooting](#troubleshooting)
6. [Known limitations and testing status](#known-limitations-and-testing-status)
7. [Roadmap](#roadmap-using-the-features-with-official-kdenlive)
8. [Updating and uninstalling](#updating-and-uninstalling)
9. [License and credits](#license-and-credits)

## What is added

- **Caption animations**: a button in the subtitle editor with ready-made animations
  (Fade, Pop-in, Bounce, Karaoke word by word) and an **"Apply to all subtitles"** option,
  so a whole video can be animated in one click.
- **Adjustment layer**: *Add Adjustment Layer* creates a transparent clip. Put effects on it
  and apply them to every video track below, only for the length of the clip.
- **Floating**: a gentle, automatic hovering motion (position and rotation), no keyframes needed.
- **Transform + Motion Blur**: position, scale and rotation with real motion blur and an
  adjustable shutter angle.

Everything else is plain Kdenlive 26.08.1. The version shows up as "Better Kdenlive" in the
title bar and in *Help > About*.

## How to use each feature

### Caption animations

1. Create or import subtitles as usual (for example with Kdenlive's automatic speech
   recognition), and select a subtitle on the timeline so the subtitle editor shows it.
2. In the editor's toolbar, next to the bold / italic / underline / font buttons, click the
   **magic wand button** ("Caption animation").
3. Pick an animation:
   - **Fade in/out**: quick fade at the start and at the end of the caption.
   - **Pop-in**: the caption pops up from small to slightly larger and settles to normal size.
   - **Bounce**: a short elastic bounce when the caption appears.
   - **Karaoke (word by word)**: the caption's duration is split between its words,
     proportionally to word length, and each word changes colour when it is "spoken".
     The colours come from the subtitle style: the **secondary colour** before the word
     is reached and the **primary colour** after. Change them in the style editor.
   - **Remove animation**: takes the animation away again.
4. To animate **all** captions at once, tick **Apply to all subtitles** in the same menu and then
   choose an animation. It is applied to every subtitle in the project, on every subtitle layer.

How it works: the animation is written as standard ASS override tags at the start of the
subtitle text, inside a block starting with `{kd-anim:...`. You can see and edit them in the
normal editor (the "Simple Editor" mode hides them). Choosing another animation replaces the
previous one and keeps your own bold, colours and position tags.

Notes: save your project before using "Apply to all", because this bulk change may not be
undoable with Ctrl+Z. The Karaoke timing is an estimate based on word length, not the real
time of each word.

### Adjustment layer

An adjustment layer lets you put one effect (colour correction, Transform, blur, anything)
on a piece of the timeline that covers several tracks.

1. In the **Project Bin**, open the **Add** menu (the arrow next to the Add Clip button) and choose
   **Add Adjustment Layer**. A transparent clip called "Adjustment Layer" appears in the Bin.
2. Drag it to a video track **above** the clips you want to affect (for example V2, above V1),
   and trim it so it covers exactly the part you want.
3. Select the adjustment layer clip and add your effects to it in the Effects panel, with
   keyframes if you like (Transform with animation works).
4. Right-click the clip and choose **Apply Effects to Tracks Below**. A message tells you
   how many effects were copied. The effects now act on the video tracks below, but **only
   inside the range of the clip**.
5. Whenever you **move or resize the clip, or change an effect parameter**, run
   **Apply Effects to Tracks Below** again. The previous copy is replaced, not duplicated.
6. **Deleting the clip removes the effects** from the tracks below. Ctrl+Z brings back both.

How it works: the effects are copied onto the track below as track effects limited to a
zone, and tagged with a unique id stored in the clip, so they can be found again later,
even after saving, closing and reopening the project.

Things to know: it is a copy, not a live link (see step 5); only video tracks below the clip
are affected; copy/pasting the adjustment layer clip makes the copy share the same effects,
so create a new one from the Bin for each use; a built-in effect such as Transform exists once
per track, so two adjustment layers using Transform on the same track overwrite each other.

### Floating

A subtle hovering motion for logos, PNGs, overlays or footage, with no keyframes: it runs for
the whole clip.

1. Select a clip and add **Floating** from the Effects panel.
2. Set **Horizontal / Vertical / Rotation Amplitude** (how far it drifts) and **Speed**.
   Low speeds (about 0.1 to 0.3) look slow and dreamy, higher values look nervous.
3. If two clips use Floating at the same time, give them different **Seed** values so
   they do not move in sync.
4. **Rotation Pivot X / Y** (0 to 1, default 0.5 = centre) set the point the rotation turns around.

The whole incoming frame is nudged as one image. To place and size the clip, use Kdenlive's
own Transform effect before it.

### Transform + Motion Blur

Like the normal Transform effect, but moving things leave a natural motion trail, like a real
camera, instead of strobing.

1. Select a clip and add **Transform + Motion Blur**.
2. Animate **Rectangle** (position and size) and/or **Rotation** with at least two keyframes,
   as you would with Transform.
3. **Shutter Angle**: 0 to 360 degrees, default 180 (the "film" look). **0 turns the blur off**
   without losing your keyframes.
4. **Blur Quality** is the number of samples (default 16): lower it on slow machines, raise it
   if you see banding in the trail.
5. **Rotation Pivot X / Y** (0 to 1) set the rotation point.

Limits: 8-bit colour only, and no blend modes. Motion is estimated from the interpolation
between keyframes, so very sharp easing curves may blur slightly differently from a true
per-subframe render.

> **Floating and Transform + Motion Blur need extra plugins.** Their definitions are included
> here, but the effects themselves are MLT plugins you install separately (step B below).
> Without them the effects appear in the list but do nothing.

## Installation on Fedora (tested)

Tested on Fedora 44 (MLT 7.40, Qt 6.11). The first build is long (about an hour on my
machine); later rebuilds only recompile what changed. Nothing is installed system-wide for
Better Kdenlive itself, and it keeps its own settings, so your normal Kdenlive stays untouched.

### A. Build Better Kdenlive

Dependencies:

~~~
sudo dnf install git cmake ninja-build gcc-c++ extra-cmake-modules dnf-plugins-core
sudo dnf builddep kdenlive
~~~

If `builddep` fails with a conflict between `ffmpeg-free-devel` and RPM Fusion's `ffmpeg-libs`,
do **not** use `--allowerasing` (it can remove your installed Kdenlive). This worked instead:

~~~
sudo dnf install ffmpeg-devel
sudo dnf builddep kdenlive --skip-unavailable --exclude=ffmpeg-free-devel --exclude=libavcodec-free-devel
~~~

Get the code. This is a fork of Kdenlive, so the download includes its history (about 1 GB):

~~~
mkdir -p ~/better-kdenlive && cd ~/better-kdenlive
git clone --branch better-kdenlive --single-branch https://github.com/marcoporz/adjustment-layer-and-captions-animations-for-kdenlive.git src
cd src
~~~

Build and install into your home directory (no `sudo`):

~~~
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=$HOME/better-kdenlive/install
cmake --build build
cmake --install build
~~~

Run it:

~~~
~/better-kdenlive/src/extras/run-better-kdenlive.sh
~~~

The script gives it its own config, data and cache folders under `~/better-kdenlive/`
(`config`, `data`, `cache`). To reset everything, delete those three folders.

### B. Install the two effect plugins

Needed only for **Floating** and **Transform + Motion Blur**. They are compiled against the
system MLT and installed into MLT's module directory (this step uses `sudo`; the normal Kdenlive
will also see them, which is harmless).

~~~
sudo dnf install mlt-devel qt6-qtbase-devel libX11-devel cmake gcc-c++ pkgconf-pkg-config

cd ~/better-kdenlive
git clone https://github.com/marcoporz/floating-shaking-kdenlive-effect.git
cd floating-shaking-kdenlive-effect && ./install.sh && cd ..

git clone https://github.com/marcoporz/kdenlive-shutter-angle-motionblur-effect.git
cd kdenlive-shutter-angle-motionblur-effect && ./install.sh && cd ..
~~~

You can ignore the part of their scripts that copies XML files into
`~/.local/share/kdenlive/effects`: Better Kdenlive does not read that folder, because the
definitions are already built in. Check that MLT sees the plugins (the command is called
`melt`, `melt-7` or `mlt-melt` depending on the distribution):

~~~
melt-7 -query filters | grep -E "floating|shutterblur"
~~~

You should see `floating` and `shutterblur`. Then restart Better Kdenlive.

### C. Menu entry and taskbar (optional)

The file name matters: Better Kdenlive identifies itself as `better-kdenlive`, so the taskbar
only pins the right program if the file is called `better-kdenlive.desktop`.

~~~
mkdir -p ~/.local/share/applications
printf '%s\n' \
'[Desktop Entry]' \
'Type=Application' \
'Name=Better Kdenlive' \
'Comment=Kdenlive with caption animations and an adjustment layer' \
"Exec=$HOME/better-kdenlive/src/extras/run-better-kdenlive.sh %U" \
'Icon=kdenlive' \
'Terminal=false' \
'Categories=AudioVideo;Video;AudioVideoEditing;' \
> ~/.local/share/applications/better-kdenlive.desktop
~~~

Fonts: because it uses its own data folder, fonts installed in `~/.local/share/fonts`
are not seen automatically. This fixes it:

~~~
mkdir -p ~/better-kdenlive/data
ln -s ~/.local/share/fonts ~/better-kdenlive/data/fonts
~~~

## Other distributions

Only Fedora 44 has been tested. A step-by-step guide for **openSUSE (Slowroll and Tumbleweed)**,
**Debian and Ubuntu**, **Arch** and others is in
[docs/INSTALL-OTHER-DISTROS.md](../docs/INSTALL-OTHER-DISTROS.md). It is untested: it documents the
method, and reports from people who try it are welcome.

## Troubleshooting

- **The effects Floating or Transform + Motion Blur show up but do nothing.** The plugins are
  not installed, or MLT cannot find them. Run the `melt-7 -query filters` check in step B.
- **`builddep` complains about ffmpeg.** See the workaround in step A.
- **The taskbar pins the normal Kdenlive.** Check the file is named exactly
  `better-kdenlive.desktop`, unpin the old icon, then pin again from the running Better Kdenlive window.
- **An effect appears twice in the list.** You probably copied its XML into
  `~/better-kdenlive/data/kdenlive/effects`. Remove that copy: the effect is already built in.
- **Titles or captions use the wrong font.** Link your fonts folder (step C).
- **The adjustment layer does not follow the clip.** After moving, resizing or changing an
  effect, run **Apply Effects to Tracks Below** again.
- **A caption looks full of `{...}` text.** Those are the animation tags. Turn on the
  "Simple Editor" mode, or use **Remove animation**.

## Known limitations and testing status

Tested by me on Fedora 44: the caption menu and "Apply to all" in the project monitor and in an exported video,
the adjustment layer (apply, move and re-apply, delete, undo/redo, save and reopen), and both
effects.

**Not tested:** opening a project that
uses the adjustment layer in the normal Kdenlive, other distributions, and very long or
high-resolution projects. Check your renders before relying on them.

## Roadmap: using the features with official Kdenlive

These are ideas, not promises. The goal is to make the features usable **without replacing your
official Kdenlive**, and without breaking it. This fork never touches an existing install: it has its
own folder and its own settings.

- **Floating and Transform + Motion Blur: already possible.** They are MLT plugins plus an effect
  definition, so they work with the official Kdenlive as long as it uses the system MLT (not the Flatpak
  or AppImage builds). Install the two plugin repositories linked above; this fork is not needed for them.
- **Caption animations: planned as a separate tool.** They are a user-interface change inside Kdenlive's
  subtitle editor, which cannot be added to the official program as a plugin. The plan is a small standalone
  tool that applies the same animations to an `.ass` subtitle file exported from Kdenlive, which you then import
  back. Status: not started. It should work because the rendering is done by Kdenlive's own subtitle code,
  but this has not been verified on the official build.
- **Adjustment layer: needs changes inside Kdenlive.** It touches the timeline model, so it cannot be a
  separate tool in a clean way. Rewriting project files from outside would depend on the project format and
  is not planned.
- **Proposing the changes upstream.** The proper way to get these features into the official Kdenlive is to
  propose them to the Kdenlive developers on [invent.kde.org](https://invent.kde.org/multimedia/kdenlive),
  one feature at a time, rebased on their current development branch, following their coding rules.
  Acceptance is not guaranteed. Until then, this fork stays available as it is.

## Updating and uninstalling

This fork is based on Kdenlive 26.08.1 and does not promise to follow upstream releases.
To update to a newer commit of this fork: `git pull`, then `cmake --build build` and
`cmake --install build`.

To uninstall: delete `~/better-kdenlive` and `~/.local/share/applications/better-kdenlive.desktop`.
The plugins are removed as described in their own repositories.

## License and credits

Better Kdenlive keeps Kdenlive's license (GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL, see
the `LICENSES` folder). The two plugin repositories are LGPL-2.1-or-later.
Transform + Motion Blur is based on the unmerged MLT pull request
[mltframework/mlt#1301](https://github.com/mltframework/mlt/pull/1301); see the credits in
its repository.

Kdenlive is developed by the Kdenlive team and contributors. This fork adds the changes described
above on top of their work.
