#ifndef PIEZO_API_H
#define PIEZO_API_H

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>

#include "Menu.h"
#include "EncoderStream.h"
#include "LCD.h"

#include "piezo_registers.h"

/******************* PIN DEFINITIONS **********************/

#define EN        9   // pin 46
#define LP_EN     30  // pin 56
#define INTERLOCK 23  // pin 45

#define DAC_CS    28  // pin 53
#define DAC_LDAC  27  // pin 54
#define DAC_CLR   29  // pin 55

#define LCD_EN    22  // pin 44
#define LCD_CS    32  // pin 41
#define LCD_RS    25  // pin 42
#define LCD_RST   15  // pin 43

// //These pins are defined in Encoder.h
// #define ENC_A     0   // pin 39
// #define ENC_B     5   // pin 64
// #define ENC_SW    1   // pin 40
#define SW        6   // pin 61

//Analog pins
#define VP_MON    A7  // pin 63
#define VM_MON    A6  // pin 62
#define HV_MON    A2  // pin 35

/******************* DAC SETTINGS **********************/

#define DAC_SPISPEED 1000000

#define PIEZO_VOLTRANGE 247.5
#define AUXDAC_VOLTRANGE 5.0  // not currently used

#define DAC_BITRESOLUTION 65535

#define PIEZO_VOLT_TO_VAL (DAC_BITRESOLUTION/PIEZO_VOLTRANGE)
#define AUXDAC_VOLT_TO_VAL (DAC_BITRESOLUTION/AUXDAC_VOLTRANGE)

#define PIEZO_VAL_TO_VOLT (PIEZO_VOLTRANGE/DAC_BITRESOLUTION)
#define AUXDAC_VAL_TO_VOLT (AUXDAC_VOLTRANGE/DAC_BITRESOLUTION)

#define AD56XXR_WRITE_INPUT_REG_N         0x00000000
#define AD56XXR_UPDATE_DAC_REG_N          0x00080000
#define AD56XXR_WRITE_INPUT_REG_N_UPDATE  0x00100000
#define AD56XXR_WRITE_UPDATE_DAC_N        0x00180000
#define AD56XXR_POWERDOWN                 0x00200000
#define AD56XXR_RESET                     0x00280000
#define AD56XXR_LDAC_REG_SETUP            0x00300000
#define AD56XXR_INT_REF_SETUP             0x00380000
#define AD56XXR_DAC_A                     0x00000000
#define AD56XXR_DAC_B                     0x00010000
#define AD56XXR_DAC_ALL                   0x00070000


/******************* EEPROM SETTINGS **********************/
#define EEPROM_INIT 0xA0  // "code word" byte to see if eeprom is initialized
                          // with valid data

void calibrate();
void persistReg();
void initReg();


class Piezo;
extern Piezo piezo;

extern LCD_ST7032 lcd;

//extern MenuController root;

class Piezo {
public:
  Piezo();

  //! begin piezo; pass chip select, clr, and ldac pins for DAC.
  void begin(uint8_t cs, uint8_t clr, uint8_t ldac);

  //!
  double setVolt(double volt);

  //! getVolt: returns voltage (double) from PIEZO output register.
  double getVolt();

  uint16_t setVal(uint16_t val);
  uint16_t getVal();

  // TODO: implement auxiliary DAC channel

  int getModVal();
  int getModVolt();

  // Read voltage rails
  uint16_t getVP();
  uint16_t getVM();

  /** writeDAC: writes into the DAC registers

      @param reg register value to write. Will send the first 3 bytes to DAC.

      NOTE: this will not respect output voltage bounds!
      Be sure you know what you are doing.
  */
  void writeDAC(uint32_t reg);

  //! writeDACReg: same as writeDAC, but takes a pointer to a register address.
  void writeDACReg(volatile uint32_t * reg);


  /** update: writes PIEZO output register to DAC.

    Note: this function respects the min/max voltage bounds held in
    PIEZOMIN and PIEZOMAX registers.
  */
  void update();


  //! enableOutput: enables the high-voltage output.
  inline void enableOutput();

  //! disableOutput: disables the high voltage output.
  inline void disableOutput();

  /** enableLP: activates the low-pass filter between the DAC and HV output.

      When enabled, the DAC sees a single-pole lowpass filter with fc = 0.8Hz.
  */
  inline void enableLP();

  /** disableLP: disables the low-corner lowpass filter between the DAC and
      HV output.

      When disabled, the DAC sees a single-pole lowpass filter with fc = 165Hz.
  */
  inline void disableLP();

  /** val_to_volt: converts DAC value to output voltage.
      @param val DAC value (16-bit) to be converted
      @returns double corresponding to HV output.
  */
  inline double val_to_volt(uint16_t val);

