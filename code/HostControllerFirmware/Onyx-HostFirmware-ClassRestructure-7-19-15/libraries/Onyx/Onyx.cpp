#include "Onyx.h"


Onyx::Onyx() 
: _ServoList {0,1,2,12,13,14,16,17,18,28,29,30} //I literally have no idea why this works 
{
	_modeCPU = STARTUP;
	_powerServo = OFF;
	_powerMain = ON;
	
	pinMode(relayServos, OUTPUT);
}

void Onyx::begin()
 {
	
}

void Onyx::servos(int power)
{
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

void Onyx::startup(int mode)
{
	if(mode == STANDARD){
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
	}
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

