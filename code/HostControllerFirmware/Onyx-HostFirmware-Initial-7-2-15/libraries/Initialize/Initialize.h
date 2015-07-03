#ifndef _INITIALIZE_H
#define _INITIALIZE_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class Initialize {
	private:
		// Private functions and variables here
	public:
		Initialize();
		void begin();
};

#endif
