#include "EncoderStream.h"

// declare pin numbers
int const EncoderStream::ENC_A = 0;
int const EncoderStream::ENC_B = 1;
int const EncoderStream::ENC_SW = 2;

void setup(){
  Serial.begin(9600);
}

void loop(){
  while (Encoder.available() > 0){
    Serial.print("Encoder character: ");
    Serial.println(Encoder.read());
  }

  while (Serial.available() > 0){
    char c = Serial.read();
    // enable/disable the encoder!
    if (c == '0') Encoder.disable();
    if (c == '1') Encoder.enable();
  }
}
