/*************************************************** 
  Controller Firmware for the ISC16 Servo Driver.
  This uses Quinn Mikelson's ISC16 PCA9685 I2C library ported from Adafruit's Library.

  Adafruit Driver Module:
  http://www.adafruit.com/products/815

  Written by Quinn Mikelson.  
 ****************************************************/

#include <Wire.h>
#include <ISC16.h>

ISC16 pwm(0x40); //Initialize Servo Module at default address
long baud = 115200;

int frequency = 60; //Hz
int resolution = 4096; // 4096 bits per 1 cycle
double uSecPerHz = 1000000 / frequency;
double  uSecPerResolution = uSecPerHz / resolution;


int Pulse = 1500;
int onstep = 0;
int offstep = ((onstep + Pulse / uSecPerResolution)-1); 

// our servo # counter
uint8_t servonum = 0;

void setup() {
  Serial.begin(baud);
  pwm.begin();
  offstep = offstep % 4096;
  pwm.setPWMFreq(frequency);  // Analog servos run at ~60 Hz updates
 
 
}


void loop() {
    switch(Serial.read()){
     case 'w':
      Pulse+=10;
     break; 
     case 's':
      Pulse-=10;
     break; 
    }
    Serial.println(Pulse);
    offstep = ((onstep + Pulse / uSecPerResolution)-1); 
    pwm.setPWM(0, onstep, offstep);
    pwm.setPWM(15, onstep, offstep);
}
