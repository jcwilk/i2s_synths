# Code Structure and Organization (Consolidated)

Authoritative rules for organizing Arduino C++ in this repo. Keep it modular, Arduino‑IDE compatible, and easy to navigate.

---

## 1) Project Layout

* The **only** C++ file at project root is the main sketch: `MyProject.ino`.
* **Everything else** lives under `src/`.

**Example layout**

```
MyProject/
├── MyProject.ino
└── src/
    ├── modules/
    │   └── some_module_name/
    │       └── some_module_name.h
    ├── feature_area_a/
    │   └── some_header.h
    └── feature_area_b/
        └── another_header.h
```

**Notes**

* `src/modules/` is **reserved** for audio processing modules (see §4).
* Other folders are placeholders—name them to match function (e.g., `sensors`, `i2s`, `ui`, `hw`).

---

## 2) Arduino IDE Compilation Rules (authoritative)

* **Auto‑compiled:** `MyProject.ino` and **all** `.cpp` files inside `src/` (recursively).
* **Not auto‑compiled:** `.cpp` files **outside** `src/` or in arbitrary non‑`src/` subfolders at root.
* **Include behavior (co‑compile safety):** If you include **any header** from a folder, **all `.cpp` in that folder** are compiled. Design folders so their contents can coexist.

---

## 3) Header‑Only Preferred

Prefer header‑only components with `inline` methods for simplicity, predictable linkage, and to avoid ODR issues.

**Example (template)**

```cpp
// src/feature_area_a/some_header.h
#ifndef SOME_HEADER_H
#define SOME_HEADER_H

#include "Arduino.h"

class ExampleThing {
public:
  inline void begin() { /* ... */ }
  inline int update() { /* ... */ return 0; }
};

#endif
```

---

## 4) Modules Convention (`src/modules/`)

**Purpose:** Encapsulate an audio processing module that the main sketch can load/select.

**Location & naming**

* Each module resides at: `src/modules/<name>/<name>.h`.

**Selection model**

* The sketch selects **exactly one** active module by including its header.
* Modules assume they are the only loaded module; **no preprocessor‑based selection** or global `#define` switching **inside** modules.

**Minimal module surface (template)**

```cpp
// src/modules/some_module_name/some_module_name.h
#ifndef SOME_MODULE_NAME_H
#define SOME_MODULE_NAME_H

#include "Arduino.h"
#include <driver/i2s.h>

// Called once during setup.
inline void moduleSetup() {
  // init peripherals / state
}

// Process upstream path: inputBuffer (from I2SD) -> outputBuffer (to I2SU)
inline void moduleLoopUpstream(int16_t* inputBuffer,
                               int16_t* outputBuffer,
                               int samplesLength) {
  // write exactly samplesLength int16_t samples to outputBuffer
}

// Process downstream path: inputBuffer (from I2SU) -> outputBuffer (to I2SD)
inline void moduleLoopDownstream(int16_t* inputBuffer,
                                 int16_t* outputBuffer,
                                 int samplesLength) {
  // write exactly samplesLength int16_t samples to outputBuffer
}

#endif
```

**Integration in `MyProject.ino` (conceptual)**

```cpp
// Select one module by including its header:
#include "src/modules/some_module_name/some_module_name.h"

// The sketch provides processPath(), I2S setup, buffers, and calls:
//   moduleSetup();
//   moduleLoopUpstream(...);
//   moduleLoopDownstream(...);
```

**Guidelines**

* Keep module headers self‑contained; avoid side‑effects outside module scope.
* If a module needs helpers, place them in its own folder or a shared concept folder—ensure co‑compilation safety per §2.

---

## 5) Concept‑Specific Headers (no junk drawer)

Instead of a global “shared utils” header, create focused, concept‑scoped headers. Modules include only the concepts they use. Prefer header‑only (see §3).

**Structure**

