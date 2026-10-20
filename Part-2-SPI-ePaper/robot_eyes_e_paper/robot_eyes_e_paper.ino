#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// Animated cartoon face on 1.54" e-paper: eyes (iris, lids, brows) + mouth.
// DEMO_MODE 1 = scripted show for filming, 0 = random idle life (use with sensors).
#define DEMO_MODE 1

// Pins: CS=5, DC=17, RST=16, BUSY=4
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(5, 17, 16, 4));

enum Mood { NEUTRAL, HAPPY, SAD, ANGRY, SLEEPY, SURPRISED, LOVE };

const int16_t CX = 100, CY = 100;      // eye line centre
const int16_t MOUTH_Y = 170;           // mouth centre
const int16_t EYE_GAP = 90;            // distance between eye centres
const int16_t R_OUT = 38, R_IN = 33;   // eyeball outline (5px thick)

float   eyeX = 0, eyeY = 0;    // look direction -1..1
uint8_t eyeOpen = 100;         // 0..100 %
Mood    mood = NEUTRAL;
uint8_t partialCount = 0;      // full refresh now and then to clear ghosting

// ---------- drawing ----------
void drawHeart(int16_t cx, int16_t cy, int16_t r) {
  display.fillCircle(cx - r, cy, r, GxEPD_BLACK);
  display.fillCircle(cx + r, cy, r, GxEPD_BLACK);
  display.fillTriangle(cx - 2 * r, cy + 2, cx + 2 * r, cy + 2, cx, cy + 2 * r + r / 2, GxEPD_BLACK);
}

