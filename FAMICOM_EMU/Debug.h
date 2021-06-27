// Ç±ÇÍÇÕDebugêÊÇ…Ç¢ÇÍÇÈÇ‡ÇÃ



void Debug_pointerOpcode(unsigned char Opcode);
void Debug_LookVRAM(void);
void SlowFrame(void);
void Debug_DetectWhenUpdated(unsigned char Opcode);
void Debug_AAA(unsigned short ReadRAMAddress, unsigned char value);

extern int cpuaction;
extern unsigned int Frame;