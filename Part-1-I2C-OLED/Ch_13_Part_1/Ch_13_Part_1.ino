/*
  ============================================================
       IoTBhai — Chapter 13.1 | I²C OLED Experiment
       SH1106 1.3" I²C OLED + ESP32

       Experiment:
       "Scan the Bus, Find the OLED, Draw on the Display"

       OLED: 1.3" SH1106 128x64 I²C
       Library: U8g2

       Typical I²C address:
       0x3C

       ESP32 default I²C:
       SDA = GPIO 21
       SCL = GPIO 22
  ============================================================
*/

#include <Wire.h>
#include <U8g2lib.h>

// ============================================================
// OLED
// ============================================================

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  /* reset = */ U8X8_PIN_NONE
);

// ============================================================
// I²C configuration
// ============================================================

#define SDA_PIN 21
#define SCL_PIN 22

uint8_t oledAddress = 0x3C;

// ============================================================
// Colors / timing
// ============================================================

unsigned long lastFrame = 0;
int progress = 0;

// ============================================================
// Helper: center text
// ============================================================

void drawCentered(const char *text, int y)
{
  int width = oled.getStrWidth(text);
  oled.drawStr((128 - width) / 2, y, text);
}

// ============================================================
// Boot screen
// ============================================================

void bootAnimation()
{
  const char *steps[] = {
    "POWER ON",
    "STARTING I2C",
    "SCANNING BUS",
    "OLED FOUND",
    "READY!"
  };

  for (int i = 0; i < 5; i++)
  {
    oled.clearBuffer();

    // Header
    oled.setFont(u8g2_font_6x10_tf);
    drawCentered("IoTBhai", 11);

    oled.drawHLine(10, 15, 108);

    // Main message
    oled.setFont(u8g2_font_helvB12_tf);
    drawCentered(steps[i], 37);

    // Progress bar
    oled.drawFrame(18, 49, 92, 9);

    int barWidth = map(i, 0, 4, 0, 88);
    oled.drawBox(20, 51, barWidth, 5);

    oled.sendBuffer();

    delay(500);
  }

  delay(300);
}

// ============================================================
// I²C bus scanner
// ============================================================

void scanI2CBus()
{
  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);

  drawCentered("I2C BUS SCAN", 10);

  oled.drawHLine(8, 14, 112);

  oled.setFont(u8g2_font_5x8_tf);

  // Animated scanning
  for (int address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    // Every 16 addresses show progress
    if (address % 8 == 0)
    {
      oled.clearBuffer();

      drawCentered("I2C BUS SCAN", 10);
      oled.drawHLine(8, 14, 112);

      char buffer[20];

      sprintf(buffer, "Checking 0x%02X", address);

      oled.setFont(u8g2_font_6x10_tf);
      drawCentered(buffer, 31);

      // Scan bar
      oled.drawFrame(10, 43, 108, 10);

      int width = map(address, 1, 127, 0, 104);

      oled.drawBox(12, 45, width, 6);

      oled.setFont(u8g2_font_5x8_tf);
      drawCentered("Searching the bus...", 62);

      oled.sendBuffer();

      delay(70);
    }

    if (error == 0)
    {
      if (address == oledAddress)
      {
        // OLED found
        oled.clearBuffer();

        oled.setFont(u8g2_font_6x10_tf);
        drawCentered("DEVICE FOUND", 12);

        oled.drawHLine(8, 17, 112);

        oled.setFont(u8g2_font_helvB14_tf);

        char addrText[10];
        sprintf(addrText, "0x%02X", address);

        drawCentered(addrText, 40);

        oled.setFont(u8g2_font_5x8_tf);
        drawCentered("OLED is answering!", 57);

        oled.sendBuffer();

        delay(1200);
      }
    }
  }
}

// ============================================================
// Address lesson
// ============================================================

void showAddress()
{
  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);
  drawCentered("MEET THE OLED", 10);

  oled.drawHLine(8, 14, 112);

  oled.setFont(u8g2_font_helvB18_tf);
  drawCentered("0x3C", 39);

  oled.setFont(u8g2_font_5x8_tf);
  drawCentered("7-bit I2C ADDRESS", 55);

  oled.sendBuffer();

  delay(1800);
}

// ============================================================
// I²C packet animation
// ============================================================

