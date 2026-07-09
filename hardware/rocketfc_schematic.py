#!/usr/bin/env python3
"""
RocketFC schematic-as-code (SKiDL) -> KiCad netlist.

This DEFINES every electrical connection for the board. Running it produces
`RocketFC.net`, which you IMPORT into KiCad's PCB editor so you don't have to
wire each net by hand. You still place footprints and route the board yourself.

------------------------------------------------------------------------------
SETUP (one time):
    pip install skidl
    # SKiDL needs KiCad's symbol libraries. Install KiCad 7/8/9, then point
    # SKiDL at the symbol dir, e.g. on Windows:
    #   setx KICAD9_SYMBOL_DIR "C:\\Program Files\\KiCad\\9.0\\share\\kicad\\symbols"
    # (restart the terminal after setx)

RUN:
    python rocketfc_schematic.py
    # -> writes RocketFC.net

IMPORT INTO KICAD:
    PCB Editor -> File -> Import -> Netlist... -> choose RocketFC.net

NOTE ON PART/LIB NAMES:
    The symbol library names below (e.g. 'RF_Module:ESP32-C3-MINI-1') are the
    standard KiCad names, but they DO drift between KiCad versions and some parts
    (TP4056) may live in a contrib/3rd-party lib. If SKiDL errors that it can't
    find a symbol, either (a) install the KiCad "official + contrib" libs, or
    (b) change the `lib`/`name` to one that exists in your install. Use
    `search()` in an skidl REPL to find names.
------------------------------------------------------------------------------
"""

from skidl import Part, Net, generate_netlist, TEMPLATE

# ---------------------------------------------------------------- power nets
vbus = Net("VBUS")     # 5V from USB-C
vbat = Net("VBAT")     # LiPo cell / switched battery rail
v3v3 = Net("+3V3")     # regulated logic rail
gnd  = Net("GND")

# ---------------------------------------------------------------- data nets
sda   = Net("SDA")
scl   = Net("SCL")
sck   = Net("SPI_SCK")
mosi  = Net("SPI_MOSI")
miso  = Net("SPI_MISO")
sd_cs = Net("SD_CS")
gps_tx = Net("GPS_TX")   # GPS -> ESP (ESP GPIO20 / RX)
gps_rx = Net("GPS_RX")   # ESP -> GPS (ESP GPIO21 / TX)
usb_dp = Net("USB_D+")
usb_dm = Net("USB_D-")
servo1 = Net("SERVO1")
servo2 = Net("SERVO2")
led_n  = Net("LED")
boot_n = Net("BOOT")
en_n   = Net("EN")

# ---------------------------------------------------------------- components
# ESP32-C3-MINI-1 module
esp = Part("RF_Module", "ESP32-C3-MINI-1", footprint="RF_Module:ESP32-C2-MINI-1")

# Sensors
bmp = Part("Sensor_Pressure", "BMP280", footprint="Package_LGA:Bosch_LGA-8_2x2.5mm_P0.65mm_ClockwisePinNumbering")
mpu = Part("Sensor_Motion", "MPU-6050", footprint="Package_DFN_QFN:QFN-24-1EP_4x4mm_P0.5mm_EP2.7x2.7mm")

# Power
reg = Part("Regulator_Linear", "AMS1117-3.3", footprint="Package_TO_SOT_SMD:SOT-223-3_TabPin2")
# TP4056 may be in a contrib lib; fall back to a generic 8-pin if missing.
try:
    tp = Part("Battery_Management", "TP4056", footprint="Package_SO:SOP-8_3.9x4.9mm_P1.27mm")
except Exception:
    tp = Part("Device", "U", value="TP4056", footprint="Package_SO:SOP-8_3.9x4.9mm_P1.27mm", dest=TEMPLATE)()

# Connectors
usb = Part("Connector", "USB_C_Receptacle_USB2.0_16P",
           footprint="Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12")
jbat = Part("Connector", "Conn_01x02_Pin", footprint="Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical")
jgps = Part("Connector", "Conn_01x04_Pin", footprint="Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical")
jsrv = Part("Connector", "Conn_01x04_Pin", footprint="Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical")
sw   = Part("Switch", "SW_SPDT", footprint="Button_Switch_SMD:SW_SPDT_PCM12")

# microSD socket
sd = Part("Connector", "microSD_Card_Det_Hirose_DM3AT",
          footprint="Connector_Card:microSD_HC_Hirose_DM3AT-SF-PEJM5")

# Passives (templates so we can stamp copies)
R = Part("Device", "R", dest=TEMPLATE, footprint="Resistor_SMD:R_0603_1608Metric")
C = Part("Device", "C", dest=TEMPLATE, footprint="Capacitor_SMD:C_0603_1608Metric")
LED = Part("Device", "LED", dest=TEMPLATE, footprint="LED_SMD:LED_0603_1608Metric")

r_sda  = R(value="4.7k")
r_scl  = R(value="4.7k")
r_cc1  = R(value="5.1k")
r_cc2  = R(value="5.1k")
r_prog = R(value="12k")
r_en   = R(value="10k")
r_boot = R(value="10k")
r_led  = R(value="220")
r_sd_miso = R(value="10k")
r_sd_cs   = R(value="10k")

