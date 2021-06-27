#include "main.h"

int R = 0;	//debug
int p=0;	//debug


// Prototypes --------------------------------
void RenderAPixel(void);
void SendSDLPixelData(unsigned short x, unsigned short y, unsigned int color);
char Loadto2ndOMA(void);
void Horinc(void);
void vt_Vertinc(void);
void fetchAttribute(void);
unsigned char getAttributeTableValue(unsigned short attribute_address, unsigned char x, unsigned char y);
// -------------------------------------------

char RenderEnable(void) {

	if ((RAM[0x2001] & 0b00011000) == 0b00011000)
		return 1;
	else
		return 0;

}

void PPURunASprite8Cycle(void) {

// Sprite Registers Shift

	int i;
	for (i = 0; i < 8; i++) {
		if (PPURegisters.OAM_2nd[i].X == 0) {
			PPURegisters.OAM_2nd[i].Pattern_Shift0 <<= 1;
			PPURegisters.OAM_2nd[i].Pattern_Shift1 <<= 1;
		}
		else PPURegisters.OAM_2nd[i].X -= 1;
	}

}

void PPURunABG8Cycle(void) {
//8Cycle:

	/*if (Frame == 33) {
		printf("Scanline %d Dot %d VRAMaddress 0x%x\n", RenderStates.Scanline, RenderStates.Dot, PPURegisters.CurrentVRAMaddress);
		//Sleep(10);
	}*/

		// BG Registers Shift
		PPURegisters.BGPattern_Shift0 <<= 1;
		PPURegisters.BGPattern_Shift1 <<= 1;
		PPURegisters.BGAttribute_Shift0 <<= 1;
		PPURegisters.BGAttribute_Shift1 <<= 1;
	
		static unsigned short NameTableBase;
		static unsigned short BGPatternBase;
		static unsigned char row;

		switch (Rendering.Dot % 8) {

		case 1: //cycle 1Å`2
			NameTableBase = 0x2000 + (PPURegisters.VRAMaddress & 0b111111111111);
			break;

		case 3:	//cycle 3Å`4
		fetchAttribute();
		break;
		
		case 4:	//cycle 5Å`6
			if (RAM[0x2000] & 0b00010000) BGPatternBase = 0x1000;
			else BGPatternBase = 0x0000;

			row = ((PPURegisters.VRAMaddress & 0b111000000000000) >> 12);
			PPURegisters.BGPattern_standby0 =
				VRAM[BGPatternBase + VRAM[NameTableBase] * 16 + row];

			/*if (Frame == 34)
				printf("0x%x\n", NameTableBase );*/
			break;

		case 7: //cycle 7Å`8
			PPURegisters.BGPattern_standby1 =
				VRAM[BGPatternBase + VRAM[NameTableBase] * 16 + 8 + row];
			break;

		case 0: //cycle 8
			PPURegisters.BGPattern_Shift0 |= (unsigned short)PPURegisters.BGPattern_standby0;
			PPURegisters.BGPattern_Shift1 |= (unsigned short)PPURegisters.BGPattern_standby1;
			PPURegisters.BGAttribute_Shift0 |= (unsigned short)PPURegisters.BGAttribute_standby0;
			PPURegisters.BGAttribute_Shift1 |= (unsigned short)PPURegisters.BGAttribute_standby1;

			Horinc();	
			break;

		default:
			break;

		}

	//RenderStates.Dot += 1;

}


