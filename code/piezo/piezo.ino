#include <SPI.h>
#include <EEPROM.h>

// #define DEBUG

#include "Thread.h"
#include "StaticThreadController.h"

#include "LCD.h"
#include "Menu.h"
#include "EncoderStream.h"

#include "src/piezo_registers.h"
#include "src/piezo_api.h"
#include "src/piezo_menus.h"
#include "src/loopfilter_module.h"
//#include "src/scan_module.h"

Piezo piezo;

LoopFilter feedback;

LCD_ST7032 lcd(LCD_RST, LCD_RS, LCD_CS);
//
// enum enc_pins {ENC_A = 0, ENC_B = 5, ENC_SW = 1, SW = 6};

using namespace EncoderStream;

int const EncoderStream::ENC_A = 0;
int const EncoderStream::ENC_B = 5;
int const EncoderStream::ENC_SW = 1;


void setup(){

  // start up with DRV2700 disabled
  pinMode(EN, OUTPUT);
  digitalWriteFast(EN, HIGH);

  pinMode(LCD_EN, OUTPUT);
  digitalWriteFast(LCD_EN, HIGH);



  SPI.begin();
  delay(100);


  #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial){ ; }
  #endif

  Encoder.begin();
  lcd.begin();
  lcd.cursor();

  // construct menu system
  root.add(&VoltMenu);
  root.add(&config_menu);

  config_menu.add(&VoltMinMenu);
  config_menu.add(&VoltMaxMenu);
  config_menu.add(&LPMenu);
  config_menu.add(&loopfilter_menu);
  config_menu.add(&CalibrateMenu);

  loopfilter_menu.add(&LFDeadMenu);
  loopfilter_menu.add(&LFTauMenu);
  loopfilter_menu.add(&LFStepMenu);

  root.enter();

  piezo.begin(DAC_CS, DAC_CLR, DAC_LDAC);

  #ifdef DEBUG
    Serial.println("Calibration:");
    for (int i = 0; i < CALIBRATION_POINTS; i++){
      uint32_t calibVal = regRead32(PIEZO_CALIB + i);
      uint16_t val = (uint16_t)calibVal;

      Serial.print(val);
      Serial.print("\t");
      val = (uint16_t)(calibVal >> 16);
      Serial.println(val);
    }
  #endif

}

void loop(){

  if (Encoder.available() > 0){
    char c = Encoder.read();
    root._active->process(c);
  }

  feedback.process();

}
