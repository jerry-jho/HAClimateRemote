#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "********";
const char *password = "********";

void setup()
{
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
  HTTPClient http;

  http.begin("http://localhost", 8080, "/hi.html");

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
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); // 结束当前连接

  delay(10000);
}

