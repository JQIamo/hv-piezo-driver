#include <Arduino.h>
#include "SPI.h"

#define AD56XXR_SPI_SPEED 4000000
#include "AD56XXR.h"

#include "LCD.h"
#include "TimerOne.h"

#define STARTUP_SCAN_MIN 0.0
#define STARTUP_SCAN_MAX 10.0
#define STARTUP_SCAN_FREQ 1
#define PIEZO_SCAN_RATE 1 //millis; fix eventually to use us?
#include "piezo.h"
#include "piezo_menu.h"

#include <Thread.h>
#include <ThreadController.h>
//#include "quadEncoder.h"
//#include <keyStream.h>//keyboard driver and fake stream (for the encoder button)
//#include <chainStream.h>// concatenate multiple input streams (this allows adding a button to the encoder)



/*
#define HVMON        //pin 26 | PC3
#define VP_MON      // pin 19 | ADC6
#define VM_MON      // pin 22 | ADC7
*/

// DAC_CS, bits, vref, offset, internal_ref
AD56XXR dac(DAC_SYNC, 16, 5.0, 0.0, 1);

// note, using named singleton class from piezo.h
Piezo pzt(&dac);

// create LCD object
LCD_ST7032 lcd(LCD_RST, LCD_RS, LCD_CS);

// create encoder object
EncoderStream enc(LCDENC);

#define DC_VOLT_NUMSTEPS 3
double dcVoltDelta[DC_VOLT_NUMSTEPS] = {1.0, 0.1, 0.01};
uint8_t dcVoltDeltaIdx = 0;

// threads

Thread scanThread = Thread();
Thread encThread = Thread();
//Thread serThread = Thread();
ThreadController controller = ThreadController();





double voltage_setpoint = 0.0;
void update_voltage(){
  pzt.set_setpoint(voltage_setpoint);
}




//bool scanEnabled = false;
double scan_min = STARTUP_SCAN_MIN;
double scan_max = STARTUP_SCAN_MAX;
unsigned int scan_freq = STARTUP_SCAN_FREQ;

double dc_volt = 0.0;

enum ScanAdjustMode_t { MIN, MAX, FREQ };
// keep track of whether adjusting upper or lower
ScanAdjustMode_t scanAdjustMode = MIN;

//bool mode = false;
//
// void update_scan(){
//   if (scanEnabled){
//     pzt.set_mode(SCAN);
//   } else {
//     pzt.set_mode(DC);
//   }
//
//   pzt.set_scan_min(scan_min);
//   pzt.set_scan_max(scan_max);
//
// }

void increment_scan(){
  pzt.increment_scan();
}

