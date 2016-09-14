#ifndef PIEZO_MENU
#define PIEZO_MENU

//#include "Arduino.h"
//#include "LCDMenu.h"

#include "LCDEncoder.h"


  //Menu main_menu('\0', nullptr, nullptr, nullptr, nullptr, 0, 0, nullptr, 0);//,

//  .incr = &MenuActions::incr,
//  .decr = &MenuActions::decr,
//  .push = &MenuActions::push,
//  .hold = &MenuActions::hold,
//  .num_submenus = 3,
//  .idx = 0,
//  .submenu = { &sub1, &sub2, &sub3 },
//  .parent = 0
// };




// typedef struct Menu {
//   char title[MAX_TITLE];
//   void (*incr)();
//   void (*decr)();
//   void (*push)();
//   void (*hold)();
//   uint8_t num_submenus;
//   uint8_t idx;
//   struct Menu *submenu[MAX_SUBMENUS];
//   struct Menu *parent;
// } Menu;



/*
#include "Arduino.h"
#include "menu.h"
#include "LCD_ST7032.h"
#include "menuFields.h"


/*  LCD Menu class (::menuDisplay::) based on ArduinoMenu/menuLCD.h
Rui Azevedo ruihfazevedo(@rrob@)gmail.com / www.r-site.net
Creative commons license 3.0: Attribution-ShareAlike CC BY-SA
This software is furnished "as is", without technical support, and with no
warranty, express or implied, as to its usefulness for any purpose.

Uses the LCD_ST7032 as output stream

*/

/*
class menuDisplay:public menuOut {
public:
  LCD_ST7032& lcd;
  menuDisplay(LCD_ST7032& lcd, int x=16, int y=2):lcd(lcd),menuOut(x,y) {}
  virtual void clearLine(int ln) {
    setCursor(0,ln);
    //for(int n=0;n<maxX;n++) print(' ');
    //setCursor(0,ln);
  }
  virtual void clear() {
  //  lcd.clear();
  }
  virtual void showCursor() {lcd.cursor();}
  virtual void hideCursor() {lcd.noCursor();}
  virtual void setCursor(int x,int y) {lcd.setCursor(x*resX,y*resY);}
  virtual size_t write(uint8_t ch) {return lcd.write(ch);}
  virtual void printPrompt(prompt &o,bool selected,int idx,int posY,int width) {
    clearLine(posY);
    //setCursor(0,posY);
    print(selected?(o.enabled?menu::enabledCursor:menu::disabledCursor):' ');
    o.printTo(*this);
    //print("");
  }
  virtual void printMenu(menu& m,bool drawExit) {
    if (drawn!=&m) clear();//clear screen when changing menu
    if (m.sel-top>=maxY) top=m.sel-maxY+1;//selected option outside device (bottom)
    else if (m.sel<top) top=m.sel;//selected option outside device (top)
    int i=0;for(;i<m.sz;i++) {
      if ((i>=top)&&((i-top)<maxY)) {
        if (needRedraw(m,i)) {
          printPrompt(*m.data[i],i==m.sel,i+1,i-top,m.width);
        }
      }
    }
    if (drawExit&&i-top<maxY&&needRedraw(m,i))
    printPrompt(menu::exitOption,m.sel==m.sz,0,i-top,m.width);
    lastTop=top;
    lastSel=m.sel;
    drawn=&m;
  }
};

*/
#endif
