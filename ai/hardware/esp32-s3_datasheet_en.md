ESP32-S3 Series Datasheet Version 2.0

Xtensa® 32-bit LX7 dual-core microprocessor 2.4 GHz Wi-Fi (IEEE 802.11b/g/n) and Bluetooth® 5 (LE) Optional 1.8 V or 3.3 V flash and PSRAM in the chip’s package

45 GPIOs

QFN56 (7×7 mm) Package

Including:

ESP32-S3

ESP32-S3FN8

ESP32-S3R2

ESP32-S3R8

ESP32-S3R8V – End of Life (EOL)

ESP32-S3R16V

ESP32-S3FH4R2

www.espressif.com

Product Overview

ESP32-S3 is a low-power MCU-based system on a chip (SoC) with integrated 2.4 GHz Wi-Fi and Bluetooth® Low Energy (Bluetooth LE). It consists of high-performance dual-core microprocessor (Xtensa® 32-bit LX7), a ULP coprocessor, a Wi-Fi baseband, a Bluetooth LE baseband, RF module, and numerous peripherals.

The functional block diagram of the SoC is shown below.

ESP32-S3 Functional Block Diagram

For more information on power consumption, see Section 4.1.3.5 Power Management Unit (PMU).

Espressif Systems

2 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Espressif ESP32-S3 Wi-Fi + Bluetooth® Low Energy SoCPower consumptionNormalLow power consumption components capable of working in Deep-sleep modeWireless Digital CircuitsWi-Fi MAC Wi-Fi BasebandBluetooth LE Link ControllerBluetooth LE BasebandSecurityFlash EncryptionRSARNGDigital SignatureSHAAESHMACSecure BootRTCRTC MemoryPMUULP CoprocessorPeripheralsUSB Serial/JTAGGPIOUARTTWAI®General-purpose TimersI2SI2CPulse CounterLED PWMCamera InterfaceSPI0/1RMTSPI2/3DIG ADCSystem TimerRTC GPIOTemperature SensorRTC Watchdog TimerGDMALCD InterfaceRTC ADCSD/MMC HostMCPWMUSB OTGeFuse ControllerTouchSensorRTC I2CRF2.4 GHz Balun + Switch2.4 GHz Receiver2.4 GHz TransmitterRF SynthesizerFast RC OscillatorExternal Main ClockPhase Lock LoopSuper WatchdogCPU and MemoryXtensa® Dual-core 32-bit LX7 MicroprocessorJTAGCacheROMSRAMInterruptMatrixPermission ControlWorld ControllerMain System Watchdog TimersFeatures

Wi-Fi

• IEEE 802.11b/g/n-compliant

• Supports 20 MHz and 40 MHz bandwidth in 2.4 GHz band

• 1T1R mode with data rate up to 150 Mbps

• Wi-Fi Multimedia (WMM)

• TX/RX A-MPDU, TX/RX A-MSDU

• Immediate Block ACK

• Fragmentation and defragmentation

• Automatic Beacon monitoring (hardware TSF)

• Four virtual Wi-Fi interfaces

• Simultaneous support for Infrastructure BSS in Station, SoftAP, or Station + SoftAP modes

Note that when ESP32-S3 scans in Station mode, the SoftAP channel will change along with the Station

channel

• Antenna diversity

• 802.11mc FTM

Bluetooth

• Bluetooth LE: Bluetooth 5, Bluetooth mesh

• High power mode (20 dBm)

• Speed: 125 Kbps, 500 Kbps, 1 Mbps, 2 Mbps

• Advertising extensions

• Multiple advertisement sets

• Channel selection algorithm #2

• Internal co-existence mechanism between Wi-Fi and Bluetooth to share the same antenna

CPU and Memory

• Xtensa® dual-core 32-bit LX7 microprocessor

• Clock speed: up to 240 MHz

• CoreMark® score:

– Two cores at 240 MHz: 1329.92 CoreMark; 5.54 CoreMark/MHz

• Five-stage pipeline

• 128-bit data bus and dedicated SIMD instructions

• Single precision floating point unit (FPU)

Espressif Systems

3 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

• L1 cache

• ROM: 384 KB

• SRAM: 512 KB

• SRAM in RTC: 16 KB

• Supported SPI protocols: SPI, Dual SPI, Quad SPI, Octal SPI, QPI and OPI interfaces that allow

connection to flash, external RAM, and other SPI devices

• Flash controller with cache is supported

• Flash in-Circuit Programming (ICP) is supported

Advanced Peripheral Interfaces

• 45 programmable GPIOs

– 4 strapping GPIOs

– 6 or 7 GPIOs needed for in-package flash or PSRAM

ESP32-S3FN8、ESP32-S3R2、ESP32-S3R8、ESP32-S3R8V、ESP32-S3R16V: 6 GPIOs needed

ESP32-S3FH4R2: 7 GPIOs needed

• Digital interfaces:

– Two SPI ports for communication with flash and RAM

– Two general-purpose SPI ports

– LCD interface (8-bit ~ 16-bit parallel RGB, I8080 and MOTO6800), supporting conversion between

RGB565, YUV422, YUV420 and YUV411

– DVP 8-bit ~ 16-bit camera interface

– Three UARTs

– Two I2Cs

– Two I2Ss

– RMT (TX/RX)

– Pulse counter

– LED PWM controller, up to 8 channels

– Full-speed USB OTG

– USB Serial/JTAG controller

– Two Motor Control PWMs (MCPWM)

– SD/MMC host controller with 2 slots

– General DMA controller (GDMA), with 5 transmit channels and 5 receive channels

– TWAI® controller, compatible with ISO 11898-1 (CAN Specification 2.0)

– On-chip debug functionality via JTAG

Espressif Systems

4 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

• Analog interfaces:

– Two 12-bit SAR ADCs, up to 20 channels

– Temperature sensor

– 14 touch sensing IOs

• Timers:

– Four 54-bit general-purpose timers

– 52-bit system timer

– Three watchdog timers

Low Power Management

• Fine-resolution power control through a selection of clock frequency, duty cycle, Wi-Fi operating modes,

and individual power control of internal components

• Four power modes designed for typical scenarios: Active, Modem-sleep, Light-sleep, Deep-sleep

• Power consumption in Deep-sleep mode is as low as 7 µA

• Ultra-Low-Power (ULP) coprocessors:

– ULP-RISC-V coprocessor

– ULP-FSM coprocessor

• RTC memory remains powered on in Deep-sleep mode

Security

• Secure boot

• Flash encryption

• 4-Kbit OTP, up to 1792 bits for users

• Cryptographic hardware acceleration:

– AES-128/256 (FIPS PUB 197)

– SHA (FIPS PUB 180-4)

– RSA

– Random Number Generator (RNG)

– HMAC

– Digital signature

Applications

With low power consumption, ESP32-S3 is an ideal choice for IoT devices in the following areas:

Espressif Systems

5 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

• Smart Home

• Industrial Automation

• Health Care

• Consumer Electronics

• Smart Agriculture

• POS machines

• Service robot

• Audio Devices

• Generic Low-power IoT Sensor Hubs

• Generic Low-power IoT Data Loggers

• Cameras for Video Streaming

• USB Devices

• Speech Recognition

• Image Recognition

• Wi-Fi + Bluetooth Networking Card

• Touch and Proximity Sensing

Espressif Systems

6 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Contents

Note:

Check the link or the QR code to make sure that you use the latest version of this document:

https://www.espressif.com/documentation/esp32-s3_datasheet_en.pdf

Contents

Product Overview Features

Applications

1 1.1

1.2

ESP32-S3 Series Comparison Nomenclature

Comparison

2 Pins 2.1

Pin Layout

2.2

2.3

Pin Overview

IO Pins

2.3.1

IO MUX Functions

2.3.2

RTC Functions

2.3.3

Analog Functions

2.3.4

Restrictions for GPIOs and RTC_GPIOs

2.4

2.5

Analog Pins

Power Supply

2.5.1

Power Pins

2.5.2 Power Scheme

2.5.3 Chip Power-up and Reset

2.6

Pin Mapping Between Chip and Flash/PSRAM

3 Boot Configurations 3.1

Chip Boot Mode Control

3.2

3.3

3.4

4 4.1

VDD_SPI Voltage Control

ROM Messages Printing Control

JTAG Signal Source Control

Functional Description System

4.1.1

Microprocessor and Master

4.1.1.1

4.1.1.2

4.1.1.3

4.1.1.4

CPU

Processor Instruction Extensions (PIE)

Ultra-Low-Power Coprocessor (ULP)

GDMA Controller (GDMA)

2

3

5

13

13

13

14

14

15

19

19

21

22

24

25

26

26

26

27

28

29

30

31

31

31

33

33

33

33

33

34

34

Espressif Systems

7 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Contents

4.1.2 Memory Organization

4.1.2.1

4.1.2.2

4.1.2.3

4.1.2.4

Internal Memory

External Flash and RAM

Cache

eFuse Controller

4.1.3

System Components

4.1.3.1

4.1.3.2

4.1.3.3

4.1.3.4

4.1.3.5

4.1.3.6

4.1.3.7

4.1.3.8

4.1.3.9

IO MUX and GPIO Matrix

Reset

Clock

Interrupt Matrix

Power Management Unit (PMU)

System Timer

General Purpose Timers

Watchdog Timers

XTAL32K Watchdog Timers

4.1.3.10

Permission Control

4.1.3.11

World Controller

4.1.3.12

System Registers

4.1.4

Cryptography and Security Component

4.1.4.1

4.1.4.2

4.1.4.3

4.1.4.4

4.1.4.5

4.1.4.6

4.1.4.7

4.1.4.8

4.1.4.9

SHA Accelerator

AES Accelerator

RSA Accelerator

Secure Boot

HMAC Accelerator

Digital Signature

External Memory Encryption and Decryption

Clock Glitch Detection

Random Number Generator

4.2

Peripherals

4.2.1

Connectivity Interface

4.2.1.1

4.2.1.2

4.2.1.3

4.2.1.4

4.2.1.5

4.2.1.6

4.2.1.7

4.2.1.8

4.2.1.9

UART Controller

I2C Interface

I2S Interface

LCD and Camera Controller

Serial Peripheral Interface (SPI) Two-Wire Automotive Interface (TWAI®) USB 2.0 OTG Full-Speed Interface

USB Serial/JTAG Controller

SD/MMC Host Controller

4.2.1.10

LED PWM Controller

4.2.1.11 Motor Control PWM (MCPWM)

4.2.1.12

Remote Control Peripheral (RMT)

4.2.1.13

Pulse Count Controller (PCNT)

4.2.2

Analog Signal Processing

4.2.2.1

SAR ADC

4.2.2.2

Temperature Sensor

35

36

36

37

37

37

37

38

39

39

40

42

42

42

43

43

44

44

45

45

45

46

46

46

47

47

47

47

48

48

48

49

49

50

50

53

53

55

55

56

56

57

57

58

58

59

Espressif Systems

8 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Contents

4.2.2.3

Touch Sensor

4.3 Wireless Communication

4.3.1

Radio

4.3.1.1

4.3.1.2

4.3.1.3

4.3.2 Wi-Fi

2.4 GHz Receiver

2.4 GHz Transmitter

Clock Generator

4.3.2.1

Wi-Fi Radio and Baseband

4.3.2.2 Wi-Fi MAC

4.3.2.3

Networking Features

4.3.3

Bluetooth LE

4.3.3.1

Bluetooth LE PHY

4.3.3.2

Bluetooth LE Link Controller

Electrical Characteristics Absolute Maximum Ratings

Recommended Power Supply Characteristics

VDD_SPI Output Characteristics

DC Characteristics (3.3 V, 25 °C)

ADC Characteristics

Current Consumption

5.6.1

RF Current Consumption in Active Mode

5.6.2 Current Consumption in Other Modes

5 5.1

5.2

5.3

5.4

5.5

5.6

5.7

Reliability

6 RF Characteristics 6.1

Wi-Fi Radio

6.1.1

Wi-Fi RF Transmitter (TX) Specifications

6.1.2 Wi-Fi RF Receiver (RX) Specifications

6.2

Bluetooth LE Radio

6.2.1

Bluetooth LE RF Transmitter (TX) Specifications

6.2.2 Bluetooth LE RF Receiver (RX) Specifications

7 Packaging

Appendix A – ESP32-S3 Consolidated Pin Overview

Related Documentation and Resources

Revision History

59

60

60

60

60

60

60

61

61

61

61

62

62

63

63

63

64

64

65

65

65

65

68

69

69

69

70

71

71

73

76

78

79

80

Espressif Systems

9 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

List of Tables

List of Tables

1-1

ESP32-S3 Series Comparison

2-1 Pin Overview

2-2 Power-Up Glitches on Pins

2-3 Peripheral Signals Routed via IO MUX

2-4 IO MUX Pin Functions

2-5 RTC Peripheral Signals Routed via RTC IO MUX

2-6 RTC Functions

2-7 Analog Signals Routed to Analog Functions

2-8 Analog Functions

2-9 Analog Pins

2-10 Power Pins

2-11 Voltage Regulators

2-12 Description of Timing Parameters for Power-up and Reset

2-13 Pin Mapping Between Chip and In-package Flash/ PSRAM

3-1 Default Configuration of Strapping Pins

3-2 Description of Timing Parameters for the Strapping Pins

3-3 Chip Boot Mode Control

3-4 VDD_SPI Voltage Control

3-5 JTAG Signal Source Control

4-1 Components and Power Domains

5-1 Absolute Maximum Ratings

5-2 Recommended Power Characteristics

5-3 VDD_SPI Internal and Output Characteristics

5-4 DC Characteristics (3.3 V, 25 °C)

5-5 ADC Characteristics

5-6 ADC Calibration Results

5-7 Wi-Fi Current Consumption Depending on RF Modes

5-8 Current Consumption in Modem-sleep Mode

5-9 Current Consumption in Low-Power Modes

5-10 Reliability Qualifications

6-1 Wi-Fi RF Characteristics

6-2 TX Power with Spectral Mask and EVM Meeting 802.11 Standards 6-3 TX EVM Test1 6-4 RX Sensitivity

6-5 Maximum RX Level

6-6 RX Adjacent Channel Rejection

6-7 Bluetooth LE Frequency

6-8 Transmitter Characteristics - Bluetooth LE 1 Mbps

6-9 Transmitter Characteristics - Bluetooth LE 2 Mbps

6-10 Transmitter Characteristics - Bluetooth LE 125 Kbps

6-11 Transmitter Characteristics - Bluetooth LE 500 Kbps

6-12 Receiver Characteristics - Bluetooth LE 1 Mbps

6-13 Receiver Characteristics - Bluetooth LE 2 Mbps

13

15

17

19

20

22

22

23

23

25

26

26

27

28

29

30

30

31

32

41

63

63

64

64

65

65

65

66

66

68

69

69

69

70

71

71

71

71

72

72

73

73

74

Espressif Systems

10 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

List of Tables

6-14 Receiver Characteristics - Bluetooth LE 125 Kbps

6-15 Receiver Characteristics - Bluetooth LE 500 Kbps

74

75

Espressif Systems

11 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

List of Figures

List of Figures

1-1

ESP32-S3 Series Nomenclature

2-1 ESP32-S3 Pin Layout (Top View)

2-2 ESP32-S3 Power Scheme

2-3 Visualization of Timing Parameters for Power-up and Reset

3-1 Visualization of Timing Parameters for the Strapping Pins

4-1 Address Mapping Structure

4-2 Components and Power Domains

7-1 QFN56 (7×7 mm) Package

7-2 QFN56 (7×7 mm) Package (Only for ESP32-S3FH4R2)

13

14

27

27

30

35

41

76

77

Espressif Systems

12 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

1 ESP32-S3 Series Comparison

1 ESP32-S3 Series Comparison

1.1 Nomenclature

1.2 Comparison

Figure 1-1. ESP32-S3 Series Nomenclature

Table 1-1. ESP32-S3 Series Comparison

Ordering Code1

In-Package Flash2

In-Package PSRAM

ESP32-S3

ESP32-S3FN8

ESP32-S3R2

ESP32-S3R8

ESP32-S3R8V (End of Life)

ESP32-S3R16V

ESP32-S3FH4R2

— 8 MB (Quad SPI)5 —

—

—

—

—

—

2 MB (Quad SPI)

8 MB (Octal SPI)

8 MB (Octal SPI)

16 MB (Octal SPI)

4 MB (Quad SPI)

2 MB (Quad SPI)

Ambient Temp.3 (°C) –40 ∼ 105 –40 ∼ 85 –40 ∼ 85 –40 ∼ 65 –40 ∼ 65 –40 ∼ 65 –40 ∼ 85

VDD_SPI Voltage4 3.3 V/1.8 V

3.3 V

3.3 V

3.3 V

1.8 V

1.8 V

3.3 V

1 For details on chip marking and packing, see Section 7 Packaging. 2 By default, the SPI flash on the chip operates at a maximum clock frequency of 80 MHz and does not support the auto suspend feature. If you have a requirement for a higher flash clock frequency of 120

MHz or if you need the flash auto suspend feature, please contact us.

3 Ambient temperature specifies the recommended temperature range of the environment immediately outside an Espressif chip. For chips with Octal SPI PSRAM (ESP32-S3R8, ESP32-S3R8V, and ESP32-

S3R16V), if the PSRAM ECC function is enabled, the maximum ambient temperature can be improved to

85 °C, while the usable size of PSRAM will be reduced by 1/16. 4 For more information on VDD_SPI, see Section 2.5 Power Supply. 5 For details about SPI modes, see Section 2.6 Pin Mapping Between Chip and Flash/PSRAM.

Espressif Systems

13 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

ESP32-S3FxChip seriesFlashFlash size (MB)RxPSRAMPSRAM size (MB)H(cid:18)(cid:49)Flash temperatureH: High temperatureN: Normal temperatureV1.8 V external SPI ﬂash only2 Pins

2 Pins

2.1 Pin Layout

Figure 2-1. ESP32-S3 Pin Layout (Top View)

Espressif Systems

14 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

1234567892930313233343536373839404142151617181920212223242526284546474849505152535455564443ESP32-S31314101112GPIO2027GPIO21GPIO19GPIO18GPIO17XTAL_32K_NXTAL_32K_PVDD3P3_RTCGPIO14GPIO13GPIO12GPIO11GPIO10GPIO9GPIO8GPIO7GPIO6GPIO5GPIO4GPIO3GPIO2GPIO1GPIO0CHIP_PUVDD3P3VDD3P3LNA_INVDDAXTAL_PXTAL_NGPIO46GPIO45U0RXDU0TXDMTMSMTDIVDD3P3_CPUMTDOMTCKGPIO38VDDAGPIO37GPIO36GPIO35GPIO34GPIO33SPICLK_PSPIDSPIQSPICLKSPICS0SPIWPSPIHDVDD_SPI57 GNDSPICS1SPICLK_N2 Pins

2.2 Pin Overview

The ESP32-S3 chip integrates multiple peripherals that require communication with the outside world. To keep

