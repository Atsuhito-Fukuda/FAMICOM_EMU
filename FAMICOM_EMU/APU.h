
/*

	ÅEAPU(audio processing unit)

*/

#define NES_CPUFreq 1789773
#define Audio_Freq 44160	


struct APUStat {	

	char ModeFrameCounter;	
	char OneFrameCounter;

	unsigned short CH1_Audio_Timer_counter;		
	unsigned char CH1_Length_Counter;
	unsigned char CH1_Duty;
	unsigned short CH1_Timer;
	unsigned char CH1_EnvVol;
	unsigned char CH1_Vol_or_EnvPerCounter;
	unsigned char CH1_shift; //0 - 7
	unsigned char CH1_SweepPeriod;
	unsigned short CH1_SweepTarget;	//Whenever the current period changes for any reason, whether by $400x writes or by sweep, the target period also changes. 

	unsigned short CH2_Audio_Timer_counter;	
	unsigned char CH2_Length_Counter;
	unsigned char CH2_Duty;
	unsigned short CH2_Timer;
	unsigned char CH2_EnvVol;	
	unsigned char CH2_Vol_or_EnvPerCounter;
	unsigned char CH2_shift; //0 - 7
	unsigned char CH2_SweepPeriod;
	unsigned short CH2_SweepTarget;	//Whenever the current period changes for any reason, whether by $400x writes or by sweep, the target period also changes. 

	unsigned char CH3_TriWave;	
	unsigned short CH3_Audio_Timer_counter;	
	unsigned char CH3_Length_Counter;
	unsigned short CH3_Timer;
	unsigned char CH3_LinearCounter;

	unsigned char CH4_Vol_or_EnvPerCounter;
	unsigned char CH4_NoisePeriod;
	unsigned char CH4_Length_Counter;
	unsigned short CH4_Shift;
	unsigned char CH4_EnvVol;

	unsigned short CH5_Audio_Timer_counter;
	unsigned char CH5_Output;
	unsigned short CH5_Address;
	unsigned char CH5_Shift;
	unsigned short CH5_BytesRemain;
	unsigned char CH5_BitsRemain;
	unsigned char CH5_SilenceFlag;

};