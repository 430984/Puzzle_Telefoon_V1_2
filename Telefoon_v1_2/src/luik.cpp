#include "luik.h"
#include <Arduino.h>

void Luik::begin()
{
  iStatus = 0;
  ulMil = 0;
  pinMode(iPin, OUTPUT);
}

void Luik::handle()
{
  if((iStatus != 0) && (millis() >= ulMil))
  {
    iStatus = 0;
  }
  digitalWrite(iPin, !iStatus);
}

void Luik::openLuik()
{
  iStatus = 1;
  ulMil = millis() + 2000;
}