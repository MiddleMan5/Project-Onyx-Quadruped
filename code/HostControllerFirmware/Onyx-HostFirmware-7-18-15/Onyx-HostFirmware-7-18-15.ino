/**
* Project Onyx
* Master Branch - Integrated Host Firmware
* Quinn Mikelson - Initial Commit 7/2/2015
*/
#include <CPU.h>

CPU Onyx;

    void setup(){ 
        
      // Onyx.begin(); //Provides config file parsing, Test Mode, Debugging Support. Loop bypass. 
       Onyx.servos(ON);
       Onyx.startup(STANDARD);
       
           
    }//setup
    
    void loop(){
        Onyx.idle();
        //onyxSaftey();    //Provides State limiting control. Overtime, Power Off,
       // onyxSense();     //Scan for input change
       // onxyState();     //Check for State change (Serial Event, Hardware Event{Gyro, etc.}, Timer shutdown event)
       // onyxMap();       //If any movement change, update Look Up Table
        
    }//loop
