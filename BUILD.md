# Build notes

**Required ESP-IDF: v5.1.6 only.** Target is classic **esp32** (Wemos D1 R32), not esp32s3.

Do not configure this tree with ESP-IDF 5.3+ or 6.x. OpenMRN does not compile cleanly on newer GCC/newlib. That is a hard constraint.

## Activate the toolchain

```bash
export IDF_PATH=~/esp/esp-idf-v5.1.6
. "$IDF_PATH/export.sh"
# or, if you use the local alias: esp5
```

Confirm:

```bash
idf.py --version    # must show v5.1.6
```

## First configure and build

```bash
cd ~/Git/wlcs15/Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi
git submodule update --init --recursive
idf.py set-target esp32
idf.py menuconfig   # optional: WiFi SSID / JMRI hub under "D1 R32 OpenMRN WiFi node"
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Submodules

| Path | Upstream | License |
|------|----------|---------|
| `components/OpenMRNIDF` | https://github.com/atanisoft/OpenMRNIDF | BSD-2-Clause |

Application code in this repo is BSD-2-Clause (see `LICENSE`). Do not vendor a second copy of OpenMRN.

## Hardware (this phase)

- Wemos TTgo D1 R32 / ESPDuino-32
- Coowell / Waveshare 4" ILI9486 shield may stay seated
- SPI DIP switches: **D11 / D12 / D13**, not ICSP
- No CAN transceiver yet (WiFi GridConnect only)

## Related local projects

| Project | IDF | Role |
|---------|-----|------|
| This repo | **5.1.6** / esp32 | OpenMRN WiFi node |
| `…/vsi5004/LCCLightingTouchscreen` | 5.1.6 / esp32s3 | Lighting UI (different glass) |
| `…/bobscott45/esp32_lever_frame` | 6.1 | Lever frame (S3/P4 4.3") |