  /** volt_to_val: converts voltage to equivalent DAC value
      @param volt double representing HV voltage
      @returns 16-bit DAC value corresponding to volt.
  */
  inline uint16_t volt_to_val(double volt);

private:
  uint8_t _cs, _clr, _ldac;
  SPISettings _spi_settings;
};

Piezo::Piezo(){;}

void Piezo::begin(uint8_t cs, uint8_t clr, uint8_t ldac){

    _cs = cs;
    pinMode(_cs, OUTPUT);
    digitalWriteFast(_cs, HIGH);

    _clr = clr;
    pinMode(_clr, OUTPUT);
    digitalWriteFast(_clr, HIGH);

    _ldac = ldac;
    pinMode(_ldac, OUTPUT);
    digitalWriteFast(_ldac, HIGH);

    // set switch to input_pullup
    pinMode(SW, INPUT_PULLUP);

    _spi_settings = SPISettings(DAC_SPISPEED, MSBFIRST, SPI_MODE2);

    SPI.begin();

    // load previous registers from eeprom
    initReg();

    // turn on internal reference.
    uint32_t reg = AD56XXR_INT_REF_SETUP | 1;
    writeDAC(reg);

    // initialize Piezo reg high byte for writing to DAC_A
    *regAddress32(PIEZO) |= AD56XXR_WRITE_UPDATE_DAC_N | AD56XXR_DAC_A;

    // begin at 0V
    regWrite(PIEZO, 0x00);

    //analogReference(INTERNAL);  // set analog reference to internal ref
    analogReference(INTERNAL);
    analogReadRes(16);          // Teensy 3.0: set ADC resolution to this many bits
    //analogReadResolution(16);

    enableOutput();

    update(); // update output; good to go!

    if(regRead(PIEZOLP)){
      enableLP();
    } else {
      disableLP();
    }
  }


  inline void Piezo::enableOutput(){
    digitalWriteFast(EN, LOW);
  }

  inline void Piezo::disableOutput(){
    digitalWriteFast(EN, HIGH);
  }

  inline void Piezo::enableLP(){
    digitalWriteFast(LP_EN, HIGH);
  }

  inline void Piezo::disableLP(){
    digitalWriteFast(LP_EN, LOW);
  }

  void Piezo::writeDAC(uint32_t reg){
    uint8_t b0 = (uint8_t)(reg >> 16);
    uint8_t b1 = (uint8_t)(reg >> 8);
    uint8_t b2 = (uint8_t)(reg);

    SPI.beginTransaction(_spi_settings);
    digitalWriteFast(_cs, LOW);

    SPI.transfer(b0);
    SPI.transfer(b1);
    SPI.transfer(b2);

    digitalWriteFast(_cs, HIGH);
    SPI.endTransaction();

  }

  void Piezo::writeDACReg(volatile uint32_t * reg){
    uint8_t b0 = (uint8_t)(*reg >> 16);
    uint8_t b1 = (uint8_t)(*reg >> 8);
    uint8_t b2 = (uint8_t)(*reg);

    SPI.beginTransaction(_spi_settings);
    digitalWriteFast(_cs, LOW);

    SPI.transfer(b0);
    SPI.transfer(b1);
    SPI.transfer(b2);

    digitalWriteFast(_cs, HIGH);
    SPI.endTransaction();
  }

  inline uint16_t Piezo::volt_to_val(double volt){
    return (uint16_t)(volt*(PIEZO_VOLT_TO_VAL));
  }

  inline double Piezo::val_to_volt(uint16_t val){
    return (double)(val*(PIEZO_VAL_TO_VOLT));
  }

  double Piezo::setVolt(double volt){
      int val_request = (int)(volt*(PIEZO_VOLT_TO_VAL));
      uint16_t val_constrained = constrain(val_request, regRead(PIEZOMIN), regRead(PIEZOMAX));

      regWrite(PIEZO, val_constrained);
      writeDACReg(regAddress32(PIEZO));

      if (val_request != val_constrained){
        return getVolt();
      } else {
        return volt;
      }
  }

  double Piezo::getVolt(){
    uint16_t val = regRead(PIEZO);
    return val_to_volt(val);
  }

  void Piezo::update(){
    uint16_t val = regRead(PIEZO);
    regWrite(PIEZO, constrain(val, regRead(PIEZOMIN), regRead(PIEZOMAX)));
    writeDACReg(regAddress32(PIEZO));
  }



/*********************** AUXILIARY FUNCTIONS *********************/

