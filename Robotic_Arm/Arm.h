//Arm library creates classes to represent the various functions and systems of the Arm Robotic.
//The current CPU is an arduino-like Chipkit Uno32

#ifndef _Arm_H
#define _Arm_H

# include <Arduino.h>


//-Extra Includes-
#include "SSC32.h"
#include <math.h>

//Arm Definitions 
#define baudrate 115200 		//Symbols per second. Default hardware set 115200
#define sscReturn 1	            //Is SSC32 Connected bi-directionally? (default "0" = no)

	#define L1 166
	#define L2 58.32
	#define L3 27.76
//----------------------------------------------------------------------------
//SERVO-Power Relay Pin
#define relayServos 27

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

#define X   0
#define Y   1
#define Z   2


#define PI 3.14159


class Arm {
	private:
		int _attitude; //Needs to be further developed (similar to speed, the robots attitude is a prototype hardware-control emotion) greateer attitude = greater speed
		int _mode;
        int _ServoList[12];
		int _Z_rotation;
		int  _X_rotation;
		int _Y_rotation;
		
	public:
		Arm();
		void startup();
		void setAttitude(int attitude); //Current servo "Speed Control" - Prototypical function
		void setMode(int mode); //sets runtime state
		void listen();
		void reconnectSSC(); //reinitialize SSC instance communication
		
		void moveScope(int scope);
		void newMove(int type);
		void move(int type);
		void rotate(int axis, int degree);
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

class SERVO {
	private:
		int _pin;
		int _time;
		bool _isInvert;
		bool _isExternal; //is "on" another controller (the ssc-32 that has a different IO pin name)
		bool _isGroup; //Is the servo part of a group?
		int _groupNumber;
		int _position;
	public: 
		SERVO();
		void move(int pulse);
		void setTime(int input); //time it takes to move
		void setGroup(int groupNumber);
		void setMax(int Max);
		void setMin(int Min);
		void invert();
		void setPin(int pin);
};

class Leg {
	private:
	int _speed;
	float _L1;
	float _L2;
	
		int _quadrant;
		bool _isGroup;
		
    SERVO Wrist;
	SERVO Elbow;
	SERVO Base;
	SERVO Shoulder1;
	SERVO Shoulder2;
		
	public: 
		Leg();
		void rotate(int degree);
		void setSpeed(int speed);
		void setMaster();
		void move(int pulse);
		void setQuadrant(int index);
		void moveFootTo(float x, float y);
};

#endif
