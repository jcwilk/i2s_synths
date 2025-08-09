// Gateway functionality for WM8960 management

#if ENABLE_GATEWAY
#include <Wire.h>
#include <SparkFun_WM8960_Arduino_Library.h>

WM8960 codec;

// I2C pin definitions for ESP32-S3-Zero
#define I2C_SDA 5
#define I2C_SCL 6

void gatewaySetup() {
  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // 100kHz for reliable communication

  if (codec.begin() == false) {
    // TODO: This occurs very frequently and randomly, there must be some race condition?
    reportError("WM8960 not detected. Check wiring and power.");
    while (1) {
      delay(10);
    }
  }
  Serial.println("WM8960 connected successfully");

  // Configure WM8960 for I2S operation
  setupWM8960ForI2S();
}

void setupWM8960ForI2S() {
  // General setup needed
  codec.enableVREF();
  codec.enableVMID();

  // Setup signal flow from onboard microphones to ADC
  codec.enableLMIC();
  codec.enableRMIC();

  // Connect from INPUT1 to "n" (aka inverting) inputs of PGAs.
  codec.connectLMN1();
  codec.connectRMN1();

  // Disable mutes on PGA inputs
  codec.disableLINMUTE();
  codec.disableRINMUTE();

  // Set input volumes for microphones
  codec.setLINVOLDB(12.00); // +12dB gain for microphone
  codec.setRINVOLDB(12.00); // +12dB gain for microphone

  // Set input boosts to get inputs 1 to the boost mixers
  codec.setLMICBOOST(WM8960_MIC_BOOST_GAIN_20DB);
  codec.setRMICBOOST(WM8960_MIC_BOOST_GAIN_20DB);

  // Connect microphone boost to boost mixers
  codec.connectLMIC2B();
  codec.connectRMIC2B();

  // Enable boost mixers
  codec.enableAINL();
  codec.enableAINR();

  // Disable analog bypass (we want digital processing)
  codec.disableLB2LO();
  codec.disableRB2RO();

  // Connect from DAC outputs to output mixer
  codec.enableLD2LO();
  codec.enableRD2RO();

  // Set gain stage between booster mixer and output mixer
  codec.setLB2LOVOL(WM8960_OUTPUT_MIXER_GAIN_NEG_21DB);
  codec.setRB2ROVOL(WM8960_OUTPUT_MIXER_GAIN_NEG_21DB);

  // Enable output mixers
  codec.enableLOMIX();
  codec.enableROMIX();

  // Clock configuration for 44.1kHz
  codec.enablePLL(); // Needed for class-d amp clock
  codec.setWL(WM8960_WL_16BIT); // 16-bit samples
  codec.setPLLPRESCALE(WM8960_PLLPRESCALE_DIV_2);
  codec.setSMD(WM8960_PLL_MODE_FRACTIONAL);
  codec.setCLKSEL(WM8960_CLKSEL_PLL);
  codec.setSYSCLKDIV(WM8960_SYSCLK_DIV_BY_2);
  codec.setBCLKDIV(4);
  codec.setDCLKDIV(WM8960_DCLKDIV_16);
  codec.setPLLN(7);
  codec.setPLLK(0x86, 0xC2, 0x26); // PLLK=86C226h for 44.1kHz

  // **IMPORTANT: Set WM8960 as I2S SLAVE (ESP32 will be master)**
  codec.enablePeripheralMode(); // WM8960 receives clocks from ESP32

  // Enable ADCs and DACs
  codec.enableAdcLeft();
  codec.enableAdcRight();
  codec.enableDacLeft();
  codec.enableDacRight();
  codec.disableDacMute();

  // **DO NOT enable internal loopback - we want external I2S processing**
  // codec.enableLoopBack(); // <-- REMOVED

  // Enable Class-D speaker drivers
  codec.enableSpeakers();
  codec.setSpeakerVolumeDB(0.00); // Set speaker volume to 0dB

  Serial.println("WM8960 configured as I2S slave");
}

#else
// Gateway disabled - provide no-op functions
void gatewaySetup() {
  Serial.println("Gateway disabled - no WM8960 management");
}

#endif
