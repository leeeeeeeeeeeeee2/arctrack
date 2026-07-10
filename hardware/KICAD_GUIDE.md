# RocketFC — KiCad 9 Step-by-Step (beginner)

Follow this in order. Keep `DESIGN.md` open in another window — it's the source of
truth for every connection. Don't rush; expect this to take a few sessions.

Big picture, there are 5 phases:
1. Draw the **schematic** (logical wiring)
2. Assign **footprints** (physical shape of each part)
3. Lay out + **route** the board
4. Run **DRC** (error check)
5. Export **manufacturing files** for JLCPCB

---

## Phase 0 — Create the project
1. Open **KiCad**.
2. `File → New Project`. Make a folder `RocketFC`, name the project `RocketFC`, Save.
3. Double-click `RocketFC.kicad_sch` to open the **Schematic Editor**.

---

## Phase 1 — Schematic (the logical wiring)

You place a symbol for each part, then draw wires between pins per `DESIGN.md`.

### 1a. Place symbols
- Press **A** (Add Symbol). Search and place each of these. If an exact name
  isn't found, use the closest match — the electrical connections matter, not the
  symbol's cosmetic name.

This is the **bare ESP32-C3FH4** parts list (see `DESIGN.md` + `BOM.md`).

| Search for | Qty | It's the… |
|-----------|-----|-----------|
| `ESP32-C3` | 1 | main chip — pick the bare **ESP32-C3FH4/FN4** (QFN-32), NOT the -MINI module |
| `BMP280` | 1 | barometer |
| `MPU-6050` | 1 | IMU |
| `microSD` | 1 | SD socket |
| `Crystal_GND` | 1 | 40MHz crystal (4-pad) |
| `Antenna` | 1 | chip antenna (or a generic 1-pin `Antenna` symbol) |
| `L` | 1 | RF series match / ferrite |
| `ME6211` or `AP2112` / `AMS1117` | 1 | 3.3V LDO |
| `TP4056` | 1 | charger |
| `USB_C_Receptacle_USB2.0` | 1 | USB-C |
| `USBLC6-2SC6` | 1 | USB ESD (optional) |
| `SW_Push` | 2 | RESET + BOOT buttons |
| `Conn_01x02` | 1 | battery JST |
| `Conn_01x04` | 1 | GPS header |
| `SW_SPDT` | 1 | slide switch |
| `R` | ~12 | resistors (see BOM) |
| `C` | ~20 | capacitors (see BOM) |

If the bare `ESP32-C3FH4` symbol isn't in your KiCad libraries, you may need to
add Espressif's library or grab the symbol from the manufacturer/SnapEDA. Tell me
if it's missing and we'll sort it.

Tips: **M** = move, **R** = rotate, **Esc** = cancel, mouse-wheel = zoom.

### 1b. Add power symbols
- Press **P** (Add Power). Place several **GND**, **+3V3**, and one **VBUS**(+5V)
  and **VBAT** symbol. These are how you connect grounds/power without drawing
  wires all across the sheet — every `GND` symbol is the same net automatically.

### 1c. Set resistor/cap values
- Hover a part, press **E** (Edit), set the Value (e.g. `4.7k`, `100n`, `22u`).
  Match the values in `DESIGN.md` / `BOM.md`.

### 1d. Wire it up
- Press **W** to draw a wire from pin to pin. Follow the **netlist table** in
  `DESIGN.md` section 5. Work one block at a time:
  1. SuperMini headers: wire 5V, GND, 3V3 and each GPIO to its net (§3/§5).
  2. Power: battery JST+ → switch → Schottky D1 → SuperMini 5V pin; JST− → GND.
  3. I2C: SDA/SCL with the two 4.7k pull-ups; BMP280 + MPU-6050 + 100nF each.
  4. SPI: microSD (4 signals + 2× 10k pull-ups + 10µF).
  5. Header: GPS (VCC/GND/TX/RX + 10µF).
