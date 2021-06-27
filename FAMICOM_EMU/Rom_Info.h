
// iNES ROM Info ---------------------

struct Rom_Info {	

	char File_Name[256];	
	char* Rom_Dir;

	char PRGSize;
	char CHRSize;
	char NTMirroring;
	char Mapper;

	char iNES_check[3] /*= { 'N','E','S' }*/;
};
