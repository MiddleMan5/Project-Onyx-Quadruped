#ifndef _ONYXIK_H
#define _ONYXIK_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class onyxIK {
	private:
		// Private functions and variables here
	public:
		onyxIK();
		void begin();
};

#endif
