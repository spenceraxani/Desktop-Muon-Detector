#include <Wire.h>
#include <OzOLED.h>


void setup(){

  OzOled.init();  //initialze OLED display

  OzOled.clearDisplay();           //clear the screen and set start position to top left corner
  OzOled.setNormalDisplay();       //Set display to Normal mode
  OzOled.setHorizontalMode();      //Set addressing mode to Horizontal Mode
  OzOled.printString("!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");  //Print String (ASCII 32 - 126 )

}

void loop(){
  
}


