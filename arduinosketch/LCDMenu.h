


#ifndef LCDMENU_H
#define LCDMENU_H

#import "LCDEncoder.h"
#import "LCD.h"

#define MAX_TITLE 32
#define MAX_SUBMENUS 8


typedef void (*callback_t)();

typedef struct Menu {
  char * title;
  callback_t incr;
  callback_t decr;
  callback_t push;
  callback_t hold;
  // void (*decr)();
  // void (*push)();
  // void (*hold)();
  uint8_t num_submenus;
  uint8_t idx;
  struct Menu *submenu[MAX_SUBMENUS];
  Menu *parent;

  Menu( char * title,
    callback_t incr,
    callback_t decr,
    callback_t push,
    callback_t hold,
    uint8_t num_submenus,
    uint8_t idx,
    Menu submenu[MAX_SUBMENUS],
    Menu * parent ) :
    title(title),
    incr(incr),
    decr(decr),
    push(push),
    hold(hold),
    num_submenus(num_submenus),
    idx(idx),
    submenu(submenu),
    parent(parent)
  {}

} Menu;




class MenuController {
public:
  MenuController(EncoderStream * enc, Menu * top_menu);

  void run();
  void draw();

  void menu_up();
  void menu_down();
  void menu_incr();
  void menu_decr();

private:
  EncoderStream * _enc;
  Menu * _top_menu;
  Menu * _curr_menu;
};

MenuController::MenuController(EncoderStream * enc, Menu * top_menu) :
  _enc(enc), _top_menu(top_menu)
{

}

void MenuController::menu_up(){
  if (_curr_menu->parent != 0){
    _curr_menu = _curr_menu->parent;
    draw();
  }
}

void MenuController::menu_down(){
  if (_curr_menu->submenu[_curr_menu->idx] != nullptr){
    _curr_menu = _curr_menu->submenu[_curr_menu->idx];
    draw();
  }
}

void MenuController::menu_incr(){
  _curr_menu->idx = (_curr_menu->idx + 1) % _curr_menu->num_submenus;
  draw();
}

void MenuController::menu_decr(){
  _curr_menu->idx = (_curr_menu->idx - 1) % _curr_menu->num_submenus;
  draw();
}

void MenuController::run(){
  switch(_enc->peek()){
    case '+':
      _curr_menu->incr();
      break;
    case '-':
      _curr_menu->decr();
      break;
    case 'P':
      _curr_menu->push();
      break;
    case 'H':
      _curr_menu->hold();
      break;
  }
}


extern MenuController MainMenuController;

class MenuActions {
  static void incr();
  static void decr();
  static void push();
  static void hold();
};

void MenuActions::incr(){
  MainMenuController.menu_incr();
}
void MenuActions::decr(){
  MainMenuController.menu_decr();
}
void MenuActions::push(){
  MainMenuController.menu_down();
}
void MenuActions::hold(){
  MainMenuController.menu_up();
}

#endif
