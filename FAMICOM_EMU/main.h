
/*

	[ 紹介(概要) ]

	・APU(audio processing unit) : サウンドを担当する部分。
	　矩形波×2、三角波、ノイズ、DPCMの計5チャンネルを用いて音を出す。

	・PPU(picture processing unit) : 描画を担当。16kBのVRAMやスプライト用のメモリを持つ。
	　毎フレーム262走査線をレンダリングする(そのうちの20はVRAM書き換えのためのアイドル期間)。

	・CPU : 演算処理を担当。MOS6502ベース。6つのレジスタと64kBのメモリ空間を持つ。
	　幾つもの命令・アドレッシングモードが用いられる。


	・その他、

		・Boot_Initializeでは初期化処理を、
		・ControlではSDLライブラリを用いた入力処理を、
		・Dataでは用いる変数や表のデータを(1つの実装ファイルに)、
		・Debugでは自身の用いたチェック処理を、
		・SDL_menubarでは独自のメニューバーを、
	　
	 扱う。


	 ・一部のライブラリのためにファイルはC++形式(cpp)であるが、内容はほぼC言語で書かれている。

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

