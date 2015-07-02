#include "config.h"
<<<<<<< HEAD
SSC32 SSC;
void setup(){ 
=======

void setup(){
    SSC32 SSC; 
>>>>>>> dd7d05d7aaa7e2a4a80c8e5f4a3a6cd263a294d5
        SSC.begin(115200);
        int SafeStartX = 4.5;
        int SafeStartY = 1.0;
            pinMode(servoPower, OUTPUT);
            for(int i=0;i<4;i++){
                LegUp[i] = 1;
            }
                 digitalWrite(servoPower,1);
                 delay(1000);
                 BodyX = SafeStartX;
                 BodyY = SafeStartY; 
                    SSC.newGroup(MOVE);
                    //IK(BodyX, BodyY);
                        for(int i=0;i<12;i++){ 
                            if(SSC.servoMoveTime(ServoList[i], 1500,1000) == false)Serial.print("Error");
                        }
                   if(SSC.executeGroup() == false)Serial.print("Error");
                    
}
void loop(){
<<<<<<< HEAD
switch (Serial.read()){
    case 'q':
    LegUp[0]=1;
    break;
    case 'a':
    LegUp[0]=0;
    break;
    case 'w':
    LegUp[1]=1;
    break;
    case 's':
    LegUp[1]=0;
    break;
    case 'e':
    LegUp[2]=1;
    break;
    case 'd':
    LegUp[2]=0;
    break;
    case 'r':
    LegUp[3]=1;
    break;
    case 'f':
    LegUp[3]=0;
    break;
    
}


    
    for(int i=0;i<4;i++){
            if(LegUp[i]==1){
            SSC.newGroup(MOVE);
            int p = i;
            p=p*3;
            IK(4,1);
                if(p+1 >= 4 && p+1 <= 11){
                    if(SSC.servoMoveTime(ServoList[p+1], pulse1,1000) == false)Serial.print("Error");
                    if(SSC.servoMoveTime(ServoList[p+2], pulse2,1000) == false)Serial.print("Error");
                }
                else{
                    if(SSC.servoMoveTime(ServoList[p+1], pulse1invert,1000) == false)Serial.print("Error");
                    if(SSC.servoMoveTime(ServoList[p+2], pulse2invert,1000) == false)Serial.print("Error");  
                }
            }
            
            if(LegUp[i]==0){
            SSC.newGroup(MOVE);
            int p = i;
            p=p*3;
            IK(l1,l2);
                if(p+1 >= 4 && p+1 <= 11){
                    if(SSC.servoMoveTime(ServoList[p+1], 1500,1000) == false)Serial.print("Error");
                    if(SSC.servoMoveTime(ServoList[p+2], 1500,1000) == false)Serial.print("Error");
                }
            }
        
    }
    
    if(SSC.executeGroup() == false)Serial.print("Error");

    
}

=======
    
}
>>>>>>> dd7d05d7aaa7e2a4a80c8e5f4a3a6cd263a294d5
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

void updatePosistion(int index){ //Updates the ServoPosistion array with pulse data for servos.  // FrontEnd for IK
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
<<<<<<< HEAD
            pulse1=(deg1*10)-1500;//offset to mechanical center (0 degrees IK) 
            pulse2invert=(deg2*10)-600;//offset to mechanical center (90 degrees IK)
=======
            pulse1=(deg1*10)+1500;//offset to mechanical center (0 degrees IK) 
            pulse2invert=(deg2*10)+600;//offset to mechanical center (90 degrees IK)
>>>>>>> dd7d05d7aaa7e2a4a80c8e5f4a3a6cd263a294d5
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
