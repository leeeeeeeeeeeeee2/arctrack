# ArcTrack firmware (ESP32-C3)

PlatformIO project. Matches the ArcTrack PCB pinout in `../hardware/`.

## Build / flash

```bash
pio run
pio run -t upload
pio device monitor
```

## WiFi dashboard

- SSID: `ArcTrack`
- Password: `launch123`
- URL: http://192.168.4.1

## Pins

| Function | GPIO |
|----------|------|
| SDA / SCL | 4 / 5 |
| SD SCK / MOSI / MISO / CS | 6 / 7 / 3 / 10 |
| GPS RX / TX | 20 / 21 |
| Buzzer | 0 |
| Servo | 1 |

Tune `servoStow()` / `servoDeploy()` pulse widths in `src/main.cpp` for your nose linkage.