the chip package size reasonably small, the number of available pins has to be limited. So the only way to

route all the incoming and outgoing signals is through pin multiplexing. Pin muxing is controlled via software

programmable registers (see ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO

Matrix).

All in all, the ESP32-S3 chip has the following types of pins:

• IO pins with the following predefined sets of functions to choose from:

– Each IO pin has predefined IO MUX functions – see Table 2-4 IO MUX Pin Functions

– Some IO pins have predefined RTC functions – see Table 2-6 RTC Functions

– Some IO pins have predefined analog functions – see Table 2-8 Analog Functions

Predefined functions means that each IO pin has a set of direct connections to certain on-chip

components. During run-time, the user can configure which component from a predefined set to

connect to a certain pin at a certain time via memory mapped registers (see

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO pins).

• Analog pins that have exclusively-dedicated analog functions – see Table 2-9 Analog Pins

• Power pins that supply power to the chip components and non-power pins – see Table 2-10 Power Pins

Table 2-1 Pin Overview gives an overview of all the pins. For more information, see the respective sections for

each pin type below, or Appendix A – ESP32-S3 Consolidated Pin Overview.

Pin

Pin

No. Name

LNA_IN

VDD3P3

VDD3P3

Table 2-1. Pin Overview

Pin Providing Power 3-6

Pin Settings 7

At Reset

After Reset

Pin Type 1 Analog

Power

Power

Pin Function Sets 1,2 RTC

IO MUX

Analog

CHIP_PU

Analog

VDD3P3_RTC

GPIO0

GPIO1

GPIO2

GPIO3

GPIO4

GPIO5

GPIO6

GPIO7

GPIO8

GPIO9

GPIO10

GPIO11

GPIO12

GPIO13

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

IE, WPU

IE, WPU

IE

IE

IE

IE

IE

IE

IE

IE

IE

IE

IE

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

RTC

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Analog

Cont’d on next page

1

2

3

4

5

6

7

8

9

10

11

12

13

14

15

16

17

18

Espressif Systems

15 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

Pin

Pin

No. Name

GPIO14

19

20

21

22

23

24

25

26

27

28

29

30

31

32

33

34

35

36

37

38

39

40

41

42

43

SPIHD

SPIWP

SPICS0

SPICLK

SPIQ

SPID

SPICLK_N

SPICLK_P

GPIO33

GPIO34

GPIO35

GPIO36

GPIO37

GPIO38

Table 2-1 – cont’d from previous page

Pin Type 1 IO

Pin Providing Power 3-6 VDD3P3_RTC

VDD3P3_RTC

Power

XTAL_32K_P

XTAL_32K_N

GPIO17

GPIO18

GPIO19

GPIO20

GPIO21

SPICS1

IO

IO

IO

IO

IO

IO

IO

IO

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD3P3_RTC

VDD_SPI

VDD_SPI

Power

Pin Settings 7

At Reset

After Reset

IE

IE

IE

USB_PU

USB_PU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE

IE

IE

IE

IE

IE

IE

IE IE 8 IE

IE

IE

IE, WPU

IE, WPU

IE, WPU

IE, WPU

IE, WPD

IE, WPD

IE, WPD

IE, WPD

Pin Function Sets 1,2 RTC

IO MUX

Analog

IO MUX

RTC

Analog

Analog

Analog

Analog

Analog

Analog

Analog

RTC

RTC

RTC

RTC

RTC

RTC

RTC

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IO MUX

IE

IE

VDD_SPI / VDD3P3_CPU

VDD_SPI / VDD3P3_CPU

VDD_SPI / VDD3P3_CPU

VDD_SPI / VDD3P3_CPU

VDD_SPI / VDD3P3_CPU

VDD_SPI / VDD3P3_CPU

VDD_SPI / VDD3P3_CPU

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

IO

VDD_SPI

VDD_SPI

VDD_SPI

VDD_SPI

VDD_SPI

VDD_SPI

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

VDD3P3_CPU

44 MTCK

45 MTDO

46

VDD3P3_CPU

Power

47 MTDI

48 MTMS

49

50

51

52

53

54

55

56

57

U0TXD

U0RXD

GPIO45

GPIO46

XTAL_N

XTAL_P

VDDA

VDDA

GND

IO

IO

IO

IO

IO

IO

Analog

Analog

Power

Power

Power

For more information, see respective sections below. Alternatively, see Appendix A – ESP32-S3 Consolidated Pin Overview.

Bold marks the pin function set in which a pin has its default function in the default boot mode. For more information about the

boot mode，see Section 3.1 Chip Boot Mode Control.

In column Pin Providing Power, regarding pins powered by VDD_SPI:

• Power actually comes from the internal power rail supplying power to VDD_SPI. For details, see Section 2.5.2 Power

Scheme.

In column Pin Providing Power, regarding pins powered by VDD3P3_CPU / VDD_SPI:

Espressif Systems

16 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

• Pin Providing Power (either VDD3P3_CPU or VDD_SPI) is decided by eFuse bit EFUSE_PIN_POWER_SELECTION (see

ESP32-S3 Technical Reference Manual > Chapter eFuse Controller) and can be configured via the

IO_MUX_PAD_POWER_CTRL bit (see ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO pins).

For ESP32-S3R8V chip, as the VDD_SPI voltage has been set to 1.8 V, the working voltage for pins SPICLK_N and SPICLK_P
(GPIO47 and GPIO48) would also be 1.8 V, which is different from other GPIOs.

The default drive strengths for each pin are as follows:
• GPIO17 and GPIO18: 10 mA • GPIO19 and GPIO20: 40 mA • All other pins: 20 mA

Column Pin Settings shows predefined settings at reset and after reset with the following abbreviations:
• IE – input enabled • WPU – internal weak pull-up resistor enabled • WPD – internal weak pull-down resistor enabled • USB_PU – USB pull-up resistor enabled

– By default, the USB function is enabled for USB pins (i.e., GPIO19 and GPIO20), and the pin pull-up is decided by the

USB pull-up. The USB pull-up is controlled by USB_SERIAL_JTAG_DP/DM_PULLUP and the pull-up resistor value is

controlled by USB_SERIAL_JTAG_PULLUP_VALUE. For details, see ESP32-S3 Technical Reference Manual > Chapter

USB Serial/JTAG Controller).

– When the USB function is disabled, USB pins are used as regular GPIOs and the pin’s internal weak pull-up and

pull-down resistors are disabled by default (configurable by IO_MUX_FUN_

WPU/WPD). For details, see ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

Depends on the value of EFUSE_DIS_PAD_JTAG
• 0 - WPU is enabled • 1 - pin floating

Some pins have glitches during power-up. See details in Table 2-2.

Table 2-2. Power-Up Glitches on Pins

Pin

GPIO1

GPIO2

GPIO3

GPIO4

GPIO5

GPIO6

GPIO7

GPIO8

GPIO9

GPIO10

GPIO11

GPIO12

GPIO13

GPIO14

XTAL_32K_P

XTAL_32K_N

GPIO17

Glitch1 Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Low-level glitch

Typical Time Period (µs)

60

60

60

60

60

60

60

60

60

60

60

60

60

60

60

60

60

Espressif Systems

17 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Cont’d on next page

2 Pins

Pin

GPIO18

GPIO19

GPIO20

Table 2-2 – cont’d from previous page

Glitch1 Low-level glitch

High-level glitch

Low-level glitch High-level glitch2 Pull-down glitch High-level glitch2

Typical Time Period (µs)

60

60

60

60

60

60

1 Low-level glitch: the pin is at a low level output status during the time period; High-level glitch: the pin is at a high level output status during the time period;

Pull-down glitch: the pin is at an internal weak pulled-down status during the time period;

Pull-up glitch: the pin is at an internal weak pulled-up status during the time period.

Please refer to Table 5-4 DC Characteristics (3.3 V, 25 °C) for detailed parameters about

low/high-level and pull-down/up.

2 GPIO19 and GPIO20 pins both have two high-level glitches during chip power-up, each lasting for about 60 µs. The total duration for the glitches and the delay are 3.2 ms and

2 ms respectively for GPIO19 and GPIO20.

Espressif Systems

18 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

2.3 IO Pins

2.3.1

IO MUX Functions

The IO MUX allows multiple input/output signals to be connected to a single input/output pin. Each IO pin of

ESP32-S3 can be connected to one of the five signals (IO MUX functions, i.e., F0-F4), as listed in Table 2-4 IO

MUX Pin Functions.

Among the five sets of signals:

• Some are routed via the GPIO Matrix (GPIO0, GPIO1, etc.), which incorporates internal signal routing

circuitry for mapping signals programmatically. It gives the pin access to almost any peripheral signals.

However, the flexibility of programmatic mapping comes at a cost as it might affect the latency of routed

signals. For details about connecting to peripheral signals via GPIO Matrix, see

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

• Some are directly routed from certain peripherals (U0TXD, MTCK, etc.), including UART0/1, JTAG,

SPI0/1, and SPI2 - see Table 2-3 Peripheral Signals Routed via IO MUX.

Table 2-3. Peripheral Signals Routed via IO MUX

Pin Function

Signal

Description

U…TXD

U…RXD

U…RTS

U…CTS

MTCK

MTDO

MTDI

MTMS

SPIQ

SPID

SPIHD

SPIWP

SPICLK

SPICS…

SPIIO…

SPIDQS

Transmit data

Receive data

Request to send

Clear to send

Test clock

Test Data Out

Test Data In

Test Mode Select

Data out

Data in

Hold

Write protect

Clock

Chip select

Data

UART0/1 interface

JTAG interface for debugging

SPI0/1 interface (powered by VDD_SPI) for connection to in-package or

off-package flash/PSRAM via the SPI bus. It supports 1-, 2-, 4-line SPI

modes. See also Section 2.6 Pin Mapping Between Chip and

Flash/PSRAM

SPI0/1 interface (powered by VDD_SPI or VDD3P3_CPU) for the higher

Data strobe/data mask

4 bits data line interface and DQS interface in 8-line SPI mode

SPICLK_N_DIFF

Negative clock signal

Differential clock negative/positive for the SPI bus

SPICLK_P_DIFF

Positive clock signal

SUBSPIQ

SUBSPID

SUBSPIHD

SUBSPIWP

SUBSPICLK

SUBSPICS…

Data out

Data in

Hold

Write protect

Clock

Chip select

SPI0/1 interface (powered by VDD3P3_RTC or VDD3V3_CPU) for

connection to in-package or off-package flash/PSRAM via the SUBSPI

bus. It supports 1-, 2-, 4-line SPI modes

SUBSPICLK_N_DIFF

Negative clock signal

Differential clock negative/positive for the SUBSPI bus

SUBSPICLK_P_DIFF

Positive clock signal

Cont’d on next page

Espressif Systems

19 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

Pin Function

FSPIQ

FSPID

FSPIHD

FSPIWP

FSPICLK

FSPICS0

FSPIIO…

FSPIDQS

Signal

Data out

Data in

Hold

Write protect

Clock

Chip select

Data

Table 2-3 – cont’d from previous page

Description

SPI2 interface for fast SPI connection. It supports 1-, 2-, 4-line SPI

modes

The higher 4 bits data line interface and DQS interface for SPI2 interface

Data strobe/data mask

in 8-line SPI mode

CLK_OUT…

Clock output

Output clock signals generated by the chip’s internal components

Table 2-4 IO MUX Pin Functions shows the IO MUX functions of IO pins.

Pin

No.

5

6

7

8

9

10

11

12

13

14

15

16

17

18

19

21

22

23

24

25

26

27

IO MUX / GPIO Name 2 GPIO0

GPIO1

GPIO2

GPIO3

GPIO4

GPIO5

GPIO6

GPIO7

GPIO8

GPIO9

GPIO10

GPIO11

GPIO12

GPIO13

GPIO14

GPIO15

GPIO16

GPIO17

GPIO18

GPIO19

GPIO20

GPIO21

F0

GPIO0

GPIO1

GPIO2

GPIO3

GPIO4

GPIO5

GPIO6

GPIO7

GPIO8

GPIO9

GPIO10

GPIO11

GPIO12

GPIO13

GPIO14

GPIO15

GPIO16

GPIO17

GPIO18

GPIO19

GPIO20

GPIO21

Table 2-4. IO MUX Pin Functions

IO MUX Function 1, 2, 3

Type 3 F1

Type

F2

Type

F3

Type

F4

Type

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

GPIO0

GPIO1

GPIO2

GPIO3

GPIO4

GPIO5

GPIO6

GPIO7

GPIO8

GPIO9

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

4d

SUBSPICS1 O/T

4c

4g

SUBSPIHD

I1/O/T FSPIHD

I1/O/T

GPIO10 I/O/T FSPIIO4

I1/O/T SUBSPICS0 O/T

FSPICS0 I1/O/T

GPIO11

I/O/T FSPIIO5

I1/O/T SUBSPID

I1/O/T FSPID

I1/O/T

GPIO12

I/O/T FSPIIO6

I1/O/T SUBSPICLK O/T

FSPICLK

I1/O/T

GPIO13

I/O/T FSPIIO7

I1/O/T SUBSPIQ

I1/O/T FSPIQ

GPIO14

I/O/T FSPIDQS

O/T

SUBSPIWP

I1/O/T FSPIWP

I1/O/T

I1/O/T

GPIO15

I/O/T U0RTS

GPIO16

I/O/T U0CTS

GPIO17

I/O/T U1TXD

GPIO18

I/O/T U1RXD

GPIO19

I/O/T U1RTS

GPIO20 I/O/T U1CTS

GPIO21

I/O/T

O

I1

O

I1

O

I1

CLK_OUT3

CLK_OUT2

CLK_OUT1

O

O

O

Cont’d on next page

Espressif Systems

20 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

Pin

No.

28

30

31

32

33

34

35

38

39

40

41

42

43

44

45

47

48

49

50

51

52

37

IO MUX / GPIO Name 2 GPIO26

GPIO27

GPIO28

GPIO29

GPIO30

GPIO31

GPIO32

GPIO33

GPIO34

GPIO35

GPIO36

GPIO37

GPIO38

GPIO39

GPIO40

GPIO41

GPIO42

GPIO43

GPIO44

GPIO45

GPIO46

GPIO47

36

GPIO48

Table 2-4 – cont’d from previous page

IO MUX Function 1, 2, 3

Type 3 F1

4a

Type

F2

Type

F3

Type

F4

Type

O/T

GPIO26 I/O/T

I1/O/T GPIO27

I/O/T

I1/O/T GPIO28 I/O/T

O/T

O/T

GPIO29 I/O/T

GPIO30 I/O/T

I1/O/T GPIO31

I/O/T

I1/O/T GPIO32 I/O/T

4f

4e

4b

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I/O/T

I1

O/T

I1

I1

O

I1

GPIO33 I/O/T FSPIHD

I1/O/T SUBSPIHD

I1/O/T SPIIO4

GPIO34 I/O/T FSPICS0

I1/O/T SUBSPICS0 O/T

SPIIO5

GPIO35 I/O/T FSPID

I1/O/T SUBSPID

I1/O/T SPIIO6

GPIO36 I/O/T FSPICLK

I1/O/T SUBSPICLK O/T

SPIIO7

I1/O/T

I1/O/T

I1/O/T

I1/O/T

GPIO37

I/O/T FSPIQ

I1/O/T SUBSPIQ

I1/O/T SPIDQS

I0/O/T

GPIO38 I/O/T FSPIWP

I1/O/T SUBSPIWP

I1/O/T

GPIO39 I/O/T CLK_OUT3 O

SUBSPICS1

O/T

GPIO40 I/O/T CLK_OUT2 O

GPIO41

I/O/T CLK_OUT1

GPIO42 I/O/T

GPIO43 I/O/T CLK_OUT1

O

O

GPIO44

I/O/T CLK_OUT2 O

I/O/T

I/O/T

GPIO45 I/O/T

GPIO46 I/O/T

O/T

GPIO47

I/O/T

O/T

GPIO48 I/O/T

SUBSPI

CLK_P_DIFF

SUBSPI

CLK_N_DIFF

O/T

O/T

F0

SPICS1

SPIHD

SPIWP

SPICS0

SPICLK

SPIQ

SPID

GPIO33

GPIO34

GPIO35

GPIO36

GPIO37

GPIO38

MTCK

MTDO

MTDI

MTMS

U0TXD

U0RXD

GPIO45

GPIO46

SPI

CLK_P_DIFF

SPI

CLK_N_DIFF

1 Bold marks the default pin functions in the default boot mode. For more information about the boot mode，see Section 3.1

Chip Boot Mode Control.

2 Regarding highlighted cells, see Section 2.3.4 Restrictions for GPIOs and RTC_GPIOs. 3 Each IO MUX function (Fn, n = 0 ~ 4) is associated with a type. The description of type is as follows:

• I – input. O – output. T – high impedance. • I1 – input; if the pin is assigned a function other than Fn, the input signal of Fn is always 1. • I0 – input; if the pin is assigned a function other than Fn, the input signal of Fn is always 0.

4a-4g For detailed pin assignment of SPI, please refer to 4.2.1.5 Serial Peripheral Interface (SPI) > Pin Assignment.

2.3.2 RTC Functions

When the chip is in Deep-sleep mode, the IO MUX described in Section 2.3.1 IO MUX Functions will not work.

That is where the RTC IO MUX comes in. It allows multiple input/output signals to be a single input/output pin

in Deep-sleep mode, as the pin is connected to the RTC system and powered by VDD3P3_RTC.

RTC IO pins can be assigned to RTC functions. They can

• Either work as RTC GPIOs (RTC_GPIO0, RTC_GPIO1, etc.), connected to the ULP coprocessor

• Or connect to RTC peripheral signals (sar_i2c_scl_0, sar_i2c_sda_0, etc.) - see Table 2-5 RTC

Peripheral Signals Routed via RTC IO MUX

Espressif Systems

21 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

Table 2-5. RTC Peripheral Signals Routed via RTC IO MUX

Pin Function

Signal

Description

sar_i2c_scl…

Serial clock

sar_i2c_sda… Serial data

RTC I2C0/1 interface

Table 2-6 RTC Functions shows the RTC functions of RTC IO pins.

Table 2-6. RTC Functions

RTC Function 2 F3

F2

F1

sar_i2c_scl_0

sar_i2c_sda_0

sar_i2c_scl_1

sar_i2c_sda_1

Pin

No.

5

6

7

8

9

10

11

12

13

14

15

16

17

18

19

21

22

23

24

25

26

27

RTC IO Name 1 RTC_GPIO0

RTC_GPIO1

RTC_GPIO2

RTC_GPIO3

RTC_GPIO4

RTC_GPIO5

RTC_GPIO6

RTC_GPIO7

RTC_GPIO8

RTC_GPIO9

F0

RTC_GPIO0

RTC_GPIO1

RTC_GPIO2

RTC_GPIO3

RTC_GPIO4

RTC_GPIO5

RTC_GPIO6

RTC_GPIO7

RTC_GPIO8

RTC_GPIO9

RTC_GPIO10

RTC_GPIO10

RTC_GPIO11

