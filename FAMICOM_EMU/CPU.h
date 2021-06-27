struct Registers {
	unsigned char A;		//Accumulator
	unsigned char X;		//Index_X,Y
	unsigned char Y;
	unsigned short PC;		//Program Counter (2Bの現在のアドレス位置)
	unsigned char S;		//Stack Pointer
	unsigned char P;		//status register(6 bits used by the ALU but is byte-wide. PHP, PLP, arithmetic, testing, and branch instructions can access this register.)
};

// Addressing Modes num allocate
#define	NONE 0
#define	IMP 1	//Implied
#define	ACC 2	//Accumulator
#define	IMM 3	//Immediate
#define	ZER 4	//Zero Page
#define	ZERX 5	//Zero Page X indexed
#define	ZERY 6	//Zero Page Y indexed
#define	REL 7	//Relative
#define	ABS 8	//Absolute
#define	ABSX 9	//Absolute X indexed
#define	ABSY 10	//Absoluete Y indexed
#define	IND 11	//Indirect
#define	INXIND 12	//Indexed Indirect
#define	INDINX 13	//Indirect Indexed

