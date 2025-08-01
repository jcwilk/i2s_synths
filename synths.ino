#include <Wire.h>
#include <SparkFun_WM8960_Arduino_Library.h>
#include <driver/i2s.h>

// Module configuration
#define ENABLE_DELAY 1

WM8960 codec;

// I2C pin definitions for ESP32-S3-Zero
#define I2C_SDA 8
#define I2C_SCL 9

// I2S pin definitions for ESP32-S3-Zero (GPIO 1-13 only)
#define I2S_WS 13 // Word Select (Left/Right Clock) - shared
#define I2S_SD_IN 12 // Serial Data In (ADC data FROM WM8960)
#define I2S_SD_OUT 10 // Serial Data Out (DAC data TO WM8960)
#define I2S_SCK 11 // Serial Clock (Bit Clock) - shared

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 44100
#define BUFFER_LEN 128

int16_t rxBuffer[BUFFER_LEN];
int16_t txBuffer[BUFFER_LEN];

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3-Zero WM8960 I2S Audio Processing (GPIO 1-13)");

  // Initialize I2C with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // 100kHz for reliable communication

  if (codec.begin() == false) {
    Serial.println("WM8960 not detected. Check wiring and power.");
    while (1);
  }
  Serial.println("WM8960 connected successfully");

  // Configure WM8960 for I2S operation (NO internal loopback)
  setupWM8960ForI2S();

  // Setup I2S for bidirectional communication
  setupI2S();

  // Initialize audio processing module
  moduleSetup();

  Serial.println("Setup complete. Audio processing active on GPIO 10-13.");
  Serial.println("Microphone -> ESP32 -> Audio Processing -> ESP32 -> Speakers");
}

void loop() {
  // Read audio data from WM8960 ADC via I2S
  size_t bytesRead = 0;
  esp_err_t result = i2s_read(I2S_PORT, rxBuffer, BUFFER_LEN * sizeof(int16_t), &bytesRead, portMAX_DELAY);

  if (result == ESP_OK && bytesRead > 0) {
    int samplesRead = bytesRead / sizeof(int16_t);

    // Process audio through module
    moduleLoop(rxBuffer, txBuffer, samplesRead);

    // Send processed audio data back to WM8960 DAC via I2S
    size_t bytesWritten = 0;
    i2s_write(I2S_PORT, txBuffer, samplesRead * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  }
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

void setupI2S() {
  // Configure I2S for bidirectional communication (RX + TX)
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // Master mode, both RX and TX
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 4, // Small buffers for low latency
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0,
    .mclk_multiple = i2s_mclk_multiple_t(I2S_MCLK_MULTIPLE_512),
    .bits_per_chan = i2s_bits_per_chan_t(I2S_BITS_PER_CHAN_DEFAULT)
  };

  // Install I2S driver
  esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    Serial.printf("I2S driver install failed: %d\n", result);
    return;
  }

  // Configure I2S pins
  const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE, // No master clock needed
    .bck_io_num = I2S_SCK, // Bit clock (shared) - GPIO 11
    .ws_io_num = I2S_WS, // Word select (shared) - GPIO 13
    .data_out_num = I2S_SD_OUT, // Data output (to WM8960 DAC) - GPIO 10
    .data_in_num = I2S_SD_IN // Data input (from WM8960 ADC) - GPIO 12
  };

  result = i2s_set_pin(I2S_PORT, &pin_config);
  if (result != ESP_OK) {
    Serial.printf("I2S set pin failed: %d\n", result);
    return;
  }

  Serial.println("I2S configured for bidirectional communication on GPIO 10-13");
}