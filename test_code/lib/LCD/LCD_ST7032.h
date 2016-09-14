/* LCD_ST7032.h -- driver for (eg) Newhaven LCDs which use 4-wire
  ST7032 interface.

  based on derived classes from https://bitbucket.org/fmalpartida/new-liquidcrystal
  (// Created by Francisco Malpartida on 20/08/11.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA)

  TODO: implement an "LCD on", such that you can power down the LCD entirely
  when not in use. (if desired...)
  */


#ifndef LCD_ST7032_H
#define LCD_ST7032_H

#include "Arduino.h"
#include "LCD.h"
#include "SPI.h"


class LCD_ST7032 : public LCD
{
public:
  LCD_ST7032(uint8_t pin_rst, uint8_t pin_rs, uint8_t pin_cs);

  virtual void send(uint8_t value, uint8_t mode);

  virtual void begin(uint8_t cols = 8, uint8_t rows = 2, uint8_t charsize = LCD_5x8DOTS);

private:
  void init();

  uint8_t _rst, _rs, _cs, _rows, _cols;
  SPISettings _spi_settings;
};


LCD_ST7032::LCD_ST7032(uint8_t pin_rst, uint8_t pin_rs, uint8_t pin_cs) :
  _rst(pin_rst), _rs(pin_rs), _cs(pin_cs)
{
  _spi_settings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
}

void LCD_ST7032::init(){
  pinMode(_rst, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_rs, OUTPUT);

  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);

  digitalWrite(_rst, LOW);
  delay(2);
  send(0x30, COMMAND); // wakeup
  delay(2);
  digitalWrite(_rst, HIGH);

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
  init();
  LCD::begin(rows, cols, dotsize);
}


void LCD_ST7032::send(uint8_t value, uint8_t mode){

  // send mode => RS pin
  // LOW: command; HIGH: data
  bool send_mode = ( mode == DATA );
  noInterrupts();
  SPI.beginTransaction(_spi_settings);
  digitalWrite(_cs, LOW);
  digitalWrite(_rs, send_mode);
  delay(2); // do we need this?
  SPI.transfer(value);
  delay(2); // do we need this?
  SPI.endTransaction();
  digitalWrite(_cs, HIGH);
  interrupts();
//if (send_mode){Serial.write(value);} else { Serial.println();}
}

#endif
