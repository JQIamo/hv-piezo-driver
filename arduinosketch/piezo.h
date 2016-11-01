#ifndef PIEZO_H
#define PIEZO_H


#include "Arduino.h"
#include "AD56XXR.h"


// Digital pins                      

#define EN        9    // pin 46 |
#define LP_EN     30   // pin 56 | 
#define INTERLOCK 23   // pin 45 |


#define DAC_SYNC  28  // pin 53  | 
#define DAC_LDAC  27  // pin 54  | 
#define DAC_CLR   29  // pin 55  | 

#define LCD_EN    22  // pin 44  | 
#define LCD_CS    32  // pin 41  | 
#define LCD_RS    25  // pin 42  | 
#define LCD_RST   15  // pin 43  | 

#define ENC_A     0   // pin 39  | 
#define ENC_B     5   // pin 64  | 
#define ENC_SW    1   // pin 40  | 
#define SW        6   // pin 61  |

//Analog pins

#define VP_MON   21  // pin 63  | 
#define VM_MON   20  // pin 62  |
#define HVMON 16  // pin 35  |



#ifndef PIEZO_RAMP_MILLIS
// delay time for ramping...
#define PIEZO_RAMP_MILLIS 1
#endif

#ifndef PIEZO_DAC_RAIL
// hard code in; avoid calibration snaffus
#define PIEZO_DAC_RAIL 65535UL
#endif

// initial scan parameters...
// roughly 3V - 100V
#define UTP 26500UL
#define LTP 750UL
#define RDELTA 750
#define FDELTA -750
//#define RAMPTIMER 1000



typedef enum piezo_mode_t { SCAN, DC, DC_LP } piezo_mode_t;



class Piezo {


public:
  Piezo(AD56XXR * dac);

  void init();
  void poll();  // run basic polling loop

  bool get_onoff();
  void set_onoff(bool state);

  double get_setpoint();
  void set_setpoint(double voltage);

  uint16_t get_dac();
  void set_dac(uint16_t dacVal);


  piezo_mode_t get_mode();
  void set_mode(piezo_mode_t mode);

  unsigned int get_scan_freq();
  unsigned int set_scan_freq(unsigned int freq);

  double get_scan_amp();
  double set_scan_amp(double amp);

  double get_scan_offset();
  double set_scan_offset(double offset);

  // set up scan about voltage midpoint
  void set_scan(double freq, double amp);

  void trip_interlock();
  void reset_interlock();

  void increment_scan();

  double get_scan_min();
  double set_scan_min(double voltage);

  double get_scan_max();
  double set_scan_max(double voltage);

  double get_scan_mid();
  double set_scan_mid(double voltage);

  double get_scan_symmetry();
  double set_scan_symmetry(double symmetry);



private:
  bool _en = false;
  bool _lp_en = false;
  bool _on = false;

  // interlock status: 0 = tripped; 1 = OK
  bool _interlock_status = true;

  piezo_mode_t _mode = DC;

  double _setpoint;   // voltage setpoint

  AD56XXR * _dac;   // pointer to DAC object

  // for linear ramping...
  uint16_t _ltp = LTP;    // lower turning point
  uint16_t _utp = UTP;    // upper turning point
  int16_t _rdelt = RDELTA;  // rising delta
  int16_t _fdelt = FDELTA;  // falling delta
  volatile int16_t _delta = RDELTA;
  volatile uint16_t _rampVal = LTP;

  // NOTE: must ensure that _ltp, _utp are far enough away from bounds such that
  //  _delta does not bring _rampVal (aka, dacVal) outside of acceptable bounds.
  //  Otherwise, problems ensue.

  double _scan_min, _scan_max, _scan_mid, _scan_amp, _scan_symmetry, _scan_freq;


  uint16_t voltage_to_dac(double voltage);
  double dac_to_voltage(uint16_t dac);
};

// setup singleton class instance
extern Piezo pzt;

// create static class for proper ISR interrupt handling
// only job is to delegate interrupts to the singleton `pzt`
class PiezoScan {
public:
  static void increment_scan();
};

void PiezoScan::increment_scan(){
  pzt.increment_scan();
}


