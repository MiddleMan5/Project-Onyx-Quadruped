//Onyx library creates classes to represent the various functions and systems of the Onyx Robotic Quadruped.
//The current CPU is an arduino-like Chipkit Uno32

#ifndef _CPU_H
#define _CPU_H


#if (ARDUINO >= 100)
# include <Arduino.h>
#endif

//-Extra Includes-
#include "SSC32.h"

//Onyx Definitions 
#define baudrate 115200 		//Symbols per second. Default hardware set 115200
#define sscReturn 0	            //Is SSC32 Connected bi-directionally? (default "0" = no)

//Physical Dimensions //all in floating point millimetres accurate to the thousandth
	#define Trochanter_X 61.40
	#define Trochanter_Y 4.34 //CAD Model Incorrect!!

	#define Femur_X 24.29 
	#define Femur_Y 50.78 
	#define Femur   56.29 //The L1 Value

	#define Tibia 41.02
	#define Tarsus 62.00 //Tibia + Tarsus = L2 Value
	
	#define Body_Width 139.72
	#define Body_Length 106.25
//----------------------------------------------------------------------------
//Servo-Power Relay Pin
#define relayServos 27

//Leg Indicator LED Pins
#define FL 35
#define FR 34
#define RR 8
#define RL 9
    
//-------------------------------------------------------------------------------------------------------------

//On Off Toggle References
#define ON      0
#define OFF     1
#define TOGGLE  2

//startup modes
#define STANDARD 0
#define DEBUG    1
#define VIRTUAL  2
#define STARTUP  3

//moveBody
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

#define pi 3.14159;

class CPU {
	private:
		 int _modeCPU;
		bool _powerServo; 
		bool _powerMain;
	
        int _ServoList[12];
	public:
		CPU();
		void begin(); //Start the object and 
		void servos(int power);
		void startup(int mode);
		void idle();
		void moveBody(int Command);
};

#endif
