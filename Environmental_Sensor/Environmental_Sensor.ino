#include <LiquidCrystal.h>
LiquidCrystal lcd(27,26,33,32,19,23);
void setup()
{
 
    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("How to Interface");
    

    // go to row 1 column 0, note that this is indexed at 0
    lcd.setCursor(0,1); 
    lcd.print ("LCD with ESP32");
}
void loop(){
  
  }
