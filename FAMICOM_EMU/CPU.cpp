#include "main.h"


void WriteVRAM_Mirror(unsigned short ReadRAMAddress, unsigned char Value);



// Operands Emulate -------------------------------------------------

// PUSH & POP -----------------
void PUSH(unsigned char value) {
	RAM[0x0100 + Registers.S] = value;
	Registers.S -= 1;
}
unsigned char POP(void) {
	Registers.S += 1;
	return RAM[0x0100 + Registers.S];
}

//-----------------------------
void Set_ZNStatusFlags(unsigned char value_result) {	

// set Zflag
	if (value_result == 0)	Registers.P |= 0b00000010;	
	else Registers.P &= 0b11111101;		

// set Nflag
	if (value_result & 0b10000000)	Registers.P |= 0b10000000;	
	else Registers.P &= ~0b10000000;
}


// Opcode Running ----------------------
unsigned short ReturnActionAddress(unsigned char opcode) {

	char AddressingModes_num = AddressingModes_Table[opcode];
	
	unsigned char low_temp, high_temp, tempx;
	unsigned short temp1, temp2;

	switch (AddressingModes_num) {

	case IMM:
		return Registers.PC += 1;		

	case ZER:
		return RAM[Registers.PC += 1];

	case ZERX:
		return (Registers.X + RAM[Registers.PC += 1]) & 0xff;	
	
	case ZERY:
		return (Registers.Y + RAM[Registers.PC += 1]) & 0xff;	
	
	case REL:
		if (0b10000000 & (low_temp = RAM[Registers.PC += 1])) {
			low_temp = ~low_temp + 1;	
			temp1 = low_temp;			
			temp1 = ~temp1 + 1;
		}	
			else temp1 = low_temp;		
		
		return temp1 + Registers.PC;	
		
	
	case ABS:
		low_temp = RAM[Registers.PC += 1];	
		high_temp = RAM[Registers.PC += 1];
		
		return (high_temp << 8) | low_temp;
	
	case ABSX:
		low_temp = RAM[Registers.PC += 1];
		high_temp = RAM[Registers.PC += 1];
		temp1 = (high_temp << 8) | low_temp;
		temp2 = temp1 + Registers.X;
		
		//Page break check
		if (PageBreakClocks[opcode] && (temp1 & 0xff00) ^ (temp2 & 0xff00))
			Rendering.PPUClocks += 3;	
		
		return temp2;
	
	case ABSY:
		low_temp = RAM[Registers.PC += 1];
		high_temp = RAM[Registers.PC += 1];
		temp1 = (high_temp << 8) | low_temp;
		temp2 = temp1 + Registers.Y;
		
		//Page break check
		if (PageBreakClocks[opcode] && (temp1 & 0xff00) ^ (temp2 & 0xff00))
			Rendering.PPUClocks += 3;
		
		return temp2;
	
	case IND:
		low_temp = RAM[Registers.PC += 1];
		high_temp = RAM[Registers.PC += 1];
		temp1 = (high_temp << 8) | low_temp;
		if (low_temp == 0xff) return (RAM[temp1 & 0xff00] << 8) | RAM[temp1];	
		
		return (RAM[temp1 + 1] << 8) | RAM[temp1];		
	
	case INXIND:
		low_temp = Registers.X + RAM[Registers.PC += 1];
		
		return  RAM[low_temp] | (RAM[(low_temp + 1) & 0xff] << 8);	
	
	case INDINX:																					
		tempx = RAM[Registers.PC += 1];
		low_temp = RAM[tempx];;
		high_temp = RAM[(tempx + 1) & 0xff];
		temp1 = (high_temp << 8) | low_temp;
		temp2 = temp1 + Registers.Y;

		//Page break check
		if (PageBreakClocks[opcode] && (temp1 & 0xff00) ^ (temp2 & 0xff00))
			Rendering.PPUClocks += 3;
		
		return temp2;
	
	default:
		break;
	}
	return 0;
}


