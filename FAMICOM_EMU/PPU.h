
/*

	・PPU(picture processing unit)

*/


#define PIXHEIGHT 2
#define PIXWIDTH 2


struct RenderStates {

	short Scanline;		
	unsigned short Dot;
	unsigned char Oddframeflag;
	unsigned char SpriteZero_flag;		
	short PPUClocks;

};


struct OAM_PrimaryRegisters {
	unsigned char Y, Index, Attribute, X;
};
struct OAM_SecondaryRegisters {
	unsigned char Y, Index, Attribute, X;
	unsigned char	Pattern_Shift0, Pattern_Shift1;	
};	

struct PPURegisters {	
	
//Registers used in Rendering --------------------------------------------
// BG --------------------
	unsigned short VRAMaddress, TempVRAMaddress/*can also be thought of as the address of the top left onscreen tile.*/;	//both 15bits
	unsigned char FineXScroll, WriteToggle;	//3bit,1bit		nesdevでは順にv,t,x,w
	
	unsigned short BGPattern_Shift0, BGPattern_Shift1;
	unsigned char BGPattern_standby0, BGPattern_standby1;	
	unsigned short BGAttribute_Shift0, BGAttribute_Shift1;
	unsigned char BGAttribute_standby0, BGAttribute_standby1;

	unsigned char Internal2007ReadBuff;

// Sprites ---------------
	struct OAM_PrimaryRegisters	OAM_1st[64];		//64spritesのデータの入るPPUレジスタ
	struct OAM_SecondaryRegisters	OAM_2nd[8];	//8sprites for current? scanline : limit is 8 
	
};
