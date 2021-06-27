#pragma warning ( disable : 4996 )
#include "main.h"

void GetFileDirAndName(void);

void Boot_Emu(void) {

// SDL window create -----------------------------
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256 * PIXWIDTH, MenuBarHeight + 240 * PIXHEIGHT, SDL_WINDOW_SHOWN);
	ScreenSurface = SDL_GetWindowSurface(window);

//	SDL Audio
	Init_SDLAudio();

	
// Buffer ----------------------------------------
	ROM = (unsigned char*)malloc(sizeof(unsigned char) * 0x20000);	//13MB ÉeÉLÉgÅ[
	RAM = (unsigned char*)malloc(sizeof(unsigned char) * 0x10000);
	VRAM = (unsigned char*)malloc(sizeof(unsigned char) * 0x4000);		//16KBï™


	Pre_StartupScreen();

}


char Boot_Game(void) {


// Load iNES file to ROM Buff -----------------------------------------

	FILE* NESROM_sfile = fopen(OpenFileName.lpstrFile, "rb");

	int temp, counter;


// Check File Info ----------------------

	for (counter = 0; counter < 0x10; counter++) {	

		if ((temp = fgetc(NESROM_sfile)) == EOF)
			exit(1);
	
		switch (counter) {
		case 0: case 1: case 2:
			if ((char)temp != Rom_Info.iNES_check[counter]) {
				fclose(NESROM_sfile);
				return 0;
			}
			else if (counter == 2) {

				GetFileDirAndName();
				ScreenModeChange(1);

			}
			break;
		case 4:
			Rom_Info.PRGSize = temp;
			break;
		case 5:
			Rom_Info.CHRSize = temp;
			break;
		case 6:
			if (temp & 0b00000001)
				Rom_Info.NTMirroring = 1;	// Vertical
			else
				Rom_Info.NTMirroring = 0;	// horizonal


		}
	}


// ROM ÇÃì«Ç›çûÇ› --------------------------

	counter = 0;
	while ((temp = fgetc(NESROM_sfile)) != EOF) {
		ROM[counter] = (unsigned char)temp;	//ROMÇÕ0x8000ÇÊÇËCPUROMÇ…é˚î[Ç≥ÇÍÇÈ
		counter++;
	}

	fclose(NESROM_sfile);	

	return 1;

}

void GetFileDirAndName(void){

// dir
	Rom_Info.Rom_Dir = OpenFileName.lpstrFile;

// title name
	int i = 0;

	while ((Rom_Info.File_Name[i] = OpenFileName.lpstrFileTitle[i]) != '.')
		i++;

	Rom_Info.File_Name[i] = '\0';
	
	//printf("%s\n", Rom_Info.File_Name);
}

void LoadROM_PRG(void) {

	int  counter, PRGrange = 0x4000 * Rom_Info.PRGSize;

	for (counter = 0; counter < PRGrange; counter++) {		
		RAM[0x8000 + counter] = ROM[counter];	//ROMÇÕ0x8000ÇÊÇËCPUROMÇ…é˚î[Ç≥ÇÍÇÈ
	}

}

void LoadROM_CHR(void) {

	int  counter, PRGrange = 0x4000 * Rom_Info.PRGSize, CHRrange = 0x2000 * Rom_Info.CHRSize;

	for (counter = 0; counter < CHRrange; counter++) {	
		VRAM[counter] = ROM[PRGrange + counter];	//ROMÇÕ0x8000ÇÊÇËCPUROMÇ…é˚î[Ç≥ÇÍÇÈ
	}

}

//-----------------------------------------
void Initialize(void) {

// Load ROM to RAM VRAM   
	
	LoadROM_PRG();
	LoadROM_CHR();


	// Initialize Registers
	Registers.A = 0;
	Registers.X = 0;
	Registers.Y = 0;
	Registers.S = 0xfd;		
	Registers.P = 0b00000100;		
	Registers.PC = (RAM[0xfffd] << 8) | RAM[0xfffc];


	// Initialize RAM(0x00 to 0x2007)
	int i;
	for (i = 0; i <= 0x4017; i++)
		RAM[i] = 0;
	
	RAM[0x2002] = 0b10100000;

	/*// Read pattern Tables to PPU (8KB)
	for (i = 0; i < 0x2000; i++)
		PPURAMBuffStart_suchar[i] = CPURAMBuffStart_suchar[0xc000 + i];		//0xc00=0x8000+0x4000
		*/

//Initialize PPU Render States
	Rendering.Scanline = 0;	
	Rendering.Dot = 1;
	Rendering.Oddframeflag = 0;
	Rendering.PPUClocks = 0;
	
//Initialize PPU Registers
	PPURegisters.VRAMaddress = 0;
	PPURegisters.TempVRAMaddress = 0;
	PPURegisters.FineXScroll = 0;
	PPURegisters.WriteToggle = 0;
	
	PPURegisters.BGPattern_Shift0 = 0;
	PPURegisters.BGPattern_Shift1 = 0;
	PPURegisters.BGAttribute_Shift0 = 0;
	PPURegisters.BGAttribute_Shift1 = 0;	

	PPURegisters.Internal2007ReadBuff = 0;


//Initialize Controller Registers
	/*int m;
	for (m = 0; m < 8; m++) Controller.Button[m] = 0;
	Controller.Button_num = 0;
*/

	Init_ControllerParameters();
	Init_AudioParameters();

}
