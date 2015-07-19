//CPU library creates a class to represent the CPU of the Onyx Robot.
//The current CPU is an arduino-like Chipkit Uno32

#ifndef _CPU_H
#define _CPU_H


#if (ARDUINO >= 100)
# include <Arduino.h>
#endif

//-Extra Includes-
#include "SSC32.h"


#define baudrate 115200 		//Symbols per second. Default hardware set 115200
#define sscReturn 0	            //Is SSC32 Connected bi-directionally? (default "0" = no)

//Servo-Power Relay Pin
	#define relayServos 27
		//Leg Indicator LED Pins
			#define FL 35
			#define FR 34
			#define RR 8
			#define RL 9
    
//-------------------------------------------------------------------------------------------------------------

#define ON      0
#define OFF     1
#define TOGGLE 2
//
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