Piezo::Piezo(AD56XXR * dac) : _dac(dac){ }

void Piezo::init(){
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH);

  _setpoint = 0.0;


  pinMode(LP_EN, OUTPUT);
  digitalWrite(LP_EN, LOW);

  pinMode(INTERLOCK, INPUT);  // do we need any internal pullups?

  //Timer1.initialize(RAMPTIMER);
  //Timer1.attachInterrupt(PiezoScan::increment_scan);
  //Timer1.stop();  // disable until ready...

  // frontpanel board with rotary encoder, enable switch
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);

  // DAC setup
  pinMode(DAC_CLR, OUTPUT);
  digitalWrite(DAC_CLR, HIGH);
  pinMode(DAC_LDAC, OUTPUT);
  digitalWrite(DAC_LDAC, LOW);

  // turn on LCD power
  pinMode(LCD_EN, OUTPUT);
  digitalWrite(LCD_EN, HIGH);

  // will want to save these to memory...
  set_scan_min(STARTUP_SCAN_MIN);
  set_scan_max(STARTUP_SCAN_MAX);
  set_scan_freq(STARTUP_SCAN_FREQ);

}


void Piezo::poll(){
  if (digitalRead(INTERLOCK) == HIGH || ! _interlock_status){
    trip_interlock();
    return;
  }

  // turn on/off depending on switch state...
  bool sw_state = digitalRead(SW);

  if (sw_state != get_onoff()){
    set_onoff(sw_state);
  }

}

void Piezo::increment_scan(void){
  if (_rampVal >= _utp){
    // if ramp is above turing point, load falling delta
    _delta = _fdelt;
  } else if (_rampVal < _ltp){
    // if ramp is below lower turing point, load rising delta
    _delta = _rdelt;
  }

  _rampVal += _delta;
  set_dac(_rampVal);
  //Serial.println(_rampVal);
}

double Piezo::set_scan_min(double voltage){
  voltage = constrain(voltage, 0.0, _scan_max);
  uint16_t scan_min_val = voltage_to_dac(voltage);
  if (abs(_fdelt) > scan_min_val){
    // NOTE: not sure if we want to adjust _fdelt or scan_min for these "bounds"
    // error checking... for now, we'll just adjust scan_min.
    // this might lead to some odd behavior though.
    scan_min_val = abs(_fdelt);
  }
  _scan_min = dac_to_voltage(scan_min_val);

  // make sure we don't get screwed when updating the ramp limits...
  noInterrupts();
  _ltp = scan_min_val;
  interrupts();
  return _scan_min;
}

double Piezo::get_scan_min(){
  return _scan_min;
}

double Piezo::set_scan_max(double voltage){
  voltage = constrain(voltage, _scan_min, 245.0);
  uint16_t scan_max_val = voltage_to_dac(voltage);
  if (_rdelt > (PIEZO_DAC_RAIL - scan_max_val)){
    // NOTE: not sure if we want to adjust _fdelt or scan_min for these "bounds"
    // error checking... for now, we'll just adjust scan_min.
    // this might lead to some odd behavior though.
    scan_max_val = PIEZO_DAC_RAIL - _rdelt;
  }
  _scan_max = dac_to_voltage(scan_max_val);

  // make sure we don't get screwed when updating the ramp limits...
  noInterrupts();
  _utp = scan_max_val;
  interrupts();
  return _scan_max;
}

double Piezo::get_scan_max(){
  return _scan_max;
}

unsigned int Piezo::set_scan_freq(unsigned int scale){
  _scan_freq = scale;
  _fdelt = -1*scale*200;
  _rdelt = scale*200;

  // make sure min/max not out of bounds now; update via scan_min/max
  set_scan_min(_scan_min);
  set_scan_max(_scan_max);
  //_scan_freq = freq;
  return _scan_freq;
}

unsigned int Piezo::get_scan_freq(){
  return _scan_freq;
}

