#include <stdio.h> 
#include <stdlib.h>


int main_ckeck(int argc, char *argv[]) {

	FILE* NESROMfile = fopen(argv[1], "rb");	
	
	int temp, counter = 0;
	int value[16];

	printf("File Name :		%s\n\n", argv[0]);

	for (counter = 0; counter < 0x10; counter++) {	

		temp = fgetc(NESROMfile);

		int n = 0;

		switch (counter) {
		case 0:
		case 1:
		case 2:
			printf("%c ", (unsigned char)temp);
			break;
		case 4:
			printf("\n\nSize of PRG ROM :	%d	*	16KB", temp);
			break;
		case 5:
			printf("\n\nSize of CHR ROM :	%d	*	8KB", temp);
			printf("\t\t(Value 0 means the board uses CHR RAM)\n\n");
			break;
		case 6:
			for (n = 0; n < 8; n++) {
				switch (n) {

				case 0:
					if (temp &(1 << n)) printf("Mirroring: vertical\n\n");
					else printf("Mirroring: horizontal\n\n");
					break;
				case 1:
					if (temp &(1 << n)) printf("Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory\n\n");
					break;
				case 2:
					if (temp &(1 << n)) printf("512-byte trainer at $7000-$71FF (stored before PRG data)\n\n");
					break;
				case 3:
					if (temp &(1 << n)) printf("Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM\n\n");
					break;
				case 4:
					printf("Lower nybble of mapper number	:  %d\n\n", temp >> 4);
					break;
				}

			}
			break;
	
		}

		value[counter] = temp;

	}
	printf("\n(Fisrt 16B are  :  )" );

	int i;
	for (i = 0; i < 16; i++)
		printf("0x%x ",value[i] );



	counter = 0;
	while ((temp = fgetc(NESROMfile)) != EOF) {
		
		counter++;
	}

	printf("\n\n\nFile Size is  :  0x%x B", counter);

	fclose(NESROMfile);



	return 0;

}