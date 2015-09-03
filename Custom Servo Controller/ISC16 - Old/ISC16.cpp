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

#include <ISC16.h>
#include <Wire.h>
 #define WIRE Wire


// Set to true to print some debug messages, or false to disable them.
#define ENABLE_DEBUG_OUTPUT false

ISC16::ISC16(byte addr) {
  _i2caddr = addr;
}

void ISC16::begin(void) {
 WIRE.begin();
 reset();
}


void ISC16::reset(void) {
 write8(PCA9685_MODE1, 0x0);
}

void ISC16::setPWMFreq(float freq) {
  freq *= 0.9467;  // Frequency Correction Factor
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;
  
  byte oldmode = read8(PCA9685_MODE1);
  byte newmode = (oldmode&0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode); // go to sleep
  write8(PCA9685_PRESCALE, prescale); // set the prescaler
  write8(PCA9685_MODE1, oldmode);
  delay(5);
  write8(PCA9685_MODE1, oldmode | 0xa1);  //  This sets the MODE1 register to turn on auto increment.
                                          // This is why the beginTransmission below was not working.
  //  Serial.print("Mode now 0x"); Serial.println(read8(PCA9685_MODE1), HEX);
}

void ISC16::setPWM(byte num, unsigned int on, unsigned int off) {
  //Serial.print("Setting PWM "); Serial.print(num); Serial.print(": "); Serial.print(on); Serial.print("->"); Serial.println(off);

  WIRE.beginTransmission(_i2caddr);
  WIRE.write(LED0_ON_L+4*num);
  WIRE.write(on);
  WIRE.write(on>>8);
  WIRE.write(off);
  WIRE.write(off>>8);
  WIRE.endTransmission();
}

// Sets pin without having to deal with on/off tick placement and properly handles
// a zero value as completely off.  Optional invert parameter supports inverting
// the pulse for sinking to ground.  Val should be a value from 0 to 4095 inclusive.
void ISC16::setPin(byte num, unsigned int val, bool invert)
{
  // Clamp value between 0 and 4095 inclusive.
  val = min(val, 4095);
  if (invert) {
    if (val == 0) {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 4095) {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else {
      setPWM(num, 0, 4095-val);
    }
  }
  else {
    if (val == 4095) {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 0) {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else {
      setPWM(num, 0, val);
    }
  }
}

byte ISC16::read8(byte addr) {
  WIRE.beginTransmission(_i2caddr);
  WIRE.write(addr);
  WIRE.endTransmission();

  WIRE.requestFrom((byte)_i2caddr, (byte)1);
  return WIRE.read();
}

void ISC16::write8(byte addr, byte d) {
  WIRE.beginTransmission(_i2caddr);
  WIRE.write(addr);
  WIRE.write(d);
  WIRE.endTransmission();
}
