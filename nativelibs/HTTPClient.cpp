#include "HTTPClient.h"
#include <iostream>

HTTPClient::HTTPClient() {
  _clinet = nullptr;
}


void HTTPClient::begin(String host, uint16_t port, String uri) {
  if (_clinet) {
    delete _clinet;
  }
  if (host.substr(0, 7) == String("http://")) {
    host = host.substr(7);
  }
  _clinet = new httplib::Client(host, port);
  _uri = uri;
}

int HTTPClient::GET() {
  auto res = _clinet->Get(_uri, _headers);
  if (res) {
    _body = res->body;
    return res->status;
  } else {
    std::cerr<<res.error()<<std::endl;
    return -1;
  }
}

void HTTPClient::addHeader(const String& name, const String& value, bool first, bool replace) {
  _headers.insert(std::make_pair(name, value));
}

String HTTPClient::errorToString(int code) {
  return String(httplib::status_message(code));
}

String HTTPClient::getString() {
  return _body;
}

void HTTPClient::end() {

}