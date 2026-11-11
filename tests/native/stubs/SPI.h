#pragma once
#include <Arduino.h>
constexpr uint8_t MSBFIRST=1, SPI_MODE0=0;
struct SPISettings { SPISettings(uint32_t, uint8_t, uint8_t) {} };
struct FakeSPI {
  void begin() {}
  void beginTransaction(const SPISettings&) {}
  void endTransaction() {}
  uint8_t transfer(uint8_t value);
};
extern FakeSPI SPI;
