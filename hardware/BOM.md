# RocketFC — Bill of Materials (bare ESP32-C3FH4)

Full bare-chip board. More parts than the module version, and it includes the RF
section (crystal + chip antenna + matching).

> LCSC/JLCPCB `C#######` codes drift — treat as starting points. Prefer **Basic**
> parts, confirm footprints. Copy RF/crystal values from Espressif's ESP32-C3
> reference schematic.

## JLCPCB assembly (SMD)

| Ref | Part | Value / Pkg | Suggested LCSC | Notes |
|-----|------|-------------|----------------|-------|
| U1  | ESP32-C3FH4 | QFN-32, 4MB flash | C3013796 | main chip (or ESP32-C3FN4) |
| U2  | BMP280 | LGA-8 | C90293 | barometer |
| U3  | MPU-6050 | QFN-24 | C24112 | IMU |
| U4  | ME6211C33M5 | SOT-23-5 | C82942 | 3V3 LDO (low dropout) |
| U5  | TP4056 | ESOP-8 | C16581 | 1S charger |
| U6  | USBLC6-2SC6 | SOT-23-6 | C7519 | USB ESD (optional) |
| Y1  | 40 MHz crystal | 3225 SMD | C9002 | check load cap CL |
| ANT1| 2.4GHz ceramic chip antenna | e.g. 2450AT18A100 | C731332 | follow keep-out |
| J1  | USB-C receptacle | 16P SMD | (pick one) | prog + charge |
| J-SD| microSD socket | push-pull SMD | C91145 | check footprint |
| SW2 | RESET button | SMD tact | C318884 | optional |
| SW3 | BOOT button | SMD tact | C318884 | flash mode |
| L1  | RF match series | 2.7nH 0402 | tune | pi-network (start here) |
| L2  | analog supply | ferrite bead 0402 | C1015 | or 0Ω |
| Cx1,Cx2 | crystal load caps | ~10–12pF 0402 | C1653 | from crystal CL |
| Cm1,Cm2 | RF match shunts | DNP 0402 | — | populate when tuning |
| R1,R2 | I2C pull-ups | 4.7k 0402 | C25900 | SDA/SCL |
| R3,R4 | SD pull-ups | 10k 0402 | C25744 | MISO/CS |
| R5,R6,R7 | strapping pull-ups | 10k 0402 | C25744 | GPIO2/8/9 |
| R8  | EN pull-up | 10k 0402 | C25744 | reset |
| R9,R10 | USB CC | 5.1k 0402 | C25905 | CC1/CC2 |
| R11 | TP4056 PROG | 12k 0402 | C25744 | ~100mA |
| R12 | LED | 220R 0402 | C25808 | status LED |
| C-vdd | ESP decoupling | 100nF 0402 ×~6 | C1525 | one per supply pin |
| C-en | EN cap | 1µF 0402 | C15849 | reset RC |
| C-spi | VDD_SPI | 1µF 0402 | C15849 | |
| C-bmp | BMP decoupling | 100nF 0402 ×2 | C1525 | VDD, VDDIO |
| C-mpu1| MPU VDD | 100nF 0402 | C1525 | |
| C-mpu2| MPU REGOUT | 100nF 0402 | C1525 | |
| C-mpu3| MPU VLOGIC | 10nF 0402 | C1546 | |
| C-mpu4| MPU CPOUT | 2.2nF 0402 | C1548 | |
| C-bulk| bulk | 10µF 0805 ×3 | C15850 | VBUS/BAT/GPS/SD |
| C-3v3 | 3V3 bulk | 22µF 0805 | C45783 | LDO out |
| D1  | status LED | 0402 | C2286 | |

## Hand-solder

| Ref | Part | Notes |
|-----|------|-------|
| J2  | JST PH 2.0mm 2-pin | battery |
| J3  | 4-pin header 2.54mm | GPS: VCC/GND/TX/RX |
| SW1 | slide switch | power arm |
| —   | ATGM336H GPS module | UART |
| —   | 1S LiPo (402035 200mAh) + JST | battery |

## Reality note
This is ~35+ parts vs ~10 for the SuperMini carrier, and the RF section can need
tuning. Budget extra board revisions. If it powers on but WiFi won't connect,
suspect the antenna/matching first.
