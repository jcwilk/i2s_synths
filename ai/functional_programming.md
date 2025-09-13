## Functional Programming Guidelines (C++)

Goals
- Eliminate shared mutable state by centralizing data in explicit structs/classes.
- Express behavior as pure functions: input state → output state.
- Minimize in-place mutation; when unavoidable (e.g., strict ISR constraints), isolate and document it.

Core Patterns
- State container: a plain struct holds all variables for a subsystem.
- Transition function: a free function takes the old state (and inputs) and returns a new state.
- Query helper: a function that inspects state and returns a decision (boolean/enum) without changing state.
- Event adapter: a tiny function safe for ISR context that transforms state quickly or defers work.

Avoid Pass-by-Reference for Functional State
- Prefer pass-by-value for state structs and return the updated copy.
- Do not use non-const references to mutate state in-place from helpers; this obscures ownership and order of effects.
- For helpers that must produce both a new state and an immediate scalar result, accept the state by value, write the scalar to an out parameter, and return the new state.

Examples (generic, not tied to project files)

1) Counter with upper bound
```cpp
struct Counter { int value; int max; };

bool canIncrement(const Counter& c) { return c.value < c.max; }

Counter increment(Counter c) {
  if (c.value < c.max) c.value += 1;  // pure transition on copy
  return c;
}

Counter reset(Counter c) { c.value = 0; return c; }
```

2) Simple button debouncer
```cpp
struct Debounce {
  bool stableLevel;
  bool pendingLevel;
  unsigned long lastChangeMs;
  unsigned long thresholdMs;
};

// Observe a raw sample; returns new debounced state
Debounce observe(Debounce d, bool rawLevel, unsigned long nowMs) {
  if (rawLevel != d.pendingLevel) {
    d.pendingLevel = rawLevel;
    d.lastChangeMs = nowMs;
  }
  if ((nowMs - d.lastChangeMs) >= d.thresholdMs && d.stableLevel != d.pendingLevel) {
    d.stableLevel = d.pendingLevel; // state flips only after threshold
  }
  return d;
}

bool isPressed(const Debounce& d) { return d.stableLevel; }
```

3) Token-bucket rate limiter
```cpp
struct Bucket { float tokens; float ratePerSec; float capacity; unsigned long lastMs; };

Bucket accrue(Bucket b, unsigned long nowMs) {
  const float dt = (nowMs - b.lastMs) / 1000.0f;
  b.lastMs = nowMs;
  b.tokens = fminf(b.capacity, b.tokens + dt * b.ratePerSec);
  return b;
}

bool canConsume(const Bucket& b, float cost) { return b.tokens >= cost; }

Bucket consume(Bucket b, float cost) { if (b.tokens >= cost) b.tokens -= cost; return b; }
```

ISR Considerations
- Keep ISR handlers minimal; avoid blocking and large copies.
- Prefer returning new state and assigning once at the call site.

### ISR mailbox pattern (functional integration)
- Maintain a tiny POD mailbox with `volatile` monotonic counters that ISR callbacks increment.
- Allocate the mailbox once (e.g., on the heap during setup) and keep its pointer for the program lifetime.
- Pass the mailbox pointer to callback registration as user data; callbacks only increment counters.
- Avoid clearing counters from main code. Keep snapshot fields in state and compute deltas: `delta = (uint32_t)(total - seen)`.
- Add a pure sync step each tick: `state' = applyDeltas(state, mailbox)`, applying one transition per observed event.
- This isolates mutation to the ISR adapter, avoids stale references across the ISR boundary, and preserves pass-by-value state semantics.

Fail Fast on Sanity Checks
- Prefer explicit validation up-front and stop early if invariants are violated.
- Example:
```cpp
struct Bounds { int min; int max; };

int clampOrFail(int value, Bounds b, void (*fail)(const char*)) {
  if (b.min > b.max) { fail("invalid bounds"); }
  if (value < b.min) return b.min;
  if (value > b.max) return b.max;
  return value;
}
```
This pattern keeps errors visible and local, preventing silent corruption.

Naming Conventions
- Make: constructors/factories for initial state.
- Try*: attempts that may be refused without side effects.
- Maintain*: background housekeeping to restore invariants.
- Is*: pure predicates.

Testing & Extensibility
- Pure functions are easy to unit test: assert that outputs match expectations for given inputs.
- Multiple instances are supported by maintaining separate state objects and passing them through the same transitions.

When Mutation is Acceptable
- Only for tight ISR/hardware paths; isolate and document. Prefer migrating to returned-state patterns when possible.