unsigned char ReadRAM_viaPortTreat(unsigned short ReadRAMAddress){

	
	unsigned char ReturnValue;	
	
	switch (ReadRAMAddress) {

	case 0x2002:	//PPU Status
		ReturnValue = RAM[0x2002];
		RAM[0x2002] &= 0b01111111;;	
		PPURegisters.WriteToggle = 0;
		break;

	case 0x2004:
		if (RAM[0x2002] & 0b10000000 ||
			(RAM[0x2001] & 0b00011000) == 0) 
			ReturnValue = *((unsigned char *)PPURegisters.OAM_1st + RAM[0x2003]);
		break;

	case 0x2007:
		if (PPURegisters.VRAMaddress <= 0x3eff) {
			ReturnValue = PPURegisters.Internal2007ReadBuff;
			PPURegisters.Internal2007ReadBuff = VRAM[PPURegisters.VRAMaddress & 0x3fff];
		}
		else {
			PPURegisters.Internal2007ReadBuff = VRAM[PPURegisters.VRAMaddress & 0x3fff];
			ReturnValue = PPURegisters.Internal2007ReadBuff;
		}
		if ((RAM[0x2000] & 0b00000100) == 0)
			PPURegisters.VRAMaddress = (PPURegisters.VRAMaddress + 1) /*& 0b0111111111111111*/;
		else PPURegisters.VRAMaddress = (PPURegisters.VRAMaddress + 32)  /*& 0b0111111111111111*/;
		break;




	case 0x4015:
		/*IF-D NT21
			DMC interrupt(I), frame interrupt(F), DMC active(D), length counter > 0 (N / T / 2 / 1)
			N / T / 2 / 1 will read as 1 if the corresponding length counter is greater than 0. For the triangle channel, the status of the linear counter is irrelevant.
			D will read as 1 if the DMC bytes remaining is more than 0.
			Reading this register clears the frame interrupt flag(but not the DMC interrupt flag).
			If an interrupt flag was set at the same moment of the read, it will read back as 1 but it will not be cleared.
			*/
		ReturnValue = RAM[ReadRAMAddress] & 0b11100000;	
		if (APUStat.CH1_Length_Counter > 0)	ReturnValue |= 0b00000001;  	
		if (APUStat.CH2_Length_Counter > 0)	ReturnValue |= 0b00000010;
		if (APUStat.CH3_Length_Counter > 0)	ReturnValue |= 0b00000100;
		if (APUStat.CH4_Length_Counter > 0)	ReturnValue |= 0b00001000;
		if (APUStat.CH5_BytesRemain > 0)	ReturnValue |= 0b00010000;

		RAM[ReadRAMAddress] &= 0b10111111;	//clear frame interrupt flag

		break;

	case 0x4016:
		ReturnValue = Controller.Button[Controller.Button_num];
		Controller.Button_num += 1;
		Controller.Button_num %= 8;		

		break;

	default:
		ReturnValue = RAM[ReadRAMAddress];

	}
	return ReturnValue;		

}



