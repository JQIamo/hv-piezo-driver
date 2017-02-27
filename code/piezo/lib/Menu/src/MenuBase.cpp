#include "Menu.h"

Menu::Menu(const char * display_name, LCD * display) {
  _parent = 0;
  strcpy(_display_name, display_name);
  _display = display;
}

void Menu::enter(){}
void Menu::exit(){}
void Menu::update(){}
void Menu::process(char c){}
