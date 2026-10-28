# ESP32 Communication Protocols: I2C, SPI, UART and One-Wire

Arduino code for **Chapter 13: Hardware Communication Protocols** of the **IoTBhai Full-Stack IoT Engineer Bootcamp**.

Each part of the chapter is a separate video and a separate folder. Each experiment is standalone: wire it, upload it, and it works.

| Part | Protocol | Experiment | Code |
|---|---|---|---|
| 13.1 | **I2C**: "The Roll Call" | Scan the bus, then draw on a 1.3" SH1106 OLED | [`Part-1-I2C-OLED`](Part-1-I2C-OLED) |
| 13.2 | **SPI**: "The Tap on the Shoulder" | Animated robot eyes on a Waveshare 1.54" e-Paper | [`Part-2-SPI-ePaper`](Part-2-SPI-ePaper) |
| 13.3 | **UART**: "The Agreed Speed" | Read a NEO-M8N GPS, with and without a library | [`Part-3-UART-GPS`](Part-3-UART-GPS) |
| 13.4 | **One-Wire**: "The Serial Number" | Find a DS18B20's 64-bit ID, then read the temperature | [`Part-4-OneWire-DS18B20`](Part-4-OneWire-DS18B20) |

---

## Part 13.1: ESP32 I2C with a 1.3" SH1106 OLED

I2C uses just two wires, **SDA** (data) and **SCL** (clock), shared by every device on the bus. Each device has a 7-bit address and only answers when the ESP32 calls that address, like a student answering a roll call.

### What you need

