#include "HTTPClient.h"


HTTPClient::HTTPClient() {
  _clinet = nullptr;
}


void HTTPClient::begin(String host, uint16_t port, String uri) {
  if (_clinet) {
    delete _clinet;
  }
  _clinet = new httplib::Client(host, port);
  _uri = uri;
}

int HTTPClient::GET() {
  auto res = _clinet->Get(_uri);
  if (res) {
    _body = res->body;
  }
  return res->status;
}

String HTTPClient::errorToString(int code) {
  return String(httplib::status_message(code));
}

String HTTPClient::getString() {
  return _body;
}

void HTTPClient::end() {

}