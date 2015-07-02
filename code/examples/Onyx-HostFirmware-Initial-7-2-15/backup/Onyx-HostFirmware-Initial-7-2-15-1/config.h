#include "SSC32.h"
#include "math.h"

//important for IK       
float knee2knee = 2.2; //in == 56mm
float l1 = knee2knee;
float foot2knee = 4.2; //in == 106mm
float l2 = foot2knee;
//useful
float hip2knee = 1.12;//in == 28.5mm

        //---Servos: 12 x HS-5645MG (~750us - ~2200us)------
        int ServoList[12] =           {0,    1,    2,    12,   13,   14,   16,   17,   18,   28,   29,   30};
        int ServoCenters[12] =        {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500}; //pre-programmer : mechanical offsets optomized, missing software offset/calibration
        int Servo_IN_DOWN_Limit[12] = {1000, 2000, 2200, 1500, 1100, 2200, 2000, 1100, 2200, 1500, 1900, 750};
        int Servo_OUT_UP_Limit[12] =  {2200, 750,  700,  750,  200,  750,  750,  2200, 750,  2200, 750,  2200}; 

        float GroupX[4];
        float GroupY[4];
        float BodyX;
        float BodyY;


////Populated on startup, 12 Servo 3 posistion frame array1s:
 
int ServoNext[12]; 

const float pi = 3.14159;
int servoPower = 27; //Servo-Power Relay
int rot = 1500;
//Global variables created by1 IK
float pulse1;
float pulse2;
    //inverted pulse values
    float pulse1invert;
    float pulse2invert;
    int ind = 4;

    int FL = 35;
    int FR = 34;
    int RR = 8;
    int RL = 9;
    
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//Walking Gait Test Variables


unsigned long time;
unsigned long oldTime = 0;
int interval = 1000;
int trig = 2000;
                    //{1,   2,   3,   4};
int CoxaRotation[4] = {1500,1500,1500,1500};
int CoxaForward[4]  = {1840,1840,1140,1140};
int CoxaBack[4]     = {1140,1140,1840,1840};

int LegUp[4] = {0,0,0,0}; //Leg up = 1 , Leg Grounded = 0; (codewise)
int resolution = 1; //incremental resolution
//----------------------------------------------------------------------------------------//
