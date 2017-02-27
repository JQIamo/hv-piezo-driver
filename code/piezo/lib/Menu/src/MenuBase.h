#ifndef MENU_BASE_H
#define MENU_BASE_H

// #define MENU_SERIAL_DEBUG

//#include "Arduino.h"

#include "LCD.h"

class Menu {
public:
  Menu(const char * display_name, LCD * display);
  virtual void enter();
  virtual void exit();
  virtual void update();
  virtual void process(char c);

  // name mangled... not meant to be used externally
  Menu * _parent;
  Menu * _active;
  char _display_name[17];

protected:
  LCD * _display;
};







#endif
