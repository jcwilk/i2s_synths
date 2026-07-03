## ADDED Requirements

### Requirement: Web Serial direct PWA transport

The hardware module bridge PWA integration SHALL support an optional Web Serial direct transport path that exchanges full-rate four-path duplex audio with a connected device without requiring the local bridge server, using the same exchange contract and sample geometry as the bridge relay path, with no downsampling, compression, or mono fallback.

#### Scenario: Web Serial connect without bridge

- **GIVEN** a Chromium-based browser with Web Serial available and a compatible device attached
- **WHEN** the operator selects Web Serial transport and completes a successful hardware connect for a designated slot
- **THEN** the simulator sustains full-rate duplex exchanges directly with the device without a running bridge server

#### Scenario: Web Serial preserves exchange geometry

- **GIVEN** an active Web Serial hardware session
- **WHEN** one complete buffer-period exchange completes
- **THEN** each audio path uses the same full 44.1 kHz stereo int16 geometry as the bridge relay path

#### Scenario: Web Serial unavailable falls back to bridge guidance

- **GIVEN** a browser without Web Serial support
- **WHEN** the operator views hardware connection controls
- **THEN** Web Serial transport is unavailable and the operator is directed to use the bridge transport path

### Requirement: Transport feature parity

The Web Serial direct transport and the bridge relay transport SHALL provide equivalent hardware-slot audio behavior for all simulator-supported module kinds, including physical control source semantics, pot telemetry relay when enabled, and Phase 3 graceful recovery policies, differing only by how the host opens the device connection.

#### Scenario: Equivalent module-kind relay on Web Serial

- **GIVEN** matching firmware for a supported module kind and an active Web Serial session
- **WHEN** the operator runs a mixed chain with microphone input
- **THEN** module-appropriate processing, physical pot authority, and recovery behavior match what the bridge path provides for the same topology

#### Scenario: Bridge remains required for unsupported browsers

- **GIVEN** a browser that does not expose Web Serial
- **WHEN** the operator connects a hardware slot
- **THEN** sustained relay is available through the bridge transport path at full fidelity

### Requirement: Session reconnect and recovery

The hardware module bridge PWA integration SHALL detect transport or device link loss during an active hardware session, keep WASM neighbors running under documented degraded recovery semantics, and allow the operator to reconnect and resume hardware-slot exchanges without a full browser restart when the device and transport become available again.

#### Scenario: Link loss enters degraded state

- **GIVEN** an active hardware session with audio running through a mixed chain
- **WHEN** the transport link is lost unexpectedly
- **THEN** the session enters a degraded state, hardware-slot paths follow documented silence substitution, WASM neighbors continue processing, and the operator sees that the connection was lost

#### Scenario: Operator reconnect restores active session

- **GIVEN** a hardware session in degraded state with chain layout and hardware designation unchanged
- **WHEN** the operator initiates reconnect and the device responds successfully to USB neighbor entry
- **THEN** full-rate exchanges resume, the session returns to active state, and a full browser restart is not required

#### Scenario: Fatal error prevents silent reconnect loop

- **GIVEN** a reconnect attempt encounters module-kind mismatch or a documented fatal device status
- **WHEN** recovery cannot proceed safely
- **THEN** the session moves to a clean disconnected state with a visible actionable error rather than retrying indefinitely

#### Scenario: Clean disconnect from degraded state

- **GIVEN** a hardware session in degraded state
- **WHEN** the operator chooses disconnect
- **THEN** the session tears down orderly, the device exits USB neighbor mode when reachable, and the slot follows disconnected structural rules

### Requirement: Firmware operating mode selection

The hardware module bridge documentation and firmware behavior SHALL distinguish normal I2S chain operating mode from USB neighbor dev operating mode, with normal I2S chain mode as the default on cold boot, USB neighbor mode entered only through documented host session actions, and return to normal I2S mode on orderly session end without requiring a reflash.

#### Scenario: Cold boot defaults to normal I2S chain mode

- **GIVEN** firmware with USB neighbor capability installed
- **WHEN** the device completes a cold boot without a host session start action
- **THEN** the device operates in normal I2S chain mode with live I2S neighbor streaming available

#### Scenario: PWA session enters USB neighbor dev mode

- **GIVEN** the device is in normal I2S chain mode and a host connects via bridge or Web Serial
- **WHEN** the operator starts a hardware session successfully
- **THEN** the device enters USB neighbor dev mode, accepts host-supplied buffer exchanges, and disables live I2S neighbor input for module processing

#### Scenario: Session end returns to normal I2S chain mode

