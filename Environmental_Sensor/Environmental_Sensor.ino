#include <LiquidCrystal.h>
#include <WiFi.h>
#include "time.h"

LiquidCrystal lcd(27,26,33,32,19,23);           // Assign lcd pins

char line1[21],line2[21],line3[21],line4[21];  //Initialize line data buffers

//set network parameters
const char* ssid       = "BrightMindsBoyz[2GHz]";
const char* password   = "D@phn313B0yz";

//Set time parameters
int tmz = 1;           // Set timezone
const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = tmz*3600;            //timezone setting
const int   daylightOffset_sec = 3600;

void setup()
{
    //Format the LCD buffers with blank spaces
    sprintf(line1,"%s","                    ");
    sprintf(line2,"%s","                    ");
    sprintf(line3,"%s","                    ");
    sprintf(line4,"%s","                    ");

    lcd.begin(20, 4);   //Initialize screen as a 20x4
    lcd.clear();
    //clear off the lcd screen spaces
    lcd.setCursor(0,1); 
    lcd.print(line1);
    lcd.setCursor(1,1); 
    lcd.print(line2);
    lcd.setCursor(2,1); 
    lcd.print(line3);
    lcd.setCursor(3,1); 
    lcd.print(line4);

    //connect to WiFi
    lcd.setCursor(0,1);
    lcd.print(" Connecting to WIFI ");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  lcd.setCursor(0,1);
  lcd.print("     CONNECTED      ");
  delay(5000);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
void loop(){
  delay(1000);
  printLocalTime();
  }

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    
   sprintf(line2,"%s","Time Update Failed! ");
   lcd.setCursor(0,1);
   lcd.print(line2);
    return;
  }
  lcd.setCursor(0,0);
  lcd.print(&timeinfo, "      %H:%M:%S      ");//
  lcd.setCursor(0,1);
  lcd.print(&timeinfo, "      %A        ");
  lcd.setCursor(0,2);
  lcd.print(&timeinfo, " %B %d %Y");
 
  
}
