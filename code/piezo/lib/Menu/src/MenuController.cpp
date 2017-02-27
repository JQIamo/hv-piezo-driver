#include "MenuController.h"


MenuController::MenuController(const char * display_name, LCD * output)
  : Menu(display_name, output)
{
  _current = 0;
  _menu_counter = 0;
};

void MenuController::add(Menu * menu){
  _menus.add(menu);
  menu->_parent = this;
};

void MenuController::enter(){
  root._active = this;

  #ifdef MENU_SERIAL_DEBUG
    Serial.println("activate parent ctl");
    Serial.println(_display_name);
  #else
    _display->setCursor(0,0);
    _display->print(_display_name);
  #endif

  update();
};

void MenuController::update(){
  #ifdef MENU_SERIAL_DEBUG
    Serial.println("\n----------------");
    Serial.println(_menus.get(_current)->_display_name);
    Serial.println("----------------");
  #else
    _display->setCursor(0, 1);
    _display->print(_menus.get(_current)->_display_name);
  #endif
}

void MenuController::exit(){
  if (_parent != 0) _parent->enter();
};

void MenuController::process(char c){
  switch (c){
    case '+':
      _menu_counter++;
      _current = _menu_counter % _menus.size();
      update();
      break;

    case '-':
      _menu_counter--;
      if (_menu_counter < 0){
        _menu_counter = _menus.size() - 1;
      }

      _current = _menu_counter % _menus.size();
      update();
      break;

    case 'P':
      Menu * next;
      next = _menus.get(_current);
      root._active = next;
      next->enter();
      break;

    case 'H':
      exit();
      break;
  }
}
