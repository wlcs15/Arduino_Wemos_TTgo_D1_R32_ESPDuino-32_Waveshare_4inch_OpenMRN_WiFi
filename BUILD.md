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

## Wi-Fi password (host-encrypt, ciphertext only)

Do **not** create `wifi_secrets.env`. Do **not** put the PSK on the command line. Do **not** paste it into chat or `idf.py menuconfig`.

Grok and any non-interactive build use `utils/build_idf5.sh`. That script never prompts and never reads a PSK.

```bash
# 1) Password-free DEBUG image (Grok can flash this)
./utils/build_idf5.sh -p /dev/ttyUSB0 flash

# 2) Harvest MAC, OpenLCB node ID, flash UID from serial
./utils/collect_hw_ids.py --port /dev/ttyUSB0
# writes gitignored local/hw_ids.env

# 3) YOUR interactive terminal only
./utils/provision_wifi_build.sh                 # hidden prompt, host-encrypt, build
./utils/provision_wifi_build.sh -p /dev/ttyUSB0 flash
```

`provision_wifi_build.sh` refuses a non-TTY, being sourced, a pre-set `WIFI_PASSWORD`, or `--password`. It writes only `main/wifi_psk_wrap.inc` (ciphertext, gitignored). The firmware decrypts with **live** chip IDs and copies the wrap into NVS. Later `./utils/build_idf5.sh` flashes keep NVS.

To wipe the chip **and** the host wrap (the secret):

```bash
./utils/erase_all_flash.sh -p /dev/ttyUSB0
```

That is a full `erase-flash` (app + NVS) plus deletion of `main/wifi_psk_wrap.inc`. Afterward flash a wrap-free image and provision again if you want Wi-Fi.

Fake-data check (no hardware, no real PSK):

```bash
./utils/test_wifi_wrap.sh
```

**Limit:** `local/hw_ids.env` plus the wrap file (or that one `.bin`) can reconstruct the PSK. Keep both gitignored. After NVS is written you can delete `main/wifi_psk_wrap.inc` and rebuild password-free.

The wrap key is HKDF-SHA256 of flash unique id ∥ MAC ∥ node, info `05.01.01.01.A5` ∥ MAC. Not Flash Encryption.

## First configure and build (no password)

```bash
cd ~/Git/wlcs15/Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi
git submodule update --init --recursive
chmod +x utils/build_idf5.sh utils/provision_wifi_build.sh
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
