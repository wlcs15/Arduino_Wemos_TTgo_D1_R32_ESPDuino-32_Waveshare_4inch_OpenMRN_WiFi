# Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi

OpenMRN (NMRA LCC / OpenLCB) node for the **Wemos TTgo D1 R32 / ESPDuino-32** and the Coowell / Waveshare **4" ILI9486 resistive touch** Arduino shield.

This phase is **WiFi GridConnect only** (no TWAI/CAN). `#if DEBUG` paints MAC, OpenLCB node ID, and SPI flash unique ID on the 4" ILI9486 and logs the same three values on serial. The PSK is never shown. SPI DIP switches must be **D11 / D12 / D13** (this Wemos does not wire ICSP).

## License

Application code in this repository is **BSD-2-Clause** (see `LICENSE`).  
`components/OpenMRNIDF` is a submodule of [atanisoft/OpenMRNIDF](https://github.com/atanisoft/OpenMRNIDF) and remains BSD-2-Clause.

## Build

See [BUILD.md](BUILD.md). Use **ESP-IDF v5.1.6** and `idf.py set-target esp32`.

## Hardware you need for this phase

- D1 R32 (CH340, typically `/dev/ttyUSB0`)
- House Wi-Fi
- **TCS CS-105** at `192.168.1.27` (GridConnect TCP **12021**) — this is the hub the node joins
- **Raspberry Pi + JMRI** at `192.168.1.61` — second view only: configure JMRI’s OpenLCB connection **to the CS-105**, do not start a competing hub for the ESP32
- Optional: the 4" shield, seated, DIPs not on ICSP
- Not yet: a CAN transceiver (#3A)

Do not point this firmware at the Pi. The CS-105 owns the LCC bus (CAN + Wi-Fi). JMRI watches that same hub.

## OpenLCB node ID

OwlThree is listed in the [OpenLCB unique ID registry](https://registry.openlcb.org/uniqueidranges) as:

**05.01.01.01.A5.*** — 256 IDs (`05.01.01.01.A5.00` … `05.01.01.01.A5.FF`)

This firmware defaults to **05.01.01.01.A5.01** (this D1 R32 WiFi node). Change it in `idf.py menuconfig` → *D1 R32 OpenMRN WiFi node*. Keep `.02` and up for more boards or the later CAN node.

Do not use the `03.00.AB.01.*` values from the registry comment as the node ID. The assigned range is the `05.01.01.01.A5.*` row.

## Wi-Fi secret

- SSID **SRIF2333** (not secret).
- PSK is your house password. It is **never** a git default.
- First provision: local `wifi_secrets.env` + `utils/build_idf5.sh`. On first boot the PSK is wrapped with **AES-256-GCM** (mbedTLS in ESP-IDF) and stored in **NVS**. Later app flashes keep NVS.
- Wrap key = HKDF-SHA256(flash unique id ∥ MAC ∥ `05.01.01.01.A5.01`). `#if DEBUG` shows MAC, Node ID, and flash UID on the 4" glass and on serial (never the PSK).
- Flash unique id is preferred over MAC (MAC is on the air). If the flash chip has no UID, MAC is the fallback. This is “not plaintext,” not dump-proof.

## Status

Scaffold: OpenMRNIDF submodule, IDF project, NVS/netif init, hub host/port Kconfig, DEBUG ILI9486 + serial ID dump. Next: `Esp32WiFiManager` + OpenMRN stack attached to the hub.
