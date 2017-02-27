
template <typename V>
MenuNumeric<V> MenuNumeric<V>::create(const char * display_name, LCD * display){
  return MenuNumeric(display_name, display);
}

template <typename V>
MenuNumeric<V>::MenuNumeric(const char * display_name, LCD * display)
  : Menu(display_name, display)
{
  strcpy(_line0, "                ");
  strcpy(_line1, "                ");
  strcpy(_formatter, "");

  _editcol = 0;
  _editrow = 0;

  _increments = 0;
  _incr_count = 0;
  _cursor_offsets = 0;
  _incr_counter = 0;

  _update_callback = 0;
  _enter_callback = 0;
  _exit_callback = 0;

};

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::line0(const char * line){
  strcpy(_line0, line);
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::line1(const char * line){
  strcpy(_line1, line);
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::incr(V * increments, uint8_t count, uint8_t incr_counter){
  _increments = increments;
  _incr_count = count;
  _incr_counter = incr_counter;
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::cursors(uint8_t * cursor_offsets){
  _cursor_offsets = cursor_offsets;
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::formatter(const char * formatter, uint8_t col, uint8_t row){
  strcpy(_formatter, formatter);
  _editcol = col;
  _editrow = row;
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::update_cb(Callback c){
  _update_callback = c;
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::enter_cb(Callback c){
  _enter_callback = c;
  return *this;
}

template <typename V>
MenuNumeric<V> & MenuNumeric<V>::exit_cb(Callback c){
  _exit_callback = c;
  return *this;
}

template <typename V>
void MenuNumeric<V>::enter(){
  if (_enter_callback != 0) (*_enter_callback)(val);

  #ifdef MENU_SERIAL_DEBUG
    Serial.println("----------------");
    Serial.println(_line0);
    Serial.println(_line1);
    Serial.println("----------------");
    update(val);
  #else
    _display->setCursor(0,0);
    _display->print(_line0);
    _display->setCursor(0,1);
    _display->print(_line1);
  #endif

  update();
}


template <typename V>
void MenuNumeric<V>::update(){
  if (_update_callback != 0) (*_update_callback)(val);

  #ifdef MENU_SERIAL_DEBUG
    Serial.println("----------------");
    Serial.printf(_formatter, val);
    Serial.println("\n----------------");
  #else
    _display->setCursor(_editcol, _editrow);
    _display->printf(_formatter, val);
    _display->setCursor(_editcol + _cursor_offsets[_incr_counter % _incr_count], _editrow);
  #endif
}


template <typename V>
void MenuNumeric<V>::process(char c){
  switch(c) {
    case '+':
      val += _increments[_incr_counter % _incr_count];
      update();
      break;
    case '-':
      val -= _increments[_incr_counter % _incr_count];
      update();
      break;
    case 'P':
      _incr_counter++;

      #ifdef MENU_SERIAL_DEBUG
        Serial.print("Cursor at: ");
        Serial.println(_cursor_offsets[_incr_counter % _incr_count]);
      #else
        _display->setCursor(_editcol + _cursor_offsets[_incr_counter % _incr_count], _editrow);
      #endif

      break;
    case 'H':
      exit();
      if (_parent != 0) _parent->enter();
      break;
  }
}
