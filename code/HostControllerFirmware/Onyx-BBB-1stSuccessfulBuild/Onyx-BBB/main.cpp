/* 
 * File:   main.cpp
 * Author: Quinn
 *
 * Created on August 2, 2015, 12:23 PM
 */

#include <cstdlib>
#include "libraries/Onyx/Onyx.h"
using namespace std;
Onyx MK1;
void setup(){
    MK1.setMode(STARTUP);
    
    for(int i=14;i<18;i++){
        pinMode(i,OUTPUT);
    }
    for(int i=14;i<18;i++){
        digitalWrite(i,HIGH);
    }
} 
void loop(){MK1.setMode(STANDBY);}
