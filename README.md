# Doctor Migrana – Tremolo

A free guitar-pedal style **tremolo** plugin (VST3 + Standalone) for Linux, Windows and macOS,
inspired by the bias-modulating tremolo ("vibrato") of the brownface Deluxe 6G3 amp.

| Knob | What it does |
|------|--------------|
| **Speed** | LFO rate, 0.5 – 12 Hz |
| **Intensity** | Depth of the tremolo (gain + bias modulation, with asymmetric tube-like saturation) |
| **Presence** | High-frequency lift (0 – +9 dB shelf at 3.2 kHz) |

The footswitch toggles bypass; the LED shows whether the effect is on.

## Download

Get the latest build for your system from the **Releases** page.

| System | Copy the `.vst3` folder to |
|--------|----------------------------|
| Linux  | `~/.vst3/` |
| Windows | `C:\Program Files\Common Files\VST3\` |
| macOS  | `~/Library/Audio/Plug-Ins/VST3/` |

Then re-scan plugins in your DAW. The standalone app is in the `Standalone` folder.

**Unsigned binaries:** the builds are not code-signed.
On macOS run `xattr -dr com.apple.quarantine "Doctor Migrana - Tremolo.vst3"` (and the app) if Gatekeeper blocks them;
on Windows SmartScreen may show a warning.

## Build from source

Requirements: CMake ≥ 3.22, git, a C++17 compiler. JUCE is downloaded automatically.

```bash
# Linux only (Debian/Ubuntu):
sudo apt install build-essential cmake git libasound2-dev libjack-jackd2-dev libfreetype-dev \
  libfontconfig1-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxext-dev libxrender-dev libxcomposite-dev libglu1-mesa-dev mesa-common-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Outputs are in `build/DoctorMigranaTremolo_artefacts/Release/`.
By default the VST3 is also copied to your user plugin folder; disable with `-DMIGRANA_COPY_AFTER_BUILD=OFF`.

## Releases (maintainers)

Pushing a tag such as `v0.1.0` makes GitHub Actions build all three platforms and attach the zips to a Release:

```bash
git tag v0.1.0 && git push origin v0.1.0
```

## Licence

- **Code:** GNU AGPL v3 — see [`LICENSE`](LICENSE).
- **Artwork:** the pedal image was **generated with Meta AI (Meta Platforms, Inc.)** and edited. It is distributed with
  this open-source, non-commercial project with attribution to Meta, and is *not* covered by the AGPL.
  The "Doctor Migrana" name and logo are © GuitarFros, all rights reserved. Forks must replace the artwork for any commercial use.
- Third-party notices and trademarks: see [`NOTICE.md`](NOTICE.md).

---

## En galego

Plugin de trémolo gratuíto en formato pedal (VST3 e Standalone), inspirado no trémolo de bias do amplificador
Deluxe 6G3. Controis: **Speed**, **Intensity** e **Presence**. Descarga os binarios na páxina de *Releases*
ou compila co CMake como se indica arriba. O código está baixo licenza AGPLv3. A imaxe do pedal foi xerada con Meta AI (Meta Platforms, Inc.) e distribúese con atribución a Meta para uso non comercial; o logo "Doctor Migrana" ten todos os dereitos reservados.