RTC_GPIO11

RTC_GPIO12

RTC_GPIO12

RTC_GPIO13

RTC_GPIO13

RTC_GPIO14

RTC_GPIO14

RTC_GPIO15

RTC_GPIO15

RTC_GPIO16

RTC_GPIO16

RTC_GPIO17

RTC_GPIO17

RTC_GPIO18

RTC_GPIO18

RTC_GPIO19

RTC_GPIO19

RTC_GPIO20

RTC_GPIO20

RTC_GPIO21

RTC_GPIO21

1 This column lists the RTC GPIO names, since RTC functions are con-

figured with RTC GPIO registers that use RTC GPIO numbering.

2 Regarding highlighted cells, see Section 2.3.4 Restrictions for GPIOs

and RTC_GPIOs.

2.3.3 Analog Functions

Some IO pins also have analog functions, for analog peripherals (such as ADC) in any power mode. Internal

analog signals are routed to these analog functions, see Table 2-7 Analog Signals Routed to Analog

Functions.

Espressif Systems

22 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

Table 2-7. Analog Signals Routed to Analog Functions

Pin Function

Signal

Description

TOUCH…

Touch sensor channel … signal

Touch sensor interface

ADC…_CH…

ADC1/2 channel … signal

ADC1/2 interface

XTAL_32K_N Negative clock signal

32 kHz external clock input/output

XTAL_32K_P

Positive clock signal

connected to ESP32-S3’s oscillator

USB_D-

USB_D+

Data -

Data +

USB OTG and USB Serial/JTAG function

Table 2-8 Analog Functions shows the analog functions of IO pins.

Table 2-8. Analog Functions

Analog IO Name 1, 2 RTC_GPIO0

RTC_GPIO1

RTC_GPIO2

RTC_GPIO3

RTC_GPIO4

RTC_GPIO5

RTC_GPIO6

RTC_GPIO7

RTC_GPIO8

RTC_GPIO9

Analog Function 3

F0

F1

TOUCH1

TOUCH2

TOUCH3

TOUCH4

TOUCH5

TOUCH6

TOUCH7

TOUCH8

TOUCH9

ADC1_CH0

ADC1_CH1

ADC1_CH2

ADC1_CH3

ADC1_CH4

ADC1_CH5

ADC1_CH6

ADC1_CH7

ADC1_CH8

RTC_GPIO10

TOUCH10

ADC1_CH9

RTC_GPIO11

RTC_GPIO12

RTC_GPIO13

RTC_GPIO14

TOUCH11

TOUCH12

TOUCH13

TOUCH14

ADC2_CH0

ADC2_CH1

ADC2_CH2

ADC2_CH3

RTC_GPIO15

XTAL_32K_P

ADC2_CH4

RTC_GPIO16

XTAL_32K_N

ADC2_CH5

RTC_GPIO17

RTC_GPIO18

RTC_GPIO19

RTC_GPIO20

RTC_GPIO21

ADC2_CH6

ADC2_CH7

ADC2_CH8

ADC2_CH9

USB_D-

USB_D+

Pin

No.

5

6

7

8

9

10

11

12

13

14

15

16

17

18

19

21

22

23

24

25

26

27

1 Bold marks the default pin functions in the default boot mode. For more information about the boot mode，see

Section 3.1 Chip Boot Mode Control.

2 This column lists the RTC GPIO names, since analog functions are configured with RTC GPIO registers that

use RTC GPIO numbering.

3 Regarding highlighted cells, see Section 2.3.4 Re-

strictions for GPIOs and RTC_GPIOs.

Espressif Systems

23 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

2.3.4 Restrictions for GPIOs and RTC_GPIOs

All IO pins of ESP32-S3 have GPIO and some have RTC_GPIO pin functions. However, the IO pins are

multiplexed and can be configured for different purposes based on the requirements. Some IOs have

restrictions for usage. It is essential to consider the multiplexed nature and the limitations when using these IO

pins.

In tables of this chapter, some pin functions are highlighted . The non-highlighted GPIO or RTC_GPIO pins are

recommended for use first. If more pins are needed, the highlighted GPIOs or RTC_GPIOs should be chosen

carefully to avoid conflicts with important pin functions.

The highlighted IO pins have the following important pin functions:

• GPIO – allocated for communication with in-package flash/PSRAM and NOT recommended for other

uses. For details, see Section 2.6 Pin Mapping Between Chip and Flash/PSRAM.

• GPIO – no restrictions, unless the chip is connected to flash/PSRAM using 8-line SPI mode. For details,

see Section 2.6 Pin Mapping Between Chip and Flash/PSRAM.

• GPIO – have one of the following important functions:

– Strapping pins – need to be at certain logic levels at startup. See Section 3 Boot Configurations.

– USB_D+/- – by default, connected to the USB Serial/JTAG Controller. To function as GPIOs, these

pins need to be reconfigured via the IO_MUX_MCU_SEL bit (see

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix for details).

– JTAG interface – often used for debugging. See Table 2-4 IO MUX Pin Functions. To free these

pins up, the pin functions USB_D+/- of the USB Serial/JTAG Controller can be used instead. See

also Section 3.4 JTAG Signal Source Control.

– UART interface – often used for debugging. See Table 2-4 IO MUX Pin Functions.

See also Appendix A – ESP32-S3 Consolidated Pin Overview.

Espressif Systems

24 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

2.4 Analog Pins

Table 2-9. Analog Pins

Pin

Pin

No. Name

Pin

Pin

Type

Function

LNA_IN

I/O

Low Noise Amplifier (RF LNA) input/output signals

1

4

CHIP_PU

53

54

XTAL_N

XTAL_P

High: on, enables the chip (powered up).

Low: off, disables the chip (powered down).

Note: Do not leave the CHIP_PU pin floating.

External clock input/output connected to chip’s crystal or oscillator.

P/N means differential clock positive/negative.

I

—

—

Espressif Systems

25 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

2.5 Power Supply

2.5.1 Power Pins

The chip is powered via the power pins described in Table 2-10 Power Pins.

Pin

Pin

No. Name

2

3

VDD3P3

VDD3P3

20

VDD3P3_RTC

VDD_SPI 3,4

Table 2-10. Power Pins

Direction

Power Domain / Other

IO Pins 5

Power Supply 1,2

Input

Input

Input

Input

Analog power domain

Analog power domain

RTC and part of Digital power domains

RTC IO

In-package memory (backup power line)

Output

In-package and off-package flash/PSRAM SPI IO

VDD3P3_CPU

Input

Digital power domain

Digital IO

VDDA

VDDA

GND

Input

Input

–

Analog power domain

Analog power domain

External ground connection

29

46

55

56

57

1 See in conjunction with Section 2.5.2 Power Scheme. 2 For recommended and maximum voltage and current, see Section 5.1 Absolute Maximum

Ratings and Section 5.2 Recommended Power Supply Characteristics.

3 To configure VDD_SPI as input or output, see ESP32-S3 Technical Reference Manual >

Chapter Low-power Management.

4 To configure output voltage, see Section 3.2 VDD_SPI Voltage Control and Section 5.3

VDD_SPI Output Characteristics.

5 RTC IO pins are those powered by VDD3P3_RTC and so on, as shown in Figure 2-2 ESP32-

S3 Power Scheme. See also Table 2-1 Pin Overview > Column Pin Providing Power.

2.5.2 Power Scheme

The power scheme is shown in Figure 2-2 ESP32-S3 Power Scheme.

The components on the chip are powered via voltage regulators.

Table 2-11. Voltage Regulators

Voltage Regulator Output

Power Supply

Digital

Low-power

1.1 V

1.1 V

Digital power domain

RTC power domain

Flash

1.8 V

in-package flash/PSRAM or

Can be configured to power

off-package memory

Espressif Systems

26 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

2 Pins

Figure 2-2. ESP32-S3 Power Scheme

2.5.3 Chip Power-up and Reset

Once the power is supplied to the chip, its power rails need a short time to stabilize. After that, CHIP_PU – the

pin used for power-up and reset – is pulled high to activate the chip. For information on CHIP_PU as well as

power-up and reset timing, see Figure 2-3 and Table 2-12.

tST BL

tRST

2.8 V

VDDA, VDD3P3, VDD3P3_RTC, VDD3P3_CPU

CHIP_PU

Figure 2-3. Visualization of Timing Parameters for Power-up and Reset

Table 2-12. Description of Timing Parameters for Power-up and Reset

Parameter Description

Time reserved for

the power

rails of VDDA, VDD3P3,

tST BL

VDD3P3_RTC, and VDD3P3_CPU to stabilize before the CHIP_PU

pin is pulled high to activate the chip

tRST

Time reserved for CHIP_PU to stay below VIL_nRST to reset the chip (see Table 5-4)

Min (µs)

50

50

Espressif Systems

27 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

VIL_nRST2 Pins

2.6 Pin Mapping Between Chip and Flash/PSRAM

Table 2-13 lists the pin mapping between the chip and flash/PSRAM for all SPI modes.

For chip variants with in-package flash/PSRAM (see Table 1-1 ESP32-S3 Series Comparison), the pins allocated

for communication with in-package flash/PSRAM can be identified depending on the SPI mode used.

For off-package flash/PSRAM, these are the recommended pin mappings.

For more information on SPI controllers, see also Section 4.2.1.5 Serial Peripheral Interface (SPI).

Notice:

It is not recommended to use the pins connected to flash/PSRAM for any other purposes.

Table 2-13. Pin Mapping Between Chip and In-package Flash/ PSRAM

Pin Name

Single SPI

Dual SPI

Quad SPI / QPI

Octal SPI / OPI

Flash

PSRAM

Flash

PSRAM

Flash

PSRAM

Flash

PSRAM

SPICLK SPICS0 1 SPICS1 2 SPID

SPIQ

SPIWP

SPIHD

GPIO33

GPIO34

GPIO35

GPIO36

GPIO37

CLK

CS#

DI

DO

CLK

CE#

SI/SIO0

SO/SIO1

CLK

CS#

DI

DO

CLK

CE#

SI/SIO0

SO/SIO1

CLK

CS#

DI

DO

CLK

CE#

SI/SIO0

SO/SIO1

WP#

HOLD#

SIO2

SIO3

WP#

HOLD#

SIO2

SIO3

WP#

HOLD#

SIO2

SIO3

CLK

CS#

DQ0

DQ1

DQ2

DQ3

DQ4

DQ5

DQ6

DQ7

CLK

CE#

DQ0

DQ1

DQ2

DQ3

DQ4

DQ5

DQ6

DQ7

DQS/DM DQS/DM

Pin

No.

33

32

28

35

34

31

30

38

39

40

41

42

1 CS0 is for in-package flash 2 CS1 is for in-package PSRAM

Espressif Systems

28 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

3 Boot Configurations

3 Boot Configurations

The chip allows for configuring the following boot parameters through strapping pins and eFuse bits at

power-up or a hardware reset, without microcontroller interaction.

• Chip boot mode

– Strapping pin: GPIO0 and GPIO46

• VDD_SPI voltage

– Strapping pin: GPIO45

– eFuse parameter: EFUSE_VDD_SPI_FORCE and EFUSE_VDD_SPI_TIEH

• ROM message printing

– Strapping pin: GPIO46

– eFuse parameter: EFUSE_UART_PRINT_CONTROL and

EFUSE_DIS_USB_SERIAL_JTAG_ROM_PRINT

• JTAG signal source

– Strapping pin: GPIO3

– eFuse parameter: EFUSE_DIS_PAD_JTAG, EFUSE_DIS_USB_JTAG, and EFUSE_STRAP_JTAG_SEL

The default values of all the above eFuse parameters are 0, which means that they are not burnt. Given that

eFuse is one-time programmable, once programmed to 1, it can never be reverted to 0. For how to program

eFuse parameters, please refer to ESP32-S3 Technical Reference Manual > Chapter eFuse Controller.

The default values of the strapping pins, namely the logic levels, are determined by pins’ internal weak

pull-up/pull-down resistors at reset if the pins are not connected to any circuit, or connected to an external

high-impedance circuit.

Table 3-1. Default Configuration of Strapping Pins

Strapping Pin Default Configuration Bit Value

GPIO0

GPIO3

GPIO45

GPIO46

Weak pull-up

Floating

Weak pull-down

Weak pull-down

1

–

0

0

To change the bit values, the strapping pins should be connected to external pull-down/pull-up resistances. If

the ESP32-S3 is used as a device by a host MCU, the strapping pin voltage levels can also be controlled by

the host MCU.

All strapping pins have latches. At system reset, the latches sample the bit values of their respective strapping

pins and store them until the chip is powered down or shut down. The states of latches cannot be changed in

any other way. It makes the strapping pin values available during the entire chip operation, and the pins are

freed up to be used as regular IO pins after reset.

The timing of signals connected to the strapping pins should adhere to the setup time and hold time

specifications in Table 3-2 and Figure 3-1.

Espressif Systems

29 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

3 Boot Configurations

Table 3-2. Description of Timing Parameters for the Strapping Pins

Parameter Description

tSU

tH

Setup time is the time reserved for the power rails to stabilize be-

fore the CHIP_PU pin is pulled high to activate the chip.

Hold time is the time reserved for the chip to read the strapping

pin values after CHIP_PU is already high and before these pins

start operating as regular IO pins.

Min (ms)

0

3

tSU

tH

CHIP_PU

Figure 3-1. Visualization of Timing Parameters for the Strapping Pins

3.1 Chip Boot Mode Control

GPIO0 and GPIO46 control the boot mode after the reset is released. See Table 3-3 Chip Boot Mode

Control.

Table 3-3. Chip Boot Mode Control

Boot Mode

GPIO0

GPIO46

SPI Boot Joint Download Boot 2

1

0

Any value

0

1 Bold marks the default value and configura-

tion.

2 Joint Download Boot mode supports the fol-

lowing download methods: • USB Download Boot:

– USB-Serial-JTAG Download Boot

– USB-OTG Download Boot

• UART Download Boot

In SPI Boot mode, the ROM bootloader loads and executes the program from SPI flash to boot the

system.

Espressif Systems

30 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Strapping pinVIL_nRSTVIH3 Boot Configurations

In Joint Download Boot mode, users can download binary files into flash using UART0 or USB interface. It is

also possible to download binary files into SRAM and execute it from SRAM.

In addition to SPI Boot and Joint Download Boot modes, ESP32-S3 also supports SPI Download Boot mode.

For details, please see ESP32-S3 Technical Reference Manual > Chapter Chip Boot Control.

3.2 VDD_SPI Voltage Control

The required VDD_SPI voltage for the chips of the ESP32-S3 series can be found in Table 1-1 ESP32-S3 Series

Comparison.

Depending on the value of EFUSE_VDD_SPI_FORCE, the voltage can be controlled in two ways.

Table 3-4. VDD_SPI Voltage Control

VDD_SPI power source 2 VDD3P3_RTC via RSP I Flash Voltage Regulator

Flash Voltage Regulator

VDD3P3_RTC via RSP I

Voltage

EFUSE_VDD_SPI_FORCE GPIO45

EFUSE_VDD_SPI_TIEH

3.3 V

1.8 V

1.8 V

3.3 V

0

1

0

1

Ignored

Ignored

0

1

1 Bold marks the default value and configuration. 2 See Section 2.5.2 Power Scheme.

3.3 ROM Messages Printing Control

During boot process the messages by the ROM code can be printed to:

• (Default) UART0 and USB Serial/JTAG controller

• USB Serial/JTAG controller

• UART0

The ROM messages printing to UART or USB Serial/JTAG controller can be respectively disabled by configuring

registers and eFuse. For detailed information, please refer to ESP32-S3 Technical Reference Manual >

Chapter Chip Boot Control.

3.4 JTAG Signal Source Control

The strapping pin GPIO3 can be used to control the source of JTAG signals during the early boot process. This

pin does not have any internal pull resistors and the strapping value must be controlled by the external circuit

that cannot be in a high impedance state.

As Table 3-5 shows, GPIO3 is used in combination with EFUSE_DIS_PAD_JTAG, EFUSE_DIS_USB_JTAG, and

EFUSE_STRAP_JTAG_SEL.

Espressif Systems

31 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

3 Boot Configurations

Table 3-5. JTAG Signal Source Control

JTAG Signal Source

EFUSE_DIS_PAD_JTAG EFUSE_DIS_USB_JTAG EFUSE_STRAP_JTAG_SEL GPIO3

USB Serial/JTAG Controller

JTAG pins 2

JTAG is disabled

0

0

1

0

0

1

1 Bold marks the default value and configuration. 2 JTAG pins refer to MTDI, MTCK, MTMS, and MTDO.

0

0

0

0

1

1

0

1

Ignored

1

Ignored

Ignored

1

Ignored

Ignored

0

Ignored

Ignored

Espressif Systems

32 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4 Functional Description

4.1 System

This section describes the core of the chip’s operation, covering its microprocessor, memory organization,

system components, and security features.

4.1.1 Microprocessor and Master

This subsection describes the core processing units within the chip and their capabilities.

4.1.1.1 CPU

ESP32-S3 has a low-power Xtensa® dual-core 32-bit LX7 microprocessor.

Feature List

• Five-stage pipeline that supports the clock frequency of up to 240 MHz

• 16-bit/24-bit instruction set providing high code density

• 32-bit customized instruction set and 128-bit data bus that provide high computing performance

• Support for single-precision floating-point unit (FPU)

• 32-bit multiplier and 32-bit divider

• Unbuffered GPIO instructions

• 32 interrupts at six levels

• Windowed ABI with 64 physical general registers

• Trace function with TRAX compressor, up to 16 KB trace memory

• JTAG for debugging

For information about the Xtensa® Instruction Set Architecture, please refer to Xtensa® Instruction Set Architecture (ISA) Summary.

4.1.1.2 Processor Instruction Extensions (PIE)

ESP32-S3 contains a series of new extended instruction set in order to improve the operation efficiency of

specific AI and DSP (Digital Signal Processing) algorithms.

Feature List

• 128-bit new general-purpose registers

• 128-bit vector operations, e.g., complex multiplication, addition, subtraction, multiplication, shifting,

comparison, etc

• Data handling instructions and load/store operation instructions combined

• Non-aligned 128-bit vector data

Espressif Systems

33 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

• Saturation operation

For details, see ESP32-S3 Technical Reference Manual > Chapter Processor Instruction Extensions.

4.1.1.3 Ultra-Low-Power Coprocessor (ULP)

The ULP coprocessor is designed as a simplified, low-power replacement of CPU in sleep modes. It can be

also used to supplement the functions of the CPU in normal working mode. The ULP coprocessor and RTC

memory remain powered up during the Deep-sleep mode. Hence, the developer can store a program for the

ULP coprocessor in the RTC slow memory to access RTC GPIO, RTC peripheral devices, RTC timers and

internal sensors in Deep-sleep mode.

ESP32-S3 has two ULP coprocessors, one based on RISC-V instruction set architecture (ULP-RISC-V) and the

other on finite state machine (ULP-FSM). The clock of the coprocessors is the internal fast RC oscillator.

