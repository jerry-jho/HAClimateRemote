#include <WiFi.h>
#include <HTTPClient.h>

#include "auth.private.h"

void setup() {

  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  delay(500);
  return;
  HTTPClient http;

  http.begin(ha_addr, ha_port, "/api/states/climate.ke_ting_kong_diao");
  http.addHeader("Authorization", token);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println(payload);
    }
  }
  else
  {
    Serial.println("[HTTP] GET... failed");
  }

  http.end();

  delay(10000);
}
