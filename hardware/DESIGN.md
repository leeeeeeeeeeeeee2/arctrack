# RocketFC — Hardware Design (ESP32-C3)

This is the electrical spec for the board. Everything the firmware and KiCad need
is here. Read it top to bottom before you start placing parts.

## 1. Block diagram

```
              +----------------------- USB-C (power + programming) --------+
              |                                                            |
   LiPo 1S --> [SW1 slide switch] --> [TP4056 charger] --> VBAT --+        |
   (JST PH2.0)                                                    |        |
                                                                  v        v
                                                       [AMS1117-3.3 LDO]  (D+/D- to ESP32-C3 GPIO19/18)
                                                                  |
                                                                3V3 rail
                                                                  |
        +----------------+----------------+----------------+------+-------------------+
        |                |                |                |                          |
   ESP32-C3         BMP280 (I2C)     MPU-6050 (I2C)   microSD (SPI)          ATGM336H GPS (UART)
   -MINI-1                                                                   + servo headers (PWM)
```

## 2. Power design

- **Battery:** 1S LiPo (3.7V nominal, 4.2V full), e.g. 402035 200mAh, JST PH 2.0mm.
- **Charging:** TP4056 (SOT-23-5 or the common module chip). Program charge
  current with `RPROG`. For a 200mAh cell use ~0.5C = ~100mA → **RPROG ≈ 12kΩ**
  gives ~100mA (I = 1200 / RPROG(kΩ) in A... use the TP4056 table; 20kΩ≈60mA,
  10kΩ≈130mA). Pick **12kΩ ≈ 100mA**. Don't fast-charge tiny cells.
- **Regulator:** AMS1117-3.3 (SOT-223). Input from VBAT (or USB 5V), output 3V3.
  - Note: AMS1117 dropout is ~1.1V, so at low LiPo voltage (~3.5V) the 3V3 rail
    can sag. For a flight that lasts minutes on a freshly charged cell this is
    fine. If you want rock-solid 3V3 at low battery, swap AMS1117 for an
    **ME6211C33** (150mV dropout) later — same idea, better margin.
- **Input caps:** 10µF on VBAT-in and 5V-in, **22µF** on 3V3 out, plus 100nF
  decoupling next to each IC's VDD pin.
- **USB vs battery:** TP4056 charges the cell from USB while the AMS1117 runs the
  board from whichever is higher. Simple approach: AMS1117 input = VBAT node,
  TP4056 keeps the cell topped up from USB. (An "ideal-diode" power-mux is nicer
  but not required for a hobby board.)

## 3. Pin assignments (ESP32-C3-MINI-1)

The ESP32-C3 has limited GPIO. This map avoids strapping-pin conflicts.

| Function        | Signal        | ESP32-C3 GPIO | Notes |
|-----------------|---------------|---------------|-------|
| USB data −      | USB_D−        | GPIO18        | native USB, fixed |
| USB data +      | USB_D+        | GPIO19        | native USB, fixed |
| I2C data        | SDA           | GPIO4         | BMP280 + MPU-6050 shared |
| I2C clock       | SCL           | GPIO5         | shared |
| SD clock        | SPI_SCK       | GPIO6         | |
| SD data out     | SPI_MOSI      | GPIO7         | |
| SD data in      | SPI_MISO      | GPIO3         | |
| SD chip select  | SD_CS         | GPIO10        | |
| GPS → ESP       | UART1_RX      | GPIO20        | receives ATGM336H TX |
| ESP → GPS       | UART1_TX      | GPIO21        | to ATGM336H RX |
| Servo 1 (TVC)   | PWM1          | GPIO0         | header, 5V servo power |
| Servo 2 (TVC)   | PWM2          | GPIO1         | header |
| Boot button     | BOOT          | GPIO9         | strapping: pull-up 10k, button to GND |
| Status LED      | LED           | GPIO8         | strapping: LED+220Ω to 3V3 (LED lit=low) |

**Strapping pins** on ESP32-C3 are GPIO2, GPIO8, GPIO9 — keep GPIO2 unused,
GPIO8 only drives an LED, GPIO9 is the boot button. Do not hang I2C/SPI devices
on these.

## 4. Netlist (connection table)

