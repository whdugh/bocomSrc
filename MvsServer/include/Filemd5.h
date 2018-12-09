#ifndef FILEMD5_H
#define FILEMD5_H
#include "global.h"

extern "C"{
	void FileMd5();
	string GetMd5(char* strFileName);
};
#endif
