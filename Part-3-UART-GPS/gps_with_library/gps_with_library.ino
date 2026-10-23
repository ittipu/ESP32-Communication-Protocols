/*
  IoTBhai · Full-Stack IoT Engineer Bootcamp
  Chapter 13.3 · UART "The Agreed Speed"
  Sketch 2 of 2: read a GPS WITH a library (TinyGPSPlus)

  Same GPS, same wires as sketch 1. This time TinyGPSPlus
  collects the sentences, checks the checksums and decodes
  every field for us. We just ask for the values.

  Wiring (4 wires, TX and RX cross over):
    GPS VCC -> VIN (5 V; the GPS board has its own regulator)
    GPS GND -> GND
    GPS TX  -> GPIO26 (ESP32 RX2)
    GPS RX  -> GPIO27 (ESP32 TX2)

  Library:        TinyGPSPlus by Mikal Hart (Library Manager)
  Board:          ESP32 Dev Module, ESP32 Arduino core 3.x
  Serial Monitor: 115200 baud
*/

#include <TinyGPSPlus.h>

// ---------- settings you can change ----------
#define GPS_RX      26     // ESP32 listens here  <- GPS TX
#define GPS_TX      27     // ESP32 talks here    -> GPS RX
#define GPS_BAUD    9600   // Break it: try 4800
#define UTC_OFFSET  6      // Dhaka is UTC+6. Change it for your city.
// ---------------------------------------------

TinyGPSPlus gps;
unsigned long lastPrint = 0;

void printReport() {
  Serial.println("--------------------------------------------------");

  if (gps.charsProcessed() < 10) {
    Serial.println("Silence: no bytes from the GPS.");
    Serial.println("Check GPS TX -> GPIO26 (not TX to TX), VCC -> VIN and GND.");
    return;
  }
  if (gps.passedChecksum() == 0) {
    Serial.println("Bytes arrive, but no valid sentence yet.");
    Serial.printf("Wrong baud rate? This sketch listens at %d. The GPS default is 9600.\n", GPS_BAUD);
    return;
  }

  if (gps.time.isValid()) {
    int localH = (gps.time.hour() + UTC_OFFSET) % 24;
    Serial.printf("Time      %02d:%02d:%02d UTC   (%02d:%02d:%02d local)\n",
                  gps.time.hour(), gps.time.minute(), gps.time.second(),
                  localH, gps.time.minute(), gps.time.second());
  }
  if (gps.date.isValid()) {
    Serial.printf("Date      %02d/%02d/%04d\n", gps.date.day(), gps.date.month(), gps.date.year());
  }
  Serial.printf("Sats      %lu\n", (unsigned long)gps.satellites.value());

  if (gps.location.isValid()) {
    Serial.printf("Lat, Lon  %.6f, %.6f\n", gps.location.lat(), gps.location.lng());
    Serial.printf("Altitude  %.1f m\n", gps.altitude.meters());
    Serial.printf("Speed     %.1f km/h\n", gps.speed.kmph());
    Serial.printf("HDOP      %.2f (lower = more accurate)\n", gps.hdop.hdop());
    Serial.printf("Map       https://maps.google.com/?q=%.6f,%.6f\n", gps.location.lat(), gps.location.lng());
  } else {
    Serial.println("Fix       none yet. Move near a window.");
  }
  Serial.printf("Checksum  %lu ok, %lu bad\n",
                (unsigned long)gps.passedChecksum(), (unsigned long)gps.failedChecksum());
}

void setup() {
  Serial.begin(115200);                                  // UART0: to your PC
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);   // UART2: to the GPS
  Serial.println();
  Serial.println("GPS with TinyGPSPlus...");
}

void loop() {
  while (Serial2.available()) {        // feed every byte to the parser
    gps.encode(Serial2.read());
  }
  if (millis() - lastPrint >= 3000) {  // Chapter 10 pattern
    lastPrint = millis();
    printReport();
  }
}
