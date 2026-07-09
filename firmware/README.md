# RocketFC Firmware (ESP32-C3)

PlatformIO project for the flight computer.

## Build & flash

```bash
# from this firmware/ folder
pio run                 # compile
pio run -t upload       # flash over USB-C (native USB CDC)
pio device monitor      # serial logs @115200
```

No PlatformIO? Install the VS Code/Cursor "PlatformIO IDE" extension, or
`pip install platformio` then use the commands above.

### Arduino IDE alternative
Board: **ESP32C3 Dev Module**. In Tools set **USB CDC On Boot: Enabled**.
Install libraries: Adafruit BMP280, Adafruit MPU6050, Adafruit Unified Sensor,
TinyGPSPlus. Then copy `src/main.cpp` into a sketch.

## Using it

1. Power on (USB or LiPo via the arm switch).
2. It calibrates the **ground altitude** during the first ~1s — keep it still.
3. On your phone, join WiFi **`RocketFC`** / password **`launch123`**.
4. Open **http://192.168.4.1** for the live dashboard (altitude, apogee, GPS,
   "Open in Maps").
5. Flight data is written to the microSD as `/flightNNN.csv`.

## Notes / tunables (top of `src/main.cpp`)

- `SAMPLE_HZ` — log/telemetry rate (default 20 Hz).
- `LAUNCH_ACCEL_G` / `LAUNCH_ALT_M` — launch detection thresholds.
- `AP_SSID` / `AP_PASS` — WiFi name/password.
- WiFi range is short (~tens of metres). It's great for pad/recovery tracking and
  the SD log is your full-flight record. For long-range live telemetry you'd add
  a LoRa module later — out of scope for v1.

## Pin map

Matches `../hardware/DESIGN.md`. If you change wiring, change both.