### ESP32-C3-MINI-1
| Pin | Net |
|-----|-----|
| 3V3 | +3V3 |
| GND (all) | GND |
| EN | +3V3 via 10k, 100nF to GND (reset RC) |
| GPIO18 | USB_D− |
| GPIO19 | USB_D+ |
| GPIO4 | SDA |
| GPIO5 | SCL |
| GPIO6 | SPI_SCK |
| GPIO7 | SPI_MOSI |
| GPIO3 | SPI_MISO |
| GPIO10 | SD_CS |
| GPIO20 | GPS_TX (into ESP) |
| GPIO21 | GPS_RX (out of ESP) |
| GPIO0 | SERVO1 |
| GPIO1 | SERVO2 |
| GPIO9 | BOOT |
| GPIO8 | LED |

### I2C bus
- `SDA` and `SCL` each pulled up to +3V3 with **4.7kΩ**.
- BMP280: VDD→3V3, GND→GND, SDA→SDA, SCL→SCL, SDO→GND (addr 0x76), CSB→3V3 (I2C mode).
- MPU-6050: VDD→3V3, GND→GND, SDA→SDA, SCL→SCL, AD0→GND (addr 0x68), INT→ n/c.

### microSD (SPI, push-pull socket)
- VDD→3V3 (add 10µF), GND→GND
- CLK→SPI_SCK, CMD/DI→SPI_MOSI, DAT0/DO→SPI_MISO, CD/DAT3→SD_CS
- 10kΩ pull-ups on MISO and CS recommended.

### ATGM336H GPS (4-pin header)
- VCC→3V3 (module accepts 3.3V), GND→GND
- TX→GPIO20 (ESP RX), RX→GPIO21 (ESP TX)
- Add 10µF near the module VCC (GPS current spikes).

### USB-C (programming/charging)
- VBUS→5V net (into TP4056 + AMS1117 input path)
- GND→GND
- D+→GPIO19, D−→GPIO18
- CC1/CC2 each to GND via **5.1kΩ** (marks device as UFP so a charger supplies 5V)
- Optional ESD diodes on D+/D−.

### TP4056 charger
- VCC/IN→5V (from USB VBUS), GND→GND
- BAT→VBAT (to battery + / JST), through the slide switch to the AMS1117 input
- PROG→ 12kΩ to GND (≈100mA charge)
- CE→VCC (enable)
- CHRG/STDBY LEDs optional (LED + 1kΩ to VCC)

### AMS1117-3.3 regulator
- IN→VBAT (switched), 10µF in
- OUT→+3V3, 22µF out
- GND→GND

### Slide switch SW1
- In series between TP4056 BAT (battery +) and AMS1117 IN. Cuts power to the
  logic without disconnecting the charger from the cell.

## 5. Layout / routing notes

- Put the **ESP32-C3 antenna** at the board edge with the keep-out area under the
  antenna clear of copper (no ground pour under the antenna). Point it away from
  metal / battery.
- Keep the **BMP280 away from heat and airflow**; add a small vent hole in the
  payload bay, not a sealed cavity, or the pressure reading lags. Do NOT put a
  copper pour directly under it that connects to hot parts.
- Mount the **MPU-6050 near the center axis** of the rocket and align its X/Y/Z
  with the rocket body; note the orientation in firmware.
- Star-ground the analog-ish parts; solid ground pour on an inner/back layer.
- Decoupling caps (100nF) go **as close as possible** to each IC power pin.
- Route USB D+/D− as a **differential pair**, short and equal length, away from
  the switching-ish nodes.
- Keep GPS antenna trace short; if using the ATGM336H module its antenna is
  onboard — keep it near the top of the payload with a clear view of the sky.
- Add **mounting holes** and a couple of **test pads** (3V3, GND, TX, RX).

## 6. Firmware pin defines (keep in sync)

These match `firmware/src/main.cpp`:

```cpp
#define PIN_SDA     4
#define PIN_SCL     5
#define PIN_SD_SCK  6
#define PIN_SD_MOSI 7
#define PIN_SD_MISO 3
#define PIN_SD_CS   10
#define PIN_GPS_RX  20   // ESP receives GPS TX here
#define PIN_GPS_TX  21   // ESP transmits to GPS RX
#define PIN_SERVO1  0
#define PIN_SERVO2  1
#define PIN_LED     8
```
