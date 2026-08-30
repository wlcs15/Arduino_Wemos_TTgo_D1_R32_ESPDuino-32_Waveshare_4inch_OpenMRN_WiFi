# Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi

OpenMRN (NMRA LCC / OpenLCB) node for the **Wemos TTgo D1 R32 / ESPDuino-32** and the Coowell / Waveshare **4" ILI9486 resistive touch** Arduino shield.

This phase is **Wi-Fi STA + OpenMRN GridConnect** to this laptop's JMRI hub (no TWAI/CAN yet). SPI DIP switches must be **D11 / D12 / D13** (this Wemos does not wire ICSP).

## What is in this baseline

- `DEBUG=1` ID screen and serial: MAC, OpenLCB node ID, SPI flash unique ID
- `WIFI PASSWORD:` line — `NOT SET`, `SET (NOT SHOWN)`, or `UNWRAP FAIL` (never the PSK)
- Host-side AES-256-GCM wrap of the house PSK; only ciphertext is compiled in
- First boot copies that wrap into **NVS**; later app flashes keep NVS
- ESP-IDF STA join after unwrap, with serial status and a 3-bar icon in the upper right (searching / connected / failed)
- OpenMRNIDF **5.1.0** submodule on ESP-IDF **v5.1.6** / `esp32`
- Defaults: node **05.01.01.01.A5.01**, tag **`v1.0.1`**. Hub is mDNS `_openlcb-can._tcp` (optional static fallback empty)

## License

Application code in this repository is **BSD-2-Clause** (see `LICENSE`).  
`components/OpenMRNIDF` is atanisoft’s **5.1.0** tree plus a one-line `mdns` REQUIRES for IDF 5.1. The commit lives on this repo’s `vendor/OpenMRNIDF-5.1.0` branch (not pushed to atanisoft). License remains BSD-2-Clause.

## Build

See [BUILD.md](BUILD.md). Use **ESP-IDF v5.1.6** and `idf.py set-target esp32`.

## Scripts (`utils/`)

| Script | Who runs it | Purpose |
|--------|-------------|---------|
| `build_idf5.sh` | Grok or any shell | Activate IDF 5.1.6 and run `idf.py`. Never prompts for a password. Bakes `main/wifi_psk_wrap.inc` only if that file already exists. |
| `collect_hw_ids.py` | Either | Parse DEBUG serial (or `--from-log`) for MAC, node ID, and flash UID. Writes gitignored `local/hw_ids.env`. Does not handle the PSK. |
| `wifi_wrap.py` | Called by provision / tests | HKDF-SHA256 + AES-256-GCM matching `wifi_cred.cpp`. Encrypts a PSK from stdin; writes ciphertext only. `selftest` uses fake data. |
| `provision_wifi_build.sh` | **Your interactive terminal only** (Ubuntu / Git Bash) | Hidden SSID/PSK prompts (not bash history, not argv). Encrypts on the host, writes `main/wifi_psk_wrap.inc`, then builds/flashes. Refuses Grok, pipes, and a pre-set `WIFI_PASSWORD`. |
| `provision_wifi_build.py` | **Your interactive terminal only** (Windows cmd or Ubuntu) | Same refusals and hidden prompts via `getpass`. Use `python -u`. On Windows, wrap is written here; IDF flash still needs a shell that can source ESP-IDF. |
| `erase_all_flash.sh` | Either | Full `idf.py erase-flash` (app + NVS wrap) and shreds host wrap/`wifi_secrets.env`. Leaves `local/hw_ids.env`. Chip is blank afterward. |
| `test_wifi_wrap.py` | Either | Same fake-data check, runs on Win11 cmd (`python -u`). Never uses the house PSK. |
| `test_wifi_wrap.sh` | Either | Same checks plus the bash provisioner's non-TTY refusal. Never uses the house PSK. |

Typical bring-up (from the **repo root**):

```bash
./utils/build_idf5.sh -p /dev/ttyUSB0 flash
python -u utils/collect_hw_ids.py --port /dev/ttyUSB0
# then, in YOUR terminal only:
./utils/provision_wifi_build.sh -p /dev/ttyUSB0 flash
```

Windows 11 cmd (CH340 is often COM7):

```bat
python -u utils\collect_hw_ids.py --port COM7
python -u utils\provision_wifi_build.py
```

Wipe chip and host secret:

```bash
./utils/erase_all_flash.sh -p /dev/ttyUSB0
```

## Hardware you need for this phase

- D1 R32 (CH340; Ubuntu `ttyUSB*` numbers swap — confirm MAC `a4:f0:0f:73:97:3c`. Opening the port with DTR resets the ESP32; `stty -F PORT -hupcl` before a serial monitor)
- House Wi-Fi (SSID **SRIF2333** is public; PSK is not)
- **This laptop JMRI OpenLCB hub** (`_openlcb-can._tcp`, TCP 12021)
- 4" shield seated, DIPs **not** on ICSP
- Not yet: a CAN transceiver (#3A)

## OpenLCB node ID

OwlThree is listed in the [OpenLCB unique ID registry](https://registry.openlcb.org/uniqueidranges) as:

**05.01.01.01.A5.*** — 256 IDs (`05.01.01.01.A5.00` … `05.01.01.01.A5.FF`)

This firmware defaults to **05.01.01.01.A5.01**. Change it in `idf.py menuconfig` → *D1 R32 OpenMRN WiFi node*. Keep `.02` and up for more boards or the later CAN node. Do not use `03.00.AB.01.*`.

## Wi-Fi secret

- The PSK is **never** a git default, **never** typed into Grok, and **never** compiled in as plaintext.
- Host wrap key = HKDF-SHA256(flash unique id ∥ MAC ∥ node), info `05.01.01.01.A5` ∥ MAC. Decrypt uses **live** chip IDs.
- `local/hw_ids.env` plus the wrap file (or a provisioning `.bin`) can reconstruct the PSK. Both stay gitignored. This is “not plaintext in the `.bin`,” not dump-proof.
- After NVS is written, later wrap-free `build_idf5.sh` flashes keep NVS. Do not `erase-flash` unless you intend to provision again (or run `erase_all_flash.sh`).

## Status

Tag **`v1.0.1`** (pushed). OpenMRN GridConnect TCP client joins this laptop’s JMRI hub after STA is up (mDNS `_openlcb-can._tcp`, then empty Kconfig fallback). Node **05.01.01.01.A5.01**. SNIP manufacturer **OwlThree**, software **v1.0.1** (flashed 26-Aug-2026 to `/dev/ttyUSB0` after MAC `a4:f0:0f:73:97:3c`; NVS kept). `How-To-Resume.txt` is gitignored under `local/` so it does not bake `v1.0.1+`. Confirm MAC before every CH340 flash (`ttyUSB*` swaps). CAN is still later (#3A). This board’s wrap IKM still includes node ID (unlike S3 wrap v2).
