#ifndef _ONYXIDLE_H
#define _ONYXIDLE_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class onyxIdle {
	private:
		// Private functions and variables here
	public:
		onyxIdle();
		void begin();
};

#endif
