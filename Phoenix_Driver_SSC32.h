//==============================================================================
//==============================================================================
// Define the class(s) for Servo Drivers.
//==============================================================================
//==============================================================================
#ifndef __DRV_SSC32_H_
#define __DRV_SSC32_H_

class ServoDriver {
  public:
    void Init(void);

    uint16_t GetBatteryVoltage(void);

  #ifdef OPT_GPPLAYER    
    inline boolean  FIsGPEnabled(void) {
      return _fGPEnabled;
    };
    boolean         FIsGPSeqDefined(uint8_t iSeq);
    inline boolean  FIsGPSeqActive(void) {
      return _fGPActive;
    };
    void            GPStartSeq(uint8_t iSeq);  // 0xff - says to abort...
    void            GPPlayer(void);
    uint8_t         GPNumSteps(void);          // How many steps does the current sequence have
    uint8_t         GPCurStep(void);           // Return which step currently on... 
    void            GPSetSpeedMultiplyer(short sm) ;      // Set the Speed multiplier (100 is default)
  #endif
    void 			  BeginServoUpdate(void);    // Start the update 
    void            OutputServoInfoForLeg(byte LegIndex, short sCoxaAngle1, short sFemurAngle1, short sTibiaAngle1);
    void            CommitServoDriver(word wMoveTime);
    void            FreeServos(void);

    void            IdleTime(void);        // called when the main loop when the robot is not on

      // Allow for background process to happen...
  #ifdef OPT_BACKGROUND_PROCESS
    void            BackgroundProcess(void);
  #endif    
      
  #ifdef OPT_TERMINAL_MONITOR  
    void            ShowTerminalCommandList(void);
    boolean         ProcessTerminalCommand(byte *psz, byte bLen);
  #endif

  private:

  #ifdef OPT_GPPLAYER    
    boolean _fGPEnabled;     // IS GP defined for this servo driver?
    boolean _fGPActive;      // Is a sequence currently active - May change later when we integrate in sequence timing adjustment code
    uint8_t    _iSeq;        // current sequence we are running
      short    _sGPSM;        // Speed multiplier +-200 
  #endif

};

#endif