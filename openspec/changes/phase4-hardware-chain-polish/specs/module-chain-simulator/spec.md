## ADDED Requirements

### Requirement: Hardware transport path selection

The module chain simulator SHALL allow the operator to choose between Web Serial direct transport and bridge relay transport before starting a hardware session, SHALL default to bridge transport when Web Serial is unavailable in the current browser, and SHALL require explicit disconnect before switching transport while a session is active or degraded.

#### Scenario: Operator selects Web Serial on supported browser

- **GIVEN** a Chromium-based browser with Web Serial available and a hardware slot designated
- **WHEN** the operator selects Web Serial transport and connects successfully
- **THEN** the hardware slot exchanges audio without a running bridge server

#### Scenario: Bridge default on unsupported browser

- **GIVEN** a browser without Web Serial support
- **WHEN** the operator views hardware transport controls
- **THEN** bridge transport is selected or implied and Web Serial is not offered as an active choice

#### Scenario: Transport switch requires disconnect

- **GIVEN** an active or degraded hardware session using one transport
- **WHEN** the operator attempts to select the other transport
- **THEN** the change is blocked until the current session is disconnected

### Requirement: Hardware session reconnect operator flow

The module chain simulator SHALL present operator-visible reconnect guidance when a hardware session enters degraded state, SHALL offer a reconnect action that preserves chain layout and hardware designation, and SHALL surface fatal recovery errors with actionable messaging without requiring a full browser restart for recoverable link-loss events.

#### Scenario: Connection lost banner with reconnect

- **GIVEN** a running mixed chain whose hardware session lost transport link
- **WHEN** the operator views the hardware unit card
- **THEN** a connection-lost indication is visible and a reconnect action is available

#### Scenario: Successful reconnect preserves chain layout

- **GIVEN** a degraded hardware session with an unchanged chain topology
- **WHEN** the operator completes reconnect successfully
- **THEN** the same hardware slot resumes participation without the operator re-designating hardware mode or rebuilding the chain layout manually

#### Scenario: Reconnect failure shows actionable error

- **GIVEN** a reconnect attempt fails due to port busy, module-kind mismatch, or unreachable device
- **WHEN** recovery cannot complete
- **THEN** the operator sees a specific error message and guidance consistent with hardware-module-bridge troubleshooting documentation

### Requirement: Operator setup and troubleshooting documentation in simulator

The module chain simulator documentation SHALL link to or include operator setup and troubleshooting guidance for hardware integration, covering transport selection, reconnect after unplug, port conflicts with upload tools, bridge versus Web Serial recommendations, firmware mode selection, and known deltas versus pure simulator chains including the Phase 3 mixed-chain latency budget.

#### Scenario: Simulator docs reference hardware setup guide

- **GIVEN** an operator reading simulator known-limitations or hardware integration documentation
- **WHEN** they seek setup instructions for connected hardware auditions
- **THEN** transport selection, connect flow, and troubleshooting entry points are documented or linked

#### Scenario: Known deltas include transport and reconnect notes

- **GIVEN** hardened hardware slot support with Phase 4 polish delivered
- **WHEN** an operator reads simulator documentation for known limitations
- **THEN** Web Serial versus bridge availability, reconnect behavior, and port exclusivity expectations are listed alongside existing hardware deltas such as async pipeline buffering and the single hardware slot limit

## MODIFIED Requirements

### Requirement: Hardware unit operator controls

Each processing unit card SHALL expose hardware connection controls when hardware mode is enabled for that unit, including transport path selection when the browser supports more than one path, connect and disconnect actions, reconnect action when the session is degraded, and visible connection status for the active transport serving that slot.

#### Scenario: Connect starts hardware session

- **GIVEN** a unit is designated hardware with a module kind matching connected firmware and the selected transport is available
- **WHEN** the operator activates connect
- **THEN** a hardware relay session starts on the selected transport and the slot begins participating in chain audio once structural reconfiguration completes

#### Scenario: Disconnect stops hardware participation

- **GIVEN** a connected hardware slot with audio running
- **WHEN** the operator activates disconnect
- **THEN** the slot stops exchanging audio through the active transport and follows structural reconfiguration rules

#### Scenario: WASM controls hidden for hardware slot

