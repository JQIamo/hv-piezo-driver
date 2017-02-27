#include <SPI.h>

#include "LCD.h"

#define LCD_RST 0
#define LCD_RS  1
#define LCD_CS  2

LCD_ST7032 lcd(LCD_RST, LCD_RS, LCD_CS);

void setup(){
  SPI.begin();
  delay(10);  // necessary??

  lcd.begin();
  delay(100);

  lcd.cursor();
  lcd.print("This is LCD test");

  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("overwrite")

  delay(1000);

  lcd.clear();
  lcd.print("printf example");
  lcd.setCursor(0, 1);
  lcd.printf("%d is a number", 10);
}

void loop(){

}
