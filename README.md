# RocketFC — Model Rocket Flight Computer (ESP32-C3)

A tiny flight computer PCB + firmware for a model rocket. It logs altitude and
motion to a microSD card, reads GPS, and lets you **live-track the rocket over
WiFi** from your phone.

## Features

- **Altitude** from BMP280 barometer (relative-to-ground, apogee detection)
- **Motion** from MPU-6050 IMU (accel + gyro, launch/burnout detection)
- **GPS** from ATGM336H (position, speed, satellites)
- **Logging** to microSD (CSV, one row per sample)
- **Live tracking** — ESP32-C3 hosts a WiFi access point + web dashboard you
  open on your phone to see live position/altitude and a "open in maps" link
- **USB-C** for programming (native USB — no USB-UART chip needed)
- **1S LiPo** powered with onboard TP4056 charging

## Repo layout

| Path | What it is |
|------|-----------|
| `hardware/DESIGN.md` | Full electrical design: pinout, netlist, power, layout notes. **Read this first for the PCB.** |
| `hardware/BOM.md` | Bill of materials with JLCPCB part numbers |
| `hardware/rocketfc_schematic.py` | SKiDL script → generates a KiCad netlist |
| `firmware/` | PlatformIO project for the ESP32-C3 |

## The PCB workflow (honest version)

I (the AI) can't draw/route the board for you — that's a hands-on job in KiCad.
Here's the realistic path:

1. Read `hardware/DESIGN.md` — it has every connection you need.
2. (Optional) Run `hardware/rocketfc_schematic.py` with SKiDL to auto-generate a
   KiCad netlist, so you don't wire every net by hand. See that file's header.
3. In KiCad: place footprints, import the netlist, then **route the board**
   (this part is you, at your screen).
4. Order from JLCPCB using `hardware/BOM.md` for assembly part numbers.
5. Solder the ESP32-C3 module, GPS header, battery connector, and switch.

## Firmware quick start

```bash
cd firmware
pio run                 # build
pio run -t upload       # flash over USB-C
pio device monitor      # view logs
```

Then power it on, connect your phone to WiFi network **`RocketFC`**
(password `launch123`), and open **http://192.168.4.1** in a browser.

See `firmware/README.md` for details.
