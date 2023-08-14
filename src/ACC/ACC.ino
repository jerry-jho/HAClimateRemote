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

int current_room = 0;

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

void setup_key() {
  pinMode(PIN_KEY_SEL_0, OUTPUT);
  pinMode(PIN_KEY_SEL_1, OUTPUT);
  pinMode(PIN_KEY_SEL_2, OUTPUT);
  pinMode(PIN_KEY_VALUE, INPUT);
}

uint32_t get_key_state() {
  static uint32_t prev_value = 0x0;
  uint32_t curr_value = 0;
  for (int i=0;i<8;i++) {
    digitalWrite(PIN_KEY_SEL_0, i&1);
    digitalWrite(PIN_KEY_SEL_1, (i>>1)&1);
    digitalWrite(PIN_KEY_SEL_2, (i>>2)&1);
    delay(10);
    if (digitalRead(PIN_KEY_VALUE) == 0) {
      curr_value |= (1<<i);
    }
  }
  uint32_t r = 0;
  if (prev_value != curr_value) {
    r = curr_value;
  }
  prev_value = curr_value;
  return r;
}

int post_services(const char * service, const char * id, const char * value_name, const char * value, int ivalue) {
  HTTPClient http;
  http.begin(service);
  http.addHeader("Authorization", token);
  DynamicJsonDocument doc(128);
  doc["entity_id"] = id;
  if (value_name != nullptr) {
    if (value == nullptr) {
      doc[value_name] = ivalue;
    } else {
      doc[value_name] = value;
    }
    
  }
  char myDoc[measureJson(doc) + 1];
  serializeJson(doc, myDoc, measureJson(doc) + 1);
  int httpCode = http.POST(String(myDoc));
  USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);
  return httpCode;
}

int room_count;

void setup() {
  setup_key();
  USE_SERIAL.begin(115200);
  setup_display();
  room_count = sizeof(room_api_climate_list) / sizeof(const char *);
  wifiMulti.addAP(ssid, password);
}

void loop() {
  if((wifiMulti.run() == WL_CONNECTED)) {
    
    for (int room_id=0;room_id<room_count;room_id++) {
      g_status[room_id].valid = 0;
      HTTPClient http;
      http.begin(String(api_status_address) + room_api_climate_list[room_id]);
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
          g_status[room_id].hvac_mode = 0;
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
      http.begin(String(api_status_address) + room_api_fan_list[room_id]);
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

    int room_id = current_room;
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
    unsigned long start = millis();
    while(1) {
      uint32_t key_state = get_key_state();
      if (key_state) {
        if (key_state & KEY_MSK_ROOM) {
          current_room ++;
          if (current_room >= room_count) {
            current_room = 0;
          }
        } else if ((key_state & KEY_MSK_MODE) || (key_state & KEY_MSK_POWER)) {
          if (g_status[current_room].hvac_mode == 0) {
            // now is off
            post_services(api_turn_on_address, room_api_climate_list[current_room], nullptr, nullptr, 0);
            post_services(api_set_hvac_address, room_api_climate_list[current_room], "hvac_mode", "cool", 0);
            
          } else {
            if (key_state & KEY_MSK_POWER) {
              post_services(api_turn_off_address, room_api_climate_list[current_room], nullptr, nullptr, 0);
            } else if (g_status[current_room].hvac_mode == 2) {
              post_services(api_set_hvac_address, room_api_climate_list[current_room], "hvac_mode", "heat", 0);
            } else {
              post_services(api_set_hvac_address, room_api_climate_list[current_room], "hvac_mode", "cool", 0);
            }
          }
        } else if ((key_state & KEY_MSK_UP) || (key_state & KEY_MSK_DOWN)) {
          int temp;
          if ((key_state & KEY_MSK_UP)) temp = g_status[current_room].set_temp + 1;
          else temp = g_status[current_room].set_temp - 1;
          post_services(api_set_temperature_address, room_api_climate_list[current_room], "temperature", nullptr, temp);
        }
        break;
      }
      if (millis() - start > REFRESH_INTERVAL) {
        break;
      }
    }
    
    //Serial.printf("key_state = %x\n", key_state);
  } else {
    delay(REFRESH_INTERVAL);
  }
}
