#ifndef _ONYXIK_H
#define _ONYXIK_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif
//Useful Math constants used in IK math.
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

class onyxIK {
	private:
		// Private functions and variables here
	public:
		onyxIK();
		void begin();
};

#endif