c_3v3  = C(value="22u")
c_in   = C(value="10u")
c_gps  = C(value="10u")
c_sd   = C(value="10u")
c_en   = C(value="100n")
c_bmp  = C(value="100n")
c_mpu  = C(value="100n")
c_esp  = C(value="100n")

d_led = LED()

# ---------------------------------------------------------------- ESP32-C3 wiring
# NOTE: pin *names* below assume the KiCad ESP32-C3-MINI-1 symbol. If a name
# doesn't match, use the pin NUMBER instead (esp[<num>] += net).
esp["3V3"] += v3v3
esp["GND"] += gnd
esp["EN"]  += en_n
esp["IO18"] += usb_dm
esp["IO19"] += usb_dp
esp["IO4"]  += sda
esp["IO5"]  += scl
esp["IO6"]  += sck
esp["IO7"]  += mosi
esp["IO3"]  += miso
esp["IO10"] += sd_cs
esp["IO20"] += gps_tx
esp["IO21"] += gps_rx
esp["IO0"]  += servo1
esp["IO1"]  += servo2
esp["IO9"]  += boot_n
esp["IO8"]  += led_n

# EN reset RC
r_en[1] += v3v3
r_en[2] += en_n
c_en[1] += en_n
c_en[2] += gnd

# BOOT pull-up (button to GND is a hand-solder tact switch; optional)
r_boot[1] += v3v3
r_boot[2] += boot_n

# ---------------------------------------------------------------- I2C sensors
r_sda[1] += v3v3; r_sda[2] += sda
r_scl[1] += v3v3; r_scl[2] += scl

bmp["VDD"] += v3v3; bmp["VDDIO"] += v3v3
bmp["GND"] += gnd
bmp["SDI"] += sda      # SDA
bmp["SCK"] += scl      # SCL (I2C clock)
bmp["SDO"] += gnd      # addr 0x76
bmp["CSB"] += v3v3     # I2C mode
c_bmp[1] += v3v3; c_bmp[2] += gnd

mpu["VDD"] += v3v3; mpu["VLOGIC"] += v3v3
mpu["GND"] += gnd
mpu["SDA"] += sda
mpu["SCL"] += scl
mpu["AD0"] += gnd      # addr 0x68
c_mpu[1] += v3v3; c_mpu[2] += gnd

# ESP decoupling
c_esp[1] += v3v3; c_esp[2] += gnd

# ---------------------------------------------------------------- microSD (SPI)
sd["VDD"] += v3v3
sd["GND"] += gnd
sd["CLK"] += sck
sd["CMD"] += mosi
sd["DAT0"] += miso
sd["DAT3"] += sd_cs
r_sd_miso[1] += v3v3; r_sd_miso[2] += miso
r_sd_cs[1]   += v3v3; r_sd_cs[2]   += sd_cs
c_sd[1] += v3v3; c_sd[2] += gnd

# ---------------------------------------------------------------- GPS header
# J: 1=VCC 2=GND 3=TX 4=RX  (module TX -> ESP RX net gps_tx)
jgps[1] += v3v3
jgps[2] += gnd
jgps[3] += gps_tx
jgps[4] += gps_rx
c_gps[1] += v3v3; c_gps[2] += gnd

# ---------------------------------------------------------------- servo header
# 1=5V 2=GND 3=SERVO1 4=SERVO2
jsrv[1] += vbus        # servos want ~5V; feed from USB VBUS (only powered on USB)
jsrv[2] += gnd
jsrv[3] += servo1
jsrv[4] += servo2

# ---------------------------------------------------------------- USB-C
usb["VBUS"] += vbus
usb["GND"] += gnd
usb["D+"] += usb_dp
usb["D-"] += usb_dm
# CC pull-downs (device / UFP)
r_cc1[1] += usb["CC1"]; r_cc1[2] += gnd
r_cc2[1] += usb["CC2"]; r_cc2[2] += gnd

# ---------------------------------------------------------------- charger + reg
# TP4056: VCC(4/IN)=VBUS, BAT(5)=battery, PROG(2)=12k->GND, GND(3)=GND, CE(8)=VCC
# Using pin numbers to stay lib-agnostic:
tp[4] += vbus          # VCC / IN
tp[3] += gnd           # GND
tp[5] += Net("BATT+")  # BAT (to JST + and, via switch, to VBAT)
tp[2] += r_prog[1]     # PROG
r_prog[2] += gnd
tp[8] += vbus          # CE enable

batt_plus = tp[5].nets[0]
jbat[1] += batt_plus   # LiPo +
jbat[2] += gnd         # LiPo -

# Slide switch: battery+ --> VBAT (arms the logic)
sw[1] += batt_plus     # common/pole
sw[2] += vbat          # throw -> VBAT rail
# (leave the 3rd terminal unconnected)

# AMS1117: IN=VBAT, OUT=+3V3
reg["VI"] += vbat
reg["VO"] += v3v3
reg["GND"] += gnd
c_in[1]  += vbat; c_in[2]  += gnd
c_3v3[1] += v3v3; c_3v3[2] += gnd

# status LED: 3V3 -> 220R -> LED -> GPIO8 (lit when IO8 low)
r_led[1] += v3v3
r_led[2] += d_led["A"]
d_led["K"] += led_n

# ---------------------------------------------------------------- output
generate_netlist(file_="RocketFC.net")
print("Wrote RocketFC.net")
