# ArduinoSerialTester

Arduino Nano (ATmega328P) firmware for 2 x 74HC595 (16 outputs) and 2 x 74HC165 (16 inputs).

Hardware SPI transfers 16 bits simultaneously using D13/SCK, D11/MOSI, D12/MISO at 1 MHz (SPI mode 0, MSB first).

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