Feature List

• ULP-RISC-V:

– Support for RV32IMC instruction set

– Thirty-two 32-bit general-purpose registers

– 32-bit multiplier and divider

– Support for interrupts

– Booted by the CPU, its dedicated timer, or RTC GPIO

• ULP-FSM:

– Support for common instructions including arithmetic, jump, and program control instructions

– Support for on-board sensor measurement instructions

– Booted by the CPU, its dedicated timer, or RTC GPIO

Note:

Note that these two coprocessors cannot work simultaneously.

For details, see ESP32-S3 Technical Reference Manual > Chapter ULP Coprocessor.

4.1.1.4 GDMA Controller (GDMA)

ESP32-S3 has a general-purpose DMA controller (GDMA) with five independent channels for transmitting and

another five independent channels for receiving. These ten channels are shared by peripherals that have DMA

feature, and support dynamic priority.

The GDMA controller controls data transfer using linked lists. It allows peripheral-to-memory and

memory-to-memory data transfer at a high speed. All channels can access internal and external RAM.

The ten peripherals on ESP32-S3 with DMA feature are SPI2, SPI3, UHCI0, I2S0, I2S1, LCD/CAM, AES, SHA,

ADC, and RMT.

For details, see ESP32-S3 Technical Reference Manual > Chapter GDMA Controller.

Espressif Systems

34 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.1.2 Memory Organization

This subsection describes the memory arrangement to explain how data is stored, accessed, and managed

for efficient operation.

Figure 4-1 illustrates the address mapping structure of ESP32-S3.

Figure 4-1. Address Mapping Structure

Note:

The memory space with gray background is not available to users.

Espressif Systems

35 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

CPU0x0000_00000x3BFF_FFFF0x3C00_00000x3DFF_FFFF0x3E00_00000x3FCB_FFFF0x3FCB_80000x3FCF_FFFF0x3FD0_00000x3FEF_FFFF0x3FF0_00000x3FF1_FFFF0x3FF2_00000x3FFF_FFFF0x4000_00000x4005_FFFF0x4006_00000x4036_FFFF0x4037_00000x403D_FFFF0x403E_00000x41FF_FFFF0x4200_00000x43FF_FFFF0x4400_00000x4FFF_FFFF0x5000_00000x5000_1FFF0x5000_20000x5FFF_FFFF0x6000_00000x600D_0FFF0x600F_E0000x600F_FFFF0x600D_10000x600F_DFFFNot available for useAvailable for useCacheMMUExternal MemorySRAMROMGDMARTCFast MemoryRTCSlow MemoryPeripherals0x6010_00000xFFFF_FFFF4 Functional Description

4.1.2.1

Internal Memory

The internal memory of ESP32-S3 refers to the memory integrated on the chip die or in the chip package,

including ROM, SRAM, eFuse, and flash.

Feature List

• 384 KB ROM: for booting and core functions

• 512 KB on-chip SRAM: for data and instructions, running at a configurable frequency of up to 240 MHz

• RTC FAST memory: 8 KB SRAM that supports read/write/instruction fetch by the main CPU (LX7

dual-core processor). It can retain data in Deep-sleep mode

• RTC SLOW Memory: 8 KB SRAM that supports read/write/instruction fetch by the main CPU (LX7

dual-core processor) or coprocessors. It can retain data in Deep-sleep mode

• 4 Kbit eFuse: 1792 bits are available for users, such as encryption key and device ID. See also Section

4.1.2.4 eFuse Controller

• In-package flash:

– See flash size in Chapter 1 ESP32-S3 Series Comparison

– More than 100,000 program/erase cycles

– More than 20 years of data retention time

– Clock frequency up to 80 MHz by default

• In-package PSRAM: See details in Table 1-1 ESP32-S3 Series Comparison

For details, see ESP32-S3 Technical Reference Manual > Chapter System and Memory.

4.1.2.2 External Flash and RAM

ESP32-S3 supports SPI, Dual SPI, Quad SPI, Octal SPI, QPI, and OPI interfaces that allow connection to

multiple external flash and RAM.

The external flash and RAM can be mapped into the CPU instruction memory space and read-only data

memory space. The external RAM can also be mapped into the CPU data memory space. ESP32-S3 supports

up to 1 GB of external flash and RAM, and hardware encryption/decryption based on XTS-AES to protect users’

programs and data in flash and external RAM.

Through high-speed caches, ESP32-S3 can support at a time up to:

• External flash or RAM mapped into 32 MB instruction space as individual blocks of 64 KB

• External RAM mapped into 32 MB data space as individual blocks of 64 KB. 8-bit, 16-bit, 32-bit, and 128-bit reads and writes are supported. External flash can also be mapped into 32 MB data space as

individual blocks of 64 KB, but only supporting 8-bit, 16-bit, 32-bit and 128-bit reads.

Note:

After ESP32-S3 is initialized, firmware can customize the mapping of external RAM or flash into the CPU address space.

For details, see ESP32-S3 Technical Reference Manual > Chapter System and Memory.

Espressif Systems

36 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.1.2.3 Cache

ESP32-S3 has an instruction cache and a data cache shared by the two CPU cores. Each cache can be

partitioned into multiple banks.

Feature List

• Instruction cache: 16 KB (one bank) or 32 KB (two banks)

Data cache: 32 KB (one bank) or 64 KB (two banks)

• Instruction cache: four-way or eight-way set associative

Data cache: four-way set associative

• Block size of 16 bytes or 32 bytes for both instruction cache and data cache

• Pre-load function

• Lock function

• Critical word first and early restart

For details, see ESP32-S3 Technical Reference Manual > Chapter System and Memory.

4.1.2.4 eFuse Controller

ESP32-S3 contains a 4-Kbit eFuse to store parameters, which are burned and read by an eFuse

controller.

Feature List

• 4 Kbits in total, with 1792 bits reserved for users, e.g., encryption key and device ID

• One-time programmable storage

• Configurable write protection

• Configurable read protection

• Various hardware encoding schemes to protect against data corruption

For details, see ESP32-S3 Technical Reference Manual > Chapter eFuse Controller.

4.1.3 System Components

This subsection describes the essential components that contribute to the overall functionality and control of

the system.

4.1.3.1

IO MUX and GPIO Matrix

The IO MUX and GPIO Matrix in the ESP32-S3 chip provide flexible routing of peripheral input and output

signals to the GPIO pins. These peripherals enhance the functionality and performance of the chip by allowing

the configuration of I/O, support for multiplexing, and signal synchronization for peripheral inputs.

Espressif Systems

37 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

Feature List

• GPIO Matrix:

– A full-switching matrix between the peripheral input/output signals and the GPIO pins

– 175 digital peripheral input signals can be sourced from the input of any GPIO pins

– The output of any GPIO pins can be from any of the 184 digital peripheral output signals

– Supports signal synchronization for peripheral inputs based on APB clock bus

– Provides input signal filter

– Supports sigma delta modulated output

– Supports GPIO simple input and output

• IO MUX:

– Provides one configuration register IO_MUX_GPIOn_REG for each GPIO pin. The pin can be

configured to

perform GPIO function routed by GPIO matrix

or perform direct connection bypassing GPIO matrix

– Supports some high-speed digital signals (SPI, JTAG, UART) bypassing GPIO matrix for better

high-frequency digital performance (IO MUX is used to connect these pins directly to peripherals)

• RTC IO MUX:

– Controls low power feature of 22 RTC GPIO pins

– Controls analog functions of 22 RTC GPIO pins

– Redirects 22 RTC input/output signals to RTC system

For details, see ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.1.3.2 Reset

ESP32-S3 provides four reset levels, namely CPU Reset, Core Reset, System Reset, and Chip Reset.

Feature List

• Support four reset levels:

– CPU Reset: only resets CPUx core. CPUx can be CPU0 or CPU1 here. Once such reset is released,

programs will be executed from CPUx reset vector. Each CPU core has its own reset logic. If CPU

Reset is from CPU0, the sensitive registers will be reset, too.

– Core Reset: resets the whole digital system except RTC, including CPU0, CPU1, peripherals, Wi-Fi,

Bluetooth® LE (BLE), and digital GPIOs.

– System Reset: resets the whole digital system, including RTC.

– Chip Reset: resets the whole chip.

• Support software reset and hardware reset:

Espressif Systems

38 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

– Software reset is triggered by CPUx configuring its corresponding registers. Refer to

ESP32-S3 Technical Reference Manual > Chapter Low-power Management for more details.

– Hardware reset is directly triggered by the circuit.

For details, see ESP32-S3 Technical Reference Manual > Chapter Reset and Clock.

4.1.3.3 Clock

CPU Clock

The CPU clock has three possible sources:

• External main crystal clock

• Internal fast RC oscillator (typically about 17.5 MHz, adjustable)

• PLL clock

The application can select the clock source from the three clocks above. The selected clock source drives

the CPU clock directly, or after division, depending on the application. Once the CPU is reset, the default

clock source would be the external main crystal clock divided by 2.

Note:

ESP32-S3 is unable to operate without an external main crystal clock.

RTC Clock

The RTC slow clock is used for RTC counter, RTC watchdog and low-power controller. It has three possible

sources:

• External low-speed (32 kHz) crystal clock

• Internal slow RC oscillator (typically about 136 kHz, adjustable)

• Internal fast RC oscillator divided clock (derived from the internal fast RC oscillator divided by 256)

The RTC fast clock is used for RTC peripherals and sensor controllers. It has two possible sources:

• External main crystal clock divided by 2

• Internal fast RC oscillator (typically about 17.5 MHz, adjustable)

For details, see ESP32-S3 Technical Reference Manual > Chapter Reset and Clock.

4.1.3.4 Interrupt Matrix

The interrupt matrix embedded in ESP32-S3 independently allocates peripheral interrupt sources to the two

CPUs’ peripheral interrupts, to timely inform CPU0 or CPU1 to process the interrupts once the interrupt signals

are generated.

Feature List

• 99 peripheral interrupt sources as input

Espressif Systems

39 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

• Generate 26 peripheral interrupts to CPU0 and 26 peripheral interrupts to CPU1 as output.

Note that the remaining six CPU0 interrupts and six CPU1 interrupts are internal interrupts.

• Disable CPU non-maskable interrupt (NMI) sources

• Query current interrupt status of peripheral interrupt sources

For details, see ESP32-S3 Technical Reference Manual > Chapter Interrupt Matrix.

4.1.3.5 Power Management Unit (PMU)

ESP32-S3 has an advanced Power Management Unit (PMU). It can be flexibly configured to power up

different power domains of the chip to achieve the best balance between chip performance, power

consumption, and wakeup latency.

The integrated Ultra-Low-Power (ULP) coprocessors allow ESP32-S3 to operate in Deep-sleep mode with

most of the power domains turned off, thus achieving extremely low-power consumption.

Configuring the PMU is a complex procedure. To simplify power management for typical scenarios, there are

the following predefined power modes that power up different combinations of power domains:

• Active mode – The CPU, RF circuits, and all peripherals are on. The chip can process data, receive,

transmit, and listen.

• Modem-sleep mode – The CPU is on, but the clock frequency can be reduced. The wireless

connections can be configured to remain active as RF circuits are periodically switched on when

required.

• Light-sleep mode – The CPU stops running, and can be optionally powered on. The RTC peripherals, as well as the ULP coprocessor can be woken up periodically by the timer. The chip can be woken up via

all wake up mechanisms: MAC, RTC timer, or external interrupts. Wireless connections can remain active.

Some groups of digital peripherals can be optionally powered off.

• Deep-sleep mode – Only RTC is powered on. Wireless connection data is stored in RTC memory.

For power consumption in different power modes, see Section 5.6 Current Consumption.

Figure 4-2 Components and Power Domains and the following Table 4-1 show the distribution of chip

components between power domains and power subdomains .

Espressif Systems

40 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

Figure 4-2. Components and Power Domains

Table 4-1. Components and Power Domains

Power

RTC

Digital

Analog

Domain

Power

Mode

Active

Modem-sleep

Light-sleep

Deep-sleep

Optional

RTC

Periph

ON

ON

ON ON1

ON

ON

ON

ON

CPU

ON

ON OFF1 OFF

ON

ON

ON

OFF

Optional

Wireless

Digital

Periph

Digital

Circuits

ON

ON ON1 OFF

ON ON1 OFF1 OFF

ON

ON

ON

ON

RC_

FAST_

CLK

ON

ON

OFF

OFF

XTAL_

CLK

ON

ON

OFF

OFF

PLL

ON

ON

OFF

OFF

RF

Circuits

ON OFF2 OFF2 OFF

1 Configurable. See ESP32-S3 Technical Reference Manual > Chapter Low-power Management for more details. 2 If Wireless Digital Circuits are on, RF circuits are periodically switched on when required by internal operation to keep

active wireless connections running.

Espressif Systems

41 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Wireless Digital CircuitsWi-Fi MAC Wi-Fi BasebandBluetooth LE Link ControllerBluetooth LE BasebandDigital Power DomainEspressif’s ESP32-S3 Wi-Fi + Bluetooth® Low Energy SoCROMSRAM2.4 GHz Balun + Switch2.4 GHz Receiver2.4 GHz TransmitterRF SynthesizerRF CircuitsPhase Lock LoopPLLXTAL_CLKExternal Main ClockRC_FAST_CLKFast RC OscillatorAnalog Power DomainFlash EncryptionRNGUSB Serial/JTAGGPIOUARTTWAI®General-purpose TimersI2SI2CPulse CounterLED PWMCamera InterfaceSPI0/1RMTDIG ADCSystem TimerLCD InterfaceMain System Watchdog TimersMCPWMRTC MemoryRTC Watchdog TimerPMURTC Power DomainRTC GPIOTemperature SensorTouchSensorULP CoprocessorRTC ADCOptional RTC PeripheralsRTC I2CeFuse ControllerPower distributionPower domainPower subdomainSuper WatchdogCPUXtensa® Dual-core 32-bit LX7 MicroprocessorJTAGCacheInterrupt MatrixWorld ControllerOptional Digital Peripherals RSADigital SignatureSHAAESHMACSecure BootSPI2/3GDMASD/MMC HostUSB OTG4 Functional Description

4.1.3.6 System Timer

ESP32-S3 integrates a 52-bit system timer, which has two 52-bit counters and three comparators.

Feature List

• Counters with a clock frequency of 16 MHz

• Three types of independent interrupts generated according to alarm value

• Two alarm modes: target mode and period mode

• 52-bit target alarm value and 26-bit periodic alarm value

• Read sleep time from RTC timer when the chip is awaken from Deep-sleep or Light-sleep mode

• Counters can be stalled if the CPU is stalled or in OCD mode

For details, see ESP32-S3 Technical Reference Manual > Chapter System Timer.

4.1.3.7 General Purpose Timers

ESP32-S3 is embedded with four 54-bit general-purpose timers, which are based on 16-bit prescalers and

54-bit auto-reload-capable up/down-timers.

Feature List

• 16-bit clock prescaler, from 2 to 65536

• 54-bit time-base counter programmable to be incrementing or decrementing

• Able to read real-time value of the time-base counter

• Halting and resuming the time-base counter

• Programmable alarm generation

• Timer value reload (Auto-reload at alarm or software-controlled instant reload)

• Level interrupt generation

For details, see ESP32-S3 Technical Reference Manual > Chapter Timer Group.

4.1.3.8 Watchdog Timers

ESP32-S3 contains three watchdog timers: one in each of the two timer groups (called Main System

Watchdog Timers, or MWDT) and one in the RTC Module (called the RTC Watchdog Timer, or RWDT).

During the flash boot process, RWDT and the first MWDT are enabled automatically in order to detect and

recover from booting errors.

Feature List

• Four stages:

– Each with a programmable timeout value

– Each stage can be configured, enabled and disabled separately

Espressif Systems

42 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

• Upon expiry of each stage:

– Interrupt, CPU reset, or core reset occurs for MWDT

– Interrupt, CPU reset, core reset, or system reset occurs for RWDT

• 32-bit expiry counter

• Write protection, to prevent RWDT and MWDT configuration from being altered inadvertently

• Flash boot protection: If the boot process from an SPI flash does not complete within a predetermined

period of time, the watchdog will reboot the entire main system

For details, see ESP32-S3 Technical Reference Manual > Chapter Watchdog Timers.

4.1.3.9 XTAL32K Watchdog Timers

Interrupt and Wake-Up

When the XTAL32K watchdog timer detects the oscillation failure of XTAL32K_CLK, an oscillation failure

interrupt RTC_XTAL32K_DEAD_INT (for interrupt description, please refer to

ESP32-S3 Technical Reference Manual > Chapter Low-power Management) is generated. At this point, the

CPU will be woken up if in Light-sleep mode or Deep-sleep mode.

BACKUP32K_CLK

Once the XTAL32K watchdog timer detects the oscillation failure of XTAL32K_CLK, it replaces XTAL32K_CLK

with BACKUP32K_CLK (with a frequency of 32 kHz or so) derived from RTC_CLK as RTC’s SLOW_CLK, so as to

ensure proper functioning of the system.

For details, see ESP32-S3 Technical Reference Manual > Chapter XTAL32K Watchdog Timers.

4.1.3.10 Permission Control

In ESP32-S3, the Permission Control module is used to control access to the slaves (including internal

memory, peripherals, external flash, and RAM). The host can access its slave only if it has the right permission.

In this way, data and instructions are protected from illegitimate read or write.

The ESP32-S3 CPU can run in both Secure World and Non-secure World where independent permission

controls are adopted. The Permission Control module is able to identify which World the host is running and

then proceed with its normal operations.

Feature List

• Manage access to internal memory by:

– CPU

– CPU trace module

– GDMA

• Manage access to external flash and RAM by:

– MMU

– SPI1

Espressif Systems

43 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

– GDMA

– CPU through Cache

• Manage access to peripherals, supporting

– independent permission control for each peripheral

– monitoring non-aligned access

– access control for customized address range

• Integrate permission lock register

– All permission registers can be locked with the permission lock register. Once locked, the

permission register and the lock register cannot be modified, unless the CPU is reset.

• Integrate permission monitor interrupt

– In case of illegitimate access, the permission monitor interrupt will be triggered and the CPU will be

informed to handle the interrupt.

For details, see ESP32-S3 Technical Reference Manual > Chapter Permission Control.

4.1.3.11 World Controller

ESP32-S3 can divide the hardware and software resources into a Secure World and a Non-Secure World to

prevent sabotage or access to device information. Switching between the two worlds is performed by the

World Controller.

Feature List

• Control of the CPU switching between secure and non-secure worlds

• Control of 15 DMA peripherals switching between secure and non-secure worlds

• Record of CPU’s world switching logs

• Shielding of the CPU’s NMI interrupt

For details, see ESP32-S3 Technical Reference Manual > Chapter World Controller.

4.1.3.12 System Registers

ESP32-S3 system registers can be used to control the following peripheral blocks and core modules:

• System and memory

• Clock

• Software Interrupt

• Low-power management

• Peripheral clock gating and reset

• CPU Control

