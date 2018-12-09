// xImaCodec.cpp : Encode Decode functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 5.99c 17/Oct/2004
 */
#include "../StdAfx.h"
#include "ximage.h"

#ifndef __IPPCORE_H__
#include "ippcore.h"
#endif

#include "jpegbase.h"

#if CXIMAGE_SUPPORT_JPG
#include "ximajpg.h"
#endif

#if CXIMAGE_SUPPORT_GIF
#include "ximagif.h"
#endif

#if CXIMAGE_SUPPORT_PNG
#include "ximapng.h"
#endif

#if CXIMAGE_SUPPORT_MNG
#include "ximamng.h"
#endif

#if CXIMAGE_SUPPORT_BMP
#include "ximabmp.h"
#endif

#if CXIMAGE_SUPPORT_ICO
#include "ximaico.h"
#endif

#if CXIMAGE_SUPPORT_TIF
#include "ximatif.h"
#endif

#if CXIMAGE_SUPPORT_TGA
#include "ximatga.h"
#endif

#if CXIMAGE_SUPPORT_PCX
#include "ximapcx.h"
#endif

#if CXIMAGE_SUPPORT_WBMP
#include "ximawbmp.h"
#endif

#if CXIMAGE_SUPPORT_WMF
#include "ximawmf.h" // <vho> - WMF/EMF support
#endif

#if CXIMAGE_SUPPORT_J2K
#include "ximaj2k.h"
#endif

#if CXIMAGE_SUPPORT_JBG
#include "ximajbg.h"
#endif

