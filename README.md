# ESP32 Communication Protocols: I2C, SPI, UART and One-Wire

Arduino code for **Chapter 13: Hardware Communication Protocols** of the **IoTBhai Full-Stack IoT Engineer Bootcamp**.

Each part of the chapter is a separate video and a separate folder. Each experiment is standalone: wire it, upload it, and it works.

| Part | Protocol | Experiment | Code |
|---|---|---|---|
| 13.1 | **I2C**: "The Roll Call" | Scan the bus, then draw on a 1.3" SH1106 OLED | [`Part-1-I2C-OLED`](Part-1-I2C-OLED) |
| 13.2 | **SPI**: "The Tap on the Shoulder" | Animated robot eyes on a Waveshare 1.54" e-Paper | [`Part-2-SPI-ePaper`](Part-2-SPI-ePaper) |
| 13.3 | UART: "The Agreed Speed" | NEO-M8N GPS module | Coming soon |
| 13.4 | One-Wire: "The Serial Number" | Two DS18B20 temperature sensors on one pin | Coming soon |

---

## Part 13.1: ESP32 I2C with a 1.3" SH1106 OLED

I2C uses just two wires, **SDA** (data) and **SCL** (clock), shared by every device on the bus. Each device has a 7-bit address and only answers when the ESP32 calls that address, like a student answering a roll call.

### What you need

| Part | Notes |
|---|---|
| ESP32 DevKit | Any ESP32 dev board with GPIO21 and GPIO22 |
| 1.3" OLED, SH1106, 128×64, I2C | The 4-pin version (GND, VCC, SCL, SDA) |
| 4 jumper wires | Female-to-female, or a breadboard |
| USB data cable | A charge-only cable won't upload |

### Wiring

| OLED pin | ESP32 pin |
|---|---|
| GND | GND |
| VCC | 3V3 |
| SCL | GPIO22 |
| SDA | GPIO21 |

> **Read the pin labels.** Some 1.3" boards put VCC first, others GND. Wire by name, not by position.

### Software

1. **Arduino IDE** with the **ESP32 board package** (core 3.x) installed.
2. Board: **ESP32 Dev Module**.
3. Library: install **U8g2** by *olikraus* from the Library Manager. The scanner needs no extra library.
4. Serial Monitor: **115200 baud**.

### The sketches

Run them in this order.

#### 1. [`scan_i2c_address`](Part-1-I2C-OLED/scan_i2c_address/scan_i2c_address.ino): find the address

The ESP32 calls every address from 1 to 126. `Wire.endTransmission()` returns `0` only when a device answers with an ACK.

Expected Serial Monitor output:

```
Scanning...
Found device at 0x3C
Done.
```

**Always scan first.** I2C fails silently: if the address in your code is wrong, nothing crashes. The screen just stays dark.

#### 2. [`Ch_13_Part_1`](Part-1-I2C-OLED/Ch_13_Part_1/Ch_13_Part_1.ino): the OLED experiment

"Scan the bus, find the OLED, draw on the display." On power-up the OLED plays through:

1. **Boot screen:** POWER ON → STARTING I2C → SCANNING BUS → OLED FOUND → READY!
2. **Live bus scan:** checks addresses 0x01 to 0x7E with a progress bar, then shows **DEVICE FOUND 0x3C**.
3. **Meet the OLED:** its 7-bit I2C address.
4. **I2C message:** START → ADDRESS → WRITE → DATA → STOP, one block at a time.
5. **The I2C bus:** data pulses moving along SDA and SCL.
6. **Dashboard:** device, address, pins and a live packet counter. This screen repeats forever.

The display uses this U8g2 constructor:

```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
```

### Troubleshooting

| Problem | Likely cause | Fix |
|---|---|---|
| Scanner finds nothing | Wiring or power | Check SDA → 21, SCL → 22, VCC → 3V3, GND → GND |
| Scanner finds **0x3D** | Address jumper set to the alternate address | Add `oled.setI2CAddress(0x3D * 2);` before `oled.begin();` |
| Board label says **0x78** | That's the address × 2 | It is 0x3C. No change needed |
| Noisy stripe down one edge | SSD1306 driver used on an SH1106 screen | Use the `U8G2_SH1106_...` constructor above |
| Blank screen, scanner finds 0x3C | Address in code doesn't match | U8g2 wants the address × 2: `0x3C * 2` |
| Upload fails | Charge-only USB cable | Use a data cable |

### Key ideas

