#ifndef WIFI_H
#define WIFI_H

#define WL_CONNECTED 1

class WiFi_t {
  public:
    WiFi_t() {
      _localIP = "127.0.0.1";
    }
    void begin (const char * ssid, const char * passwd) {}
    const char * localIP() {
      return _localIP;
    }
    int status() {
      return WL_CONNECTED;
    }
  private:
    const char * _localIP;
};

extern WiFi_t WiFi;



#endif