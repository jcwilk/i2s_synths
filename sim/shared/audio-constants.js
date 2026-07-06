/** Firmware-aligned mono bridge geometry (22.05 kHz, 128 samples/path). */

export const SAMPLE_RATE = 22050;
export const BUFFER_LEN = 128;
export const PERIOD_MS = (BUFFER_LEN / SAMPLE_RATE) * 1000;
