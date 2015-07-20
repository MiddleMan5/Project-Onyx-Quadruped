#include "Onyx.h"


Onyx::Onyx() 
: _ServoList {0,1,2,12,13,14,16,17,18,28,29,30} //I literally have no idea why this works 
{
	Power Actuators;
	Actuators.setMainRelay(relayServos);
	setMode(LOWPOWER);
}

void Onyx::setMode(int mode)
{
	_modeCPU = mode;
	if(mode==STARTUP)stand();
}

void Onyx::stand()
{
	SSC32 SSC;
	SSC.begin(baudrate);
	if (Serial.available() > 0){
		if (sscReturn == 1){
			Serial.println("ver");
		}
	}
	SSC.newGroup(MOVE);
	delay(20);
	for(int i=0; i<12;i++){
		SSC.servoMove(_ServoList[i],1500,1000);
	}
	SSC.executeGroup();
	setMode(STANDARD);
}

void Onyx::power(int subSystem, int state)
{
	
}

void Onyx::idle()
{
	if(Serial.available()>0){
		switch (Serial.read()){
			case 'p':
				servos(TOGGLE);
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

void Power::Power()
{
	_powerMain = OFF;
}

void Power::setMainRelay(int mainPin)
{
	ToggleMain(OFF);
    pinMode(Main_Relay, OUTPUT);
	ToggleMain(_powerMain);
}

void Power::ToggleMain(int state){
	if(power==ON){ //Turn Servo Relay On
		if(_powerServo==ON)return; //Are Servos already on?
		else digitalWrite(relayServos, HIGH);
			_powerServo = ON;
	}
	if(power==OFF){ //Turn Servo Relay Off
		if(_powerServo==OFF)return; //Are Servos already off?
		else digitalWrite(relayServos, LOW);
			_powerServo = OFF;
	}
	if(power==TOGGLE){ //Toggle Servo Relay
    _powerServo = !_powerServo;
	digitalWrite(relayServos, _powerServo);
		
	}
	
}
