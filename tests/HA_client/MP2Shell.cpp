#include "MP2Shell.h"

MP2Shell::MP2Shell() {

}

int g_value;
int g_start;

unsigned char Dis_table[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90,0xFF};
unsigned char Dis_buf[]   = {0xF1,0xF2,0xF4,0xF8};
int disp_value;
void display(void * pvParameters) {
  Serial.println("thread started");
  MP2Shell * shell = (MP2Shell * )pvParameters;
  while(1) {
    int x = g_value;
    int v[4];
    for(char i=0; i<=3; i++) {
      v[i] = 10;
    }
      //Serial.print("g_start=");
      //Serial.println(g_start);
      //Serial.print("g_value=");
      //Serial.println(g_value);
    if (g_start >= 0) {

      if (x >= 0 && x < 10) {
        v[g_start] = x;
      } else if (x >= 10 && x < 100 && g_start < 3) {
        v[g_start] = x / 10;
        v[g_start + 1] = x % 10;
      } else if (x >= 100 && x < 1000 && g_start < 2) {
        v[g_start] = x / 100;
        v[g_start + 1] = (x / 10) % 10;
        v[g_start + 3] = (x / 100) % 10;     
      } else {
        for(char i=0; i<=3; i++) {
          v[3-i] = x % 10;
          x = x / 10;
        }
      }
    }


    for(char i=0; i<=3; i++) {
      digitalWrite(shell->_latchPin,LOW);
      shiftOut(shell->_dataPin,shell->_clockPin,MSBFIRST,Dis_table[v[i]]);
      shiftOut(shell->_dataPin,shell->_clockPin,MSBFIRST,Dis_buf[i] );
      digitalWrite(shell->_latchPin,HIGH);
      delay(1);
    }
    //delay(500);
  }
}

void MP2Shell::begin(int latchPin, int clockPin, int dataPin) {
  this->_latchPin = latchPin;
  this->_clockPin = clockPin;
  this->_dataPin = dataPin;
  g_start = -1;
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin,OUTPUT);
  xTaskCreatePinnedToCore(
                display,
                "TaskIOFunc",
                100000,
                this,
                1,
                NULL,
                0); 
}

void MP2Shell::setDigitalValue(int value, int start) {
  g_value = value;
  g_start = start;
}
