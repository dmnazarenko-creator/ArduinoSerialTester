# Wokwi 16-bit loopback diagram

The wiring is defined in [diagram.json](diagram.json). It uses an Arduino Nano,
two cascaded 74HC595 output shift registers, and two cascaded 74HC165 input
shift registers.

## Exact bit-to-bit loopback

| Word bit | Source output | Destination input |
| ---: | --- | --- |
| 0 | 74HC595 #1 Q0 | 74HC165 #1 D0 |
| 1 | 74HC595 #1 Q1 | 74HC165 #1 D1 |
| 2 | 74HC595 #1 Q2 | 74HC165 #1 D2 |
| 3 | 74HC595 #1 Q3 | 74HC165 #1 D3 |
| 4 | 74HC595 #1 Q4 | 74HC165 #1 D4 |
| 5 | 74HC595 #1 Q5 | 74HC165 #1 D5 |
| 6 | 74HC595 #1 Q6 | 74HC165 #1 D6 |
| 7 | 74HC595 #1 Q7 | 74HC165 #1 D7 |
| 8 | 74HC595 #2 Q0 | 74HC165 #2 D0 |
| 9 | 74HC595 #2 Q1 | 74HC165 #2 D1 |
| 10 | 74HC595 #2 Q2 | 74HC165 #2 D2 |
| 11 | 74HC595 #2 Q3 | 74HC165 #2 D3 |
| 12 | 74HC595 #2 Q4 | 74HC165 #2 D4 |
| 13 | 74HC595 #2 Q5 | 74HC165 #2 D5 |
| 14 | 74HC595 #2 Q6 | 74HC165 #2 D6 |
| 15 | 74HC595 #2 Q7 | 74HC165 #2 D7 |

The cascade order matters:

- MOSI -> 595 #1 DS; 595 #1 Q7S -> 595 #2 DS.
- 165 #1 Q7 -> 165 #2 DS; 165 #2 Q7 -> MISO.
- On an MSB-first 16-bit transfer, 595 #1 holds bits 0..7 and #2
  holds bits 8..15. 165 #1 contributes bits 0..7 and #2 bits 8..15.
- Both pairs share their respective clock, load/latch, clear/enable signals.
- The first 165 DS input is tied to GND, and all four ICs are powered.

## Expected loopback sequence

Inputs are parallel-loaded **before** the outgoing word is latched into
the 595 outputs. Thus RX represents the **previous** TX word.

| Serial command | Expected RX |
| --- | --- |
| A55A | 0000 (after power-up/reset) |
| 5AA5 | A55A |
| FFFF | 5AA5 |
| 0000 | FFFF |
| 0000 | 0000 |

Send each hexadecimal command followed by Enter at 115200 baud.
The sketch lives at `../src/ArduinoSerialTester/ArduinoSerialTester.ino`.

This is a wiring model, not evidence of a completed simulator run.
To run in the Wokwi web editor, use the diagram together with a copy of
the main sketch. The Wokwi build/emulation has not been executed here.
