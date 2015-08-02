#include "Onyx.h"

SSC32 SSC;
Leg Legs[4];
Power Actuators(relayServos);

Onyx::Onyx() 
: _ServoList {0,1,2,12,13,14,16,17,18,28,29,30} //I literally have no idea why this works 
{
	setAttitude(200);
	setMode(LOWPOWER);
	for(int i=0;i<4;i++){
		Legs[i].setQuadrant(i);
	}
}

void Onyx::setMode(int mode)
{
	_mode = mode;
	if(mode==STARTUP)startup();
	if(mode==STANDBY)listen();
	if(mode==SHUTDOWN)Actuators.ToggleMain(OFF);
	if(mode==LOWPOWER)setMode(SHUTDOWN);
	if(mode==REBOOT)startup();
}

void Onyx::reconnectSSC()
{
	Serial.end();
	SSC.begin(baudrate);
	SSC.check();
}

void Onyx::Log()
{
	static int O = 0;
	Serial.println(O++);
}

void Onyx::startup()
{
	reconnectSSC();
	Actuators.ToggleMain(ON);
	move(SIT);
	setMode(STANDBY);
	rotate(Z,0);
	rotate(X,0);
	rotate(Y,0);
}

void Onyx::setAttitude(int attitude)
{
	_attitude = constrain(attitude,0,255);
	for(int i=0;i<4;i++){
		Legs[i].setSpeed(_attitude);
	}
	
}

void Onyx::moveScope(int scope)
{
	
	if(scope!=SERVO)
	{
		SSC.newGroup(MOVE);
	}
	else return;
}

void Onyx::move(int type)
{
	
	if(type == STAND){
		moveScope(BODY);
		for(int i=0; i<4;i++){
			Legs[i].moveFootTo(L1,L2);
		}
		
	}
	if(type == SIT){
		moveScope(BODY);
		for(int i=0; i<4;i++){
			Legs[i].moveFootTo(114.3,25.4);
		}
	}
	
	if(type == UP){
		moveScope(BODY);
		
	}
	
	if(type == DOWN){
		moveScope(BODY);
		
	}
	
}

void Onyx::rotate(int axis,int degree)
{
	
	if(axis == X){
		_X_rotation = constrain(degree,-45,45);
		
			for(int i=0; i<4;i++){
				Legs[i].moveFootTo(L1,L2+_X_rotation);
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

void Onyx::listen()
{
	if(Serial.available()>0){
		switch (Serial.read()){
			case 'p':
				Actuators.ToggleMain(TOGGLE);
				Log();
			break;
			
			case 'w':
			move(STAND);
			break;
			case 's':
			move(SIT);
			break;
			case 'd':
			move(STAND);
			break;
			case 'a':
			move(SIT);
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
		}
	
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
	Servo Coxa;
	Servo Knee;
	Servo Ankle;
	
	_L1=Femur;
	_L2=Tibia+Tarsus;
	
}

void Leg::rotate(int degree)
{
	Coxa.move(degree*10+1500);
}

void Leg::setSpeed(int speed)
{
	_speed=speed;
	Coxa.setTime(_speed);
	Knee.setTime(_speed);
	Ankle.setTime(_speed);
}

void Leg::move(int pulse)
{
	Knee.move(pulse);
	Ankle.move(pulse);
	
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

	Knee.move(pulse1); //conversion to degrees using new variable
    Ankle.move(pulse2);
	SSC.executeGroup();
} 



void Leg::setQuadrant(int index)
{
	_quadrant = index + 1;
	
	if(_quadrant==1)
	{
		index=16;
		
	}
	if(_quadrant==2)
	{
		index=0;
		Knee.invert();
		Ankle.invert();
	}
	if(_quadrant==3)
	{
		index=12;
	}
	if(_quadrant==4)
	{
		index=28;
		Knee.invert();
		Ankle.invert();
	}
	
	Coxa.setPin(index);
	Knee.setPin(index+1);
	Ankle.setPin(index+2);
	
}




Servo::Servo()
{
	_isExternal = true;
	_isInvert = false;
}

void Servo::setPin(int pin)
{
		_pin=pin;
}

void Servo::invert()
{
	_isInvert = !_isInvert;
}

void Servo::setTime(int time)
{
		_time = map(time,0,255,5000,1);
}

void Servo::move(int pulse)
{
	if(_isInvert) _position = map(pulse,600,2400,2400,600);
	else _position = pulse;
	SSC.servoMoveTime(_pin,_position,_time); //Greater time means slower speed (DUH!)
}


