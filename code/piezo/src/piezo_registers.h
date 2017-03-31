#ifndef PIEZO_REGISTERS_H
#define PIEZO_REGISTERS_H

#define RWREG_COUNT 0x4C
#define CALIBRATION_POINTS 64



enum Registers
{
  PIEZO =           0x00,        // piezo dac output tuning word
  PIEZOMIN =        0x01,        // piezo min val
  PIEZOMAX =        0x02,        // piezo max val
  PIEZOEN =         0x03,        // enable piezo output
  PIEZOLP =         0x04,        // low-pass corner
  PIEZOINTERLOCK =  0x05,        // interlock register

  LF_STATE =        0x06,
  LF_ERROR =        0x07,
  LF_DEADBAND =     0x08,
  LF_STEP =         0x09,
  LF_HYST =         0x0A,
  LF_TAU =          0x0B,

  /* 16 calibration points, PIEZO_CALIB run sfrom 0x0C to 0x1B */
  PIEZO_CALIB =     0x0C,

  MONPIEZO =        0x4C, // monitor of piezo; not currently used
  MONVP =           0x4D, // monitor pos rail; not currently used
  MONVM =           0x4E,  // monitor neg rail; not currently used
  REG_COUNT =       0x4F
};

volatile uint32_t * RegArray[REG_COUNT];

  // 16-bit register accessors
inline volatile uint16_t *regAddress(int reg){
  return reinterpret_cast<volatile uint16_t*>(RegArray + reg);
}

inline uint16_t regRead(int reg){
  return *regAddress(reg);
}

inline void regWrite(int reg, uint16_t value){
  *regAddress(reg) = value;
}

// 32-bit register accessors
inline volatile uint32_t *regAddress32(int reg){
  return reinterpret_cast<volatile uint32_t*>(RegArray + reg);
}

inline uint32_t regRead32(int reg){
  return *regAddress32(reg);
}

inline void regWrite32(int reg, uint32_t value){
  *regAddress32(reg) = value;
}

#endif