void showPacket()
{
  const char *labels[] = {
    "START",
    "ADDRESS",
    "WRITE",
    "DATA",
    "STOP"
  };

  for (int step = 0; step < 5; step++)
  {
    oled.clearBuffer();

    oled.setFont(u8g2_font_6x10_tf);
    drawCentered("I2C MESSAGE", 10);

    oled.drawHLine(8, 14, 112);

    // Packet blocks
    int x = 5;

    for (int i = 0; i < 5; i++)
    {
      int w = 22;

      if (i == step)
      {
        oled.drawBox(x, 25, w, 18);
        oled.setDrawColor(0);
      }
      else
      {
        oled.drawFrame(x, 25, w, 18);
      }

      oled.setFont(u8g2_font_4x6_tf);

      int textWidth = oled.getStrWidth(labels[i]);

      oled.drawStr(
        x + (w - textWidth) / 2,
        36,
        labels[i]
      );

      oled.setDrawColor(1);

      x += 25;
    }

    oled.setFont(u8g2_font_5x8_tf);

    char stepText[20];
    sprintf(stepText, "Step %d / 5", step + 1);

    drawCentered(stepText, 56);

    oled.sendBuffer();

    delay(650);
  }

  delay(500);
}

// ============================================================
// Bus visualization
// ============================================================

void showBus()
{
  for (int frame = 0; frame < 45; frame++)
  {
    oled.clearBuffer();

    oled.setFont(u8g2_font_6x10_tf);
    drawCentered("THE I2C BUS", 10);

    oled.drawHLine(8, 14, 112);

    // SDA
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(2, 28, "SDA");

    oled.drawHLine(22, 26, 104);

    // SCL
    oled.drawStr(2, 46, "SCL");

    oled.drawHLine(22, 44, 104);

    // Moving data pulse
    int x = 22 + (frame * 3) % 100;

    oled.drawBox(x, 23, 5, 6);
    oled.drawBox(x, 41, 5, 6);

    // OLED
    oled.drawFrame(42, 50, 44, 12);

    oled.setFont(u8g2_font_4x6_tf);
    oled.drawStr(51, 58, "OLED");

    oled.sendBuffer();

    delay(35);
  }
}

// ============================================================
// Final dashboard
// ============================================================

void showDashboard()
{
  for (int frame = 0; frame < 80; frame++)
  {
    oled.clearBuffer();

    // Header
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(3, 9, "IoTBhai");
    oled.drawStr(79, 9, "I2C DEMO");

    oled.drawHLine(2, 13, 124);

    // Address
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(3, 25, "OLED");

    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(35, 25, "SH1106");

    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(91, 25, "0x3C");

    // Bus
    oled.drawStr(3, 38, "BUS");

    oled.drawStr(35, 38, "SDA 21");
    oled.drawStr(84, 38, "SCL 22");

    // Animated activity
    oled.drawStr(3, 51, "STATUS");

    if ((frame / 8) % 2 == 0)
    {
      oled.drawDisc(38, 48, 3);
      oled.drawStr(45, 51, "ONLINE");
    }
    else
    {
      oled.drawCircle(38, 48, 3);
      oled.drawStr(45, 51, "ONLINE");
    }

    // Packet counter
    oled.setFont(u8g2_font_5x8_tf);

    char packets[20];
    sprintf(packets, "PACKETS %03d", frame);

    oled.drawStr(72, 51, packets);

    // Footer
    oled.drawHLine(2, 55, 124);

    oled.setFont(u8g2_font_4x6_tf);
    drawCentered("TWO WIRES. MANY DEVICES.", 63);

    oled.sendBuffer();

    delay(80);
  }
}

// ============================================================
// Setup
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(500);

  // Start I²C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Start OLED
  oled.begin();

  // Set contrast
  oled.setContrast(255);

  // Boot animation
  bootAnimation();

  // Scan the I²C bus
  scanI2CBus();

  // Show discovered address
  showAddress();

  // Explain the I²C frame
  showPacket();

  // Visualize SDA + SCL
  showBus();

  // Final dashboard
  showDashboard();
}

// ============================================================
// Loop
// ============================================================

void loop()
{
  // Keep the experiment alive.
  // Every few seconds replay the dashboard animation.

  showDashboard();

  delay(500);
}