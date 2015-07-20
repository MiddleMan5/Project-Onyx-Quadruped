/*
  SSC32.h - Library for controlling the SSC32 servo controller
  Created by Quinn Mikelson
  Initial Commit 6/10/2015
*/
#ifndef SSC32_h
#define SSC32_h

#include "Arduino.h"

#define CHANNEL_MIN 0
#define CHANNEL_MAX 31

#define PWM_MIN 500
#define PWM_MAX 2500

#define TIME_MIN 1
#define TIME_MAX 65535

#define OFFSET_MIN -100
#define OFFSET_MAX 100

#define NONE 0
#define MOVE 1
#define OFFSET 2
#define DISCRETE 3

#define BANK_0 0
#define BANK_1 1
#define BANK_2 2
#define BANK_3 3

#define DIGITAL_INPUT_A 0
#define DIGITAL_INPUT_B 1
#define DIGITAL_INPUT_C 2
#define DIGITAL_INPUT_D 3

#define DIGITAL_INPUT_AL 4
#define DIGITAL_INPUT_BL 5
#define DIGITAL_INPUT_CL 6
#define DIGITAL_INPUT_DL 7

#define ANALOG_INPUT_VA 8
#define ANALOG_INPUT_VB 9
#define ANALOG_INPUT_VC 10
#define ANALOG_INPUT_VD 11
class SSC32
{
	private:
		int _moveTime; //Time it should take to complete move specified
		int _cmdType;  //Type of command

	public:
		SSC32();
		
		void begin(int br);
		void check();
		boolean newGroup(int type);
		boolean abortGroup();
		boolean executeGroup();

		boolean servoMove(int channel, int position);
		boolean servoMove(int channel, int position, int speed);
		boolean servoMoveTime(int channel, int position, int moveTime);

		boolean offset(int channel, int offset);
		boolean discrete(int channel, boolean level);
		boolean byte(int bank, int value);

		boolean moving();
		int query(int channel);
		int readDigital(int input);
		int readAnalog(int input);
		

};

#endif
