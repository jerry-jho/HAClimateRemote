#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "auth.private.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#define USE_SERIAL Serial

WiFiMulti wifiMulti;

typedef struct {
  int current_room;  
} config_t;

typedef struct {
  int valid;
  int sensor_temp;
  int set_temp;
  int hvac_mode; // off - 0 heat - 1 cool - 2
  int fan_speed; // auto - 0 low = 1, mid = 2, high = 3
} status_t;

const char * hvac_names[]= {"off","heat","cool"};

#define MAX_ROOM_COUNT 10
status_t g_status[MAX_ROOM_COUNT];

config_t load_config() {
  config_t t;
  t.current_room = 0;
  return t;
}

Adafruit_SSD1306 display(128, 32, &Wire, -1);

void setup_display() {
  Wire.begin(5,4);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setRotation(2);
  display.clearDisplay();
  display.setTextSize(2); // 10x2 in size 2
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("PLM AC CTL");
  display.display();
  delay(1000);
}

void setup() {

    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();
    setup_display();
    /*
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }*/
    
    wifiMulti.addAP(ssid, password);

}



void loop() {
  if((wifiMulti.run() == WL_CONNECTED)) {
    int room_count = sizeof(room_api_climate_list) / sizeof(const char *);
    for (int room_id=0;room_id<room_count;room_id++) {
      g_status[room_id].valid = 0;
      HTTPClient http;
      http.begin(room_api_climate_list[room_id]);
      http.addHeader("Authorization", token);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.GET();
      USE_SERIAL.printf("Room %d\n", room_id);
      if(httpCode > 0) {
        USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          USE_SERIAL.println(payload);
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);
          g_status[room_id].valid = 1;
          g_status[room_id].set_temp = doc["attributes"]["temperature"];
          g_status[room_id].sensor_temp = doc["attributes"]["current_temperature"];
          String state = doc["state"];
          for (int m=0;m<3;m++) {
            if (state == String(hvac_names[m])) {
              g_status[room_id].hvac_mode = m;
              break;
            }
          }
        } else {
          USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
      }
      http.end();
    }  
    for (int room_id=0;room_id<room_count;room_id++) {
      g_status[room_id].fan_speed = 0;
      HTTPClient http;
      http.begin(room_api_fan_list[room_id]);
      http.addHeader("Authorization", token);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.GET();
      if(httpCode > 0) {
        USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          USE_SERIAL.println(payload);
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);
          g_status[room_id].valid = 1;
        } else {
          USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
      }
      http.end();
    } 
    config_t c = load_config();

    int room_id = c.current_room;
    
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    //0 1 2 3 4 5 6 7 8 9
    //# R   S S   T T   M  
    display.setCursor(0, 0);
    display.printf("#%d %02d %02d %c",
      room_id,
      g_status[room_id].hvac_mode == 0 ? 0 : g_status[room_id].set_temp,
      g_status[room_id].sensor_temp,
      g_status[room_id].hvac_mode == 0 ? 'X' : g_status[room_id].hvac_mode == 2 ? 'C' : 'H');

    display.display();
  }
  delay(10000);
}
