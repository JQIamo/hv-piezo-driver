#ifndef MENU_NUMERIC_H
#define MENU_NUMERIC_H

#include "Menu.h"

template <typename V>
class MenuNumeric : public Menu {
public:
  
  /** Callback: typedef for callback function.

    Function signature must be `void(V &)`; the callback will be passed the
    value stored in `val`.
  */
  typedef void (*Callback)(V&);

  //! val: (templated) value indicated by MenuNumeric.
  V val;

  MenuNumeric(const char * display_name, LCD * display);

  // using named parameter idiom to avoid massive constructor parameter list;
  // see https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter

  /** create: static function which returns an instance of MenuNumeric.

    @param display_name 16-character string to be displayed from parent menu.
    @param display Pointer to LCD object used for menu display.

    This function is designed to be used with the named parameter idiom
    (see https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter).

    Example:

      double Volt_incr[5] = {10.0, 1.0, 0.1, 0.01, 0.004};
      uint8_t Volt_cursor[5] = {1, 2, 4, 5, 6};

      MenuNumeric<double> VoltMenu = MenuNumeric<double>::create("Voltage Setpoint", &lcd)
          .line0("Volt:           ")
          .line1("               V")
          .incr(Volt_incr, 5, 1)
          .cursors(Volt_cursor)
          .formatter("%07.3f", 0, 1)
          .update_cb(update_voltage)
          .enter_cb(enter_voltage);
  */
  static MenuNumeric create(const char * display_name, LCD * display);

  /** line0: sets display string for first line

    @param line 16-character string literal for display on first line.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & line0(const char * line);

  /** line1: sets display string for second line

    @param line 16-character string literal for display on second line.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & line1(const char * line);

  /** incr: Specifies increment sizes for controlling numeric value.

    @param increments Pointer to head of increment array
    @param count Number of increments expected (eg, length of incremnet array)
    @param incr_counter Position of increment to initialize menu with. Defaults to zero (first increment value).

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & incr(V * increments, uint8_t count, uint8_t incr_counter = 0);

  /** cursors: Specifies cursor positions corresponding to digit affected by incr.

    @param cursor_offsets Pointer to head of cursor offsets array, which must be of same length as `increments`.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & cursors(uint8_t * cursor_offsets);

  /** formatter: Specifies how to format indicated numeric value.

    @param formatter Format string used to format numeric value; see printf for details.
    @param col Column on display to start writing formatted number.
    @param row Row on display to start writing formatted number.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & formatter(const char * formatter, uint8_t col, uint8_t row);

  /** update_cb: Callback to be executed when menu is updated.

    @param c Callback with signature `void(V &)`

    The callback is passed a reference to the requested "new" value of the
    numeric menu; this allows the menu implementation to, for example, implement
    bounds checking or other such logic before the display is updated.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & update_cb(Callback c);

  /** enter_cb: Callback to be executed when menu is updated.

    @param c Callback with signature `void(V &)`

    The callback is passed a reference to the value of the
    numeric menu; this allows the menu implementation to modify the value if
    needed before the display is updated.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & enter_cb(Callback c);

  /** exit_cb: Callback to be executed when menu is updated.

    @param c Callback with signature `void(V &)`

    The callback is passed a reference to the value of the
    numeric menu; this allows the menu implementation to modify the value if
    needed, or persist the value to some non-volatile memory.

    This function is meant to be chained and called after MenuNumeric::Create,
    as described in https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Named_Parameter
  */
  MenuNumeric & exit_cb(Callback c);

  /** enter: called when menu is entered.

    Executes callback set with `enter_cb`, if any, and updates display.
  */
  void enter();

  /** update: called when menu needs to be updated, eg, in response to change of numeric value.

    Executes callback set with `update_cb`, if any, and updates display.
  */
  void update();

  /** exit: called at menu exit.

    Executes callback set with `exit_cb`, if any, and passes control to parent menu.
  */
  inline void exit(){
    if (_exit_callback != 0) (*_exit_callback)(val);
  };

  /** process: processes user input and calls update/exit as needed.

    @param c Caracter to be processed.

    Valid characters are:

    '+': Increment indicated value by amount given in `incr` array.
    '-': Decrement indicated value by amount given in `incr` array.
    'P': Cycle to next increment value in `incr` array.
    'H': Exit menu.
  */
  void process(char c);


private:
  char _line0[17];
  char _line1[17];
  char _formatter[16];
  V * _increments;
  uint8_t _incr_counter, _incr_count;
  uint8_t * _cursor_offsets;
  uint8_t _editcol, _editrow;
  Callback _enter_callback, _update_callback, _exit_callback;

};

// implemenation included here; see http://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
#include "MenuNumeric.tpp"

#endif