void RenderAPixel(void) {


	unsigned char temp0, temp1;	


//get BG Pixel Color Index
	unsigned char shift = 15 - PPURegisters.FineXScroll;	
	temp0 = (PPURegisters.BGPattern_Shift0 & (1 << shift)) >> shift;
	temp1 = (PPURegisters.BGPattern_Shift1 & (1 << shift)) >> shift;	
	unsigned char BGColorIndex = (temp1 << 1) | temp0;	
	temp0 = (PPURegisters.BGAttribute_Shift0 & (1 << shift)) >> shift;
	temp1 = (PPURegisters.BGAttribute_Shift1 & (1 << shift)) >> shift;
	unsigned char BGAttributeIndex = (temp1 << 1) | temp0;

	//get Sprite Pixel Color Index
	int current_sprite;
	unsigned char SpriteColorIndex = 0;
	for (current_sprite = 0; current_sprite < 8; current_sprite++) {
		if (PPURegisters.OAM_2nd[current_sprite].X == 0) {
			temp0 = (PPURegisters.OAM_2nd[current_sprite].Pattern_Shift0 & 0b10000000) >> 7;
			temp1 = (PPURegisters.OAM_2nd[current_sprite].Pattern_Shift1 & 0b10000000) >> 7;
			SpriteColorIndex = (temp1 << 1) | temp0;
			if (SpriteColorIndex != 0b00) 
				goto Next1;
		}
	}

Next1:

	
	unsigned short PaletteBase;	

	if (BGColorIndex == 0 && SpriteColorIndex == 0) {	
		SendSDLPixelData(Rendering.Dot, Rendering.Scanline, NESColor[VRAM[0x3f00]]);
		goto Next2;
	}

	// Sprite ï`é 	
	if (SpriteColorIndex != 0) {	
		if (BGColorIndex == 0 || (PPURegisters.OAM_2nd[current_sprite].Attribute & 0b00100000) == 0) {

			PaletteBase = 0x3f10 + (PPURegisters.OAM_2nd[current_sprite].Attribute & 0b00000011) * 0x04;	
			SendSDLPixelData(Rendering.Dot, Rendering.Scanline, NESColor[VRAM[PaletteBase + SpriteColorIndex]]);

			//printf("color");

			goto Next2;

		
		}
	}
	
// Background ï`é 
	PaletteBase = 0x3f00 + BGAttributeIndex * 0x04;	
	SendSDLPixelData(Rendering.Dot, Rendering.Scanline, NESColor[VRAM[PaletteBase + BGColorIndex]]);


Next2:


	// if Sprite Zero Hit --------------------------------------------

	if (/*(CPURAMBuffStart_suchar[0x2002] & 0b01000000) == 0 && */Rendering.SpriteZero_flag == 1
		&& current_sprite == 0 && SpriteColorIndex != 0 && BGColorIndex != 0) 
	{
		//if(RenderStates.Scanline==30)
		RAM[0x2002] |= 0b01000000;		//Spite Zero bit

		/*if (Frame == 66)
			printf("Scanline %d\n", RenderStates.Scanline);*/
	}

}


void SendSDLPixelData(unsigned short x, unsigned short y, unsigned int color) {	

	//printf("%x\n", color);

	unsigned int* pixels = (unsigned int *)ScreenSurface->pixels;	
	int dx = 0, dy = 0;
	for (dy = 0; dy < PIXHEIGHT; dy++) {
		for (dx = 0; dx < PIXWIDTH; dx++) {
			pixels[((MenuBarHeight + (y * PIXHEIGHT) + dy) * ScreenSurface->w) + ((x - 1) * PIXWIDTH) + dx] = color;
		}	
	}		
}



char Loadto2ndOMA(void) {
	// load: Primary OAM -> Secondary OAM
		//Initialize
	int i, secOAM_num = 0;	
	Rendering.SpriteZero_flag = 0;
						

	
	unsigned short SpritePatternBase;
	if (RAM[0x2000] & 0b00001000) SpritePatternBase = 0x1000;
	else SpritePatternBase = 0x0000;

	for (i = 0; /*OAMstart +*/ i < 64; i++) {

		if (PPURegisters.OAM_1st[/*OAMstart +*/ i].Y <= (Rendering.Scanline /*+ 1*/) && (PPURegisters.OAM_1st[/*OAMstart +*/ i].Y + 8) > (Rendering.Scanline /*+ 1*/)) {
			
			// fill in Secondary OAM ----------------------------------------
			unsigned char Y = PPURegisters.OAM_2nd[secOAM_num].Y = PPURegisters.OAM_1st[/*OAMstart +*/ i].Y;
			unsigned char Index = PPURegisters.OAM_2nd[secOAM_num].Index = PPURegisters.OAM_1st[/*OAMstart +*/ i].Index;
			unsigned char Attribute = PPURegisters.OAM_2nd[secOAM_num].Attribute = PPURegisters.OAM_1st[/*OAMstart +*/ i].Attribute;
			PPURegisters.OAM_2nd[secOAM_num].X = PPURegisters.OAM_1st[/*OAMstart +*/ i].X;

			// fill in bit0,1 Pattern table Registers --------
			//if vertical flip
			if (Attribute & 0b10000000) {	//vertical flip Ç†ÇË
				PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift0 =
					VRAM[SpritePatternBase + Index * 16 + 7 - (Rendering.Scanline /*+ 1*/ - Y)];
				PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift1 =
					VRAM[SpritePatternBase + Index * 16 + 8 + 7 - (Rendering.Scanline /*+ 1*/ - Y)];
			}
			else {	// Ç»Çµ
				PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift0 =
					VRAM[SpritePatternBase + Index * 16 + (Rendering.Scanline /*+ 1*/ - Y)];	
				PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift1 =
					VRAM[SpritePatternBase + Index * 16 + 8 + (Rendering.Scanline /*+ 1*/ - Y)];	
			}
			//horizonal flip
			if (Attribute & 0b01000000) {
				int bit;
				unsigned char temp0 = 0, temp1 = 0;
				unsigned char temp2ndOAM0 = PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift0;
				unsigned char temp2ndOAM1 = PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift1;
				for (bit = 0; bit < 8; bit++) {
					temp0 <<= 1;
					temp0 |= (temp2ndOAM0 & 0b00000001);
					temp2ndOAM0 >>= 1;
					temp1 <<= 1;
					temp1 |= (temp2ndOAM1 & 0b00000001);
					temp2ndOAM1 >>= 1;
				}
				PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift0 = temp0;
				PPURegisters.OAM_2nd[secOAM_num].Pattern_Shift1 = temp1;
			}
			// ------------------------------------------------------------

			if (i == 0)	Rendering.SpriteZero_flag = 1;

		/*	if (Frame ==1203)
				printf("Scanline %d SP0flag %d\n",RenderStates.Scanline, RenderStates.SpriteZero_flag);*/

			secOAM_num++;
			if (secOAM_num == 8) return 0;
		}
	}

	/*if(Frame==33)*/
		//printf("SP0flag %d\n", RenderStates.SpriteZero_flag);
		

	return 0;
}

