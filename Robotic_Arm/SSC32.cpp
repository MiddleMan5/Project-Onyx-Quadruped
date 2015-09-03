
/**
*	@file SSC32.h
*	@brief Control Lynxmotion's SSC-32 with Arduino.
*	@author Quinn Mikelson (Mikelson.ml)
*	@date 6/10/2015
*/


  //SSC32.cpp - Control Lynxmotion's SSC-32 
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"	// for digitalRead, digitalWrite, etc
#endif

#include "SSC32.h"
#include "Servo.h"


Servo s1;
Servo s2;
Servo s3;
  // attaches the servo on pin 9 to the servo object  

SSC32::SSC32() 
{
	 s1.attach(3);  // attaches the servo on pin 9 to the servo object                            // in steps of 1 degree 
s2.attach(5);  // attaches the servo on pin 9 to the servo object 
s3.attach(6);
	_moveTime = -1;
	_cmdType = NONE;
}

void SSC32::begin(int br)
{
	Serial.begin(br);
}

void SSC32::check()
{
			Serial.println("ver");
			Serial.println(" ");
			delay(20);
}

bool SSC32::newGroup(int type)
{

	if (type < NONE || type > DISCRETE)
	{
		//The command type is not valid
		return false;
	}


	if ( _cmdType != NONE)
	{
		//Can not start another command group while in the middle of another command group
		return false;
	}

	_cmdType = type;
	delay(20); //Weird SSC-32 behaviour without delay 
	return true;

}
/*  */
/**
*	Abort a group of commands
*	@return It will return false if a group command has not been started by calling newGroup. True otherwise.
*/
bool SSC32::abortGroup()
{

	if (_cmdType == NONE)
	{
		//Can not abort, we are not in a group of commands
		return false;
	}

	//According to the manual I should write the ascii character for <esc>
	
	Serial.write(27);
	_cmdType = NONE;
	_moveTime = -1;

	return true;
}

/**
*	End a group of commands
*	@return It will return false if a group command has not been started by calling newGroup or it has been aborted by abortGroup. True otherwise.
*/
bool SSC32::executeGroup()
{


	return true;

}

/**
*	Move the servo at #channel to "position".
*	If this function is called from outside a pair of newGroup/executeGroup	then the servo will move right away.
*
*	But if this function is called inside a gruop of commands, then the servo will not move	until you call executeGroup
*	@param	channel	The servo to move
*	@param	position	The position where to move the servo
*	@return False if the channel or position is not valid or if this function is called while inside a command group other than MOVE. True otherwise.
*/
bool SSC32::servoMove(int channel, int position)
{
	//We are good to go
if(channel==3)s1.write(position);
if(channel==5)s2.write(position);
if(channel==6)s3.write(position);
	}


/**
*	Move the servo at #channel to "position" with speed "speed".
*	If this function is called from outside a pair of newGroup/executeGroup	then the servo will move right away.
*
*	But if this function is called inside a gruop of commands, then the servo will not move	until you call executeGroup.
*	@param channel The servo to move
*	@param position	The position where to move the servo
*	@param speed	The speed for the movement
*	@return False if the channel or position or speed is invalid or if this function is called while inside a command group other than MOVE. True otherwise.
*/
bool SSC32::servoMove(int channel, int position, int speed)
{
	if (channel < CHANNEL_MIN || channel > CHANNEL_MAX)
	{
		//Channel not valid
		return false;
	}

	if (position < PWM_MIN || position > PWM_MAX)
	{
		//Position not valid
		return false;
	}

	if (speed < 0)
	{
		//Speed not valid
		return false;
	}

	if (_cmdType != NONE && _cmdType != MOVE)
	{
		//This can only be called as a single command or inside a group of 
		//commands of type MOVE
		return false;	
	}

	//We are good to go
	Serial.print("#");
	Serial.print(channel);
	Serial.print("P");
	Serial.print(position);
	Serial.print("S");
	Serial.print(speed);
	Serial.print(" ");

	if (_cmdType == NONE)
	{
		//This is a single command so execute it
		Serial.println();
	}

	return true;

}

