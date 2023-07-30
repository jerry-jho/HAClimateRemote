#include "arduino.h"

Serial_t Serial;

void delay(int m) {
  
}

#ifdef ARDUINO
extern void setup();
extern void loop();
int main(void) {
  setup();
  loop();
}
#endif