#ifndef PIEZO_MENUS_H
#define PIEZO_MENUS_H

#include "Menu.h"
#include "LCD.h"


#include "piezo_registers.h"
#include "piezo_api.h"

extern LCD_ST7032 lcd;
extern Piezo piezo;

/****************** PIEZO menu **************************/

void enter_piezo(double & volt){
  volt = piezo.val_to_volt(regRead(PIEZO));
}

void update_piezo(double & volt){
  volt = piezo.setVolt(volt);

  #ifdef DEBUG
    Serial.print("setval: ");
    Serial.print(regRead(PIEZO));
    Serial.print("\tmeasVal: ");
    uint16_t mval = analogRead(HV_MON);
    Serial.print(mval);
    Serial.print("\tCalibVal: ");
    Serial.println(measured_to_val(mval));
  #endif
}


/****************** PIEZOMIN menu **************************/
void enter_min(double & volt){
  volt = piezo.val_to_volt(regRead(PIEZOMIN));
}

void update_min(double & volt){
  uint16_t new_min;
  uint16_t old_max = regRead(PIEZOMAX);

  // first, constrain absolute bounds so we don't roll over zero in
  // volt_to_val conversion
  volt = constrain(volt, 0, PIEZO_VOLTRANGE);

  new_min = constrain(piezo.volt_to_val(volt), 0, old_max);
  regWrite(PIEZOMIN, new_min);

  // now, set to actual register value
  volt = piezo.val_to_volt(new_min);
}

void exit_min(double & volt){
  persistReg(PIEZOMIN);
}

/****************** PIEZOMAX menu **************************/

void enter_max(double & volt){
  volt = piezo.val_to_volt(regRead(PIEZOMAX));
}

void update_max(double & volt){
  uint16_t new_max;
  uint16_t old_min = regRead(PIEZOMIN);
  // first, constrain absolute bounds so we don't roll over zero in
  // volt_to_val conversion
  volt = constrain(volt, 0, PIEZO_VOLTRANGE);

  new_max = constrain(piezo.volt_to_val(volt), old_min, 65535);
  regWrite(PIEZOMAX, new_max);

  // now, set to actual register value
  volt = piezo.val_to_volt(new_max);
}

void exit_max(double & volt){
    persistReg(PIEZOMAX);
}

/****************** LoopFilter DEADBAND menu *********************/

void enter_dead(double & volt){
  volt = piezo.val_to_volt(regRead(LF_DEADBAND));
}

void update_dead(double & volt){
  volt = constrain(volt, 0.0, 5.0);
}

void exit_dead(double & volt){
  regWrite(LF_DEADBAND, piezo.volt_to_val(volt));
  persistReg(LF_DEADBAND);
}

/****************** LoopFilter update period menu *************/

void update_tau(int & tau){
  tau = constrain(tau, 10, 60000);
}
void enter_tau(int & tau){
  tau = constrain(regRead(LF_TAU), 10, 60000);
}
void exit_tau(int & tau){
  regWrite(LF_TAU, tau);
  persistReg(LF_TAU);
}

/*************** LoopFilter update stepsize menu *************/


void enter_step(double & volt){
  volt = piezo.val_to_volt(regRead(LF_STEP));
}

void update_step(double & volt){
  volt = constrain(volt, 0.0, 5.0);
}

void exit_step(double & volt){
  regWrite(LF_STEP, piezo.volt_to_val(volt));
  persistReg(LF_STEP);
}


double Volt_incr[5] = {10.0, 1.0, 0.1, 0.01, 0.004};
uint8_t Volt_cursor[5] = {1, 2, 4, 5, 6};

double SmallVolt_incr[4] = {1.0, 0.1, 0.01, 0.004};
uint8_t SmallVolt_cursor[4] = {0, 2, 3, 4};

int ms_incr[4] = {1000, 100, 10, 1};
uint8_t ms_cursor[4] = {9, 10, 11, 12};

MenuNumeric<double> VoltMenu = MenuNumeric<double>::create("DC Setpoint      ", &lcd)
    .line0("Volt:           ")
    .line1("               V")
    .incr(Volt_incr, 5, 1)
    .cursors(Volt_cursor)
    .formatter("%07.3f", 0, 1)
    .update_cb(update_piezo)
    .enter_cb(enter_piezo);

MenuNumeric<double> VoltMinMenu = MenuNumeric<double>::create("Min Voltage     ", &lcd)
    .line0("Min Volt:       ")
    .line1("               V")
    .incr(Volt_incr, 5, 1)
    .cursors(Volt_cursor)
    .formatter("%07.3f", 0, 1)
    .update_cb(update_min)
    .enter_cb(enter_min)
    .exit_cb(exit_min);

MenuNumeric<double> VoltMaxMenu = MenuNumeric<double>::create("Max Voltage     ", &lcd)
    .line0("Max Volt:       ")
    .line1("               V")
    .incr(Volt_incr, 5)
    .cursors(Volt_cursor)
    .formatter("%07.3f", 0, 1)
    .update_cb(update_max)
    .enter_cb(enter_max)
    .exit_cb(exit_max);

MenuNumeric<double> LFDeadMenu = MenuNumeric<double>::create("Deadband        ", &lcd)
    .line0("FB Deadband:    ")
    .line1("               V")
    .incr(SmallVolt_incr, 4)
    .cursors(SmallVolt_cursor)
    .formatter("%05.3f", 0, 1)
    .update_cb(update_dead)
    .enter_cb(enter_dead)
    .exit_cb(exit_dead);

MenuNumeric<int> LFTauMenu = MenuNumeric<int>::create("Update Rate     ", &lcd)
    .line0("Update Rate:    ")
    .line1("              ms")
    .incr(ms_incr, 4)
    .cursors(ms_cursor)
    .formatter("%13d", 0, 1)
    .update_cb(update_tau)
    .enter_cb(enter_tau)
    .exit_cb(exit_tau);

MenuNumeric<double> LFStepMenu = MenuNumeric<double>::create("Update stepsize ", &lcd)
    .line0("Update stepsize:")
    .line1("               V")
    .incr(SmallVolt_incr, 4)
    .cursors(SmallVolt_cursor)
    .formatter("%05.3f", 0, 1)
    .update_cb(update_step)
    .enter_cb(enter_step)
    .exit_cb(exit_step);


void calibrate_select_callback(int & v){
  calibrate();
}

MenuSelection CalibrateMenu("calibrate       ", &lcd, calibrate_select_callback);

void lp_select_callback(int & v){
  if (regRead(PIEZOLP)){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("LP disabled!");
    regWrite(PIEZOLP, 0);
    persistReg(PIEZOLP);
    piezo.disableLP();
    delay(1500);
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("LP enabled!");
    regWrite(PIEZOLP, 1);
    persistReg(PIEZOLP);
    piezo.enableLP();
    delay(1500);
  }
}
MenuSelection LPMenu("Lowpass en/dis  ", &lcd, lp_select_callback);


MenuController root("Menu >          ", &lcd);
MenuController config_menu("Settings >      ", &lcd);
MenuController loopfilter_menu("Feedback >      ", &lcd);


#endif
