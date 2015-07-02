
/*
  Simple .h and .cpp Templates to facilitate creation of libraries 
*/

/*
  <docname> - <description>
  Created by:     <author>
  Initial Commit: <date>
*/

//-------------------Include Other Libraries------------------
#include "Arduino.h


//--------------------Static Definitions----------------------
//#define example 11;


//---------------------End Static Definitions------------------


//--------------------Start Object Calls-----------------------


/*Create class association with constructor.
examp::examp()
{
Initial Variable Set
}
This creates a format called by "examp examp(<variables>)" and allows for examp.<object> calls.

*/
<classname>::<classname>() //can take mode parameters
{
	//private variable initialization
}

/* Function object declaration IE: "<functionclass>.begin(int baud)"

void SSC32::begin(int baud)
{
	Serial.begin(baud);
}
*/




