/*
  <docname> - <description>
  Created by:     <author>
  Initial Commit: <date>
*/
#ifndef <docname>_h
#define Initialize_h


//-------------------Include Other Libraries------------------
#include "Arduino.h"




//--------------------Static Definitions----------------------
// Use #define <Value>








//---------------------End Static Definitions------------------

class classname
{
	private:
		int _moveTime; //Time it should take to complete move specified
		int _cmdType;  //Type of command

	public:
		SSC32();
		
		void begin(int baud);

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
