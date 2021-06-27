
#include <stdio.h> 
#include <stdlib.h>
#include "CPU.h"
#include "PPU.h"
#include "Control.h"
#include "SDL.h"
#include "Rom_Info.h"
#include <windows.h>

// GetFileName ---------------------------
TCHAR iNES_Path[256];		
TCHAR iNES_Title[256];	

OPENFILENAME OpenFileName = {
sizeof(OPENFILENAME),	// DWORD lStructSize;
NULL,	//HWND hwndOwner
0,		//HINSTANCE hInstance;
TEXT("iNES file (*.nes)\0*.nes\0All files (*.*)\0*.*\0"),		//LPCTSTR lpstrFilter;
0,		//LPTSTR lpstrCustomFilter;
0,		//DWORD nMaxCustFilter;
0,		  //DWORD nFilterIndex;
iNES_Path,		  //LPTSTR lpstrFile;
256,	  //DWORD nMaxFile;
iNES_Title,		//LPTSTR lpstrFileTitle;
256,	  //DWORD nMaxFileTitle;
TEXT("\Rom"),  //LPCTSTR lpstrInitialDir;
0,		  //LPCTSTR lpstrTitle;
0,		  //DWORD Flags;
0,		  //WORD nFileOffset;
0,		  //WORD nFileExtension;
0,		  //LPCTSTR lpstrDefExt;
0,		  //LPARAM lCustData;
		  //LPOFNHOOKPROC lpfnHook;
		 //LPCTSTR lpTemplateName;
		//#if (_WIN32_WINNT >= 0x0500)
		//void* pvReserved;
		//DWORD dwReserved;
		//DWORD FlagsEx;
		//#endif // (_WIN32_WINNT >= 0x0500)
};


struct Rom_Info Rom_Info = { {},NULL,0,0,0,0,{ 'N','E','S' } };



// SDL Screen ----------------------------
SDL_Window* window;
SDL_Surface* ScreenSurface;
SDL_Event event;
SDL_Event* Event = &event;  

// ROM RAM VRAM --------------------------

//char NTMirroring;
unsigned char* ROM;
unsigned char* RAM;
unsigned char* VRAM;

// Rendering -------------------------

struct RenderStates Rendering;

// Registers -------------------------

// CPU
struct Registers Registers;
// PPU
struct PPURegisters PPURegisters;
// Controller
struct Controller Controller;


// Table -----------------------------

char AddressingModes_Table[256] =	
{ IMP, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, ACC, NONE, ABS, ABS, ABS, ABS, //00 to 0F
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERX, ZERX, IMP, ABSY, NONE, ABSY, ABSX, ABSX, ABSX, ABSX, //10 to 1F
 ABS, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, ACC, NONE, ABS, ABS, ABS, ABS, //20 to 2F
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERX, ZERX, IMP, ABSY, NONE, ABSY, ABSX, ABSX, ABSX, ABSX, //30 to 3F
 IMP, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, ACC, NONE, ABS, ABS, ABS, ABS, //40 to 4F
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERX, ZERX, IMP, ABSY, NONE, ABSY, ABSX, ABSX, ABSX, ABSX, //50 to 5F
 IMP, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, ACC, NONE, IND, ABS, ABS, ABS, //60 to 6F
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERX, ZERX, IMP, ABSY, NONE, ABSY, ABSX, ABSX, ABSX, ABSX, //70 to 7F
 IMM, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, NONE, IMP, NONE, ABS, ABS, ABS, ABS, //80 to 8F
 REL, INDINX, NONE, NONE, ZERX, ZERX, ZERY, ZERY, IMP, ABSY, IMP, NONE, NONE, ABSX, NONE, NONE, //90 to 9F
 IMM, INXIND, IMM, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, IMP, NONE, ABS, ABS, ABS, ABS, //A0 to AF
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERY, ZERY, IMP, ABSY, IMP, NONE, ABSX, ABSX, ABSY, ABSY, //B0 to BF
 IMM, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, IMP, NONE, ABS, ABS, ABS, ABS, //C0 to CF
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERX, ZERX, IMP, ABSY, NONE, ABSY, ABSX, ABSX, ABSX, ABSX, //D0 to DF
 IMM, INXIND, NONE, INXIND, ZER, ZER, ZER, ZER, IMP, IMM, NONE, IMM, ABS, ABS, ABS, ABS, //E0 to EF
 REL, INDINX, NONE, INDINX, ZERX, ZERX, ZERX, ZERX, IMP, ABSY, NONE, ABSY, ABSX, ABSX, ABSX, ABSX //F0 to FF
};

unsigned char CPUClocks[256] = {
   7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6, //00 -> 0F
   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, //10 -> 1F
   6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6, //20 -> 2F
   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, //30 -> 3F
   6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6, //40 -> 4F
   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, //50 -> 5F
   6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6, //60 -> 6F
   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, //70 -> 7F
   2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 0, 4, 4, 4, 4, //80 -> 8F
   2, 6, 0, 0, 4, 4, 4, 4, 2, 5, 2, 0, 0, 5, 0, 0, //90 -> 9F
   2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 0, 4, 4, 4, 4, //A0 -> AF
   2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 0, 4, 4, 4, 4, //B0 -> BF
   2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, //C0 -> CF
   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, //D0 -> DF
   2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, //E0 -> EF
   2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7 //F0 -> FF
};

unsigned char PageBreakClocks[256] = {	
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //00 -> 0F
  0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, //10 -> 1F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20 -> 2F
  0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, //30 -> 3F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //40 -> 4F
  0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, //50 -> 5F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60 -> 6F
  0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, //70 -> 7F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80 -> 8F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90 -> 9F
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //A0 -> AF
  0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, //B0 -> BF
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //C0 -> CF
  0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, //D0 -> DF
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //E0 -> EF
  0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0 //F0 -> FF
};


// PPU --------------

unsigned int NESColor[64] = {
0x757575, 0x271B8F, 0x0000AB, 0x47009F, 0x8F0077, 0xAB0013, 0xA70000, 0x7F0B00, 0x432F00, 0x004700, 0x005100, 0x003F17, 0x1B3F5F, 0x000000, 0x000000, 0x000000,
0xBCBCBC, 0x0073EF, 0x233BEF, 0x8300F3, 0xBF00BF, 0xE7005B, 0xDB2B00, 0xCB4F0F, 0x8B7300, 0x009700, 0x00AB00, 0x00933B, 0x00838B, 0x000000, 0x000000, 0x000000,
0xFFFFFF, 0x3FBFFF, 0x5F97FF, 0xA78BFD, 0xF77BFF, 0xFF77B7, 0xFF7763, 0xFF9B3B, 0xF3BF3F, 0x83D313, 0x4FDF4B, 0x58F898, 0x00EBDB, 0x000000, 0x000000, 0x000000,
0xFFFFFF, 0xABE7FF, 0xC7D7FF, 0xD7CBFF, 0xFFC7FF, 0xFFC7DB, 0xFFBFB3, 0xFFDBAB, 0xFFE7A3, 0xE3FFA3, 0xABF3BF, 0xB3FFCF, 0x9FFFF3, 0x000000, 0x000000, 0x000000
};
