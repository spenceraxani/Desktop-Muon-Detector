#include <Wire.h>
#include <OzOLED.h>


void setup(){

  OzOled.init();  //initialze Oscar OLED display
  OzOled.printString("Hello World!"); //Print the String

}

void loop(){
  
}

