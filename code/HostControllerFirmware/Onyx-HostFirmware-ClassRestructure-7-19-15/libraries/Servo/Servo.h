#ifndef _Servo_H
#define _Servo_H

#include <vector>

#define COXA  0
#define KNEE  1
#define ANKLE 2

class Servo {
	private:
	
	
	
	
	public:
	Servo();
	void store(int type, int value);
	void update();
	
	
};