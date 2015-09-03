#include "Arm.h"

SSC32 SSC;
Leg Legs[4];
Power Actuators(relayServos);

Arm::Arm() 
: _ServoList {3,5,6,7} //I literally have no idea why this works 
{
	setAttitude(200);
	setMode(LOWPOWER);
	Legs[0].setQuadrant(0);
}

void Arm::setMode(int mode)
{
	_mode = mode;
	if(mode==STARTUP)startup();
	if(mode==STANDBY)listen();
	if(mode==SHUTDOWN)Actuators.ToggleMain(OFF);
	if(mode==LOWPOWER)setMode(SHUTDOWN);
	if(mode==REBOOT)startup();
}

void Arm::reconnectSSC()
{
	Serial.end();
	SSC.begin(baudrate);
	SSC.check();
}

void Arm::Log()
{
	static int O = 0;
	Serial.println(O++);
}

void Arm::startup()
{
	reconnectSSC();
	Actuators.ToggleMain(ON);
	move(STAND);
	setMode(STANDBY);
}

void Arm::setAttitude(int attitude)
{
	_attitude = constrain(attitude,0,255);
	for(int i=0;i<4;i++){
		Legs[i].setSpeed(_attitude);
	}
	
}

void Arm::moveScope(int scope)
{
	
	if(scope!=SERVO)
	{
		SSC.newGroup(MOVE);
	}
	else return;
}

void Arm::move(int type)
{
	
	if(type == STAND){
		moveScope(BODY);
 
			Legs[0].moveFootTo(L1,L2);
		 
		
	}
	if(type == SIT){
		moveScope(BODY);
			Legs[0].moveFootTo(L1,L2);
	}
	
	if(type == UP){
		moveScope(BODY);
		
	}
	
	if(type == DOWN){
		moveScope(BODY);
		
	}
	
}

void Arm::rotate(int axis,int degree)
{
	
	if(axis == X){
		_X_rotation = constrain(degree,-45,45);
		
			for(int i=0; i<4;i++){
				Legs[0].moveFootTo(L1,L2+_X_rotation);
			}
			
			
			//Legs[3].moveFootTo(L1,L2-(L1*tan(_X_rotation)));
			
	}
	
	/*if(axis == Y){
		_Y_rotation = constrain(degree,-45,45);
			Legs[0].move();
			Legs[1].move();
			Legs[2].move();
			Legs[3].move();
			
	}*/
	
	if(axis == Z){
		_Z_rotation = constrain(degree,-45,45);
			for(int i=0; i<4;i++){
				Legs[i].rotate(_Z_rotation);
			}
	}
	
}

void Arm::listen()
{
	static int x=L1;
	static int y=L2;
	
	
		switch (Serial.read()){
			case 'p':
				Actuators.ToggleMain(TOGGLE);
				Log();
			break;
			
			case 'w':
			y+=1;
			break;
			case 's':
			y-=1;
			break;
			case 'd':
			x+=1;
			break;
			case 'a':
			x-=1;
			break;
			
			case 'q':
			rotate(X,_X_rotation+5);
			break;
			case 'e':
			rotate(X,_X_rotation-5);
			break;
			
			case 'r':
			setAttitude(_attitude + 10);
			Serial.println(_attitude);
			break;
			case 'f':
			setAttitude(_attitude - 10);
			Serial.println(_attitude);
			break;
			
		}
		Legs[0].moveFootTo(x,y);
	
}

Power::Power(int Main_Relay)
{
	_powerMain = OFF;
	setMainPin(Main_Relay);
}

void Power::setMainPin(int pin)
{
	Main_Relay = pin;
	ToggleMain(OFF);
    pinMode(Main_Relay, OUTPUT);
	ToggleMain(_powerMain);
}

void Power::ToggleMain(int state){
	if(state != _powerMain){
		if(state==TOGGLE)ToggleMain(!_powerMain);
		else {
			digitalWrite(Main_Relay,state);
			_powerMain = state;
		}
	}
	else return;
	
}

Leg::Leg()
{
	SERVO Shoulder1;
	SERVO Shoulder2;
	SERVO Elbow;
	SERVO Wrist;
	
	SERVO Base;
	_L1=L1;
	_L2=L2;
	
}

void Leg::rotate(int degree)
{
	Base.move(degree*10+1500);
}

void Leg::setSpeed(int speed)
{
	_speed=speed;
    Shoulder1.setTime(_speed);
	Shoulder2.setTime(_speed);
	Elbow.setTime(_speed);
	Wrist.setTime(_speed);
}

void Leg::move(int pulse)
{
	Shoulder1.move(pulse);
	Shoulder2.move(pulse);
	Elbow.move(pulse);
	
	SSC.executeGroup();
}

void Leg::moveFootTo(float x, float y)
{ //Inverse Kinematic
        float theta2=acos(((x*x)+(y*y)-(_L1*_L1)-(_L2*_L2))/(2*_L1*_L2));
        float theta1=((-((_L2*sin(theta2))*x) + ((_L1+_L2*cos(theta2))*y))/(((_L2*sin(theta2)*y) +(_L1+(_L2*cos(theta2)))*x)));
        float deg1=theta1*180/PI; //conversion to degrees
        float deg2=theta2*180/PI;
		Serial.println(deg1);
		Serial.println(deg2);
        float pulse1=-(deg1*10)+1500;//offset to mechanical center (0 degrees IK)
        float pulse2=(deg2*10)+600;//offset to mechanical center (90 degrees IK)

	Shoulder1.move(pulse1); //conversion to degrees using new variable
    Shoulder2.move(pulse1);
	Elbow.move(pulse2);
	SSC.executeGroup();
} 



void Leg::setQuadrant(int index)
{
	_quadrant = index + 1;
	
	if(_quadrant==1)
	{
		index=16;
		
	}
	Shoulder1.setPin(index);
	Shoulder2.setPin(index+1);
	Elbow.setPin(index+4);
	Wrist.setPin(index+5);
	
}




SERVO::SERVO()
{
	_isExternal = true;
	_isInvert = false;
}

void SERVO::setPin(int pin)
{
		_pin=pin;
}

void SERVO::invert()
{
	_isInvert = !_isInvert;
}

void SERVO::setTime(int time)
{
		_time = map(time,0,255,5000,1);
}

void SERVO::move(int pulse)
{
	if(_isInvert) _position = map(pulse,600,2400,2400,600);
	else _position = pulse;
	SSC.servoMoveTime(_pin,_position,_time); //Greater time means slower speed (DUH!)
}