| Part | Notes |
|---|---|
| ESP32 DevKit | Any ESP32 dev board with GPIO21 and GPIO22. Buy: [AliExpress](https://s.click.aliexpress.com/e/_c2IAMVUX) · [Amazon](https://amzn.to/493Wv5A) |
| 1.3" OLED, SH1106, 128×64, I2C | The 4-pin version (GND, VCC, SCL, SDA). Buy: [AliExpress](https://s.click.aliexpress.com/e/_c4pvQOon) · [Amazon](https://link.amazon/B00cJwU1y) |
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
| ESP32 DevKit | Any ESP32 dev board. Buy: [AliExpress](https://s.click.aliexpress.com/e/_c2IAMVUX) · [Amazon](https://amzn.to/493Wv5A) |
| Waveshare 1.54" e-Paper **V2** | 200×200, black and white, 8-pin SPI module. Check the sticker for "V2". Buy: [AliExpress](https://s.click.aliexpress.com/e/_c4q44gOb) · [Amazon](https://link.amazon/B00VT3Mpw) |
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
- **Reading takes two bytes: ask, then listen.** The device can't answer a command it hasn't finished hearing. Byte 1 sends the command, and what comes back is junk, so ignore it. Byte 2 sends a dummy byte just to make 8 clock ticks, and the answer comes back on MISO. Keep CS LOW across both bytes:

  ```cpp
  digitalWrite(CS, LOW);
  SPI.transfer(0x8F);                // ask: "read register 0x0F". Ignore what comes back
  uint8_t id = SPI.transfer(0x00);   // dummy byte out, the answer comes back in
  digitalWrite(CS, HIGH);
  ```

  In short: **write = command + data**, **read = command + dummy**. Our e-Paper only ever writes, so it has no MISO pin.
- **Wires:** SCK, MOSI and MISO are shared by every device. Each device gets its own CS wire.
- **Speed:** MHz. The e-Paper runs at 4 MHz. The ESP32 can go up to 80 MHz.
- **Short distances:** on the board, a few cm up to about 1 m.
- **e-Paper keeps its image with no power.** `hibernate()` cuts the display's power and the picture stays.

---

## Part 13.3: ESP32 UART with a NEO-M8N GPS

UART has **no clock wire**. Both sides agree on a speed in advance, the **baud rate**. At 9600 baud every bit lasts 104 µs. The receiver waits for the falling edge of the start bit, then reads each bit in its middle, timed by its own clock. You already know UART: it's what `Serial.begin(115200)` has been using since Chapter 6.

### What you need

| Part | Notes |
|---|---|
| ESP32 DevKit | Any ESP32 dev board. Buy: [AliExpress](https://s.click.aliexpress.com/e/_c2IAMVUX) · [Amazon](https://amzn.to/493Wv5A) |
| u-blox NEO-M8N GPS module | With its antenna. Default speed: 9600 baud. Buy: [AliExpress](https://s.click.aliexpress.com/e/_c3tp9i1t) · [Amazon](https://link.amazon/B06Ttmrpk) |
| 4 jumper wires | Female-to-female, or a breadboard |
| USB data cable | A charge-only cable won't upload |
| A window | GPS needs a view of the sky for a fix |

### Wiring

| GPS pin | ESP32 pin | Notes |
|---|---|---|
| VCC | VIN | 5 V from USB. The GPS board has its own regulator |
| GND | GND | The shared reference for HIGH and LOW |
| TX | GPIO26 | ESP32 RX2: the GPS talks, the ESP32 listens |
| RX | GPIO27 | ESP32 TX2: the ESP32 talks, the GPS listens |

> **Cross the wires:** TX always goes to the other side's RX. TX to TX gives total silence.

GPIO26 and 27 work on every ESP32, including WROVER boards, which use GPIO16 and 17 for PSRAM. The ESP32 can put its hardware UART on almost any pins, so you don't need SoftwareSerial:

```cpp
Serial2.begin(9600, SERIAL_8N1, 26, 27);   // baud, 8 data bits / no parity / 1 stop bit, RX, TX
```

### Software

1. **Arduino IDE** with the **ESP32 board package** (core 3.x) installed.
2. Board: **ESP32 Dev Module**.
3. Library: install **TinyGPSPlus** by *Mikal Hart* from the Library Manager. It's only needed for sketch 2.
4. Serial Monitor: **115200 baud**. That's UART0 to your PC. The GPS uses UART2 at 9600.

### The sketches

Both print a report every 3 seconds without `delay()`: UTC and local time, date, satellites, latitude and longitude, speed and a Google Maps link. Set `UTC_OFFSET` for your city (Dhaka is 6).

#### 1. [`gps_no_library`](Part-3-UART-GPS/gps_no_library/gps_no_library.ino): decode NMEA by hand

The GPS sends plain text sentences like this:

```
$GNRMC,081642.00,A,2346.84210,N,09024.61560,E,0.12,,290926,,,A*55
```

The sketch does everything itself:
- Collects bytes into lines without blocking. `$` starts a sentence and a newline ends it.
- Prints every raw sentence (`SHOW_RAW true`).
- **Checks the checksum:** XOR of every character between `$` and `*` must equal the two hex digits after `*`. Sentences with a flipped bit are dropped.
- **Decodes `$GxRMC`:** time, fix status (`A` = valid, `V` = no fix), position, speed and date.
- **Decodes `$GxGGA`:** the number of satellites.
- **Converts positions:** NMEA gives `ddmm.mmmm`, so `2346.84210` = 23° + 46.84210′ ÷ 60 = **23.780702°**.

#### 2. [`gps_with_library`](Part-3-UART-GPS/gps_with_library/gps_with_library.ino): let TinyGPSPlus do it

The same result in far less code. Feed every byte to `gps.encode()`, then ask for the values: `gps.location.lat()`, `gps.time.hour()` and so on. You also get altitude and HDOP (accuracy) for free.

### Troubleshooting

Both sketches detect the two classic UART faults and print a hint in the Serial Monitor:

| Symptom | Cause | Fix |
|---|---|---|
| **Garbage characters**, or "Bytes arrive, but no valid sentence yet" | Wrong baud rate. Bytes arrive but are read at the wrong moments | Use 9600, the NEO-M8N default. Try `GPS_BAUD 4800` to see the garbage on purpose |
| **Total silence**, or "Silence: no bytes from the GPS" | TX and RX not crossed (TX to TX), or no shared GND | GPS TX → GPIO26, GPS RX → GPIO27, GND → GND |
| "Fix none yet" / status `V` | No view of the sky | Move near a window. A cold start can take 30 s or more. Indoors at a desk, maybe never |
| Wrong local time | Time zone | GPS time is always UTC. Change `UTC_OFFSET` |

### Key ideas

- **No clock wire:** both sides agree on the baud rate. Each character starts with a start bit that restarts the receiver's stopwatch.
- **Bits go lowest first.** "G" (0x47 = 01000111) goes out as 1 1 1 0 0 0 1 0. The UART hardware reverses it for you.
- **8N1** = 8 data bits, no parity, 1 stop bit. That's 10 bits per character, so 9600 baud carries 960 characters per second.
- **One partner per port:** no addresses and no chip select. Need another device? Use another UART. The ESP32 has three.
- **Wires:** TX and RX, crossed over, plus a shared GND. Good for a few meters at 3.3 V logic levels.

---

## Part 13.4: ESP32 One-Wire with a DS18B20 temperature sensor

One-Wire uses **one data wire** (DQ), shared by every sensor. There's no clock wire and no address you set: each DS18B20 carries a unique **64-bit ID burned in at the factory**, and the ESP32 picks a sensor by that ID. Many sensors can share one pin and one resistor, over tens of meters of cable.

### What you need

| Part | Notes |
|---|---|
| ESP32 DevKit | Any ESP32 dev board. Buy: [AliExpress](https://s.click.aliexpress.com/e/_c2IAMVUX) · [Amazon](https://amzn.to/493Wv5A) |
| DS18B20 temperature sensor | The TO-92 chip or the waterproof probe. Both work the same |
| 4.7 kΩ resistor | The pull-up. **Don't skip it** |
| Breadboard + 3 jumper wires | |
| USB data cable | A charge-only cable won't upload |

### Wiring

| DS18B20 pin | ESP32 pin | Waterproof probe wire |
|---|---|---|
| VDD | 3V3 | Red |
| GND | GND | Black |
| DQ | GPIO27 (D27) | Yellow (sometimes white) |

**Plus a 4.7 kΩ resistor between DQ and 3V3.** It works the same way as on I2C: sensors can only pull the line LOW, and the resistor pulls it back HIGH. One resistor serves the whole bus.

> **Want more sensors?** Connect them to the same three wires: same pin, still one resistor. Both sketches loop over every sensor they find, so a second one simply shows up.

### Software

1. **Arduino IDE** with the **ESP32 board package** (core 3.x) installed.
2. Board: **ESP32 Dev Module**.
3. Libraries: install **OneWire** by *Paul Stoffregen* and **DallasTemperature** by *Miles Burton* from the Library Manager.
4. Serial Monitor: **115200 baud**.

### The sketches

Run them in this order.

#### 1. [`ds18b20_find_id`](Part-4-OneWire-DS18B20/ds18b20_find_id/ds18b20_find_id.ino): ask the bus who's out there

`sensors.begin()` searches the bus and finds every sensor's 64-bit ID. The sketch prints each one and **checks its CRC**:

```
Found 1 sensor(s) on GPIO27
Sensor 0: 28 61 64 12 3C 4A 1F 1E (CRC OK)
```

Your ID will be different. Every DS18B20 has its own. How to read it:

| Byte | Example | Meaning |
|---|---|---|
| 0 | `28` | Family code: "I'm a DS18B20" |
| 1 to 6 | `61 64 12 3C 4A 1F` | 48-bit serial number, laser-burned at the factory |
| 7 | `1E` | CRC, calculated from bytes 0 to 6. If a bit flips on a long cable, the ESP32 notices |

#### 2. [`ds18b20_read_temp`](Part-4-OneWire-DS18B20/ds18b20_read_temp/ds18b20_read_temp.ino): read the temperature without freezing the loop

A 12-bit reading (0.0625 °C steps) takes up to **750 ms**. With `delay(750)` the ESP32 would freeze for three quarters of every second. Instead, the sketch starts a conversion with `setWaitForConversion(false)` and comes back with `millis()` when it's ready, so `loop()` stays free:

```
Sensor 0: 27.44 C
Sensor 0: 27.50 C
```

Hold the sensor between your fingers and watch the number climb.

| Resolution | Steps | Time per reading |
|---|---|---|
| 9-bit | 0.5 °C | 94 ms |
| 10-bit | 0.25 °C | 188 ms |
| 11-bit | 0.125 °C | 375 ms |
| **12-bit** | **0.0625 °C** | **750 ms** (this sketch) |

### Troubleshooting

| You see | Cause | Fix |
|---|---|---|
| **`-127.00`** | The sensor didn't answer | Check DQ → GPIO27, VDD, GND and, above all, the **4.7 kΩ pull-up** between DQ and 3V3 |
| **`85.00`** | Read before the conversion finished. That's the sensor's power-on value | Wait the full conversion time, as the sketch does with `millis()` |
| `Found 0 sensor(s)` | Wiring, or the sensor is in backwards | TO-92 flat side facing you: GND, DQ, VDD from left to right. Backwards it gets hot |
| `CRC BAD` | Noise on a long cable | Shorter cable, or a stronger pull-up (e.g. 2.2 kΩ) on long runs |

### Key ideas

- **One data wire:** DQ, plus GND and power. The DS18B20 can even steal power from DQ ("parasite power"), but normal 3-wire power is more reliable.
- **Timing is the language:** a long LOW is a reset, the sensors answer with a "presence" pulse, a short LOW is a 1 and a long LOW is a 0.
- **Every sensor has a 64-bit ID:** family code + 48-bit serial + CRC. The ESP32 picks a sensor by its ID (Match ROM, 0x55).
- **The search game (Search ROM, 0xF0):** bit by bit, every sensor says its bit, then the opposite. Because LOW always wins, the ESP32 can tell "everyone has 0", "everyone has 1" or "they disagree". It follows one branch to find one ID, then searches again for the next.
- **Speed:** about 16 kbit/s. Slow, but plenty for temperature, and it works over **tens of meters** of cable.

---

## Chapter 13 complete

| | I2C | SPI | UART | One-Wire |
|---|---|---|---|---|
| **Signal wires** | 2 (SDA + SCL) | 4+ (SCK, MOSI, MISO + 1 CS each) | 2 (TX + RX) | 1 (DQ) |
| **Speed** | 100 / 400 kHz | MHz (up to 80 on ESP32) | 9600–115200 baud | ~16 kbit/s |
| **Cable length** | Short (under ~1 m) | Short (on the board) | A few meters | Long (tens of meters) |
| **Picks a device by** | 7-bit address | CS pin | Nobody: one partner per port | 64-bit ROM ID |
| **Our device** | 1.3" SH1106 OLED | Waveshare 1.54" e-Paper | NEO-M8N GPS | DS18B20 |

**The pattern:** more wires buy you speed. Fewer wires buy you distance and sharing.

Next: **Chapter 14, Working with Actuators.** The ESP32 can talk; now it starts moving things: relays, DC motors and servos.

---

## About

Made for the **IoTBhai Full-Stack IoT Engineer Bootcamp**, Phase 1.
Tested on an ESP32 DevKit with ESP32 Arduino core 3.x, a 1.3" SH1106 I2C OLED and a Waveshare 1.54" e-Paper V2.

Some "Buy" links in this README are affiliate links. Buying through them supports the channel at no extra cost to you.
More recommended parts: [amazon.com/shop/iotbhai](https://www.amazon.com/shop/iotbhai)

## License

[MIT](LICENSE). Use it, change it and share it in your own projects.
