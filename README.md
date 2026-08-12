# Arduino_Wemos_TTgo_D1_R32_ESPDuino-32_Waveshare_4inch_OpenMRN_WiFi

OpenMRN (NMRA LCC / OpenLCB) node for the **Wemos TTgo D1 R32 / ESPDuino-32** and the Coowell / Waveshare **4" ILI9486 resistive touch** Arduino shield.

This phase is **WiFi GridConnect only**. There is no TWAI/CAN driver and no display driver yet. The shield may stay on the board; SPI DIP switches must be **D11 / D12 / D13** (this Wemos does not wire ICSP).

## License

Application code in this repository is **BSD-2-Clause** (see `LICENSE`).  
`components/OpenMRNIDF` is a submodule of [atanisoft/OpenMRNIDF](https://github.com/atanisoft/OpenMRNIDF) and remains BSD-2-Clause.

## Build

See [BUILD.md](BUILD.md). Use **ESP-IDF v5.1.6** and `idf.py set-target esp32`.

## Hardware you need for this phase

- D1 R32 (CH340, typically `/dev/ttyUSB0`)
- Wi-Fi and a JMRI **Start Hub** (or other GridConnect TCP server) on port 12021
- Optional: the 4" shield, seated, DIPs not on ICSP
- Not yet: a CAN transceiver (#3A)

## OpenLCB node ID

OwlThree is listed in the [OpenLCB unique ID registry](https://registry.openlcb.org/uniqueidranges) as:

**05.01.01.01.A5.*** — 256 IDs (`05.01.01.01.A5.00` … `05.01.01.01.A5.FF`)

This firmware defaults to **05.01.01.01.A5.01** (this D1 R32 WiFi node). Change it in `idf.py menuconfig` → *D1 R32 OpenMRN WiFi node*. Keep `.02` and up for more boards or the later CAN node.

Do not use the `03.00.AB.01.*` values from the registry comment as the node ID. The assigned range is the `05.01.01.01.A5.*` row.

## Status

Scaffold: OpenMRNIDF submodule, IDF project, NVS/netif init, hub host/port Kconfig. Next: `Esp32WiFiManager` + OpenMRN stack attached to the hub.
