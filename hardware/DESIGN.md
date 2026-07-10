# RocketFC — Hardware Design (bare ESP32-C3FH4 + chip antenna)

Full bare-chip flight computer. Everything the SuperMini gave us for free
(regulator, USB, buttons, reset) is now on our board, plus the RF section
(crystal + antenna + matching). Keep this open while working in KiCad.

> **RF WARNING:** the antenna matching network usually needs tuning to work well,
> ideally with a VNA. To de-risk, copy component **values and layout** from
> Espressif's *ESP32-C3 Hardware Design Guidelines* reference schematic + the
> ESP32-C3-MINI-1 module schematic. Those are the known-good source of truth.

## 0. Main chip

Use **ESP32-C3FH4** (QFN-32, 5×5mm) — it has **4MB flash built in**, so no
external flash chip needed. (ESP32-C3FN4 is the same idea.)

Wire by **pin NAME** using the KiCad symbol — pin numbers vary, names don't.

## 1. Power

```
USB-C VBUS(5V) --> TP4056 --> LiPo(BAT) --> SW1 --> LDO(3.3V) --> +3V3 rail
                     |                                   (ME6211C33 / AMS1117)
                   charge
```

- **USB-C**: VBUS→5V net; GND→GND; D+→GPIO19; D−→GPIO18; CC1,CC2 each 5.1kΩ→GND.
- **TP4056**: IN←VBUS; BAT↔LiPo+; PROG→12kΩ→GND (~100mA); CE→IN; GND→GND.
- **SW1** slide switch: LiPo+ → LDO input (arms logic; charger stays on the cell).
- **LDO**: prefer **ME6211C33M5** (SOT-23-5, 150mV dropout — good at low LiPo V)
  over AMS1117 (1.1V dropout). IN←switched battery; OUT→+3V3; EN→IN; 1µF in, 1µF+
  10µF out.
- Bulk: 10µF on VBUS, 10µF on battery, 22µF on +3V3.

## 2. ESP32-C3FH4 core wiring

### Supply pins (all to +3V3, each with its own 100nF)
- `VDD3P3` (all instances) → +3V3, 100nF each
- `VDD3P3_CPU` → +3V3, 100nF
- `VDD3P3_RTC` → +3V3, 100nF
- `VDDA` / analog VDD3P3 → +3V3 via a **small ferrite bead / 0Ω** for RF cleanliness,
  then 100nF + 10µF (see reference)
- `VDD_SPI` → **1µF to GND** (internal LDO output for in-package flash; do NOT tie
  to 3V3 — follow reference)
- EP (exposed pad) → GND (stitch with vias)

### Enable / reset
- `CHIP_EN` (EN) → 10kΩ pull-up to +3V3, **1µF** to GND (reset RC).
- Optional **RESET button**: EN → button → GND.

### Strapping pins (critical for boot)
- `GPIO2` → 10kΩ pull-up to +3V3 (must be high at boot; keep nothing pulling it low)
- `GPIO8` → 10kΩ pull-up to +3V3 (must be high at boot)
- `GPIO9` → 10kΩ pull-up to +3V3 + **BOOT button** to GND (hold low at reset = flash mode)

### USB (native — no USB-UART chip)
- `GPIO18` → USB_D−
- `GPIO19` → USB_D+
- (optional USBLC6-2 ESD device on D+/D−)

### Clock
- 40 MHz crystal between `XTAL_P` and `XTAL_N`.
- Two load caps to GND (start ~**10–12pF** each; exact value = 2×(CL−Cstray),
  from the crystal datasheet). Some designs also add a small series R (~0Ω).

## 3. RF section (antenna + matching) — copy the reference!

- `LNA_IN` (RF pin) → **pi-matching network** → 50Ω feed line → **chip antenna**.
- Pi network = 3 footprints: shunt C1 (to GND), series L, shunt C2 (to GND).
  Start with **L = series 0Ω/2.7nH, C1/C2 = unpopulated (DNP)**, then tune. Use
  the antenna vendor's app note values as the starting point.
