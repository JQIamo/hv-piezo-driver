
// simple quad encoder driver; uses INT0,1 pin HW interrupt on Atmega328P

#ifndef LCDENCODER_H
#define LCDENCODER_H

#import "Arduino.h"




#define PUSH_TIMEOUT 10
#define HOLD_TIMEOUT 1000

// assuming encoder is on INT0, INT1 pins.

struct Encoder {
  volatile int16_t pos = 0;
  //volatile bool state = 1;
  volatile bool flag = 0;
  volatile uint32_t millis_down = 0;
  volatile uint32_t millis_up = 0;
} LCDENC;


void encoder_a_has_changed(){
  if (digitalReadFast(ENC_B)){
  

    LCDENC.pos++;
  } else {
    LCDENC.pos--;
  }
}

// for switch; hopefully doesn't get out of sync!
void encoder_sw_has_changed(){
  //Serial.println("r");
    // if interrupted and enc is high == up;
    if(digitalReadFast(ENC_SW)){
    LCDENC.millis_up = millis();
    LCDENC.flag = 1;
    } else {
          LCDENC.millis_down = millis();
  
    LCDENC.flag = 0;
      
    }
   // Serial.println(LCDENC.millis_up);
}

void encoder_sw_has_fallen(){
  Serial.println("f");
    // if enc is DOWN, record push
    LCDENC.millis_down = millis();
  
    LCDENC.flag = 0;
}

void init_encoder(int e_a,int e_b,int e_sw){
attachInterrupt(e_a,encoder_a_has_changed,FALLING);
attachInterrupt(e_sw,encoder_sw_has_changed,CHANGE);
//attachInterrupt(e_sw,encoder_sw_has_fallen,FALLING);
}



// class Encoder {
// public:
//   // pin A is used as interrupt; pin B polled for direction information.
//   // interrupt on falling edge
//   Encoder(int pin_A, int pin_B, int pin_SW);
//   volatile int16_t pos;
//   static void update(){
//     pos
//   }
//
// private:
//   int _a, _b, _sw;
//
// };

class EncoderStream : public Stream {
public:

  //EncoderStream(Encoder & enc) : enc(enc), _buff_en(0){};
EncoderStream(Encoder & enc) : enc(enc){};
  // enable it to buffer
  //EncoderStream(Encoder & enc, bool buff_en) : enc(enc), _buff_en(buff_en){};


  Encoder & enc;
  int oldPos;

  // begin
  void begin(){
    //The following code was necessary with the ATMEGA chip. MK20 architecture has different interrupt capabilities
    // set up interrupts for both INT0, 1 only...
    //EICRA = _BV(3);
    //EICRA = 0x07; // binary 0b0110; INT0 on falling edge, INT1 on change
    //EIMSK = 0x03;
  }

  int available() {
//    if (_buff_en){
  //    return abs(enc.pos - oldPos);
  //  } else {

      return (enc.pos != oldPos || enc.flag);
//      return enc.pos != oldPos;
    //}
  }

  int peek() {
    if (enc.pos < oldPos) return '-';
    if (enc.pos > oldPos) return '+';
    if (enc.flag) {
      if ((enc.millis_up - enc.millis_down) > HOLD_TIMEOUT) return 'H';
      if ((enc.millis_up - enc.millis_down) > PUSH_TIMEOUT) return 'P';
    }
     // what about hold?
    // need to fix this!! FIX
    //if (_push_action || (PIND & _BV(6))) return _push_action;
    return -1;
  }

  int read() {

    if (enc.pos < oldPos) {
      //oldPos = _buff_en ? oldPos + 1 : enc.pos;
      oldPos = enc.pos;
      return '-';
    }
    if (enc.pos > oldPos) {
      //oldPos = _buff_en ? oldPos + 1 : enc.pos;
      oldPos = enc.pos;
      return '+';
    }

    if (enc.flag) {
      enc.flag = 0;
      if ((enc.millis_up - enc.millis_down) > HOLD_TIMEOUT) return 'H';
      if ((enc.millis_up - enc.millis_down) > PUSH_TIMEOUT) return 'P';
    }

    // push button... if encoder not rotating
    //if (!digitalRead(ENC_SW) && ! _sw_pushed){
    // custom... switch on PD6

    // switch state: bit 1 = ~ switch; bit 0 = internal switch state tracker
    // note: because of pullup, switch -> 1 = off, switch -> 0 = on
    //  0 0 -> pushed, wasn't pushed last time = pushed state
    //  0 1 -> pushed, was pushed last time = hold vs push state timeout
    //  1 0 -> not pushed, wasn't pushed last time = nothing
    //  1 1 -> not pushed, was pushed last time = released state
    // bool switch_state = PIND & _BV(6);
    // int state = (switch_state << 1) | _sw_pushed;
    // int curr_millis;
    // Serial.print("State ");
    // Serial.println(state);
    //
    // switch (state){
    //   case 0:
    //     // pushed, wasn't pushed last time; change to pushed state
    //     _sw_pushed = true;
    //     _push_millis = millis();
    //     break;
    //   case 1:
    //     // pushed, was pushed last time; check if holding or pushing
    //     curr_millis = millis();
    //     if (curr_millis - _push_millis > HOLD_TIMEOUT){
    //       _push_action = 'H';
    //     } else if (curr_millis - _push_millis > PUSH_TIMEOUT){
    //       _push_action = 'P';
    //     } else {
    //       _push_action = 0;
    //     }
    //     break;
    //   //case 2:
    //     // not pushed, wasn't pushed last time so do nothing
    //     //break;
    //   case 3:
    //     // not pushed, was pushed last time so release state
    //     int action = _push_action;
    //     _push_action = 0;
    //
    //     return action;
    //     //break;
    // }

    return -1;
  }


  void flush() {
    oldPos = enc.pos;
    enc.flag = 0;
  }

  size_t write(uint8_t val) {
    oldPos = val;
    enc.pos = val;  // update encoder to this position as well...
    return 1;
  }

//private:
  //bool _buff_en;

  // for push button/hold
  // uint16_t _push_millis;
  //bool _sw_pushed, _disable_push;
  //int _push_action;

};


#endif