/**
*	Move the servo at #channel to "position" taking "time" milliseconds.
*	If this function is called from outside a pair of newGroup/executeGroup	then the servo will move right away.
*
*	But if this function is called inside a gruop of commands, then the servo will not move	until you call executeGroup
*	@param channel The servo to move
*	@param position	The position where to move the servo
*	@param moveTime	Time to complete the movement
*	@return False if the channel or position or time is invalid or if this function is called while inside a command group other than MOVE. True otherwise.
*/
bool SSC32::servoMoveTime(int channel, int position, int moveTime)
{
	if (channel < CHANNEL_MIN || channel > CHANNEL_MAX)
	{
		//Channel not valid
		return false;
	}

	if (position < PWM_MIN || position > PWM_MAX)
	{
		//Position not valid
		return false;
	}

	if (moveTime < TIME_MIN || moveTime > TIME_MAX )
	{
		//time not valid
		return false;
	}

	if (_cmdType != NONE && _cmdType != MOVE)
	{
		//This can only be called as a single command or inside a group of 
		//commands of type MOVE
		return false;	
	}

	//We are good to go
	Serial.print("#");
	Serial.print(channel);
	Serial.print("P");
	Serial.print(position);

	if (_cmdType == NONE)
	{
		//This is a single command so execute it
		Serial.print(" T");
		Serial.println(moveTime);
	}else{
		//This is a command group. Store the "move time" for later
		_moveTime = moveTime;
	}

	return true;

}

/**
*	The servo channel will be offset by the amount indicated in the offset value
*	@param channel The servo to move
*	@param offset	The offset to apply
*	@return False if the channel or offset is invalid or if this function is called while inside a command group other than OFFSET. True otherwise.
*/

bool SSC32::offset(int channel, int offset)
{

	if (channel < CHANNEL_MIN || channel > CHANNEL_MAX)
	{
		//Channel not valid
		return false;
	}

	if (offset < OFFSET_MIN || offset > OFFSET_MAX)
	{
		//Offset not valid
		return false;
	}

	if (_cmdType != NONE && _cmdType != OFFSET)
	{
		//This can only be called as a single command or inside a group of 
		//commands of type OFFSET
		return false;	
	}
	
	//We are good to go
	Serial.print("#");
	Serial.print(channel);
	Serial.print(" PO");
	Serial.print(offset);
	Serial.print(" ");

	if (_cmdType == NONE)
	{
		//This is a single command so execute it
		Serial.println();
	}

	return true;

}

/**
*	The channel will go to the indicated level
*	@param channel The output channel
*	@param level	The level to write (HIGH or LOW)
*	@return	It will return false if channel is invalid or if this function is called while inside a command group other than DISCRETE. True otherwise.
*/
bool SSC32::discrete(int channel, bool level)
{
	if (channel < CHANNEL_MIN || channel > CHANNEL_MAX)
	{
		//Channel not valid
		return false;
	}

	if (_cmdType != NONE && _cmdType != DISCRETE)
	{
		//This can only be called as a single command or inside a group of 
		//commands of type OFFSET
		return false;	
	}

	//We are good to go
	Serial.print("#");
	Serial.print(channel);

	if (level == HIGH)
		Serial.print("H");
	else
		Serial.print("L");

	if (_cmdType == NONE)
	{
		//This is a single command so execute it
		Serial.println();
	}

	return true;
}

/**
*	This command allows 8 bits of binary data to be written at once.
*	@param bank The output bank of bits. Value should be one of these : BANK_0, BANK_1, BANK_2, BANK_3
*	@param value The value to write. Should be between 0 and 255.
*	@return It will return false if the bank or the value is not valid. True otherwise.  
*/
bool SSC32::byte(int bank, int value)
{

	if (bank < BANK_0 || bank > BANK_3)
	{
		//Bank is not valid
		return false;
	}

	if (value < 0 || value > 255)
	{
		//Value is not valid
		return false;
	}

	//We are good to go
	Serial.print("#");
	Serial.print(bank);
	Serial.print(":");
	Serial.print(value);
	Serial.println();
	

	return true;
}