- **GIVEN** the device is in USB neighbor dev mode through an active or degraded PWA session
- **WHEN** the operator disconnects or the session completes orderly teardown
- **THEN** the device exits USB neighbor mode and resumes normal I2S chain behavior without reflash

### Requirement: Operator setup and troubleshooting documentation

The hardware module bridge documentation SHALL provide an operator-facing setup guide and troubleshooting section covering transport selection, firmware mode selection, BOOT and upload workflow, serial port conflicts between upload tools and connected sessions, bridge versus Web Serial choice, reconnect procedure after cable unplug, and known behavioral deltas versus pure simulator chains.

#### Scenario: Operator finds transport guidance

- **GIVEN** an operator preparing a hardware audition
- **WHEN** they read the setup guide
- **THEN** the guide explains when to use Web Serial direct connection versus the bridge server and which browsers support each path

#### Scenario: Operator resolves port conflict

- **GIVEN** a connect attempt fails because the serial port is held by an upload tool or another process
- **WHEN** the operator reads troubleshooting guidance
- **THEN** the documentation explains port exclusivity and the steps to release the port before reconnecting

#### Scenario: Operator understands mode selection

- **GIVEN** an operator unsure whether to run standalone I2S chain or PWA dev mode
- **WHEN** they read mode selection documentation
- **THEN** the guide states when normal I2S chain mode applies versus USB neighbor dev mode and how to switch through session actions

## MODIFIED Requirements

### Requirement: PWA duplex session lifecycle

The bridge server and module chain simulator SHALL coordinate an explicit session lifecycle: connect, enter USB neighbor processing on the device, sustain exchanges while audio runs, detect link loss and enter degraded recovery when applicable, allow operator reconnect without full browser restart, and disconnect with orderly teardown that returns the device to normal I2S behavior when the session ends. Web Serial direct sessions SHALL follow the same lifecycle semantics without requiring the bridge server.

#### Scenario: Session start enters USB neighbor mode

- **GIVEN** the simulator requests a hardware connection using bridge or Web Serial transport and a device is available
- **WHEN** the session start action completes successfully
- **THEN** the device is in USB neighbor mode and the simulator can submit realtime exchanges through the selected transport

#### Scenario: Session end restores normal device mode

- **GIVEN** an active hardware relay session
- **WHEN** the operator disconnects or the simulator tears down the session
- **THEN** the transport session closes, relaying stops, and the device exits USB neighbor mode without requiring a reflash

#### Scenario: Disconnect while audio running

- **GIVEN** audio is running through a hardware slot
- **WHEN** the operator disconnects the hardware session
- **THEN** the simulator stops treating the slot as connected, ceases exchanges for that slot, and follows structural reconfiguration rules for the chain

#### Scenario: Reconnect without browser restart

- **GIVEN** a degraded hardware session after unexpected link loss with chain layout preserved
- **WHEN** the operator completes a successful reconnect action
- **THEN** USB neighbor mode is re-entered and exchanges resume without reloading the browser tab

### Requirement: Bridge connection status visibility

The bridge server, Web Serial transport, and simulator SHALL expose connection status visible to the operator, including whether the selected transport is available, whether a device is reachable, whether an active relay session is in progress, and whether the session is degraded and awaiting reconnect.

#### Scenario: Disconnected bridge indicated

- **GIVEN** the simulator is open, bridge transport is selected, and no bridge server is reachable
- **WHEN** the operator views hardware connection controls
- **THEN** the operator sees that the bridge is unavailable and cannot start a hardware session on that transport

#### Scenario: Device attached without session

- **GIVEN** the selected transport is ready with a device reachable but no active session
- **WHEN** the operator views hardware connection controls
- **THEN** the operator sees the device as available and may start a session when a slot is designated hardware

#### Scenario: Degraded session indicated

- **GIVEN** an active hardware session that lost its transport link
- **WHEN** the operator views the hardware unit card
- **THEN** the operator sees a connection-lost indication and a reconnect action

### Requirement: Documented USB round-trip latency delta

The hardware module bridge documentation SHALL state that substituting a hardware slot introduces additional end-to-end latency from USB round-trip, selected transport relay when using the bridge path, and async pipeline buffering compared with an all-WASM chain at the same buffer geometry, SHALL publish a numeric planning budget and tolerance for mixed-chain audition consistent with Phase 3 hardened guidance, and SHALL describe how operators should interpret that delta during extended runs for both bridge and Web Serial transport paths.

#### Scenario: Operator reads latency guidance

- **GIVEN** documentation for PWA hardware slot integration
- **WHEN** an operator reviews known limitations before an extended hardware audition
- **THEN** added USB, transport relay when applicable, and pipeline latency is explicitly named with a published budget and tolerance, including notes for Web Serial versus bridge paths where they differ