- For messy long connections, use **labels** instead of wires: press **L**, type a
  net name (e.g. `SDA`), and stick it on a pin. Two pins with the same label name
  are connected. This keeps the sheet clean.

### 1e. Annotate + check
- Top toolbar: **Annotate Schematic** (gives parts R1, C1, U1… numbers).
- Run **ERC** (Electrical Rules Check). Fix real errors (unconnected pins,
  power not driven). Some warnings are OK; read them.

---

## Phase 2 — Assign footprints (physical shapes)
1. Tools → **Assign Footprints** (or the footprint-assignment button).
2. For each symbol pick the footprint from `BOM.md`. Common ones:
   - Resistors/caps → `R_0402_1005Metric` / `C_0402_1005Metric` for smallest
     (use `_0603_1608Metric` if you'd rather hand-solder them).
   - SuperMini rows → `PinSocket_1x08_P2.54mm_Vertical` (×2) if using female
     sockets (your SuperMini has male pins), or `PinHeader_1x08...` if soldering
     flat. Measure the spacing between your SuperMini's two rows and set the two
     connectors that far apart.
   - Schottky D1 → `D_SOD-123`.
   - GPS header / JST → 2.54mm header / JST PH footprints.
3. **For JLCPCB assembly**: for each SMD part JLC will solder, add a field named
   **`LCSC`** with the part number from `BOM.md` (Edit symbol → add field
   `LCSC` = e.g. `C6186`). This is what lets JLC auto-source the part. The
   easiest path is installing the **"JLCPCB Fabrication Toolkit"** plugin later —
   it reads these fields.

---

## Phase 3 — Layout & route the board
1. Back in the Schematic Editor: **Tools → Update PCB from Schematic** (F8).
   This opens the **PCB Editor** with all parts stacked together, connected by
   thin "ratsnest" lines. (This step replaces manually importing a netlist.)
2. **Board outline**: select the **Edge.Cuts** layer, draw a rectangle the size
   of your payload bay (e.g. 25×50 mm). Add mounting holes if you want.
3. **Place parts** (press M to move) sensibly:
   - ESP32-C3 **antenna at a board edge**, nothing under it.
   - USB-C at one end, battery JST/switch at the other.
   - Sensors central-ish; BMP280 where air can reach it.
4. **Route**: press **X** and click pin-to-pin to draw copper traces, following
   the ratsnest. Use the bottom layer (F.Cu/B.Cu) as needed; add **vias** (V) to
   switch layers. Keep power traces a bit wider (e.g. 0.4mm).
5. **Ground pour**: draw a filled zone on both copper layers tied to **GND**
   (Add Filled Zone), then **B** to fill. This makes grounding easy — keep it
   OUT of the antenna keep-out area.

---

## Phase 4 — Check it
- Run **DRC** (Design Rules Check). Fix any clearance/unrouted errors until it's
  clean (or only harmless warnings remain).

---

## Phase 5 — Export for JLCPCB
Easiest: install the **JLCPCB Fabrication Toolkit** plugin — one click makes the
Gerbers, BOM, and placement (CPL) files. Manual way:
1. `File → Fabrication Outputs → Gerbers (.gbr)` → export all layers → **zip** them.
2. `File → Fabrication Outputs → Drill Files` → add to the zip.
3. `File → Fabrication Outputs → Component Placement (.pos)` → CPL file.
4. Schematic Editor: `Tools → Generate BOM` → the BOM (with your `LCSC` column).
5. On jlcpcb.com: upload the Gerber zip, turn on **SMT Assembly**, upload the BOM
   + CPL, match/confirm each part, review the render, order.

---

## When you're stuck
KiCad has a curve — getting stuck is normal. Tell me which **phase and step**
you're on and what you see, and I'll walk you through that specific bit. Common
first snags: a symbol not found (use nearest match), ERC "power not driven"
(add a `PWR_FLAG` on your +3V3/VBUS/GND nets), or footprints not matching pins.
