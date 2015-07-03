#ifndef _ONYXSAFETY_H
#define _ONYXSAFETY_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class onyxSafety {
	private:
		// Private functions and variables here
	public:
		onyxSafety();
		void begin();
};

#endif