/**
*	Check whether there is a movement in progress or not.
*	@return This will return false if the previous movement is complete and true if it is still in progress.
*/
bool SSC32::moving()
{
	char c = '.';
	Serial.println("Q");
	Serial.println();
	
	//delay(50);
	if (Serial.available())
	{

		c = Serial.read();
	
	}
	

	if ( c == '+' )
		return true;
	
	return false;

}

/**
*	This will return a value indicating the pulse width of the selected servo with a resolution	of 10uS. For example, if the pulse width is 1500uS, the returned value would be 150.
*	@param channel The channel to query
*	@return Will return -1 if the channel is not valid or the function is called inside a group of commands. Will return the pusle width of the desired channel otherwise.
*/
int SSC32::query(int channel)
{

	if (channel < CHANNEL_MIN || channel > CHANNEL_MAX)
	{
		//Channel not valid
		return -1;
	}

	if (_cmdType != NONE)
	{
		//This can only be executed from outside a group of commands. That is, as a single command.
		return -1;
	}

	char c;
	Serial.print("QP");
	Serial.print(channel);
	Serial.println();

	delay(50);
	if (Serial.available())
	{	
		c = Serial.read();
		return int(c);
	}else{
		return -1;
	}
}

/**
*	This function reads the digital inputs A, B, C, D, AL, BL, CL and DL according to the SSC-32 user manual
*	@param input The digital input to read. Should be one of these : DIGITAL_INPUT_A, DIGITAL_INPUT_B, DIGITAL_INPUT_C, DIGITAL_INPUT_D, DIGITAL_INPUT_AL, DIGITAL_INPUT_BL, DIGITAL_INPUT_CL, DIGITAL_INPUT_DL
*	@return It will return the state of the desired input (0 or 1). Or -1 if any error happens.  
*/
int SSC32::readDigital(int input)
{

	if (_cmdType != NONE)
	{
		//This can only be executed from outside a group of commands. That is, as a single command.
		return -1;
	}

	switch(input)
	{
		case DIGITAL_INPUT_A:
			//Read digital input A
			Serial.print("A");
			break;
		case DIGITAL_INPUT_B:
			//Read digital input B
			Serial.print("B");
			break;
		case DIGITAL_INPUT_C:
			//Read digital input C
			Serial.print("C");
			break;
		case DIGITAL_INPUT_D:
			//Read digital input D
			Serial.print("D");
			break;
		case DIGITAL_INPUT_AL:
			//Read digital input AL
			Serial.print("AL");
			break;
		case DIGITAL_INPUT_BL:
			//Read digital input BL
			Serial.print("BL");
			break;
		case DIGITAL_INPUT_CL:
			//Read digital input CL
			Serial.print("CL");
			break;
		case DIGITAL_INPUT_DL:
			//Read digital input DL
			Serial.print("DL");
			break;
		default:
			//Input not valid
			return -1;
	}

	char c;
	delay(50);
	if (Serial.available())
	{
		c = Serial.read();
		return int(c);
	}else{
		return -1;
	}

}

/**
*	This function reads the analog inputs VA, VB, VC, VD according to the SSC-32 user manual
*	@param input The analog input to read. Should be one of these : DIGITAL_INPUT_VA, DIGITAL_INPUT_VB, DIGITAL_INPUT_VC, DIGITAL_INPUT_VD
*	@return It will return the state of the desired input (Between 0 and 255). Or -1 if any error happens.  
*/
int SSC32::readAnalog(int input)
{

	if (_cmdType != NONE)
	{
		//This can only be executed from outside a group of commands. That is, as a single command.
		return -1;
	}

	switch(input)
	{
		case ANALOG_INPUT_VA:
			//Read digital input A
			Serial.print("VA");
			break;
		case ANALOG_INPUT_VB:
			//Read digital input B
			Serial.print("VB");
			break;
		case ANALOG_INPUT_VC:
			//Read digital input C
			Serial.print("VC");
			break;
		case ANALOG_INPUT_VD:
			//Read digital input D
			Serial.print("D");
			break;
		
		default:
			//Input not valid
			return -1;
	}

	char c;
	delay(50);
	if (Serial.available())
	{
		c = Serial.read();
		return int(c);
	}else{
		return -1;
	}

}


