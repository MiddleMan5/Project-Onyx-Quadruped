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

void Onyx::reconnectSSC();
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

void Onyx::moveScope(int scope)
{
	
	if(scope==BODY)
	{
		SSC.newGroup(MOVE);
	}
	if(scope==leg)
	{
		
	}
	if(scope==servo)
	{
		
	}
	else return;
}

Onyx::move(int type)
{
	
	if(type == STAND){
		moveScope(BODY);
		for(int i=0; i<12;i++){
		SSC.servoMove(_ServoList[i],1500,1000);
		}
		SSC.executeGroup();
	}
	if(type == SIT){
		moveScope(BODY);
		for(int i=1; i<12;i+=3){
		SSC.servoMove(_ServoList[i],1700,1000);
		}
		SSC.executeGroup();
	}
	
}

void Onyx::listen()
{
	if(Serial.available()>0){
		switch (Serial.read()){
			case 'p':
				Actuators.ToggleMain(TOGGLE);
				Serial.println("Toggle");
			break;
			case 'w':
			move(STAND);
			Serial.println("Stand");
			break;
			case 's':
			move(SIT);
			Serial.println("Sit");
			break;
			case 'd':
			
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

Leg::Leg(int index)
{
	Servo Coxa(index);
	Servo Knee(index + 1);
	Servo Ankle(index + 2);
}

Leg::setMaster()
{
	if
}

Servo::Servo(int pin)
{
	_pin = pin;
	_isExternal = 1;
}