```
src/
├── concept_a/
│   └── concept_a.h
├── concept_b/
│   └── concept_b.h
└── modules/
    └── <module>/<module>.h   // Processing modules (see §4)
```

* If a symbol is needed by a module and not in a concept header, add it there first; do not declare it in the module header.

**Minimal concept header template**

```cpp
// src/concept_a/concept_a.h
#ifndef CONCEPT_A_H
#define CONCEPT_A_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#endif

// Public constants/types for this concept go here (generic).

// Public inline APIs for this concept go here (generic shapes).
inline int apiFunction(/* args */) {
  // implement using Arduino APIs under ARDUINO, otherwise stub (see §7)
  #ifdef ARDUINO
  // ...
  #else
  return 0;
  #endif
}

#endif
```

---

## 6) When `.cpp` Is Necessary

* Use `.cpp` when you truly need separate translation units (e.g., large implementations, to hide heavy code, etc.).
* Always `#include "Arduino.h"` and the corresponding header.
* Keep code in each folder compatible with the co‑compile behavior described in §2.

---

## 7) Lint‑Only Stubs and Portability

**Goal:** Allow headers to parse under non‑Arduino tooling without breaking Arduino builds.

**Rules**

* Never redefine Arduino core enums/macros (e.g., `ADC_11db`).
* Prefer `#ifdef ARDUINO` to gate Arduino‑specific code and includes.
* Under non‑Arduino builds, provide no‑op or trivial stubs **only** for your own concept APIs, not core symbols.
* If you need simple utilities (e.g., min/abs) in headers, define local helpers with distinct names to avoid macro collisions.

**Pattern (generic)**

```cpp
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#include <string.h>
#endif

// Local helpers to avoid relying on Arduino macros in headers
static inline uint32_t util_min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
static inline int32_t util_abs_i32(int32_t v) { return v < 0 ? -v : v; }

#ifndef ARDUINO
// Provide stubs for your own concept APIs only
static inline void conceptBegin() {}
#endif
```

---

## 8) Helper Linkage and Ownership

* Do **not** declare sketch‑local functions `extern` in module headers.
* If multiple modules need a helper, move it into a concept header (§5) where its signature and ownership are clear.
* For temporary bridging, add a thin wrapper in the relevant concept header that calls the sketch function under `#ifdef ARDUINO` and provides a safe stub otherwise.
* Do not provide fallback #defines for project‑owned shared constants inside modules. Define them once in the relevant concept header and include that.

**Example wrapper pattern (temporary during migration)**

```cpp
// src/concept_a/concept_a.h
#ifdef ARDUINO
// declare the sketch‑provided function here only if its signature is stable
// void sketchConceptApi(/* args */);
inline void conceptApi(/* args */) {
  // sketchConceptApi(/* args */);
}
#else
inline void conceptApi(/* args */) { /* no‑op */ }
#endif
```

---

## 9) Naming Conventions (lightweight)

* **Macros/constants:** `ALL_CAPS_WITH_UNDERSCORES` (e.g., `SAMPLE_RATE`, `BUFFER_LEN`).
* **Classes/Types:** `PascalCase` (e.g., `TemperatureSensor`).
* **Functions/Methods/Variables:** `camelCase` (e.g., `processPath`, `rxBufferUpstream`).
* **Files/Folders:** lowercase, short, descriptive (e.g., `modules/`, `some_module_name.h`).

---

## 10) Configuration & Constants

- Put globally relevant knobs and values users may tune in `src/config/constants.h` (e.g., sample rate, buffer counts, feature toggles).
- Keep derived or local thresholds as `constexpr` in the module where they’re used (e.g., computed fractions of a global setting). These are private implementation details and should not be edited directly.
- Document how derived constants relate to globals in comments near their definitions so changes propagate predictably.

## 11) API Hygiene

* Prefer int for mode/flag parameters in public APIs to avoid promotion/signature mismatches across C/C++ and toolchains; avoid narrow unsigned types (uint8_t) for such parameters unless protocol‑mandated.