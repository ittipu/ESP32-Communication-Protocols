/*
  IoTBhai · Full-Stack IoT Engineer Bootcamp
  Chapter 13.3 · UART "The Agreed Speed"
  Sketch 1 of 2: read a GPS with NO library

  The NEO-M8N sends plain text sentences (NMEA) over UART.
  This sketch reads them ourselves, checks each checksum,
  and decodes $GxRMC (time, fix, position, speed, date)
  and $GxGGA (satellites) by hand.

  Wiring (4 wires, TX and RX cross over):
    GPS VCC -> VIN (5 V; the GPS board has its own regulator)
    GPS GND -> GND
    GPS TX  -> GPIO26 (ESP32 RX2)
    GPS RX  -> GPIO27 (ESP32 TX2)

  Board:          ESP32 Dev Module, ESP32 Arduino core 3.x
  Serial Monitor: 115200 baud
*/

// ---------- settings you can change ----------
#define GPS_RX      26     // ESP32 listens here  <- GPS TX
#define GPS_TX      27     // ESP32 talks here    -> GPS RX
#define GPS_BAUD    9600   // Break it: try 4800 and watch the garbage
#define SHOW_RAW    true   // print every sentence as it arrives
#define UTC_OFFSET  6      // Dhaka is UTC+6. Change it for your city.
// ---------------------------------------------

char line[100];            // one NMEA sentence, e.g. "$GNRMC,081642.00,A,..."
uint8_t len = 0;

// what we decoded
bool   fix = false;
double lat = 0, lon = 0;
float  speedKmh = 0;
int    sats = 0;
int    hh = 0, mm = 0, ss = 0;
int    day = 0, month = 0, year = 0;

// health counters
unsigned long bytesIn = 0, sentencesOk = 0, sentencesBad = 0;
unsigned long lastByte = 0, lastPrint = 0;


// ---------- the checksum: XOR every character between '$' and '*' ----------
bool checksumOk(const char* s) {
  const char* star = strchr(s, '*');
  if (s[0] != '$' || star == nullptr || strlen(star) < 3) return false;
  uint8_t sum = 0;
  for (const char* p = s + 1; p < star; p++) sum ^= *p;
  uint8_t given = strtol(star + 1, nullptr, 16);   // the two hex digits after '*'
  return sum == given;
}

// ---------- copy field number n into out (field 0 is "$GNRMC") ----------
void getField(const char* s, int n, char* out, int size) {
  int field = 0, i = 0;
  for (const char* p = s; *p && *p != '*'; p++) {
    if (*p == ',') {
      if (field == n) break;
      field++;
      continue;
    }
    if (field == n && i < size - 1) out[i++] = *p;
  }
  out[i] = '\0';
}

// ---------- ddmm.mmmm (or dddmm.mmmm) -> decimal degrees ----------
double toDegrees(const char* value, const char* hemisphere) {
  if (value[0] == '\0') return 0;
  double raw = atof(value);              // e.g. 2346.84210
  int degrees = (int)(raw / 100);        // 23
  double minutes = raw - degrees * 100;  // 46.84210
  double d = degrees + minutes / 60.0;   // 23.780702
  if (hemisphere[0] == 'S' || hemisphere[0] == 'W') d = -d;
  return d;
}

// ---------- $GxRMC: Recommended Minimum ----------
// $GNRMC,081642.00,A,2346.84210,N,09024.61560,E,0.12,,290926,,,A*55
//        time      st latitude   N  longitude  E speed  date
void parseRMC(const char* s) {
  char f[16], h[4];

  getField(s, 1, f, sizeof f);                          // time hhmmss.ss (UTC)
  if (strlen(f) >= 6) sscanf(f, "%2d%2d%2d", &hh, &mm, &ss);

  getField(s, 2, f, sizeof f);                          // status: A = valid, V = void
  fix = (f[0] == 'A');

  if (fix) {
    getField(s, 3, f, sizeof f); getField(s, 4, h, sizeof h);
    lat = toDegrees(f, h);
    getField(s, 5, f, sizeof f); getField(s, 6, h, sizeof h);
    lon = toDegrees(f, h);
    getField(s, 7, f, sizeof f);                        // speed in knots
    speedKmh = atof(f) * 1.852;
  }

  getField(s, 9, f, sizeof f);                          // date ddmmyy
  if (strlen(f) == 6) sscanf(f, "%2d%2d%2d", &day, &month, &year);
}

// ---------- $GxGGA: fix data (we only need the satellite count) ----------
void parseGGA(const char* s) {
  char f[8];
  getField(s, 7, f, sizeof f);
  sats = atoi(f);
}

void handleSentence(const char* s) {
  if (SHOW_RAW) Serial.println(s);
  if (!checksumOk(s)) {          // a flipped bit on the wire: drop the sentence
    sentencesBad++;
    return;
  }
  sentencesOk++;
  if (strncmp(s + 3, "RMC", 3) == 0) parseRMC(s);       // works for GP, GN, GL...
  else if (strncmp(s + 3, "GGA", 3) == 0) parseGGA(s);
}

// ---------- collect bytes into lines, without blocking ----------
void readGPS() {
  while (Serial2.available()) {
    char c = Serial2.read();
    bytesIn++;
    lastByte = millis();
    if (c == '$') len = 0;                 // a new sentence starts
    if (c == '\r') continue;
    if (c == '\n') {                       // the sentence is complete
      line[len] = '\0';
      if (len > 0) handleSentence(line);
      len = 0;
    } else if (len < sizeof(line) - 1) {
      line[len++] = c;
    }
  }
}

void printSummary() {
  Serial.println("--------------------------------------------------");

  if (bytesIn == 0 || millis() - lastByte > 3000) {
    Serial.println("Silence: no bytes from the GPS.");
    Serial.println("Check GPS TX -> GPIO26 (not TX to TX), VCC -> VIN and GND.");
    return;
  }
  if (sentencesOk == 0) {
    Serial.println("Bytes arrive, but no valid sentence yet.");
    Serial.printf("Wrong baud rate? This sketch listens at %d. The GPS default is 9600.\n", GPS_BAUD);
    return;
  }

  int localH = (hh + UTC_OFFSET + 24) % 24;
  Serial.printf("Time      %02d:%02d:%02d UTC   (%02d:%02d:%02d local)\n", hh, mm, ss, localH, mm, ss);
  Serial.printf("Date      %02d/%02d/20%02d\n", day, month, year);
  Serial.printf("Sats      %d\n", sats);
  if (fix) {
    Serial.printf("Lat, Lon  %.6f, %.6f\n", lat, lon);
    Serial.printf("Speed     %.1f km/h\n", speedKmh);
    Serial.printf("Map       https://maps.google.com/?q=%.6f,%.6f\n", lat, lon);
  } else {
    Serial.println("Fix       none yet (status V). Move near a window.");
  }
  Serial.printf("Checksum  %lu ok, %lu bad\n", sentencesOk, sentencesBad);
}

void setup() {
  Serial.begin(115200);                                  // UART0: to your PC
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);   // UART2: to the GPS
  Serial.println();
  Serial.println("GPS without a library: reading raw NMEA...");
}

void loop() {
  readGPS();                                   // never blocks
  if (millis() - lastPrint >= 3000) {          // Chapter 10 pattern
    lastPrint = millis();
    printSummary();
  }
}