double Piezo::set_scan_mid(double voltage){
  // tune around the midpoint...
  // uint16_t half_amplitude = (_utp - _ltp) >> 1;
  double half_amplitude = (_scan_max - _scan_min)/2.0;

  // set new min/max...
  set_scan_max(voltage + half_amplitude);
  set_scan_min(voltage - half_amplitude);
  _scan_mid = voltage;
  return _scan_mid;

}

double Piezo::get_scan_mid(){
  return _scan_mid;
}

//!! NOTE: KIND OF BROKEN...! also untested.
double Piezo::set_scan_amp(double voltage){
  double half_amplitude = voltage/2.0;


  // adjust _rDelt, _fDelt to keep same period...
  // do this by keeping fDelta = amplitude / 100
  // rDelta is set by scan symmetry
  // do we want to call constrain() on these?
  _fdelt = -1*(voltage_to_dac(voltage) * _scan_freq / 100);
  _rdelt = (int16_t)(_scan_symmetry * abs(_fdelt));

  set_scan_max(_scan_mid + half_amplitude);
  set_scan_min(_scan_mid - half_amplitude);

  // update scan amplitude based on what was actually set -- in case
  // there is funny business with the bounds.
  _scan_amp = (_scan_max - _scan_min);
  return _scan_amp;

}

double Piezo::get_scan_amp(){
  return _scan_amp;
}

// symmetry: sets _rDelt to be a multiple of _fDelt; eg:
// _rDelt = symmetry*abs(_fDelt).
// Thus, symmetry should be between (eg, sensible values [0.2, 5])
double Piezo::set_scan_symmetry(double symmetry){
  _scan_symmetry = constrain(symmetry, 0.2, 5.0);

  // re-run set scan amp, which will adjust _rDelt, _fDelt appropriately.
  set_scan_amp(_scan_amp);
  return _scan_symmetry;
}

double Piezo::get_scan_symmetry(){
  return _scan_symmetry;
}



void Piezo::trip_interlock(){
  digitalWrite(EN, 1);
  _dac->setVal(0, 0);
  _interlock_status = 0; // tripped
  set_mode(DC);
}

void Piezo::reset_interlock(){
  _interlock_status = 0;
}

bool Piezo::get_onoff(){
  return _on;
}

void Piezo::set_onoff(bool state){
  if (_interlock_status) {
    // write enable pin; note LOW = output enabled.
    _on = state;
    _on ? digitalWrite(EN, LOW) : digitalWrite(EN, HIGH);

  } else {
    // if interlock tripped, keep off.
    // interlock status 1 = OK, 0 = tripped
    _on = 0;
    digitalWrite(EN, 1);
    _dac->setVal(0, 0);
  }
}

double Piezo::get_setpoint(){
  return _setpoint;
}

void Piezo::set_setpoint(double voltage){
  voltage = constrain(voltage, 0.0, 247.0);
  _setpoint = voltage;
  uint16_t dacVal = voltage_to_dac(voltage);
  set_dac(dacVal);
}

uint16_t Piezo::get_dac(){
  return _dac->getVal(0);
}

void Piezo::set_dac(uint16_t dacVal){
  _dac->setVal(0, dacVal);
}

piezo_mode_t Piezo::get_mode(){
  return _mode;
}

void Piezo::set_mode(piezo_mode_t mode){
  _mode = mode;
  switch (mode) {
    case SCAN:
      // turn off lowpass filter, enable interrupts
      digitalWrite(LP_EN, LOW);
      //Timer1.start();
      break;

    case DC:
      //Timer1.stop();
      digitalWrite(LP_EN, LOW);
      set_setpoint(_setpoint);
      break;

    case DC_LP:
      //Timer1.stop();
      set_setpoint(_setpoint);
      digitalWrite(LP_EN, HIGH);
      break;

    default:
      // something bad; turn off output.
      //Timer1.stop();
      trip_interlock();

      break;
  }

}

uint16_t Piezo::voltage_to_dac(double voltage){
  // gain = 49.5; 247.5 = 49.5*5
  uint16_t dacVal = (65535.0 * voltage / 247.5);
  return (dacVal);
}

double Piezo::dac_to_voltage(uint16_t dacVal){
  return (dacVal * 247.5 / 65535.0);
}

#endif