- **Chip antenna**: e.g. Johanson 2450AT18A100 or similar 2.4GHz ceramic. Follow
  its datasheet **ground keep-out** exactly (a copper-free region under/around it).
- **50Ω trace**: keep short; on 1.6mm 2-layer FR4 a top trace over the ground
  plane ≈ 2.9mm wide for 50Ω (use KiCad's calculator with your stackup).
- Nothing (no copper/ground pour) under the antenna keep-out.

## 4. Pin assignments (peripherals — same as before, firmware unchanged)

| Function        | Signal   | GPIO  | Notes |
|-----------------|----------|-------|-------|
| I2C data        | SDA      | GPIO4 | BMP280 + MPU-6050 |
| I2C clock       | SCL      | GPIO5 | |
| SD clock        | SPI_SCK  | GPIO6 | |
| SD data out     | SPI_MOSI | GPIO7 | |
| SD data in      | SPI_MISO | GPIO3 | |
| SD chip select  | SD_CS    | GPIO10| |
| GPS → ESP       | UART1_RX | GPIO20| |
| ESP → GPS       | UART1_TX | GPIO21| |
| USB D−          | —        | GPIO18| native USB |
| USB D+          | —        | GPIO19| native USB |
| Status LED      | LED      | GPIO8 | LED+220Ω to +3V3 (also strapping-pulled high) |

## 5. Sensors (bare chips) — you've already wired most of this

### I2C pull-ups
- 4.7kΩ from SDA→+3V3 and SCL→+3V3.

### BMP280
- VDD (8)→+3V3 (100nF), VDDIO (6)→+3V3 (100nF), GND (1)→GND, GND (7)→GND
- SDI (5)→SDA, SCK (4)→SCL, SDO (3)→GND (0x76), CSB (2)→+3V3 (I2C mode)

### MPU-6050
- VDD (13)→+3V3 (100nF), VLOGIC (8)→+3V3 (10nF), GND (18)→GND
- REGOUT (10)→100nF to GND (REQUIRED), CPOUT (20)→2.2nF to GND (REQUIRED)
- SDA (24)→SDA, SCL (23)→SCL, AD0 (9)→GND (0x68)
- FSYNC (11)→GND, CLKIN (1)→GND
- INT (12), AUX_DA (6), AUX_CL (7) → No-Connect

## 6. microSD (SPI)
- VDD→+3V3 (10µF), GND→GND
- CLK→SPI_SCK, CMD→SPI_MOSI, DAT0→SPI_MISO, DAT3→SD_CS
- 10kΩ pull-ups on MISO and CS.

## 7. GPS (ATGM336H, 4-pin header)
- VCC→+3V3 (10µF), GND→GND, module TX→GPIO20, module RX→GPIO21.

## 8. Layout / routing notes (RF matters now)
- **Antenna at a board edge/corner**, its keep-out zone kept 100% copper-free
  (no ground pour, no traces). Everything else stays away from it.
- Solid **ground plane** on the opposite layer; stitch the chip's EP and grounds
  with lots of vias.
- Keep the **40MHz crystal** right next to the chip, short traces, guarded by
  ground, load caps close.
- Keep the **RF trace** (LNA_IN → antenna) short, 50Ω, over unbroken ground.
- Decoupling caps hug each supply pin; analog supply gets the ferrite + extra cap.
- Route USB D+/D− as a short, equal-length differential pair.
- BMP280 gets air (vent, not sealed); MPU-6050 near the rocket's center axis.

## 9. Firmware pin defines (unchanged)
```cpp
#define PIN_SDA     4
#define PIN_SCL     5
#define PIN_SD_SCK  6
#define PIN_SD_MOSI 7
#define PIN_SD_MISO 3
#define PIN_SD_CS   10
#define PIN_GPS_RX  20
#define PIN_GPS_TX  21
#define PIN_LED     8
```
