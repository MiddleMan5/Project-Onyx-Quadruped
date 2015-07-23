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
#define iFL 35
#define iFR 34
#define iRR 8
#define iRL 9
    
//-------------------------------------------------------------------------------------------------------------

//On Off Toggle References
#define ON      1
#define OFF     0
#define HIGH    1
#define LOW     0
#define TOGGLE  2

//Movement Scopes
#define BODY  0
#define LEG   1
#define SERVO 2

//runtime modes
#define REBOOT    0
#define LOWPOWER  1
#define STARTUP   2
#define STANDBY   3
#define STOPPED   4
#define MOVING    5
#define DEBUG     6 
#define VIRTUAL   7
#define SHUTDOWN  8
#define ERROR     9

//move types
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3
#define STAND 4
#define SIT   5

#define pi 3.14159;


class Onyx {
	private:
		int _mode;
        int _ServoList[12];
		
		void startup();
	public:
		Onyx();
		void setMode(int mode); //sets runtime state
		void listen();
		void reconnectSSC(); //reinitialize SSC instance communication
		
		void moveScope(int scope);
		void newMove(int type);
		void move(int type);
		
		void Log();
};

class Power {
	private:
		int Main_Relay;
		bool _powerMain; //Power State (ON,OFF)
	public: 
		Power(int Main_Relay);
		void setMainPin(int pin);
		void ToggleMain(int state);
};

class Servo {
	private:
		int _pin;
		bool _isExternal;
		bool _isGroup; //Is the servo part of a group?
		int _groupNumber;
		int _position;
	public: 
		Servo();
		void move(int pulse);
		
		void setGroup(int groupNumber);
		void setMax(int Max);
		void setMin(int Min);
		void intvert();
		void setPin(int pin);
};

class Leg {
	private:
	float _Trochanter_X;
	float _Trochanter_Y;

	float _Femur_X;
	float _Femur_Y; 
	float _Femur;
	float _L1;
	
	float _Tibia;
	float _Tarsus;
	float _L2;
	
		int _quadrant;
		bool _isGroup;
		
    Servo Coxa;
	Servo Knee;
	Servo Ankle;
		
	public: 
		Leg();
		void modify(int servoNum);
		void setMaster();
		void move(int pulse);
		void setQuadrant(int index);
};

#endif