- **GIVEN** a unit is designated and connected as hardware
- **WHEN** the operator views that unit card during playback
- **THEN** simulated potentiometer sliders for that unit are not the active control path and the card indicates that physical device controls apply

#### Scenario: Reconnect action on degraded session

- **GIVEN** a hardware slot in degraded state after link loss
- **WHEN** the operator activates reconnect and recovery succeeds
- **THEN** the slot returns to connected active exchange without requiring a full browser restart

### Requirement: Documented remaining simulator deltas

After merger timing parity is implemented, the simulator documentation SHALL list any intentional remaining differences from on-device chain behavior so operators can interpret audition results. When hardware slot integration is available, the documentation SHALL additionally list hardware-specific deltas including USB and transport round-trip latency, the published mixed-chain latency budget and tolerance, async pipeline buffering, transport path availability by browser, reconnect and port exclusivity expectations, and the single-hardware-slot limit.

#### Scenario: Post-parity delta list

- **WHEN** an operator reads simulator documentation for known limitations
- **THEN** merger cross-path timing is not listed as an intentional MVP approximation and any remaining gaps (such as startup mute or render-quantum buffering) are explicitly named

#### Scenario: Hardware integration deltas named

- **WHEN** an operator reads simulator documentation after Phase 4 hardware polish is delivered
- **THEN** added USB latency, transport path options, reconnect behavior, the published mixed-chain latency budget, async pipeline buffering, and the single hardware slot limit are explicitly listed among known differences from an all-hardware or all-WASM reference

### Requirement: Audio rebuild on structural reconfiguration

The simulator SHALL treat the following operator actions as structural reconfiguration: adding a processing unit, removing a processing unit, reordering processing units, changing a unit's module type, toggling loopback on the rightmost unit, designating or clearing hardware mode on a unit, and connecting or disconnecting a hardware slot session. Transport reconnect after link loss SHALL NOT be structural reconfiguration. When audio is running and the operator performs structural reconfiguration, the simulator SHALL stop audio output, clear buffered playback and per-unit level histories, re-establish the audio processing graph for the updated chain, and resume continuous playback without requiring a manual start action. Potentiometer slider adjustments on WASM units SHALL NOT be structural reconfiguration. Physical potentiometer movement on a connected hardware device SHALL NOT be structural reconfiguration.

#### Scenario: Reorder restarts audio automatically

- **GIVEN** audio is running with at least two processing units
- **WHEN** the operator completes a reorder using the drag handle
- **THEN** audio briefly stops, level histories clear, and playback resumes on the updated chain order

#### Scenario: Module type change restarts audio

- **GIVEN** audio is running on a WASM processing unit
- **WHEN** the operator selects a different module type for that unit
- **THEN** audio briefly stops, that unit's level histories clear, and playback resumes with the new module type active

#### Scenario: Delete restarts audio

- **GIVEN** audio is running with at least two processing units
- **WHEN** the operator removes a unit
- **THEN** audio briefly stops, removed unit metering disappears, and playback resumes on the remaining units

#### Scenario: Pot adjustment stays live on WASM units

- **GIVEN** audio is running on a WASM processing unit
- **WHEN** the operator moves a potentiometer slider without any structural reconfiguration
- **THEN** audio continues without stop or rebuild and output reflects the new control position within a small number of buffer periods

#### Scenario: Hardware connect restarts audio

- **GIVEN** audio is running on a chain with a hardware-designated unit that is not yet connected
- **WHEN** the operator completes a successful hardware connect
- **THEN** audio briefly stops, level histories clear, and playback resumes with the hardware slot active in the chain

#### Scenario: Hardware reconnect does not rebuild chain layout

- **GIVEN** audio is running on a mixed chain whose hardware session is degraded after link loss
- **WHEN** the operator completes a successful reconnect without changing chain layout
- **THEN** WASM neighbors continue under degraded recovery until exchanges resume and a full structural rebuild is not required solely because of reconnect

#### Scenario: Structural edit while stopped

- **GIVEN** audio is not running
- **WHEN** the operator adds, removes, reorders units, changes module type, toggles loopback, or changes hardware designation or connection state
- **THEN** the chain layout and configuration update without starting audio automatically
