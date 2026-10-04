# Bad Apple Desktop Wallpaper

A tiny native **Windows app** that plays the [Bad Apple!!](https://en.wikipedia.org/wiki/Bad_Apple!!) animation **on your wallpaper**, like Wallpaper Engine — but written from scratch in C with [raylib](https://www.raylib.com/) and [pl_mpeg](https://github.com/phoboslab/pl_mpeg), with no external runtime dependencies.

> **This is a fan-made, non-commercial tribute project. It is not affiliated with ZUN, Team Shanghai Alice, or the original animation's creators.**

I saw this as a fun little challanging project idea so I decided to make it, and it was well worth it.

## Features

- Renders behind desktop icons via the undocumented `WorkerW` reparenting trick, with a fallback for the newer Windows 11 (24H2+) desktop layout
- Click-through and taskbar-icon-free (animated wallpaper)
- Background rendered as fully transparent, only the black silhouette is drawn over your actual desktop wallpaper
- Audio/video playback synced to the system audio clock to avoid drift over the full runtime on potencially slower mashines
- Hold <kbd>Esc</kbd> to close, a simple indicator on the right top of the screen

## How it works

1. A single-header MPEG-1 decoder ([pl_mpeg](https://github.com/phoboslab/pl_mpeg)) decodes the video frame-by-frame; only the luma (brightness) plane is read, since the source is pure black and white
2. Each pixel is thresholded at runtime: bright pixels become fully transparent, dark pixels become opaque black
3. The resulting pixel buffer is uploaded to a raylib `Texture2D` and drawn every frame
4. A Win32 `WorkerW`/`Progman` reparenting (in `win32_helpers.c`) moves the raylib window behind the desktop icons at startup
5. Audio plays via raylib's music-stream API (video decoding is in sync with `GetMusicTimePlayed()` so it never drift out of sync)

## Building it yourself

You'll need your own copy of a Bad Apple source video — this repo does not include one. Grab one from wherever you like, then run the following from the project folder. You'll also need your own `libraylib.a` file from the official build.

**1. Compress and convert to grayscale**

Scales the video down and strips color information, because the original source content is pure black and white (to save space):

```bash
ffmpeg -i BadApple.mp4 -vf scale=480:360,format=gray compressed.mp4
```

**2. Split video and audio**

The video is re-encoded as a bare MPEG-1 video with no sound, and the audio is pulled out separately so it can be played and synced independently:

```bash
ffmpeg -i compressed.mp4 -c:v mpeg1video -an video.m1v
ffmpeg -i compressed.mp4 -map_metadata -1 -vn audio.ogg
```

**3. Embed both files as C headers**

This bakes the video and audio directly into the executable as byte arrays. The final build will be a single portable `.exe`:

```bash
xxd -i video.m1v > data_video.h
xxd -i audio.ogg > data_audio.h
```

> `xxd` ships with [Git for Windows](https://gitforwindows.org/) (Git Bash) if you don't have it installed.

**4. Compile**

```bash
gcc -o main main.c win32_helpers.c battery.c -L./lib -I./include -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows
```

- `-mwindows` is used to make it a GUI app (no console)
- `-L./lib` / `-I./include` point at your local raylib build and headers — see [raylib's releases](https://github.com/raysan5/raylib/releases) for a prebuilt MinGW `libraylib.a`, or build from source

## Dependencies

| Library | License | Notes |
|---|---|---|
| [raylib](https://www.raylib.com/) | zlib/libpng | Windowing, rendering, audio |
| [pl_mpeg](https://github.com/phoboslab/pl_mpeg) | MIT | Single-header MPEG-1 video decoder |

Full license text for both is included in [`licenses/`](./licenses).

## License

This project's own code is licensed under the [MIT License](./LICENSE). See [`licenses/`](./licenses) for the licenses of bundled third-party libraries.

The Bad Apple!! animation and song are the property of their original creators and are not covered by this project's license.