void drawBrow(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {   // 6px thick bar
  display.fillTriangle(x1, y1, x2, y2, x2, y2 + 6, GxEPD_BLACK);
  display.fillTriangle(x1, y1, x1, y1 + 6, x2, y2 + 6, GxEPD_BLACK);
  display.fillCircle(x1, y1 + 3, 3, GxEPD_BLACK);
  display.fillCircle(x2, y2 + 3, 3, GxEPD_BLACK);
}

// side = -1 for left eye, +1 for right eye (points to the outer corner)
void drawEye(int16_t cx, int16_t cy, int8_t side) {
  // eyeball with thick outline
  display.fillCircle(cx, cy, R_OUT, GxEPD_BLACK);
  display.fillCircle(cx, cy, R_IN, GxEPD_WHITE);

  if (mood == HAPPY && eyeOpen > 50) {             // classic ^ ^ smiling eyes
    display.fillRect(cx - R_OUT - 1, cy + 4, 2 * R_OUT + 3, R_OUT, GxEPD_WHITE);
    display.fillCircle(cx - R_OUT + 3, cy + 4, 3, GxEPD_BLACK);   // round the ends
    display.fillCircle(cx + R_OUT - 2, cy + 4, 3, GxEPD_BLACK);
  } else {
    // iris + pupil shine (moves inside the eyeball)
    int16_t ix = cx + eyeX * 13, iy = cy + eyeY * 11;
    if (mood == LOVE) {
      drawHeart(ix, iy - 7, 9);
    } else {
      int16_t ir = (mood == SURPRISED) ? 12 : 18;
      display.fillCircle(ix, iy, ir, GxEPD_BLACK);
      display.fillCircle(ix + ir / 3 + 1, iy - ir / 3 - 1, ir / 3 + 1, GxEPD_WHITE);  // big shine
      display.fillCircle(ix - ir / 3, iy + ir / 3 + 1, 2, GxEPD_WHITE);              // small shine
    }

    // eyelids, drawn column by column so they can be slanted or curved
    float shut = (100 - eyeOpen) / 100.0f;            // 0 = open, 1 = closed
    for (int16_t dx = -R_IN; dx <= R_IN; dx++) {
      int16_t half = sqrt((float)(R_IN * R_IN - dx * dx));
      int16_t top = cy - half, bot = cy + half;
      float up = -R_IN + shut * R_IN;                  // upper lid (blink)
      float lo =  R_IN - shut * R_IN;                  // lower lid (blink)
      switch (mood) {
        case SLEEPY: up = max(up, -2.0f); break;                          // heavy lids
        case ANGRY:  up = max(up, -16.0f - 0.55f * side * dx); break;     // inner side low
        case SAD:    up = max(up, -16.0f + 0.50f * side * dx); break;     // outer side low
        default: break;
      }
      int16_t x = cx + dx, uy = cy + up, ly = cy + lo;
      if (uy > top) {
        display.drawFastVLine(x, top, min(uy, bot) - top, GxEPD_WHITE);
        if (uy < bot) display.drawFastVLine(x, uy - 2, 4, GxEPD_BLACK);   // lid edge
      }
      if (ly < bot) {
        int16_t from = max(ly, top);
        display.drawFastVLine(x, from, bot - from + 1, GxEPD_WHITE);
        if (ly > top) display.drawFastVLine(x, ly - 1, 4, GxEPD_BLACK);
      }
    }
  }

  // eyebrows: (inner y, outer y) above the eye centre
  int8_t inY, outY;
  switch (mood) {
    case ANGRY:     inY = -42; outY = -58; break;
    case SAD:       inY = -60; outY = -46; break;
    case SURPRISED: inY = -64; outY = -62; break;
    case SLEEPY:    inY = -46; outY = -46; break;
    case HAPPY:     inY = -54; outY = -56; break;
    default:        inY = -52; outY = -54; break;
  }
  drawBrow(cx - side * 26, cy + inY, cx + side * 28, cy + outY);
}

void thickArc(int16_t cx, int16_t cy, int16_t r, int16_t a0, int16_t a1) {   // degrees, 0 = right, 90 = down
  for (int16_t a = a0; a <= a1; a += 4) {
    float rad = a * DEG_TO_RAD;
    display.fillCircle(cx + r * cos(rad), cy + r * sin(rad), 2, GxEPD_BLACK);
  }
}

void drawCheeks(int16_t ox, int16_t oy) {          // blush "///" under the eyes
  const int16_t cheekX[] = {18, 160};
  for (int16_t x0 : cheekX)
    for (uint8_t i = 0; i < 3; i++) {
      int16_t x = x0 + ox + i * 8, y = CY + 44 + oy;
      display.drawLine(x, y + 8, x + 5, y, GxEPD_BLACK);
      display.drawLine(x + 1, y + 8, x + 6, y, GxEPD_BLACK);
    }
}

void drawMouth(int16_t mx, int16_t my) {
  switch (mood) {
    case HAPPY:                                    // big open D smile with teeth
      display.fillCircle(mx, my - 10, 22, GxEPD_BLACK);
      display.fillRect(mx - 23, my - 33, 47, 23, GxEPD_WHITE);
      display.fillRect(mx - 14, my - 10, 29, 5, GxEPD_WHITE);
      break;
    case SAD:       thickArc(mx, my + 16, 20, 215, 325); break;      // frown
    case SURPRISED:                                // "O"
      display.fillCircle(mx, my, 13, GxEPD_BLACK);
      display.fillCircle(mx, my, 7, GxEPD_WHITE);
      break;
    case ANGRY:                                    // gritted teeth
      display.fillRoundRect(mx - 28, my - 9, 56, 18, 6, GxEPD_BLACK);
      display.fillRoundRect(mx - 25, my - 6, 50, 12, 4, GxEPD_WHITE);
      display.drawFastHLine(mx - 25, my, 50, GxEPD_BLACK);
      for (int8_t t = -14; t <= 14; t += 14) display.fillRect(mx + t - 1, my - 6, 2, 12, GxEPD_BLACK);
      break;
    case SLEEPY:
      if (eyeOpen <= 25) {                         // snoring "o"
        display.fillCircle(mx, my, 7, GxEPD_BLACK);
        display.fillCircle(mx, my, 3, GxEPD_WHITE);
      } else display.fillRoundRect(mx - 12, my - 2, 24, 5, 2, GxEPD_BLACK);
      break;
    case LOVE:      thickArc(mx, my - 24, 24, 30, 150); break;       // wide smile
    default:        thickArc(mx, my - 20, 20, 35, 145); break;       // gentle smile
  }
}

void render() {
  if (++partialCount >= 80) { display.setFullWindow(); partialCount = 0; }
  else display.setPartialWindow(0, 0, 200, 200);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    int16_t ox = eyeX * 5, oy = eyeY * 4;          // whole face shifts a little too
    drawEye(CX - EYE_GAP / 2 + ox, CY + oy, -1);
    drawEye(CX + EYE_GAP / 2 + ox, CY + oy,  1);
    drawMouth(CX + ox, MOUTH_Y + oy);
    if (mood == HAPPY || mood == LOVE) drawCheeks(ox, oy);
    if (mood == SLEEPY && eyeOpen <= 25) {               // Zzz
      display.setTextColor(GxEPD_BLACK);
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(150, 40); display.print("z");
      display.setFont(&FreeMonoBold18pt7b);
      display.setCursor(166, 28); display.print("Z");
    }
  } while (display.nextPage());
}

