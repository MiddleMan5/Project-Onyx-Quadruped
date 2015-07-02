/**
* Project Onyx
* Master Branch - Integrated Host Firmware
* Quinn Mikelson
*/

//Configuration Definitions 
#include "config.h"
#include "verbosity.h"

//State-Specific Headers
#include "onyxIdle.h"
#include "onyxSafety.h"
#include "onyxSense.h"
#include "onyxState.h"

//Support Headers
#include "Math.h"
#include "onyxIK"

    void setup(){ 
        
        Initialize(); //Provides config file parsing, Test Mode, Debugging Support. Loop bypass. 
                 
    }//setup
    
    void loop(){
        
        onyxIdle();      //Default State. Auto Safe mode(auto standLevel, auto standCenter, auto power, auto refresh)
        onyxSaftey(); //Provides State limiting control. Overtime, Power Off,
        onyxSense();     //Scan for input change
        onxyState();     //Check for State change (Serial Event, Hardware Event{Gyro, etc.}, Timer shutdown event)
        
    }//loop
