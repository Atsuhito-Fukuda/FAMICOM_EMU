
/*

	[ �Љ�(�T�v) ]

	�EAPU(audio processing unit) : �T�E���h��S�����镔���B
	�@��`�g�~2�A�O�p�g�A�m�C�Y�ADPCM�̌v5�`�����l����p���ĉ����o���B

	�EPPU(picture processing unit) : �`���S���B16kB��VRAM��X�v���C�g�p�̃����������B
	�@���t���[��262�������������_�����O����(���̂�����20��VRAM���������̂��߂̃A�C�h������)�B

	�ECPU : ���Z������S���BMOS6502�x�[�X�B6�̃��W�X�^��64kB�̃�������Ԃ����B
	�@����̖��߁E�A�h���b�V���O���[�h���p������B


	�E���̑��A

		�EBoot_Initialize�ł͏������������A
		�EControl�ł�SDL���C�u������p�������͏������A
		�EData�ł͗p����ϐ���\�̃f�[�^��(1�̎����t�@�C����)�A
		�EDebug�ł͎��g�̗p�����`�F�b�N�������A
		�ESDL_menubar�ł͓Ǝ��̃��j���[�o�[���A
	�@
	 �����B


	 �E�ꕔ�̃��C�u�����̂��߂Ƀt�@�C����C++�`��(cpp)�ł��邪�A���e�͂ق�C����ŏ�����Ă���B

*/

#include <stdio.h> 
#include <stdlib.h>
#include "SDL.h"
#include "CPU.h"
#include "Control.h"
#include "PPU.h"
#include "SDL_menubar.h"
#include "SDL_ttf.h"
#include "Rom_Info.h"
#include "APU.h"
#include <Windows.h>

#include "Debug.h"		



// Extern * * * * * * * * * * * * * * * * * * * * * * * * * *

extern SDL_Window* window;
extern SDL_Surface* ScreenSurface;
extern SDL_Event* Event;

extern struct Rom_Info Rom_Info;

//SDL Audio
extern SDL_AudioSpec Audio_Spec;
extern SDL_AudioDeviceID Audio_Device;

//extern char NTMirroring;
extern unsigned char* ROM;
extern unsigned char* RAM;
extern unsigned char* VRAM;


//extern unsigned char CPUClocksProcessed;
extern struct Registers Registers;


extern struct RenderStates Rendering;
extern struct PPURegisters PPURegisters;

extern struct Controller Controller;

extern struct APUStat APUStat;

// Tables ------------------------
// CPU
extern char AddressingModes_Table[256];
extern unsigned char CPUClocks[256];
extern unsigned char PageBreakClocks[256];

// PPU
extern unsigned int NESColor[64];

//APU
extern const unsigned char LengthCounter[0x20];		
extern const unsigned short NoisePeriod[16];
extern const unsigned short DPCMRate[16];


// ProtoTypes * * * * * * * * * * * * * * * * * * * * * * * * * *

// Boot_Initialize ----------------
void Boot_Emu(void);
char Boot_Game(void);
void Initialize(void);

// CPU ------------------------------
void PUSH(unsigned char value);
unsigned char POP(void);
void FetchAndExecuteOpcode(void);

// PPU ------------------------------
char RenderEnable(void);
void PPURunASprite8Cycle(void);
void PPURunABG8Cycle(void);
void RenderAPixel(void);
char Loadto2ndOMA(void);
void Clear2ndOMA(void);
void NMI(void);
void Horinc(void);
void vt_Vertinc(void);

// Control -----------------------
void Init_ControllerParameters(void);
char ReadSDLInput(void);
void ScreenModeChange(char Branch);

// APU ---------------------------
void Init_SDLAudio(void);
void Init_AudioParameters(void);
//void Audio_Output(void);


// SDL_menubar ----------------------	
extern OPENFILENAME OpenFileName;
void Pre_StartupScreen(void);
void StartupScreen(void);
char MousePointing(int Mouse_x, int Mouse_y);
void Click_ColorChange(char MousePoint);
void ReleaseClick_ColorChange(char MousePoint);
void SwitchtoPlaymode(void);
void SwitchtoStartScreenmode(void);
//void Screen_GetinPause(void);
//void Screen_GetoutPause(void);
#define Screen_GetinPause() Click_ColorChange(2)
#define Screen_GetoutPause() ReleaseClick_ColorChange(2)
void ScreenModeChange(char Branch);

