#include "MenuSelection.h"

MenuSelection::MenuSelection(
      const char * display_name,
      LCD * output,
      Callback selected_callback,
      int selected_id)
  : Menu(display_name, output)
{
  strcpy(_display_name, display_name);
  _selected_callback = selected_callback;
  _selected_id = selected_id;
}

void MenuSelection::enter(){
  if (_selected_callback != 0) (*_selected_callback)(_selected_id);
  if (_parent != 0) _parent->enter();
}
