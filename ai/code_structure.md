# Code Structure and Organization

Authoritative rules for organizing Arduino C++ in this repo. Keep it modular, Arduino-IDE compatible, and easy to navigate.

---

## 1) Root Directory
- The **only** C++ file at project root is the main sketch:
  - `MyProject.ino`
- **Everything else** lives under `src/`.

---

## 2) Folder Layout (placeholders only)
Organize by concept under `src/` using clear, single-purpose folders. Avoid hard-coding names in this guide so the structure remains stable as files move.

```

MyProject/
├── MyProject.ino
└── src/
├── modules/
│   └── some\_module\_name/
│       └── some\_module\_name.h      // header-only preferred (see §4, §5)
├── feature\_area\_a/
│   └── some\_header.h
└── feature\_area\_b/
└── another\_header.h

````

Notes:
- **`src/modules/` is reserved** for audio processing modules (see §5).
- Other folders are placeholders (e.g., `feature_area_a/`)—name them to match function (sensors, i2s, ui, hw, etc.).
- Keep each folder self-contained (all `.cpp` in a folder compile together if any header from that folder is included).

---

## 3) Arduino IDE Compilation Rules (must-know)
- **Auto-compiled:** `MyProject.ino`, and **all** `.cpp` files inside `src/` (recursively).
- **Not auto-compiled:** `.cpp` outside `src/`, or in arbitrary non-`src/` subfolders at root.
- **Include behavior:** If you include **any header** from a folder, **all `.cpp` in that folder** are compiled. Design folders so their contents can coexist.

---

## 4) Header-Only Preferred
Use header-only with `inline` methods for simplicity and predictable linkage.

Example (template):
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
````

---

## 5) Modules Convention (`src/modules/`)

**Purpose:** Encapsulate a processing module that the main sketch can load/select.

**Location & naming:**

* Each module resides in: `src/modules/<some_module_name>/<some_module_name>.h`
* Module should **assume it is the only module loaded**; no internal preprocessor selection.
* The main sketch is responsible for selecting/including exactly one active module.

**Minimal module surface (template):**

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

**Integration in `MyProject.ino` (conceptual):**

```cpp
// Select one module by including its header:
#include "src/modules/some_module_name/some_module_name.h"

// The sketch provides processPath(), I2S setup, buffers, and calls:
//   moduleSetup();
//   moduleLoopUpstream(...);
//   moduleLoopDownstream(...);
```

Guidelines:

* Keep module headers self-contained; avoid side-effects outside module scope.
* No global `#define` switches inside modules for choosing other modules.
* If a module needs helpers, place them in its own folder or a shared feature folder—ensure co-compilation safety (§3).

---

## 6) When `.cpp` Is Necessary

* Place `.cpp` **only** under `src/` (or its subfolders).
* Always `#include "Arduino.h"` and the corresponding header.
* Keep folders conflict-free: everything in a folder must be safe to compile together.

---

## 7) Do / Don’t (quick)

**Do**

* Keep only `MyProject.ino` at root; everything else under `src/`.
* Prefer header-only; use `inline` to avoid ODR/link issues.
* Use `src/modules/<some_module_name>/<some_module_name>.h` for processing modules.
* Organize by function; design folders so all contained `.cpp` can compile together.

**Don’t**

* Put `.cpp` at root or outside `src/`.
* Mix incompatible components in one folder.
* Depend on hidden global switches inside modules.

---

## 8) Naming Conventions (lightweight)

* **Macros/constants:** `ALL_CAPS_WITH_UNDERSCORES` (e.g., `SAMPLE_RATE`, `BUFFER_LEN`).
* **Classes/Types:** `PascalCase` (e.g., `TemperatureSensor`).
* **Functions/Methods/Variables:** `camelCase` (e.g., `processPath`, `rxBufferUpstream`).
* **Files/Folders:** lowercase, short, descriptive (e.g., `modules/`, `some_module_name.h`).

---