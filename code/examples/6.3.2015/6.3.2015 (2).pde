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
int foot2knee = 106; //mm == 4.2in
int knee2knee = 56; //mm == 2.2in
//useful
int hip2knee = 28.5; //mm == 1.12in




//---Servos: 12 x HS-5645MG (~750us - ~2200us)------
int ServoList[12] =           {0,    1,    2,    12,   13,   14,   16,   17,   18,   28,   29,   30};
int ServoCenters[12] =        {1800, 1900, 1250, 1200, 1100, 1750, 1200, 1100, 1750, 1800, 1900, 1250};
int Servo_OUT_UP_Limit[12] =  {2200, 750,  700,  750,  200,  750,  750,  2200, 750,  2200, 750,  2200};
int Servo_IN_DOWN_Limit[12] = {1000, 2000, 2200, 1500, 1100, 2200, 2000, 1100, 2200, 1500, 1900, 750};
int ServoGroup[12] =          {1,    1,    1,    2,    2,    2,    3,    3,    3,    4,    4,    4,};
int InversionBit[12] =        {1,    0,    0,    0,    0,    0,    0,    1,    0,    1,    0,    1,}; //If 1: Counts from 2200 down to 750 (Inverted) ||| If 0: Counts from 750 up to 2200 (Non-Inverted)

////Populated on startup, 12 Servo 3 posistion frame arrays:

int ServoPosistion[12]; //legacy 

int ServoPrevious[12];
int ServoCurrent[12]; 
int ServoNext[12]; 

//byte* SSC32MATRIX[12][7]; //Precompiled SSC-32 Commands packaged as 12x7 matrix
//Limits:
// #  (OUT/UP, IN/DOWN) (CENTER)
// 0  (O2200, I1000) (1800)
// 1  (U750, D2000)  (1900)-
// 2  (U700, D2200)  (1250)

// 12 (O750, I1500)  (1200)
// 13 (U2200, D1100) (1100)-
// 14 (U750, D2200)  (1750)

// 16 (O750, I2000)  (1200)
// 17 (U2200, D1100) (1100)-
// 18 (U750, D2200)  (1750)

// 28 (O2200, I1500) (1800)
// 29 (U750, D1900)  (1900)-
// 30 (U2200, D750)  (1250)

//Modes (Set in initialize function):
//1 --- Leg Power Disabled
//2 --- Leg Power Enabled
//3 --- Inverse Kinematic Leg Test (Cycle Legs)
#include "math.h"

const float pi = 3.14159;

 int servoPower = 27;
 int pm = 500;


void setup() {
   initialize(1, 115200);
}

void loop() {
    switch(Serial.read()){

     case ' ':
     executeMove(2);
     break;
     
     case 'w':
     digitalWrite(servoPower,1);
     break;
     case 's':
     digitalWrite(servoPower,0);
     break;
     case 'q':
     pm += 50;
     for(int i=0;i<12;i++){
       updatePosistion(i, pm);    
     }
       executeMove(1);
     break;
     case 'e':
     pm -= 50;
     for(int i=0;i<12;i++){
       updatePosistion(i, pm);
     }  
       executeMove(1);
     break;
}
  }
  

void initialize(int mode, int baud){ //takes mode # and communication baud rate to begin and control following movements //setup style preference 
    Serial.begin(baud);
    pinMode(servoPower, OUTPUT);
    
      if(mode == 1){
    	  digitalWrite(servoPower,0);
          executeMove(1); //Calls centering mode of movement function
          delay(1000);
      }

	 if(mode == 2){
	    	digitalWrite(servoPower,1);
	        executeMove(2); //Calls centering mode of movement function
	        delay(1000);
	    }
	                          
	}

/*void raiseLeg(int group,int factor, int cor){ //Raises and lowers legs, takes group number, rasies it by a factor(255-full down---->0-full rasied, can be used to control single group(cor=0) or the corresponding walking group(cor=1)
//Groups 1,4 750us-->2000us:0-->255 y=4.902x+750
//Groups 2,3 2200us-->1100us:0-->255 y=-4.31x+2200
//ServoList[12] = {0, 1, 2, 12, 13, 14, 16, 17, 18, 28, 29, 30};
if(group==1){
  float pulse = factor*4.9+750;
  int pulse2 = 1800-pulse;
groupSet(1,pulse);
groupSet(2,pulse-700); 
groupMove(100);
}
if(group==2){

}
if(group==3){
for(int i=0;i<3;i++)groupSet(ServoList[i], factor*5+750);
groupMove(100);
}
if(group==4){

}
  
}
*/

void updatePosistion(int index, int permille){ //Updates the ServoPosistion array with pulse data for servos. Execute move with executeMove() function.
int center = ServoCenters[index];
int maxLimit = Servo_OUT_UP_Limit[index];
int minLimit = Servo_IN_DOWN_Limit[index];
int group = ServoGroup[index];
int invert = InversionBit[index];
float Pulse;

	if(permille>1000)pm=1000;
	if(permille<0)pm=0;
	
	  if(invert == 0) Pulse = ((minLimit-maxLimit)/(0-1000)*permille) + minLimit;
	  if(invert == 1) Pulse = ((minLimit-maxLimit)/(0-1000)*permille) + minLimit;

	  

	//  ServoPrevious =;
	 // ServoCurrent[index] =;	
	  ServoNext[index] = Pulse;  
	
}


void executeMove(int mode){

	  if(mode==1){
	   for(int i=0;i<12;i++){
	   	
	  	Serial.print("#");
	    Serial.print(ServoList[i]);
	    Serial.print(" P");
	    Serial.print(ServoNext[i]);
	   }
	    Serial.print(" T");
	    Serial.println(100);
	    	for(int i=0;i<12;i++){
	    		ServoCurrent[i] = ServoNext[i];
	    	}
	    }
	    
	  

	  if(mode == 2){
	   for(int i=0;i<12;i++){
	  	Serial.print("#");
	    Serial.print(ServoList[i]);
	    Serial.print(" P");
	    Serial.print(ServoCenters[i]);
	   }
	    Serial.print(" T");
	    Serial.println(100);
	    
	  }
	  
	  }



void IK(float x, float y, float l1, float l2){ //Inverse Kinematic equation

float c2, s2, k1, k2;
c2=(x*x+y*y-l1*l1-l2*l2)/(2*l1*l2);
s2=sqrt(1-c2*c2);
k1=l1+l2*c2;
k2=l2*s2;
float theta1=atan2(y,x)-atan2(k2,k1);
float theta2=atan2(s2,c2);
    float deg1=theta1*180/pi; //conversion to degrees using new variable
    float deg2=theta2*180/pi;
   
   /*
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    Serial.print("T1=");
    Serial.println(theta1);
    Serial.print("Deg1=");
    Serial.println(deg1);
    Serial.print("T2=");
    Serial.println(theta2);
    Serial.print("Deg2=");
    Serial.println(deg2);
   */
} 


