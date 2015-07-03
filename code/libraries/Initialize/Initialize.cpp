#include <Initialize.h>
SSC32 SSC;

Initialize::Initialize() {
	_mode = STANDARD;
}

void Initialize::begin() {
	SSC.begin(baud);
	if(Serial.available() > 0){
		if(sscReturn == 1){
			Serial.println("ver");
		}
	}
}
