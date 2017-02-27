#ifndef MENUENCODER_H
#define MENUENCODER_H

#include <Arduino.h>

#ifndef HOLD_TIMEOUT
  #define HOLD_TIMEOUT 300
#endif

#ifndef PUSH_TIMEOUT
  #define PUSH_TIMEOUT 50
#endif

namespace EncoderStream {
  extern int const ENC_A;
  extern int const ENC_B;
  extern int const ENC_SW;

  typedef struct Encoder_t {
      volatile int16_t pos = 0;
      volatile bool flag = 0;
      volatile unsigned long millis_down = 0;
      volatile unsigned long millis_up = 0;
  } Encoder_t;

  // update functions; meant to be attached to ISR in Encoder.begin()
  void pollEncoder();
  void pollEncoderSW();

};


class EncoderWrapper : public Stream {
public:

  /** EncoderWrapper: constructor for wrapper on encoder stream.

    @param encoder Encoder_t struct representing the encoder state.

    This class should be used with the singleton object `Encoder()`,
    declared in EncoderStream.cpp.

  */
  EncoderWrapper(EncoderStream::Encoder_t & encoder);

  //! enc: reference to encoder struct.
  EncoderStream::Encoder_t & enc;

  //! oldPos: old encoder position; used to determine what char to emit next.
  int oldPos;

  /** begin: sets pin states and attaches interrupts.

    In your sketch you should define the pins like

      int const EncoderStream::ENC_A = 0;
      int const EncoderStream::ENC_B = 1;
      int const EncoderSTream::ENC_SW = 2;

    such that they can be attached to the right ISR.
  */
  void begin();

  //! available: returns the number of characters available on the Encoder stream.
  int available();

  //! peek: returns next character on the encoder stream without popping it off.
  int peek();

  /** read: pops and returns next character on the encoder stream.

    valid characters (currently) are:

    '+': encoder incremented;
    '-': encoder decremented;
    'P': encoder switch was pushed (momentarily);
    'H': encoder switch was held
  */
  int read();

  //! flush: flushes encoder stream
  void flush();

  //! enable: enable encoder (turns on interrupts)
  inline void enable(){
    attachInterrupt(EncoderStream::ENC_A, EncoderStream::pollEncoder, FALLING);
    attachInterrupt(EncoderStream::ENC_SW, EncoderStream::pollEncoderSW, CHANGE);
  }

  //! disable: disable encoder (turns off interrupts)
  inline void disable(){
    detachInterrupt(EncoderStream::ENC_A);
    detachInterrupt(EncoderStream::ENC_SW);
  }

  /** write: write position to encoder.

    @param val Value to write to encoder.

    This function isn't really useful, but Stream declares a pure virtual
    function `write` and so we must override.
  */
  size_t write(uint8_t val);


private:
  int _a, _b, _sw;
};

extern EncoderWrapper Encoder;

#endif
