#include "main.h"

char ScreenMode;	//0: Start Screen 1: Playing Game
char MousePoint[2] = {};


void Init_ControllerParameters(void) {

	Controller = {};

}

char ReadSDLInput(void) {


	static char Pause = 0;

	while (SDL_PollEvent(Event)) {

		if (Event->type == SDL_QUIT)

			exit(EXIT_SUCCESS);


		if (Event->type == SDL_KEYDOWN) {

			switch (Event->key.keysym.sym) {

			case SDLK_c:
				Controller.Button[0] = 1; //A
				break;

			case SDLK_z:
				Controller.Button[1] = 1; //B
				break;

			case SDLK_q:
				Controller.Button[2] = 1; //SELECT
				break;

			case SDLK_w:
				Controller.Button[3] = 1; //START
				break;

			case SDLK_UP:
				Controller.Button[4] = 1; //UP
				break;

			case SDLK_DOWN:
				Controller.Button[5] = 1; //DOWN
				break;

			case SDLK_LEFT:
				Controller.Button[6] = 1; //LEFT
				break;

			case SDLK_RIGHT:
				Controller.Button[7] = 1; //RIGHT
				break;


			case SDLK_DELETE:
				exit(0);

			case SDLK_ESCAPE:	//RESET
				if (ScreenMode == 1)
					return 3;
				break;

			case SDLK_BACKSPACE:	
				Pause ^= 1;	//	toggle
				Screen_GetinPause();
				break;
			default:
				break;
			}
		}
		else if (Event->type == SDL_KEYUP) {

			switch (Event->key.keysym.sym) {

			case SDLK_c:
				Controller.Button[0] = 0; //A
				break;

			case SDLK_z:
				Controller.Button[1] = 0; //B
				break;

			case SDLK_q:
				Controller.Button[2] = 0; //SELECT
				break;

			case SDLK_w:
				Controller.Button[3] = 0; //START
				break;

			case SDLK_UP:
				Controller.Button[4] = 0; //UP
				break;

			case SDLK_DOWN:
				Controller.Button[5] = 0; //DOWN
				break;

			case SDLK_LEFT:
				Controller.Button[6] = 0; //LEFT
				break;

			case SDLK_RIGHT:
				Controller.Button[7] = 0; //RIGHT
				break;

			default:
				break;
			}
		}

		else if (Event->type == SDL_MOUSEBUTTONDOWN) {


			switch (MousePoint[0] = MousePointing(Event->button.x, Event->button.y)) {

			case 0:	//Open ROM
				if (ScreenMode == 0) {

					MousePoint[1] = 1;
					Click_ColorChange(MousePoint[0]);
				
					if (GetOpenFileName(&OpenFileName))
						return 0;
					else
						return -1;
				}
				break;

			case 1:	//Close ROM
				if (ScreenMode == 1) {
					MousePoint[1] = 1;
					Click_ColorChange(MousePoint[0]);
					SDL_Delay(280);//演出用
					ScreenModeChange(0);
					return 1;
				}

				break;

			case 2:	//Pause
				if (ScreenMode == 1) {
					Pause ^= 1;
					MousePoint[1] = 1;
					Click_ColorChange(MousePoint[0]);
				}

				break;

			case 3:	// Reset
				if (ScreenMode == 1) {
					MousePoint[1] = 1;
					Click_ColorChange(MousePoint[0]);
					SDL_Delay(280);//演出用
					return 3;
				}

				break;

			case 4:	//quit
				MousePoint[1] = 1;
				Click_ColorChange(MousePoint[0]);
				SDL_Delay(280);	//演出用
				exit(0);


			default:
				break;

			}




		}
		else if (Event->type == SDL_MOUSEBUTTONUP) {

			if (MousePoint[1]) {	

				ReleaseClick_ColorChange(MousePoint[0]);		
				MousePoint[1] = 0;

			}
		}


	}


	// Pause

	while (Pause) {

		SDL_Delay(33);

		while (SDL_PollEvent(Event)) {

			if (Event->type == SDL_KEYDOWN) {
				switch (Event->key.keysym.sym) {
				case SDLK_DELETE:
					exit(0);

				case SDLK_ESCAPE:	//RESET
					Screen_GetoutPause();
						return 3;
				

				case SDLK_BACKSPACE:
					Pause ^= 1;
					Screen_GetoutPause();
					return 2;

				}
			}
			else if (Event->type == SDL_MOUSEBUTTONDOWN) {


				switch (MousePoint[0] = MousePointing(Event->button.x, Event->button.y)) {

				case 1:	//Close ROM
					if (ScreenMode == 1) {
						MousePoint[1] = 1;
						Click_ColorChange(MousePoint[0]);
						SDL_Delay(280);//演出用
						ScreenModeChange(0);
						Pause ^= 1;
						return 1;
					}

					break;

				case 2:	//Pause
					if (ScreenMode == 1) {
						Pause ^= 1;
						Screen_GetoutPause();
						return 2;
					}

					break;

				case 3:	// Reset
					if (ScreenMode == 1) {
						MousePoint[1] = 1;
						Click_ColorChange(MousePoint[0]);
						SDL_Delay(280);//演出用
						Screen_GetoutPause();
						Pause ^= 1;
						return 3;
					}

					break;

				case 4:	//quit
					MousePoint[1] = 1;
					Click_ColorChange(MousePoint[0]);
					SDL_Delay(280);	//演出用
					exit(0);


				default:
					break;

				}
			}
			else if (Event->type == SDL_MOUSEBUTTONUP) {

				if (MousePoint[1]) {
					
					if (MousePoint[0] != 2)	
						ReleaseClick_ColorChange(MousePoint[0]);
					

					MousePoint[1] = 0;
				}
			}
			


		}

	}
	

	return -1;


}

