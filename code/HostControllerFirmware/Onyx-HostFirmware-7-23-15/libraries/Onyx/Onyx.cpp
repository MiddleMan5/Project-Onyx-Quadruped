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
		Legs[i].setQuadrant(i+1);
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
	move(STAND);
	setMode(STANDBY);
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
			Legs[i].move(1500);
		}
		
	}
	if(type == SIT){
		moveScope(BODY);
		for(int i=0; i<4;i++){
			Legs[i].move(1700);
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
			Log();
			break;
			case 's':
			move(SIT);
			Log();
			break;
			case 'q':
			setAttitude(_attitude + 10);
			Serial.println(_attitude);
			break;
			case 'a':
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
	Coxa.move(pulse);
	Knee.move(pulse);
	Ankle.move(pulse);
	
	SSC.executeGroup();
}

void Leg::setQuadrant(int index)
{
	_quadrant = index;
	
	if(_quadrant==1)index=16;
	if(_quadrant==2)index=0;
	if(_quadrant==3)index=12;
	if(_quadrant==4)index=28;
	
	Coxa.setPin(index);
	Knee.setPin(index+1);
	Ankle.setPin(index+2);
	
	Coxa.invert();
	Knee.invert();
	Ankle.invert();
}




Servo::Servo()
{
	_isExternal = true;
	_isInvert = false;
	Serial.println(_isInvert);
}

void Servo::setPin(int pin)
{
		_pin=pin;
}

void Servo::invert()
{
	_isInvert = true;
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


