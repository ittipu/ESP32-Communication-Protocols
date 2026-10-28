/*
  IoTBhai · Full-Stack IoT Engineer Bootcamp
  Chapter 13.4 · One-Wire "The Serial Number"
  Sketch 1 of 2: ask the bus who's out there

  Every DS18B20 has a unique 64-bit ROM ID, burned in at the factory:
    byte 0     = family code (0x28 means "I'm a DS18B20")
    bytes 1..6 = 48-bit serial number
    byte 7     = CRC, calculated from bytes 0..6

  Wiring (3 wires + 1 resistor):
    DS18B20 VDD -> 3V3
    DS18B20 GND -> GND
    DS18B20 DQ  -> GPIO27
    4.7 kΩ resistor between DQ and 3V3 (the pull-up)

  Waterproof probe colors: red = VDD, yellow (or white) = DQ, black = GND.
  Add a second sensor on the same three wires and it shows up too.

  Libraries:      OneWire (Paul Stoffregen), DallasTemperature (Miles Burton)
  Board:          ESP32 Dev Module, ESP32 Arduino core 3.x
  Serial Monitor: 115200 baud
*/

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire bus(27);                       // DQ on GPIO27
DallasTemperature sensors(&bus);

void setup() {
  Serial.begin(115200);
  sensors.begin();                     // search the bus
  int count = sensors.getDeviceCount();
  Serial.printf("Found %d sensor(s) on GPIO27\n", count);

  for (int i = 0; i < count; i++) {
    DeviceAddress id;                  // 8 bytes
    sensors.getAddress(id, i);
    Serial.printf("Sensor %d: ", i);
    for (int b = 0; b < 8; b++) Serial.printf("%02X ", id[b]);
    bool crcOk = OneWire::crc8(id, 7) == id[7];
    Serial.println(crcOk ? "(CRC OK)" : "(CRC BAD)");
  }
}

void loop() {}