void WriteRAM_viaPortTreat(unsigned short ReadRAMAddress, unsigned char Value) {

	unsigned short base;	
	static unsigned char previous = 0;	


	Debug_AAA(ReadRAMAddress, Value);


	switch (ReadRAMAddress) {

	case 0x2000:
		PPURegisters.TempVRAMaddress &= 0b1111001111111111;	
		PPURegisters.TempVRAMaddress |= (Value & 0b00000011) << 10;		


		//手動NMI
		if ((RAM[0x2002] & 0b10000000) && (RAM[0x2000] & 0b10000000) == 0
			&& (Value & 0b10000000)) {		
			NMI();
			Registers.PC -= 1;	//調節
			/*printf("a");
			static int u = 0;
			if (u == 0) {
				printf("PC=0x%x\n", Registers.PC);
				Registers.PC -= 1;	//調節
				u++;
			}*/
		}
		break;

	case 0x2004:
		RAM[0x2003] += 1;
		break;

	case 0x2005:
		if (PPURegisters.WriteToggle == 0) {
			PPURegisters.TempVRAMaddress &= 0b1111111111100000;	
			PPURegisters.TempVRAMaddress |= (unsigned short)(Value >> 3);
			PPURegisters.FineXScroll = Value & 0b00000111;
		}
		else {
			PPURegisters.TempVRAMaddress &= 0b000110000011111;	
			PPURegisters.TempVRAMaddress |= ((unsigned short)(Value & 0b00000111) << 12) |
				(unsigned short)(Value & 0b11111000) << 2;
		}
		PPURegisters.WriteToggle ^= 1;	
		break;

	case 0x2006:
		if (PPURegisters.WriteToggle == 0) {
			PPURegisters.TempVRAMaddress &= 0b0000000011111111;
			PPURegisters.TempVRAMaddress |= (unsigned short)(Value & 0b00111111) << 8;

			//if ((PPURegisters.TempVRAMaddress & 0xff00) == 0x3f00)printf("%d : 0x%x\n", cpuaction, PPURegisters.TempVRAMaddress);

		}
		else {

			PPURegisters.TempVRAMaddress &= 0b1111111100000000;
			PPURegisters.TempVRAMaddress |= (unsigned short)Value;
			PPURegisters.VRAMaddress = PPURegisters.TempVRAMaddress;

		}
		PPURegisters.WriteToggle ^= 1;
		break;

	case 0x2007:

		WriteVRAM_Mirror(PPURegisters.VRAMaddress & 0x3fff, Value);

		
		if (RAM[0x2000] & 0b00000100) {
			PPURegisters.VRAMaddress += 32;
			
		}
		else {
			PPURegisters.VRAMaddress += 1;
			}
		
		/*if (cpuaction < 25500) {
			printf("after_VRAMadrress 0x%x\n", PPURegisters.CurrentVRAMaddress);
		}*/
		
		break;


	case 0x4000:
		APUStat.CH1_Vol_or_EnvPerCounter = Value & 0b00001111;

		break;

	case 0x4001:
		//A divider can also be forced to reload its counter immediately (counter = P), but this does not output a clock.
		//Similarly, changing a divider's period reload value does not affect the counter. ←無視してよい？
		APUStat.CH1_SweepPeriod = (Value & 0b01110000) >> 4;

		break;

	case 0x4002:	
		APUStat.CH1_Timer &= 0xff00;
		APUStat.CH1_Timer |= (unsigned short)Value;
		APUStat.CH1_SweepTarget = APUStat.CH1_Timer + (APUStat.CH1_Timer >> (RAM[0x4001] & 0b00000111));

		break;

	case 0x4003:
		// restarts the envelope, and resets the phase of the pulse generator
		if (RAM[0x4015] & 0b00000001)
			APUStat.CH1_Length_Counter = LengthCounter[(Value & 0b11111000) >> 3];		
		APUStat.CH1_EnvVol = 0xf;
		APUStat.CH1_shift = 0;		 //resets the phase of the pulse generator.のつもり
		APUStat.CH1_Vol_or_EnvPerCounter = RAM[0x4000] & 0b00001111;	
		APUStat.CH1_Timer &= 0x00ff;
		APUStat.CH1_Timer |= ((unsigned short)Value & 0b00000111) << 8;	
		if (RAM[0x4001] & 0b00001000)
			APUStat.CH1_SweepTarget = (APUStat.CH1_Timer + ~(APUStat.CH1_Timer >> (RAM[0x4001] & 0b00000111))) & 0x7ff;	
		else
			APUStat.CH1_SweepTarget = APUStat.CH1_Timer + (APUStat.CH1_Timer >> (RAM[0x4001] & 0b00000111));
		APUStat.CH1_Audio_Timer_counter = Audio_Freq * 16 / 8 * (APUStat.CH1_Timer + 1) / NES_CPUFreq;	
		APUStat.CH1_SweepPeriod = (RAM[0x4001] & 0b01110000) >> 4;
		
		//printf("[0x4000]=0x%x", RAM[0x4000]);
	//	if (Frame > 600) printf("Length_Counter=%d EnvVol=%d shift=%d Vol_or_EnvPerCounter=%d\n",
		//	APUStat.CH1_Length_Counter, APUStat.CH1_EnvVol, APUStat.CH1_shift,APUStat.CH1_Vol_or_EnvPerCounter);
		//RAM[0x4015] |= 0b00000001;	//必ずLength Counterの値は非0

		break;

	case 0x4004:
		APUStat.CH2_Vol_or_EnvPerCounter = Value & 0b00001111;

		break;

	case 0x4005:
		APUStat.CH2_SweepPeriod = (Value & 0b01110000) >> 4;

		break;

	case 0x4006:	
		APUStat.CH2_Timer &= 0xff00;
		APUStat.CH2_Timer |= (unsigned short)Value;
		APUStat.CH2_SweepTarget = APUStat.CH2_Timer + (APUStat.CH2_Timer >> (RAM[0x4005] & 0b00000111));

		break;

	case 0x4007:
		if (RAM[0x4015] & 0b00000001)
			APUStat.CH2_Length_Counter = LengthCounter[(Value & 0b11111000) >> 3];		
		APUStat.CH2_EnvVol = 0xf;
		APUStat.CH2_shift = 0;		 //resets the phase of the pulse generator.のつもり
		APUStat.CH2_Vol_or_EnvPerCounter = RAM[0x4004] & 0b00001111;
		APUStat.CH2_Timer &= 0x00ff;
		APUStat.CH2_Timer |= ((unsigned short)Value & 0b00000111) << 8;	
		if (RAM[0x4005] & 0b00001000)
			APUStat.CH2_SweepTarget = (APUStat.CH2_Timer + ~(APUStat.CH2_Timer >> (RAM[0x4005] & 0b00000111)) + 1);	
		else
			APUStat.CH2_SweepTarget = APUStat.CH2_Timer + (APUStat.CH2_Timer >> (RAM[0x4005] & 0b00000111));
		APUStat.CH2_Audio_Timer_counter = Audio_Freq * 16 / 8 * (APUStat.CH2_Timer + 1) / NES_CPUFreq;	
		APUStat.CH2_SweepPeriod = (RAM[0x4005] & 0b01110000) >> 4;	

		break;

	case 0x4008:
		APUStat.CH3_LinearCounter = Value & 0b01111111;

		break;

	case 0x400a:
		APUStat.CH3_Timer &= 0xff00;
		APUStat.CH3_Timer |= (unsigned short)Value;

		break;

	case 0x400b:
		APUStat.CH3_Timer &= 0x00ff;
		APUStat.CH3_Timer |= ((unsigned short)Value & 0b00000111) << 8;
		APUStat.CH3_Length_Counter = LengthCounter[Value >> 3];

		break;

	case 0x400c:
		APUStat.CH4_Vol_or_EnvPerCounter = Value & 0b00001111;

		break;

	case 0x400e:
		APUStat.CH4_NoisePeriod = NoisePeriod[Value & 0b00001111];

		break;

	case 0x400f:
		APUStat.CH4_Length_Counter = LengthCounter[(Value & 0b11111000) >> 3];
		APUStat.CH4_EnvVol = 0xf;
		APUStat.CH4_Vol_or_EnvPerCounter = RAM[0x400c] & 0b00001111;

		break;

	case 0x4010:
		APUStat.CH5_Audio_Timer_counter = DPCMRate[Value & 0xf] * Audio_Freq / NES_CPUFreq;	
		if ((Value & 0x80) == 0)
			RAM[0x4015] &= 0x7f;	
		break;

	case 0x4011:
		APUStat.CH5_Output = Value & 0x7f;
		break;

	case 0x4012:
		APUStat.CH5_Address = 0xc000 + ((unsigned short)Value << 6);
		break;

	case 0x4013:
		APUStat.CH5_BytesRemain = ((unsigned short)Value << 4) + 1;
		break;


	case 0x4015:
		/*Writing a zero to any of the channel enable bits will silence that channel and immediately set its length counter to 0.
If the DMC bit is clear, the DMC bytes remaining will be set to 0 and the DMC will silence when it empties.
If the DMC bit is set, the DMC sample will be restarted only if its bytes remaining is 0. If there are bits remaining in the 1-byte sample buffer, these will finish playing before the next sample is fetched.
Writing to this register clears the DMC interrupt flag.
Power-up and reset have the effect of writing $00, silencing all channels
---D NT21  Enable DMC (D), noise (N), triangle (T), and pulse channels (2/1)*/

		if ((Value & 0b00000001) == 0)	APUStat.CH1_Length_Counter = 0;
		if ((Value & 0b00000010) == 0)	APUStat.CH2_Length_Counter = 0;
		if ((Value & 0b00000100) == 0)	APUStat.CH3_Length_Counter = 0;
		if ((Value & 0b00001000) == 0)	APUStat.CH4_Length_Counter = 0;	
		if (Value & 0b00010000) {
			if (APUStat.CH5_BytesRemain == 0) { 
				APUStat.CH5_Address = 0xc000 + ((unsigned short)RAM[0x4012] << 6);
				APUStat.CH5_BytesRemain = ((unsigned short)RAM[0x4013] << 4) + 1;
			}
		}	
		else {
			APUStat.CH5_BytesRemain = 0;	
		}

		Value &= 0b01111111;

		break;

	case 0x4017:
		//Writing to $4017 with bit 7 set($80) will immediately clock all of its controlled units 
		//at the beginning of the 5 - step sequence; with bit 7 clear, only the sequence is reset without clocking any of its units.	←無視
		
		APUStat.ModeFrameCounter = 0;

		break;


	case 0x4014:		//OAM DMA
		
		base = Value << 8;
		unsigned short i;
		for (i = 0; i <= 0xff; i++) {
			*((unsigned char *)PPURegisters.OAM_1st + i) = RAM[base + i];
		}

		Rendering.PPUClocks += 513 * 3;		

		break;

	case 0x4016:	//Controller

		if (previous == 1 && Value == 0) 
			Controller.Button_num = 0;
			previous = Value;
		
		break;

	default:
		break;
	}

	RAM[ReadRAMAddress] = Value;

}

