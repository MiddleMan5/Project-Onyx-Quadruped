#define baudrate 115200 		//Symbols per second. Default hardware set 115200
#define sscReturn 0	//Is SSC32 Connected bi-directionally? (default "0" = assume connected)

//IK Leg Measurments in Inches       
	#define l1 2.2 //in == 56mm
	#define l2 4.2 //in == 106mm
	#define hipSize 1.12//in == 28.5mm

//Servo-Power Relay Pin
	#define servoPower 27
		//Leg Indicator LED Pins
			#define FL 35
			#define FR 34
			#define RR 8
			#define RL 9
    
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------//
//Walking Gait Variables
	//Walk speed in resolution per stepTime
		#define resolution 100
		#define stepTime 1000

//----------------------------------------------------------------------------------------//