- **Two shared wires, many devices.** Up to 112 usable 7-bit addresses on one bus.
- **Open-drain + pull-ups.** Devices can only pull a line LOW. Pull-up resistors (already on most OLED boards) pull it HIGH.
- **Speed:** 100 kHz standard, 400 kHz fast mode.
- **Short distances:** on the board, under about 1 m.

---

## Part 13.2: ESP32 SPI with a Waveshare 1.54" e-Paper

SPI picks a device by **which wire you pull**, not by an address. The ESP32 pulls one device's **CS** (chip select) pin LOW, like a tap on the shoulder, then clocks the data out fast on its own clock line. There are no addresses and no ACKs, which is why SPI runs at MHz speeds.

### What you need

| Part | Notes |
|---|---|
| ESP32 DevKit | Any ESP32 dev board |
| Waveshare 1.54" e-Paper **V2** | 200×200, black and white, 8-pin SPI module. Check the sticker for "V2" |
| 8 jumper wires | Female-to-female, or a breadboard |
| USB data cable | A charge-only cable won't upload |

### Wiring

| e-Paper pin | ESP32 pin | What it does |
|---|---|---|
| VCC | 3V3 | Power |
| GND | GND | Ground |
| DIN | GPIO23 | SPI data in (MOSI) |
| CLK | GPIO18 | SPI clock (SCK) |
| CS | GPIO5 | Chip select |
| DC | GPIO17 | Data or command |
| RST | GPIO16 | Hardware reset |
| BUSY | GPIO4 | Screen says "still refreshing, wait" |

Only DIN, CLK and CS are SPI. DC, RST and BUSY are extra control pins the display needs. There's no MISO: the e-Paper never sends data back.

> **WROVER board?** GPIO16 and 17 are used by its PSRAM. Move RST and DC to free pins and change the numbers in the constructor.

### Software

1. **Arduino IDE** with the **ESP32 board package** (core 3.x) installed.
2. Board: **ESP32 Dev Module**.
3. Libraries: install **GxEPD2** by *Jean-Marc Zingg* and **Adafruit GFX Library** from the Library Manager.

### The sketch

#### [`robot_eyes_e_paper`](Part-2-SPI-ePaper/robot_eyes_e_paper/robot_eyes_e_paper.ino): give the e-Paper a face

An animated cartoon face: eyes with irises, eyelids and eyebrows, plus a mouth. It cycles through moods: neutral, blink, look left and right, happy, surprised, angry, sad, love, falls asleep ("Zz"), then wakes up.

- **Partial refresh** draws each new face in about 0.3 s with no flash.
- **A full refresh every 80 frames** wipes the faint "ghosts" that partial refreshes leave behind.
- `#define DEMO_MODE 1` plays a scripted show. Set it to `0` for random, alive-looking behaviour.
- `checkSensors()` has ready-made hooks (commented out) for a touch sensor, PIR, LDR or ultrasonic sensor to change the mood.

The display uses this GxEPD2 constructor (pins: CS, DC, RST, BUSY):

```cpp
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(5, 17, 16, 4));
```

### Troubleshooting

| Problem | Likely cause | Fix |
|---|---|---|
| Garbage or nothing on screen | Wrong driver for your panel version | V2 uses `GxEPD2_154_D67`. The older V1 uses `GxEPD2_154` |
| Serial Monitor says `Busy Timeout!` | BUSY wire missing or on the wrong pin | Check BUSY → GPIO4, and the last number in the constructor |
| Screen flashes black and white | That's a full refresh | Normal for e-Paper. It happens on start-up and every 80 frames |
| Faint grey shapes from old faces | Ghosting from partial refresh | Normal. The next full refresh clears it |
| Upload fails or the board resets on a WROVER | GPIO16/17 are taken by PSRAM | Move RST and DC to free pins |

### Key ideas

- **Every SPI transfer is a swap.** On each clock tick one bit goes out on MOSI and one comes back on MISO.
- **Wires:** SCK, MOSI and MISO are shared by every device. Each device gets its own CS wire.
- **Speed:** MHz. The e-Paper runs at 4 MHz. The ESP32 can go up to 80 MHz.
- **Short distances:** on the board, a few cm up to about 1 m.
- **e-Paper keeps its image with no power.** `hibernate()` cuts the display's power and the picture stays.

---

## About

Made for the **IoTBhai Full-Stack IoT Engineer Bootcamp**, Phase 1.
Tested on an ESP32 DevKit with ESP32 Arduino core 3.x, a 1.3" SH1106 I2C OLED and a Waveshare 1.54" e-Paper V2.

## License

[MIT](LICENSE). Use it, change it and share it in your own projects.
