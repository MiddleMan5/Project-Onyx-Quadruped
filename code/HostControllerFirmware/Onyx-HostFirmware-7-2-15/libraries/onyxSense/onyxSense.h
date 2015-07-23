#ifndef _ONYXSENSE_H
#define _ONYXSENSE_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class onyxSense {
	private:
		// Private functions and variables here
	public:
		onyxSense();
		void begin();
};

#endif
