#include <Wire.h>
#include <OzOLED.h>


void setup(){

  OzOled.init();  //initialze OLED display

  OzOled.clearDisplay();          //clear the screen and set start position to top left corner
  OzOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
  OzOled.setPageMode();           //Set addressing mode to Page Mode
  OzOled.printString("Hello World!", 0, 0); //Print the String

}

void loop(){
  
}