For details, see ESP32-S3 Technical Reference Manual > Chapter System Registers.

Espressif Systems

44 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.1.4 Cryptography and Security Component

This subsection describes the security features incorporated into the chip, which safeguard data and

operations.

4.1.4.1 SHA Accelerator

ESP32-S3 integrates an SHA accelerator, which is a hardware device that speeds up SHA algorithm

significantly.

Feature List

• All the hash algorithms introduced in FIPS PUB 180-4 Spec.

– SHA-1

– SHA-224

– SHA-256

– SHA-384

– SHA-512

– SHA-512/224

– SHA-512/256

– SHA-512/t

• Two working modes

– Typical SHA

– DMA-SHA

• interleaved function when working in Typical SHA working mode

• Interrupt function when working in DMA-SHA working mode

For details, see ESP32-S3 Technical Reference Manual > Chapter SHA Accelerator.

4.1.4.2 AES Accelerator

ESP32-S3 integrates an Advanced Encryption Standard (AES) Accelerator, which is a hardware device that

speeds up AES algorithm significantly.

Feature List

• Typical AES working mode

– AES-128/AES-256 encryption and decryption

• DMA-AES working mode

– AES-128/AES-256 encryption and decryption

– Block cipher mode

ECB (Electronic Codebook)
Espressif Systems

45 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

CBC (Cipher Block Chaining)

OFB (Output Feedback)

CTR (Counter)

CFB8 (8-bit Cipher Feedback)

CFB128 (128-bit Cipher Feedback)

– Interrupt on completion of computation

For details, see ESP32-S3 Technical Reference Manual > Chapter AES Accelerator.

4.1.4.3 RSA Accelerator

The RSA Accelerator provides hardware support for high precision computation used in various RSA

asymmetric cipher algorithms.

Feature List

• Large-number modular exponentiation with two optional acceleration options

• Large-number modular multiplication, up to 4096 bits

• Large-number multiplication, with operands up to 2048 bits

• Operands of different lengths

• Interrupt on completion of computation

For details, see ESP32-S3 Technical Reference Manual > Chapter RSA Accelerator.

4.1.4.4 Secure Boot

Secure Boot feature uses a hardware root of trust to ensure only signed firmware (with RSA-PSS signature) can

be booted.

4.1.4.5 HMAC Accelerator

The Hash-based Message Authentication Code (HMAC) module computes Message Authentication Codes

(MACs) using Hash algorithm and keys as described in RFC 2104.

Feature List

• Standard HMAC-SHA-256 algorithm

• Hash result only accessible by configurable hardware peripheral (in downstream mode)

• Compatible to challenge-response authentication algorithm

• Generates required keys for the Digital Signature (DS) peripheral (in downstream mode)

• Re-enables soft-disabled JTAG (in downstream mode)

For details, see ESP32-S3 Technical Reference Manual > Chapter HMAC Accelerator.

Espressif Systems

46 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.1.4.6 Digital Signature

A Digital Signature is used to verify the authenticity and integrity of a message using a cryptographic

algorithm.

Feature List

• RSA Digital Signatures with key length up to 4096 bits

• Encrypted private key data, only decryptable by DS peripheral

• SHA-256 digest to protect private key data against tampering by an attacker

For details, see ESP32-S3 Technical Reference Manual > Chapter Digital Signature.

4.1.4.7 External Memory Encryption and Decryption

ESP32-S3 integrates an External Memory Encryption and Decryption module that complies with the XTS-AES

standard.

Feature List

• General XTS-AES algorithm, compliant with IEEE Std 1619-2007

• Software-based manual encryption

• High-speed auto encryption, without software’s participation

• High-speed auto decryption, without software’s participation

• Encryption and decryption functions jointly determined by registers configuration, eFuse parameters,

and boot mode

For details, see ESP32-S3 Technical Reference Manual > Chapter External Memory Encryption and

Decryption.

4.1.4.8 Clock Glitch Detection

The Clock Glitch Detection module on ESP32-S3 monitors input clock signals from XTAL_CLK. If it detects a

glitch with a width shorter than 3 ns, input clock signals from XTAL_CLK are blocked.

For details, see ESP32-S3 Technical Reference Manual > Chapter Clock Glitch Detection.

4.1.4.9 Random Number Generator

The random number generator (RNG) in ESP32-S3 generates true random numbers, which means random

number generated from a physical process, rather than by means of an algorithm. No number generated

within the specified range is more or less likely to appear than any other number.

For details, see ESP32-S3 Technical Reference Manual > Chapter Random Number Generator.

Espressif Systems

47 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.2 Peripherals

This section describes the chip’s peripheral capabilities, covering connectivity interfaces and on-chip sensors

that extend its functionality.

4.2.1 Connectivity Interface

This subsection describes the connectivity interfaces on the chip that enable communication and interaction

with external devices and networks.

4.2.1.1 UART Controller

ESP32-S3 has three UART (Universal Asynchronous Receiver Transmitter) controllers, i.e., UART0, UART1, and

UART2, which support IrDA and asynchronous communication (RS232 and RS485) at a speed of up to 5

Mbps.

Feature List

• Three clock sources that can be divided

• Programmable baud rate

• 1024 x 8-bit RAM shared by TX FIFOs and RX FIFOs of the three UART controllers

• Full-duplex asynchronous communication

• Automatic baud rate detection of input signals

• Data bits ranging from 5 to 8

• Stop bits of 1, 1.5, 2, or 3 bits

• Parity bit

• Special character AT_CMD detection

• RS485 protocol

• IrDA protocol

• High-speed data communication using GDMA

• UART as wake-up source

• Software and hardware flow control

For details, see ESP32-S3 Technical Reference Manual > Chapter UART Controller.

Pin Assignment

• UART0

– The pins U0TXD and U0RXD that are connected to transmit and receive signals are multiplexed with

GPIO43 ~ GPIO44 via IO MUX, and can also be connected to any GPIO via the GPIO Matrix.

– The pins U0RTS and U0CTS that are connected to hardware flow control signals are multiplexed

with GPIO15 ~ GPIO16, RTC_GPIO15 ~ RTC_GPIO16, XTAL_32K_P and XTAL_32K_N, and SAR ADC2

interface via IO MUX, and can also be connected to any GPIO via the GPIO Matrix.

Espressif Systems

48 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

– The pins U0DTR and U0DSR that are connected to hardware flow control signals can be chosen

from any GPIO via the GPIO Matrix.

• UART1

– The pins U1TXD and U1RXD that are connected to transmit and receive signals are multiplexed with

GPIO17 ~ GPIO18, RTC_GPIO17 ~ RTC_GPIO18, and SAR ADC2 interface via IO MUX, and can also be

connected to any GPIO via the GPIO Matrix.

– The pins U1RTS and U1CTS that are connected to hardware flow control signals are multiplexed with

GPIO19 ~ GPIO20, RTC_GPIO19 ~ RTC_GPIO20, USB_D- and USB_D+ pins, and SAR ADC2 interface

via IO MUX, and can also be connected to any GPIO via the GPIO Matrix.

– The pins U1DTR and U1DSR that are connected to hardware flow control signals can be chosen

from any GPIO via the GPIO Matrix.

• UART2: The pins used can be chosen from any GPIO via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.2 I2C Interface

ESP32-S3 has two I2C bus interfaces which are used for I2C master mode or slave mode, depending on the

user’s configuration.

Feature List

• Standard mode (100 kbit/s)

• Fast mode (400 kbit/s)

• Up to 800 kbit/s (constrained by SCL and SDA pull-up strength)

• 7-bit and 10-bit addressing mode

• Double addressing mode (slave addressing and slave register addressing)

The hardware provides a command abstraction layer to simplify the usage of the I2C peripheral.

For details, see ESP32-S3 Technical Reference Manual > Chapter I2C Controller.

Pin Assignment

For I2C, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.3 I2S Interface

ESP32-S3 includes two standard I2S interfaces. They can operate in master mode or slave mode, in

full-duplex mode or half-duplex communication mode, and can be configured to operate with an 8-bit, 16-bit,

24-bit, or 32-bit resolution as an input or output channel. BCK clock frequency, from 10 kHz up to 40 MHz, is

supported.

Espressif Systems

49 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

The I2S interface has a dedicated DMA controller. It supports TDM PCM, TDM MSB alignment, TDM LSB

alignment, TDM Phillips, and PDM interface.

Pin Assignment

For I2S, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.4 LCD and Camera Controller

The LCD and Camera controller of ESP32-S3 consists of a LCD module and a camera module.

The LCD module is designed to send parallel video data signals, and its bus supports 8-bit ~ 16-bit parallel

RGB, I8080, and MOTO6800 interfaces. These interfaces operate at 40 MHz or lower, and support conversion

among RGB565, YUV422, YUV420, and YUV411.

The camera module is designed to receive parallel video data signals, and its bus supports an 8-bit ~ 16-bit

DVP image sensor, with clock frequency of up to 40 MHz. The camera interface supports conversion among

RGB565, YUV422, YUV420, and YUV411.

Pin Assignment

For LCD and Camera controller, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.5 Serial Peripheral Interface (SPI)

ESP32-S3 has the following SPI interfaces:

• SPI0 used by ESP32-S3’s GDMA controller and cache to access in-package or off-package flash/PSRAM

• SPI1 used by the CPU to access in-package or off-package flash/PSRAM

• SPI2 is a general purpose SPI controller with access to a DMA channel allocated by the GDMA controller

• SPI3 is a general purpose SPI controller with access to a DMA channel allocated by the GDMA controller

Feature List

• SPI0 and SPI1:

– Supports Single SPI, Dual SPI, Quad SPI, Octal SPI, QPI, and OPI modes

– 8-line SPI mode supports single data rate (SDR) and double data rate (DDR)

– Configurable clock frequency with a maximum of 120 MHz for 8-line SPI SDR/DDR modes

– Data transmission is in bytes

• SPI2:

– Supports operation as a master or slave

Espressif Systems

50 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

– Connects to a DMA channel allocated by the GDMA controller

– Supports Single SPI, Dual SPI, Quad SPI, Octal SPI, QPI, and OPI modes

– Configurable clock polarity (CPOL) and phase (CPHA)

– Configurable clock frequency

– Data transmission is in bytes

– Configurable read and write data bit order: most-significant bit (MSB) first, or least-significant bit

(LSB) first

– As a master

Supports 2-line full-duplex communication with clock frequency up to 80 MHz

Full-duplex 8-line SPI mode supports single data rate (SDR) only

Supports 1-, 2-, 4-, 8-line half-duplex communication with clock frequency up to 80 MHz

Half-duplex 8-line SPI mode supports both single data rate (up to 80 MHz) and double data rate

(up to 40 MHz)

Provides six SPI_CS pins for connection with six independent SPI slaves

Configurable CS setup time and hold time

– As a slave

Supports 2-line full-duplex communication with clock frequency up to 60 MHz

Supports 1-, 2-, 4-line half-duplex communication with clock frequency up to 60 MHz

Full-duplex and half-duplex 8-line SPI mode supports single data rate (SDR) only

• SPI3:

– Supports operation as a master or slave

– Connects to a DMA channel allocated by the GDMA controller

– Supports Single SPI, Dual SPI, Quad SPI, and QPI modes

– Configurable clock polarity (CPOL) and phase (CPHA)

– Configurable clock frequency

– Data transmission is in bytes

– Configurable read and write data bit order: most-significant bit (MSB) first, or least-significant bit

(LSB) first

– As a master

Supports 2-line full-duplex communication with clock frequency up to 80 MHz

Supports 1-, 2-, 4-line half-duplex communication with clock frequency up to 80 MHz

Provides three SPI_CS pins for connection with three independent SPI slaves

Configurable CS setup time and hold time

– As a slave

Espressif Systems

51 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

Supports 2-line full-duplex communication with clock frequency up to 60 MHz

Supports 1-, 2-, 4-line half-duplex communication with clock frequency up to 60 MHz

For details, see ESP32-S3 Technical Reference Manual > Chapter SPI Controller.

Pin Assignment

• SPI0/1

– Via IO MUX:

Interface 4a (see Table 2-4) is multiplexed with GPIO26 ~ GPIO32 via IO MUX. When used in
conjunction with 4b, it can operate as the lower 4 bits data line interface and the CLK, CS0, and

CS1 interfaces in 8-line SPI mode.

Interface 4b (see Table 2-4) is multiplexed with GPIO33 ~ GPIO37 and SPI interfaces 4e and 4f
via IO MUX. When used in conjunction with 4a, it can operate as the higher 4 bits data line

interface and DQS interface in 8-line SPI mode.

Interface 4d (see Table 2-4) is multiplexed with GPIO8 ~ GPIO14, RTC_GPIO8 ~ RTC_GPIO14,
Touch Sensor interface, SAR ADC interface, and SPI interfaces 4c and 4g via IO MUX. Note that

the fast SPI2 interface will not be available.

Interface 4e (see Table 2-4) is multiplexed with GPIO33 ~ GPIO39, JTAG MTCK interface, and
SPI interfaces 4b and 4f via IO MUX. It is an alternative group of signal lines that can be used if

SPI0/1 does not use 8-line SPI connection.

– Via GPIO Matrix: The pins used can be chosen from any GPIOs via the GPIO Matrix.

• SPI2

– Via IO MUX:

Interface 4c (see Table 2-4) is multiplexed with GPIO9 ~ GPIO14, RTC_GPIO9 ~ RTC_GPIO14,
Touch Sensor interface, SAR ADC interface, and SPI interfaces 4d and 4g via IO MUX. It is the

SPI2 main interface for fast SPI connection.

(not recommended) Interface 4f (see Table 2-4) is multiplexed with GPIO33 ~ GPIO38, SPI
interfaces 4e and 4b via IO MUX. It is the alternative SPI2 interface if the main SPI2 is not

available. Its performance is comparable to SPI2 via GPIO matrix, so use the GPIO matrix

instead.

(not recommended) Interface 4g (see Table 2-4) is multiplexed with GPIO10 ~ GPIO14,
RTC_GPIO10 ~ RTC_GPIO14, Touch Sensor interface, SAR ADC interface, and SPI interfaces 4c

and 4d via IO MUX. It is the alternative SPI2 interface signal lines for 8-line SPI connection.

– Via GPIO Matrix: The pins used can be chosen from any GPIOs via the GPIO Matrix.

• SPI3: The pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

Espressif Systems

52 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.2.1.6 Two-Wire Automotive Interface (TWAI®)

The Two-Wire Automotive Interface (TWAI®) is a multi-master, multi-cast communication protocol with error detection and signaling as well as inbuilt message priorities and arbitration.

Feature List

• Compatible with ISO 11898-1 protocol (CAN Specification 2.0)

• Standard frame format (11-bit ID) and extended frame format (29-bit ID)

• Bit rates from 1 Kbit/s to 1 Mbit/s

• Multiple modes of operation:

– Normal

– Listen Only

– Self-Test (no acknowledgment required)

• 64-byte receive FIFO

• Acceptance filter (single and dual filter modes)

• Error detection and handling:

– Error counters

– Configurable error interrupt threshold

– Error code capture

– Arbitration lost capture

For details, see ESP32-S3 Technical Reference Manual > Chapter Two-wire Automotive Interface.

Pin Assignment

For TWAI, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.7 USB 2.0 OTG Full-Speed Interface

ESP32-S3 features a full-speed USB OTG interface along with an integrated transceiver. The USB OTG

interface complies with the USB 2.0 specification.

General Features

• FS and LS data rates

• HNP and SRP as A-device or B-device

• Dynamic FIFO (DFIFO) sizing

• Multiple modes of memory access

Espressif Systems

53 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

– Scatter/Gather DMA mode

– Buffer DMA mode

– Slave mode

• Can choose integrated transceiver or external transceiver

• Utilizing integrated transceiver with USB Serial/JTAG by time-division multiplexing when only integrated

transceiver is used

• Support USB OTG using one of the transceivers while USB Serial/JTAG using the other one when both

integrated transceiver or external transceiver are used

Device Mode Features

• Endpoint number 0 always present (bi-directional, consisting of EP0 IN and EP0 OUT)

• Six additional endpoints (endpoint numbers 1 to 6), configurable as IN or OUT

• Maximum of five IN endpoints concurrently active at any time (including EP0 IN)

• All OUT endpoints share a single RX FIFO

• Each IN endpoint has a dedicated TX FIFO

Host Mode Features

• Eight channels (pipes)

– A control pipe consists of two channels (IN and OUT), as IN and OUT transactions must be handled

separately. Only Control transfer type is supported.

– Each of the other seven channels is dynamically configurable to be IN or OUT, and supports Bulk,

Isochronous, and Interrupt transfer types.

• All channels share an RX FIFO, non-periodic TX FIFO, and periodic TX FIFO. The size of each FIFO is

configurable.

For details, see ESP32-S3 Technical Reference Manual > Chapter USB On-The-Go.

Pin Assignment

When using the on-chip PHY, the differential signal pins USB_D- and USB_D+ of the USB OTG are multiplexed

with GPIO19 ~ GPIO20, RTC_GPIO19 ~ RTC_GPIO20, UART1 interface, and SAR ADC2 interface via IO

MUX.

When using external PHY, the USB OTG pins are multiplexed with GPIO21, RTC_GPIO21, GPIO38 ~ GPIO42, and

SPI interface via IO MUX:

• VP signal connected to MTMS pin

• VM signal connected to MTDI pin

• RCV signal connected to GPIO21

• OEN signal connected to MTDO pin

• VPO signal connected to MTCK pin

Espressif Systems

54 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

• VMO signal connected to GPIO38

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.8 USB Serial/JTAG Controller

ESP32-S3 integrates a USB Serial/JTAG controller.

Feature List

• USB Full-speed device.

• Can be configured to either use internal USB PHY of ESP32-S3 or external PHY via GPIO matrix.

• Fixed function device, hardwired for CDC-ACM (Communication Device Class - Abstract Control Model)

and JTAG adapter functionality.

• Two OUT Endpoints, three IN Endpoints in addition to Control Endpoint 0; Up to 64-byte data payload

size.

• Internal PHY, so no or very few external components needed to connect to a host computer.

• CDC-ACM adherent serial port emulation is plug-and-play on most modern OSes.

• JTAG interface allows fast communication with CPU debug core using a compact representation of JTAG

instructions.

• CDC-ACM supports host controllable chip reset and entry into download mode.

For details, see ESP32-S3 Technical Reference Manual > Chapter USB Serial/JTAG Controller.

Pin Assignment

When using the on-chip PHY, the differential signal pins USB_D- and USB_D+ of the USB Serial/JTAG

controller are multiplexed with GPIO19 ~ GPIO20, RTC_GPIO19 ~ RTC_GPIO20, UART1 interface, and SAR ADC2

interface via IO MUX.

When using external PHY, the USB Serial/JTAG controller pins are multiplexed with GPIO38 ~ GPIO42 and SPI

interface via IO MUX:

• VP signal connected to MTMS pin

• VM signal connected to MTDI pin

• OEN signal connected to MTDO pin

• VPO signal connected to MTCK pin

