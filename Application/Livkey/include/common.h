#ifndef __COMMON_HH_
#define __COMMON_HH_

#if defined _X64_ || defined __x86_64__
#include "Clave2.h"
#else
#include "living1.h"
#pragma comment(lib, "living1.lib")
#endif

#include     <stdio.h>
#include     <stdlib.h> 
#include <string>


unsigned char WriteDataMemory(unsigned char* data);
std::string getStringMd5(unsigned char* buff,int nsize);

#endif