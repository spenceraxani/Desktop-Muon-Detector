/*
 
Desktop Muon Counter Arduino Code
Questions?
Spencer N. Axani 
saxani@mit.edu

Requirements: Sketch->Include->Manage Libraries:
  1. Adafruit GFX Library -- by Adafruit Version 1.0.2
  2. Adafruit SSD1306     -- by Adafruit Version 1.0.0
  3. Wire                 -- by Arduino Version 1.0.0
  4. OzOLED               -- Version unknown
  5. SPI                  -- by Arduino Version 1.0.0 
  6. TimerOne             -- by Jesse Tane et al. Version 1.1.0

A tutorial on how interrupts are used can be found here:
http://www.instructables.com/id/Arduino-Timer-Interrupts/step2/Structuring-Timer-Interrupts/
http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/

*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <OzOLED.h>
#include <SPI.h>
#include <TimerOne.h>  

const int OLED = 1;               // Turn on/off the LED [1,0]

//INTERUPT SETUP
#define TIMER_INTERVAL 1000000     // Every 1,000,000 us the timer will update the clock on the screen


//OLED SETUP
#define OLED_RESET 10              // A signal to this pin will reset the OLED screen
Adafruit_SSD1306 display(OLED_RESET);

//Linear regression parameters
int x1,x2,x3,x4,x5,x6,x7,x8,x9;
float y1,y2,y3,y4,y5,y6,y7,y8,y9;
float Sx2Sy, SxSxy, nSx2, Sx2;
float b;

//Timing variables
long initial  = 0L;                // Time stamp
long final    = 0L;                // Used to calculate deadtime of signal measurement
long t1       = 0L;                      // Used to calcualte deadtime of OLED screen update
long t2       = 0L;                      // Used to calcualte deadtime of OLED screen update
long timeOLED = 0L;                // Tallies the total deadtime of the OLED screen between signal measurements
long timePulse = 0L;
long totalDeadtime = 0L;

String      hist = "Initiallizing...";  // A histogram representing the relative number of photons seen by the SiPM
const float signalThreshold  = 35;      // Signal threshold, measured from 0-1023. Measureing from 0 to 300mV. 70 corresponds to approximately 20 mV threshold. 
const int   led              = 3;       // The LED digital pin connections. 
long int    count            = -1L;      // A tally of the number of muon counts observed
const float timeInterval     = 5.8;     //Using a prescaler of 4, the time between analogue samples is 5.8 us.          


float pulseV = 0;                  // This is the calculated pulse amplitude of the SiPM signal
float ampV   = 0;                        // This is the measured pulse amplitude from the pulse detector circuit

const String currentcountstr  = "Total Count:";
const String runtimestr       = "Run Time:   ";
const String ratestr          = "Rate: ";
const String pm               = "+/-";

long int time           = 0.;
int      seconds        = 0.;
int      minutes        = 0.;
int      hours          = 0.;
float    stdev          = 0.;
float    average        = 0.;
char tmp2[15];
char tmp[15];
String currentcount;
String runtime;
String rate;

void setup() {
Serial.begin(9600);

Serial.println("###                  Desktop Muon Detector data logging                 ###");
Serial.println("###                       Questions? saxani@mit.edu                     ###");
Serial.println("### CompTime Counts ArduinoTime[ms] Amplitude[V] Pulse[mV] DeadTime[ms] ###");

ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2));    // clear prescaler bits
ADCSRA |= bit (ADPS1);                                   // Set prescaler to 4  

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);     // OLED screen

//display.clearDisplay();                        // OLED screen
display.setTextSize(1);
display.setTextColor(WHITE);

pinMode(led, OUTPUT);                          // Setup the LED pin to output

Timer1.initialize(TIMER_INTERVAL);             // Initialise timer 1
Timer1.attachInterrupt(timerIsr);              // attach the ISR routine

getTime();
}

void loop() {
  while (1) {
    if (analogRead(A0) > signalThreshold) 
        {
        y2 = analogRead(A0);
        y3 = analogRead(A0);
        y4 = analogRead(A0);
        y5 = analogRead(A0);
        y6 = analogRead(A0);
        y7 = analogRead(A0);
        y8 = analogRead(A0);
        y9 = analogRead(A0);  
        initial = millis();               // Time stamp

        noInterrupts();                   // Turn of the interupts, so that it doesn't update the OLED screen mid measuremnt
        x1 = 0;                           // This is the time of the first measurement: y1
        x2 = x1 + timeInterval + 0.5;     // It takes 504ns to perform the IF statement
        x3 = x2 + timeInterval;
        x4 = x3 + timeInterval;
        x5 = x4 + timeInterval;
        x6 = x5 + timeInterval;
        x7 = x6 + timeInterval;
        x8 = x7 + timeInterval;
        x9 = x8 + timeInterval;
        digitalWrite(led, HIGH);
        // Calculate the initial pulse amplitude. This is just a linear regression fit.
        nSx2 = 8 * (x2*x2+x3*x3+x4*x4+x5*x5+x6*x6+x7*x7+x8*x8+x9*x9);
        Sx2 = (x2+x3+x4+x5+x6+x7+x8+x9)* (x2+x3+x4+x5+x6+x7+x8+x9);
        Sx2Sy = (x2*x2+x3*x3+x4*x4+x5*x5+x6*x6+x7*x7+x8*x8+x9*x9) * (y2+y3+y4+y5+y6+y7+y8+y9);
        SxSxy = (x2+x3+x4+x5+x6+x7+x8+x9)*(x2*y2+x3*y3+x4*y4+x5*y5+x6*y6+x7*y7+x8*y8+x9*y9);
        b = (Sx2Sy - SxSxy)/(nSx2-Sx2);
        ampV = b * 5./1023.;
        
        count++;                           // Increment the number of counts
        
        float aa = -0.0000213241;
        float bb =  0.0236566939;
        float cc = -(0.3378053306 + ampV);
        pulseV = (-bb + sqrt(sq(bb) - 4*aa*cc))/(2*aa);
        totalDeadtime += timePulse + timeOLED;
        Serial.println((String)count + " " + (String)initial + " " + (String)ampV + " " + (String)pulseV + " " + (String)(timePulse + timeOLED));
        //Serial.println("ln1 "+(String)count + " " + (String)initial + " " + (String)ampV + " " + (String)pulseV + " " + (String)(18. + timeOLED) + " " + (String)y2);
        //Serial.println("ln2 " + (String)y2 + " "+(String)y3+ " "+(String)y4+ " "+(String)y5+ " "+(String)y6+ " "+(String)y7);
        timeOLED = 0;                      // Reset the OLED screen deadtime
        final = millis();
        timePulse = final - initial;
        interrupts();                      // Allow for interupts to update OLED
        digitalWrite(led, LOW);
        }
    }
}

//TIMER INTERUPT
void timerIsr(){
interrupts(); 
if (OLED == 1){
  getTime();}
noInterrupts();
}

//UPDATE OLED SCREEN. It takes approximately 33ms to run the getTime function
void getTime(){
  
  t1 = millis();
  
  if (ampV > 0.00)   {hist = "-";};
  if (ampV > 0.25)   {hist = "--";};
  if (ampV > 0.50)   {hist = "---";};
  if (ampV > 0.75)   {hist = "----";};
  if (ampV > 1.00)   {hist = "-----";};
  if (ampV > 1.25)   {hist = "------";};
  if (ampV > 1.50)   {hist = "-------";};
  if (ampV > 1.75)   {hist = "--------";};
  if (ampV > 2.00)   {hist = "---------";};
  if (ampV > 2.25)   {hist = "----------";};
  if (ampV > 2.50)   {hist = "-----------";};
  if (ampV > 2.75)   {hist = "------------";};
  if (ampV > 3.00)   {hist = "-------------";};
  if (ampV > 3.25)   {hist = "--------------";};
  if (ampV > 3.50)   {hist = "---------------";};
  if (ampV > 3.75)   {hist = "----------------";};
  if (ampV > 4.00)   {hist = "-----------------";};
  if (ampV > 4.25)   {hist = "------------------";};
  if (ampV > 4.50)   {hist = "-------------------";};
  if (ampV > 4.75)   {hist = "--------------------";};
  
  display.setCursor(0, 0);
  time                = millis() / 1000.;
  seconds             = time    % 60;
  minutes             = time / 60 % 60;
  hours               = time / 3600;
  stdev               = sqrt(count) / (time-totalDeadtime/1000.);
  average             = (float)count / (time-totalDeadtime/1000.);
  display.clearDisplay();
  currentcount        = currentcountstr + " " + count ;
  runtime             = runtimestr + hours + ":" + minutes + ":" + seconds;
  dtostrf(average, 1, 3, tmp);
  
  if(stdev <= 0.001){
    char a = 0.000;
    dtostrf(a, 1, 3, tmp2);
  }
  else{
    dtostrf(stdev, 1, 3, tmp2);
  }
  
  rate = ratestr + tmp + " " + pm + " " + tmp2;
  display.println(currentcount);
  display.println(runtime);
  display.println(hist);
  display.println(rate);  
  display.display();
  
  t2 = millis();
  timeOLED += (t2-t1);

}


