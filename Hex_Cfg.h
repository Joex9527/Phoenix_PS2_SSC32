
//====================================================================
// Project Lynxmotion Phoenix
// Description: 
//     This is the hardware configuration file for the Hex Robot.
//     This Header file is specific for Phoenix with 3 DOF
//  
//     This version of the Configuration file is set up to run on the
//     Lynxmotion BotboardDuino board, which is similar to the Arduino Duemilanove
//
//     This version of configuration file assumes that the servos will be controlled
//     by a Lynxmotion Servo controller SSC-32 and the user is using a Lynxmotion 
//     PS2 to control the robot.
//
// Date: March 18, 2012
// Programmer: Kurt (aka KurtE)
//
// NEW IN V2.0
//    - 3 DOF
// V1.0
//    - First Release
//
//    \    /
//     \__/             ___
//  _ _|  |_ _      /\_|XXX|_/\ 
//     |__|        /           \ 
//     /  \ 
//    /    \ 
//
//====================================================================
#ifndef HEX_CFG_PHOENIX3_H
#define HEX_CFG_PHOENIX3_H
//[CONDITIONAL COMPILING] - COMMENT IF NOT WANTED
// Define other optional compnents to be included or not...
#define OPT_TERMINAL_MONITOR  

#ifdef OPT_TERMINAL_MONITOR   // turning off terminal monitor will turn these off as well...
//#define OPT_SSC_FORWARDER  // only useful if terminal monitor is enabled
#define OPT_FIND_SERVO_OFFSETS    // Only useful if terminal monitor is enabled
#endif

#define OPT_GPPLAYER

// Which type of control(s) do you want to compile in
#define DBGSerial         Serial


#define USEPS2

//==================================================================================================================================
//==================================================================================================================================
//==================================================================================================================================
// THex-3
//==================================================================================================================================
#define USE_SSC32
//#define	cSSC_BINARYMODE	1			// Define if your SSC-32 card supports binary mode.

//[SERIAL CONNECTIONS]


// Warning I will undefine some components as the non-megas don't have enough memory...
//#undef OPT_FIND_SERVO_OFFSETS 

#define cSSC_BAUD        38400   //SSC32 BAUD rate

//--------------------------------------------------------------------
//[Botboarduino Pin Numbers]
#define SOUND_PIN    5        // Botboarduino JR pin number
#define PS2_DAT      6        
#define PS2_CMD      7
#define PS2_SEL      8
#define PS2_CLK      9
// If we are using a SSC-32 then:
// If were are running on an Arduino Mega we will use one of the hardware serial port, default to Serial1 above.
// If on Non mega, if the IO pins are set to 0, we will overload the hardware Serial port 
// Else we will user SoftwareSerial to talk to the SSC-32
#define cSSC_OUT     12      	//Output pin for (SSC32 RX) on BotBoard (Yellow)
#define cSSC_IN      13      	//Input pin for (SSC32 TX) on BotBoard (Blue)

//====================================================================
//[SSC PIN NUMBERS]
enum SSC_PIN_NUMBERS {
    cRRCoxaPin ,     //Rear Right leg Hip Horizontal
    cRRFemurPin,     //Rear Right leg Hip Vertical
    cRRTibiaPin,     //Rear Right leg Knee

    cRMCoxaPin ,     //Middle Right leg Hip Horizontal
    cRMFemurPin,     //Middle Right leg Hip Vertical
    cRMTibiaPin,     //Middle Right leg Knee

    cRFCoxaPin ,     //Front Right leg Hip Horizontal
    cRFFemurPin,     //Front Right leg Hip Vertical
    cRFTibiaPin,      //Front Right leg Knee

    cLRCoxaPin ,      //Rear Left leg Hip Horizontal
    cLRFemurPin,      //Rear Left leg Hip Vertical
    cLRTibiaPin,      //Rear Left leg Knee

    cLMCoxaPin ,      //Middle Left leg Hip Horizontal
    cLMFemurPin,      //Middle Left leg Hip Vertical
    cLMTibiaPin,      //Middle Left leg Knee

    cLFCoxaPin ,      //Front Left leg Hip Horizontal
    cLFFemurPin,      //Front Left leg Hip Vertical
    cLFTibiaPin,      //Front Left leg Knee
};

//--------------------------------------------------------------------
//[MIN/MAX ANGLES]
#define cRRCoxaMin1	    -260	//Mechanical limits of the Right Rear Leg, decimals = 1
#define cRRCoxaMax1	     740
#define cRRFemurMin1   -1010
#define cRRFemurMax1	 950
#define cRRTibiaMin1   -1060
#define cRRTibiaMax1	 770

#define cRMCoxaMin1	    -530	//Mechanical limits of the Right Middle Leg, decimals = 1
#define cRMCoxaMax1      530
#define cRMFemurMin1   -1010
#define cRMFemurMax1	 950
#define cRMTibiaMin1   -1060
#define cRMTibiaMax1	 770

#define cRFCoxaMin1	    -580	//Mechanical limits of the Right Front Leg, decimals = 1
#define cRFCoxaMax1	     740
#define cRFFemurMin1   -1010
#define cRFFemurMax1	 950
#define cRFTibiaMin1   -1060
#define cRFTibiaMax1	 770

