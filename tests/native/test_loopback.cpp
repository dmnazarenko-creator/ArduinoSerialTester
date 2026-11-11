// Native behavioral test of the actual production sketch.
// This is NOT an AVR CPU emulator or an electrical timing simulation.
#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <stdexcept>
#include <string>

FakeSerial Serial;
FakeSPI SPI;
uint8_t pinLevel[20] = {};
uint16_t shift595=0, latched595=0, shift165=0;
uint64_t clocks=0, captures=0, latches=0;
uint8_t highByte(uint16_t x) {return uint8_t(x>>8);}
uint8_t lowByte(uint16_t x) {return uint8_t(x);}
void pinMode(uint8_t, uint8_t) {}
int digitalRead(uint8_t pin) { if(pin!=12) throw std::runtime_error("unexpected input pin"); return (shift165>>15)&1; }
void delayMicroseconds(unsigned long) {}
void digitalWrite(uint8_t pin, uint8_t value) {
  uint8_t old=pinLevel[pin];
  pinLevel[pin]=value;
  if(pin==9 && value==LOW) shift595=0; // /MR
  if(pin==7 && value==LOW) { shift165=latched595; ++captures; } // /PL
  if(pin==8 && old==LOW && value==HIGH) { latched595=shift595; ++latches; }
}
uint8_t FakeSPI::transfer(uint8_t tx) {
  uint8_t rx=0;
  for(int bit=7;bit>=0;--bit) {
    if(pinLevel[6]!=LOW || pinLevel[7]!=HIGH)
      throw std::runtime_error("165 clock inhibited or /PL asserted");
    if(pinLevel[9]!=HIGH) throw std::runtime_error("595 reset asserted");
    shift165=uint16_t(shift165<<1);
    rx=uint8_t((rx<<1)|((shift165>>15)&1));
    shift595=uint16_t((shift595<<1)|((tx>>bit)&1));
    ++clocks;
  }
  return rx;
}
#include "../../../src/ArduinoSerialTester/ArduinoSerialTester.ino"

void require(bool passed, const std::string& what) {
  if(!passed) throw std::runtime_error(what);
}
void testWord(uint16_t tx,uint16_t previous) {
  auto c=clocks,p=captures,l=latches;
  auto rx=exchange16(tx);
  require(rx==previous,"RX mismatch for TX="+std::to_string(tx));
  require(latched595==tx,"595 output mismatch");
  require(clocks==c+16,"Expected exactly 16 SPI clocks");
  require(captures==p+1,"Expected one parallel input capture");
  require(latches==l+1,"Expected one output latch");
}
int main() {
  setup();
  require(latched595==0,"595 outputs not zero at startup");
  uint16_t previous=0;
  for(uint16_t tx:{uint16_t(0xA55A),uint16_t(0x5AA5),uint16_t(0xFFFF),uint16_t(0),uint16_t(0)}) {
    testWord(tx,previous); previous=tx;
  }
  for(int bit=0;bit<16;++bit) {
    auto tx=uint16_t(1u<<bit);
    testWord(tx,previous);testWord(0,tx);previous=0;
  }
  for(uint32_t tx=0;tx<65536;++tx) {
    testWord(uint16_t(tx),previous);previous=uint16_t(tx);
  }
  testWord(0,previous);
  std::cout<<"PASS: patterns, 16 walking bits, 65536 words, "
           <<clocks<<" SPI clocks"<<std::endl;
}