void Clear2ndOMA(void) {

	int i;
	for (i = 0; i < 8; i++) {
		PPURegisters.OAM_2nd[i] = { 0xff,0xff,0xff,0xff };
	}
}

void NMI(void) {
	PUSH((unsigned char)(Registers.PC >> 8));
	PUSH((unsigned char)(Registers.PC & 0xff));
	PUSH(Registers.P | 0b00100000);
	Registers.P |= 0b00000100;		
	Registers.PC = (RAM[0xfffb] << 8) | RAM[0xfffa];
	
}

void Horinc(void) {

	if ((PPURegisters.VRAMaddress & 0b11111) == 0b11111) {
		PPURegisters.VRAMaddress &= ~0b11111;
		PPURegisters.VRAMaddress ^= 0x0400;
	}
	else PPURegisters.VRAMaddress += 1;
}

void vt_Vertinc(void) {

// v ÇÃFine Y scroll Ç∆course Y scroll ÇÃêiçs
	if ((PPURegisters.VRAMaddress & 0b0111000000000000) == 0b0111000000000000) {
		PPURegisters.VRAMaddress &= 0b1000111111111111;	
		unsigned short courseYscroll = (PPURegisters.VRAMaddress & 0b1111100000) >> 5;
		if (courseYscroll == 29) {
			courseYscroll = 0;
			PPURegisters.VRAMaddress ^= 0x0800;	//XOR	
		}
		else if (courseYscroll == 31) courseYscroll = 0;	
		else   courseYscroll += 1;

		PPURegisters.VRAMaddress =
			((PPURegisters.VRAMaddress & 0b1111110000011111) | (courseYscroll << 5));	
	}
	else PPURegisters.VRAMaddress += 0b001000000000000;

	//v: ....F.. ...EDCBA = t: ....F.. ...EDCBA
	PPURegisters.VRAMaddress &= 0b0111101111100000;	
	PPURegisters.VRAMaddress |= PPURegisters.TempVRAMaddress & 0b1000010000011111;

}


// Å´lig-masterÇÊÇËä€ÉRÉs -----------------------------------
void fetchAttribute(void) {
	unsigned short attribute_address = (0x23C0 | (PPURegisters.VRAMaddress & 0x0C00) | ((PPURegisters.VRAMaddress >> 4) & 0x38) | ((PPURegisters.VRAMaddress >> 2) & 0x07));
	unsigned char at = getAttributeTableValue(attribute_address, (PPURegisters.VRAMaddress & 0x001F) * 8, ((PPURegisters.VRAMaddress & (0x001F << 5)) >> 5) * 8);
	if (at & 1) {
		PPURegisters.BGAttribute_standby0 = 0xFF;
	}
	else {
		PPURegisters.BGAttribute_standby0 = 0x00;
	}
	if (at & 2) {
		PPURegisters.BGAttribute_standby1 = 0xFF;
	}
	else {
		PPURegisters.BGAttribute_standby1 = 0x00;
	}
}

unsigned char getAttributeTableValue(unsigned short attribute_address, unsigned char x, unsigned char y) {
	unsigned char attribute_value = VRAM[attribute_address];

	unsigned char bottom = 1;
	unsigned char right = 1;
	if ((y / 32) * 32 == (y / 16) * 16) {
		//top
		bottom = 0;
	}
	if ((x / 32) * 32 == (x / 16) * 16) {
		//left
		right = 0;
	}

	unsigned char mask = 0;
	if (bottom == 0 && right == 0) {
		mask = (1 << 1) | (1 << 0);
		attribute_value = (attribute_value & mask);
	}
	if (bottom == 0 && right == 1) {
		mask = (1 << 3) | (1 << 2);
		attribute_value = (attribute_value & mask) >> 2;
	}
	else if (bottom == 1 && right == 0) {
		mask = (1 << 5) | (1 << 4);
		attribute_value = (attribute_value & mask) >> 4;
	}
	else if (bottom == 1 && right == 1) {
		mask = (1 << 7) | (1 << 6);
		attribute_value = (attribute_value & mask) >> 6;
	}

	return attribute_value;
}
