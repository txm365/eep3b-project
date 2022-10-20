#include <stdbool.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "time.h"
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

//Define LCD Pins
#define RS 27
#define EN 26
#define D4 25
#define D5 33
#define D6 32
#define D7 19
#define fan 4
LiquidCrystal lcd(RS,EN,D4,D5,D6,D7);           // Assign lcd pins

//interrupts
#define fanInt 5
#define wifiInt 23
#define modeInt 18

char line1[21],line2[21],line3[21],line4[21];  //Initialize line data buffers
char sysTime[9];
char sysDate[20];
char temps[10];
char humid[10];
char pres[10];
char fanStatus[4];
bool fan_Status;

float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

//Set time parameters
int tmz = 1;           // Set timezone
const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = tmz*3600;            //timezone setting
const int   daylightOffset_sec = 3600;
bool res;
bool timeConfigured = false;
unsigned long lasttick= 0;
int strsize = 0;
int pos = 0;
bool wifiSetup = false;
int timeout = 60;
float thresholdTemp = 30.0;
bool dispTime  = true;
// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";

//Publishing Topics
const char *tmp_tpc = "txm365/esp32/tmp";
const char *hum_tpc = "txm365/esp32/hum";
const char *prs_tpc = "txm365/esp32/prs";
const char *timer_tpc = "txm365/esp32/time";
const char *fan_ctrl_tpc_dev_pub = "txm365/esp32/fan_ctrl_pub";

//Subscription topics
const char *fan_ctrl_tpc_dev_sub = "txm365/esp32/fan_ctrl_sub";

const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

unsigned long button_time = 0;  
unsigned long last_button_time = 0;
String tmp = "";
bool autoFan = false;
WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR setWIFI() {
 button_time = millis();
  
  if (button_time - last_button_time > 500)
  {
      last_button_time = millis();
      wifiSetup = true; 
  }
}

void IRAM_ATTR changeDispMode() {
   button_time = millis();
  
  if (button_time - last_button_time > 500){
    last_button_time = millis();
    if (dispTime == true){
      dispTime = false;
    }
    else  if (dispTime == false){
      dispTime = true;
    }
        
  }
}

void IRAM_ATTR cntlFan() {
   button_time = millis();
  
  if (button_time - last_button_time > 500){
    last_button_time = millis();
    if(fan_Status){
     // ets_printf("Fan: OFF\n");
        digitalWrite(fan, LOW);
        fan_Status = false;
        //client.publish(fan_ctrl_tpc_dev_pub, "0");
        sprintf(fanStatus,"%3s", "OFF");
    }

     else if(fan_Status==false){
        //ets_printf("Fan: ON\n");
        digitalWrite(fan, HIGH);
        fan_Status = true;
       //client.publish(fan_ctrl_tpc_dev_pub,"1");
        sprintf(fanStatus,"%3s", "ON");
      }     
    }
}

void setup(){
  Serial.begin(115200);
  
  pinMode(fan, OUTPUT); // set the pin as output
  digitalWrite(fan, LOW);
  fan_Status  = false;

  pinMode(fanInt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fanInt), cntlFan, CHANGE);
  pinMode(wifiInt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(wifiInt), setWIFI, CHANGE);
  pinMode(modeInt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(modeInt), changeDispMode, CHANGE);
  
    //Format the LCD buffers with blank spaces
  for(int i = 0; i<4; i++){
      lcd.setCursor(0,i);
      lcd.print("                    ");
   }
     
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
      delay(1000);
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
        
      
      res = wm.autoConnect("ESP_WIFI_SETUP"); 
      if(!res) {
            Serial.println("Failed to connect");
            lcd.setCursor(0,1);
            lcd.print("  Failed to connect ");
            delay(5000);
            ESP.restart();
        }  
      
   }
         strsize = sizeof(WiFi.SSID());
         pos = floor(abs((20 - (strsize-1)))/2);
        lcd.setCursor(0,0);
        lcd.print("    Connected to    ");
        lcd.setCursor(0,1);
        lcd.print("                    ");
        lcd.setCursor(pos,1);
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,2);
        lcd.print(" Network IP ADDRESS ");
        lcd.setCursor(0,3);
        lcd.print("                    ");
        lcd.setCursor(2,3);
        lcd.print(WiFi.localIP());
        
        delay(5000);
        
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
         timeConfigured = true;
         
        lcd.setCursor(0,0);
          lcd.print("   Checking Sensor  ");
          lcd.setCursor(0,3);
          lcd.print("                    ");
        if (! bme.begin(0x76, &Wire)) {
          
          lcd.setCursor(0,1);
          lcd.print("                    ");
         
          lcd.setCursor(0,2);
          lcd.print("BME280 SENSOR Failed");
          
        }
        else{
          lcd.setCursor(0,1);
          lcd.print("                    ");
         
          lcd.setCursor(0,2);
          lcd.print("BME280 SENSOR Ready");
           delay(1000);
          displaySensor();
          
        }
        delay(1000);
         

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
       // subscribe and listen to fan control commands
       client.subscribe(fan_ctrl_tpc_dev_sub);


        
}
      