void WriteVRAM_Mirror(unsigned short VRAMAddress, unsigned char Value) {

// Name Table Mirror
	if(VRAMAddress>=0x2000)
		if (VRAMAddress < 0x2400) {
			if (Rom_Info.NTMirroring)
				VRAM[VRAMAddress + 0x800] = Value;
			else
				VRAM[VRAMAddress + 0x400] = Value;
			}
		else if (VRAMAddress < 0x2800) {
			if (Rom_Info.NTMirroring)
				VRAM[VRAMAddress + 0x800] = Value;
			else
				VRAM[VRAMAddress - 0x400] = Value;
		}
		else if (VRAMAddress < 0x2c00) {
			if (Rom_Info.NTMirroring)
				VRAM[VRAMAddress - 0x800] = Value;
			else
				VRAM[VRAMAddress + 0x400] = Value;
		}
		else if (VRAMAddress < 0x3000) {
			if (Rom_Info.NTMirroring)
				VRAM[VRAMAddress - 0x800] = Value;
			else
				VRAM[VRAMAddress - 0x400] = Value;
		}
		

	//PPU Palette Mirror

	switch (VRAMAddress) {

	case 0x3f00: case 0x3f04: case 0x3f08: case 0x3f0c:
		VRAM[VRAMAddress + 0x0010] = Value;
		break;

	case 0x3f10: case 0x3f14: case 0x3f18: case 0x3f1c:
		VRAM[VRAMAddress - 0x0010] = Value;
		break;

	default:
		break;
	}




	VRAM[VRAMAddress] = Value;

}



