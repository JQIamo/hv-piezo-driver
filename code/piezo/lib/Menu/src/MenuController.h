#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include "Menu.h"
#include "LinkedList.h"

class MenuController : public Menu {
public:
  MenuController(const char * display_name, LCD * output);

  void add(Menu * menu);

  void enter();
  void update();
  void exit();
  void process(char c);

private:
  LinkedList<Menu*> _menus;
  uint8_t _current;
  int _menu_counter;

};

// Root menu singleton
extern MenuController root;

#endif
