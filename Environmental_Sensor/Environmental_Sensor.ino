#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "time.h"
#include <PubSubClient.h>

#define RS 27
#define EN 26
#define D4 33
#define D5 32
#define D6 19
#define D7 25

LiquidCrystal lcd(RS,EN,D4,D5,D6,D7);           // Assign lcd pins

char line1[21],line2[21],line3[21],line4[21];  //Initialize line data buffers
char times[9];

//Set time parameters
int tmz = 1;           // Set timezone
const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = tmz*3600;            //timezone setting
const int   daylightOffset_sec = 3600;
bool res;
bool timeConfigured = false;
unsigned long lasttick= 0;

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "txm365/esp32/temp";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(9600);
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

      lcd.setCursor(0,1);
      lcd.print(" Connecting to WIFI ");
      delay(2000);
    //connect to WiFi
    
 
    while ((WiFi.status() != WL_CONNECTED)){
      WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
      WiFiManager wm;
      
   
        lcd.setCursor(0,0);
        lcd.print("  Connect to wifi:  ");
        lcd.setCursor(0,1);
        lcd.print("   ESP_WIFI_SETUP   ");
        lcd.setCursor(0,2);
        lcd.print("On your phone, then ");
        lcd.setCursor(0,3);
        lcd.print(" http://192.168.4.1 "); 
        
      
      res = wm.autoConnect("ESP_WIFI_SETUP","setup_password"); // password protected ap
      if(!res) {
            Serial.println("Failed to connect");
            lcd.setCursor(0,1);
            lcd.print("  Failed to connect ");
            delay(15000);
            ESP.restart();
        }  
      
   }
         
        lcd.setCursor(0,0);
        lcd.print("    Connected to    ");
        lcd.setCursor(0,1);
        lcd.print("                    ");
        lcd.setCursor(0,1);
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,2);
        lcd.print(" Network IP ADDRESS ");
        lcd.setCursor(0,3);
        lcd.print("                    ");
        lcd.setCursor(2,3);
        lcd.print(WiFi.localIP());
        
        delay(10000);
        
         configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
         timeConfigured = true;

         //connecting to a mqtt broker
       client.setServer(mqtt_broker, mqtt_port);
       client.setCallback(callback);
       while (!client.connected()) {
           String client_id = "esp32-client-txm365";
           client_id += String(WiFi.macAddress());
           Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
           if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
               Serial.println("Public emqx mqtt broker connected");
           } else {
               Serial.print("failed with state ");
               Serial.print(client.state());
               delay(2000);
           }
       }
       // publish and subscribe
       client.publish(topic, "Hi EMQX I'm ESP32 ^^");
       client.subscribe(topic);
      }
      
      void callback(char *topic, byte *payload, unsigned int length) {
       Serial.print("Message arrived in topic: ");
       Serial.println(topic);
       Serial.print("Message:");
       for (int i = 0; i < length; i++) {
           Serial.print((char) payload[i]);
       }
         
}

void loop(){
  client.loop();
  
  if (millis() - lasttick > 1000){
    lasttick = millis();
    if(timeConfigured){
      printLocalTime();
    }
    else {
     lcd.setCursor(0,0);
        lcd.print("                    ");
        lcd.setCursor(0,1);
        lcd.print("                    ");
         lcd.setCursor(0,1);
         lcd.print("   Running without  ");
         lcd.setCursor(0,2);
         lcd.print(" updated Server time ");
         lcd.setCursor(0,3);
         lcd.print("                    ");  
     }
  }
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
  lcd.print("                    ");
 Serial.println(&timeinfo, "      %H:%M:%S      ");//
 sprintf(times,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec);
 client.publish(topic, times);
  
}
