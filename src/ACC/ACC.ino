#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "hardware.h"
#include "characters.h"

#include "auth.private.h"

#define USE_SERIAL Serial

WiFiMulti wifiMulti;

typedef struct {
  int current_room;  
} config_t;

typedef struct {
  int valid;
  int fan_valid;
  int sensor_temp;
  int set_temp;
  int hvac_mode; // off - 0 heat - 1 cool - 2
  int fan_speed; // auto - 0 low = 1, mid = 2, high = 3
} status_t;

const char * hvac_names[]= {"off","heat","cool"};
const char * speed_names[]= {"AUTO","]]","]]]]","]]]]]]"};
#define MAX_ROOM_COUNT 10
status_t g_status[MAX_ROOM_COUNT];

config_t load_config() {
  config_t t;
  t.current_room = 0;
  return t;
}

Adafruit_SSD1306 display(128, 32, &Wire, -1);

void setup_display() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setRotation(OLED_ROTATION);
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
  setup_display(); 
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
        //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          //USE_SERIAL.println(payload);
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
          //USE_SERIAL.println(payload);
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);
          g_status[room_id].fan_valid = 1;
          
          JsonArray array = doc["attributes"]["options"].as<JsonArray>();
          g_status[room_id].fan_speed = 0;
          String fan_state = doc["state"];
          //USE_SERIAL.printf("fan_state %s\n",fan_state);
          for(JsonVariant v : array) {
            //USE_SERIAL.printf("fan_opt %s\n",v.as<String>());
            if (v.as<String>() == fan_state) break;
            g_status[room_id].fan_speed ++;
          }
        } else {
          USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
      }
      http.end();
    }

    config_t c = load_config();

    int room_id = c.current_room;
    if (g_status[room_id].valid && g_status[room_id].fan_valid) {
      display.clearDisplay();
      display.drawBitmap(0,0,NAME_GET(room_id),32,16,1);
      display.drawBitmap(32,0,bitmap_shice,32,16,1);
      display.drawBitmap(94,0,bitmap_du,16,16,1);
      if (g_status[room_id].hvac_mode == 1) {
        display.drawBitmap(112,0,bitmap_re,16,16,1);
      } else if (g_status[room_id].hvac_mode == 2) {
        display.drawBitmap(112,0,bitmap_leng,16,16,1);
      } else {
        display.drawBitmap(112,0,bitmap_guan,16,16,1);
      }
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(68, 1);
      display.printf("%02d", g_status[room_id].sensor_temp);
      if (g_status[room_id].hvac_mode != 0) {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 25);
        display.printf("SET %02d", g_status[room_id].set_temp);
        if (g_status[room_id].fan_speed < 4) {
          display.setCursor(50, 25);
          display.printf("FAN %s", speed_names[g_status[room_id].fan_speed]);
        }     
      }
      display.display();
    }
  }
  delay(2000);
}
