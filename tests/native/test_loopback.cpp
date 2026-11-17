// Native behavioral test of the actual production sketch.
// This is NOT an AVR CPU emulator or an electrical timing simulation.
#include <Arduino.h>
#include <SPI.h>
#include <iostream>
#include <stdexcept>
#include <string>

FakeSerial Serial;
FakeSPI SPI;
bool corruptNextRead=false;
uint32_t simulatedMs=0;
uint8_t pinLevel[20] = {};
uint16_t shift595=0, latched595=0, shift165=0;
uint64_t clocks=0, captures=0, latches=0;
uint8_t highByte(uint16_t x) {return uint8_t(x>>8);}
uint8_t lowByte(uint16_t x) {return uint8_t(x);}
void pinMode(uint8_t, uint8_t) {}
int digitalRead(uint8_t pin) {
  if(pin!=12) throw std::runtime_error("unexpected input pin");
  int value=(shift165>>15)&1;
  if(corruptNextRead) { corruptNextRead=false; value ^= 1; }
  return value;
}
void delayMicroseconds(unsigned long) {}
void delay(unsigned long ms) { simulatedMs += uint32_t(ms); }
uint32_t millis() { return simulatedMs; }
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
#if TestMode == '2'
  // A loop call before the first 50-ms boundary must produce no SPI clocks.
  const uint64_t initialClocks=clocks;
  loop();
  require(clocks==initialClocks,"SPI fired before the first 50-ms interval");
  simulatedMs=50;
  const uint16_t patterns[]={0xA55A,0x5AA5,0xFFFF,0};
  for(uint8_t i=0;i<4;++i) {
    uint64_t before=clocks;
    loop();
    require(clocks==before+16,"Expected exactly one 16-bit exchange per period");
    require(latched595==patterns[i],"Incorrect TX pattern in continuous mode");
    loop();
    require(clocks==before+16,"Duplicate SPI exchange within 50-ms period");
    simulatedMs+=50;
  }
  // Force a single receive bit mismatch. Testing must continue afterward.
  corruptNextRead=true;
  uint64_t beforeError=clocks;
  loop();
  require(clocks==beforeError+16,"SPI was interrupted by injected RX fault");
  require(latched595==1,"Walking-one sequence did not begin");
  simulatedMs+=50;
  loop();
  require(clocks==beforeError+32,"SPI stopped following a mismatched input bit");
  require(latched595==0,"Walking-bit zero reset missing");
  simulatedMs+=50;
  loop();
  require(latched595==2,"Continuous walking-one did not advance");
  require(clocks==beforeError+48,"SPI did not continue after error");

#endif
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
