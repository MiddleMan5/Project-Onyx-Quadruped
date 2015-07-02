//                 ONYX
//                  __
//Rear Right:4 |   |SC|   |Rear Left:2           
//             |___|32|___|
//                  ||
//              __|wMPU|__
//             |          |
//Front Right:3|          |Front Left:1
//
//Servo Groups:
//  Group Name (HIP,KNEE,ANKLE):GROUP #
//  Front Left (0,1,2):1
//   Rear Left (12,13,14):2
// Front Right (16,17,18):3
//  Rear Right (28,29,30):4

//Leg Measurements
//Diagram:  

//(x4)
/*[S]-H2K-[S] 
           |
          K2K
           |
          [S]
           |
          F2K
           |
         __ ___
*/
//important for IK       
float knee2knee = 2.2; //in == 56mm
float l1 = knee2knee;
float foot2knee = 4.2; //in == 106mm
float l2 = foot2knee;
//useful
float hip2knee = 1.12;//in == 28.5mm

//Quadruped leg naming scheme:
//coxa-femur-tibia-foot/claw



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

#include "math.h"
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

    int FL =35;
    int FR =34;
    int RR =8;
    int RL =9;
    
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
void setup(){
   initialize(1, 115200);
   delay(3000);
   ind = 3;
}
void loop(){
    
}
void rotateServo(){
        for(int i=0;i<4;i++){
        ServoNext[i*3] = CoxaRotation[i];    
        Serial.print("#");
        Serial.print(ServoList[i]);
        Serial.print("P");
        Serial.print(ServoNext[i*3]);
       }
        Serial.print("T");
        Serial.println(700);        
      }   
      
void initialize(int mode, int baud){ //takes mode # and communication baud rate to begin and control following movements //setup style preference 
    Serial.begin(baud);
    pinMode(FL,OUTPUT);
    pinMode(FR,OUTPUT);
    pinMode(RR,OUTPUT);
    pinMode(RL,OUTPUT);
    int SafeStartX = 4.5;
    int SafeStartY = 1.0;
    pinMode(servoPower, OUTPUT);
    for(int i=0;i<4;i++){
        LegUp[i] = 1;
    }
    
      if(mode == 1){
         digitalWrite(servoPower,1);
         BodyX = SafeStartX;
         BodyY = SafeStartY;
         executeMove(2);
      }
                             
    }

void raiseLeg(int leg){
    if(leg<4){
        LegUp[leg]=1;
        for(int i=0;i<4;i++){
    if(leg!=i){
        GroupX[i]=l1; 
        GroupY[i]=l2;
        LegUp[i]=0;
        updatePosistion(i);
    }
}
    GroupX[leg] = 4;
    GroupY[leg] = 1.0;
    updatePosistion(leg);
    }
}

void updatePosistion(int index){ //Updates the ServoPosistion array with pulse data for servos. Execute move with executeMove() function. // FrontEnd for IK
    //int center = ServoCenters[index];
    //int maxLimit = Servo_OUT_UP_Limit[index];
    //int minLimit = Servo_IN_DOWN_Limit[index];
    //int invert = InversionBit[index];


    if(index != 4)IK(GroupX[index],GroupY[index]);
    if(index == 0){
     digitalWrite(FL, 1);
     digitalWrite(FR, 0);
     digitalWrite(RL, 0);
     digitalWrite(RR, 0);
   // ServoNext[0] = rot;
    ServoNext[1] = pulse1;
    ServoNext[2] = pulse2;
    }
    if(index == 1){
     digitalWrite(FL, 0);
     digitalWrite(FR, 1);
     digitalWrite(RL, 0);
     digitalWrite(RR, 0);
    //ServoNext[3] = rot;
    ServoNext[4] = pulse1invert;
    ServoNext[5] = pulse2invert;
    }
    if(index == 2){
     digitalWrite(FL, 0);
     digitalWrite(FR, 0);
     digitalWrite(RL, 1);
     digitalWrite(RR, 0);
   // ServoNext[6] = rot;
    ServoNext[7] = pulse1invert;
    ServoNext[8] = pulse2invert;
    }
    if(index == 3){
     digitalWrite(FL, 0);
     digitalWrite(FR, 0);
     digitalWrite(RL, 0);
     digitalWrite(RR, 1);
   // ServoNext[9] = rot;
    ServoNext[10] = pulse1;
    ServoNext[11] = pulse2;
    }
    if(index == 4){
     digitalWrite(FL, 1);
     digitalWrite(FR, 1);
     digitalWrite(RL, 1);
     digitalWrite(RR, 1);
        for(int i=0;i<4;i++){
            GroupX[i]=BodyX;
            GroupY[i]=BodyY;
        }
        
   
    IK(BodyX,BodyY);
    ServoNext[0] = rot;
    ServoNext[1] = pulse1;
    ServoNext[2] = pulse2;
    ServoNext[3] = rot;
    ServoNext[4] = pulse1invert;
    ServoNext[5] = pulse2invert;
    ServoNext[6] = rot;
    ServoNext[7] = pulse1invert;
    ServoNext[8] = pulse2invert;
    ServoNext[9] = rot;
    ServoNext[10] = pulse1;
    ServoNext[11] = pulse2;
    }
    /* //Condensed Servo Identifiers
            for(int i=0;int i < 12;i++){
            if (index != 0 || index != 3 || index != 6 || index != 9){ //IK Joints
                
                
            }
                      if (index == 0 || index == 3 || index == 6 || index == 9){ //Rotation joints, GroupX GroupY refresh.
                        int i = (index/3);
                        nextGroupX[i] = X;
                        nextGroupY[i] = Y;
                      }
*/
executeMove(1);
}