#if CXIMAGE_SUPPORT_JASPER
#include "ximajas.h"
#endif

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
bool CxImage::EncodeSafeCheck(CxFile *hFile)
{
	if (hFile==NULL) {
		strcpy(info.szLastError,CXIMAGE_ERR_NOFILE);
		return true;
	}

	if (pDib==NULL){
		strcpy(info.szLastError,CXIMAGE_ERR_NOIMAGE);
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
//#ifdef WIN32
//bool CxImage::Save(LPCWSTR filename, DWORD imagetype)
//{
//	FILE* hFile;	//file handle to write the image
//	if ((hFile=_wfopen(filename,L"wb"))==NULL)  return false;
//	bool bOK = Encode(hFile,imagetype);
//	fclose(hFile);
//	return bOK;
//}
//#endif //WIN32
////////////////////////////////////////////////////////////////////////////////
// For UNICODE support: char -> TCHAR
/**
 * Saves to disk the image in a specific format.
 * \param filename: file name
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Save(const char * filename, DWORD imagetype)
{
	FILE* hFile;	//file handle to write the image

#ifdef WIN32
	if ((hFile=_tfopen(filename,_T("wb")))==NULL)  return false;	// For UNICODE support
#else
	if ((hFile=fopen(filename,"wb"))==NULL)  return false;
#endif

	bool bOK = Encode(hFile,imagetype);
	fclose(hFile);
	return bOK;
}
bool CxImage::Save()
{
	FILE* hFile;	//file handle to write the image

#ifdef WIN32
	if ((hFile=_tfopen(ippFileName,_T("wb")))==NULL)  return false;	// For UNICODE support
#else
	if ((hFile=fopen(ippFileName,"wb"))==NULL)  return false;
#endif

	bool bOK = Encode(hFile,info.dwType);
	fclose(hFile);
	return bOK;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to disk the image in a specific format.
 * \param hFile: file handle, open and enabled for writing.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Encode(FILE *hFile, DWORD imagetype)
{
	CxIOFile file(hFile);
	return Encode(&file,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to memory buffer the image in a specific format.
 * \param buffer: output memory buffer pointer. Must be NULL,
 * the function allocates and fill the memory,
 * the application must free the buffer, see also FreeMemory().
 * \param size: output memory buffer size.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Encode(BYTE * &buffer, long &size, DWORD imagetype)
{
	if (buffer!=NULL){
		strcpy(info.szLastError,"the buffer must be empty");
		return false;
	}
	CxMemFile file;
	file.Open();
	if(Encode(&file,imagetype)){
		buffer=file.GetBuffer();
		size=file.Size();
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to disk the image in a specific format.
 * \param hFile: file handle (implemented using CxMemFile or CxIOFile),
 * open and enabled for writing.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 * \sa ENUM_CXIMAGE_FORMATS
 */
bool CxImage::Encode(CxFile *hFile, DWORD imagetype)
{

#if CXIMAGE_SUPPORT_BMP

	if (imagetype==CXIMAGE_FORMAT_BMP){
		CxImageBMP newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_ICO
	if (imagetype==CXIMAGE_FORMAT_ICO){
		CxImageICO newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_TIF
	if (imagetype==CXIMAGE_FORMAT_TIF){
		CxImageTIF newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_JPG
	if (imagetype==CXIMAGE_FORMAT_JPG){
		CxImageJPG newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_GIF
	if (imagetype==CXIMAGE_FORMAT_GIF){
		CxImageGIF newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_PNG
	if (imagetype==CXIMAGE_FORMAT_PNG){
		CxImagePNG newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_MNG
	if (imagetype==CXIMAGE_FORMAT_MNG){
		CxImageMNG newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_TGA
	if (imagetype==CXIMAGE_FORMAT_TGA){
		CxImageTGA newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_PCX
	if (imagetype==CXIMAGE_FORMAT_PCX){
		CxImagePCX newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_WBMP
	if (imagetype==CXIMAGE_FORMAT_WBMP){
		CxImageWBMP newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_WMF && CXIMAGE_SUPPORT_WINDOWS // <vho> - WMF/EMF support
	if (imagetype==CXIMAGE_FORMAT_WMF){
		CxImageWMF newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_J2K
	if (imagetype==CXIMAGE_FORMAT_J2K){
		CxImageJ2K newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_JBG
	if (imagetype==CXIMAGE_FORMAT_JBG){
		CxImageJBG newima;
		newima.Ghost(this);
		if (newima.Encode(hFile)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_JASPER
	if (
 #if	CXIMAGE_SUPPORT_JP2
		imagetype==CXIMAGE_FORMAT_JP2 ||
 #endif
 #if	CXIMAGE_SUPPORT_JPC
		imagetype==CXIMAGE_FORMAT_JPC ||
 #endif
 #if	CXIMAGE_SUPPORT_PGX
		imagetype==CXIMAGE_FORMAT_PGX ||
 #endif
 #if	CXIMAGE_SUPPORT_PNM
		imagetype==CXIMAGE_FORMAT_PNM ||
 #endif
 #if	CXIMAGE_SUPPORT_RAS
		imagetype==CXIMAGE_FORMAT_RAS ||
 #endif
		 false ){
		CxImageJAS newima;
		newima.Ghost(this);
		if (newima.Encode(hFile,imagetype)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif

	strcpy(info.szLastError,"Encode: Unknown format");
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to disk or memory pagecount images, referenced by an array of CxImage pointers.
 * \param hFile: file handle.
 * \param pImages: array of CxImage pointers.
 * \param pagecount: number of images.
 * \param imagetype: can be CXIMAGE_FORMAT_TIF or CXIMAGE_FORMAT_GIF.
 * \return true if everything is ok
 */
bool CxImage::Encode(FILE * hFile, CxImage ** pImages, int pagecount, DWORD imagetype)
{
	CxIOFile file(hFile);
	return Encode(&file, pImages, pagecount,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to disk or memory pagecount images, referenced by an array of CxImage pointers.
 * \param hFile: file handle (implemented using CxMemFile or CxIOFile).
 * \param pImages: array of CxImage pointers.
 * \param pagecount: number of images.
 * \param imagetype: can be CXIMAGE_FORMAT_TIF or CXIMAGE_FORMAT_GIF.
 * \return true if everything is ok
 */
bool CxImage::Encode(CxFile * hFile, CxImage ** pImages, int pagecount, DWORD imagetype)
{
#if CXIMAGE_SUPPORT_TIF
	if (imagetype==CXIMAGE_FORMAT_TIF){
		CxImageTIF newima;
		newima.Ghost(this);
		if (newima.Encode(hFile,pImages,pagecount)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_GIF
	if (imagetype==CXIMAGE_FORMAT_GIF){
		CxImageGIF newima;
		newima.Ghost(this);
		if (newima.Encode(hFile,pImages,pagecount)){
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
	strcpy(info.szLastError,"Multipage Encode, Unsupported operation for this format");
	return false;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * exports the image into a RGBA buffer, Useful for OpenGL applications.
 * \param buffer: output memory buffer pointer. Must be NULL,
 * the function allocates and fill the memory,
 * the application must free the buffer, see also FreeMemory().
 * \param size: output memory buffer size.
 * \return true if everything is ok
 */
bool CxImage::Encode2RGBA(BYTE * &buffer, long &size)
{
	if (buffer!=NULL){
		strcpy(info.szLastError,"the buffer must be empty");
		return false;
	}
	CxMemFile file;
	file.Open();
	if(Encode2RGBA(&file)){
		buffer=file.GetBuffer();
		size=file.Size();
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * exports the image into a RGBA buffer, Useful for OpenGL applications.
 * \param hFile: file handle (implemented using CxMemFile or CxIOFile).
 * \return true if everything is ok
 */
bool CxImage::Encode2RGBA(CxFile *hFile)
{
	if (EncodeSafeCheck(hFile)) return false;

	for (DWORD y = 0; y<GetHeight(); y++) {
		for(DWORD x = 0; x<GetWidth(); x++) {
			RGBQUAD color = BlindGetPixelColor(x,y);
			hFile->PutC(color.rgbRed);
			hFile->PutC(color.rgbGreen);
			hFile->PutC(color.rgbBlue);
			hFile->PutC(color.rgbReserved);
		}
	}
	return true;
}

bool CxImage::IppEncode(unsigned char* imgData, int imgWidth, int imgHeight,int imgChannel, int* ippLen,unsigned char* imgOutPut,int quality)
{
//	* ippLen = 0;

	CxImageJPG newima;
	newima.Ghost(this);

	return newima.IppEncode(imgData, imgWidth, imgHeight,imgChannel, ippLen,imgOutPut,quality);
}

////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
// For UNICODE support: char -> TCHAR
/**
 * Reads from disk the image in a specific format.
 * - If decoding fails using the specified image format,
 * the function will try the automatic file format recognition.
 *
 * \param filename: file name
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Load(const char * filename, DWORD imagetype)
//bool CxImage::Load(const char * filename, DWORD imagetype)
{
	/*FILE* hFile;	//file handle to read the image
	if ((hFile=fopen(filename,"rb"))==NULL)  return false;
	bool bOK = Decode(hFile,imagetype);
	fclose(hFile);*/

	/* automatic file type recognition */
	ippFileName = filename;
	bool bOK = false;
	if ( imagetype > 0 && imagetype < CMAX_IMAGE_FORMATS ){
		FILE* hFile;	//file handle to read the image

#ifdef WIN32
		if ((hFile=_tfopen(filename,_T("rb")))==NULL)  return false;	// For UNICODE support
#else
		if ((hFile=fopen(filename,"rb"))==NULL)  return false;
#endif

		bOK = Decode(hFile,imagetype);
		fclose(hFile);
		if (bOK) return bOK;
	}

	char szError[256];
	strcpy(szError,info.szLastError); //save the first error

	// if failed, try automatic recognition of the file...
	FILE* hFile;

#ifdef WIN32
	if ((hFile=_tfopen(filename,_T("rb")))==NULL)  return false;	// For UNICODE support
#else
	if ((hFile=fopen(filename,"rb"))==NULL)  return false;
#endif

	bOK = Decode(hFile,CXIMAGE_FORMAT_UNKNOWN);
	fclose(hFile);

	if (!bOK && imagetype > 0) strcpy(info.szLastError,szError); //restore the first error

	return bOK;
}
////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
//bool CxImage::Load(LPCWSTR filename, DWORD imagetype)
//{
//	/*FILE* hFile;	//file handle to read the image
//	if ((hFile=_wfopen(filename, L"rb"))==NULL)  return false;
//	bool bOK = Decode(hFile,imagetype);
//	fclose(hFile);*/
//
//	/* automatic file type recognition */
//	bool bOK = false;
//	if ( imagetype > 0 && imagetype < CMAX_IMAGE_FORMATS ){
//		FILE* hFile;	//file handle to read the image
//		if ((hFile=_wfopen(filename,L"rb"))==NULL)  return false;
//		bOK = Decode(hFile,imagetype);
//		fclose(hFile);
//		if (bOK) return bOK;
//	}
//
//	char szError[256];
//	strcpy(szError,info.szLastError); //save the first error
//
//	// if failed, try automatic recognition of the file...
//	FILE* hFile;	//file handle to read the image
//	if ((hFile=_wfopen(filename,L"rb"))==NULL)  return false;
//	bOK = Decode(hFile,CXIMAGE_FORMAT_UNKNOWN);
//	fclose(hFile);
//
//	if (!bOK && imagetype > 0) strcpy(info.szLastError,szError); //restore the first error
//
//	return bOK;
//}
////////////////////////////////////////////////////////////////////////////////
/**
 * Loads an image from the application resources.
 * \param hRes: the resource handle returned by FindResource().
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS.
 * \param hModule: NULL for internal resource, or external application/DLL hinstance returned by LoadLibray.
 * \return true if everything is ok
 */
bool CxImage::LoadResource(HRSRC hRes, DWORD imagetype, HMODULE hModule)
{
	DWORD rsize=SizeofResource(hModule,hRes);
	HGLOBAL hMem=::LoadResource(hModule,hRes);
	if (hMem){
		char* lpVoid=(char*)LockResource(hMem);
		if (lpVoid){
			// FILE* fTmp=tmpfile(); doesn't work with network
			/*char tmpPath[MAX_PATH] = {0};
			char tmpFile[MAX_PATH] = {0};
			GetTempPath(MAX_PATH,tmpPath);
			GetTempFileName(tmpPath,"IMG",0,tmpFile);
			FILE* fTmp=fopen(tmpFile,"w+b");
			if (fTmp){
				fwrite(lpVoid,rsize,1,fTmp);
				fseek(fTmp,0,SEEK_SET);
				bool bOK = Decode(fTmp,imagetype);
				fclose(fTmp);
				DeleteFile(tmpFile);
				return bOK;
			}*/

			CxMemFile fTmp((BYTE*)lpVoid,rsize);
			return Decode(&fTmp,imagetype);
		}
	} else strcpy(info.szLastError,"Unable to load resource!");
	return false;
}
#endif //WIN32
////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor from file name, see Load()
 * \param filename: file name
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 */
//
// > filename: file name
// > imagetype: specify the image format (CXIMAGE_FORMAT_BMP,...)
// For UNICODE support: char -> TCHAR
CxImage::CxImage(const char * filename, DWORD imagetype, bool thumbnail)
//CxImage::CxImage(const char * filename, DWORD imagetype)
{

	Startup(imagetype,filename,thumbnail);
	if(!thumbnail)
	{
		Load(filename,imagetype);
	}
	/*else
	{
		bppFileName = filename;
	}*/
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor from file handle, see Decode()
 * \param stream: file handle, with read access.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 */
CxImage::CxImage(FILE * stream, DWORD imagetype)
{
	Startup(imagetype);
	Decode(stream,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor from CxFile object, see Decode()
 * \param stream: file handle (implemented using CxMemFile or CxIOFile), with read access.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 */
CxImage::CxImage(CxFile * stream, DWORD imagetype)
{
	Startup(imagetype);
	Decode(stream,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor from memory buffer, see Decode()
 * \param buffer: memory buffer
 * \param size: size of buffer
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 */
CxImage::CxImage(BYTE * buffer, DWORD size, DWORD imagetype)
{
	Startup(imagetype);
	CxMemFile stream(buffer,size);
	Decode(&stream,imagetype);
}


char* CxImage::ReadIppImage(const char * filename,DWORD imagetype, int iMax, int& imageLength, int& sz)
{
	if(iMax == 0)return NULL;

	ippJpegData = 0;
	FILE* hFile;	//file handle to read the image

#ifdef WIN32
	if ((hFile=_tfopen(filename,_T("rb")))==NULL)  return false;	// For UNICODE support
#else
	if ((hFile=fopen(filename,"rb"))==NULL)  return false;
#endif

	CxIOFile file(hFile);
	CxImageJPG newima;

	//开始读取JPG数据
	if(imagetype == 3)
	{
		ippJpegData = newima.ReadIppJPEG(&file,imageLength,iMax,sz);
		fclose(hFile);

		if(imageLength == 0) return NULL;
		if(NULL != ippJpegData)
		{
			//CFile file;

			//Transfer(newima);
			//小图高宽
			head.biWidth = newima.GetWidth();
			head.biHeight = newima.GetHeight();


			//原图高宽
			m_ippScrInfo.Height = newima.m_ippScrInfo.Height;
			m_ippScrInfo.Width = newima.m_ippScrInfo.Width;
			return (char*)ippJpegData;
		}
	}
	fclose(hFile);

	//Cximage正常读数据
	bool err = Load(filename,imagetype);

	if(!err)
	{
		return NULL;
	}

	if(NULL != info.pImage)
	{
		//原图高宽
		m_ippScrInfo.Height = head.biHeight;
		m_ippScrInfo.Width = head.biWidth;

		if (head.biWidth > iMax || head.biHeight > iMax)
		{
			//原图高或宽大于iMax

			//24位
			if (!this->IsGrayScale()) this->IncreaseBpp(24);
			this->SetTransIndex(-1);

			int width    = head.biWidth;
			int height   = head.biHeight;
			int channels = head.biBitCount >> 3;

			//按比例缩放

			double   m_ratio ;
			int      dstHeight ;
			int      dstWidth ;

			if(width >= height)
			{
				m_ratio = (double)iMax/(double)width;
				dstWidth = width * m_ratio;
				dstHeight = height * m_ratio;
			}
			else
			{
				m_ratio = (double)iMax/(double)height;
				dstWidth = width * m_ratio;
				dstHeight = height * m_ratio;
			}
			this->IppResize(dstWidth,dstHeight,IPPI_INTER_SUPER);

			sz = GetSize() - head.biSizeImage;

			imageLength = GetSize();
			ippJpegData = (unsigned char *) malloc(imageLength);

			//copy attributes
			memcpy(pDib,&head,sizeof(BITMAPINFOHEADER));

			//memcpy(ippJpegData,&hdr,min(14,sizeof(BITMAPFILEHEADER)));
			memcpy(ippJpegData,pDib,GetSize());

			return (char*)ippJpegData;
		}
	}

	imageLength = 0;
	return NULL;
}

char* CxImage::ReadThumbBmp(int * ippLen, int& sz)
{
	//初始化数据
	* ippLen = 0;
	ippJpegData = 0;

	//开始读取JPG小图
	if(bImgType == 3)
	{
		ippJpegData = ReadIppJPEGExif(ippLen,true);
		if(0 != ippJpegData)
		{

			//原图高宽
			//m_ippScrInfo.Height = head.biHeight;
			//m_ippScrInfo.Width = head.biWidth;

			CxImageJPG newima;
			//解码为BMP
			ippJpegData = newima.ReadJpgToBmp(ippJpegData,ippLen,sz);
			if(*ippLen == 0) return NULL;
			if(NULL != ippJpegData)
			{
			//	Transfer(newima);
				//只需要宽，高
				head.biWidth = newima.GetWidth();
				head.biHeight = newima.GetHeight();

				//原图高宽
				m_ippScrInfo.Height = newima.GetWidth();
				m_ippScrInfo.Width = newima.GetHeight();

				//返回数据
				return (char*)ippJpegData;
			}
		}
	}

	//用IPP读，正确则直接返回数据
	CxImageJPG newima;
	ippJpegData = newima.IppDecode(ippFileName,ippLen,sz);
//	if(*ippLen == 0) return NULL;
	if(NULL != ippJpegData && newima.GetWidth() != 0 && newima.GetHeight() != 0)
	{
		head.biWidth = newima.GetWidth();
		head.biHeight = newima.GetHeight();


		//原图高宽
		m_ippScrInfo.Height = newima.m_ippScrInfo.Height;
		m_ippScrInfo.Width = newima.m_ippScrInfo.Width;

		return (char*)ippJpegData;
	}

	//Cximage正常读数据
	bool err = Load(ippFileName,bImgType);

	if(!err)
	{
		return NULL;
	}

	int iMax = 160;

	if(NULL != info.pImage)
	{

		//原图高宽
		m_ippScrInfo.Height = head.biHeight;
		m_ippScrInfo.Width = head.biWidth;

		if (head.biWidth > iMax || head.biHeight > iMax)
		{
			//24位
			if (!this->IsGrayScale()) this->IncreaseBpp(24);
			this->SetTransIndex(-1);

			//原图高或宽大于iMax
			int width    = head.biWidth;
			int height   = head.biHeight;
			int channels = head.biBitCount >> 3;

			//按比例缩放

			double   m_ratio ;
			int      dstHeight ;
			int      dstWidth ;

			if(width >= height)
			{
				m_ratio = (double)iMax/(double)width;
				dstWidth = width * m_ratio;
				dstHeight = height * m_ratio;
			}
			else
			{
				m_ratio = (double)iMax/(double)height;
				dstWidth = width * m_ratio;
				dstHeight = height * m_ratio;
			}
			this->IppResize(dstWidth,dstHeight,IPPI_INTER_SUPER);

			sz = GetSize() - head.biSizeImage;

			* ippLen = GetSize();
			ippJpegData = (unsigned char *) malloc(* ippLen);

			//copy attributes
			memcpy(pDib,&head,sizeof(BITMAPINFOHEADER));

			//memcpy(ippJpegData,&hdr,min(14,sizeof(BITMAPFILEHEADER)));
			memcpy(ippJpegData,pDib,GetSize());

			head.biWidth = dstWidth;
			head.biHeight = dstHeight;

			head.biWidth = dstWidth;
			head.biHeight = dstHeight;

			return (char*)ippJpegData;
		}
		* ippLen = GetSize();
		ippJpegData = (unsigned char *) malloc(* ippLen);

		sz = GetSize() - head.biSizeImage;
		//copy attributes
		memcpy(pDib,&head,sizeof(BITMAPINFOHEADER));

		memcpy(ippJpegData,pDib,GetSize());

		return (char*)ippJpegData;
	}

	* ippLen = 0;
	return NULL;
}

bool CxImage::IppDataDestroy()
{

	if(0 != ippJpegData){free(ippJpegData);ippJpegData = 0;}
	return true;
}

char* CxImage::ReadIppThumb(int * ippLen)
{
	//初始化数据
	* ippLen = 0;
	if(0 != ippJPEG)
	{
		free(ippJPEG);
		ippJPEG = 0;
	}
//    TRACE1("err = %d\n", 2 );

	//开始读取JPG小图
	if(bImgType == 3)
	{
		ippJPEG = ReadIppJPEGExif(ippLen,false);
		if(0 != ippJPEG)
		{
			return (char*)ippJPEG;
		}
	}
//	TRACE1("err = %d\n", 3 );

	//没有找到EXIF小图
	if(*ippLen <= 0)
	{
		//Cximage正常读数据
		bool err = Load(ippFileName,bImgType);

		if(NULL != info.pImage)
		{
			//24位
			if (!this->IsGrayScale()) this->IncreaseBpp(24);
			this->SetTransIndex(-1);

			if (head.biWidth > 160 || head.biHeight > 160)
			{
				//按比例缩放

				double   m_ratio ;
				int      dstHeight ;
				int      dstWidth ;

				int width = head.biWidth;
				int height = head.biHeight;

				if(width >= height)
				{
					m_ratio = (double)160/(double)width;
					dstWidth = width * m_ratio;
					dstHeight = height * m_ratio;
				}
				else
				{
					m_ratio = (double)160/(double)height;
					dstWidth = width * m_ratio;
					dstHeight = height * m_ratio;
				}
				this->IppResize(dstWidth,dstHeight,IPPI_INTER_SUPER);
			}

			/**ippLen = this->head.biSize;

			long tmpLen=0;
			this->Encode(ippJPEG, tmpLen, 3);

			*ippLen = (int)tmpLen;*/

			CxImageJPG newima;
			newima.Ghost(this);
			ippJPEG = newima.IppEncode(ippLen);

			/*FILE * fp = fopen("C:\\test.jpg", "wb");
			fwrite(ippJPEG, *ippLen, 1, fp);
			fclose(fp);*/

			return (char*)ippJPEG;
		}
	}

	return NULL;
}

unsigned char* CxImage::ReadIppJPEGExif(int * ippLen, bool drwImage)
{
	    unsigned char* tmpData = 0;
		FILE* hFile;	//file handle to read the image

#ifdef WIN32
		if ((hFile=_tfopen(ippFileName,_T("rb")))==NULL)  return false;	// For UNICODE support
#else
		if ((hFile=fopen(ippFileName,"rb"))==NULL)  return false;
#endif

		CxIOFile file(hFile);

		CxImageJPG newima;
		if(drwImage)
			tmpData = newima.IppExifThumb(&file, ippLen);
		else
			tmpData = newima.IppJpgThumb(&file, ippLen);

		fclose(hFile);

		/*if(newima.skp_exifinfo.IsExif)
		{
			strncpy(skp_exifinfo.CameraModel, newima.m_exifinfo.CameraModel, 39);
			strncpy(skp_exifinfo.DateTime, newima.m_exifinfo.DateTime, 39);
			skp_exifinfo.CCDWidth = newima.m_exifinfo.CCDWidth;
			skp_exifinfo.FocalLength = newima.m_exifinfo.FocalLength;
		}*/

		if(NULL != tmpData)
		{
			Transfer(newima);
			return tmpData;
		}

		return NULL;
}
bool CxImage::ReadExifInfo(const char * filename)
{
	FILE* hfile = fopen(filename,"rb");
	CxIOFile file(hfile);
	CxImageJPG newima;
    bool bres = newima.DecodeExif(&file);
	if(bres)
	{
		strncpy(skp_exifinfo.Version, newima.m_exifinfo.Version, 4);
		strncpy(skp_exifinfo.CameraMake, newima.m_exifinfo.CameraMake, 31);
		strncpy(skp_exifinfo.CameraModel, newima.m_exifinfo.CameraModel, 39);
		strncpy(skp_exifinfo.DateTime, newima.m_exifinfo.DateTime, 19);
		skp_exifinfo.Height = newima.m_exifinfo.Height;
		skp_exifinfo.Width = newima.m_exifinfo.Width;
		skp_exifinfo.Orientation = newima.m_exifinfo.Orientation;
		skp_exifinfo.IsColor = newima.m_exifinfo.IsColor;
		skp_exifinfo.Process = newima.m_exifinfo.Process;
		skp_exifinfo.FlashUsed = newima.m_exifinfo.FlashUsed;
		skp_exifinfo.FocalLength = newima.m_exifinfo.FocalLength;
		skp_exifinfo.ExposureTime = newima.m_exifinfo.ExposureTime;
		skp_exifinfo.ApertureFNumber = newima.m_exifinfo.ApertureFNumber;
		skp_exifinfo.Distance = newima.m_exifinfo.Distance;
		skp_exifinfo.CCDWidth = newima.m_exifinfo.CCDWidth;
		skp_exifinfo.ExposureBias = newima.m_exifinfo.ExposureBias;
		skp_exifinfo.Whitebalance = newima.m_exifinfo.Whitebalance;
		skp_exifinfo.MeteringMode = newima.m_exifinfo.MeteringMode;
		skp_exifinfo.ExposureProgram = newima.m_exifinfo.ExposureProgram;
		skp_exifinfo.ISOequivalent = newima.m_exifinfo.ISOequivalent;
		skp_exifinfo.CompressionLevel = newima.m_exifinfo.CompressionLevel;
		skp_exifinfo.FocalplaneXRes = newima.m_exifinfo.FocalplaneXRes;
		skp_exifinfo.FocalplaneYRes = newima.m_exifinfo.FocalplaneYRes;
		skp_exifinfo.FocalplaneUnits = newima.m_exifinfo.FocalplaneUnits;
		skp_exifinfo.Xresolution = newima.m_exifinfo.Xresolution;
		skp_exifinfo.Yresolution = newima.m_exifinfo.Yresolution;
		skp_exifinfo.ResolutionUnit = newima.m_exifinfo.ResolutionUnit;
		skp_exifinfo.Brightness = newima.m_exifinfo.Brightness;
		strncpy(skp_exifinfo.Comments, newima.m_exifinfo.Comments, MAX_COMMENT-1);
		//SetExifInfo(newima);
	}

	fclose(hfile);

	return bres;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Loads an image from memory buffer
 * \param buffer: memory buffer
 * \param size: size of buffer
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Decode(BYTE * buffer, DWORD size, DWORD imagetype)
{
	CxMemFile file(buffer,size);
	return Decode(&file,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Loads an image from file handle.
 * \param hFile: file handle, with read access.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Decode(FILE *hFile, DWORD imagetype)
{
	CxIOFile file(hFile);
	return Decode(&file,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Loads an image from CxFile object
 * \param hFile: file handle (implemented using CxMemFile or CxIOFile), with read access.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 * \sa ENUM_CXIMAGE_FORMATS
 */
bool CxImage::Decode(CxFile *hFile, DWORD imagetype)
{

	if (imagetype==CXIMAGE_FORMAT_UNKNOWN){
		DWORD pos = hFile->Tell();
#if CXIMAGE_SUPPORT_BMP
		{ CxImageBMP newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_JPG
		{ CxImageJPG newima; newima.CopyInfo(*this); if (newima.CxDecode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_ICO
		{ CxImageICO newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_GIF
		{ CxImageGIF newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_PNG
		{ CxImagePNG newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_TIF
		{ CxImageTIF newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_MNG
		{ CxImageMNG newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_TGA
		{ CxImageTGA newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_PCX
		{ CxImagePCX newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_WBMP
		{ CxImageWBMP newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_WMF && CXIMAGE_SUPPORT_WINDOWS
		{ CxImageWMF newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_J2K
		{ CxImageJ2K newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
#if CXIMAGE_SUPPORT_JBG
		{ CxImageJBG newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
#endif
//#if CXIMAGE_SUPPORT_JASPER
//		{ CxImageJAS newima; newima.CopyInfo(*this); if (newima.Decode(hFile)) { Transfer(newima); return true; } else hFile->Seek(pos,SEEK_SET); }
//#endif
	}

#if CXIMAGE_SUPPORT_BMP
	if (imagetype==CXIMAGE_FORMAT_BMP){
		CxImageBMP newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_JPG
	if (imagetype==CXIMAGE_FORMAT_JPG){
		CxImageJPG newima;
		newima.CopyInfo(*this); // <ignacio>
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_ICO
	if (imagetype==CXIMAGE_FORMAT_ICO){
		CxImageICO newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			info.nNumFrames = newima.info.nNumFrames;
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_GIF
	if (imagetype==CXIMAGE_FORMAT_GIF){
		CxImageGIF newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			info.nNumFrames = newima.info.nNumFrames;
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_PNG
	if (imagetype==CXIMAGE_FORMAT_PNG){
		CxImagePNG newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_TIF
	if (imagetype==CXIMAGE_FORMAT_TIF){
		CxImageTIF newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			info.nNumFrames = newima.info.nNumFrames;
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_MNG
	if (imagetype==CXIMAGE_FORMAT_MNG){
		CxImageMNG newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			info.nNumFrames = newima.info.nNumFrames;
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_TGA
	if (imagetype==CXIMAGE_FORMAT_TGA){
		CxImageTGA newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_PCX
	if (imagetype==CXIMAGE_FORMAT_PCX){
		CxImagePCX newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_WBMP
	if (imagetype==CXIMAGE_FORMAT_WBMP){
		CxImageWBMP newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_WMF && CXIMAGE_SUPPORT_WINDOWS // vho - WMF support
	if (imagetype == CXIMAGE_FORMAT_WMF){
		CxImageWMF newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_J2K
	if (imagetype==CXIMAGE_FORMAT_J2K){
		CxImageJ2K newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_JBG
	if (imagetype==CXIMAGE_FORMAT_JBG){
		CxImageJBG newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif
/*#if CXIMAGE_SUPPORT_JASPER
	if (
 #if	CXIMAGE_SUPPORT_JP2
		imagetype==CXIMAGE_FORMAT_JP2 ||
 #endif
 #if	CXIMAGE_SUPPORT_JPC
		imagetype==CXIMAGE_FORMAT_JPC ||
 #endif
 #if	CXIMAGE_SUPPORT_PGX
		imagetype==CXIMAGE_FORMAT_PGX ||
 #endif
 #if	CXIMAGE_SUPPORT_PNM
		imagetype==CXIMAGE_FORMAT_PNM ||
 #endif
 #if	CXIMAGE_SUPPORT_RAS
		imagetype==CXIMAGE_FORMAT_RAS ||
 #endif
		 false ){
		CxImageJAS newima;
		newima.CopyInfo(*this);
		if (newima.Decode(hFile,imagetype)){
			Transfer(newima);
			return true;
		} else {
			strcpy(info.szLastError,newima.GetLastError());
			return false;
		}
	}
#endif*/

	strcpy(info.szLastError,"Decode: Unknown or wrong format");
	return false;
}
////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////










