• VMO signal connected to GPIO38

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.9 SD/MMC Host Controller

ESP32-S3 has an SD/MMC Host controller.

Espressif Systems

55 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

Feature List

• Secure Digital (SD) memory version 3.0 and version 3.01

• Secure Digital I/O (SDIO) version 3.0

• Consumer Electronics Advanced Transport Architecture (CE-ATA) version 1.1

• Multimedia Cards (MMC version 4.41, eMMC version 4.5 and version 4.51)

• Up to 80 MHz clock output

• Three data bus modes:

– 1-bit

– 4-bit (supports two SD/SDIO/MMC 4.41 cards, and one SD card operating at 1.8 V in 4-bit mode)

– 8-bit

For details, see ESP32-S3 Technical Reference Manual > Chapter SD/MMC Host Controller.

Pin Assignment

For SD/MMC Host, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.10 LED PWM Controller

The LED PWM controller can generate independent digital waveforms on eight channels.

Feature List

• Can generate a digital waveform with configurable periods and duty cycle. The duty cycle resolution can

be up to 14 bits within a 1 ms period

• Multiple clock sources, including APB clock and external main crystal clock

• Can operate when the CPU is in Light-sleep mode

• Gradual increase or decrease of duty cycle, useful for the LED RGB color-fading generator

For details, see ESP32-S3 Technical Reference Manual > Chapter LED PWM Controller.

Pin Assignment

For LED PWM, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.11 Motor Control PWM (MCPWM)

ESP32-S3 integrates two MCPWMs that can be used to drive digital motors and smart light. Each MCPWM

peripheral has one clock divider (prescaler), three PWM timers, three PWM operators, and a capture module.

Espressif Systems

56 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

PWM timers are used for generating timing references. The PWM operators generate desired waveform based

on the timing references. Any PWM operator can be configured to use the timing references of any PWM

timers. Different PWM operators can use the same PWM timer’s timing references to produce related PWM

signals. PWM operators can also use different PWM timers’ values to produce the PWM signals that work

alone. Different PWM timers can also be synchronized together.

For details, see ESP32-S3 Technical Reference Manual > Chapter Motor Control PWM.

Pin Assignment

For MCPWM, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.12 Remote Control Peripheral (RMT)

The Remote Control Peripheral (RMT) is designed to send and receive infrared remote control signals.

Feature List

• Four TX channels

• Four RX channels

• Support multiple channels (programmable) transmitting data simultaneously

• Eight channels share a 384 x 32-bit RAM

• Support modulation on TX pulses

• Support filtering and demodulation on RX pulses

• Wrap TX mode

• Wrap RX mode

• Continuous TX mode

• DMA access for TX mode on channel 3

• DMA access for RX mode on channel 7

For details, see ESP32-S3 Technical Reference Manual > Chapter Remote Control Peripheral.

Pin Assignment

For RMT, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.1.13 Pulse Count Controller (PCNT)

The pulse count controller (PCNT) captures pulse and counts pulse edges through multiple modes.

Espressif Systems

57 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

Feature List

• Four independent pulse counters (units) that count from 1 to 65535

• Each unit consists of two independent channels sharing one pulse counter

• All channels have input pulse signals (e.g. sig_ch0_un) with their corresponding control signals (e.g.

ctrl_ch0_un)

• Independently filter glitches of input pulse signals (sig_ch0_un and sig_ch1_un) and control signals

(ctrl_ch0_un and ctrl_ch1_un) on each unit

• Each channel has the following parameters:

Selection between counting on positive or negative edges of the input pulse signal

Configuration to Increment, Decrement, or Disable counter mode for control signal’s high and low

states

For details, see ESP32-S3 Technical Reference Manual > Chapter Pulse Count Controller.

Pin Assignment

For pulse count controller, the pins used can be chosen from any GPIOs via the GPIO Matrix.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

4.2.2 Analog Signal Processing

This subsection describes components on the chip that sense and process real-world data.

4.2.2.1 SAR ADC

ESP32-S3 integrates two 12-bit SAR ADCs and supports measurements on 20 channels (analog-enabled pins).

For power-saving purpose, the ULP coprocessors in ESP32-S3 can also be used to measure voltage in sleep

modes. By using threshold settings or other methods, we can awaken the CPU from sleep modes.

Note:

Please note that the ADC2_CH… analog functions (see Table 2-8 Analog Functions) cannot be used with Wi-Fi simul-

taneously.

For more details, see ESP32-S3 Technical Reference Manual > Chapter On-Chip Sensors and Analog Signal

Processing.

Pin Assignment

The pins for the SAR ADC are multiplexed with GPIO1 ~ GPIO20, RTC_GPIO1 ~ RTC_GPIO20, Touch Sensor

interface, SPI interface, UART interface, and USB_D- and USB_D+ pins via IO MUX.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

Espressif Systems

58 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.2.2.2 Temperature Sensor

The temperature sensor generates a voltage that varies with temperature. The voltage is internally converted

via an ADC into a digital value.

The temperature sensor has a range of –40 °C to 125 °C. It is designed primarily to sense the temperature

changes inside the chip. The temperature value depends on factors such as microcontroller clock frequency

or I/O load. Generally, the chip’s internal temperature is higher than the ambient temperature.

For more details, see ESP32-S3 Technical Reference Manual > Chapter On-Chip Sensors and Analog Signal

Processing.

4.2.2.3 Touch Sensor

ESP32-S3 has 14 capacitive-sensing GPIOs, which detect variations induced by touching or approaching the

GPIOs with a finger or other objects. The low-noise nature of the design and the high sensitivity of the circuit

allow relatively small pads to be used. Arrays of pads can also be used, so that a larger area or more points

can be detected. The touch sensing performance can be further enhanced by the waterproof design and

digital filtering feature.

Note:

ESP32-S3 touch sensor has not passed the Conducted Susceptibility (CS) test for now, and thus has limited application

scenarios.

For more details, see ESP32-S3 Technical Reference Manual > Chapter On-Chip Sensors and Analog Signal

Processing.

Pin Assignment

The pins for touch sensor are multiplexed with GPIO1 ~ GPIO14, RTC_GPIO1 ~ RTC_GPIO14, SAR ADC interface,

and SPI interface via IO MUX.

For more information about the pin assignment, see Section 2.3 IO Pins and

ESP32-S3 Technical Reference Manual > Chapter IO MUX and GPIO Matrix.

Espressif Systems

59 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.3 Wireless Communication

This section describes the chip’s wireless communication capabilities, spanning radio technology, Wi-Fi,

Bluetooth, and 802.15.4.

4.3.1 Radio

This subsection describes the fundamental radio technology embedded in the chip that facilitates wireless

communication and data exchange.

4.3.1.1 2.4 GHz Receiver

The 2.4 GHz receiver demodulates the 2.4 GHz RF signal to quadrature baseband signals and converts them

to the digital domain with two high-resolution, high-speed ADCs. To adapt to varying signal channel

conditions, ESP32-S3 integrates RF filters, Automatic Gain Control (AGC), DC offset cancelation circuits, and

baseband filters.

4.3.1.2 2.4 GHz Transmitter

The 2.4 GHz transmitter modulates the quadrature baseband signals to the 2.4 GHz RF signal, and drives the

antenna with a high-powered CMOS power amplifier. The use of digital calibration further improves the linearity

of the power amplifier.

To compensate for receiver imperfections, additional calibration methods are built into the chip,

including:

• Carrier leakage compensation

• I/Q amplitude/phase matching

• Baseband nonlinearities suppression

• RF nonlinearities suppression

• Antenna matching

These built-in calibration routines reduce the cost and time to the market for your product, and eliminate the

need for specialized testing equipment.

4.3.1.3 Clock Generator

The clock generator produces quadrature clock signals of 2.4 GHz for both the receiver and the transmitter. All

components of the clock generator are integrated into the chip, including inductors, varactors, filters,

regulators, and dividers.

The clock generator has built-in calibration and self-test circuits. Quadrature clock phases and phase noise

are optimized on chip with patented calibration algorithms which ensure the best performance of the receiver

and the transmitter.

4.3.2 Wi-Fi

This subsection describes the chip’s Wi-Fi capabilities, which facilitate wireless communication at a high data

rate.

Espressif Systems

60 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

4.3.2.1 Wi-Fi Radio and Baseband

The ESP32-S3 Wi-Fi radio and baseband support the following features:

• 802.11b/g/n

• 802.11n MCS0-7 that supports 20 MHz and 40 MHz bandwidth

• 802.11n MCS32

• 802.11n 0.4 µs guard-interval

• Data rate up to 150 Mbps

• RX STBC (single spatial stream)

• Adjustable transmitting power

• Antenna diversity:

ESP32-S3 supports antenna diversity with an external RF switch. This switch is controlled by one or

more GPIOs, and used to select the best antenna to minimize the effects of channel imperfections.

4.3.2.2 Wi-Fi MAC

ESP32-S3 implements the full 802.11b/g/n Wi-Fi MAC protocol. It supports the Basic Service Set (BSS) STA

and SoftAP operations under the Distributed Control Function (DCF). Power management is handled

automatically with minimal host interaction to minimize the active duty period.

The ESP32-S3 Wi-Fi MAC applies the following low-level protocol functions automatically:

• Four virtual Wi-Fi interfaces

• Simultaneous Infrastructure BSS Station mode, SoftAP mode, and Station + SoftAP mode

• RTS protection, CTS protection, Immediate Block ACK

• Fragmentation and defragmentation

• TX/RX A-MPDU, TX/RX A-MSDU

• TXOP

• WMM

• GCMP, CCMP, TKIP, WAPI, WEP, BIP, WPA2-PSK/WPA2-Enterprise, and WPA3-PSK/WPA3-Enterprise

• Automatic beacon monitoring (hardware TSF)

• 802.11mc FTM

4.3.2.3 Networking Features

Users are provided with libraries for TCP/IP networking, ESP-WIFI-MESH networking, and other networking

protocols over Wi-Fi. TLS 1.2 support is also provided.

4.3.3 Bluetooth LE

This subsection describes the chip’s Bluetooth capabilities, which facilitate wireless communication for

low-power, short-range applications. ESP32-S3 includes a Bluetooth Low Energy subsystem that integrates a

Espressif Systems

61 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

4 Functional Description

hardware link layer controller, an RF/modem block and a feature-rich software protocol stack. It supports the

core features of Bluetooth 5 and Bluetooth mesh.

4.3.3.1 Bluetooth LE PHY

Bluetooth Low Energy radio and PHY in ESP32-S3 support:

• 1 Mbps PHY

• 2 Mbps PHY for high transmission speed and high data throughput

• Coded PHY for high RX sensitivity and long range (125 Kbps and 500 Kbps)

• Class 1 transmit power without external PA

• HW Listen before talk (LBT)

4.3.3.2 Bluetooth LE Link Controller

Bluetooth Low Energy Link Layer Controller in ESP32-S3 supports:

• LE advertising extensions, to enhance broadcasting capacity and broadcast more intelligent data

• Multiple advertisement sets

• Simultaneous advertising and scanning

• Multiple connections in simultaneous central and peripheral roles

• Adaptive frequency hopping and channel assessment

• LE channel selection algorithm #2

• Connection parameter update

• High duty cycle non-connectable advertising

• LE privacy 1.2

• LE data packet length extension

• Link layer extended scanner filter policies

• Low duty cycle directed advertising

• Link layer encryption

• LE Ping

Espressif Systems

62 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

5 Electrical Characteristics

5 Electrical Characteristics

5.1 Absolute Maximum Ratings

Stresses above those listed in Table 5-1 Absolute Maximum Ratings may cause permanent damage to the

device. These are stress ratings only and normal operation of the device at these or any other conditions

beyond those indicated in Section 5.2 Recommended Power Supply Characteristics is not implied. Exposure

to absolute-maximum-rated conditions for extended periods may affect device reliability.

Table 5-1. Absolute Maximum Ratings

Parameter Input power pins1 Ioutput

2

Description

Allowed input voltage

Cumulative IO output current

TST ORE

Storage temperature

Min

Max

Unit

–0.3

—

–40

3.6

1500

150

V

mA

°C

1 For more information on input power pins, see Section 2.5.1 Power Pins. 2 The product proved to be fully functional after all its IO pins were pulled high while being connected to ground for 24 consecutive hours at ambient tem-

perature of 25 °C.

5.2 Recommended Power Supply Characteristics

For recommended ambient temperature, see Section 1 ESP32-S3 Series Comparison.

Table 5-2. Recommended Power Characteristics

Parameter 1 VDDA, VDD3P3 VDD3P3_RTC 2 VDD_SPI (as input) — VDD3P3_CPU 3 IV DD

4

Description

Recommended input voltage

Recommended input voltage

Recommended input voltage

Cumulative input current

Min

Typ

Max

Unit

3.0

3.0

1.8

3.0

0.5

3.3

3.3

3.3

3.3

—

3.6

3.6

3.6

3.6

—

V

V

V

V

A

1 See in conjunction with Section 2.5 Power Supply. 2 If VDD3P3_RTC is used to power VDD_SPI (see Section 2.5.2 Power Scheme), the voltage drop on RSP I should be accounted for. See also Section 5.3 VDD_SPI Output Characteristics.

3 If writing to eFuses, the voltage on VDD3P3_CPU should not exceed 3.3 V as the

circuits responsible for burning eFuses are sensitive to higher voltages.

4 If you use a single power supply, the recommended output current is 500 mA or

more.

Espressif Systems

63 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

5 Electrical Characteristics

5.3 VDD_SPI Output Characteristics

Table 5-3. VDD_SPI Internal and Output Characteristics

Parameter Description 1

RSP I

ISP I

VDD_SPI powered by VDD3P3_RTC via RSP I for 3.3 V flash/PSRAM 2 Output current when VDD_SPI is powered by

Flash Voltage Regulator for 1.8 V flash/PSRAM

Typ

Unit

14

Ω

40

mA

1 See in conjunction with Section 2.5.2 Power Scheme. 2 VDD3P3_RTC must be more than VDD_flash_min + I_flash_max * RSP I ;

where

• VDD_flash_min – minimum operating voltage of flash/PSRAM • I_flash_max – maximum operating current of flash/PSRAM

5.4 DC Characteristics (3.3 V, 25 °C)

Table 5-4. DC Characteristics (3.3 V, 25 °C)

Parameter Description

Min

Typ

Max

CIN

VIH

VIL

IIH

IIL

2

VOH 2

VOL

IOH

IOL

RP U

RP D

VIH_nRST

VIL_nRST

Pin capacitance

High-level input voltage

Low-level input voltage

High-level input current

Low-level input current

High-level output voltage

Low-level output voltage High-level source current (VDD1 = 3.3 V, VOH

= 2.64 V, PAD_DRIVER = 3) Low-level sink current (VDD1 = 3.3 V, VOL = 0.495 V, PAD_DRIVER = 3)

Internal weak pull-up resistor

Internal weak pull-down resistor

— 0.75 × VDD1 –0.3

—

— 0.8 × VDD1 —

—

—

—

—

Chip reset release voltage (CHIP_PU voltage

is within the specified range)

0.75 × VDD1

2

— VDD1 + 0.3 — — 0.25 × VDD1 50 —

—

—

—

40

28

45

45

—

50

— 0.1 × VDD1

—

—

—

—

VDD1 + 0.3

Chip reset voltage (CHIP_PU voltage is within

the specified range)

–0.3

— 0.25 × VDD1

Unit

pF

V

V

nA

nA

V

V

mA

mA

kΩ

kΩ

V

V

1 VDD – voltage from a power pin of a respective power domain. 2 VOH and VOL are measured using high-impedance load.

Espressif Systems

64 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

5 Electrical Characteristics

5.5 ADC Characteristics

The measurements in this section are taken with an external 100 nF capacitor connected to the ADC, using DC

signals as input, and at an ambient temperature of 25 °C with disabled Wi-Fi.

Table 5-5. ADC Characteristics

Symbol DNL (Differential nonlinearity) 1 INL (Integral nonlinearity)

Sampling rate

Min

Max

–4

–8

—

4

8

100

Unit

LSB

LSB kSPS 2

1 To get better DNL results, you can sample multiple times and

apply a filter, or calculate the average value.

2 kSPS means kilo samples-per-second.

The calibrated ADC results after hardware calibration and software calibration are shown in Table 5-6. For

higher accuracy, you may implement your own calibration methods.

Table 5-6. ADC Calibration Results

Parameter

Description

Min

Max

Unit

Total error

ATTEN0, effective measurement range of 0 ~ 850

ATTEN1, effective measurement range of 0 ~ 1100

ATTEN2, effective measurement range of 0 ~ 1600

ATTEN3, effective measurement range of 0 ~ 2900

–5 –6 –10 –50

5

6

10

50

mV

mV

mV

mV

5.6 Current Consumption

5.6.1 RF Current Consumption in Active Mode

The current consumption measurements are taken with a 3.3 V supply at 25 °C of ambient temperature at the

RF port. All transmitters’ measurements are based on a 100% duty cycle.

Table 5-7. Wi-Fi Current Consumption Depending on RF Modes

Work Mode 1

Description

Peak (mA)

Active (RF working)

802.11b, 1 Mbps, @21 dBm

802.11g, 54 Mbps, @19 dBm

802.11n, HT20, MCS7, @18.5 dBm

802.11n, HT40, MCS7, @18 dBm

802.11b/g/n, HT20

802.11n, HT40

TX

RX

340

291

283

286

88

91

1 The CPU work mode: Single core runs 32-bit data access instructions at

80 MHz, the other core is in idle state.

5.6.2 Current Consumption in Other Modes

The measurements below are applicable to ESP32-S3 and ESP32-S3FH8. Since ESP32-S3R2, ESP32-S3R8,

ESP32-S3R8V, ESP32-S3R16V, and ESP32-S3FN4R2 are embedded with PSRAM, their current consumption

Espressif Systems

65 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

5 Electrical Characteristics

might be higher.

Table 5-8. Current Consumption in Modem-sleep Mode

Work mode

Frequency

(MHz)

Description

WAITI (Dual core in idle state)

Single core running 32-bit data access instructions, the

other core in idle state

Typ1 (mA)

13.2

16.2

Typ2 (mA)

18.8

21.8

Modem-sleep3

40

Dual core running 32-bit data access instructions

18.7

24.4

Single core running 128-bit data access instructions, the

other core in idle state

Dual core running 128-bit data access instructions

WAITI

Single core running 32-bit data access instructions, the

other core in idle state

19.9

25.4

23.0

22.0

28.8

36.1

28.4

42.6

80

Dual core running 32-bit data access instructions

33.1

47.3

Single core running 128-bit data access instructions, the

other core in idle state

Dual core running 128-bit data access instructions

WAITI

Single core running 32-bit data access instructions, the

other core in idle state

35.1

49.6

41.8

27.6

56.3

42.3

39.9

54.6

160

Dual core running 32-bit data access instructions

49.6

64.1

Single core running 128-bit data access instructions, the

other core in idle state

Dual core running 128-bit data access instructions

WAITI