void pollEncoder(){
  // mode == 0, scan
  // mode == 1, DC
  int mode = digitalRead(SW);


  if (mode == 0){
    // put into scan mode if not there already
    if (pzt.get_mode() != SCAN){
      pzt.set_mode(SCAN);
      scanAdjustMode = MIN;
      lcd.setCursor(0, 0);
      lcd.print("min- ");
      lcd.print(pzt.get_scan_min());
      lcd.print("      ");
      lcd.setCursor(0, 1);
      lcd.print("max: ");
      lcd.print(pzt.get_scan_max());
      lcd.print("      ");
      scanThread.enabled = true;
    }

    // check if encoder buffer has anything to process
    if (enc.available() > 0){
      char c = enc.read();
      switch(c){
        case '+':
          switch (scanAdjustMode){
            case MIN:
              scan_min = pzt.set_scan_min(scan_min + 1.0);
              lcd.setCursor(5, 0);
              lcd.print(pzt.get_scan_min());
              break;
            case MAX:
              scan_max = pzt.set_scan_max(scan_max + 1.0);
              lcd.setCursor(5, 1);
              lcd.print(pzt.get_scan_max());
              break;
            case FREQ:
              scan_freq = constrain(scan_freq + 1, 1, 100);
              pzt.set_scan_freq(scan_freq);
              lcd.setCursor(6, 0);
              lcd.print(scan_freq);
              //lcd.print("    ");
              break;
          }
          break;

          case '-':
            switch (scanAdjustMode){
              case MIN:
                scan_min = pzt.set_scan_min(scan_min - 1.0);
                lcd.setCursor(5, 0);
                lcd.print(pzt.get_scan_min());
                break;
              case MAX:
                scan_max = pzt.set_scan_max(scan_max - 1.0);
                lcd.setCursor(5, 1);
                lcd.print(pzt.get_scan_max());
                break;
              case FREQ:
                scan_freq = constrain(scan_freq - 1, 1, 100);
                pzt.set_scan_freq(scan_freq);
                lcd.setCursor(6, 0);
                lcd.print(scan_freq);
                lcd.print("     ");
            }
            break;


            case 'P':
              switch (scanAdjustMode){
                case MIN:
                  scanAdjustMode = MAX;
                  lcd.setCursor(3, 0);
                  lcd.print(":");
                  lcd.setCursor(3, 1);
                  lcd.print("-");
                  break;
                case MAX:
                  scanAdjustMode = FREQ;
                  lcd.setCursor(0, 0);
                  lcd.print("freq- ");
                  lcd.print(pzt.get_scan_freq());
                  lcd.print("      ");
                  lcd.setCursor(0, 1);
                  lcd.print("                ");
                  break;
                case FREQ:
                  scanAdjustMode = MIN;
                  lcd.setCursor(0, 0);
                  lcd.print("min- ");
                  lcd.print(pzt.get_scan_min());
                  lcd.print("      ");
                  lcd.setCursor(0, 1);
                  lcd.print("max: ");
                  lcd.print(pzt.get_scan_max());
                  lcd.print("      ");
                  break;
              }
              break;
      }
    }

  } else {
    // else if mode is DC
    if (pzt.get_mode() != DC_LP){
      scanThread.enabled = false;
      pzt.set_setpoint(dc_volt);
      pzt.set_mode(DC_LP);

      lcd.setCursor(0, 0);
      lcd.write("DC Volt:        ");

      lcd.setCursor(0, 1);
      lcd.print(pzt.get_setpoint());
      lcd.print("             ");
    }

    if (enc.available() > 0){
      char c = enc.read();
      if (c == '+'){
        dc_volt += dcVoltDelta[dcVoltDeltaIdx % DC_VOLT_NUMSTEPS];
      } else if (c == '-'){
        dc_volt -= dcVoltDelta[dcVoltDeltaIdx % DC_VOLT_NUMSTEPS];
      } else if (c == 'P'){
        // just to test hold vs push now...

        dcVoltDeltaIdx++;
      }

      pzt.set_setpoint(dc_volt);
      // make sure we've got the real deal (bounds checking)
      dc_volt = pzt.get_setpoint();
      lcd.setCursor(0, 1);
      lcd.print(pzt.get_setpoint());
      lcd.print("     ");
    }

  }

}

// void serCallback(){
//   Serial.print(scanAdjustFlag);
//   Serial.print(" , scanpts: ");
//   Serial.print(scan_min);
//   Serial.print(scan_max);
//   Serial.print(", DC:");
//   Serial.println(dc_volt);
//   Serial.print("curset: ");
//   Serial.println(pzt.get_setpoint());
//   int v = (PIND & _BV(3));
//   Serial.print("pushed: ");
//   Serial.println(v);
//
// }

void setup() {
  pzt.init();

  //Serial.begin(9600);

  //Serial.println("alive");
  SPI.begin();

  // DAC setup
  dac.setIntRef(1);
  dac.setVolt(0, 0.0);
  dac.setVolt(1, 0.0);
  pzt.set_onoff(1);

  enc.begin();


  // LCD setup
  lcd.begin();
  lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("JQI - Piezo");
  // lcd.setCursor(0, 1);
  // lcd.print("Starting...");

  scanThread.onRun(increment_scan);
  scanThread.setInterval(PIEZO_SCAN_RATE);

  encThread.onRun(pollEncoder);
  encThread.setInterval(50);

  // serThread.onRun(serCallback);
  // serThread.setInterval(2000);

  controller.add(&scanThread);
  controller.add(&encThread);
//  controller.add(&serThread);
//  Serial.println("enabled");
  delay(1000);
  pzt.set_mode(SCAN);
//  Serial.println("scanning");
//  delay(1000);
}



void loop() {
  //Serial.println("going");
//  mainMenu.poll(update_lcd, allIn);
//  pzt.poll();  //Serial.println("pool");
  //delay(1000);
  //quadEncoder.poll();
controller.run();

}
