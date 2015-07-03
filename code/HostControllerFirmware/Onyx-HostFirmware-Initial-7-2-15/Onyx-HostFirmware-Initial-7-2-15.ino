/**
* Project Onyx
* Master Branch - Integrated Host Firmware
* Quinn Mikelson - Initial Commit 7/2/2015
*/


#include <onyxIK.h>
#include <onyxState.h>
#include <onyxSense.h>
#include <onyxSafety.h>
#include <onyxIdle.h>
#include <Initialize.h>

Initialize Onyx;

    void setup(){ 
        
        Onyx.begin(); //Provides config file parsing, Test Mode, Debugging Support. Loop bypass. 
                 
    }//setup
    
    void loop(){
        
        //onyxIdle();      //Default State. Auto Safe mode(auto standLevel, auto standCenter, auto power, auto refresh)
        //onyxSaftey();    //Provides State limiting control. Overtime, Power Off,
       // onyxSense();     //Scan for input change
       // onxyState();     //Check for State change (Serial Event, Hardware Event{Gyro, etc.}, Timer shutdown event)
       // onyxMap();       //If any movement change, update Look Up Table
        
    }//loop