Single core running 32-bit data access instructions, the

other core in idle state

240

Dual core running 32-bit data access instructions

Single core running 128-bit data access instructions, the

other core in idle state

54.4

69.2

66.7

32.9

81.1

47.6

51.2

65.9

66.2

72.4

81.3

87.9

Dual core running 128-bit data access instructions

91.7

107.9

1 Current consumption when all peripheral clocks are disabled. 2 Current consumption when all peripheral clocks are enabled. In practice, the current consumption might be

different depending on which peripherals are enabled.

3 In Modem-sleep mode, Wi-Fi is clock gated, and the current consumption might be higher when accessing

flash. For a flash rated at 80 Mbit/s, in SPI 2-line mode the consumption is 10 mA.

Table 5-9. Current Consumption in Low-Power Modes

Work mode

Description

Light-sleep1

Deep-sleep

VDD_SPI and Wi-Fi are powered down, and all GPIOs

are high-impedance.

RTC memory and RTC peripherals are powered up.

Typ (µA)

240

8

Espressif Systems

66 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

5 Electrical Characteristics

RTC memory is powered up. RTC peripherals are

powered down.

Power off

CHIP_PU is set to low level. The chip is shut down.

7

1

1 In Light-sleep mode, all related SPI pins are pulled up. For chips embedded with PSRAM, please add corresponding PSRAM consumption values, e.g., 140 µA for 8 MB Octal PSRAM (3.3 V), 200 µA for 8 MB Octal PSRAM (1.8 V) and 40 µA for 2 MB Quad PSRAM (3.3 V).

Espressif Systems

67 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

5 Electrical Characteristics

5.7 Reliability

Table 5-10. Reliability Qualifications

Test Item

Test Conditions

HTOL (High Temperature

Operating Life)

ESD (Electro-Static

Discharge Sensitivity)

Latch up

125 °C, 1000 hours

HBM (Human Body Mode)1 ± 2000 V CDM (Charge Device Mode)2 ± 1000 V Current trigger ± 200 mA

Voltage trigger 1.5 × VDDmax

Bake 24 hours @125 °C

Preconditioning

Moisture soak (level 3: 192 hours @30 °C, 60% RH)

IR reflow solder: 260 + 0 °C, 20 seconds, three times

Test Standard

JESD22-A108

JS-001

JS-002

JESD78

J-STD-020, JESD47,

JESD22-A113

TCT (Temperature Cycling

Test)

uHAST (Highly

–65 °C / 150 °C, 500 cycles

JESD22-A104

Accelerated Stress Test,

130 °C, 85% RH, 96 hours

JESD22-A118

unbiased)

HTSL (High Temperature

Storage Life)

LTSL (Low Temperature

Storage Life)

150 °C, 1000 hours

–40 °C, 1000 hours

JESD22-A103

JESD22-A119

1 JEDEC document JEP155 states that 500 V HBM allows safe manufacturing with a standard ESD control process. 2 JEDEC document JEP157 states that 250 V CDM allows safe manufacturing with a standard ESD control process.

Espressif Systems

68 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

6 RF Characteristics

This section contains tables with RF characteristics of the Espressif product.

The RF data is measured at the antenna port, where RF cable is connected, including the front-end loss. The front-end circuit is a 0 Ω resistor.

Devices should operate in the center frequency range allocated by regional regulatory authorities. The target

center frequency range and the target transmit power are configurable by software. See ESP RF Test Tool and

Test Guide for instructions.

Unless otherwise stated, the RF tests are conducted with a 3.3 V (±5%) supply at 25 ºC ambient temperature.

6.1 Wi-Fi Radio

Table 6-1. Wi-Fi RF Characteristics

Name

Description

Center frequency range of operating channel

2412 ~ 2484 MHz

Wi-Fi wireless standard

IEEE 802.11b/g/n

6.1.1 Wi-Fi RF Transmitter (TX) Specifications

Table 6-2. TX Power with Spectral Mask and EVM Meeting 802.11 Standards

Rate

802.11b, 1 Mbps

802.11b, 11 Mbps

802.11g, 6 Mbps

802.11g, 54 Mbps

802.11n, HT20, MCS0

802.11n, HT20, MCS7

802.11n, HT40, MCS0

802.11n, HT40, MCS7

Min

Typ

Max

(dBm)

(dBm)

(dBm)

—

—

—

—

—

—

—

—

21.0

21.0

20.5

19.0

19.5

18.5

19.5

18.0

—

—

—

—

—

—

—

—

Table 6-3. TX EVM Test1

Rate

802.11b, 1 Mbps, @21 dBm

802.11b, 11 Mbps, @21 dBm

802.11g, 6 Mbps, @20.5 dBm

802.11g, 54 Mbps, @19 dBm

802.11n, HT20, MCS0, @19.5 dBm

802.11n, HT20, MCS7, @18.5 dBm

Min

(dB)

Typ

(dB)

Limit

(dB)

— –24.5

— –24.5

—

–21.5

— –28.0

— –23.0

— –29.5

–10

–10

–5

–25

–5

–27

Cont’d on next page

Espressif Systems

69 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

Table 6-3 – cont’d from previous page

Rate

802.11n, HT40, MCS0, @19.5 dBm

802.11n, HT40, MCS7, @18 dBm

Min

(dB)

Typ

(dB)

— –23.0

— –29.5

Limit

(dB)

–5

–27

1 EVM is measured at the corresponding typical TX power provided in Table 6-2 TX Power with Spectral Mask and EVM Meeting 802.11

Standards above.

6.1.2 Wi-Fi RF Receiver (RX) Specifications

For RX tests, the PER (packet error rate) limit is 8% for 802.11b, and 10% for 802.11g/n.

Table 6-4. RX Sensitivity

Rate

802.11b, 1 Mbps

802.11b, 2 Mbps

802.11b, 5.5 Mbps

802.11b, 11 Mbps

802.11g, 6 Mbps

802.11g, 9 Mbps

802.11g, 12 Mbps

802.11g, 18 Mbps

802.11g, 24 Mbps

802.11g, 36 Mbps

802.11g, 48 Mbps

802.11g, 54 Mbps

802.11n, HT20, MCS0

802.11n, HT20, MCS1

802.11n, HT20, MCS2

802.11n, HT20, MCS3

802.11n, HT20, MCS4

802.11n, HT20, MCS5

802.11n, HT20, MCS6

802.11n, HT20, MCS7

802.11n, HT40, MCS0

802.11n, HT40, MCS1

802.11n, HT40, MCS2

802.11n, HT40, MCS3

802.11n, HT40, MCS4

802.11n, HT40, MCS5

802.11n, HT40, MCS6

802.11n, HT40, MCS7

Min

Typ

Max

(dBm)

(dBm)

(dBm)

— –98.4

— –95.4

— –93.0

— –88.6

— –93.2

—

—

–91.8

–91.2

— –88.6

— –86.0

— –82.4

—

—

–78.2

–76.5

— –92.6

—

–91.0

— –88.2

— –85.0

—

—

—

—

–81.8

–77.4

–75.8

–74.2

— –90.0

— –88.0

— –85.2

— –82.0

—

—

—

—

–79.0

–74.4

–72.8

–71.4

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

Espressif Systems

70 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

Table 6-5. Maximum RX Level

Rate

802.11b, 1 Mbps

802.11b, 11 Mbps

802.11g, 6 Mbps

802.11g, 54 Mbps

802.11n, HT20, MCS0

802.11n, HT20, MCS7

802.11n, HT40, MCS0

802.11n, HT40, MCS7

Min

Typ

Max

(dBm)

(dBm)

(dBm)

—

—

—

—

—

—

—

—

5

5

5

0

5

0

5

0

—

—

—

—

—

—

—

—

Table 6-6. RX Adjacent Channel Rejection

Rate

802.11b, 1 Mbps

802.11b, 11 Mbps

802.11g, 6 Mbps

802.11g, 54 Mbps

802.11n, HT20, MCS0

802.11n, HT20, MCS7

802.11n, HT40, MCS0

802.11n, HT40, MCS7

Min

(dB)

Typ

(dB)

Max

(dB)

—

—

—

—

—

—

—

—

35

35

31

20

31

16

25

11

—

—

—

—

—

—

—

—

6.2 Bluetooth LE Radio

Table 6-7. Bluetooth LE Frequency

Parameter

Min

Typ

Max

(MHz)

(MHz)

(MHz)

Center frequency of operating channel

2402

—

2480

6.2.1 Bluetooth LE RF Transmitter (TX) Specifications

Table 6-8. Transmitter Characteristics - Bluetooth LE 1 Mbps

Parameter

Description

RF transmit power

Carrier frequency offset and drift

RF power control range

Gain control step Max |fn| Max |f0 − fn| Max |fn − fn−5|

n=0, 1, 2, ..k

Min

–24.00

Typ

Max

0

20.00

—

—

—

—

3.00

2.50

2.00

1.39

—

—

—

—

Unit

dBm

dB

kHz

kHz

kHz

Cont’d on next page

Espressif Systems

71 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

Parameter

Modulation characteristics

Table 6-8 – cont’d from previous page

Description |f1 − f0| ∆ f 1avg Min ∆ f 2max (for at least 99.9% of all ∆ f 2max) ∆ f 2avg/∆ f 1avg ±2 MHz offset

Min

—

—

—

—

—

—

—

Typ

0.80

249.00

198.00

0.86

–37.00 –42.00 –44.00

Max

—

—

—

—

Unit

kHz

kHz

kHz

—

— dBm

— dBm

— dBm

Table 6-9. Transmitter Characteristics - Bluetooth LE 2 Mbps

Min

–24.00

Typ

Max

0

20.00

In-band spurious emissions

±3 MHz offset

±3 MHz offset

Parameter

Description

RF transmit power

Carrier frequency offset and drift

Modulation characteristics

RF power control range

n=0, 1, 2, ..k

Gain control step Max |fn| Max |f0 − fn| Max |fn − fn−5| |f1 − f0| ∆ f 1avg Min ∆ f 2max (for at least 99.9% of all ∆ f 2max) ∆ f 2avg/∆ f 1avg ±4 MHz offset

In-band spurious emissions

±5 MHz offset

±5 MHz offset

Parameter

Description

RF transmit power

Carrier frequency offset and drift

Modulation characteristics

RF power control range

n=0, 1, 2, ..k

Gain control step Max |fn| Max |f0 − fn| |fn − fn−3| |f0 − f3| ∆ f 1avg Min ∆ f 1max (for at least 99.9% of all∆ f 1max) ±2 MHz offset

In-band spurious emissions

±3 MHz offset

±3 MHz offset

—

—

—

—

—

—

—

—

—

—

—

3.00

2.50

1.90

1.40

1.10

499.00

416.00

0.89

–43.80 –45.80 –47.00

—

—

—

—

—

—

—

—

—

—

3.00

0.80

0.98

0.30

1.00

248.00

222.00

–37.00 –42.00 –44.00

— dBm

— dBm

— dBm

Unit

dBm

dB

kHz

kHz

kHz

kHz

kHz

kHz

—

Unit

dBm

dB

kHz

kHz

kHz

kHz

kHz

kHz

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

— dBm

— dBm

— dBm

Table 6-10. Transmitter Characteristics - Bluetooth LE 125 Kbps

Min

–24.00

Typ

Max

0

20.00

Espressif Systems

72 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

Table 6-11. Transmitter Characteristics - Bluetooth LE 500 Kbps

Parameter

Description

RF transmit power

Carrier frequency offset and drift

Modulation characteristics

RF power control range

n=0, 1, 2, ..k

Gain control step Max |fn| Max |f0 − fn| |fn − fn−3| |f0 − f3| ∆ f 2avg Min ∆ f 2max (for at least 99.9% of all ∆ f 2max) ±2 MHz offset

In-band spurious emissions

±3 MHz offset

±3 MHz offset

Min

–24.00

Typ

Max

0

20.00

—

—

—

—

—

—

—

—

—

—

3.00

0.70

0.90

0.85

0.34

213.00

196.00

–37.00 –42.00 –44.00

Unit

dBm

dB

kHz

kHz

kHz

kHz

kHz

kHz

—

—

—

—

—

—

—

— dBm

— dBm

— dBm

6.2.2 Bluetooth LE RF Receiver (RX) Specifications

Table 6-12. Receiver Characteristics - Bluetooth LE 1 Mbps

Parameter

Sensitivity @30.8% PER

Description

—

Maximum received signal @30.8% PER —

Co-channel C/I

Adjacent channel selectivity C/I

Image frequency

Adjacent channel to image frequency

Out-of-band blocking performance

F = F0 MHz

F = F0 + 1 MHz

F = F0 – 1 MHz

F = F0 + 2 MHz

F = F0 – 2 MHz

F = F0 + 3 MHz

F = F0 – 3 MHz F > F0 + 3 MHz F > F0 – 3 MHz

—

F = Fimage + 1 MHz F = Fimage – 1 MHz

30 MHz ~ 2000 MHz

2003 MHz ~ 2399 MHz

2484 MHz ~ 2997 MHz

3000 MHz ~ 12.75 GHz

Intermodulation

—

Min

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

Typ

–97.5

8

9

–3 –3 –28 –30 –31 –33 –32 –36

–32

–39 –31

–9 –19 –16 –5

–31

Max

Unit

— dBm

— dBm

—

—

—

—

—

—

—

—

—

—

—

—

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

— dBm

— dBm

— dBm

— dBm

— dBm

Espressif Systems

73 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

Table 6-13. Receiver Characteristics - Bluetooth LE 2 Mbps

Parameter

Sensitivity @30.8% PER

Description

—

Maximum received signal @30.8% PER —

Co-channel C/I

Adjacent channel selectivity C/I

Image frequency

Adjacent channel to image frequency

Out-of-band blocking performance

F = F0 MHz

F = F0 + 2 MHz

F = F0 – 2 MHz

F = F0 + 4 MHz

F = F0 – 4 MHz

F = F0 + 6 MHz

F = F0 – 6 MHz F > F0 + 6 MHz F > F0 – 6 MHz

—

F = Fimage + 2 MHz F = Fimage – 2 MHz

30 MHz ~ 2000 MHz

2003 MHz ~ 2399 MHz

2484 MHz ~ 2997 MHz

3000 MHz ~ 12.75 GHz

Intermodulation

—

Min

Typ

Max

Unit

— –93.5

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

—

3

10

–8 –5 –31 –33 –37 –37 –40 –40

–31

–37 –8

–16 –20 –16 –16

–30

— dBm

— dBm

—

—

—

—

—

—

—

—

—

—

—

—

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

— dBm

— dBm

— dBm

— dBm

— dBm

Table 6-14. Receiver Characteristics - Bluetooth LE 125 Kbps

Parameter

Sensitivity @30.8% PER

Description

—

Maximum received signal @30.8% PER —

Co-channel C/I

Adjacent channel selectivity C/I

Image frequency

Adjacent channel to image frequency

F = F0 MHz

F = F0 + 1 MHz

F = F0 – 1 MHz

F = F0 + 2 MHz

F = F0 – 2 MHz

F = F0 + 3 MHz

F = F0 – 3 MHz F > F0 + 3 MHz F > F0 – 3 MHz

—

F = Fimage + 1 MHz F = Fimage – 1 MHz

Min

Typ

Max

Unit

— –104.5

—

—

—

—

—

—

—

—

—

—

—

—

—

8

6

–6 –5 –32 –39 –35 –45 –35 –48

–35

–49 –32

— dBm

— dBm

—

—

—

—

—

—

—

—

—

—

—

—

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

Espressif Systems

74 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

6 RF Characteristics

Table 6-15. Receiver Characteristics - Bluetooth LE 500 Kbps

Parameter

Sensitivity @30.8% PER

Description

—

Maximum received signal @30.8% PER —

Co-channel C/I

Adjacent channel selectivity C/I

Image frequency

Adjacent channel to image frequency

F = F0 MHz

F = F0 + 1 MHz

F = F0 – 1 MHz

F = F0 + 2 MHz

F = F0 – 2 MHz

F = F0 + 3 MHz

F = F0 – 3 MHz F > F0 + 3 MHz F > F0 – 3 MHz

—

F = Fimage + 1 MHz F = Fimage – 1 MHz

Min

—

—

—

—

—

—

—

—

—

—

—

—

—

—

Typ

–101

8

4

–5 –5 –28 –36 –36 –38 –37 –41

–37

–44 –28

Max

Unit

— dBm

— dBm

—

—

—

—

—

—

—

—

—

—

—

—

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

dB

Espressif Systems

75 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

7 Packaging

7 Packaging

• For information about tape, reel, and product marking, please refer to

ESP32-S3 Chip Packaging Information.

• The pins of the chip are numbered in anti-clockwise order starting from Pin 1 in the top view. For pin

numbers and pin names, see also Figure 2-1 ESP32-S3 Pin Layout (Top View).

• The recommended land pattern source file (asc) is available for download. You can import the file with

software such as PADS and Altium Designer.

• All ESP32-S3 chip variants have identical land pattern (see Figure 7-1) except ESP32-S3FH4R2 has a

bigger EPAD (see Figure 7-2). The source file (asc) may be adopted for ESP32-S3FH4R2 by altering the

size of the EPAD (see dimensions D2 and E2 in Figure 7-2).

Figure 7-1. QFN56 (7×7 mm) Package

Espressif Systems

76 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Pin 1Pin 2Pin 3Pin 1Pin 2Pin 37 Packaging

Figure 7-2. QFN56 (7×7 mm) Package (Only for ESP32-S3FH4R2)

Espressif Systems

77 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

