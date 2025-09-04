9/3/25, 7:10 PM

Pmod I2S2 Reference Manual - Digilent Reference

Pmod I2S2 Reference Manual

The Digilent Pmod I2S2 (Revision A) features a Cirrus CS5343 Multi-Bit Audio A/D Converter (https://www.cirrus.com/products/cs5343-44/) and a Cirrus CS4344 Stereo D/A Converter (https://www.cirrus.com/products/cs4344-45-48/), each connected to one of two audio jacks. These circuits allow a system board to transmit and receive stereo audio signals via the I2S protocol. The Pmod I2S2 supports 24 bit resolution per channel at input sample rates up to 108 kHz () and output sample rates up to 200 kHz ().





(https://digilent.com/reference/_media/reference/pmod/pmodi2s2/pmod_i2s2_oblique_1200.png)

https://digilent.com/reference/pmod/pmodi2s2/reference-manual

1/7

9/3/25, 7:10 PM

Pmod I2S2 Reference Manual - Digilent Reference

https://digilent.com/reference/pmod/pmodi2s2/reference-manual

2/7

9/3/25, 7:10 PM

Pmod I2S2 Reference Manual - Digilent Reference

Additional Important Documentation:

EMC Disclaimer, Digilent Development and Evaluation Kits (https://digilent.com/reference/lib/exe/fetch.php? tok=a2aed5&media=https%3A%2F%2Ffiles.digilent.com%2Fresources%2Fdocuments%2FEMC_DISCLAIMER_DIGILENT_DEVELOPMENT_AND_EVALUATION_KITS.pdf)

https://digilent.com/reference/pmod/pmodi2s2/reference-manual

3/7

9/3/25, 7:10 PM Features

Pmod I2S2 Reference Manual - Digilent Reference

Stereo 24-bit A/D () and D/A () converters for I2S audio input and output Standard 1/8 in (3.5mm) stereo audio jacks Optional automatic serial clock generation for audio input 12-pin Pmod Port with two I2S interfaces Follows the Digilent Pmod Interface Specification (https://digilent.com/reference/_media/reference/pmod/digilent-pmod-interface-specification.pdf)

Specifications

Parameter

Power Supply Voltage

Audio Input Sample Rate (Single-Speed Mode)

Audio Input Sample Rate (Double-Speed Mode)

Audio Output Sample Rate

Min

3.0

4

86

2

Typical

3.3

Max

5.25

54

108

200

Units

V

kHz ()

kHz ()

kHz ()

Pinout Table Diagram

Header J1

Pin

Signal

Description

Pin

Signal

Description

1

2

3

4

5

6

D/A () MCLK

I2S Line Out Converter Master Clock

D/A () LRCK

I2S Line Out Converter Word Select

D/A () SCLK ()

I2S Line Out Converter Serial Clock

D/A () SDIN

I2S Line Out Converter Serial Data Input

GND ()

VCC ()

Power Supply Ground

Power Supply (3.3V)

7

8

9

10

11

12

A/D () MCLK

I2S Line In Converter Master Clock

A/D () LRCK

I2S Line In Converter Word Select

A/D () SCLK ()

I2S Line In Converter Serial Clock

A/D () SDOUT

I2S Line In Converter Serial Data Output

GND ()

VCC ()

Power Supply Ground

Power Supply (3.3V)

State

MST

SLV

Physical Dimensions

Jumper JP1

Description

Line-in converter Master Mode selected

Line-in converter Slave Mode selected

The pins on the pin header are spaced 100 mil apart. The PCB is 1.0 inches long on the sides parallel to the pins on the pin header and 0.8 inches long on the sides perpendicular to the pin header. The Line In and Line Out audio jacks are approximately 0.44 inches apart, measured from the center of each jack.

Functional Description

The Pmod I2S2 utilizes a (Cirrus Logic CS4344 Stereo D/A () converter) to take digital audio data and output the corresponding analog signal through a standard stereo headphone jack (labeled Line Out). In addition, a (Cirrus Logic CS5343 Stereo A/D () converter) is used to convert analog audio signals from a second 3.5mm audio jack (labeled Line In) into digital audio data. It is designed to work at a wide variety of standard audio sampling rates.

Serial Communication

The two primary integrated circuits of the Pmod I2S2 communicate with the host board via the (GPIO () protocol). As each IC uses the Integrated Interchip Sound (I2S) protocol, several different clock lines are required, as described (below).

The CS4344 and CS5343 (henceforth referred to as the “line-out converter” and “line-in converter”, respectively) are each connected (to the host board) via their own I2S interface. As seen in the (Pinout table) above, the line-out converter's I2S interface is connected to the top row interface of Pmod connector J1, while the line-in converter's I2S interface is connected to the bottom row.

Any external power applied to the Pmod I2S must be within 3 V and 5.25 V; however, it is recommended that Pmod is operated at 3.3 V. Digital logic levels must correspond to the power supply voltage.

I2S Overview

The fastest clock signal of each I2S interface will be the Master Clock (MCLK); as the name implies, this signal will keep everything nicely synchronized. The Left- Right Clock (LRCK), also known as the Word Select Clock, indicates whether a particular set of data is associated with the left or right audio channel for stereo sound.

https://digilent.com/reference/pmod/pmodi2s2/reference-manual

4/7

9/3/25, 7:10 PM

Pmod I2S2 Reference Manual - Digilent Reference

The final clock is the Serial Clock (SCLK ()), also known as the Bit Clock. The line-in and line-out converters can each either be provided this clock signal, or generate it internally. More information on how the serial clocks for each converter can be found (below).

The I2S protocol requires that data is clocked in on the falling edge of SCLK (). The first bit of data (MSB) is not clocked in on the falling edge until the first complete serial clock cycle has passed after LRCK has changed state. Data must be valid on the rising edge of SCLK ().

NOTE: The term “I2S input/output sample rate” refers to the frequency that a full frame of data, consisting of both the left and right channels, is transmitted over an I2S interface.

An example timing diagram of a single I2S frame is shown below.

Line Out Serial Clock Generation

The line-out converter will internally derive its SCLK () if it is provided at least two consecutive frames of the LRCK without providing any SCLK () signals. In this case, the line-out converter will measure the MCLK and LRCK rates and determine an appropriate SCLK () rate. However, the MCLK/LRCK ratio must meet one of several specific ratios in order to properly generate SCLK (), as outlined in the table below from the CS4344 datasheet.

Internal SCK Mode

External SCK Mode

16-bit data and SCK = 32*Fs if MCLK/LRCK = 1024, 512, 256, 128, or 64

Up to 24-bit data with data valid on the rising edge of SCK

Up to 24-bit data and SCK = 48*Fs if MCLK/LRCK = 768, 384, 192, or 96

Up to 24-bit data and SCK = 72*Fs if MCLK/LRCK = 1152

The ratio between the MCLK and LRCK rates must be an integer ratio so that the line-out converter's internal clock dividers can determine an appropriate bit rate. A table of commonly used sample rates and their corresponding MCLK rates, from the CS4344 datasheet, is provided below.

LRCK (kHz ())

MCLK (MHz ())

64x

96x

128x

192x

256x

384x

512x

768x

1024x

1152x

32

44.1

48

64

88.2

96

128

176.4

192

Mode

8.1920

12.2880

32.7680

36.8640

11.2896

16.9344

22.5792

33.8680

45.1580

12.2880

18.4320

24.5760

36.8640

49.1520

8.1920

12.2880

32.7680

49.1520

11.2896

16.9344

22.5792

33.8680

12.2880

18.4320

24.5760

36.8640

8.1920

12.2880

32.7680

49.1520

11.2896

16.9344

22.5792

33.8680

12.2880

18.4320

24.5760

36.8640

QSM

DSM

SSM

Line In Serial Clock Generation

The line-in converter can be placed in either Master Mode or Slave Mode by setting mode jumper JP1 to the corresponding position. The position of this jumper should not be changed while the Pmod I2S2 is powered on.

In Slave Mode, LRCK and SCLK () must be generated by the host board. Supported sample rate ranges and their corresponding MCLK/LRCK and SCLK ()/LRCK ratios are provided in the table below from the CS5343 datasheet. The line-in converter automatically selects as needed from single- and double-speed modes.

Speed Mode

MCLK/LRCK Ratio

SCLK ()/LRCK Ratio

Input Sample Rate Range (kHz ())

Single-Speed Mode

256x

512x

384x

784x

64

64

64

64

4-24, 43-54

43-54

4-24, 43-54

43-54

https://digilent.com/reference/pmod/pmodi2s2/reference-manual

5/7

9/3/25, 7:10 PM

Pmod I2S2 Reference Manual - Digilent Reference

Speed Mode

MCLK/LRCK Ratio

SCLK ()/LRCK Ratio

Input Sample Rate Range (kHz ())

Double-Speed Mode

128x

256x

192x

384x

64

64

64

64

86-108

86-108

86-108

86-108

In Master Mode, both LRCK and SCLK () are automatically generated by the line-in converter. For Master Mode, the provided MCLK rate must be within the range of 4-54 KHz. Once the line-in converter has powered up, it automatically selects an MCLK/LRCK ratio of 256x/512x, depending on the MCLK rate.

Note: The CS5343's Double-Speed Mode is not available in Master Mode on the Pmod I2S2.

A table of common MCLK frequencies, for both Master and Slave Modes, with their corresponding MCLK/LRCK ratios and audio sample rates, from the CS5343 datasheet, is provided below.

Master and Slave Mode

Sample Rate (kHz ())

Speed Mode

MCLK (MHz ())

MCLK (MHz ())

32 (*Slave Mode Only)

44.1

48

SSM

SSM

SSM

256x

*8.192

11.289

12.288

512x

384x

768x

*16.384

*12.288

*24.576

22.579

24.576

16.934

18.432

33.868

36.864

Sample Rate (kHz ())

Speed Mode

MCLK (MHz ())

MCLK (MHz ())

88.2

96

DSM

DSM

128x

11.289

12.288

256x

22.579

24.576

192x

16.934

18.432

384x

33.868

36.864

Quick Start

To set up a simple 44.1 kHz () audio passthrough, three control signals need to be generated by the host system board.

A master clock (MCLK) at a frequency of approximately 22.579 MHz ().

A serial clock (SCLK ()), which fully toggles once per 8 MCLK periods.

A Left/Right Word Select signal, which fully toggles once per 64 SCLK () periods.

The Pmod I2S2's Master/Slave select jumper (JP1) should be placed into the Slave (SLV) position.

Each of these control signals should be provided to the appropriate pin on both the top and bottom rows of the Pmod I2S2.

The ADOUT_SDIN pin should be driven by the ADIN_SDOUT signal.

See the I2S Overview section for more information on the timing of these signals.

Additional Information

The schematics of the Pmod I2S2 are available here (https://digilent.com/reference/_media/reference/pmod/pmodi2s2/pmodi2s2_sch.pdf). Additional information about the line-in and line-out converters (CS5343 and CS4344, respectively) including master/slave modes and specific timings of the chips can be found by checking out their datasheets here: CS5343 (https://www.cirrus.com/products/cs5343-44/), CS4344 (https://www.cirrus.com/products/cs4344-45-48/).





More specific information about how to use the Pmod I2S2 can be found by checking out the additional resources on the Pmod I2S2 Resource Center (https://digilent.com/reference/pmod/pmodi2s2/start#additional_resources). Example code demonstrating how to get information from the PmodI2S2 can be found on its Resource Center here (https://digilent.com/reference/pmod/pmodi2s2/start#example_projects).

If you have any questions or comments about the Pmod I2S2, feel free to post them under the appropriate section (“Add-on Boards”) of the Digilent Forum (https://forum.digilent.com/).



rm (https://digilent.com/reference/tag/rm?do=showtag&tag=rm), doc (https://digilent.com/reference/tag/doc?do=showtag&tag=doc), pmodi2s2 (https://digilent.com/reference/tag/pmodi2s2?do=showtag&tag=pmodi2s2)

Company (https://digilent.com/company/)

About Us (https://digilent.com/company/#about-digilent) FAQs (https://digilent.com/company/#faqs) Distributors (https://digilent.com/shop/distributors/) Shipping & Returns (https://digilent.com/shipping-returns/) Jobs (https://digilent.com/company/#jobs) Legal & Privacy (https://digilent.com/legal-privacy/)

News (https://digilent.com/news/)

Blog (https://digilent.com/blog/) Newsletter (https://digilent.com/news/#newsletter) Events (https://digilent.com/news/#events)

https://digilent.com/reference/pmod/pmodi2s2/reference-manual

6/7

9/3/25, 7:10 PM

Pmod I2S2 Reference Manual - Digilent Reference

Submit

Subscribe to our newsletter

Get the latest updates on new products and upcoming sales

Your email address

Contact Us

Technical Support Forum (https://forum.digilent.com) Support Channels (https://digilent.com/support/#channels)

Digilent 1300 NE Henley Ct. Suite 3 Pullman, WA 99163 United States of America

© 2023 Digilent

(http://twitter.com/DigilentInc) (http://facebook.com/Digilent) (https://www.youtube.com/user/DigilentInc) (https://github.com/digilent) (https://instagram.com/digilentinc) (https://www.linkedin.com/company/1454013)