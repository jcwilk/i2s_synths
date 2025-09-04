#ifndef GATEWAY_H
#define GATEWAY_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_WM8960_Arduino_Library.h>
#include "../config/constants.h"
#include "../ui/neopixel.h"

#if ENABLE_GATEWAY

static WM8960 codec;

#ifndef I2C_SDA
#define I2C_SDA 5
#endif
#ifndef I2C_SCL
#define I2C_SCL 6
#endif

#ifndef WM8960_I2C_ADDR_PRIMARY
#define WM8960_I2C_ADDR_PRIMARY 0x1A
#endif
#ifndef WM8960_I2C_ADDR_SECONDARY
#define WM8960_I2C_ADDR_SECONDARY 0x1B
#endif

#ifndef WM8960_RESET_PIN
#define WM8960_RESET_PIN -1
#endif

static inline void i2cScan() {
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

static inline void resetCodecIfPossible() {
  if (WM8960_RESET_PIN >= 0) {
    pinMode(WM8960_RESET_PIN, OUTPUT);
    digitalWrite(WM8960_RESET_PIN, LOW);
    delay(5);
    digitalWrite(WM8960_RESET_PIN, HIGH);
    delay(5);
  }
}

static inline bool i2cDevicePresent(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

static inline bool tryCodecBegin() {
  Serial.println("Attempting WM8960.begin() at default address (0x1A)...");
  return codec.begin(Wire);
}

static inline bool initCodecWithRetries() {
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

inline void setupWM8960ForI2S() {
  codec.enableVREF();
  codec.enableVMID();
  codec.enableLMIC();
  codec.enableRMIC();
  codec.connectLMN1();
  codec.connectRMN1();
  codec.disableLINMUTE();
  codec.disableRINMUTE();
  codec.setLINVOLDB(12.00);
  codec.setRINVOLDB(12.00);
  codec.setLMICBOOST(WM8960_MIC_BOOST_GAIN_20DB);
  codec.setRMICBOOST(WM8960_MIC_BOOST_GAIN_20DB);
  codec.connectLMIC2B();
  codec.connectRMIC2B();
  codec.enableAINL();
  codec.enableAINR();
  codec.disableLB2LO();
  codec.disableRB2RO();
  codec.enableLD2LO();
  codec.enableRD2RO();
  codec.setLB2LOVOL(WM8960_OUTPUT_MIXER_GAIN_NEG_21DB);
  codec.setRB2ROVOL(WM8960_OUTPUT_MIXER_GAIN_NEG_21DB);
  codec.enableLOMIX();
  codec.enableROMIX();
  codec.enablePLL();
  codec.setWL(WM8960_WL_16BIT);
  codec.setPLLPRESCALE(WM8960_PLLPRESCALE_DIV_2);
  codec.setSMD(WM8960_PLL_MODE_FRACTIONAL);
  codec.setCLKSEL(WM8960_CLKSEL_PLL);
  codec.setSYSCLKDIV(WM8960_SYSCLK_DIV_BY_2);
  codec.setBCLKDIV(4);
  codec.setDCLKDIV(WM8960_DCLKDIV_16);
  codec.setPLLN(7);
  codec.setPLLK(0x86, 0xC2, 0x26);
  codec.enablePeripheralMode();
  codec.enableAdcLeft();
  codec.enableAdcRight();
  codec.enableDacLeft();
  codec.enableDacRight();
  codec.disableDacMute();
  codec.enableSpeakers();
  codec.setSpeakerVolumeDB(0.00);
  Serial.println("WM8960 configured as I2S slave");
}

inline void gatewaySetup() {
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  Serial.println("Gateway enabled: I2SD MASTER, I2SU MASTER");

  delay(20);
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

#else

inline void gatewaySetup() {
  Serial.println("Gateway disabled - no WM8960 management");
}

#endif // ENABLE_GATEWAY

#endif // GATEWAY_H