(cid:18)(cid:21)(cid:1239)(cid:2801)(cid:3910)(cid:5522)(cid:3374)(cid:54)(cid:44)(cid:42)(cid:49)(cid:36)(cid:55)(cid:56)(cid:53)(cid:40)(cid:3)(cid:36)(cid:53)(cid:40)(cid:36)(cid:7420)(cid:4299)(cid:1931)(cid:5107) FOREHOPE ELECTRONIC(cid:41)(cid:50)(cid:53)(cid:40)(cid:43)(cid:50)(cid:51)(cid:40)(cid:3)(cid:38)(cid:50)(cid:49)(cid:41)(cid:44)(cid:39)(cid:40)(cid:49)(cid:55)(cid:44)(cid:36)(cid:47)(cid:16)(cid:37)(cid:55)(cid:75)(cid:76)(cid:86)(cid:3)(cid:71)(cid:82)(cid:70)(cid:88)(cid:80)(cid:72)(cid:81)(cid:87)(cid:3)(cid:68)(cid:81)(cid:71)(cid:3)(cid:76)(cid:87)(cid:86)(cid:3)(cid:76)(cid:81)(cid:73)(cid:82)(cid:85)(cid:80)(cid:68)(cid:87)(cid:76)(cid:82)(cid:81)(cid:3)(cid:75)(cid:72)(cid:85)(cid:72)(cid:76)(cid:81)(cid:3)(cid:68)(cid:85)(cid:72)(cid:3)(cid:87)(cid:75)(cid:72)(cid:3)(cid:83)(cid:85)(cid:82)(cid:83)(cid:72)(cid:85)(cid:87)(cid:92)(cid:3)(cid:82)(cid:73)(cid:3)(cid:41)(cid:82)(cid:85)(cid:72)(cid:75)(cid:82)(cid:83)(cid:72)(cid:3)(cid:68)(cid:81)(cid:71)(cid:3)(cid:68)(cid:79)(cid:79)(cid:3)(cid:88)(cid:81)(cid:68)(cid:88)(cid:87)(cid:75)(cid:82)(cid:85)(cid:76)(cid:93)(cid:72)(cid:71)(cid:3)(cid:88)(cid:86)(cid:72)(cid:3)(cid:68)(cid:81)(cid:71)(cid:3)(cid:85)(cid:72)(cid:83)(cid:85)(cid:82)(cid:71)(cid:88)(cid:70)(cid:87)(cid:76)(cid:82)(cid:81)(cid:3)(cid:68)(cid:85)(cid:72)(cid:3)(cid:83)(cid:85)(cid:82)(cid:75)(cid:76)(cid:69)(cid:76)(cid:87)(cid:72)(cid:71)(cid:17)(cid:54)(cid:75)(cid:68)(cid:90)(cid:81)(cid:3)(cid:21)(cid:19)(cid:21)(cid:20)(cid:17)(cid:20)(cid:19)(cid:17)(cid:20)(cid:21)(cid:51)(cid:68)(cid:71)(cid:85)(cid:68)(cid:76)(cid:70)(cid:3)(cid:21)(cid:19)(cid:21)(cid:20)(cid:17)(cid:20)(cid:19)(cid:17)(cid:20)(cid:21)(cid:52)(cid:41)(cid:49)(cid:58)(cid:37)(cid:16)(cid:26)(cid:104)(cid:26)(cid:16)(cid:24)(cid:25)(cid:47)(cid:16)(cid:37)(cid:11)(cid:51)(cid:19)(cid:17)(cid:23)(cid:3)(cid:3)(cid:55)(cid:19)(cid:17)(cid:27)(cid:24)(cid:12)(cid:16)(cid:16)(cid:36)(cid:20)(cid:24)(cid:29)(cid:20)(cid:20)(cid:3)(cid:3)(cid:50)(cid:41)(cid:3)(cid:3)(cid:21)(cid:103)S u b m

i t

D o c u m e n t a t i o n F e e d b a c k

E s p r e s s i f S y s t e m s

7 8

E S P 3 2
S 3 S e r i e s D a t a s h e e t

v 2 . 0

Appendix A – ESP32-S3 Consolidated Pin Overview

sar_i2c_scl_0 sar_i2c_sda_0 sar_i2c_scl_1 sar_i2c_sda_1

RTC Function

3

0

IE IE

At Reset

After Reset

Pin Settings

IE IE IE IE IE IE

Pin Providing Power

IE, WPU IE IE IE

IE, WPU IE IE IE

VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD_SPI

RTC_GPIO0 RTC_GPIO1 RTC_GPIO2 RTC_GPIO3 RTC_GPIO4 RTC_GPIO5 RTC_GPIO6 RTC_GPIO7 RTC_GPIO8 RTC_GPIO9 RTC_GPIO10 RTC_GPIO11 RTC_GPIO12 RTC_GPIO13 RTC_GPIO14

VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC VDD3P3_RTC

Pin Name LNA_IN VDD3P3 VDD3P3 CHIP_PU GPIO0 GPIO1 GPIO2 GPIO3 GPIO4 GPIO5 GPIO6 GPIO7 GPIO8 GPIO9 GPIO10 GPIO11 GPIO12 GPIO13 GPIO14 VDD3P3_RTC XTAL_32K_P XTAL_32K_N GPIO17 GPIO18 GPIO19 GPIO20 GPIO21 SPICS1 VDD_SPI SPIHD SPIWP SPICS0 SPICLK SPIQ SPID SPICLK_N SPICLK_P GPIO33 GPIO34 GPIO35 GPIO36 GPIO37 GPIO38 MTCK MTDO VDD3P3_CPU MTDI MTMS U0TXD U0RXD GPIO45 GPIO46 XTAL_N XTAL_P VDDA VDDA GND

Pin No. 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57

For details, see Section 2 Pins. Regarding highlighted cells, see Section 2.3.4 Restrictions for GPIOs and RTC_GPIOs.
Pin Type Analog Power Power Analog IO IO IO IO IO IO IO IO IO IO IO IO IO IO IO Power IO IO IO IO IO IO IO IO Power IO IO IO IO IO IO IO IO IO IO IO IO IO IO IO IO Power IO IO IO IO IO IO Analog Analog Power Power Power

VDD_SPI VDD_SPI VDD_SPI VDD_SPI VDD_SPI VDD_SPI VDD_SPI / VDD3P3_CPU VDD_SPI / VDD3P3_CPU VDD_SPI / VDD3P3_CPU VDD_SPI / VDD3P3_CPU VDD_SPI / VDD3P3_CPU VDD_SPI / VDD3P3_CPU VDD_SPI / VDD3P3_CPU VDD3P3_CPU VDD3P3_CPU VDD3P3_CPU

IE, WPU IE, WPU IE, WPU IE, WPU IE, WPU IE, WPU IE IE IE IE IE IE IE IE IE* IE

RTC_GPIO15 RTC_GPIO16 RTC_GPIO17 RTC_GPIO18 RTC_GPIO19 RTC_GPIO20 RTC_GPIO21

VDD3P3_CPU VDD3P3_CPU VDD3P3_CPU VDD3P3_CPU VDD3P3_CPU VDD3P3_CPU

IE, WPU IE, WPU IE, WPU IE, WPU IE, WPU IE, WPU IE IE

IE IE IE, WPU IE, WPU IE, WPD IE, WPD

IE, WPU IE, WPU IE, WPD IE, WPD

USB_PU

USB_PU

IE, WPU

IE, WPU

Analog Function

IO MUX Function

0

1

0

Type

1

Type

2

Type

3

Type

4

Type

TOUCH1 TOUCH2 TOUCH3 TOUCH4 TOUCH5 TOUCH6 TOUCH7 TOUCH8 TOUCH9 TOUCH10 TOUCH11 TOUCH12 TOUCH13 TOUCH14

XTAL_32K_P XTAL_32K_N

USB_D- USB_D+

ADC1_CH0 ADC1_CH1 ADC1_CH2 ADC1_CH3 ADC1_CH4 ADC1_CH5 ADC1_CH6 ADC1_CH7 ADC1_CH8 ADC1_CH9 ADC2_CH0 ADC2_CH1 ADC2_CH2 ADC2_CH3

ADC2_CH4 ADC2_CH5 ADC2_CH6 ADC2_CH7 ADC2_CH8 ADC2_CH9

GPIO0 GPIO1 GPIO2 GPIO3 GPIO4 GPIO5 GPIO6 GPIO7 GPIO8 GPIO9 GPIO10 GPIO11 GPIO12 GPIO13 GPIO14

GPIO15 GPIO16 GPIO17 GPIO18 GPIO19 GPIO20 GPIO21 SPICS1

SPIHD SPIWP SPICS0 SPICLK SPIQ SPID SPI CLK_N_DIFF SPI CLK_P_DIFF GPIO33 GPIO34 GPIO35 GPIO36 GPIO37 GPIO38 MTCK MTDO

MTDI MTMS U0TXD U0RXD GPIO45 GPIO46

I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T

I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T O/T

I1/O/T I1/O/T O/T O/T I1/O/T I1/O/T O/T O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I1 O/T

I1 I1 O I1 I/O/T I/O/T

GPIO0 GPIO1 GPIO2 GPIO3 GPIO4 GPIO5 GPIO6 GPIO7 GPIO8 GPIO9 GPIO10 GPIO11 GPIO12 GPIO13 GPIO14

GPIO15 GPIO16 GPIO17 GPIO18 GPIO19 GPIO20 GPIO21 GPIO26

GPIO27 GPIO28 GPIO29 GPIO30 GPIO31 GPIO32 GPIO48 GPIO47 GPIO33 GPIO34 GPIO35 GPIO36 GPIO37 GPIO38 GPIO39 GPIO40

GPIO41 GPIO42 GPIO43 GPIO44 GPIO45 GPIO46

I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T

I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T

I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T

I/O/T I/O/T I/O/T I/O/T I/O/T I/O/T

SUBSPICS1 SUBSPIHD SUBSPICS0 SUBSPID SUBSPICLK SUBSPIQ SUBSPIWP

O/T I1/O/T O/T I1/O/T O/T I1/O/T I1/O/T

FSPIHD FSPICS0 FSPID FSPICLK FSPIQ FSPIWP

I1/O/T I1/O/T I1/O/T I1/O/T I1/O/T I1/O/T

CLK_OUT3 CLK_OUT2 CLK_OUT1

O O O

SUBSPIHD SUBSPICS0 SUBSPID SUBSPICLK SUBSPIQ SUBSPIWP SUBSPICS1

I1/O/T O/T I1/O/T O/T I1/O/T I1/O/T O/T

SPIIO4 SPIIO5 SPIIO6 SPIIO7 SPIDQS

I1/O/T I1/O/T I1/O/T I1/O/T I0/O/T

FSPIIO4 FSPIIO5 FSPIIO6 FSPIIO7 FSPIDQS

U0RTS U0CTS U1TXD U1RXD U1RTS U1CTS

SUBSPI CLK_N_DIFF SUBSPI CLK_P_DIFF FSPIHD FSPICS0 FSPID FSPICLK FSPIQ FSPIWP CLK_OUT3 CLK_OUT2

CLK_OUT1

CLK_OUT1 CLK_OUT2

I1/O/T I1/O/T I1/O/T I1/O/T O/T

O I1 O I1 O I1

O/T O/T I1/O/T I1/O/T I1/O/T I1/O/T I1/O/T I1/O/T O O

O

O O

i

A p p e n d x A – E S P 3 2
S 3 C o n s o

l i

i

d a t e d P n O v e r v e w

i

Related Documentation and Resources

Related Documentation and Resources

Related Documentation

• ESP32-S3 Technical Reference Manual – Detailed information on how to use the ESP32-S3 memory and periph-

erals.

• ESP32-S3 Hardware Design Guidelines – Guidelines on how to integrate the ESP32-S3 into your hardware product. • ESP32-S3 Series SoC Errata – Descriptions of known errors in ESP32-S3 series of SoCs. • Certificates

https://espressif.com/en/support/documents/certificates • ESP32-S3 Product/Process Change Notifications (PCN)

https://espressif.com/en/support/documents/pcns?keys=ESP32-S3

• ESP32-S3 Advisories – Information on security, bugs, compatibility, component reliability.

https://espressif.com/en/support/documents/advisories?keys=ESP32-S3

• Documentation Updates and Update Notification Subscription

https://espressif.com/en/support/download/documents

Developer Zone

• ESP-IDF Programming Guide for ESP32-S3 – Extensive documentation for the ESP-IDF development framework. • ESP-IDF and other development frameworks on GitHub.

https://github.com/espressif

• ESP32 BBS Forum – Engineer-to-Engineer (E2E) Community for Espressif products where you can post questions,

share knowledge, explore ideas, and help solve problems with fellow engineers.

https://esp32.com/

• The ESP Journal – Best Practices, Articles, and Notes from Espressif folks.

https://blog.espressif.com/

• See the tabs SDKs and Demos, Apps, Tools, AT Firmware. https://espressif.com/en/support/download/sdks-demos

Products

• ESP32-S3 Series SoCs – Browse through all ESP32-S3 SoCs. https://espressif.com/en/products/socs?id=ESP32-S3

• ESP32-S3 Series Modules – Browse through all ESP32-S3-based modules.

https://espressif.com/en/products/modules?id=ESP32-S3

• ESP32-S3 Series DevKits – Browse through all ESP32-S3-based devkits.

https://espressif.com/en/products/devkits?id=ESP32-S3

• ESP Product Selector – Find an Espressif hardware product suitable for your needs by comparing or applying filters.

https://products.espressif.com/#/product-selector?language=en

Contact Us

• See the tabs Sales Questions, Technical Enquiries, Circuit Schematic & PCB Design Review, Get Samples

(Online stores), Become Our Supplier, Comments & Suggestions.

https://espressif.com/en/contact-us/sales-questions

Espressif Systems

79 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Revision History

Revision History

Date

Version

Release notes

2025-04-24

v2.0

2024-09-11

v1.9

• Updated the status of ESP32-S3R8V to End of Life • Updated the CoreMark® score in Section CPU and Memory • Updated Figure 4.1.2 Memory Organization in Section 4-1 Address Map-

ping Structure

• Updated the temperature sensor’s measurement range in Section 4.2.2.2

Temperature Sensor

• Added some notes in Chapter 6 RF Characteristics • Updated the source file link for the recommended land pattern in Chapter

7 Packaging

• Updated descriptions on the title page • Updated feature descriptions in Section Features and adjusted the format • Updated the pin introduction in Section 2.2 Pin Overview and adjusted

the format

• Updated descriptions in Section 2.3 IO Pins, and divided Section RTC and Analog Pin Functions into Section 2.3.3 Analog Functions and Section

2.3.2 RTC Functions

• Updated Section Strapping Pins to Section 3 Boot Configurations • Adjusted the structure and section order in Section 4 Functional Descrip- tion, deleted Section Peripheral Pin Configurations, and added the Pin

Assignment part in each subsection in Section 4.2 Peripherals

• Added chip variant ESP32-S3R16V and updated related information • Added the second and third table notes in Table 1-1 ESP32-S3 Series

2023-11-24

v1.8

Comparison

• Updated Section 3.1 Chip Boot Mode Control • Updated Section 5.5 ADC Characteristics • Other minor updates

Cont’d on next page

Espressif Systems

80 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Revision History

Date

Version

Release notes

Cont’d from previous page

2023-06

v1.7

• Removed the sample status for ESP32-S3FH4R2 • Updated Figure ESP32-S3 Functional Block Diagram and Figure 4-2 Com-

ponents and Power Domains

• Added the predefined settings at reset and after reset for GPIO20 in Table

2-1 Pin Overview

• Updated notes for Table 2-4 IO MUX Pin Functions • Updated the clock name “FOSC_CLK” to “RC_FAST_CLK” in Section

4.1.3.5 Power Management Unit (PMU)

• Updated descriptions in Section 4.2.1.5 Serial Peripheral Interface (SPI)

and Section 4.1.4.3 RSA Accelerator

• Other minor updates

• Improved the content in the following sections:

– Section Product Overview

– Section 2 Pins

– Section 4.1.3.5 Power Management Unit (PMU)

– Section 4.2.1.5 Serial Peripheral Interface (SPI)

– Section 5.1 Absolute Maximum Ratings

– Section 5.2 Recommended Power Supply Characteristics

– Section 5.3 VDD_SPI Output Characteristics

2023-02

v1.6

– Section 5.5 ADC Characteristics

• Added Appendix A • Updated the notes in Section 1 ESP32-S3 Series Comparison and Section

7 Packaging

• Updated the effective measurement range in Table 5-5 ADC Characteris-

tics

• Updated the Bluetooth maximum transmit power • Other minor updates

• Removed the ”External PA is supported” feature from Section Features • Updated the ambient temperature for ESP32-S3FH4R2 from –40 ∼ 105

2022-12

v1.5

°C to –40 ∼ 85 °C

• Added two notes in Section 7

2022-11

v1.4

• Added the package information for ESP32-S3FH4R2 in Section 7 • Added ESP32-S3 Series SoC Errata in Section • Other minor updates

Cont’d on next page

Espressif Systems

81 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Revision History

Date

Version

Release notes

Cont’d from previous page

2022-09

v1.3

2022-07

v1.2

2022-04

v1.1

2022-01

v1.0

• Added a note about the maximum ambient temperature of R8 series chips

to Table 1-1 and Table 5-2

• Added information about power-up glitches for some pins in Section 2.2 • Added the information about VDD3P3 power pins to Table 2.2 and Sec-

tion 2.5.2

• Updated section 4.3.3.1 • Added the fourth note in Table 2-1 • Updated the minimum and maximum values of Bluetooth LE RF transmit

power in Section 6.2.1

• Other minor updates

• Updated description of ROM code printing in Section 3 • Updated Figure ESP32-S3 Functional Block Diagram • Update Section 5.6 • Deleted the hyperlinks in Application

• Synchronized eFuse size throughout • Updated pin description in Table 2-1 • Updated SPI resistance in Table 5-3 • Added information about chip ESP32-S3FH4R2

• Added wake-up sources for Deep-sleep mode • Added Table 3-4 for default configurations of VDD_SPI • Added ADC calibration results in Table 5-5 • Added typical values when all peripherals and peripheral clocks are en-

abled to Table 5-8

• Added more descriptions of modules/peripherals in Section 4 • Updated Figure ESP32-S3 Functional Block Diagram • Updated JEDEC specification • Updated Wi-Fi RF data in Section 5.6 • Updated temperature for ESP32-S3R8 and ESP32-S3R8V • Updated description of Deep-sleep mode in Table 5-9 • Updated wording throughout

2021-10-12

v0.6.1

Updated text description

Cont’d on next page

Espressif Systems

82 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Revision History

Date

Version

Release notes

Cont’d from previous page

2021-09-30

v0.6

• Updated to chip revision 1 by swapping pin 53 and pin 54 (XTAL_P and

XTAL_N)

• Updated Figure ESP32-S3 Functional Block Diagram • Added CoreMark score in section Features • Updated Section 3 • Added data for cumulative IO output current in Table 5-1 • Added data for Modem-sleep current consumption in Table 5-8 • Updated data in section 5.6, 6.1, and 6.2 • Updated wording throughout

• Added “for chip revision 0” on cover, in footer and watermark to indicate that the current and previous versions of this datasheet are for chip ver-

2021-07-19

v0.5.1

sion 0

• Corrected a few typos

2021-07-09

v0.5

Preliminary version

Espressif Systems

83 Submit Documentation Feedback

ESP32-S3 Series Datasheet v2.0

Disclaimer and Copyright Notice Information in this document, including URL references, is subject to change without notice.

ALL THIRD PARTY’S INFORMATION IN THIS DOCUMENT IS PROVIDED AS IS WITH NO WARRANTIES TO ITS AUTHENTICITY AND ACCURACY.

NO WARRANTY IS PROVIDED TO THIS DOCUMENT FOR ITS MERCHANTABILITY, NON-INFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, NOR DOES ANY WARRANTY OTHERWISE ARISING OUT OF ANY PROPOSAL, SPECIFICATION OR SAMPLE.

All liability, including liability for infringement of any proprietary rights, relating to use of information in this document is disclaimed. No licenses express or implied, by estoppel or otherwise, to any intellectual property rights are granted herein.

The Wi-Fi Alliance Member logo is a trademark of the Wi-Fi Alliance. The Bluetooth logo is a registered trademark of Bluetooth SIG.

All trade names, trademarks and registered trademarks mentioned in this document are property of their respective owners, and are hereby acknowledged.