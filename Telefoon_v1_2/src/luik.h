#ifndef LUIK_H
#define LUIK_H
#include <Arduino.h>

class Luik
{
private:
  int iPin;
  int iStatus;
  unsigned long ulMil;
public:
  Luik(int iPin)
  { 
    this->iPin = iPin;
  };
  void begin();
  
  void openLuik();

  void handle();
  
};


#endif
