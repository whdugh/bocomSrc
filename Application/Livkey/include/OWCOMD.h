//This head file defines the commands used in one wire system

//The 1st part of the head file is the Rom function commands

#define		ReadRom		0x33
#define		MatcRom		0x55
#define		SeacRom		0xF0
#define		SkipRom		0xCC
#define		Resume		0xA5
#define		OvskRom		0x3C
#define		OvmaRom		0x69

//The next part is the Memory function commands (applied for DS2431 and DS28E01-100)
#define		Wrscratch	0x0F
#define		Rescratch	0xAA
#define		Coscratch	0x55
#define		Redmemory	0xF0

#define		Loadfirse	0x5A
#define		Comnextse	0x33
#define		Reautpage	0xA5
#define		Anautpage	0xCC
#define		Refreshsc	0xA3






