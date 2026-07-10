# RocketFC — Exact pin-by-pin wiring (bare ESP32-C3FH4)

Pin numbers below match the KiCad symbols in the current schematic
(U4 = ESP32-C3FH4, U1 = MPU-6050, U2 = BMP280).

"100nF→GND" etc. means: put that cap with one leg on this pin's net and the other
leg on GND, right next to the pin.

Confusing-but-important: on the ESP32-C3, `MTMS/MTDI/MTCK/MTDO` = GPIO4/5/6/7.

---

## U4 — ESP32-C3FH4

| Pin | Symbol name | Connect to |
|-----|-------------|-----------|
| 1  | LNA_IN       | RF: → matching network → antenna |
| 2  | VDD3P3       | **+3V3** + 100nF→GND |
| 4  | XTAL_32K_P (GPIO0) | **BZ1 −** (buzzer, v1.1) |
| 5  | XTAL_32K_N (GPIO1) | **J5 pin 3** (servo signal, v1.1) |
| 6  | GPIO2        | 10k→**+3V3** (strapping, must be high at boot) |
| 7  | CHIP_EN      | 10k→**+3V3**, 1µF→GND, RESET button→GND |
| 8  | GPIO3        | **SD MISO** (U-SD pin DAT0) + 10k→+3V3 |
| 9  | MTMS (GPIO4) | **SDA** |
| 10 | MTDI (GPIO5) | **SCL** |
| 11 | VDD3P3_RTC   | **+3V3** + 100nF→GND |
| 12 | MTCK (GPIO6) | **SD SCK** (U-SD pin CLK) |
| 13 | MTDO (GPIO7) | **SD MOSI** (U-SD pin CMD) |
| 14 | GPIO8        | 10k→+3V3 (strapping only — no LED here in v1.1) |
| 15 | GPIO9        | 10k→**+3V3** + BOOT button→GND |
| 16 | GPIO10       | **SD CS** (U-SD pin DAT3) |
| 17 | VDD3P3_CPU   | **+3V3** + 100nF→GND |
| 18 | VDD_SPI      | **1µF→GND ONLY** (do not tie to +3V3) |
| 19 | SPIHD        | **No-Connect (Q)** — internal flash |
| 20 | SPIWP        | **No-Connect (Q)** |
| 21 | SPICS0       | **No-Connect (Q)** |
| 22 | SPICLK       | **No-Connect (Q)** |
| 23 | SPID         | **No-Connect (Q)** |
| 24 | SPIQ         | **No-Connect (Q)** |
| 25 | GPIO18       | **USB D−** |
| 26 | GPIO19       | **USB D+** |
| 27 | U0RXD (GPIO20) | **GPS module TX** |
| 28 | U0TXD (GPIO21) | **GPS module RX** |
| 29 | XTAL_N       | 40MHz crystal + ~10pF load cap→GND *(confirm pin #)* |
| 30 | XTAL_P       | 40MHz crystal + ~10pF load cap→GND *(confirm pin #)* |
| 31 | VDDA         | **+3V3** (via ferrite bead / 0Ω) + 100nF→GND |
| 33 | GND (EP pad) | **GND** |

---

## v1.1 additions (ArcTrack — LED, buzzer, servo, 5 V boost)

Add these in the **same** KiCad project. Do **not** start a new one.

### D1 + R12 — power LED (always on when board is powered)

| Ref | Value | Footprint |
|-----|-------|-----------|
| D1  | LED   | `LED_SMD:LED_0402_1005Metric` |
| R12 | **330Ω** | `Resistor_SMD:R_0402_1005Metric` |

**Wire:** **+3V3 → R12 → D1 anode (+) → D1 cathode (−) → GND**

Not 330**k** — that would block almost all current and the LED would stay dark.

---

### BZ1 — buzzer

| Ref | Value | Footprint |
|-----|-------|-----------|
| BZ1 | Active buzzer **3.3 V** SMD | pick to match LCSC part |

**Wire:**
- **+3V3 → BZ1 +**
- **BZ1 − → U4 pin 4** (XTAL_32K_P / GPIO0)

Remove any **No-Connect** flag on U4 pin 4 if you added one earlier.

---

### J5 — servo header (3-pin, 2.54 mm)

| Ref | Value | Footprint |
|-----|-------|-----------|
| J5  | `Conn_01x03` | `Connector_PinHeader_2.54mm:PinHeader_1x03` |

| J5 pin | Net |
|--------|-----|
| 1 | **GND** |
| 2 | **+5V** (servo power — from boost U6) |
| 3 | **U4 pin 5** (XTAL_32K_N / GPIO1) |

Remove any **No-Connect** on U4 pin 5.

---

### U6 — 5 V boost (servo power on the PCB)

Powers the servo only. Do **not** connect +5V to the ESP32.

| Ref | Value | Notes |
|-----|-------|-------|
| U6  | 5 V boost IC (e.g. TPS61223, FP6296) | SOT-23-6 + inductor per datasheet |
| L2  | **2.2 µH** | boost inductor |
| C21 | **100 µF** | +5V bulk, near J5 |
| C22 | **10 µF** | U6 input |
| C23 | **10 µF** | U6 output |

**Wire:**
- **U6 IN** → same net as **LDO input** (battery side **after SW1**)
- **U6 GND** → **GND**
- **U6 OUT** → **+5V** power symbol
- **+5V → J5 pin 2**
- **C21:** +5V ↔ GND near J5

Add a **+5V** power symbol (press **P**, search `+5V`).

---

### Board outline (PCB editor, after schematic)

- **Edge.Cuts:** **70 × 40 mm** rectangle (long side ≥ 70 mm → no JLC edge rails)
- **4×** `MountingHole_2.7mm` ~3 mm in from each corner

---

### Firmware pins (v1.1)

```cpp
#define PIN_BUZZER  0   // U4 pin 4
#define PIN_SERVO   1   // U4 pin 5
// Power LED is hardwired — no GPIO
```

---

## U1 — MPU-6050

| Pin | Name   | Connect to |
|-----|--------|-----------|
| 13 | VDD    | **+3V3** + 100nF→GND |
| 8  | VLOGIC | **+3V3** + 10nF→GND |
| 18 | GND    | **GND** |
| 24 | SDA    | **SDA** (→ U4 pin 9) |
| 23 | SCL    | **SCL** (→ U4 pin 10) |
| 9  | AD0    | **GND** (I2C address 0x68) |
| 11 | FSYNC  | **GND** |
| 1  | CLKIN  | **GND** |
| 10 | REGOUT | **100nF→GND** (required) |
| 20 | CPOUT  | **2.2nF→GND** (required) |
| 12 | INT    | **No-Connect (Q)** |
| 6  | AUX_DA | **No-Connect (Q)** |
| 7  | AUX_CL | **No-Connect (Q)** |

---

## U2 — BMP280

| Pin | Name  | Connect to |
|-----|-------|-----------|
| 8  | VDD   | **+3V3** + 100nF→GND |
| 6  | VDDIO | **+3V3** + 100nF→GND |
| 1  | GND   | **GND** |
| 7  | GND   | **GND** |
| 5  | SDI   | **SDA** (→ U4 pin 9) |
| 4  | SCK   | **SCL** (→ U4 pin 10) |
| 3  | SDO   | **GND** (I2C address 0x76) |
| 2  | CSB   | **+3V3** (selects I2C mode — required) |

---

## The I2C bus + pull-ups (ties it all together)
- Net **SDA** = U4 pin 9 + U1 pin 24 + U2 pin 5 + one 4.7k resistor to +3V3
- Net **SCL** = U4 pin 10 + U1 pin 23 + U2 pin 4 + one 4.7k resistor to +3V3

Use **net labels** (press **L**, type `SDA` / `SCL`) on each of those pins instead
of drawing long wires — same label name = connected.

---

## Support parts still to place (then wire per above references)
- **40MHz crystal (Y1)** → U4 pins 29/30, load caps to GND
- **Chip antenna + matching (L,C,C)** → U4 pin 1 (LNA_IN)
- **RESET button** → U4 pin 7; **BOOT button** → U4 pin 15
- **USB-C (J1)** → U4 pins 25/26 (D−/D+), VBUS, GND, CC 5.1k×2
- **LDO (U-reg)** → makes +3V3
- **TP4056** → USB VBUS in, LiPo, PROG 12k
- **microSD, GPS header, JST, slide switch** → per tables above
