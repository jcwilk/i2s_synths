9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

ESP32-S3-Zero

From Waveshare Wiki

Jump to: navigation, search

Overview

Introduction

ESP32-S3-Zero

ESP32-S3-Zero (without pin header) and ESP32-S3-Zero-M (with pin

header) are tiny in size with castellated holes, making them easy to

integrate into other host boards. ESP32-S3-Zero comes with an onboard

Type-C USB connector, which exposes most of the unused pins in a small

(https://www.waveshare.com/esp32-

form factor. It adopts ESP32-FH4R2 as a System-on-Chip (SoC) that

integrates low-power WiFi and BLE5.0 with 4MB Flash and 2MB

PSRAM. In addition, there are hardware encryption accelerator, RNG,

HMAC and Digital Signature modules to meet the safety requirements of

s3-zero.htm)

ESP32-S3FH4R2

Type C USB

IoT and provide rich peripheral interfaces. Moreover, its low-power working mode supports most application

scenarios such as IoT, mobile devices, wearable electronic devices, and smart homes.

Features

Equipped with Xtensa® 32-bit LX7 dual-core processor, up to 240MHz main frequency. Supports 2.4GHz Wi-Fi (802.11 b/g/n) and Bluetooth® 5 (LE). Built-in 512KB of SRAM and 384KB ROM, onboard 4MB Flash memory and 2MB PSRAM. Castellated module and onboard ceramic antenna, allow soldering direct to carrier boards. Supports flexible clock, module power supply independent setting, and other controls to realize low power consumption in different scenarios. Integrated with USB serial port full-speed controller, 24 × GPIO pins allow flexible configuring pin functions. 4 × SPI, 2 × I2C, 3 × UART, 2 × I2S, 2 × ADC, etc.

Hardware Description

When using ESP32-S3-Zero with daughterboards, please avoid covering the ceramic antenna with PCB boards, metal, or plastic components. In ESP32-S3-Zero, GPIO33 to GPIO37 pins are not exposed; these pins are used for Octal PSRAM.

ESP32-S3-Zero uses GPIO21 to connect with WS2812 RGB LED. Please refer to this link (https://files.wave

share.com/wiki/ESP32-S3-Zero/XL-0807RGBC-WS2812B.pdf) for WS2812 specifications. ESP32-S3-Zero does not employ a USB to UART chip. When flashing firmware, press and hold the BOOT button (GPIO0) and then connect the Type-C cable.

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

1/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

The "TX" and "RX" markings on the board indicate the default UART0 pins for ESP32-S3-Zero. Specifically, TX is GPIO43, and RX is GPIO44.

Hardware Connection

Press the BOOT (GPIO0) key before connecting the Type-C cable each time download the firmware.

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

2/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

Input 3.7V~6V for the castellated hole with 5V silkscreen when connecting the external power.

Pinout

(/wiki/File:ESP32-S3-Zero-details-inter.jpg)

Dimensions

(/wiki/File:ESP32-S3-

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

3/14

9/3/25, 7:04 PM

Zero_02.jpg)

Environment Setting

ESP32-S3-Zero - Waveshare Wiki

The software framework for ESP32 series development boards is completed, and you can use CircuitPython,

MicroPython, and C/C++ (Arduino, ESP-IDF) for rapid prototyping of product development. Here's a brief

introduction to these three development approaches:

CircuitPython is a programming language designed to simplify coding tests and learning on low-cost microcontroller boards. It is an open-source derivative of the MicroPython programming language, primarily aimed at students and beginners. CircuitPython development and maintenance are supported by Adafruit Industries.

You can refer to development documentation (https://docs.circuitpython.org/en/latest/shared-bindings/i

ndex.html) for CircuitPython-related applications development.

