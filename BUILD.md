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

## Wi-Fi secrets (do not commit, do not paste in chat)

```bash
cd ~/Git/wlcs15/Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi
cp wifi_secrets.env.example wifi_secrets.env
# edit wifi_secrets.env locally — it is gitignored
chmod 600 wifi_secrets.env
```

SSID default is `SRIF2333` (public). Put the **PSK only** in `wifi_secrets.env`.

First boot with that build wraps the PSK into NVS using AES-256-GCM. Later `./utils/build_idf5.sh flash` keeps NVS. **Do not** `erase-flash` unless you intend to provision again.

The wrap key is HKDF-SHA256 of this module’s flash unique id + MAC, mixed with OwlThree prefix `05.01.01.01.A5`. That is not plaintext in git; it is not Flash Encryption.

## First configure and build

```bash
cd ~/Git/wlcs15/Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi
git submodule update --init --recursive
chmod +x utils/build_idf5.sh
./utils/build_idf5.sh set-target esp32
./utils/build_idf5.sh build
./utils/build_idf5.sh -p /dev/ttyUSB0 flash monitor
```

## Submodules

| Path | Upstream | License |
|------|----------|---------|
| `components/OpenMRNIDF` | https://github.com/atanisoft/OpenMRNIDF | BSD-2-Clause |

Application code in this repo is BSD-2-Clause (see `LICENSE`). Do not vendor a second copy of OpenMRN.

`components/OpenMRNIDF` tracks atanisoft’s **5.1.0** branch (ESP-IDF v5.1.x). CMake applies `patches/OpenMRNIDF-idf51-mdns.patch` so the component REQUIRES the registry `mdns` package. That one-line edit stays local. Do not push it to atanisoft.

## DEBUG ID screen

With `DEBUG=1` (default in `main/CMakeLists.txt`) boot paints MAC, OpenLCB node ID, and SPI flash unique ID on the 4" panel and prints the same three lines on serial. The Wi-Fi PSK is never displayed or logged.

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
