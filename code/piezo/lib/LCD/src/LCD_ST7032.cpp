#include "LCD_ST7032.h"

LCD_ST7032::LCD_ST7032(uint8_t pin_rst, uint8_t pin_rs, uint8_t pin_cs) :
  _rst(pin_rst), _rs(pin_rs), _cs(pin_cs)
{
  _spi_settings = SPISettings(5000000, MSBFIRST, SPI_MODE0);
}

void LCD_ST7032::init(){
  pinMode(_rst, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_rs, OUTPUT);

  digitalWriteFast(_rst, HIGH);
  digitalWriteFast(_rs, HIGH);
  digitalWriteFast(_cs, HIGH);

  digitalWriteFast(_rst, LOW);
  delay(2);
  send(0x30, COMMAND); // wakeup
  delay(2);
  digitalWriteFast(_rst, HIGH);

  send(0x30, COMMAND); // wakeup
  send(0x30, COMMAND); // wakeup

  send(0x39, COMMAND); // function set
  send(0x14, COMMAND);   // internal osc frequency
  send(0x56, COMMAND);   // power control
  send(0x6D, COMMAND);   // follower control

  send(0x70, COMMAND);   // contrast
  send(0x0C, COMMAND);   // display on
  send(0x06, COMMAND);   // entry mode
  send(0x01, COMMAND);   // clear

  delay(10);
}

void LCD_ST7032::begin(uint8_t rows, uint8_t cols, uint8_t dotsize){
  SPI.begin();
  init();
  // do we need to call parent begin??
  LCD::begin(rows, cols, dotsize);
  clear();
  setCursor(0,0);
}

void LCD_ST7032::send(uint8_t value, uint8_t mode){
  // send mode => RS pin
  // LOW: command; HIGH: data
  bool send_mode = ( mode == DATA );

  noInterrupts();
  SPI.beginTransaction(_spi_settings);
  digitalWriteFast(_cs, LOW);
  digitalWriteFast(_rs, send_mode);
  delayMicroseconds(150); // do we need this?
  SPI.transfer(value);
  delayMicroseconds(150); // do we need this?
  SPI.endTransaction();
  digitalWriteFast(_cs, HIGH);
  interrupts();
}
