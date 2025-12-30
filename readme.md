# 🗿 PROJECT TOTEM : Hybrid Dual-Teensy Workstation

**Repository:** https://github.com/propann/totem  
**Base Project:** https://github.com/FundamentalFrequency/LMN-3

## 📖 Overview

Project Totem is a radical re-engineering of the LMN-3 workstation. It replaces the Raspberry Pi (SBC) with a secondary **Teensy 4.1**, converting the system into a "Bare-Metal Cluster". This eliminates OS latency, boot times, and jitter, dedicating one CPU to UI/Synth Engine and the other to I/O/Controls.

### Architecture

* **Node A (Master):** Teensy 4.1 running MicroDexed-touch + Peanut-GB (GameBoy Emulator). Handles Audio Engine, UI, and Mix.
* **Node B (Slave):** Teensy 4.1 (on LMN-3 PCB). Handles Key Matrix, Joystick, Encoders, and Auxiliary Audio.

---

## ⚠️ HARDWARE MODIFICATION REQUIRED (CRITICAL)

**DO NOT FLASH THE SLAVE FIRMWARE ON A STOCK LMN-3 PCB WITHOUT MODIFICATION.**
The stock LMN-3 PCB routes Matrix Columns to Pins 0, 1, and 14. Project Totem requires these pins for High-Speed UART and S/PDIF.

### The "Pin Bending" Hack (Slave Unit)

To build the Slave unit, you must isolate specific pins from the PCB and reroute them via fly-wires.

1.  **Isolate Pins:** During assembly, bend **Pin 0, 1, and 14** outwards (horizontal). Do not solder them to the PCB headers.
2.  **The Bridge (Inter-Teensy Connection):**
    * **UART:** Slave Pin 0 (RX) <-> Master Pin 1 (TX)
    * **UART:** Slave Pin 1 (TX) <-> Master Pin 0 (RX)
    * **Audio:** Slave Pin 14 (SPDIF OUT) -> Master Pin 15 (SPDIF IN)
    * **Ground:** Common Ground is mandatory.
3.  **The Reroute (Fly-wires):**
    Restore the broken matrix connections by soldering wires from the PCB holes to unused SD card pins on the Teensy:
    * PCB Hole 0 -> Teensy **Pin 29**
    * PCB Hole 1 -> Teensy **Pin 33**
    * PCB Hole 14 -> Teensy **Pin 37**

---

## 🎹 Firmware Modules

### 1. Firmware Slave (Controller)
Located in `/firmware_slave`.
* **Based on:** LMN-3 Firmware.
* **Modifications:**
    * Removed USB-MIDI / Added High-Speed Serial MIDI (2 Mbps).
    * Added `AudioSynthWaveform` + `AudioOutputSPDIF3` (Pin 14).
    * Implemented raw binary protocol for Joystick (X/Y/Btn) over Serial.
    * Remapped Matrix Columns in `config.h` to match the hardware hack.

### 2. Firmware Master (Engine)
Located in `/firmware_master`.
* **Based on:** MicroDexed-touch.
* **Features:**
    * `AsyncAudioInputSPDIF3` to sync Slave audio (ASRC).
    * Peanut-GB integration for emulation.
    * Global Mixer (FM Synth + Emulator + Aux Input).

---

## 🛠️ Protocols

**Serial Communication (Pins 0/1):**
* **Baud Rate:** 2,000,000 (2 Mbps).
* **Format:**
    * Standard MIDI messages (NoteOn, CC) via `Control_Surface`.
    * Custom Binary Packet for Joystick: `[0xFF, X, Y, Btn]`.

**Audio Transport:**
* S/PDIF over TTL (3.3V) via Pin 14->15.
* Sample Rate: 44.1 kHz.

---

## Credits
* **LMN-3:** Fundamental Frequency
* **Teensy Audio Library:** Paul Stoffregen
* **Control Surface:** tttapa
* **MicroDexed-touch:** Holger Wirtz
* **Peanut-GB:** deltabeard
