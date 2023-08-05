#ifndef _MP2SHELL_H_
#define _MP2SHELL_H_
#include <arduino.h>

class MP2Shell {

public:
  MP2Shell();
  void begin(int latchPin = D4, int clockPin =D7, int dataPin = D8);
  void setDigitalValue(int value, int start=0);


  int _latchPin;
  int _clockPin;
  int _dataPin;

};

#endif
