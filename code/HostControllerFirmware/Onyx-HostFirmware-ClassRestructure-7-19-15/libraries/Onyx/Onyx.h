//Onyx library creates classes to represent the various functions and systems of the Onyx Robotic Quadruped.
//The current CPU is an arduino-like Chipkit Uno32

#ifndef _Onyx_H
#define _Onyx_H


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

//runtime modes
#define REBOOT    0
#define LOWPOWER  1
#define STARTUP   2
#define IDLE      3
#define LISTENING 4
#define STOPPED   5
#define MOVING    6
#define DEBUG     7 
#define VIRTUAL   8
#define SHUTDOWN  9
#define EMERGENCY 10 

//moveXXXX directions
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

#define pi 3.14159;

class Onyx {
	private:
		static int _modeCPU;
        int _ServoList[12];
		
		void stand();
	public:
		Onyx();
		void setMode(int mode); //Starts the system in predefined runtime state
		void power(int subSystem, int state); //IE: Onyx.power(Servos,ON); (Takes ON, OFF, or TOGGLE)
		void moveBody(int direction);
};

class Power {
	private:
		static bool _powerServo; 
		static bool _powerMain;
	public: 
		Power();
		void Servos(int power);
		void moveBody(int Command);
};


#endif
