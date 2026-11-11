#include <Arduino.h>
#include <SPI.h>

// Nano ATmega328P: D13=SCK, D12=MISO, D11=MOSI.
constexpr uint8_t CE_R = 6;   // 74HC165 /CE, pin 15
constexpr uint8_t LOAD_R = 7; // 74HC165 /PL, pin 1
constexpr uint8_t LOAD_W = 8; // 74HC595 ST_CP, pin 12
constexpr uint8_t CLR_W = 9;  // 74HC595 /MR, pin 10
constexpr uint8_t OE_W = 10;  // 74HC595 /OE, pin 13, also AVR SS

// Two daisy-chained 595s and two daisy-chained 165s.
// 595 #1 Q7' (pin 9) -> 595 #2 SER (pin 14).
// 165 #1 Q7 (pin 9) -> 165 #2 DS (pin 10).
// 165 #2 Q7 (pin 9) -> Nano D12; unused 165 #1 DS tied to GND.
// The pin naming (#1/#2) follows the physical chain, not bit numbering.
const SPISettings SPI_CONFIG(1000000UL, MSBFIRST, SPI_MODE0);

uint16_t exchange16(uint16_t outgoing) {
  SPI.beginTransaction(SPI_CONFIG);
  // Parallel capture of all 16 inputs.
  digitalWrite(CE_R, HIGH);
  digitalWrite(LOAD_R, LOW);
  delayMicroseconds(1);
  digitalWrite(LOAD_R, HIGH);
  delayMicroseconds(1);

  digitalWrite(LOAD_W, LOW);
  digitalWrite(CE_R, LOW);
  // Full duplex: every SPI clock simultaneously writes MOSI and reads MISO.
  const uint8_t hi = SPI.transfer(highByte(outgoing));
  const uint8_t lo = SPI.transfer(lowByte(outgoing));
  digitalWrite(CE_R, HIGH);

  // Latch the output word only after both bytes have shifted through.
  digitalWrite(LOAD_W, HIGH);
  delayMicroseconds(1);
  digitalWrite(LOAD_W, LOW);
  SPI.endTransaction();
  return (uint16_t(hi) << 8) | lo;
}

void printHex16(uint16_t v) {
  Serial.print(F("0x"));
  for (int shift = 12; shift >= 0; shift -= 4) {
    uint8_t n = (v >> shift) & 15;
    Serial.print(char(n < 10 ? '0' + n : 'A' + n - 10));
  }
}

void setup() {
  // Establish inactive levels before switching pins to outputs.
  digitalWrite(CE_R, HIGH);
  digitalWrite(LOAD_R, HIGH);
  digitalWrite(LOAD_W, LOW);
  digitalWrite(CLR_W, HIGH);
  digitalWrite(OE_W, HIGH);
  pinMode(CE_R, OUTPUT);
  pinMode(LOAD_R, OUTPUT);
  pinMode(LOAD_W, OUTPUT);
  pinMode(CLR_W, OUTPUT);
  pinMode(OE_W, OUTPUT); // D10 remains OUTPUT for SPI master mode.

  SPI.begin();
  pinMode(OE_W, OUTPUT);
  digitalWrite(OE_W, HIGH);
  // Shift-register clear does not clear the 595 output latch until ST_CP rises.
  digitalWrite(CLR_W, LOW);
  delayMicroseconds(1);
  digitalWrite(CLR_W, HIGH);
  digitalWrite(LOAD_W, HIGH);
  delayMicroseconds(1);
  digitalWrite(LOAD_W, LOW);
  digitalWrite(OE_W, LOW);

  Serial.begin(115200);
  Serial.println(F("ArduinoSerialTester: 16-bit SPI ready"));
  Serial.println(F("Send HEX word (0000..FFFF) and Enter"));
}

char buffer[7];
uint8_t length = 0;
void handleCommand() {
  if (!length) return;
  buffer[length] = '\0';
  const char *p = buffer;
  if (length >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
  uint8_t count = strlen(p);
  if (!count || count > 4) {
    Serial.println(F("ERR: expected 1..4 hex digits"));
    return;
  }
  uint16_t tx = 0;
  for (uint8_t i = 0; i < count; ++i) {
    char c = p[i];
    uint8_t d;
    if (c >= '0' && c <= '9') d = c - '0';
    else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
    else {
      Serial.println(F("ERR: invalid HEX"));
      return;
    }
    tx = (tx << 4) | d;
  }
  uint16_t rx = exchange16(tx);
  Serial.print(F("TX=")); printHex16(tx);
  Serial.print(F(" RX=")); printHex16(rx);
  Serial.println();
}
void loop() {
  while (Serial.available()) {
    char c = char(Serial.read());
    if (c == '\n' || c == '\r') {
      handleCommand();
      length = 0;
    } else if (length < sizeof(buffer) - 1) {
      buffer[length++] = c;
    } else {
      length = 0;
      Serial.println(F("ERR: command too long"));
    }
  }
}
