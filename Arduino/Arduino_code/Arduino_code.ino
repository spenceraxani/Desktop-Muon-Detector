/*
Desktop Muon Counter Arduino Code
Questions?
Spencer N. Axani 
saxani@mit.edu

Requirements:
  1. Adafruit GFX Library -- by Adafruit Version 1.0.2
  2. Adafruit SSD1306     -- by Adafruit Version 1.0.0
  3. Wire                 -- by Arduino Version 1.0.0
  4. OzOLED               -- Version unknown, can be found on github. Or in the supplementary material.
  5. SPI                  -- by Arduino Version 1.0.0 
  6. TimerOne             -- by Jesse Tane et al. Version 1.1.0

 Simply un-zip the libraries.zip file and place the files into the Arduino/libraries folder on your machin
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <OzOLED.h>
#include <SPI.h>
#include <TimerOne.h>  

const int OLED = 1;                // Turn on/off the OLED [1,0] (Set to 0 to improve deadtime)
const int LED  = 1;                // Turn on/off the LED delay [1,0] (Set to 0 to improve deadtime)

//INTERUPT SETUP
#define TIMER_INTERVAL 1000000     // Every 1,000,000 us the timer will update the clock on the OLED

//OLED SETUP
#define OLED_RESET 10              // A signal to this pin will reset the OLED screen
Adafruit_SSD1306 display(OLED_RESET);

//Linear regression parameters
float x1,x2,x3,x4,x5,x6;
float y1,y2,y3,y4,y5,y6;

float Sx2y,Sylny,Sxy,Sxylny,Sy;
float a, b;


//Timing variables
long measurement_t1  = 0L;                // Time stamp
long measurement_t2  = 0L;                // Used to calculate deadtime of signal measurement

long OLED_t1         = 0L;                // Used to calcualte deadtime of OLED screen update
long OLED_t2         = 0L;                // Used to calcualte deadtime of OLED screen update

long OLED_deadtime        = 0L;           // Tallies the total deadtime of the OLED screen between signal measurements
long measurement_deadtime = 0L;
long total_deadtime       = 0L;

const float TIME_INTERVAL      = 5.8;     // Using a prescaler of 4, the time between analogue samples is 5.8 us.          

String      hist = "Initiallizing...";    // A histogram representing the relative number of photons seen by the SiPM

const float SIGNAL_THRESHOLD  = 80;        // Signal threshold, measured from 0-1023. Measureing from 0 to 300mV. 70 corresponds to approximately 20 mV threshold. 
const int   LED_PIN           = 3;         // The LED digital pin connections. 
long int    count             = -1L;       // A tally of the number of muon counts observed



float SiPM_voltage     = 0;                  // This is the calculated pulse amplitude of the SiPM signal
float signal_voltage   = 0;                  // This is the measured signal amplitude from the pulse detector circuit

const String current_count_str  = "Total Count:";
const String runtime_str        = "Run Time:   ";
const String rate_str           = "Rate: ";
const String pm                 = "+/-";

long int time           = 0.;
int      seconds        = 0.;
int      minutes        = 0.;
int      hours          = 0.;
float    stdev          = 0.;
float    average        = 0.;
char     tmp2[15];
char     tmp[15];
String   current_count;
String   runtime;
String   rate;

void setup() {
Serial.begin(9600);

ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2));    // clear prescaler bits
ADCSRA |= bit (ADPS1);                                   // Set prescaler to 4  

 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);              // OLED screen
 display.clearDisplay(); 

if (OLED == 1){
  display.setTextSize(1);
  display.setTextColor(WHITE);
}


pinMode(LED_PIN, OUTPUT);                                // Setup the LED pin to output

Timer1.initialize(TIMER_INTERVAL);                       // Initialise timer 1
Timer1.attachInterrupt(timerIsr);                        // attach the ISR routine

getTime();
}

void loop() {
  while (1) {
    if (analogRead(A0) > SIGNAL_THRESHOLD) // If true, we have an event!
        {
        y2 = analogRead(A0);              // Read analogue pin A0 five times.
        y3 = analogRead(A0);
        y4 = analogRead(A0);
        y5 = analogRead(A0);
        y6 = analogRead(A0);
       
        total_deadtime += measurement_deadtime + OLED_deadtime; // time_pulse == deadtime of previous measuremnet. time_oled == deadtime associated with updating clock.
        
        measurement_t1 = millis();        // Time stamp of event
                      
        noInterrupts();                   // Turn of the interupts, so that it doesn't update the OLED screen mid measuremnt
        x1 = 0;                           // This is the time of the first measurement: y1
        x2 = x1 + TIME_INTERVAL + 0.5;    // It takes 504ns to perform the IF statement
        x3 = x2 + TIME_INTERVAL;
        x4 = x3 + TIME_INTERVAL;
        x5 = x4 + TIME_INTERVAL;
        x6 = x5 + TIME_INTERVAL;
        digitalWrite(LED_PIN, HIGH);          // Turn on the LED
        
        // Calculate the initial pulse amplitude. This is just a exponential regression fit.
        Sx2y   = x2*x2*y2 + x3*x3*y3 + x4*x4*y4 + x5*x5*y5 + x6*x6*y6;
        Sylny  = y2*log(y2) + y3*log(y3) + y4*log(y4) + y5*log(y5) + y6*log(y6);
        Sxy    = x2*y2 + x3*y3 + x4*y4 + x5*y5 + x6*y6;
        Sxylny = x2*y2*log(y2) + x3*y3*log(y3) + x4*y4*log(y4) + x5*y5*log(y5) + x6*y6*log(y6);
        Sy     = y2 + y3 + y4 + y5 + y6;

        a = (Sx2y*Sylny - Sxy*Sxylny) / (Sy*Sx2y - Sxy*Sxy);
        b = (Sy*Sxylny - Sxy*Sylny) / (Sy*Sx2y - Sxy*Sxy);
        y1 = exp(a);                          // y1 is the calculated amplitude of the triggering pulses at the time of the trigger.
        
        
        signal_voltage = y1 * 5./1023.;    // Convert the measured value, y1 , to a voltage (0-1024 == 0-5V)
        count++;                           // Increment the number of counts
        
        float aa = -0.0000213241;         // These numbers come from the calibration of the electronics. See Calibration section in paper.
        float bb =  0.0236566939;
        float cc = -(0.3378053306 + signal_voltage);
        SiPM_voltage = (-bb + sqrt(sq(bb) - 4*aa*cc))/(2*aa);
        if (count > 0){
          Serial.println((String)count + " " + (String)measurement_t1 + " " + (String)signal_voltage + " " + (String)SiPM_voltage + " " + (String)(measurement_deadtime + OLED_deadtime));
        }
        
        
        interrupts();                      // Allow for interupts, to update OLED
        
        if (LED == 1){                        
          delay(signal_voltage*500);}
        digitalWrite(LED_PIN, LOW);            // Turn off the LED
        
        measurement_t2 = millis();
        measurement_deadtime = measurement_t2 - measurement_t1;
        OLED_deadtime = 0;
        
        
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
  
  OLED_t1 = millis();
  
  if (signal_voltage > 0.0)   {hist = "-";};
  if (signal_voltage > 0.2)   {hist = "--";};
  if (signal_voltage > 0.4)   {hist = "---";};
  if (signal_voltage > 0.6)   {hist = "----";};
  if (signal_voltage > 0.8)   {hist = "-----";};
  if (signal_voltage > 1.0)   {hist = "------";};
  if (signal_voltage > 1.2)   {hist = "-------";};
  if (signal_voltage > 1.4)   {hist = "--------";};
  if (signal_voltage > 1.6)   {hist = "---------";};
  if (signal_voltage > 1.8)   {hist = "----------";};
  if (signal_voltage > 2.0)   {hist = "-----------";};
  if (signal_voltage > 2.2)   {hist = "------------";};
  if (signal_voltage > 2.4)   {hist = "-------------";};
  if (signal_voltage > 2.6)   {hist = "--------------";};
  if (signal_voltage > 2.8)   {hist = "---------------";};
  if (signal_voltage > 3.0)   {hist = "----------------";};
  if (signal_voltage > 3.2)   {hist = "-----------------";};
  if (signal_voltage > 3.4)   {hist = "------------------";};
  if (signal_voltage > 3.6)   {hist = "-------------------";};
  if (signal_voltage > 3.8)   {hist = "--------------------";};
  
  display.setCursor(0, 0);
 
  time                = millis() / 1000.;
  seconds             = time    % 60;
  minutes             = time / 60 % 60;
  hours               = time / 3600;
  stdev               = sqrt(count) / (time-total_deadtime/1000.);
  average             = (float)count / (time-total_deadtime/1000.);
  display.clearDisplay();
  current_count        = current_count_str + " " + count ;
  runtime             = runtime_str + hours + ":" + minutes + ":" + seconds;
  dtostrf(average, 1, 3, tmp);
  
  if(stdev <= 0.001){
    char a = 0.000;
    dtostrf(a, 1, 3, tmp2);
  }
  else{
    dtostrf(stdev, 1, 3, tmp2);
  }
  
  rate = rate_str + tmp + " " + pm + " " + tmp2;
  display.println(current_count);
  display.println(runtime);
  display.println(hist);
  display.println(rate);  
  display.display();
  //Serial.println(rate);
  OLED_t2 = millis();
  
  OLED_deadtime += (OLED_t2-OLED_t1);


}


