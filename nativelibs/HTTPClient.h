#ifndef HTTP_CLIENT
#define HTTP_CLIENT

#include <httplib.h>
#include <cstdint>
#include <string>
#include "arduino.h"

class HTTPClient {
  public:
    HTTPClient();

    void begin(String host, uint16_t port = 80, String uri = "/");
    int GET();
    String getString();

    void end();

    String errorToString(int code);

  private:
    httplib::Client * _clinet;
    String _uri;
    String _body;
};

#define HTTP_CODE_OK 200

#endif //HTTP_CLIENT