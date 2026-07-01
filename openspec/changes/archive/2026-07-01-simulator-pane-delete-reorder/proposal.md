## Why

The chain MVP lets operators append processing units but not remove or reorder them. Mistakes require refreshing the page and rebuilding the chain from scratch. On mobile, there is also no clear affordance for rearranging units without conflicting with page scroll. Structural chain edits (add, delete, reorder, module type, loopback) can leave stale audio graph state running; operators need a clean audio restart when topology changes while pot adjustments should stay live.

## What Changes

- Add **delete** control on each processing unit card so operators can remove units from the chain (subject to a minimum chain length when audio is running).
- Add **drag-to-reorder** for processing units so chain order can be changed without reloading the page.
- Add a visible **drag handle** on each unit card so touch users can distinguish reorder gestures from vertical page scrolling.
- When the operator performs any **structural reconfiguration** (add unit, delete unit, reorder units, change module type, toggle loopback), the simulator SHALL **stop, clear, and restart** the audio stream if audio was running. **Pot slider** adjustments SHALL continue to apply live without restarting audio.
- Update chain wiring, rightmost loopback visibility, and unit labels after delete or reorder.
- Extend simulator verification and documentation for the new interactions.

**Non-goals:** preset save/load, undo/redo history, gateway card reordering, hardware firmware changes, changing the maximum unit count, or altering pot-smoothing behavior.

## Capabilities

### New Capabilities

<!-- None — extending module-chain-simulator -->

### Modified Capabilities

- `module-chain-simulator`: add removable and reorderable processing units with touch-friendly drag handles; require audio stream teardown and rebuild on structural reconfiguration while preserving live pot adjustment during playback.

## Impact

- **Specs:** Delta to `openspec/specs/module-chain-simulator/spec.md` (ADDED delete/reorder/audio-rebuild requirements; MODIFIED real-time operation and module-type-change requirements).
- **Repo:** `sim/host/` UI (unit cards, drag-and-drop, delete controls), chain scheduler sync, audio engine restart orchestration, styles for mobile grab handles, README and headless verification updates.
- **Dependencies:** No new runtime dependencies; browser pointer/touch events and existing Web Audio stack.