void calibrate(){

  uint16_t stashed_piezo_val = regRead(PIEZO);

  piezo.disableLP();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("calibrating...!");
  lcd.setCursor(0, 1);
  lcd.print("disconn. output");
  delay(5000);
  lcd.setCursor(0,0);
  lcd.print("push to continue");
  lcd.setCursor(0, 1);
  lcd.print("turn to cancel  ");

  // wait for user input to proceed...
  while (!Encoder.available());
  char c = Encoder.read();
  if (c != 'P'){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("canceled!");
    delay(2000);

    root.enter();
    return;
  }

  lcd.clear();

  uint16_t val = 0;
  uint16_t read_val;

  #ifdef DEBUG
    Serial.println("SetVal\tAnalogRead");
  #endif

  for (int i = 0; i < CALIBRATION_POINTS; i++){
    lcd.setCursor(0, 0);
    regWrite(PIEZO, val);

    // DANGER!! Does not respect output limits during calibration
    // this is why user should disconnect output first.
    piezo.writeDACReg(regAddress32(PIEZO));
    lcd.print("set: ");
    lcd.print(val);

    // wait 15 seconds for voltage to stabilize before recording reading
    for (int j = 0; j < 15; j++){
      lcd.setCursor(0, 1);
      lcd.print("read: ");
      lcd.print(analogRead(HV_MON));
      delay(1000);
    }


    read_val = analogRead(HV_MON);
    // store into calibration registers; high two bytes are setval,
    // low two bytes are read val.
    regWrite32(PIEZO_CALIB + i,  (((uint32_t)val) << 16 | read_val));

    #ifdef DEBUG
      Serial.print(val);
      Serial.print("\t");
      Serial.println(read_val);
    #endif

    if (i < 40){
      val += 400;
    } else if (i == CALIBRATION_POINTS - 2){
      val = 65535;  // last point should be full scale
    } else {
      val += 2000;
    }
  }

  persistReg();
  lcd.clear();
  lcd.print("finished!");

  regWrite(PIEZO, stashed_piezo_val);
  piezo.update();

  if(regRead(PIEZOLP)) piezo.enableLP();

  delay(2000);

  root.enter();
}


void persistReg(){
  for (int i = 0; i < RWREG_COUNT; i++){
    uint32_t regValue = regRead32(i);
    EEPROM.put(1 + 4*i, regValue);
    #ifdef DEBUG
      Serial.print("Persisting reg ");
      Serial.print(i);
      Serial.print("; val: ");
      Serial.println(regValue, HEX);
    #endif
  }
}

void persistReg(Registers reg){
  uint32_t regValue = regRead32(reg);
  EEPROM.put(1+4*reg, regValue);
  #ifdef DEBUG
    Serial.print("Persisting reg ");
    Serial.print((int)reg);
    Serial.print("; val: ");
    Serial.println(regValue, HEX);
  #endif
}


void initReg(){
  uint8_t eepromInit;
  EEPROM.get(0, eepromInit);
  if (eepromInit == EEPROM_INIT){
    // load registers from eeprom
    for (int i = 0; i < RWREG_COUNT; i++){
      uint32_t regValue;
      EEPROM.get(1 + 4*i, regValue);
      regWrite32(i, regValue);
      #ifdef DEBUG
        Serial.print("Initializing reg ");
        Serial.print(i);
        Serial.print("; val: ");
        Serial.println(regValue, HEX);
      #endif
    }
  } else {
    // initialize registers; write to eeprom
    regWrite(PIEZO, 0x00);
    regWrite(PIEZOMIN, 0x00);
    regWrite(PIEZOMAX, 0xFFFF);
    regWrite(PIEZOEN, 0x00);
    regWrite(PIEZOLP, 0x00);
    regWrite(PIEZOINTERLOCK, 0x00);
    EEPROM.put(0, EEPROM_INIT);
    persistReg();
  }
}


uint16_t measured_to_val(uint16_t meas){

  // assume zero is first calibration point; 65535 is last; monotonic
  if (meas < regRead(PIEZO_CALIB)) return 0;
  if (meas > regRead(PIEZO_CALIB + CALIBRATION_POINTS - 1)) return 65535;

  int i = 0;
  while (meas > regRead(PIEZO_CALIB + i)) i++;

  uint32_t calibration_pt1 = regRead32(PIEZO_CALIB + i);
  uint16_t cal_x1 = (uint16_t) calibration_pt1;
  uint16_t cal_y1 = (uint16_t) (calibration_pt1 >> 16);

  uint32_t calibration_pt0 = regRead32(PIEZO_CALIB + i - 1);
  uint16_t cal_x0 = (uint16_t) calibration_pt0;
  uint16_t cal_y0 = (uint16_t) (calibration_pt0 >> 16);

  double slope = (double)(cal_y1 - cal_y0) / (cal_x1 - cal_x0);

  return (uint16_t)(slope*(meas - cal_x0) + cal_y0);
}

#endif
