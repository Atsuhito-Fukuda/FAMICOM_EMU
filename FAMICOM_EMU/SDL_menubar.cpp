#include "main.h"


char MousePointing(int x, int y);
char ReadSDLInput(void);


const char MENU_NUM = 5;		//Open ROM, Close ROM, Reset, Quit, Pause
SDL_Surface* menu[MENU_NUM];
const char* label[MENU_NUM] = { "Open", "Close", "Pause", "Reset", "Quit" };	
TTF_Font* font;
SDL_Rect rect, scr_rect[MENU_NUM];
SDL_Color ActionColor[3] = { {0x00,0x00,0x00},{0x00,0x00,0xff} ,{0xa4,0xa4,0xa4} };		



void Pre_StartupScreen(void) {

	TTF_Init();
	font = TTF_OpenFont("arial.ttf", FONTHEIGHT);

	// 画像の矩形情報設定 
	rect.x = 0;
	rect.y = 0;
	rect.w = ScreenSurface->w / 8;
	rect.h = MenuBarHeight;

	for (int i = 0; i < MENU_NUM; i++) {

	// 画像配置位置情報の設定 
		scr_rect[i].x = i * rect.w + MENUBAR_LEFTEDGE;
		scr_rect[i].y = 0;

	}
	
}


void StartupScreen(void) {

	//Start up 画面の表示
	unsigned int* pixels = (unsigned int*)ScreenSurface->pixels;	
	int dx, dy;

	for (dy = 0; dy < MenuBarHeight; dy++) {
		for (dx = 0; dx < ScreenSurface->w; dx++) {
			pixels[dy * ScreenSurface->w + dx] = 0xffffff;
		}
	}
	for (dy = 0; dy < PIXHEIGHT * 240; dy++) {
		for (dx = 0; dx < ScreenSurface->w; dx++) {
			pixels[((MenuBarHeight + dy) * ScreenSurface->w) + dx] = 0xbdbdbd;
		}
	}


	for (int i = 0; i < MENU_NUM; i++) {

		if(i==0 || i==4)
			menu[i] = TTF_RenderUTF8_Blended(font, label[i], ActionColor[0]);
		else
			menu[i] = TTF_RenderUTF8_Blended(font, label[i], ActionColor[2]);


		SDL_BlitSurface(menu[i], &rect, ScreenSurface, &scr_rect[i]);

	}


	SDL_UpdateWindowSurface(window);



	// 入力待ち --------------------------------------------

	while (ReadSDLInput()) {

		SDL_Delay(33);

	}

}



char MousePointing(int Mouse_x, int Mouse_y) {

	for (int i = 0; i < MENU_NUM; i++) {
		if (Mouse_x >= scr_rect[i].x && Mouse_x < scr_rect[i].x + rect.w
			&& Mouse_y >= scr_rect[i].y && Mouse_y < scr_rect[i].y + rect.h)
			return i;
	}

	return -1;

}

void Click_ColorChange(char MousePoint) {

	menu[MousePoint] = TTF_RenderUTF8_Blended(font, label[MousePoint], ActionColor[1]);
	SDL_BlitSurface(menu[MousePoint], &rect, ScreenSurface, &scr_rect[MousePoint]);
	SDL_UpdateWindowSurface(window); 

}

void ReleaseClick_ColorChange(char MousePoint) {

	SDL_Rect BG_rect = { scr_rect[MousePoint].x, scr_rect[MousePoint].y,rect.w ,rect.h };
	SDL_FillRect(ScreenSurface, &BG_rect, SDL_MapRGB(ScreenSurface->format, 0xff, 0xff, 0xff));

	menu[MousePoint] = TTF_RenderUTF8_Blended(font, label[MousePoint], ActionColor[0]);
	SDL_BlitSurface(menu[MousePoint], &rect, ScreenSurface, &scr_rect[MousePoint]);
	SDL_UpdateWindowSurface(window);

}


extern char MousePoint[2];

void Reset_ControllBuff(void) {

	while (SDL_PollEvent(Event))
		;	
	MousePoint[1] = 0;


}

void SwitchtoPlaymode(void) {

	Reset_ControllBuff();

	SDL_Rect BG_rect = { 0, 0,ScreenSurface->w ,MenuBarHeight };
	SDL_FillRect(ScreenSurface, &BG_rect, SDL_MapRGB(ScreenSurface->format, 0xff, 0xff, 0xff));


	for (int i = 0; i < MENU_NUM; i++) {
		if (i >= 1 && i <= 4)
			menu[i] = TTF_RenderUTF8_Blended(font, label[i], ActionColor[0]);
		else
			menu[i] = TTF_RenderUTF8_Blended(font, label[i], ActionColor[2]);


		SDL_BlitSurface(menu[i], &rect, ScreenSurface, &scr_rect[i]);

	}



	SDL_SetWindowTitle(window, Rom_Info.File_Name);

	SDL_PauseAudio(0);

}

void SwitchtoStartScreenmode(void) {

	Reset_ControllBuff();

	SDL_Rect BG_rect = { 0, 0,ScreenSurface->w ,MenuBarHeight };
	SDL_FillRect(ScreenSurface, &BG_rect, SDL_MapRGB(ScreenSurface->format, 0xff, 0xff, 0xff));


	for (int i = 0; i < MENU_NUM; i++) {
		if (i == 0 || i == 4)
			menu[i] = TTF_RenderUTF8_Blended(font, label[i], ActionColor[0]);
		else
			menu[i] = TTF_RenderUTF8_Blended(font, label[i], ActionColor[2]);


		SDL_BlitSurface(menu[i], &rect, ScreenSurface, &scr_rect[i]);

	}


	SDL_SetWindowTitle(window, "NES Emulator");

	SDL_PauseAudio(1);	

}

extern char ScreenMode;

void ScreenModeChange(char Branch) {


	if (Branch == 1) {
		ScreenMode = 1;
		SwitchtoPlaymode(); 



	}

	if (Branch == 0) {
		ScreenMode = 0;
		//SDL_Delay(280);	//演出用
		SwitchtoStartScreenmode();

	}


}
