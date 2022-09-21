#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "time.h"

LiquidCrystal lcd(27,26,33,32,19,23);           // Assign lcd pins

char line1[21],line2[21],line3[21],line4[21];  //Initialize line data buffers
char local_net_ssid[21];                       //Network Parameters

//Set time parameters
int tmz = 1;           // Set timezone
const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = tmz*3600;            //timezone setting
const int   daylightOffset_sec = 3600;
bool res;

String myssid;
String myIP;
String myRSSI;
void setup()
{
    //Format the LCD buffers with blank spaces
    sprintf(line1,"%s","                    ");
    sprintf(line2,"%s","                    ");
    sprintf(line3,"%s","                    ");
    sprintf(line4,"%s","                    ");

    sprintf(local_net_ssid,"%s","                    "); 
     
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
      lcd.print("Connecting to  WIFI ");
      WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
      WiFiManager wm; 
    while (WiFi.status() != WL_CONNECTED) {
      
//      delay(2000);
        lcd.setCursor(0,0);
        lcd.print("previously connected");
        lcd.setCursor(0,1);
        lcd.print("wifi connot be found");
        lcd.setCursor(0,2);
        lcd.print("Change wifi details:");
        lcd.setCursor(0,3);
        lcd.print(" http://192.168.4.1 "); 
       
        res = wm.autoConnect("ESP_WIFI_SETUP","setup_password"); // password protected ap
    
        if(!res) {
            Serial.println("Failed to connect");
            ESP.restart();
         }
         else{
          delay(2000);

       
        
        lcd.setCursor(0,0);
        lcd.print("    Connected to    ");
        lcd.setCursor(0,1);
         sprintf(local_net_ssid,"WIFI: %-14s",WiFi.SSID());
         lcd.print(local_net_ssid);
         
         lcd.setCursor(2,2);
         lcd.print("                    ");
        lcd.setCursor(2,2);
        lcd.print(WiFi.localIP());
        
        lcd.setCursor(0,3);
        lcd.print("                    ");
            delay(5000);
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
            printLocalTime();
         }
     } 
}

void loop(){
  delay(1000);
  printLocalTime();
  }

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    lcd.clear();
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
  lcd.print(&timeinfo, "  %d %B %Y ");
  lcd.setCursor(0,3);
  lcd.print(&timeinfo, "                    ");
 
  
}
