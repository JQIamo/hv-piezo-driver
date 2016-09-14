/*

AD56XXR.h  - Library for controlling Analog Devices AD56XX quad channel DACs w/internal reference

Created by Ben Reschovsky, 2015
JQI - Joint Quantum Institute

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*!	\file	AD56XXR.h
*	\brief	Library for controlling AD56XXR chips
*	\details Library for controlling a AD5664R, AD5644R, and AD5624R (16, 14, 12 bits)
*          See http://www.analog.com/media/en/technical-documentation/data-sheets/AD5624R_5644R_5664R.pdf
*          for the datasheet. This library uses a software-emulated MOSI communication scheme but could
*			easily be extended to use the built-in SPI interface.
*
*	\author	Ben Reschovsky <breschov@umd.edu>
*  \version 0.1
*	\date	2015-06-22
*/

#include "Arduino.h"
#include "AD56XXR.h"

#include "SPI.h"

/* CONSTRUCTOR */

// Constructor function; initializes communication pinouts
AD56XXR::AD56XXR(uint8_t CS_PIN, uint8_t SCK_PIN, uint8_t MOSI_PIN, uint8_t num_bits, double volt_ref, double volt_offset, bool int_ref){
  _emulate_spi = 1;
  _CS_PIN = CS_PIN;
  _SCK_PIN = SCK_PIN;
  _MOSI_PIN = MOSI_PIN;

  if (num_bits == 12) {
    _dac_precision = 12;
  } else if (num_bits == 14) {
    _dac_precision = 14;
  } else if (num_bits == 16) {
    _dac_precision = 16;
  } else {
    //Serial.println("WARNING:DAC object initialized with wrong number of bits of precision. Allowed values are 12, 14, or 16");
    _dac_precision = 12; //choose a reasonable default
  }
  _volt_ref = volt_ref;
  _volt_offset = volt_offset;

  // set min/max val; not sure what best thing to do is for "global" channel set
  for (int i = 0; i < 4; i++){
    _minVal[i] = 0;
    _maxVal[i] = (1UL << _dac_precision) - 1;
  }


  // sets up the pinmodes for output
  pinMode(_CS_PIN, OUTPUT);
  digitalWrite(_CS_PIN, HIGH);

  pinMode(_MOSI_PIN, OUTPUT);
  digitalWrite(_MOSI_PIN, LOW);

  pinMode(_SCK_PIN, OUTPUT);
  digitalWrite(_SCK_PIN, HIGH);
}

// If using builtin SPI
// will want to figure otu better way for configuring SPISettings...
AD56XXR::AD56XXR(uint8_t CS_PIN, uint8_t num_bits, double volt_ref, double volt_offset, bool int_ref){
  _emulate_spi = 0;
  _CS_PIN = CS_PIN;

  if (num_bits == 12) {
    _dac_precision = 12;
  } else if (num_bits == 14) {
    _dac_precision = 14;
  } else if (num_bits == 16) {
    _dac_precision = 16;
  } else {
    //Serial.println("WARNING:DAC object initialized with wrong number of bits of precision. Allowed values are 12, 14, or 16");
    _dac_precision = 16; //choose a reasonable default
  }
  _volt_ref = volt_ref;
  _volt_offset = volt_offset;

  // set min/max val; not sure what best thing to do is for "global" channel set
  for (int i = 0; i < 4; i++){
    _minVal[i] = 0;
    _maxVal[i] = (1UL << _dac_precision) - 1;
  }

  // sets up the pinmodes for output
  pinMode(_CS_PIN, OUTPUT);
  digitalWrite(_CS_PIN, HIGH);
}


/* PUBLIC CLASS FUNCTIONS */

//enable or disable internal voltage reference (1/True = use internal, 0/False = use external)
void AD56XXR::setIntRef(bool state){
  writeDAC(AD56XXR_INT_REF_SETUP, 0, state); //enable or disable internal voltage reference
}

void AD56XXR::setMinVal(uint8_t ch, uint16_t val){
  _minVal[ch] = val;
}

void AD56XXR::setMaxVal(uint8_t ch, uint16_t val){
  _maxVal[ch] = val;
}

void AD56XXR::setVal(uint8_t ch, uint16_t val){
  if (ch >= 4) {
    //Serial.println("WARNING:unrecognized DAC Channel, allowed values are 0,1,2,3,4"); // now only let individual channel addressing
    return; //channel isn't valid, return without doing anything
  }

  val = constrain(val, _minVal[ch], _maxVal[ch]);

  // bit shift depending on DAC precision
  if (_dac_precision == 12){
    val = (0x0FFF & val) << 4;
  } else if (_dac_precision == 14){
    val = (0x3FFF & val) << 2;
  }

  writeDAC(AD56XXR_WRITE_UPDATE_DAC_N, ch, val);
  _val[ch] = val;
}

//function to get current tuning value at specified DAC address
uint16_t AD56XXR::getVal(uint8_t ch){
  return _val[ch];
}


//function that writes the voltage val to the DAC channel ch (executes immediately):
void AD56XXR::setVolt(uint8_t ch, double volt){
  uint16_t val = volt_to_val(volt);
  setVal(ch, val);
}

//function to get set voltage at specified DAC address
double AD56XXR::getVolt(uint8_t ch){
  // _val holds authoritative DAC output
  return val_to_volt(_val[ch]);
}



// leaving this public, for flexibility.

void AD56XXR::writeDAC(uint8_t command, uint8_t address, uint16_t data){

  uint8_t packet[3];
  packet[0] = ((command & AD56XXR_COMMAND_MASK) << 3) | (address & AD56XXR_COMMAND_MASK);
  packet[1] = (data >> 8) & 0xFF;  // high byte
  packet[2] = data & 0xFF;  // low byte

  if (_emulate_spi){

    int send_bit;

    //Make sure communication lines are initialized
    digitalWrite(_SCK_PIN, HIGH);
    digitalWrite(_MOSI_PIN, LOW);

    digitalWrite(_CS_PIN,LOW);

    for (int i = 0; i < 3; i++){
      for (int b = 7; b >=0; b--){
        send_bit = (packet[i] >> b) & 0x01; // mask out b_th bit
        // bit-bang to DAC
        digitalWrite(_MOSI_PIN, send_bit);
        delayMicroseconds(1);

        digitalWrite(_SCK_PIN, LOW);
        delayMicroseconds(1);
        digitalWrite(_SCK_PIN, HIGH);
        delayMicroseconds(1);
      }
    }
    digitalWrite(_CS_PIN,HIGH);

  } else {
    SPI.beginTransaction(SPISettings(AD56XXR_SPI_SPEED, MSBFIRST, SPI_MODE2));
    digitalWrite(_CS_PIN, LOW);
    for (int i = 0; i < 3; i++){
      SPI.transfer(packet[i]);
    }
    digitalWrite(_CS_PIN, HIGH);
    SPI.endTransaction();
  }
}

/* Private class functions */

double AD56XXR::val_to_volt(uint16_t val){
  double ret =  ((double) val * _volt_ref / ((1UL << _dac_precision) - 1) + _volt_offset);
  return ret;
}

uint16_t AD56XXR::volt_to_val(double volt){
  uint16_t ret = (round(((1UL << _dac_precision) - 1)*(volt - _volt_offset) / _volt_ref));
  return ret;
}
