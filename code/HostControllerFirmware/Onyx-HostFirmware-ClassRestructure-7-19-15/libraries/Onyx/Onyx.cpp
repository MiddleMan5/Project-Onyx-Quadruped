#include "Onyx.h"

SSC32 SSC;
Power Actuators(relayServos);
Leg FL(0);
Leg RL(12);
Leg FR(16);
Leg RR(28);

Onyx::Onyx() 
: _ServoList {0,1,2,12,13,14,16,17,18,28,29,30} //I literally have no idea why this works 
{
	setMode(LOWPOWER);
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

void Onyx::startup()
{
	Serial.end();
	SSC.begin(baudrate);
	SSC.check();
	SSC.newGroup(MOVE);
	for(int i=0; i<12;i++){
		SSC.servoMove(_ServoList[i],1500,1000);
	}
	SSC.executeGroup();
	setMode(STANDBY);
}

void Onyx::listen()
{
	if(Serial.available()>0){
		switch (Serial.read()){
			case 'p':
				Actuators.ToggleMain(TOGGLE);
			break;
			case 's':
			break;
			case 'a':
			
			break;
			case 'd':
			
			break;
		}
	}
	
}

void Onyx::moveBody(int Command) //Move center of mass in direction (UP,DOWN,LEFT,RIGHT)
{
	
}

Power::Power(int Main_Relay)
{
	ToggleMain(OFF);
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

Leg::Leg(int index)
{
	Servo Coxa(index);
	Servo Knee(index + 1);
	Servo Ankle(index + 2);
}

Servo::Servo(int pin)
{
	_pin = pin;
	_isExternal = 1;
}
