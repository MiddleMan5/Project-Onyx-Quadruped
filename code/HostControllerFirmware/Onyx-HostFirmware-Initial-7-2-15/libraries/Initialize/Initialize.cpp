#include "Initialize.h"


Initialize::Initialize() {
	_mode = STANDARD;
}

void Initialize::begin() {
	SSC32 SSC;
	SSC.begin(baudrate);
	if (Serial.available() > 0){
		if (sscReturn == 1){
			Serial.println("ver");
		}
	}
	pinMode(servoPower, OUTPUT);
	digitalWrite(servoPower, HIGH);
	SSC.newGroup(MOVE);
	delay(20);
	for(int i=0; i<32;i++){
		SSC.servoMove(i,1500,1000);
	}
	SSC.executeGroup();
	SSC.newGroup(MOVE);
	
}
