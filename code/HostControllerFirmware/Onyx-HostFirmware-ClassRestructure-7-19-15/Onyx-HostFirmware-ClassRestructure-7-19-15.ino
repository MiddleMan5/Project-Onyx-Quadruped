/**
* Project Onyx
* Master Branch - Integrated Host Firmware
* Quinn Mikelson - Initial Commit 7/2/2015
*/
#include <Onyx.h>
Onyx MK1;
void setup(){MK1.setMode(STARTUP);} 
void loop(){MK1.setMode(LISTENING);}
