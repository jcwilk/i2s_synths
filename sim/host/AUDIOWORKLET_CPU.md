# AudioWorklet CPU budget (Phase 3)

During mixed max-chain soak, profile render quantum usage in Chrome DevTools → Performance → AudioWorklet thread.

| Budget | Threshold | Mitigation if exceeded |
|--------|-----------|-------------------------|
| Mixed max-chain AudioWorklet | < 50% of render quantum p99 | Reduce unit count during audition; close other tabs; prefer hardware slot on interior unit to shorten WASM fan-out |

**Mitigations documented for apply:**

1. **Chain length** — At eight WASM units plus hardware, worklet time scales linearly with slot count; stress matrix S1 is worst case.
2. **Level graph redraw** — Graphs update at 30 Hz on main thread; not on AudioWorklet path.
3. **Hardware async decouple** — USB/bridge I/O runs on `setInterval` at firmware buffer cadence (~5.8 ms), not inside the worklet.
4. **Structural rebuild** — Avoid add/delete/reorder during soak; only pot and mic toggles are non-structural.

If p99 exceeds 50% sustained over 60 s, soak fails per Phase 3 design; reduce chain length or disable loopback for diagnosis.
