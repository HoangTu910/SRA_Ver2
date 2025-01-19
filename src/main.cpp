#include <Arduino.h>
#include "setupConfiguration/SetupNumberHelper.hpp"


void setup() {
    Serial.begin(Serial::BAUD_RATE);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}