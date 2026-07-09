# RocketFC — Bill of Materials

Two groups: parts **JLCPCB solders** (SMD, pick from their catalog) and parts
**you solder** by hand when the board arrives.

> LCSC/JLCPCB part numbers change over time. Treat the `C#######` codes below as
> a **starting point** — search JLCPCB's "Assembly" parts library and pick a
> **Basic** part where possible (Basic parts have no extra loading fee). Confirm
> the footprint matches before ordering.

## JLCPCB assembly (SMD)

| Ref | Part | Value / Pkg | Suggested LCSC | Notes |
|-----|------|-------------|----------------|-------|
| U1  | ESP32-C3-MINI-1 | module | C2934569 | can also hand-solder |
| U2  | BMP280 | LGA-8 | C90293 | barometer |
| U3  | MPU-6050 | QFN-24 | C24112 | IMU |
| U4  | AMS1117-3.3 | SOT-223 | C6186 | 3V3 LDO (or ME6211C33 C82942) |
| U5  | TP4056 | SOP-8/ESOP-8 | C16581 | 1S charger |
| J-SD| microSD socket | push-pull SMD | C91145 | check footprint |
| R1,R2 | I2C pull-ups | 4.7k 0402/0603 | C25900 | SDA/SCL |
| R3,R4 | USB CC resistors | 5.1k 0402/0603 | C25905 | CC1/CC2 to GND |
| R5  | TP4056 PROG | 12k 0603 | C25795 | ~100mA charge |
| R6  | EN pull-up | 10k 0603 | C25804 | reset |
| R7  | BOOT pull-up | 10k 0603 | C25804 | |
| R8  | LED resistor | 220R 0603 | C22962 | status LED |
| R9,R10 | SD pull-ups | 10k 0603 | C25804 | MISO/CS |
| C1  | 3V3 bulk | 22µF 0805 | C45783 | reg out |
| C2,C3 | input bulk | 10µF 0805 | C15850 | VBAT/5V/GPS |
| C4-C9 | decoupling | 100nF 0402/0603 | C1525 | one per IC VDD |
| C10 | EN cap | 100nF | C1525 | reset RC |
| D1  | status LED | 0603 | C2286 | |
| D2  | USB ESD (opt) | SOT-23 | — | e.g. USBLC6-2SC6 C7519 |

## Hand-solder (buy from AliExpress, cheap)

| Ref | Part | Notes |
|-----|------|-------|
| U1  | ESP32-C3-MINI-1 (if not JLC-assembled) | |
| J1  | USB-C receptacle (16-pin SMD) | programming + charge |
| J2  | JST PH 2.0mm 2-pin | battery |
| J3  | 4-pin header (2.54mm) | GPS: VCC/GND/TX/RX |
| J4  | 3–6 pin header | TVC servos (SERVO1/SERVO2 + 5V + GND) |
| SW1 | slide switch | power arm |
| —   | ATGM336H GPS module | UART |
| —   | 1S LiPo 402035 200mAh + JST PH 2.0 | battery |

## Rough cost

| Item | Cost |
|------|------|
| Bare PCB (x5) | ~£2 |
| JLCPCB SMT assembly | ~£8–12 |
| Shipping | ~£3–5 |
| Hand-solder parts + GPS + LiPo | ~£8 |
| **Total** | **~£20–25** |
