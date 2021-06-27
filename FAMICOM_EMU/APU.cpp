#include "main.h"

unsigned char EmuVolume = 25;	

const unsigned char LengthCounter[0x20] = {		//Length Counter
	10,254, 20,  2, 40,  4, 80,  6, 160,  8, 60, 10, 14, 12, 26, 14,		// 0x00 - 0x0f
	12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30			// 0x10 - 0x1f
};

const unsigned char Square_Duty[4] = {	
	0b01000000,	// 12.5%
	0b01100000,	//	25%
	0b01111000, // 50%
	0b10011111	// negate 25%
};

const unsigned char TriangleWave[32] = {
	15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
};

const unsigned short NoisePeriod[16] = {
	4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

const unsigned short DPCMRate[16] = {
	428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106,  84,  72,  54
};

struct APUStat APUStat;

SDL_AudioDeviceID Audio_Device;
SDL_AudioSpec Audio_Spec;/* = {	

Audio_Freq,		//	freq
AUDIO_S16LSB,		//format
2,		//	channels
0,		//silence
2048,		//samples
0,		//size
NULL ,		//callback
NULL		//userdata
};
*/

void Audio_Output(void* userdata, unsigned char* stream, int len);

void Init_SDLAudio(void) {

	SDL_Init(SDL_INIT_AUDIO);
	Audio_Device = SDL_OpenAudioDevice(NULL, 0, &Audio_Spec, NULL, 0);

	
	Audio_Spec.freq = Audio_Freq;		//	freq
	Audio_Spec.format = AUDIO_S16SYS;		//format
	Audio_Spec.channels = 1;		//	channels
	Audio_Spec.silence = 0;		//silence
	Audio_Spec.samples = 2048;		//samples	
	Audio_Spec.size = 0;		//size
	Audio_Spec.callback = Audio_Output;		//callback
	Audio_Spec.userdata = NULL;		//userdata
	

	SDL_OpenAudio(&Audio_Spec, NULL);		
	SDL_PauseAudio(1);
	//SDL_PauseAudioDevice(Audio_Device, 0);

}

void Init_AudioParameters(void) {

	APUStat = {};
	APUStat.CH4_Shift = 1;	
	APUStat.CH5_BitsRemain = 8;

}


void Length_Clock(void);
void Envelope_Clock(void);
void LinearClock(void);
void SweepClock(void);

void Audio_Output(void* userdata, unsigned char* stream, int len) {

	static short CounterUpdateRate = Audio_Spec.freq / 240;

	unsigned char CH1_Duty = (RAM[0x4000] & 0b11000000) >> 6;	
	unsigned char CH2_Duty = (RAM[0x4004] & 0b11000000) >> 6;

	//unsigned short CH1_Timer = (((unsigned short)RAM[0x4003] & 0b00000111) << 8) | RAM[0x4002];
	//unsigned short CH1_Audio_Timer = Audio_Freq * 16 / 8 * (APUStat.CH1_Timer + 1) / NES_CPUFreq;		

	unsigned char CH1_Output, CH2_Output, CH3_Output = 0, CH4_Output, CH5_Output;	

	//static unsigned short CH1_Audio_Timer_counter = CH1_Audio_Timer;	
	//static char OneFrameCounter = 0;		

	len /= sizeof(short);		

	for (int i = 0; i < len; i++) {

		// CH1 output -----------------------
			//Muting is regardless of the sweep enable flag and regardless of whether the sweep divider is not outputting a clock signal. 
		if (RAM[0x4000] & 0b00010000) {	//constant volume
			if (APUStat.CH1_Length_Counter == 0 || APUStat.CH1_Timer < 8 || APUStat.CH1_SweepTarget >= 0x800) {
				CH1_Output = 0;
			}
			else {
				CH1_Output = ((Square_Duty[CH1_Duty] & (1 << APUStat.CH1_shift)) >> APUStat.CH1_shift)* APUStat.CH1_Vol_or_EnvPerCounter;
			}
		}
		else {	//envelope volume
			CH1_Output = ((Square_Duty[CH1_Duty] & (1 << APUStat.CH1_shift)) >> APUStat.CH1_shift)* APUStat.CH1_EnvVol;
		}

	// CH2 output -----------------------
		if (RAM[0x4004] & 0b00010000) {	//constant volume
			if (APUStat.CH2_Length_Counter == 0 || APUStat.CH2_Timer < 8 || APUStat.CH2_SweepTarget >= 0x800) {
				CH2_Output = 0;
			}
			else {
				CH2_Output = ((Square_Duty[CH2_Duty] & (1 << APUStat.CH2_shift)) >> APUStat.CH2_shift)* APUStat.CH2_Vol_or_EnvPerCounter;
			}
		}
		else {	//envelope volume
			CH2_Output = ((Square_Duty[CH2_Duty] & (1 << APUStat.CH2_shift)) >> APUStat.CH2_shift)* APUStat.CH2_EnvVol;
		}

		// CH3 output -----------------------

			/*static int a = 0;
			if (a < 200 && Frame>600) {
				printf("len=%d lin=%d \n", APUStat.CH3_Length_Counter, APUStat.CH3_LinearCounter);
				a++;
			}*/

		if (APUStat.CH3_Length_Counter && APUStat.CH3_LinearCounter) {
			CH3_Output = TriangleWave[APUStat.CH3_TriWave];

		}

		// CH4 output -----------------------
		if (RAM[0x400c] & 0b00010000) {	//constant volume
			if (APUStat.CH4_Length_Counter == 0)
				CH4_Output = 0;
			else
				if (APUStat.CH4_Shift & 0b00000001)
					CH4_Output = 0;
				else
					CH4_Output = APUStat.CH4_Vol_or_EnvPerCounter;	

		}
		else {
			if (APUStat.CH4_Shift & 0b00000001)
				CH4_Output = 0;
			else
				CH4_Output = APUStat.CH4_EnvVol;

		}

		// CH5 output -----------------------
		if (APUStat.CH5_SilenceFlag == 0)	
			CH5_Output = APUStat.CH5_Output;
		else
			CH5_Output = 0;
			
		//CH1_Output = 0;
		//CH2_Output = 0;
		//CH3_Output = 0;
		//CH4_Output = 0;
		//CH5_Output = 0;


	//	Linear Approximation -----------------------		//dmc 0-127 others 0-15
		short Audio_Sample = (short)(((float)(CH1_Output + CH2_Output) * 7.52 + (float)CH3_Output * 8.51
			+ (float)CH4_Output * 4.94 + (float)CH5_Output * 3.35) * EmuVolume);

		*((short*)stream + i) = Audio_Sample;


		// Timer Counter shift ---------------------------

		// CH1 Square1 ------------------------------------------
		if (APUStat.CH1_Audio_Timer_counter == 1) {
			APUStat.CH1_Audio_Timer_counter = Audio_Freq * 16 / 8 * (APUStat.CH1_Timer + 1) / NES_CPUFreq;
			APUStat.CH1_shift = (APUStat.CH1_shift + 1) % 8;
		}
		else
			APUStat.CH1_Audio_Timer_counter -= 1;

		// CH2 Square2 ------------------------------------------
		if (APUStat.CH2_Audio_Timer_counter == 1) {
			APUStat.CH2_Audio_Timer_counter = Audio_Freq * 16 / 8 * (APUStat.CH2_Timer + 1) / NES_CPUFreq;
			APUStat.CH2_shift = (APUStat.CH2_shift + 1) % 8;
		}
		else
			APUStat.CH2_Audio_Timer_counter -= 1;

		// CH3 Triangle ------------------------------------------
		if (APUStat.CH3_Audio_Timer_counter == 1) {
			APUStat.CH3_Audio_Timer_counter = Audio_Freq * (APUStat.CH3_Timer + 1) / NES_CPUFreq;
			APUStat.CH3_TriWave = (APUStat.CH3_TriWave + 1) % 32;
		}
		else
			APUStat.CH3_Audio_Timer_counter -= 1;

		// CH4 Noise ------------------------------------------
		if (APUStat.CH4_NoisePeriod == 1) {	
			APUStat.CH4_NoisePeriod = NoisePeriod[RAM[0x400e] & 0b00001111];
			if (RAM[0x400e] & 0b10000000)
				APUStat.CH4_Shift = (APUStat.CH4_Shift >> 1) |	
				(((APUStat.CH4_Shift & 0x0001) ^ ((APUStat.CH4_Shift & 0x0040) >> 6)) << 14);
			else
				APUStat.CH4_Shift = (APUStat.CH4_Shift >> 1) |	
				(((APUStat.CH4_Shift & 0x0001) ^ ((APUStat.CH4_Shift & 0x0002) >> 1)) << 14);

		}
		else
			APUStat.CH4_NoisePeriod -= 1;

		// CH5 DPCM ------------------------------------------
		if ((--APUStat.CH5_Audio_Timer_counter) == 0) {

			APUStat.CH5_Audio_Timer_counter = DPCMRate[RAM[0x4010] & 0xf] * Audio_Freq / NES_CPUFreq;
	
			if (APUStat.CH5_SilenceFlag == 0) {
				if (APUStat.CH5_Shift & 0x1) {
					if (APUStat.CH5_Output <= 0x7d)
						APUStat.CH5_Output += 2;
				}else {
					if (APUStat.CH5_Output >= 0x2)
						APUStat.CH5_Output -= 2;
				}

				APUStat.CH5_Shift >>= 1;
			}

			if ((--APUStat.CH5_BitsRemain) == 0) {	

				APUStat.CH5_BitsRemain = 8;

				if (APUStat.CH5_BytesRemain) {
					
					APUStat.CH5_Shift = RAM[APUStat.CH5_Address];
					APUStat.CH5_SilenceFlag = 0;	

					if ((++APUStat.CH5_Address) == 0x0000)
						APUStat.CH5_Address = 0x8000;

					//--APUStat.CH5_BytesRemain;

					if ((--APUStat.CH5_BytesRemain) == 0) {
						if (RAM[0x4010] & 0b01000000) {
							APUStat.CH5_Output = RAM[0x4011] & 0x7f;
							APUStat.CH5_Address = 0xc000 + ((unsigned short)RAM[0x4012] << 6);
							APUStat.CH5_BytesRemain = ((unsigned short)RAM[0x4013] << 4) + 1;
						}
						else if (RAM[0x4010] & 0b10000000)
							RAM[0x4015] |= 0b10000000;

					}

				}else 
					APUStat.CH5_SilenceFlag = 1;	
			}
		}

		// Clocks -----------------------------------------	
		static unsigned int counter = 1;		

		if ((APUStat.OneFrameCounter != 4) && (counter % CounterUpdateRate == 0)) {	

			APUStat.OneFrameCounter += 1;

			if (RAM[0x4017] & 0b10000000) {

				//mode5
				switch ((APUStat.ModeFrameCounter = (APUStat.ModeFrameCounter + 1) % 20) % 5) {
					
				case 1: case 3:

					Envelope_Clock();
					LinearClock();

					break;
				case 2: case 0:

					Length_Clock();
					Envelope_Clock();
					LinearClock();
					SweepClock();

					break;

				case 4:

					break;

				}

			}
			else {
				//mode4	

				switch ((APUStat.ModeFrameCounter = (APUStat.ModeFrameCounter + 1) % 20) % 4) {	
				case 1: case 3:

					Envelope_Clock();
					LinearClock();

					break;
				case 2:

					Length_Clock();
					Envelope_Clock();
					LinearClock();
					SweepClock();

					break;
				case 0:

					Length_Clock();
					Envelope_Clock();
					LinearClock();
					SweepClock();

					if ((RAM[0x4017] & 0b01000000) == 0) {}// IRQ


					break;

				}

			}

		}

		if (APUStat.OneFrameCounter < 4)
			counter++;
		else if (APUStat.OneFrameCounter == 4)	
			counter = 1;

	}




}

void Length_Clock(void) {

	// CH1 Length Clock -------------------------------
	if ((RAM[0x4015] & 0b00000001) && APUStat.CH1_Length_Counter && (RAM[0x4000] & 0b00100000) == 0)
		APUStat.CH1_Length_Counter -= 1;

	// CH2 Length Clock -------------------------------
	if ((RAM[0x4015] & 0b00000010) && APUStat.CH2_Length_Counter && (RAM[0x4004] & 0b00100000) == 0)
		APUStat.CH2_Length_Counter -= 1;

	// CH3 Length Clock -------------------------------
	if ((RAM[0x4015] & 0b00000100) && APUStat.CH3_Length_Counter && (RAM[0x4008] & 0b10000000) == 0)
		APUStat.CH3_Length_Counter -= 1;

	// CH4 Length Clock -------------------------------
	if ((RAM[0x4015] & 0b00001000) && APUStat.CH4_Length_Counter && (RAM[0x400c] & 0b00100000) == 0)
		APUStat.CH4_Length_Counter -= 1;


}

void Envelope_Clock(void) {

	// CH1 Envelope Clock ------------------------------
	if (APUStat.CH1_Vol_or_EnvPerCounter)
		APUStat.CH1_Vol_or_EnvPerCounter -= 1;
	else if (APUStat.CH1_EnvVol) {
		APUStat.CH1_EnvVol -= 1;
		APUStat.CH1_Vol_or_EnvPerCounter = RAM[0x4000] & 0b00001111;
	}
	else if (RAM[0x4000] & 0b00100000) {
		APUStat.CH1_EnvVol = 0xf;
		APUStat.CH1_Vol_or_EnvPerCounter = RAM[0x4000] & 0b00001111;
	}

	// CH2 Envelope Clock ------------------------------
	if (APUStat.CH2_Vol_or_EnvPerCounter)
		APUStat.CH2_Vol_or_EnvPerCounter -= 1;
	else if (APUStat.CH2_EnvVol) {
		APUStat.CH2_EnvVol -= 1;
		APUStat.CH2_Vol_or_EnvPerCounter = RAM[0x4004] & 0b00001111;
	}
	else if (RAM[0x4004] & 0b00100000) {
		APUStat.CH2_EnvVol = 0xf;
		APUStat.CH2_Vol_or_EnvPerCounter = RAM[0x4004] & 0b00001111;
	}

	// CH4 Envelope Clock ------------------------------
	if (APUStat.CH4_Vol_or_EnvPerCounter)
		APUStat.CH4_Vol_or_EnvPerCounter -= 1;
	else if (APUStat.CH4_EnvVol) {
		APUStat.CH4_EnvVol -= 1;
		APUStat.CH4_Vol_or_EnvPerCounter = RAM[0x400c] & 0b00001111;
	}
	else if (RAM[0x400c] & 0b00100000) {
		APUStat.CH4_EnvVol = 0xf;
		APUStat.CH4_Vol_or_EnvPerCounter = RAM[0x400c] & 0b00001111;
	}


}

void LinearClock(void) {	

	if (APUStat.CH3_LinearCounter)
		APUStat.CH3_LinearCounter -= 1;
	else
		if (RAM[0x4008] & 0b10000000) {
			APUStat.CH3_LinearCounter = RAM[0x4008] & 0b01111111;

		}


}

void SweepClock(void) {

	// CH1 sweep clock ----------------------------------
	if (APUStat.CH1_SweepPeriod)
		APUStat.CH1_SweepPeriod -= 1;	
	else {
		APUStat.CH1_SweepPeriod = (RAM[0x4001] & 0b01110000) >> 4;

		if (RAM[0x4001] & 0b10000000) {

			unsigned short temp = APUStat.CH1_Timer >> (RAM[0x4001] & 0b00000111);
			if (RAM[0x4001] & 0b00001000) {
				APUStat.CH1_Timer = (APUStat.CH1_Timer + ~temp) & 0x7ff;	
				APUStat.CH1_SweepTarget = APUStat.CH1_Timer;
			}
			else
				if ((APUStat.CH1_SweepTarget = APUStat.CH1_Timer + temp) < 0x800) {	
					APUStat.CH1_Timer += temp;
				}
		}
	}

	// CH2 sweep clock ----------------------------------
	if (APUStat.CH2_SweepPeriod)	
		APUStat.CH2_SweepPeriod -= 1;	
	else {
		APUStat.CH2_SweepPeriod = (RAM[0x4005] & 0b01110000) >> 4;

		if (RAM[0x4005] & 0b10000000) {

			unsigned short temp = APUStat.CH2_Timer >> (RAM[0x4005] & 0b00000111);
			if (RAM[0x4006] & 0b00001000) {
				APUStat.CH2_Timer = (APUStat.CH2_Timer + ~temp + 1);	
				APUStat.CH2_SweepTarget = APUStat.CH2_Timer;
			}
			else
				if ((APUStat.CH2_SweepTarget = APUStat.CH2_Timer + temp) < 0x800) {	
					APUStat.CH2_Timer += temp;
				}
		}
	}



}