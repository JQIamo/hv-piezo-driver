/*
AD56XXR.h  - Library for controlling Analog Devices AD56XX dual/quad channel DACs w/internal reference

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

/*!	\file	AD56X4R.h
*	\brief	Library for controlling AD56X4R chips
*	\details Library for controlling a AD5664R, AD5644R, and AD5624R (16, 14, 12 bits)
*          See http://www.analog.com/media/en/technical-documentation/data-sheets/AD5624R_5644R_5664R.pdf
*          for the datasheet. This library uses a software-emulated MOSI communication scheme but could
*			easily be extended to use the built-in SPI interface.
*
*	\author	Ben Reschovsky <breschov@umd.edu>
*  \version 0.1
*	\date	2015-06-22
*/

#ifndef AD56XXR_h
#define AD56XXR_h

#include "Arduino.h"

#define AD56XXR_WRITE_INPUT_REG_N         0x00
#define AD56XXR_UPDATE_DAC_REG_N          0x01
#define AD56XXR_WRITE_INPUT_REG_N_UPDATE  0x02
#define AD56XXR_WRITE_UPDATE_DAC_N        0x03
#define AD56XXR_POWERDOWN                 0x04
#define AD56XXR_RESET                     0x05
#define AD56XXR_LDAC_REG_SETUP            0x06
#define AD56XXR_INT_REF_SETUP             0x07

#define AD56XXR_COMMAND_MASK 0x07

// if speed not defined, default to 500kHz.
#ifndef AD56XXR_SPI_SPEED
#define AD56XXR_SPI_SPEED 500000
#endif

class AD56XXR
{
public:
  // Constructor function.
  AD56XXR(uint8_t CS_PIN, uint8_t SCK_PIN, uint8_t MOSI_PIN, uint8_t num_bits, double volt_ref, double volt_offset, bool int_ext_ref);

  // constructor if using default SPI (not software-emulated)
  AD56XXR(uint8_t CS_PIN, uint8_t num_bits, double volt_ref, double volt_offset, bool int_ext_ref);

  // writes voltage Vout to the DAC on channel ch
  void setVolt(uint8_t ch, double volt);

  //function to get voltage at specified DAC address
  double getVolt(uint8_t ch);

  // set DAC tuning word
  void setVal(uint8_t ch, uint16_t val);

  //function to get current tuning value at specified DAC address
  uint16_t getVal(uint8_t ch);

  //function to send DATA to the DAC, this version uses a software emulated SPI protocol:
  void writeDAC(uint8_t command, uint8_t address,  uint16_t data);

  //function to enable or disable internal voltage reference (1/True = use internal, 0/False = use external)
  void setIntRef(bool state);

  void setMaxVal(uint8_t ch, uint16_t val);

  void setMinVal(uint8_t ch, uint16_t val);


private:

  //Instance variable to hold pin mappings:
  uint8_t _CS_PIN, _SCK_PIN, _MOSI_PIN;

  //Instance variable to hold DAC precision (12, 14, or 16 bits) and refernce voltage:
  uint8_t _dac_precision;
  double _volt_ref, _volt_offset;

  // transfer functions
  uint16_t volt_to_val(double volt);
  double val_to_volt(uint16_t val);

  //Instance variables to hold DAC set values and voltages for channels 0-3:
  uint16_t _val[4], _minVal[4], _maxVal[4];

  bool _emulate_spi;
};

#endif
