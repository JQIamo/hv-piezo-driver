#include "EncoderStream.h"

namespace EncoderStream {

  extern int const ENC_A;
  extern int const ENC_B;
  extern int const ENC_SW;

  // singleton; so far only supports one encoder stream.
  Encoder_t ENCODER0;

  // update functions; meant to be attached to ISR
  void pollEncoder(){
    digitalReadFast(ENC_B) ? ENCODER0.pos++ : ENCODER0.pos--;
  }

  void pollEncoderSW(){
    if(digitalReadFast(ENC_SW)){
      ENCODER0.millis_up = millis();
      ENCODER0.flag = 1;
    } else {
      ENCODER0.millis_down = millis();
      ENCODER0.flag = 0;
    }
  }
};

EncoderWrapper::EncoderWrapper(EncoderStream::Encoder_t & encoder)
  : enc(encoder)
{
  oldPos = 0;
}

void EncoderWrapper::begin(){
  pinMode(EncoderStream::ENC_A, INPUT_PULLUP);
  pinMode(EncoderStream::ENC_B, INPUT_PULLUP);
  pinMode(EncoderStream::ENC_SW, INPUT_PULLUP);

  attachInterrupt(EncoderStream::ENC_A, EncoderStream::pollEncoder, FALLING);
  attachInterrupt(EncoderStream::ENC_SW, EncoderStream::pollEncoderSW, CHANGE);
}

int EncoderWrapper::available() {
    return (enc.pos != oldPos || enc.flag);
}

int EncoderWrapper::peek() {
  if (enc.pos < oldPos) return '-';
  if (enc.pos > oldPos) return '+';
  if (enc.flag) {
    if ((enc.millis_up - enc.millis_down) > HOLD_TIMEOUT) return 'H';
    if ((enc.millis_up - enc.millis_down) > PUSH_TIMEOUT) return 'P';
  }

  return -1;
}

int EncoderWrapper::read() {

  if (enc.pos < oldPos) {
    oldPos = enc.pos;
    return '-';
  }

  if (enc.pos > oldPos) {
    oldPos = enc.pos;
    return '+';
  }

  if (enc.flag) {
    enc.flag = 0;
    if ((enc.millis_up - enc.millis_down) > HOLD_TIMEOUT) {
      return 'H';
    } else if ((enc.millis_up - enc.millis_down) > PUSH_TIMEOUT){
      return 'P';
    }
  }

  return -1;
}

void EncoderWrapper::flush() {
  oldPos = enc.pos;
  enc.flag = 0;
}

size_t EncoderWrapper::write(uint8_t val){
  oldPos = val;
  enc.pos = val;
  return 1;
}

// declare singleton instance Encoder
// TODO: allow multiple encoders? eg, Encoder1, Encoder2, ...
EncoderWrapper Encoder(EncoderStream::ENCODER0);
