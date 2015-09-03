/**
* Project Onyx
* Master Branch - Integrated Host Firmware
* Quinn Mikelson - Initial Commit 7/2/2015
*/
#include "Arm.h"
Arm MK1;
void setup(){MK1.setMode(STARTUP);} 
void loop(){MK1.setMode(STANDBY);}
