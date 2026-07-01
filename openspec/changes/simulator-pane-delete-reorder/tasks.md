## 1. Structural reconfiguration orchestration

- [ ] 1.1 Add a shared helper in the simulator host that detects structural edits and, when audio is running, stops the engine, clears per-unit level histories and playback buffers, re-syncs the chain scheduler, and auto-restarts while preserving mic toggle and pot slider positions
- [ ] 1.2 Wire the helper into add-unit, module-type change, and loopback toggle handlers (pot sliders must not invoke it)
- [ ] 1.3 Surface restart progress or errors in the status line without leaving the chain in a half-updated state

## 2. Delete processing units

- [ ] 2.1 Add a delete control to each processing unit card with accessible labeling
- [ ] 2.2 Implement removal: destroy WASM bindings and level graphs, splice the units array, refresh DOM order, renumber unit labels, update loopback visibility, and invoke structural rebuild when audio is running
- [ ] 2.3 Disable delete on the sole remaining unit while audio is running; re-enable add-unit when below the maximum after a delete

## 3. Reorder processing units

- [ ] 3.1 Add a dedicated drag-handle control to each unit card header with styles that distinguish it on touch (`touch-action: none` on the handle)
- [ ] 3.2 Implement drag-and-drop reorder among unit cards only (gateway stays fixed); restrict drag initiation to the handle so sliders and selectors do not reorder
- [ ] 3.3 On successful drop, update `units[]`, DOM order, displayed indices, scheduler slot order, loopback visibility, and invoke structural rebuild when audio is running

## 4. Verification and documentation

- [ ] 4.1 Update `sim/README.md` with delete, reorder, grab-handle, and audio-rebuild behavior
- [ ] 4.2 Extend headless or manual verification notes for delete/reorder DOM and scheduler order (document touch QA on iOS Safari and Android Chrome in verification notes)
- [ ] 4.3 Run existing WASM verification (`sim/verify-wasm.mjs`) and confirm no regressions

## Explicitly deferred

- Undo/redo for delete or reorder
- Keyboard-accessible reorder shortcuts without pointer drag
- Animated drag placeholders or cross-row chain layout polish beyond minimal drop feedback
