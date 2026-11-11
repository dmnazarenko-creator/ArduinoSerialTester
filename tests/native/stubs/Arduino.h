#pragma once
#include <cstdint>
#include <cstring>
using uint8_t=std::uint8_t;
using uint16_t=std::uint16_t;
using uint32_t=std::uint32_t;
constexpr uint8_t LOW=0, HIGH=1, OUTPUT=1;
#define F(x) x
uint8_t highByte(uint16_t x);
uint8_t lowByte(uint16_t x);
void digitalWrite(uint8_t pin,uint8_t value);
int digitalRead(uint8_t pin);
void pinMode(uint8_t pin,uint8_t mode);
void delayMicroseconds(unsigned long us);
struct FakeSerial {
  void begin(unsigned long) {}
  void print(const char*) {}
  void print(char) {}
  void println() {}
  void println(const char*) {}
  int available(){return 0;}
  int read(){return -1;}
};
extern FakeSerial Serial;
