//public header. by qiansen

//#ifndef _STDAFX_H
//#define _STDAFX_H

#if !defined(__StdAfx_h) //cximage's style
#define __StdAfx_h

#include <stdlib.h> //for malloc() & free()

#include "./CxImage/ximadef.h" //for BITMAPINFOHEADER

/*
typedef unsigned long  DWORD;

typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	long       biWidth;
	long       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;
//*/

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;


#endif //#ifndef _STDAFX_H
