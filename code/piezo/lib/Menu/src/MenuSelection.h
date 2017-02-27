#ifndef MENU_SELECTION_H
#define MENU_SELECTION_H

#include "Menu.h"

class MenuSelection : public Menu {
public:
  typedef void (*Callback)(int&);

  /** MenuSelection: constructor for Selection menu class

      Tip for exit menu: don't pass callback; will automatically re-enter
      parent menu.
  */
  MenuSelection(
    const char * display_name,
    LCD * output,
    Callback selected_callback = 0,
    int selected_id = 0
  );

  void enter();

private:
  Callback _selected_callback;
  int _selected_id;

};

#endif
