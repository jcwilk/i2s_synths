# Hardware Overview

Authoritative entry point for hardware architecture and assets. Keep this concise and source-accurate.

---

## 1) Module Types & Naming

- **Gateway Module**: Digital+analog bridge using Pmod I2S2; sets the I2S operating parameters for the analog bridge; injects and distributes power.
- **Control Modules** (preferred term): Interchangeable, digital-only processing nodes with instrumented knobs.

**Interface naming (defined once):**
- **I2SD** (“Downstream”) = the **first** I2S interface (**aka interface 0**).
- **I2SU** (“Upstream”) = the **second** I2S interface (**aka interface 1**).
> Use **I2SD/I2SU** throughout. Do **not** use raw “I2S0/I2S1” elsewhere in docs or code comments.

---

## 2) Gateway Module (Digital+Analog)

**Purpose**
- Convert analog line-level audio ↔ digital (via **Pmod I2S2**).
- Establish I2S rate/format required by the Pmod bridge.
- Provide shared 5V power distribution to Control Modules.

**Core Silicon & Interfaces**
- **MCU**: ESP32-S3-Zero.
- **I2S roles**: **I2SD = master**, **I2SU = master** (both set master on the gateway to control Pmod I2S2 rate/specs).  
  *Note:* The gateway is **not** the single clock for the whole chain.

**Power (high-level)**
- USB-C 5V → bulk electrolytic + local HF ceramic → shared 5V rail (18 AWG) to modules.
- Local ESP32 branch: ferrite → diode → 1 Ω → local 5V node with bulk + HF decoupling.
- Clean 3.3V node (ferrite + decoupling) for Pmod power control via MOSFET.

**Sequencing**
1) ESP32 boots → 2) I2S clocks stabilize → 3) GPIO enables Pmod power → 4) Audio processing starts.

**Signal Flow**
- **Downstream**: Gateway (I2SD) → Control Module chain.  
- **Upstream**: Chain → Gateway (I2SU) → Pmod I2S2 → analog out.

---

## 3) Control Modules (Digital-Only)

**Purpose**
- Digital audio processing with physical controls (potentiometers) on a private, quiet 3.3V rail.

**Interfaces**
- **Upstream side (left)**: **I2SU = slave** to the module on its left.
- **Downstream side (right)**: **I2SD = master** to the module on its right.
- **Important:** Each module’s two interfaces may run on **different clock domains** → **potential clock mismatch** exists today and must be managed (to be addressed in a future revision).

**Power**
- Shared 5V rail (18 AWG) passes through each module.
- Per-module path: ferrite → 1 Ω → local 5V node with bulk + HF decoupling.
- ESP32 3.3V → ferrite → private 3.3V rail for controls (with local decoupling).
- Private ground ties to shared ground (22 AWG local).

**Interconnect**
- Short, direct header runs for SCLK/LRCK/SD lines; avoid stubs and long fan-outs.

---

## 4) Timing & Policy

- Maintain a consistent target **sample rate** (e.g., 44.1 kHz) and matching word-length/format across the chain.
- Gateway sets both of its interfaces to master **only** to satisfy Pmod I2S2 requirements.
- Control Modules: **master to the right, slave to the left**; be aware of **clock-domain boundary** between I2SD and I2SU within each module (risk of drift/jitter until resolved).

---

## 5) File Index (`ai/hardware/`)

- `ai/hardware/esp32-s3_datasheet_en.md`  
  Brief: ESP32-S3 peripherals (I2S), clocks, power domains, memory, register-level details relevant to timing and I/O. 83 pages long and very low level.

- `ai/hardware/ESP32-S3-Zero_datasheet.md`
  Brief: PDF printout of the waveshare ESP32-S3-Zero board's datsheet webpage. Some useful information about how the ESP32-S3 chip is connected and which pins are exposed via the headers.

- `ai/hardware/Pmod_I2S2_Reference.md`  
  Brief: Pmod I2S2 pinout, power, expected I2S formats and clocking, electrical interface notes. How to correctly start it up and interface with it.

---

## 6) Expectations for Coding Agents

- Treat this as the hardware starting point.  
- If a task involves I2S/power/pins, consult the PDFs above before making assumptions.  
- Flag assumptions explicitly when clock-domain interactions are implicated.

---
