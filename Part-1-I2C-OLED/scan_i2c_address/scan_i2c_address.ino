#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);            // SDA, SCL
  Serial.println("Scanning...");

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);   // call the name
    if (Wire.endTransmission() == 0) {  // 0 = ACK
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
  Serial.println("Done.");
}

void loop() {}