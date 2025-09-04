# ESP32 Digital Modular Synthesizers Project v1

A project that grew out of the frustration of how expensive and tedious it is to explore hardware synthesizers.

### Problem

Traditional purely analog synthesizers are extremely time consuming and expensive to build and reconfigure, and are not generalizable except for
the signal passed between them. Exploring different ways of combining signals can be excruciating with extensive soldering required throughout, or
needing to buy many different expensive premade modules. Also, certain effects like delays are impractical in analog without siginificant signal degredation (eg writing/reading to tape loops) making things like looping a drone and continuously perturbing it infeasible.

Digital modular synthesizers tend to have analog interfaces between them, meaning they suffer from analog signal degredation, and require expensive and complicated ADC/DAC in every module. Although they are more suitable for delay effects, if signals are continously looped through these modules there will be distortions introduced and built up over time from noise which always exists to some degree in analog signals, as well as potential artifacts introduced by ADC/DAC. Each loop brings it in and out of analog over and over, degrading the signal.

Digital semi-modular systems address this by keeping the signal digital within the larger meta-module and introducing interfaces for connecting the virtual modules together in different ways, but then you're at the mercy of the monolithic semi-modular system and it's impossible to introduce your own custom physical modules because the system is virtualized.

There isn't a widely used approach which satisfies all of the following:
* Physically distinct modular system
* Modules can be swapped out for different custom built ones with a standardized digital interface
* Digital-only signals between modules of the chain
* Typically one gateway which will serve to capture analog signals and bring them into the chain as digital signals, as well as take the output digital signal and convert it to analog
* Arbitrarily reprogrammable modules which operate and communicate strictly at the sample level, any concepts of "notes" or operations or anything would be fully internal to the module
* Physical knobs for every module which can be arbitrarily programmed

### Solution

Use a series of arduino-compatible ESP32 chips (specifically, for now, the ESP32-S3-Zero) and connect them in a chain with the first module being a special gateway module which has analog input and analog output. The modules communicate with each other via I2S, a low level digital sound interface for streaming digital samples.

Each ESP32 chip has two I2S interfaces which each provide their own input and output. I2S requires each connection to have exactly one master, with the other participants needing to be configured as slave. This provides a challenge in an arbitrarily arranged modular system, leading to a design where the modules must be connect in a linear manner going towards the right from the gateway:

```
gateway (master) -> (slave) MODULE A (master) -> (slave) MODULE B (master) -> ...etc
```

Basically each module serves as master for the connection to its right, and serves as slave for the connection to its left. This works because the last module on the right will be master with corresponding slave, which works fine since a clock signal is still being generated.

Because each interface is full duplex (input and output) this means that we can think of the system as having downstream and upstream sample flows:

```
  (line out) | <-------- upstream ----------
gateway      | MODULE A - MODULE B - MODULE C 
  (line in)  | --------- downstream ------->
```

And then we can use a special connector at the end to connect the last module's downstream output to its upstream input, which causes a "U turn" of the downstream back into the upstream which will eventually make its way back to the gateway's analog line out.

Because modules are arbitrarily programmable, they can tap into these two streams and redirect samples between them. For example, if in the above diagram "MODULE A" had a feature where it merged upstream samples back into downstream samples then it would form an echo/feedback loop. In other words, even though the modules physically form a linear chain, non-linear behaviors can still be induced due to the downstream/upstream paradigm.

### Hardware Design

For version 1 the hardware built around the ESP32-S3-Zero chip is extremely minimal with lots of stranded wires soldered into a breadboard, long transmission lines, and other problematic rushed aspects as I just wanted to confirm the overall paradigm and start seeing some results before going down too many hardware rabbit holes. I won't go into the specifics of the hardware further because I don't recommend it to be reproduced, see version 2 for better circuit designs.

Waveshare ESP32-S3-Zero chips for all modules
Waveshare WM8960 Audio HAT attached to the gateway module for ADC/DAC

### Pinouts

See [./synths.ino](synths.ino) for which specific pin was chosen, but in general for the non-gateway modules:
* 4 pins for downstream I2S interface (bit clock, word clock, input data, output data)
* 4 pins for upstream I2S interface (bit clock, word clock, input data, output data)
* 2 pins for potentiometers

The gateway module is mostly the same except it needs another two pins for I2C to configure the WM8960

### Versions

v2 - the next version currently under construction
* Each module only sends signals to its upstream and downstream neighbors
* Each interface dedicated to a particular neighbor
* Shared power rail which gets extended by each module, and drawn from for power - only the gateway needs to be powered
* Reasonably rigorous effort towards regulating power with capacitors, diodes, and ferrite beads
* Carefully chosen and arranged pins for being able to connect straight out to the neighbor via headers for very short clock transmission lines
* Use an additional "master clock" clock signal which may help keep the samples more reliably transmitted
* Replace WM8960 with Pmod I2S2 for hopefully more consistent and better documented ADC/DAC behavior

v1 - the current version
* Each module only sends signals to its upstream and downstream neighbors
* Each interface dedicated to a particular neighbor
* Still no power sharing, each chip needs to be individually powered via USB-C
* Due to poorly regulated power and poorly designed and organized transmisson lines, there's a fair amount of intrusive noise that happens

v0 - brief initial attempt
* Have one master clock from the gateway forwarded from module to module and driving all interfaces
* Have each interface dedicated to a stream direction to try to sidestep some clock domain issues, which meant both interfaces were connecting to each neighbor
* No power sharing, each chip needs to be individually powered via USB-C
* I quickly discovered this was begging for electrical problems, high frequency clock signals and long transmission lines with lots of stubs drawing energy from it along the line made the signal untenable after a module or two.
