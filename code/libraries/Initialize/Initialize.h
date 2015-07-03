#ifndef _INITIALIZE_H
#define _INITIALIZE_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

//-Extra Includes-
#include <config.h>
#include <SSC32.h>

#define STANDARD 0
#define DEBUG    1
#define VIRTUAL  2

class Initialize {
	private:
		int _mode;
	public:
		Initialize();
		void begin();

};

#endif
