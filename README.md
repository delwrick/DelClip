# DelClip

A saturation and soft-clipping VST3/Standalone audio plugin built with [JUCE](https://juce.com) and CMake.

![Plugin GUI](https://raw.githubusercontent.com/delwrick/DelClip/main/screenshot.png)

## Features

- **Drive** — 0–100% pre-gain fed into a tanh waveshaper for smooth, musical soft clipping
- **Tone** — State-variable low-pass filter (500 Hz – 20 kHz) to tame harshness after saturation
- **Output Level** — -24 to +12 dB output trim to compensate for loudness changes
- Stereo in/out, no latency
- Minimal dark UI

## Formats

| Format | Platform |
|--------|----------|
| VST3 | Windows |
| Standalone | Windows |

## Building from Source

### Prerequisites

- [CMake](https://cmake.org) 3.22+
- Visual Studio 2022 with **Desktop development with C++** workload

### Steps

```bash
# Clone with submodules (includes JUCE)
git clone --recurse-submodules https://github.com/delwrick/DelClip.git
cd DelClip

# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Release
```

The built plugin will be at:
```
build/DelClip_artefacts/Release/VST3/DelClip.vst3
build/DelClip_artefacts/Release/Standalone/DelClip.exe
```

### Installation

Copy `DelClip.vst3` to `C:\Program Files\Common Files\VST3\` and rescan plugins in your DAW.

## License

MIT
