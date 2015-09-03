/*************************************************** 
  This is a library for our Adafruit 16-channel PWM & Servo driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These displays use I2C to communicate, 2 pins are required to  
  interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef _ISC16_H
#define _ISC16_H
 #include "Arduino.h"

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define _ONBYTE1 0xFA
#define _ONBYTE2 0xFB
#define _OFFBYTE1 0xFC
#define _OFFBYTE2 0xFD


class ISC16{
 public:
  ISC16(byte addr = 0x40);
  void begin(void);
  void reset(void);
  void setPWMFreq(float freq);
  void setPWM(byte num, unsigned int on, unsigned int off);
  void setPin(byte num, unsigned int val, bool invert=false);

 private:
  byte _i2caddr;

  byte read8(byte addr);
  void write8(byte addr, byte d);
};

#endif