void callback(char *topic, byte *payload, unsigned int length) {
     
       if(payload[0] =='0'){
        Serial.println("Fan: OFF");
        digitalWrite(fan, LOW);
        fan_Status = false;
        client.publish(fan_ctrl_tpc_dev_pub, "0");
        sprintf(fanStatus,"%3s", "OFF");
       }
       else if(payload[0] =='1'){
        Serial.println("Fan: ON");
        digitalWrite(fan, HIGH);
        fan_Status = true;
        client.publish(fan_ctrl_tpc_dev_pub,"1");
        sprintf(fanStatus,"%3s", "ON");
       }
      
}

void loop(){
  client.loop();
  struct tm timeinfo;
  if (millis() - lasttick > 1000){
    lasttick = millis();

    
    if(!getLocalTime(&timeinfo)){
      lcd.clear();
     sprintf(line2,"%s","Time Update Failed! ");
     lcd.setCursor(0,1);
     lcd.print(line2);
      return;
  }
    
    bme.takeForcedMeasurement(); 
    if(bme.readTemperature() > thresholdTemp && fan_Status == false){
      //turn fan
      autoFan == true;
      digitalWrite(fan, HIGH);
      fan_Status = true;
        sprintf(fanStatus,"%3s", "ON");
    }

    else if (bme.readTemperature() < thresholdTemp && autoFan == true){
      autoFan == false;
       digitalWrite(fan, LOW);
       fan_Status = false;
        sprintf(fanStatus,"%3s", "OFF");
    }
     if (dispTime ==false) {
     
      displaySensor();
     }
     else if(dispTime == true){
      printLocalTime();
     }
     sprintf(sysTime,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec);
     
     client.publish(timer_tpc, sysTime);


          pressure = bme.readPressure()/ 100.0F;
          delay(50);
          humidity = bme.readHumidity();
           delay(50);
          temperature = bme.readTemperature();
           delay(50);

          dtostrf(pressure, 5, 1, pres);
          dtostrf(humidity, 2, 2, humid);
          dtostrf(temperature, 2, 2, temps);
          
     client.publish(tmp_tpc, temps);
      client.publish(prs_tpc, pres);
     client.publish(hum_tpc, humid);
    
  }

  if(wifiSetup == true){
    
     WiFiManager wm;
    Serial.println("\n Starting");
     lcd.setCursor(0,0);
        lcd.print("  Connect to wifi:  ");
        lcd.setCursor(0,1);
        lcd.print("   ESP_WIFI_SETUP   ");
        lcd.setCursor(0,2);
        lcd.print("On your phone, then ");
        lcd.setCursor(0,3);
        lcd.print(" http://192.168.4.1 "); 
     wm.setConfigPortalTimeout(timeout);
     
     if (!wm.startConfigPortal("ESP_WIFI_SETUP")) {
      Serial.println("failed to connect and hit timeout");
      lcd.setCursor(0,0);
        lcd.print("                    ");
        lcd.setCursor(0,1);
        lcd.print(" FAILED to Connect! ");
        lcd.setCursor(0,2);
        lcd.print("  PORTAL TIMED OUT! ");
        lcd.setCursor(0,3);
        lcd.print("                    "); 
        delay(15000);
      
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
    wifiSetup = false;
    delay(1000);
    strsize = sizeof(WiFi.SSID());
         pos = floor(abs((20 - (strsize-1)))/2);
         delay(1000);
        lcd.setCursor(0,0);
        lcd.print("    Connected to    ");
        lcd.setCursor(0,1);
        lcd.print("                    ");
        lcd.setCursor(pos,1);
        lcd.print(WiFi.SSID());
        lcd.setCursor(0,2);
        lcd.print(" Network IP ADDRESS ");
        lcd.setCursor(0,3);
        lcd.print("                    ");
        lcd.setCursor(2,3);
        lcd.print(WiFi.localIP());
        
        delay(5000);
    
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
  lcd.print("   FAN status: ");
  lcd.setCursor(15,0);
  lcd.print(fanStatus);
  lcd.setCursor(0,1);
  lcd.print(&timeinfo, "      %H:%M:%S      ");//
  lcd.setCursor(0,2);
  lcd.print(&timeinfo, "      %A        ");
  lcd.setCursor(0,3);
  lcd.print(&timeinfo, "  %d %B %Y ");
  
 sprintf(sysTime,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec);
 
  
}

void displaySensor(){ 
           
        pressure = bme.readPressure()/ 100.0F;
        humidity = bme.readHumidity();
        temperature = bme.readTemperature();
                       
        lcd.setCursor(0,0);
        lcd.print("   FAN status: ");
        lcd.setCursor(15,0);
        lcd.print(fanStatus);
        lcd.setCursor(0,1);
        lcd.print("Temperature: ");
        Serial.println(temps);
        lcd.setCursor(13,1);
        lcd.print(temperature);
        lcd.setCursor(0,2);
         lcd.print("                    ");
         lcd.setCursor(0,2);
        lcd.print("Humidity: ");
        lcd.setCursor(10,2);
        lcd.print(humidity);
        Serial.println(humid);
        lcd.setCursor(0,3);
        lcd.print("Pressure: "); 
        lcd.setCursor(10,3);
        lcd.print(pressure);
        
          
}
