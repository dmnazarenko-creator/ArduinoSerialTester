# ArduinoSerialTester

Arduino Nano (ATmega328P) firmware for 2 x 74HC595 (16 outputs) and 2 x 74HC165 (16 inputs).

Hardware SPI transfers 16 bits simultaneously using D13/SCK, D11/MOSI, D12/MISO (SPI mode 0, MSB first). TestMode '1' retains 1 MHz; TestMode '2' uses 125 kHz.

## Wiring

| Nano | Signal | Register pin |
| --- | --- | --- |
| D13 / SCK | Clock | 74HC595 pin 11 (both), 74HC165 pin 2 (both) |
| D11 / MOSI | Data out | First 74HC595 pin 14 |
| D12 / MISO | Data in | Last 74HC165 pin 9 |
| D6 | CE_R | Both 74HC165 pin 15 (active LOW) |
| D7 | LOAD_R | Both 74HC165 pin 1 (active LOW) |
| D8 | LOAD_W | Both 74HC595 pin 12 (rising-edge latch) |
| D9 | CLR_W | Both 74HC595 pin 10 (active LOW) |
| D10 | OE_W | Both 74HC595 pin 13 (active LOW) |

Daisy-chain: 595 #1 pin 9 -> 595 #2 pin 14. 165 #1 pin 9 -> 165 #2 pin 10; 165 #2 pin 9 -> Nano D12; 165 #1 pin 10 -> GND. All devices: pin 16 -> +5 V and pin 8 -> GND. Add 100 nF per IC close to VCC/GND. Inputs of 165 must not float.

## Serial testing

Open Serial Monitor at **115200 baud**. Send `A55A` and newline. Firmware writes 0xA55A to the outputs and reads the 16 input bits over the same 16 SPI clocks, printing `TX=0xA55A RX=0x....`.

Target board: classic 5 V Arduino Nano ATmega328P. Compilation and hardware verification are pending.

## Compile-time test modes

Select the test algorithm in `src/ArduinoSerialTester/ArduinoSerialTester.ino`:

```cpp
#define TestMode '1'  // existing one-shot loopback test
// or:
#define TestMode '2'  // repeat the same test forever
```

- **Mode '1' (default):** run one full loopback test in `setup()`. Afterwards the
  original serial HEX-command interface remains available in `loop()`.
- **Mode '2':** one full-duplex 16-bit SPI transfer and one serial report **every 50 ms**, repeatedly, even when inputs disagree. A sweep uses four patterns (`A55A, 5AA5, FFFF, 0000`) followed by 16 walking-one patterns, each followed by zero (36 transfers total). The SPI clock is **125 kHz** for this mode only.
  Every report includes `TX`, `EXPECTED` (previous TX, because 165 captures the previous 595 outputs), `RX`, `DIFF=EXPECTED XOR RX`, erroneous bit indices (`0..15`), and cumulative `ERRORS`. `MODE2_ERROR` is logged on mismatch without stopping SPI.
  Example: `MODE2_SAMPLE=2 TX=0x5AA5 EXPECTED=0xA55A RX=0xA55A DIFF=0x0000 BITS=none ERRORS=0`.
- Each test includes the previous-word readback latency of the 595/165 pipeline.
  The last 595 output is zero, so cycles start from the same defined state.
- Both modes are compiled, native-tested, and simulated in Wokwi by GitHub Actions.
  Wokwi checks at least 37 consecutive samples (one entire 36-step sweep plus restart) in mode '2'.

Use `-DTestMode=50` as a compiler override for mode '2' (ASCII value of `'2'`).
The firmware itself uses character constants `'1'` and `'2'`.