void FetchAndExecuteOpcode(void) {//return CPU cycles


	unsigned char Opcode = RAM[Registers.PC];
	
	Debug_pointerOpcode(Opcode);


// * * Opcodes * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

	unsigned char temp_low, temp_high;
	unsigned short Temp;
	int Temp_int;

	switch (Opcode) {


	//ADC ------------------	
	case 0x69: case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71:
		temp_low = ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Temp = Registers.A + temp_low + (Registers.P & 0b00000001);	
		// set Cflag
		if (Temp >> 8) Registers.P |= 0b00000001;
			else Registers.P &= ~0b0000001;
		// set Vflag
		if ((Registers.A^ Temp)&~(Registers.A^temp_low) & 0b10000000)	Registers.P |= 0b01000000;	
			else Registers.P &= ~0b01000000;	
		
		Registers.A = (unsigned char)Temp;
		Set_ZNStatusFlags(Registers.A);
		break;

	//AND ------------------
	case 0x29: case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31:
		Registers.A &= ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Set_ZNStatusFlags(Registers.A);
		break;

	//ASL1 ------------------	:左へシフト
	case 0x0A:
		if (Registers.A & 0b10000000) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		Registers.A <<= 1;
		Set_ZNStatusFlags(Registers.A);
		break;

	//ASL2 ------------------
	case 0x1E: case 0x0E: case 0x16: case 0x06:
		Temp = ReturnActionAddress(Opcode);
		temp_low = ReadRAM_viaPortTreat(Temp);
		if (temp_low & 0b10000000) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		temp_low = RAM[Temp] << 1;	
		WriteRAM_viaPortTreat(Temp, temp_low);		//一応
		Set_ZNStatusFlags(temp_low);
		break;

	//BIT ------------------
	case 0x24: case 0x2C:
		temp_low = ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		// set Zflag
		if ((temp_low & Registers.A) == 0)	Registers.P |= 0b00000010;
			else Registers.P &= 0b11111101;
		// set Vflag
		if (temp_low & 0b01000000)	Registers.P |= 0b01000000;
			else Registers.P &= 0b10111111;
		// set Nflag
		if (temp_low & 0b10000000)	Registers.P |= 0b10000000;
			else Registers.P &= ~0b10000000;
		break;

	//BCC ------------------	
	case 0x90: 
		if ((Registers.P & 0b00000001) == 0) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;		
		break; 

	//BCS ------------------
	case 0xB0:
		if (Registers.P & 0b00000001) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break;

	//BEQ ------------------
	case 0xF0:
		if (Registers.P & 0b00000010) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break;

	//BMI ------------------
	case 0x30:
		if (Registers.P & 0b10000000) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break;

	//BNE ------------------
	case 0xD0:
		if ((Registers.P & 0b00000010) == 0) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break;

	//BPL ------------------
	case 0x10:
		if ((Registers.P & 0b10000000) == 0) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break;

	//BRK ------------------ 
	case 0x00:
		Registers.PC += 2;	
		PUSH((unsigned char)(Registers.PC >> 8));	
		PUSH((unsigned char)(Registers.PC & 0xff));	
		PUSH(Registers.P | 0b00110000);
		Registers.P |= 0b00000100;
		temp_low = RAM[0xfffe];
		temp_high = RAM[0xffff];
		Registers.PC = (temp_high << 8) | temp_low;
		goto JumpCommand_End;	

	//BVC ------------------
	case 0x50: 
		if ((Registers.P & 0b01000000) == 0) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break; 

	//BVS ------------------
	case 0x70:
		if (Registers.P & 0b01000000) Registers.PC = ReturnActionAddress(Opcode);
		else Registers.PC += 1;
		break;

	//CLC ------------------
	case 0x18:
		Registers.P &= ~0b00000001;
		break;

	//CLD ------------------
	case 0xD8:
		Registers.P &= ~0b00001000;
		break;

	//CLI ------------------
	case 0x58:
		Registers.P &= ~0b00000100;
		break;

	//CLV ------------------
	case 0xB8:
		Registers.P &= ~0b01000000;
		break;

	//CMP ------------------
	case 0xC9: case 0xC5: case 0xD5: case 0xCD:	case 0xDD: case 0xD9: case 0xC1: case 0xD1:
		Temp_int = Registers.A - ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));	
		//set Cflag
		if (Temp_int >= 0) Registers.P |= 0b00000001;
			else Registers.P &= ~(0b00000001);
		Set_ZNStatusFlags((unsigned char)Temp_int);		
		break;								

	//CPX ------------------
	case 0xE0: case 0xE4: case 0xEC:
		Temp_int = Registers.X - ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		//set Cflag
		if (Temp_int >= 0) Registers.P |= 0b00000001;
			else Registers.P &= ~(0b00000001);
		Set_ZNStatusFlags((unsigned char)Temp_int);
		break;

	//CPY ------------------
	case 0xC0: case 0xC4: case 0xCC:
		Temp_int = Registers.Y - ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		//set Cflag
		if (Temp_int >= 0) Registers.P |= 0b00000001;
			else Registers.P &= ~(0b00000001);
		Set_ZNStatusFlags((unsigned char)Temp_int);
		break;

	//DEC ------------------
	case 0xC6: case 0xD6: case 0xCE: case 0xDE:
		Set_ZNStatusFlags(RAM[ReturnActionAddress(Opcode)] -= 1);
		break;

	//DEX ------------------
	case 0xCA:
		Registers.X -= 1;
		Set_ZNStatusFlags(Registers.X);
		break;

	//DEY ------------------
	case 0x88: 
		Registers.Y -= 1;
		Set_ZNStatusFlags(Registers.Y);
		break; 

	//EOR ------------------
	case 0x49: case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51:
		Registers.A ^= ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Set_ZNStatusFlags(Registers.A);
		break;

	//INC ------------------
	case 0xE6: case 0xF6: case 0xEE: case 0xFE:
		Set_ZNStatusFlags(RAM[ReturnActionAddress(Opcode)] += 1);
		break;

	//INX ------------------
	case 0xE8: 
		Registers.X += 1;
		Set_ZNStatusFlags(Registers.X);
		break; 

	//INY ------------------
	case 0xC8: 
		Registers.Y += 1;
		Set_ZNStatusFlags(Registers.Y);
		break; 

	//JMP ------------------
	case 0x4C: case 0x6C:
		Registers.PC = ReturnActionAddress(Opcode);
		goto JumpCommand_End;

	//JSR ------------------
	case 0x20:	
		Temp = ReturnActionAddress(Opcode);	
		PUSH((unsigned char)(Registers.PC >> 8));
		PUSH((unsigned char)(Registers.PC & 0xff));
		Registers.PC = Temp;
		goto JumpCommand_End;

	//LDA ------------------
	case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1:
		Registers.A = ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Set_ZNStatusFlags(Registers.A);
		break;

	//LDX ------------------
	case 0xA2: case 0xA6: case 0xB6: case 0xAE:	case 0xBE:
		Registers.X = ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));	
		Set_ZNStatusFlags(Registers.X);
		break; 

	//LDY ------------------
	case 0xA0: case 0xA4: case 0xB4: case 0xAC:	case 0xBC: 
		Registers.Y = ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Set_ZNStatusFlags(Registers.Y);
		break;

	//LSR1 ------------------	:右シフト
	case 0x4A:
		if (Registers.A & 0b00000001) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		Registers.A >>= 1;
		Set_ZNStatusFlags(Registers.A);
		break;

	//LSR2 ------------------
	case 0x46: case 0x56: case 0x4E: case 0x5E:
		Temp = ReturnActionAddress(Opcode);
		temp_low = ReadRAM_viaPortTreat(Temp);
		if (temp_low & 0b00000001) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		temp_low = RAM[Temp] >> 1;	
		WriteRAM_viaPortTreat(Temp, temp_low);	
		Set_ZNStatusFlags(temp_low);
		break;

	//ORA ------------------
	case 0x09: case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11:
		Registers.A |= ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Set_ZNStatusFlags(Registers.A);
		break;

	//PHA ------------------ 
	case 0x48:
		PUSH(Registers.A);
		break;

	//PHP ------------------	:PをスタックへPUSH
	case 0x08:
		PUSH(Registers.P | 0b00110000);
		break;

	//PLA ------------------
	case 0x68:
		Registers.A = POP();
		Set_ZNStatusFlags(Registers.A);
		break;

	//PLP ------------------	:スタックからPへPOPUP
	case 0x28:
		Registers.S += 1;
		Registers.P = RAM[0x0100 + Registers.S] & 0b11001111;
		break;

	//ROL1 ------------------	:左へrotate
	case 0x2A:
		if (Registers.A & 0b10000000) temp_low = 1;	else temp_low = 0;
		Registers.A <<= 1;
		if (Registers.P & 0b00000001) Registers.A |= 0b00000001;
		if (temp_low) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		Set_ZNStatusFlags(Registers.A);
		break;

	//ROL2 ------------------
	case 0x26: case 0x2E: case 0x3E: case 0x36:
		Temp = ReturnActionAddress(Opcode);
		temp_low = ReadRAM_viaPortTreat(Temp);
		char check_ROL2;
		if (temp_low & 0b10000000) check_ROL2 = 1;	else check_ROL2 = 0;
		temp_low <<= 1;
		if (Registers.P & 0b00000001) temp_low |= 0b00000001;
		if (check_ROL2) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		Set_ZNStatusFlags(temp_low);
		WriteRAM_viaPortTreat(Temp, temp_low);
		break;

	//ROR1 ------------------	:右へrotate
	case 0x6A:
		if (Registers.A & 0b00000001) temp_low = 1;	else temp_low = 0;
		Registers.A >>= 1;
		if (Registers.P & 0b00000001) Registers.A |= 0b10000000;
		if (temp_low) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		Set_ZNStatusFlags(Registers.A);
		break;

	//ROR2 ------------------
	case 0x66: case 0x76: case 0x6E: case 0x7E:
		Temp = ReturnActionAddress(Opcode);
		temp_low = ReadRAM_viaPortTreat(Temp);
		char check_ROR2;
		if (temp_low & 0b00000001) check_ROR2 = 1;	else check_ROR2 = 0;
		temp_low >>= 1;
		if (Registers.P & 0b00000001) temp_low |= 0b10000000;
		if (check_ROR2) Registers.P |= 0b00000001;
			else Registers.P &= ~0b00000001;
		Set_ZNStatusFlags(temp_low);
		WriteRAM_viaPortTreat(Temp, temp_low);
		break;

	//RTI ------------------ :NMI/IRQからの復帰
	case 0x40:
		Registers.P = POP() & ~0b00110000;
		temp_low = POP();
		temp_high = POP();
		Registers.PC = (temp_high << 8) | temp_low;
		goto JumpCommand_End;	

	//RTS ------------------ :JSRからの復帰
	case 0x60:
		temp_low = POP();
		temp_high = POP();
		Registers.PC = (temp_high << 8) | temp_low;	
		break;

	//SBC ------------------
	case 0xEB: case 0xE9: case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1:
		temp_low = ~ReadRAM_viaPortTreat(ReturnActionAddress(Opcode));
		Temp = Registers.A + temp_low + (Registers.P & 0b00000001);
		// set Cflag
		if (Temp >> 8) Registers.P |= 0b00000001;
			else Registers.P &= ~0b0000001;
		// set Vflag
		if ((Registers.A^ Temp)&~(Registers.A^ temp_low) & 0b10000000)	Registers.P |= 0b01000000;
			else Registers.P &= ~0b01000000;

		Registers.A = (unsigned char)Temp;
		Set_ZNStatusFlags(Registers.A);
		break;

	//SEC ------------------
	case 0x38:
		Registers.P |= 0b00000001;
		break;

	//SED ------------------
	case 0xF8:
		Registers.P |= 0b00001000;
		break;

	//SEI ------------------
	case 0x78:
		Registers.P |= 0b00000100;
		break;

	//STA ------------------
	case 0x85: case 0x95: case 0x8D: case 0x9D:	case 0x99: case 0x81: case 0x91:
		WriteRAM_viaPortTreat(ReturnActionAddress(Opcode), Registers.A);
		break;

	//STX ------------------
	case 0x86: case 0x8E: case 0x96:
		WriteRAM_viaPortTreat(ReturnActionAddress(Opcode), Registers.X);
		break;
		
	//STY ------------------
	case 0x84: case 0x8C: case 0x94:
		WriteRAM_viaPortTreat(ReturnActionAddress(Opcode), Registers.Y);
		break;

	//TAX ------------------
	case 0xAA:
		Registers.X = Registers.A;
		Set_ZNStatusFlags(Registers.X);
		break;

	//TAY ------------------
	case 0xA8:
		Registers.Y = Registers.A;
		Set_ZNStatusFlags(Registers.Y);
		break;

	//TSX ------------------
	case 0xBA:
		Registers.X = Registers.S;
		Set_ZNStatusFlags(Registers.X);
		break;

	//TYA ------------------
	case 0x98:
		Registers.A = Registers.Y;
		Set_ZNStatusFlags(Registers.A);
		break;

	//TXA ------------------
	case 0x8A:
		Registers.A = Registers.X;
		Set_ZNStatusFlags(Registers.A);
		break;
		
	//TXS ------------------
	case 0x9A:
		Registers.S = Registers.X;
		break; 
	
	// default ------------------------
	default:
		break;


	}

	Registers.PC += 1;

JumpCommand_End:	


// CPU cycle の加算 -------------------------------

	Rendering.PPUClocks += CPUClocks[Opcode] * 3;


	Debug_DetectWhenUpdated(Opcode);
	//Debug_LookVRAM();

}