The GitHub library (https://github.com/adafruit/Adafruit_CircuitPython_Bundle) for CircuitPython allows for recompilation for custom development.

MicroPython is an efficient implementation of the Python 3 programming language. It includes a small subset of the Python standard library and has been optimized to run on microcontrollers and resource- constrained environments.

You can refer to development documentation (https://docs.micropython.org/en/latest/) for MicroPython-related application development.

The GitHub library (https://github.com/micropython/micropython) for MicroPython allows for recompilation for custom development.

The official libraries and support from Espressif Systems for C/C++ development make it convenient for rapid installation. For Windows 10 system:

Users can select Arduino (https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)

Or Visual Studio Code (ESP-IDF (https://docs.espressif.com/projects/esp-idf/en/stable/esp32s2/get-star

ted/index.html)) as their Integrated Development Environment (IDE).

Mac and Linux operating system users can refer to the official instructions (https://docs.espressif.com/project

s/esp-idf/en/latest/esp32/get-started/index.html). Additionally, you can find development manuals for ESP32 series Arduino and ESP-IDF development to assist you in the process.

Arduino

Download and install Arduino IDE (https://www.arduino.cc/en/software).

Install ESP32 on the Arduino IDE as shown below, and you can refer to this link (https://docs.espressif.com/

projects/arduino-esp32/en/latest/installing.html). Fill in the following link in the Additional Boards Manager URLs section of the Settings interface under File -> Preferences and save.

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_

index.json

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

4/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

(/wiki/File:ESP32-C3-Zero_-05.jpg)

Search esp32 on Board Manager to install, and restart Arduino IDE to take effect.

ESP32-S3-Zero does not have a USB to UART chip mounted, you need to use the USB of the ESP32-S3 as
the download interface and the Log print interface, and you need to enable the USB CDC when using the

Arduino IDE.

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

5/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

(/wiki/File:ESP32-C3-Zero_-06.jpg)

MicroPython

Download and install the latest Thonny (https://thonny.org/), open Thonny IDE -> Configure interpreter... as shown below:
https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

6/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

(/wiki/File:CircuitPython_Thonny06.jpg)

Press the BOOT key on the board, connect it to the USB cable, find the device manager or the corresponding
COM port, and download or run the demo. For more details, you can see #Hardware Connection.

Select the ESP32 series online MPY firmware to download according to the following steps. The Flash content of the development board will be cleared before downloading, and the whole download process lasts about 1 minute.
https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

7/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

(/wiki/File:CircuitPython_Thonny03.jpg)

Download the local firmware using flash_download_tool according to the following figure. Please note, the application firmware address for ESP32-S3 is 0x10000, "partition_tables.bin" address is 0x8000, and "bootloader.bin" address is 0x0. You can choose either step 3 or step 4, but we recommend to choose step 4.
https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

8/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

(/wiki/File:ESP32-C3-Zero_09.jpg)

Please refer to MicroPython Documentation (https://github.com/micropython/micropython/releases/tag/v1.18)
and Release Note (https://github.com/micropython/micropython/releases/tag/v1.18) to program.

Sample Demo

Please refer to arduino-esp32 (https://github.com/espressif/arduino-esp32) for Arduino sample demo or File -

examples on the Arduino IDE.

Please refer to MicroPython (https://docs.micropython.org/en/latest/) development documentation and sample demo for mpy example.

Resource

Document

Schematic (https://files.waveshare.com/wiki/ESP32-S3-Zero/ESP32-S3-Zero-Sch.pdf)

MicroPython development documentation (https://docs.micropython.org/en/latest/)

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

9/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

ESP32 Arduino Core's documentation (https://docs.espressif.com/projects/arduino-esp32/en/latest/index.htm

l)

arduino-esp32 (https://github.com/espressif/arduino-esp32)

ESP-IDF (https://github.com/espressif/esp-idf)

Demo

Sample Demo (https://files.waveshare.com/wiki/ESP32-S3-Zero/Esp32-s3-zero-code.zip)

Software

Sscom serial port assistant (https://files.waveshare.com/upload/b/b3/Sscom5.13.1.zip)

Thonny Python IDE (https://thonny.org/)

Arduino IDE (https://www.arduino.cc/en/software)

mpy firmware (https://files.waveshare.com/wiki/ESP32-S3-Zero/Esp32-s3-zero-mpy.zip)

Datasheet

ESP32-S3 Datasheet (https://www.espressif.com.cn/en/support/documents/technical-documents?keys&field_

type_tid%5B%5D=842)

WS2812B (https://files.waveshare.com/wiki/ESP32-C3-Zero/XL-0807RGBC-WS2812B%20(1).pdf)

3D Model

3D Model (https://files.waveshare.com/wiki/ESP32-S3-Zero/manual/ESP32-S3-Zero%20V2.step)

Dimensional Drawing (https://www.waveshare.net/w/upload/3/30/ESP32-S3-Zero-2D-size.jpg)

FAQ

Question:Can the ESP32-S3-Zero be pin-powered?

 Answer:

You can use 5V pins for power supply.

Question:The ESP32-S3-Zero programming failed?

 Answer:

If programming fails, press and hold the boot button while connecting to the computer, then release it, and

programming again.

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

10/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

Question:Does the ESP32-S3-Zero have a version that supports CAN?

 Answer:

The ESP32-S3 has a CAN controller, but CAN communication requires an external transceiver for

implementation, which users need to develop themselves.

Question:What is the accurate spacing between the two rows of female headers on the

PCB?

 Answer:

15.24mm.

Question:How many mA is required if 5V is supplied?

 Answer:

At least 500mA.

Question: Can ESP32-S3-Zero realize external microphone and speakers?

 Answer:

Speakers need an amplifier, which can not be directly connected to the GPIO pin, so does not support direct

connection to the speaker, the microphone is I2S-driven, and can be directly connected to the microphone!

Question: Does the ESP32-S3-Zero have touch pins?

 Answer:

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

11/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

Yes, you can refer to the manual.

(/wiki/File:ESP32-S3-Zero-FAQ.png)

Question:How Can I utilize pins 14, 15 and 16? They don't have a connection to pins, do

you have a way to connect to these pins?

 Answer:

It is a pad for soldering or using pogo pins.

Question:How many PWM channels for ESP32-S3-Zero?

 Answer:

It includes two MCPWM for driving digital motors and smart LEDs. For details, you can refer to this link

(https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_datasheet_en.pdf).

Question:Can this product work with 1.47inch LCD Module or 1.69inch LCD Module?

 Answer:

Yes.

Question:Which board can I use when using platformio?

 Answer:

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

12/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

Please select Espressif-S3-DevKitM-1(esp32-s3-devkitm-1).

Question:Does this board have onboard download circuit?

 Answer:

No.

Question:What is the operation temperature of this product?

 Answer:

The ambient temperature for the chip is at –40 ∼ 85 degrees.

Question:What is the maximum bandwidth for communication data?

 Answer:

40 MHz.

Question:How to connect a display that uses SDA and SCL to ESP32-S3-Zero. How to

wire this, and use this in Arduino code?

 Answer:

You need to create the code by yourself, refer to the attached demo code (https://files.waveshare.com/wiki/

ESP32-S3-Zero/0.91%20oled%20with%20esp32.txt).

0.91-inch OLED Module:

SDA: Connect to the ESP32-S3-Zero's SDA pin (e.g., GPIO 21)

SCL: Connect to the ESP32-S3-Zero's SCL pin (e.g., GPIO 22)

VCC: Connect to the ESP32-S3-Zero's VCC pin (3.3V or 5V)

GND: Connect to the ESP32-S3-Zero's GND pin

Install OLED Libraries:

Go to Sketch > Include Library > Manage Libraries.

Search for and install the following libraries:

Adafruit SSD1306

https://www.waveshare.com/wiki/ESP32-S3-Zero#Datasheet

13/14

9/3/25, 7:04 PM

ESP32-S3-Zero - Waveshare Wiki

Adafruit GFX Library

you can check Waveshare OLED https://www.waveshare.com/0.91inch-oled-module.htm

(https://www.waveshare.com/0.91inch-oled-module.htm)

Support

Technical Support

If you need technical support or have any

feedback/review, please click the Submit Now button

to submit a ticket, Our support team will check and

reply to you within 1 to 2 working days. Please be

patient as we make every effort to help you to resolve

the issue.

Working Time: 9 AM - 6 PM GMT+8 (Monday to

Friday)

Submit Now (https://service.wav

eshare.com/)

Retrieved from "https://www.waveshare.com/w/index.php?title=ESP32-S3-Zero&oldid=101443