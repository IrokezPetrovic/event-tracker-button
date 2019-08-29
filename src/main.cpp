#include <Arduino.h>
#include "device_config.hpp"
#include "prod.hpp"
#include "maint.hpp"


bool productionMode = true;
typedef void(*loopfn)();
loopfn loop_function;
void setup(){  
  pinMode(BTN_MAINT, INPUT_PULLUP);
  productionMode = digitalRead(BTN_MAINT);  
  delay(10);
  if (productionMode){
    setup_prod();
    loop_function = loop_prod;
  } else {
    setup_maint();
    loop_function = loop_maint;
  }
}


void loop(){
  loop_function();
}


