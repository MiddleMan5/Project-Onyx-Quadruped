#ifndef _ONYXSTATE_H
#define _ONYXSTATE_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class onyxState {
	private:
		// Private functions and variables here
	public:
		onyxState();
		void begin();
};

#endif
