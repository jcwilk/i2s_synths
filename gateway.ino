// Gateway functionality for WM8960 management

#if ENABLE_GATEWAY
#include <Wire.h>
#include <SparkFun_WM8960_Arduino_Library.h>

WM8960 codec;

// I2C pin definitions for ESP32-S3-Zero
#define I2C_SDA 5
#define I2C_SCL 6

// WM8960 I2C addresses (depends on board's CSB strap)
#ifndef WM8960_I2C_ADDR_PRIMARY
#define WM8960_I2C_ADDR_PRIMARY 0x1A
#endif
#ifndef WM8960_I2C_ADDR_SECONDARY
#define WM8960_I2C_ADDR_SECONDARY 0x1B
#endif

// Optional reset pin for the codec (set this to the pin wired to WM8960 RESET, or leave -1 if not wired)
#ifndef WM8960_RESET_PIN
#define WM8960_RESET_PIN -1
#endif

static void i2cScan() {
  Serial.println("I2C scan start...");
  uint8_t count = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print(" - Found device at 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      count++;
    }
  }
  if (count == 0) {
    Serial.println(" - No I2C devices found");
  } else {
    Serial.printf(" - Total devices: %u\n", count);
  }
}

static void resetCodecIfPossible() {
  if (WM8960_RESET_PIN >= 0) {
    pinMode(WM8960_RESET_PIN, OUTPUT);
    digitalWrite(WM8960_RESET_PIN, LOW);
    delay(5);
    digitalWrite(WM8960_RESET_PIN, HIGH);
    delay(5);
  }
}

static bool i2cDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

static bool tryCodecBegin() {
  Serial.println("Attempting WM8960.begin() at default address (0x1A)...");
  return codec.begin(Wire);
}

static bool initCodecWithRetries() {
  const uint8_t maxAttempts = 3;
  for (uint8_t attempt = 1; attempt <= maxAttempts; attempt++) {
    resetCodecIfPossible();
    delay(10);
    if (tryCodecBegin()) {
      Serial.printf("WM8960 init succeeded on attempt %u\n", attempt);
      return true;
    }
    Serial.printf("WM8960 init failed (attempt %u/%u). Retrying...\n", attempt, maxAttempts);
    delay(50);
  }
  return false;
}

void gatewaySetup() {
  // Initialize I2C with custom pins
  // Enable weak pull-ups to improve signal integrity if external pull-ups are marginal.
  // External 2.2k-4.7k pull-ups are still recommended for robust I2C.
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // 100kHz for reliable communication
  Wire.setTimeOut(50);

  // Provide the codec a brief power-up window before configuration
  delay(20);

  // Quick bus scan to aid debugging
  i2cScan();

  const bool has1A = i2cDevicePresent(WM8960_I2C_ADDR_PRIMARY);
  const bool has1B = i2cDevicePresent(WM8960_I2C_ADDR_SECONDARY);

  if (!has1A) {
    if (has1B) {
      Serial.println("Detected WM8960 at 0x1B, but library default is 0x1A.");
      Serial.println("Recompile with '#define WM8960_ADDR 0x1B' BEFORE including SparkFun_WM8960_Arduino_Library.h, or modify the library address.");
      reportError("WM8960 appears at 0x1B. Library is fixed to 0x1A.");
    } else {
      reportError("WM8960 not found on I2C (no 0x1A/0x1B). Check wiring and power.");
    }
  }

  if (!initCodecWithRetries()) {
    reportError("WM8960 not detected after retries. Check wiring, reset timing, and pull-ups.");
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
