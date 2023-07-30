#ifndef ARDUINO_H
#define ARDUINO_H

#include <string>
#include <iostream>

#ifdef CUSTOM_STRING
class String {
  public:
    String() {}

    String(std::string s) : _s(s) {

    }

    String(const char * s) : _s(s) {

    }

    String(String & other) {
      this->_s = other._s;
    }

    String(const String & other) {
      this->_s = other._s;
    }

    String & operator = (const String & other) {
      this->_s = other._s;
      return *this;
    }

    String & operator = (char * s) {
      this->_s = s;
      return *this;
    }

    operator std::string() {
      return _s;
    }

    operator const std::string() {
      return _s;
    }

    operator const char *() {
      return _s.c_str();
    }

    const char * c_str() {
      return _s.c_str();
    }

  private:
    std::string _s;
};
#else
typedef std::string String;
#endif

class Serial_t {
  public:
    Serial_t() {

    }
    void begin(int baudrate) {

    }
    void println (String s) {
      println(s.c_str());
    }
    void println (const char * s = nullptr) {
      if (s) {
        print(s);
      }
      print("\n");
    }
    void print(const char * s) {
      std::cout<<s;
    }
    void printf(const char * s, ...) {

    }
};

void delay(int m);

extern Serial_t Serial;

#endif