void executeMove(int mode){

      if(mode==1){
        delay(20);
       for(int i=0;i<12;i++){
        Serial.print("#");
        Serial.print(ServoList[i]);
        Serial.print("P");
        Serial.print(ServoNext[i]);
       }
        Serial.print("T");
        Serial.println(700);        
      }    
      
    if(mode == 2){
        
        //updatePosistion(4);
//Serial.println(BodyX);
//Serial.println(BodyY);
delay(20);
IK(BodyX,BodyY);
    ServoNext[0] = rot;
    ServoNext[1] = pulse1;
    ServoNext[2] = pulse2;
    ServoNext[3] = rot;
    ServoNext[4] = pulse1invert;
    ServoNext[5] = pulse2invert;
    ServoNext[6] = rot;
    ServoNext[7] = pulse1invert;
    ServoNext[8] = pulse2invert;
    ServoNext[9] = rot;
    ServoNext[10] = pulse1;
    ServoNext[11] = pulse2;


       
        for(int i=0;i<12;i++){
        Serial.print("#");
        Serial.print(ServoList[i]);
        Serial.print("P");
        Serial.print(ServoNext[i]);
       }
        Serial.print("T");
        Serial.println(1000); 
        delay(1000);
       for(int i=0;i<12;i++){
        Serial.print("#");
        Serial.print(ServoList[i]);
        Serial.print("P");
        Serial.print(1500);
       }
       
        Serial.print("T");
        Serial.print(2000);
        Serial.println();
        for(int i=0;i<4;i++){
            GroupX[i]=l1;
            GroupY[i]=l2;
        }
            BodyX=l1;
            BodyY=l2;
        
      }
}

float IK(float x, float y){ //Inverse Kinematic routine

    float c2, s2, k1, k2;
        c2=((x*x)+(y*y)-(l1*l1)-(l2*l2))/(2*l1*l2);
        s2=sqrt(1-c2*c2);
        k1=l1+l2*c2;
        k2=l2*s2;
        float theta1=atan2(y,x)-atan2(k2,k1);
        float theta2=atan2(s2,c2);
            float deg1=theta1*180/pi; //conversion to degrees using new variable
            float deg2=theta2*180/pi;
           //IK Global Variable updates
            pulse1=(deg1*10)+1500;//offset to mechanical center (0 degrees IK) 
            pulse2invert=(deg2*10)+600;//offset to mechanical center (90 degrees IK)
            pulse1invert = map(pulse1,750, 2200,2200,750);
            pulse2 = map(pulse2invert, 750, 2200,2200,750);
          
         
        //1500 = 0*:G1S1
        //1500 = 90*:G1S2


           
           /* //IK Serial Monitor
            Serial.println(" ");
            Serial.println(" ");
            Serial.println(" ");
             Serial.print("P1=");
            Serial.println(pulse1);
            Serial.print("P2=");
            Serial.println(pulse2);
            Serial.print("T1=");
            Serial.println(theta1);
            Serial.print("Deg1=");
            Serial.println(deg1);
            Serial.print("T2=");
            Serial.println(theta2);
            Serial.print("Deg2=");
            Serial.println(deg2);
            Serial.println(" ");
            Serial.println(" ");
            Serial.println(x);
            Serial.print(y);
            //Test Routines:
                    float PulsetoDegree(int pulse){ //Test to convert PWM signal to Degree readout
                    //HS5645MG: 10us = 1 degree
                        return(pulse / 10); 
                    }
                    
                    float DegreetoPulse(int degree){ //Test to convert degree output to PWM signal
                        return(degree * 10);
                    }

            
        */
        
    } 
