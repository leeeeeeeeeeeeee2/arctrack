# ArcTrack — Model Rocket Flight Computer

**ArcTrack** is a compact flight computer for model rockets. It logs altitude and
motion to a microSD card, reads GPS, and lets you **live-track the rocket over
WiFi** from your phone. v1.3 adds a power LED, buzzer, and servo header for
nose-cone parachute deploy at apogee.

Designed for a **BT-70** body tube with a 3D-printed nose avionics bay.

**Hardware:** bare **ESP32-C3FH4** (QFN-32, 4MB flash) + 2.4 GHz chip antenna,
USB-C, LiPo charger, 3.3 V regulator, BMP280, MPU-6050, microSD, GPS header,
servo + 5 V boost.

> RF note: chip-antenna matching may need tuning. See `hardware/DESIGN.md`.

## Features

- **Altitude** — BMP280 barometer + apogee detection  
- **Motion** — MPU-6050 IMU (launch / burnout)  
- **GPS** — ATGM336H (UART)  
- **Logging** — microSD CSV  
- **Live tracking** — WiFi AP + phone web dashboard  
- **USB-C** — charge + program (native USB)  
- **1S LiPo** — TP4056 charging  
- **Power LED** — on whenever the board is powered  
- **Buzzer** — beeps (apogee / status)  
- **Servo** — parachute / nose deploy at apogee (5 V boost on board)

## Repo layout

| Path | What |
|------|------|
| `hardware/DESIGN.md` | Electrical design, pinout, power, layout |
| `hardware/BOM.md` | Bill of materials |
| `hardware/WIRING.md` | Pin-by-pin wiring |
| `hardware/kicad/` | PCB project (schematic + board) |
| `hardware/gerbers/` | Fabrication outputs (when exported) |
| `firmware/` | PlatformIO firmware for ESP32-C3 |

## PCB / order

1. Read `hardware/DESIGN.md` and `hardware/WIRING.md`
2. Open the project in `hardware/kicad/`
3. Order boards from **JLCPCB** (4-layer, ~70×40 mm). Use Gerbers from `hardware/gerbers/` when present.
4. Hand-solder: GPS module, battery connector, switch, servo header as needed

## Firmware

```bash
cd firmware
pio run                 # build
pio run -t upload       # flash over USB-C
pio device monitor      # serial logs
```

Phone: join WiFi **`ArcTrack`** / password **`launch123`**, open **http://192.168.4.1**

### Pin map (firmware)

| Function | GPIO |
|----------|------|
| I2C SDA / SCL | 4 / 5 |
| SD SCK / MOSI / MISO / CS | 6 / 7 / 3 / 10 |
| GPS RX / TX | 20 / 21 |
| Buzzer | 0 |
| Servo PWM | 1 |
| Power LED | hardwired to 3.3 V (not a GPIO) |

## Author

Leighton — UK maker / rocketry + electronics builds.
