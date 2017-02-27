#ifndef LOOPFILTER_H
#define LOOPFILTER_H

#include "Thread.h"
#include "StaticThreadController.h"

#include "piezo_api.h"

// #define LOOPFILTER_SERIAL_DEBUG

class LoopFilter {
public:
  LoopFilter(){
    _feedback = new Thread(LoopFilter::updateHV, 500);
    _updateLCD = new Thread(LoopFilter::updateLCD, 1000);

    _thread_controller = new StaticThreadController<2>(_feedback, _updateLCD);
    _thread_controller->enabled = false;
  };


  // possible initialization here...?
  static void begin(){

  }

  static void updateLCD(){
    uint16_t current_val = measured_to_val(analogRead(HV_MON));
    lcd.setCursor(0, 1);
    lcd.printf("%07.3f", piezo.val_to_volt(current_val));

    #ifdef LOOPFILTER_SERIAL_DEBUG
      Serial.print("measVal: ");
      Serial.print(current_val);
      Serial.print("\tmeasVolt: ");
      Serial.println(piezo.val_to_volt(current_val));
    #endif
  }

  static void updateHV(){
    int current_val = (int)measured_to_val(analogRead(HV_MON));
    int set_val = (int)regRead(PIEZO);
    int err = set_val - current_val;

    #ifdef LOOPFILTER_SERIAL_DEBUG
      Serial.println("-----LoopFilter------");
      Serial.print("measVal: ");
      Serial.print(current_val);
      Serial.print("\tSetVal: ");
      Serial.print(set_val);
      Serial.print("\terr:");
      Serial.println(err);
    #endif

    if (abs(err) > regRead(LF_DEADBAND)){
      if (err > 0){
        set_val -= regRead(LF_STEP);
      } else {
        set_val += regRead(LF_STEP);
      }
      set_val = constrain(set_val, 0, 65535);
      regWrite(PIEZO, (uint16_t)set_val);
      piezo.update();

      #ifdef LOOPFILTER_SERIAL_DEBUG
        Serial.print("outside deadband of ");
        Serial.print(regRead(LF_DEADBAND));
        Serial.print("\tupdating to: ");
        Serial.println(set_val);
      #endif
    }

    #ifdef LOOPFILTER_SERIAL_DEBUG
      Serial.println("--------------------");
    #endif
  }

  void start(){
    // TODO: should we deactivate processing encoder? drop into DC val menu?
    // encoder can now be disabled with Encoder.disable()
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lock Enabled    ");
    lcd.setCursor(0, 1);
    lcd.print("               V");
    lcd.setCursor(0, 1);

    _feedback->setInterval(regRead(LF_TAU));
    _thread_controller->enabled = true;
  };

  void stop(){
    _thread_controller->enabled = false;
    root._active->enter();
  };

  void process(){
   if (!digitalReadFast(SW) && !_thread_controller->enabled){
      start();
    } else if (digitalReadFast(SW) && _thread_controller->enabled){
      stop();
    }

    _thread_controller->run();
  }

private:

  Thread * _feedback;
  Thread * _updateLCD;
  StaticThreadController<2> * _thread_controller;
};

#endif
