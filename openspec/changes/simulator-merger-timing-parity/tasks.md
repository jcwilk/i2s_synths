## 1. Chain scheduler path timing

- [ ] 1.1 Refactor chain host routing to maintain separate per-slot delayed state for upstream-path inputs and downstream-path inputs, preserving gateway and loopback rules
- [ ] 1.2 Ensure right-to-left upstream sweep and left-to-right downstream propagation use path-correct delayed feeds rather than same-period snapshots
- [ ] 1.3 Reset all path delay state when the chain length, order, or module type changes (compatible with structural audio rebuild from pane UX work)

## 2. Merger stress feedback

- [ ] 2.1 Surface brief operator-visible underrun and overrun indications on merger unit cards when the compiled module triggers those recovery paths
- [ ] 2.2 Prefer a minimal WASM harness export for stress events if needed; avoid duplicating merger ring logic in the host language

## 3. Verification and documentation

- [ ] 3.1 Add headless chain-routing verification for merger delayed blend and forward-path behavior in a short multi-buffer scenario
- [ ] 3.2 Update simulator README limitations table: remove merger timing as MVP gap; list any remaining intentional deltas (startup mute, render quantum, etc.)
- [ ] 3.3 Extend manual acceptance matrix with passthrough → merger → delay (loopback) scenario focused on merge latency and feedback
- [ ] 3.4 Run existing WASM verification and new merger chain checks without regression

## Explicitly deferred

- Firmware startup mute modeling in the simulator
- Web Audio render-quantum to firmware-buffer internal ring buffering
- Hardware A/B buffer capture regression harness
- npm/CI standard entrypoints (Phase 3)
