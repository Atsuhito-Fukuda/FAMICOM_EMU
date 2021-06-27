#include <stdio.h> 
#include <stdlib.h>
#include "SDL.h"
#include "CPU.h"
#include "Control.h"
#include "PPU.h"
#include <windows.h>		//Sleep—p

extern unsigned char* ROM;
extern unsigned char* RAM;
extern unsigned char* VRAM;
extern struct Registers Registers;
extern struct RenderStates Rendering;
extern struct PPURegisters PPURegisters;
extern struct Controller Controller;


// DEBUG ***********************************************************************
#pragma warning(disable:4996)
//#pragma warning(disable:6387)

unsigned int Frame = 0;

//FILE* outputfile = fopen("C:\\Users\\User\\Desktop\\Debug.txt", "w");

#define GO  1
#define GO2  0

int UU = 0;
int cpuaction = 0;
#define ppp 0
unsigned char debug;
unsigned short Debug;
#define COUNT 2

int RR = 0;//”Ä—p

/*
	if(UU>=ppp*13&&UU<(ppp+1)*13)
	{ printf("debug %x\n",debug);
	//if(Opcode==0xa9)printf("A=0x%x\n",Registers.A);
	}
*/
void Debug_pointerOpcode(unsigned char Opcode) {

	static int counter = 0;
	static char toggle = 0;
	static char check = 0;

	if (/*NMIRegisters.Occurred == 1 && NMIRegisters.Output == 1 &&*/ toggle == 0) {
		toggle = 1;
		counter++;
	}
	if (/*NMIRegisters.Occurred == 0 && */toggle == 1)
		toggle = 0;

	if (Registers.PC == 0x8eed) check++;

	if(0)
	if(cpuaction>=23000 && cpuaction<23050)
	if (UU >= ppp * 70 && UU < (ppp + 1) * 70) {

		printf("%d: \nPC=0x%x Opcode=0x%x VRAMaddress 0x%x\n", cpuaction/*UU+ppp*50*/, Registers.PC, Opcode, PPURegisters.VRAMaddress);
		printf("A=0x%x X==0x%x Y=0x%x PC=0x%x S=0x%x P=0x%x \n", Registers.A, Registers.X, Registers.Y, Registers.PC, Registers.S, Registers.P);
		printf("PPU port : [0x2000]=0x%x [0x2001]==0x%x [0x2002]=0x%x [0x2003]=0x%x [0x2004]=0x%x [0x2005]=0x%x [0x2006]=0x%x [0x2007]=0x%x  \n", RAM[0x2000], RAM[0x2001], RAM[0x2002],
			RAM[0x2003], RAM[0x2004], RAM[0x2005], RAM[0x2006], RAM[0x2007]);

		//printf("CurrentVRAMaddress=0x%x [0x8084]=0x%x\n", PPURegisters.CurrentVRAMaddress, CPURAMBuffStart_suchar[0x8084]);

		UU++;

		/*if(UU==1)for(R=0;R<=0xff;R++)
			printf("[0x%x]=0x%x\n",0x3f00+R ,PPURAMBuffStart_suchar[0x3f00 + R]);*/
	}


	cpuaction++;
	//UU++;

/*	if (RR == 1)
		printf("%d\n", RR);

	if (RR == 0) {

		RR++;
}*/
	
}//place CPU fetchexe





void Debug_LookVRAM(void) {

	/*FILE* outputfile = NULL;
	errno_t err = fopen_s(&outputfile, "C:\\Users\\User\\Desktop\\Debug.txt", "w");

	if (!err && outputfile != NULL)
		fclose(outputfile);

	FILE* f = NULL;
	errno_t err;

	err = _tfopen_s(&f, _T("Source.cpp"), _T("rb"));
	if (!err && f != NULL)
		fclose(f);


	long int i;
	for (i = 0; i < 0x02000; i++) {

		if (i % 0x10 == 0)
			fprintf_s(outputfile, "\n");

		fprintf_s(outputfile, "VRAM[0x%x]=0x%x ", i, PPURAMBuffStart_suchar[i]);



	}// place main NMI

	fprintf_s(outputfile, "\ncpuaction=%d ",cpuaction);
	scanf_s("%d\n\n\n", &RR);

	//fclose(outputfile);
	*/

	static char f = 0;
	long int i;
	if (0/*cpuaction==25400*/)
	if(f==31){
		if(0)
		for (i = 0x2000; i < 0x3000; i++) {

			if (i % 0x10 == 0)
				printf("\n");

			printf("[0x%x]=0x%x ", i, VRAM[i]);



		}

		printf("\nFrame %d cpuaction=%d ", Frame, cpuaction);
		scanf_s("%d", &RR);
	}

	f++;

}// place main NMI fetchexe end

void SlowFrame(void) {

	if(0)
	if (Frame >= 33) {
		printf("Frame %d\n", Frame);
		Sleep(2000);
	}

}// main end

void Debug_DetectWhenUpdated(unsigned char Opcode) {

	static char N = 0;
	static unsigned char box[128];

	/*if (PPURAMBuffStart_suchar[0x00] != 0x03) {
		int i;
		for (i = 0; i < N; i++) {
			if (box[i] == Opcode)
				goto Next;
		}

		box[i] = Opcode;
		N++;
		printf("cpuaction : %d Opcode : 0x%x\n", cpuaction, Opcode);
	}

Next:

	printf("\r");*/
if(GO2)
	if (VRAM[0x00] != 0x03) {

		if (N == 0) {
			printf("cpuaction : %d Opcode : 0x%x\n", cpuaction, Opcode);
			printf("A : 0x%x VRAM address : 0x%x\n", Registers.A,PPURegisters.VRAMaddress);
			N++;
		}
	}





}//CPU fetchexe end


void Debug_AAA(unsigned short ReadRAMAddress,unsigned char value) {

	static int count = 0;
	
	if(0)
	if(count<0x800)
	if (ReadRAMAddress == 0x2007 && PPURegisters.VRAMaddress >= 0x2000 && PPURegisters.VRAMaddress <= 0x2800/*&& cpuaction >= 25390  && cpuaction < 25500*/) {
		printf("cpuaction %d value 0x%x VRAMaddress 0x%x Scanline %d Dot %d\n", cpuaction, value, PPURegisters.VRAMaddress, Rendering.Scanline, Rendering.Dot);
		count++;
	}

}//Writeviappuport


// ****************************************************************************