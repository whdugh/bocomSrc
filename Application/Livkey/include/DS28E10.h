//This head file includes the function prototype of OWBASIC.C
#include	"PORT.h"
#include	"OWERROR.h"

uint8	getran(uint8*);
uint8	setromid(uint8*);
uint8	getromid(uint8* id);
uint8	set64secret(uint8,uint8* secret);
uint8	auth64(uint8,uint8,uint8,uint8*,uint8*);
uint8 	readmemo(uint8, uint8, uint8, uint8, uint8*);
uint8	writememo(uint8, uint8, uint8, uint8, uint8, uint8*);
uint8	set256secret(uint8,uint8);
uint8	auth320(uint8,uint8, uint8);
uint8   CalculateSHA(uint8* SRC,uint8* DST);
