

#include "main.h"


//* * * * * * * * * * * * * * * * * * * * * * * * * * 

int main(int argc, char* argv[]) {


	Boot_Emu();

	

StartupScreen:

	StartupScreen();	



	if (Boot_Game() == 0)
		goto StartupScreen;



RESET:

	Initialize();

	unsigned int Timer = SDL_GetTicks();

	//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


Frame_Start:

	Frame++;

	// Controller Input ---------------------------------

	switch (ReadSDLInput()) {

	case 1:
		goto StartupScreen;
	case 2:
		Timer = SDL_GetTicks();
		break;
	case 3:
		goto RESET;
	default:
		break;
	}


	// Run PPU & CPU ---------------------------------------------------------------------------------

	// Render 240 Scanlines	-----------------------------------------------------

		// Pre Scanline0 ˆ—	

			// Odd frame
	if (Rendering.Oddframeflag == 1 && RenderEnable())
		Rendering.Dot = 1;


Continue:

	if (Rendering.Dot == 0) {


		Rendering.PPUClocks -= 1;		
		Rendering.Dot = 1;	  

		for (; Rendering.PPUClocks > 0; Rendering.PPUClocks -= 1) { 
			
			if (RenderEnable()) {

				RenderAPixel();
				PPURunASprite8Cycle();
				PPURunABG8Cycle();
			}
			
			Rendering.Dot += 1;
		}

	}



	while (1) {

		FetchAndExecuteOpcode();

		for (; Rendering.PPUClocks > 0; Rendering.PPUClocks -= 1) {

			if (Rendering.Dot <= 256) goto Render_1;
			if (Rendering.Dot <= 320) goto Render_2;	
			if (Rendering.Dot <= 336) goto Render_3;
			if (Rendering.Dot <= 340) goto Render_4;

	
		Render_1:
			if (RenderEnable()) {
				RenderAPixel();
				PPURunASprite8Cycle();
				PPURunABG8Cycle();
			}
			goto Render_end;

		Render_2:
			if (Rendering.Dot == 257 && RenderEnable()) {
				vt_Vertinc();
				Clear2ndOMA();
				Loadto2ndOMA();
			}
			goto Render_end;

		Render_3:
			if (RenderEnable()) {
				PPURunABG8Cycle();
			}
			goto Render_end;

		Render_4:
			Rendering.Dot = 0;
			Rendering.Scanline += 1;
			Rendering.PPUClocks -= 1;

			if (Rendering.Scanline == 240)
				goto Post_Render;
			else
				goto Continue;


		Render_end:

			Rendering.Dot += 1;


		}
	}


	// Post Render ----------------------------------------------------------------

Post_Render:

	//printf("a");

	for (; Rendering.PPUClocks > 0; Rendering.PPUClocks -= 1) {
		Rendering.Dot += 1;
	}

	while (1) {

		FetchAndExecuteOpcode();

		for (; Rendering.PPUClocks > 0; Rendering.PPUClocks -= 1) {

			if (Rendering.Scanline == 241 && Rendering.Dot == 1) {

				RAM[0x2002] |= 0b10000000;	//v blank flag

				//printf("a");

				// NMI -------------------

				if (RAM[0x2000] & 0b10000000) {
					NMI();
					//printf("NMI!©");
					Debug_LookVRAM();
				}

				Rendering.Dot += 1;

			}
			else {

				if (Rendering.Dot == 340) {

					Rendering.Dot = 0;
					Rendering.Scanline += 1;

					if (Rendering.Scanline == 261) {
						Rendering.PPUClocks -= 1;
						goto Pre_Render;
					}

				}
				else
					Rendering.Dot += 1;

			}

		}
	}


// Pre Render ----------------------------------------------------------------

Pre_Render:
	

	for (; Rendering.PPUClocks > 0; Rendering.PPUClocks -= 1) {

		//printf("0x%x\n", CPURAMBuffStart_suchar[0x2002] &= 0b01000000);

		if (Rendering.Dot == 1)
			RAM[0x2002] &= 0b00011111;	//sprite overflow, 0 hit, v blank flag

		Rendering.Dot += 1;
	}

	while (1) {

		FetchAndExecuteOpcode();

		for (; Rendering.PPUClocks > 0; Rendering.PPUClocks -= 1) {

			if (Rendering.Dot == 1) goto Pre_1;
			if (Rendering.Dot == 304) goto Pre_2;
			if (Rendering.Dot >= 321 && Rendering.Dot <= 336) goto Pre_3;
			if (Rendering.Dot == 340) goto Pre_4;
			/* else */ goto Pre_End;


		Pre_1:
			RAM[0x2002] &= 0b00011111;	//sprite overflow, 0 hit, v blank flag
			goto Pre_End;

		Pre_2:
			// v: ....F.. ...EDCBA = t: ....F.. ...EDCBA		
			if (RenderEnable()) {
				PPURegisters.VRAMaddress &= 0b0111101111100000;
				PPURegisters.VRAMaddress |= PPURegisters.TempVRAMaddress & 0b1000010000011111;

				Clear2ndOMA();
			
			}
			goto Pre_End;

		Pre_3:
			if (RenderEnable()) {
				PPURunABG8Cycle();
			}
			goto Pre_End;

		Pre_4:
			Rendering.Scanline = 0;
			Rendering.Dot = 0;
			Rendering.Oddframeflag ^= 1;

			goto Update_SDL;


		Pre_End:
			Rendering.Dot += 1;


		}
	}


	// Update SDL  ---------------------------------------------------------

Update_SDL:


	//unsigned int GetTime, AAA;

	SDL_Delay(/*AAA=*/(17 - SDL_GetTicks() + Timer));		// 17 - (SDL_GetTicks() - Previous_Timer)	

	//printf("%d\n",AAA);

	Timer = SDL_GetTicks();
	//if(RenderEnable())	
	SDL_UpdateWindowSurface(window);	

	//Sleep(10);

	//SlowFrame();

	//Audio_Output();
	//SDL_PauseAudioDevice(Audio_Device, 0);
	APUStat.OneFrameCounter = 0;

	//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	
	goto Frame_Start;



END:

	return 0;

}