// ---------- actions (call these from sensors later) ----------
// Every render() is one e-paper refresh (~0.3s), so each action uses as few frames as possible.
void lookAt(float tx, float ty) { eyeX = tx; eyeY = ty; render(); }   // -1..1 : left/right, up/down

void face(Mood m, float tx, float ty) {  // change mood + look direction in ONE refresh
  mood = m; eyeX = tx; eyeY = ty; render();
}

void blink() {
  uint8_t keep = eyeOpen;
  eyeOpen = 0;  render();
  eyeOpen = keep; render();
}

void setMood(Mood m) { mood = m; render(); }

void fallAsleep() {
  mood = SLEEPY;
  eyeOpen = 30; render();
  eyeOpen = 0;  render();
}

void wakeUp() {
  eyeOpen = 100;
  setMood(SURPRISED);
  delay(300);
  setMood(NEUTRAL);
}

// ---------- sensor hooks (fill in later) ----------
// Return true when a sensor reacted, so the idle/demo behaviour skips this round.
bool checkSensors() {
  // Touch sensor (TTP223) on GPIO 32 -> love eyes:
  //   if (digitalRead(32) == HIGH) { setMood(LOVE); delay(1500); setMood(HAPPY); return true; }
  //
  // PIR motion (HC-SR501) on GPIO 33 -> wakes up surprised when someone walks by:
  //   if (digitalRead(33) == HIGH && mood == SLEEPY) { wakeUp(); return true; }
  //
  // LDR on GPIO 34 (analog) -> sleeps when the room gets dark:
  //   if (analogRead(34) < 500 && mood != SLEEPY) { fallAsleep(); return true; }
  //
  // Ultrasonic HC-SR04 -> too close = angry, close = surprised:
  //   float cm = readDistanceCm();
  //   if (cm < 8)  { setMood(ANGRY);     return true; }
  //   if (cm < 25) { setMood(SURPRISED); return true; }
  return false;
}

// ---------- behaviours ----------
void demoShow() {                         // scripted ~15s for the Short
  eyeOpen = 100;
  face(NEUTRAL, 0, 0);
  blink();
  lookAt(-1, 0);
  lookAt(1, 0);
  face(HAPPY, 0, -0.6);     delay(500);
  face(SURPRISED, 0, 0);    delay(300);
  face(ANGRY, 0.6, 0.3);    delay(500);
  face(SAD, -0.5, 0.8);     delay(500);
  face(LOVE, 0, 0);         delay(600);
  fallAsleep();             delay(800);
  wakeUp();
}

void idleLife() {                         // random, alive-looking behaviour
  uint8_t r = random(100);
  if (r < 45)      lookAt(random(-10, 11) / 10.0f, random(-8, 9) / 10.0f);
  else if (r < 75) blink();
  else if (r < 90) { setMood((Mood)random(0, 4)); }
  else             face(NEUTRAL, 0, 0);
  delay(random(300, 1500));
}

void setup() {
  display.init(115200);
  display.setRotation(1);
  randomSeed(esp_random());
  // pinMode(32, INPUT); pinMode(33, INPUT);   // enable when sensors are wired
  partialCount = 39;                       // first frame = full refresh
  render();
}

void loop() {
  if (checkSensors()) return;
#if DEMO_MODE
  demoShow();
#else
  idleLife();
#endif
}