#define cLRCoxaMin1	    -740	//Mechanical limits of the Left Rear Leg, decimals = 1
#define cLRCoxaMax1	     260
#define cLRFemurMin1	-950
#define cLRFemurMax1	1010
#define cLRTibiaMin1	-770
#define cLRTibiaMax1	1060

#define cLMCoxaMin1	    -530	//Mechanical limits of the Left Middle Leg, decimals = 1
#define cLMCoxaMax1	     530
#define cLMFemurMin1	-950
#define cLMFemurMax1	1010
#define cLMTibiaMin1	-770
#define cLMTibiaMax1	1060

#define cLFCoxaMin1	    -740	//Mechanical limits of the Left Front Leg, decimals = 1
#define cLFCoxaMax1	     580
#define cLFFemurMin1	-950
#define cLFFemurMax1	1010
#define cLFTibiaMin1	-770
#define cLFTibiaMax1	1060


//--------------------------------------------------------------------
//[LEG DIMENSIONS]
//Universal dimensions for each leg in mm
#define cXXCoxaLength     29    // This is for TH3-R legs
#define cXXFemurLength    76
#define cXXTibiaLength    106

#define cRRCoxaLength     cXXCoxaLength	    //Right Rear leg
#define cRRFemurLength    cXXFemurLength
#define cRRTibiaLength    cXXTibiaLength

#define cRMCoxaLength     cXXCoxaLength	    //Right middle leg
#define cRMFemurLength    cXXFemurLength
#define cRMTibiaLength    cXXTibiaLength

#define cRFCoxaLength     cXXCoxaLength	    //Rigth front leg
#define cRFFemurLength    cXXFemurLength
#define cRFTibiaLength    cXXTibiaLength

#define cLRCoxaLength     cXXCoxaLength	    //Left Rear leg
#define cLRFemurLength    cXXFemurLength
#define cLRTibiaLength    cXXTibiaLength

#define cLMCoxaLength     cXXCoxaLength	    //Left middle leg
#define cLMFemurLength    cXXFemurLength
#define cLMTibiaLength    cXXTibiaLength

#define cLFCoxaLength     cXXCoxaLength	    //Left front leg
#define cLFFemurLength    cXXFemurLength
#define cLFTibiaLength    cXXTibiaLength


//--------------------------------------------------------------------
//[BODY DIMENSIONS]
#define cRRCoxaAngle1   -600       //Default Coxa setup angle, decimals = 1
#define cRMCoxaAngle1    0          //Default Coxa setup angle, decimals = 1
#define cRFCoxaAngle1    600        //Default Coxa setup angle, decimals = 1
#define cLRCoxaAngle1    -600       //Default Coxa setup angle, decimals = 1
#define cLMCoxaAngle1    0          //Default Coxa setup angle, decimals = 1
#define cLFCoxaAngle1    600        //Default Coxa setup angle, decimals = 1

#define cRROffsetX 	-43	    //Distance X from center of the body to the Right Rear coxa
#define cRROffsetZ 	82	    //Distance Z from center of the body to the Right Rear coxa
#define cRMOffsetX 	-63	    //Distance X from center of the body to the Right Middle coxa
#define cRMOffsetZ 	0	    //Distance Z from center of the body to the Right Middle coxa
#define cRFOffsetX 	-43	    //Distance X from center of the body to the Right Front coxa
#define cRFOffsetZ 	-82	    //Distance Z from center of the body to the Right Front coxa

#define cLROffsetX 	43	    //Distance X from center of the body to the Left Rear coxa
#define cLROffsetZ 	82	    //Distance Z from center of the body to the Left Rear coxa
#define cLMOffsetX 	63	    //Distance X from center of the body to the Left Middle coxa
#define cLMOffsetZ 	0	    //Distance Z from center of the body to the Left Middle coxa
#define cLFOffsetX 	43	    //Distance X from center of the body to the Left Front coxa
#define cLFOffsetZ 	-82	    //Distance Z from center of the body to the Left Front coxa

//--------------------------------------------------------------------
//[START POSITIONS FEET]
#define cHexInitXZ	 105 
#define CHexInitXZCos60  53        // COS(60) = .5
#define CHexInitXZSin60  91        // sin(60) = .866
#define CHexInitY	 25

#define cRRInitPosX     CHexInitXZCos60      //Start positions of the Right Rear leg
#define cRRInitPosY     CHexInitY
#define cRRInitPosZ     CHexInitXZSin60

#define cRMInitPosX     cHexInitXZ      //Start positions of the Right Middle leg
#define cRMInitPosY     CHexInitY
#define cRMInitPosZ     0

#define cRFInitPosX     CHexInitXZCos60      //Start positions of the Right Front leg
#define cRFInitPosY     CHexInitY
#define cRFInitPosZ     -CHexInitXZSin60

#define cLRInitPosX     CHexInitXZCos60      //Start positions of the Left Rear leg
#define cLRInitPosY     CHexInitY
#define cLRInitPosZ     CHexInitXZSin60

#define cLMInitPosX     cHexInitXZ      //Start positions of the Left Middle leg
#define cLMInitPosY     CHexInitY
#define cLMInitPosZ     0

#define cLFInitPosX     CHexInitXZCos60      //Start positions of the Left Front leg
#define cLFInitPosY     CHexInitY
#define cLFInitPosZ     -CHexInitXZSin60

#endif CFG_HEX_H



