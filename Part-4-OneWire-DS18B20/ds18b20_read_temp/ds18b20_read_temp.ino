/*
  IoTBhai · Full-Stack IoT Engineer Bootcamp
  Chapter 13.4 · One-Wire "The Serial Number"
  Sketch 2 of 2: read the temperature without freezing the loop

  A 12-bit reading takes up to 750 ms. Instead of delay(750),
  we start a conversion, let loop() keep running, and come back
  with millis() when it's ready (the Chapter 10 pattern).

  Wiring (3 wires + 1 resistor):
    DS18B20 VDD -> 3V3
    DS18B20 GND -> GND
    DS18B20 DQ  -> GPIO27
    4.7 kΩ resistor between DQ and 3V3 (the pull-up)

  Two numbers to remember:
    -127.00  the sensor didn't answer: check DQ and the pull-up
     85.00   read before the conversion finished (power-on value)

  Libraries:      OneWire (Paul Stoffregen), DallasTemperature (Miles Burton)
  Board:          ESP32 Dev Module, ESP32 Arduino core 3.x
  Serial Monitor: 115200 baud
*/

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire bus(27);
DallasTemperature sensors(&bus);
unsigned long convStart = 0;

void setup() {
  Serial.begin(115200);
  sensors.begin();
  sensors.setResolution(12);            // 0.0625 °C steps
  sensors.setWaitForConversion(false);  // don't block
  sensors.requestTemperatures();        // start measuring
  convStart = millis();
}

void loop() {
  if (millis() - convStart >= 750) {    // Ch 10 pattern
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
      float c = sensors.getTempCByIndex(i);
      if (c == DEVICE_DISCONNECTED_C)
        Serial.printf("Sensor %d: -127.00 C (no answer!)\n", i);
      else
        Serial.printf("Sensor %d: %.2f C\n", i, c);
    }
    sensors.requestTemperatures();      // next reading
    convStart = millis();
  }                                     // loop() stays free
}
