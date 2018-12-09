/*
 * File:	ximajpg.cpp
 * Purpose:	Platform Independent JPEG Image Class Loader and Writer
 * 07/Aug/2001 Davide Pizzolato - www.xdp.it
 * CxImage version 5.99c 17/Oct/2004
 */
//#include <time.h>
#include "../StdAfx.h"

#ifndef __MEMBUFFIN_H__
#include "membuffin.h"
#endif

#ifndef __MEMBUFFOUT_H__
#include "membuffout.h"
#endif

#ifndef __IPPIMAGE_H__
#include "ippimage.h"
#endif

#ifndef __JPEG_H__
#include "jpeg.h"
#endif

#ifndef __METADATA_H__
#include "metadata.h"
#endif
#ifndef __JPEGENC_H__
#include "jpegenc.h"
#endif
#ifndef __JPEGDEC_H__
#include "jpegdec.h"
#endif

#include "ximajpg.h"

#if CXIMAGE_SUPPORT_JPG

#include "../jpeg/jmorecfg.h"

#include "ximaiter.h"

#include <setjmp.h>

struct jpg_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;		/* for return to caller */
	char* buffer;				/* error message <CSC>*/
};
typedef jpg_error_mgr *jpg_error_ptr;

////////////////////////////////////////////////////////////////////////////////
// Here's the routine that will replace the standard error_exit method:
////////////////////////////////////////////////////////////////////////////////
static void
ima_jpeg_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	jpg_error_ptr myerr = (jpg_error_ptr) cinfo->err;
	/* Create the message */
	myerr->pub.format_message (cinfo, myerr->buffer);
	/* Send it to stderr, adding a newline */
	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

static void RevertScanLinesOrder(
								 Ipp8u*   pSrcDst,
								 int      step,
								 int      nchannels,
								 IppiSize roi)
{
	switch(nchannels)
	{
	case 1:
		ippiMirror_8u_C1IR(pSrcDst,step,roi,ippAxsHorizontal);
		break;

	case 3:
		ippiMirror_8u_C3IR(pSrcDst,step,roi,ippAxsHorizontal);
		break;

	case 4:
		ippiMirror_8u_C4IR(pSrcDst,step,roi,ippAxsHorizontal);
		break;

	default:
		break;
	}

	return;
} // RevertScanLinesOrder()
////////////////////////////////////////////////////////////////////////////////
CxImageJPG::CxImageJPG(): CxImage(CXIMAGE_FORMAT_JPG)
{
#if CXIMAGEJPG_SUPPORT_EXIF
	m_exif = NULL;
	memset(&m_exifinfo, 0, sizeof(EXIFINFO));
#endif
}
////////////////////////////////////////////////////////////////////////////////
CxImageJPG::~CxImageJPG()
{
#if CXIMAGEJPG_SUPPORT_EXIF
	if (m_exif) delete m_exif;
#endif
}
////////////////////////////////////////////////////////////////////////////////
#if CXIMAGEJPG_SUPPORT_EXIF
bool CxImageJPG::DecodeExif(CxFile * hFile)
{
	m_exif = new CxExifInfo(&m_exifinfo);
	if (m_exif){
		long pos=hFile->Tell();
		m_exif->DecodeExif(hFile);
		hFile->Seek(pos,SEEK_SET);
		return m_exif->m_exifinfo->IsExif;
	} else {
		return false;
	}
}
/*bool CxImageJPG::GetExif(CxFile * hFile)
{
	bool bres = DecodeExif(hFile);
	if(bres)
	{
		strncpy(skp_exifinfo.CameraModel, m_exifinfo.CameraModel, 39);
		strncpy(skp_exifinfo.DateTime, m_exifinfo.DateTime, 39);
		skp_exifinfo.CCDWidth = m_exifinfo.CCDWidth;
		skp_exifinfo.FocalLength = m_exifinfo.FocalLength;
	}

	return bres;
}*/
#endif //CXIMAGEJPG_SUPPORT_EXIF
////////////////////////////////////////////////////////////////////////////////
bool CxImageJPG::Decode(CxFile * hFile)
{
	if(this->bThumbnail)
		return IppThumbDecode(hFile);
	else
		return IppDecode(hFile);
}
/*//////////////////////////////////////////////////////////////////////////////
从文件中读取图片生成最大长度Resize为iMax的BMP数据
//////////////////////////////////////////////////////////////////////////////*/
unsigned char* CxImageJPG::ReadIppJPEG(CxFile* hFile, int& imageLength, int iMax, int& sz)
{
	int        width;
	int        height;
	int        channels;
	int        precision;
	JCOLOR     color;
	JSS        sampling;
	int        imageStep;
	int        imageSize;
	int        imageChannels;
	JCOLOR     imageColor;
	IppiSize   roi;
	JERRCODE   jerr;

	Ipp8u*     pJPEG = 0;
	Ipp8u*     m_imageData = 0;
	unsigned char*     ippData = 0;
	bool       bres     = TRUE;
	int        JPEGSize = (int)hFile->Size();
	CMemBuffInput in;

	//检查看是不是JPEG图
	if(JPEGSize<3)
	{
		imageLength = -1;
		return NULL;
	}
	CJPEGDecoder    decoder;
	CImageIterator  iter(this);

	pJPEG = (Ipp8u*)malloc(JPEGSize);
	if(NULL == pJPEG)
	{
		imageLength = -1;
		return NULL;
	}

	//正常读入全部图片数据
	hFile->Read(pJPEG,JPEGSize,1);
	hFile->Close();

	//粗略判断是否是JPEG图片
	if(pJPEG[0] != 255 || pJPEG[1] != 216 || pJPEG[2] != 255)
	{
		if(0 != pJPEG)
		{
			free(pJPEG);
			pJPEG = 0;
		}
		imageLength = -1;
		return NULL;
	}

	jerr = in.Open(pJPEG,JPEGSize);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	jerr = decoder.SetSource(&in);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	jerr = decoder.ReadHeader(&width,&height,&channels,&color,&sampling,&precision);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	sz = sizeof(BITMAPINFO);//增加

	if (width > iMax || height > iMax)
    {
		//原图长或宽大于iMax，重新读取大图解码

		//imageChannels = (channels == 1) ? 3 : channels;

		switch(channels)
		{
		case 1:
			imageColor    = JC_GRAY;
			imageChannels = 1;
			sz += sizeof(RGBQUAD)*256;
			break;

		case 3:
			imageColor    = JC_BGR;
			imageChannels = 3;
			break;

		//case 4:
			//imageColor    = JC_CMYK;
			//imageChannels = 4;
			//break;

		default:
			//color         = JC_UNKNOWN;
			//imageColor    = JC_UNKNOWN;
			//imageChannels = channels;
			//break;
			bres = FALSE;
			goto Exit;
		}

		imageStep = width*imageChannels + DIB_PAD_BYTES(width,imageChannels);
		imageSize = imageStep*height;

		roi.width  = width;
		roi.height = height;

		m_imageData = (Ipp8u*)malloc(imageSize);

		if(NULL == m_imageData)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetDestination(m_imageData,imageStep,roi,imageChannels,imageColor,JS_444,8);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadData();

		//原图高宽
		m_ippScrInfo.Height = height;
		m_ippScrInfo.Width = width;

		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		double  m_ratio ;
		int     dstHeight = height;
		int     dstWidth  = width;

		IppiRect srcroi={0,0,width,height};

		//按比例缩放
		if(width >= height)
		{
			m_ratio = (double)iMax/(double)width;
			dstWidth = roi.width * m_ratio;
			dstHeight = roi.height * m_ratio;
		}
		else
		{
			m_ratio = (double)iMax/(double)height;
			dstWidth = roi.width * m_ratio;
			dstHeight = roi.height * m_ratio;
		}

		head.biWidth = dstWidth;
		head.biHeight = dstHeight;

		int dstImgStep = dstWidth*imageChannels + DIB_PAD_BYTES(dstWidth,imageChannels);
		int dstImgSize = dstImgStep*dstHeight;

		IppiSize   dstRoi;
		dstRoi.width = dstWidth;
		dstRoi.height = dstHeight;

		imageLength = dstImgSize + sz;

		ippData = (unsigned char*)malloc(imageLength);

		BITMAPINFO* m_imageInfo = reinterpret_cast<BITMAPINFO*>(ippData);
		m_imageInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
		m_imageInfo->bmiHeader.biBitCount      = (WORD)(imageChannels << 3);
		m_imageInfo->bmiHeader.biWidth         = dstWidth;
		m_imageInfo->bmiHeader.biHeight        = dstHeight;
		m_imageInfo->bmiHeader.biPlanes        = 1;
		m_imageInfo->bmiHeader.biCompression   = BI_RGB;
		m_imageInfo->bmiHeader.biSizeImage     = 0;
		m_imageInfo->bmiHeader.biClrUsed       = imageChannels == 1 ? 256 : 0;
		m_imageInfo->bmiHeader.biClrImportant  = imageChannels == 1 ? 256 : 0;
		m_imageInfo->bmiHeader.biXPelsPerMeter = 0;
		m_imageInfo->bmiHeader.biYPelsPerMeter = 0;

		/*if(imageChannels == 3)
			ippiResize_8u_C3R( m_imageData, roi, imageStep, srcroi, ippData+sz, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );
		else if(imageChannels == 1)
			ippiResize_8u_C1R( m_imageData, roi, imageStep, srcroi, ippData+sz, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );
		else if(imageChannels == 4)
			ippiResize_8u_C4R( m_imageData, roi, imageStep, srcroi, ippData+sz, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );
*/
		/*FILE * fp = fopen("C:\\1.raw", "wb");
		fwrite(ippData+sz, dstImgSize, 1, fp);
		fclose(fp);*/

		if(imageChannels == 1)
		{
			for(int i = 0; i < 256; i++)
			{
				m_imageInfo->bmiColors[i].rgbBlue     = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbGreen    = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbRed      = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbReserved = (Ipp8u)0;
			}
		}

		imageStep = dstImgStep;
		roi = dstRoi;

		RevertScanLinesOrder(ippData+sz,imageStep,imageChannels,roi);
	}

Exit:

	if(0 != m_imageData)
	{
		free(m_imageData);
		m_imageData = 0;
	}

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}

//	decoder.Clean();

	if(bres)
	{
		return ippData;
	}

	imageLength = -1;

	return NULL;
}
/*//////////////////////////////////////////////////////////////////////////////
截段读取EXIF图片数据（显示）
//////////////////////////////////////////////////////////////////////////////*/
unsigned char* CxImageJPG::IppExifThumb(CxFile* hFile, int * ippLen)
{
	//int        width;
	//int        height;
	//int        imageStep;
	//int        imageSize;
	//int        imageChannels;
	//JCOLOR     imageColor;
	//IppiSize   roi;
	JERRCODE   jerr;

	unsigned char*      ippThumbImg;
	Ipp8u*     pJPEG          = 0;
	Ipp8u*     m_imageData    = 0;
	bool       bres           = FALSE;
	int        JPEGSize       = (int)hFile->Size();
	int        iExif          = 20000;
	CMemBuffInput in;

	try{

		//检查看是不是JPEG图
		if(JPEGSize<3)
		{
			return NULL;
		}
		CJPEGDecoder    decoder;
		CImageIterator  iter(this);

		pJPEG = (Ipp8u*)malloc(JPEGSize);
		if(NULL == pJPEG)
		{
			return NULL;
		}
		if(iExif > JPEGSize)
			return NULL;

		//正常读入全部图片数据
		hFile->Read(pJPEG,iExif,1);
		hFile->Close();

		//粗略判断是否是JPEG图片
		if(pJPEG[0] != 255 || pJPEG[1] != 216 || pJPEG[2] != 255)
		{
			if(0 != pJPEG)
			{
				free(pJPEG);
				pJPEG = 0;
			}
			return NULL;
		}

		jerr = in.Open(pJPEG,iExif);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetSource(&in);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		//jerr = decoder.ReadHeader(&width,&height,&channels,&color,&sampling,&precision);
		jerr = decoder.ParseExifBitStream(iExif);
		if(JPEG_OK != jerr)
		{
			if(JPEG_ERR_DATALENGTH == jerr)
			{
				//EXIF图片数据部分在20000长度以外，补充读取全部EXIF图片数据
				int iSurplus = decoder.GetExifAPP1DataSize() - iExif;

				hFile->Read(pJPEG+iExif,iSurplus,1);
				hFile->Close();

				iExif += iSurplus;

				in.Close();
				//decoder.Reset();

				jerr = in.Open(pJPEG,iExif);
				if(JPEG_OK != jerr)
				{
					bres = FALSE;
					goto Exit;
				}

				jerr = decoder.SetSource(&in);
				if(JPEG_OK != jerr)
				{
					bres = FALSE;
					goto Exit;
				}

				jerr = decoder.ParseExifBitStream(iExif);
				if(JPEG_OK != jerr)
				{
					bres = FALSE;
					goto Exit;
				}
			}
			else
			{
				bres = FALSE;
				goto Exit;
			}
		}

		/* =========================================================================== */
		/* =========================================================================== */

		if(decoder.IsExifAPP1Detected())
		{
			//读取EXIF INFO 里的大图宽、高
			/*m_exif = new CxExifInfo(&m_exifinfo);
			if (m_exif){
				m_exif->process_EXIF( pJPEG + decoder.m_exif_info_len, decoder.GetExifAPP1DataSize());

				head.biWidth = m_exif->m_exifinfo->Width;
				head.biHeight = m_exif->m_exifinfo->Height;

				if(head.biWidth <= 0 || head.biHeight <=0)
				{
					delete m_exif;
					m_exif = 0;

					bres = FALSE;
					goto Exit;
				}

				strncpy(skp_exifinfo.CameraModel, m_exif->m_exifinfo->CameraModel, 39);
				strncpy(skp_exifinfo.DateTime, m_exif->m_exifinfo->DateTime, 39);
				skp_exifinfo.CCDWidth    = m_exif->m_exifinfo->CCDWidth;
				skp_exifinfo.FocalLength = m_exif->m_exifinfo->FocalLength;
				skp_exifinfo.IsExif      = true;

				delete m_exif;
				m_exif = 0;
			}*/

			//head.biWidth = width;
			//head.biHeight = height;

			CJPEGMetaData* m_metadata = new CJPEGMetaData;

			try
			{
				m_metadata->ProcessAPP1_Exif(decoder.GetExifAPP1Data(),decoder.GetExifAPP1DataSize());

				* ippLen = m_metadata->GetJPEGThumbnailsLen();
				m_imageData = m_metadata->GetJPEGThumbnailsData();

				if(0 != m_metadata)
				{
					//m_metadata->Destroy();
					delete m_metadata;
					m_metadata = 0;
				}
			}
			catch (...) {

				if(0 != pJPEG)
				{
					free(pJPEG);
					pJPEG = 0;
				}

				if(0 != m_metadata)
				{
					//m_metadata->Destroy();
					delete m_metadata;
					m_metadata = 0;
				}

				return NULL;
			}

			if(* ippLen > 0)
			{
				ippThumbImg = (unsigned char*)malloc(* ippLen);
				memcpy(ippThumbImg,m_imageData,*ippLen);

				bres = TRUE;
			}
		}
	}
	catch (...) {

		if(0 != pJPEG)
		{
			free(pJPEG);
			pJPEG = 0;
		}

		return NULL;
	}

Exit:

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}

	if(bres)
	{
		return ippThumbImg;
	}

	return NULL;

}
/*//////////////////////////////////////////////////////////////////////////////
读取EXIF图片数据（入库）
//////////////////////////////////////////////////////////////////////////////*/
unsigned char* CxImageJPG::IppJpgThumb(CxFile* hFile, int * ippLen)
{
	int        width;
	int        height;
	int        channels;
	int        precision;
	JCOLOR     color;
	JSS        sampling;
	//int        imageStep;
	//int        imageSize;
	//int        imageChannels;
	//JCOLOR     imageColor;
	//IppiSize   roi;
	JERRCODE   jerr;

	unsigned char*      ippThumbImg;
	Ipp8u*     pJPEG          = 0;
	Ipp8u*     m_imageData    = 0;
	bool       bres           = FALSE;
	int        JPEGSize       = (int)hFile->Size();
	CMemBuffInput in;

	try{

		//检查看是不是JPEG图
		if(JPEGSize<3)
		{
			return NULL;
		}
		CJPEGDecoder    decoder;
		CImageIterator  iter(this);

		pJPEG = (Ipp8u*)malloc(JPEGSize);
		if(NULL == pJPEG)
		{
			return NULL;
		}

		//正常读入全部图片数据
		hFile->Read(pJPEG,JPEGSize,1);
		hFile->Close();

		//粗略判断是否是JPEG图片
		if(pJPEG[0] != 255 || pJPEG[1] != 216 || pJPEG[2] != 255)
		{
			if(0 != pJPEG)
			{
				free(pJPEG);
				pJPEG = 0;
			}
			return NULL;
		}

		jerr = in.Open(pJPEG,JPEGSize);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetSource(&in);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadHeader(&width,&height,&channels,&color,&sampling,&precision);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		/* =========================================================================== */
		/* =========================================================================== */

		if(decoder.IsExifAPP1Detected())
		{
			//读取EXIF INFO 里的大图宽、高
			/*m_exif = new CxExifInfo(&m_exifinfo);
			if (m_exif){
				m_exif->process_EXIF( pJPEG + decoder.m_exif_info_len, decoder.GetExifAPP1DataSize());

				head.biWidth = m_exif->m_exifinfo->Width;
				head.biHeight = m_exif->m_exifinfo->Height;

				if(head.biWidth <= 0 || head.biHeight <=0)
				{
					delete m_exif;
					m_exif = 0;

					bres = FALSE;
					goto Exit;
				}

				strncpy(skp_exifinfo.CameraModel, m_exif->m_exifinfo->CameraModel, 39);
				strncpy(skp_exifinfo.DateTime, m_exif->m_exifinfo->DateTime, 39);
				skp_exifinfo.CCDWidth    = m_exif->m_exifinfo->CCDWidth;
				skp_exifinfo.FocalLength = m_exif->m_exifinfo->FocalLength;
				skp_exifinfo.IsExif      = true;

				delete m_exif;
				m_exif = 0;
			}*/

			head.biWidth = width;
			head.biHeight = height;

			CJPEGMetaData* m_metadata = new CJPEGMetaData;

			try
			{
				m_metadata->ProcessAPP1_Exif(decoder.GetExifAPP1Data(),decoder.GetExifAPP1DataSize());

				* ippLen = m_metadata->GetJPEGThumbnailsLen();
				m_imageData = m_metadata->GetJPEGThumbnailsData();

				if(0 != m_metadata)
				{
					//m_metadata->Destroy();
					delete m_metadata;
					m_metadata = 0;
				}
			}
			catch (...) {

				if(0 != pJPEG)
				{
					free(pJPEG);
					pJPEG = 0;
				}

				if(0 != m_metadata)
				{
					//m_metadata->Destroy();
					delete m_metadata;
					m_metadata = 0;
				}

				return NULL;
			}

			if(* ippLen > 0)
			{
				ippThumbImg = (unsigned char*)malloc(* ippLen);
				memcpy(ippThumbImg,m_imageData,*ippLen);

				bres = TRUE;
			}
		}
	}
	catch (...) {

		if(0 != pJPEG)
		{
			free(pJPEG);
			pJPEG = 0;
		}

		return NULL;
	}

Exit:

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}

	if(bres)
	{
		return ippThumbImg;
	}

	return NULL;

}

unsigned char* CxImageJPG::ReadJpgToBmp(unsigned char* pJPEG,int * ippLen, int& sz)
{
    int        channels;
	int        precision;
	JCOLOR     color;
	JSS        sampling;
	int        imageStep;
	int        imageSize;
	JCOLOR     imageColor;
	IppiSize   roi;
	JERRCODE   jerr;
	unsigned char* m_pDIBBuffer = 0;
	int m_nWidth,m_nHeight;
	CMemBuffInput in;

	bool       bres     = TRUE;

	if(NULL == pJPEG || * ippLen <= 0)
	{
		return NULL;
	}

	CJPEGDecoder    decoder;


	jerr = in.Open(pJPEG,* ippLen);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	jerr = decoder.SetSource(&in);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	jerr = decoder.ReadHeader(&m_nWidth,&m_nHeight,&channels,&color,&sampling,&precision);

	head.biWidth = m_nWidth;
	head.biHeight = m_nHeight;

	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	{

		sz = sizeof(BITMAPINFO);

		switch(channels)
		{
		case 1:
			imageColor    = JC_GRAY;
			 sz += sizeof(RGBQUAD)*256;
			break;

		case 3:
			imageColor    = JC_BGR;
			break;

		case 4:
			imageColor    = JC_CMYK;
			break;

		default:
			color         = JC_UNKNOWN;
			imageColor    = JC_UNKNOWN;
			break;
		}

		imageStep = m_nWidth*channels + DIB_PAD_BYTES(m_nWidth,channels);
		imageSize = imageStep*m_nHeight;

		* ippLen = imageSize + sz;

		roi.width  = m_nWidth;
		roi.height = m_nHeight;

		m_pDIBBuffer = (unsigned char*)malloc(* ippLen);

		if(NULL == m_pDIBBuffer)
		{
			bres = FALSE;
			goto Exit;
		}

		BITMAPINFO* m_imageInfo = reinterpret_cast<BITMAPINFO*>(m_pDIBBuffer);
		m_imageInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
		m_imageInfo->bmiHeader.biBitCount      = (WORD)(channels << 3);
		m_imageInfo->bmiHeader.biWidth         = m_nWidth;
		m_imageInfo->bmiHeader.biHeight        = m_nHeight;
		m_imageInfo->bmiHeader.biPlanes        = 1;
		m_imageInfo->bmiHeader.biCompression   = BI_RGB;
		m_imageInfo->bmiHeader.biSizeImage     = 0;
		m_imageInfo->bmiHeader.biClrUsed       = channels == 1 ? 256 : 0;
		m_imageInfo->bmiHeader.biClrImportant  = channels == 1 ? 256 : 0;
		m_imageInfo->bmiHeader.biXPelsPerMeter = 0;
		m_imageInfo->bmiHeader.biYPelsPerMeter = 0;
		head.biWidth = m_nWidth;
		head.biHeight = m_nHeight;
		if(channels == 1)
		{
			for(int i = 0; i < 256; i++)
			{
				m_imageInfo->bmiColors[i].rgbBlue     = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbGreen    = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbRed      = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbReserved = (Ipp8u)0;
			}
		}

		jerr = decoder.SetDestination(m_pDIBBuffer+sz,imageStep,roi,channels,imageColor,JS_444,8);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadData();

		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		RevertScanLinesOrder(m_pDIBBuffer+sz,imageStep,channels,roi);
	}

Exit:

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}

//	decoder.Clean();

	if(bres)
	{
		return m_pDIBBuffer;
	}

	* ippLen = -1;

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
bool CxImageJPG::IppThumbDecode(CxFile * hFile)
{
	int        width;
	int        height;
	int        channels;
	int        precision;
	JCOLOR     color;
	JSS        sampling;
	int        imageStep;
	int        imageSize;
	int        imageChannels;
	JCOLOR     imageColor;
	IppiSize   roi;
	JERRCODE   jerr;

	Ipp8u*     pJPEG = 0;
	Ipp8u*     m_imageData = 0;
	bool       bres     = TRUE;
	int        JPEGSize = (int)hFile->Size();
	CMemBuffInput in;

	//检查看是不是JPEG图
	if(JPEGSize<3)
		return FALSE;


	CJPEGDecoder    decoder;
	CImageIterator  iter(this);

	try
	{

		pJPEG = (Ipp8u*)malloc(JPEGSize);
		if(NULL == pJPEG)
		{
			return FALSE;
		}

		//正常读入全部图片数据
		hFile->Read(pJPEG,JPEGSize,1);
		hFile->Close();

		//粗略判断是否是JPEG图片
		if(pJPEG[0] != 255 || pJPEG[1] != 216 || pJPEG[2] != 255)
		{
			if(0 != pJPEG)
			{
				free(pJPEG);
				pJPEG = 0;
			}

			return NULL;
		}

		jerr = in.Open(pJPEG,JPEGSize);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetSource(&in);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadHeader(&width,&height,&channels,&color,&sampling,&precision);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		//imageChannels = (channels == 1) ? 3 : channels;

		//int bmisz;
		switch(channels)
		{
		case 1:
		  imageColor    = JC_GRAY;
		  imageChannels = 1;
		  //bmisz = sizeof(BITMAPINFO) + sizeof(RGBQUAD)*256;
		  break;

		case 3:
		  imageColor    = JC_BGR;
		  imageChannels = 3;
		  //bmisz = sizeof(BITMAPINFO);
		  break;

		//case 4:
		  //imageColor    = JC_CMYK;
		  //imageChannels = 4;
		  //bmisz = sizeof(BITMAPINFO);
		  //break;

		default:
		  //color         = JC_UNKNOWN;
		  //imageColor    = JC_UNKNOWN;
		  //imageChannels = channels;
		  //bmisz = sizeof(BITMAPINFO);
		  //break;
			bres = FALSE;
			goto Exit;
		}

		imageStep = width*imageChannels + DIB_PAD_BYTES(width,imageChannels);
		imageSize = imageStep*height;

		roi.width  = width;
		roi.height = height;

		m_imageData = (Ipp8u*)malloc(imageSize);

		if(NULL == m_imageData)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetDestination(m_imageData,imageStep,roi,imageChannels,imageColor);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadData();
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		if (imageChannels > 1 && (width > 160 || height > 160))
		{
			//按比例缩放

			double   m_ratio ;
			int      dstHeight ;
			int      dstWidth ;

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

			int dstImgStep = dstWidth*imageChannels + DIB_PAD_BYTES(dstWidth,imageChannels);
			int dstImgSize = dstImgStep*dstHeight;

			IppiRect srcroi={0,0,width,height};
			IppiSize   dstRoi;
			dstRoi.width = dstWidth;
			dstRoi.height = dstHeight;

			//ippiResize_8u_C3R( m_imageData, roi, imageStep, srcroi, m_imageData, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );

			width = dstWidth;
			height = dstHeight;
		}


		Create(width, height, 8*channels, CXIMAGE_FORMAT_JPG);

		if (imageColor == JC_GRAY){
			SetGrayPalette();
			head.biClrUsed =256;
		}

		iter.Upset();
		for(int j=0;j<height;j++)
		{
			/* Assume put_scanline_someplace wants a pointer and sample count. */
			iter.SetRow(m_imageData+info.dwEffWidth*j, width*channels);
			iter.PrevRow();
		}

		m_ippScrInfo.Width  = roi.width;
		m_ippScrInfo.Height = roi.height;
	}
	catch (...) {

		if(0 != m_imageData)
		{
			free(m_imageData);
			m_imageData = 0;
		}

		if(0 != pJPEG)
		{
			free(pJPEG);
			pJPEG = 0;
		}

//		decoder.Clean();

		return FALSE;
	}

Exit:


	if(0 != m_imageData)
	{
		free(m_imageData);
		m_imageData = 0;
	}

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}


//	decoder.Clean();

	return bres;
}

////////////////////////////////////////////////////////////////////////////////
bool CxImageJPG::IppDecode(CxFile * hFile)
{
	int        width;
	int        height;
	int        channels;
	int        precision;
	JCOLOR     color;
	JSS        sampling;
	int        imageStep;
	int        imageSize;
	int        imageChannels;
	JCOLOR     imageColor;
	IppiSize   roi;
	JERRCODE   jerr;

	Ipp8u*     pJPEG = 0;
	Ipp8u*     m_imageData = 0;
	bool       bres     = TRUE;
	int        JPEGSize = (int)hFile->Size();
	CMemBuffInput in;

	//检查图像文件长度
	if(JPEGSize<3)
		return FALSE;

	CJPEGDecoder    decoder;
	CImageIterator  iter(this);

	try
	{
		pJPEG = (Ipp8u*)malloc(JPEGSize);
		if(NULL == pJPEG)
		{
			return FALSE;
		}

		//DecodeExif(hFile);

		//正常读入全部图片数据
		hFile->Read(pJPEG,JPEGSize,1);
		hFile->Close();

		//粗略判断是否是JPEG图片
		if(pJPEG[0] != 255 || pJPEG[1] != 216 || pJPEG[2] != 255)
		{
			if(0 != pJPEG)
			{
				free(pJPEG);
				pJPEG = 0;
			}

			return FALSE;
		}

		jerr = in.Open(pJPEG,JPEGSize);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetSource(&in);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadHeader(&width,&height,&channels,&color,&sampling,&precision);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}


		//int bmisz;
		switch(channels)
		{
		case 1:
		  imageColor    = JC_GRAY;
		  imageChannels = 1;
		  //bmisz = sizeof(BITMAPINFO) + sizeof(RGBQUAD)*256;
		  break;

		case 3:
		 // imageColor    = JC_BGR;
		  imageColor    = JC_RGB;
		  imageChannels = 3;
		  //bmisz = sizeof(BITMAPINFO);
		  break;

		//case 4:
		  //imageColor    = JC_CMYK;
		  //imageChannels = 4;
		  //bmisz = sizeof(BITMAPINFO);
		  //break;

		default:
		  //color         = JC_UNKNOWN;
		  //imageColor    = JC_UNKNOWN;
		  //imageChannels = channels;
		  //bmisz = sizeof(BITMAPINFO);
		  //break;
			bres = FALSE;
			goto Exit;
		}

		imageStep = width*imageChannels + DIB_PAD_BYTES(width,imageChannels);
		imageSize = imageStep*height;

		roi.width  = width;
		roi.height = height;

		m_imageData = (Ipp8u*)malloc(imageSize);

		if(NULL == m_imageData)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetDestination(m_imageData,imageStep,roi,imageChannels,imageColor,JS_444,8);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		/*clock_t start, finish;
		double  duration;

		start = clock();*/

		jerr = decoder.ReadData();

		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		/*finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		//TRACE1("duration = %f\n", duration );
		char tmp[100];
		sprintf(tmp,"%f",duration);
		MessageBox(NULL,tmp,NULL,MB_OK);*/

		Create(width, height, 8*channels, CXIMAGE_FORMAT_JPG);

		if (imageColor == JC_GRAY){
			SetGrayPalette();
			head.biClrUsed =256;
		}

		//iter.Upset();
		iter.Reset();
		for(int j=0;j<height;j++)
		{
			/* Assume put_scanline_someplace wants a pointer and sample count. */
			iter.SetRow(m_imageData+info.dwEffWidth*j, width*channels);
			//iter.PrevRow();
			iter.NextRow();
		}
		//RevertScanLinesOrder(m_imageData,imageStep,imageChannels,roi);
		//info.pImage = m_imageData;

	}
	catch (...) {

		if(0 != m_imageData)
		{
			free(m_imageData);
			m_imageData = 0;
		}

		if(0 != pJPEG)
		{
			free(pJPEG);
			pJPEG = 0;
		}

//		decoder.Clean();

		return FALSE;
	}

Exit:


	if(0 != m_imageData)
	{
		free(m_imageData);
		m_imageData = 0;
	}

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}

//	decoder.Clean();

	return bres;
}

bool CxImageJPG::CxDecode(CxFile * hFile)
{

	bool is_exif = false;

	CImageIterator iter(this);
	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler. <CSC> */
	struct jpg_error_mgr jerr;
	jerr.buffer=info.szLastError;
	/* More stuff */
	JSAMPARRAY buffer;	/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

	/* Step 1: allocate and initialize JPEG decompression object */
	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = ima_jpeg_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */
	//jpeg_stdio_src(&cinfo, infile);
	CxFileJpg src(hFile);
    cinfo.src = &src;

	/* Step 3: read file parameters with jpeg_read_header() */
	(void) jpeg_read_header(&cinfo, TRUE);

	/* Step 4 <chupeev> handle decoder options*/
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_GRAYSCALE) != 0)
		cinfo.out_color_space = JCS_GRAYSCALE;
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_QUANTIZE) != 0) {
		cinfo.quantize_colors = TRUE;
		cinfo.desired_number_of_colors = info.nQuality;
	}
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_DITHER) != 0)
		cinfo.dither_mode = m_nDither;
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_ONEPASS) != 0)
		cinfo.two_pass_quantize = FALSE;
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & DECODE_NOSMOOTH) != 0)
		cinfo.do_fancy_upsampling = FALSE;

//<DP>: Load true color images as RGB (no quantize)
/* Step 4: set parameters for decompression */
/*  if (cinfo.jpeg_color_space!=JCS_GRAYSCALE) {
 *	cinfo.quantize_colors = TRUE;
 *	cinfo.desired_number_of_colors = 128;
 *}
 */ //</DP>

	// Set the scale <ignacio>
	cinfo.scale_denom = GetJpegScale();

	// Borrowed the idea from GIF implementation <ignacio>
	if (info.nEscape == -1) {
		// Return output dimensions only
		jpeg_calc_output_dimensions(&cinfo);
		head.biWidth = cinfo.output_width;
		head.biHeight = cinfo.output_height;
		jpeg_destroy_decompress(&cinfo);
		return true;
	}

	/* Step 5: Start decompressor */
	jpeg_start_decompress(&cinfo);

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	*/
	//Create the image using output dimensions <ignacio>
	//Create(cinfo.image_width, cinfo.image_height, 8*cinfo.output_components, CXIMAGE_FORMAT_JPG);
	Create(cinfo.output_width, cinfo.output_height, 8*cinfo.output_components, CXIMAGE_FORMAT_JPG);

	if (!pDib) longjmp(jerr.setjmp_buffer, 1);  //<DP> check if the image has been created

	if (is_exif){
#if CXIMAGEJPG_SUPPORT_EXIF
	if ((m_exifinfo.Xresolution != 0.0) && (m_exifinfo.ResolutionUnit != 0))
		SetXDPI((long)(m_exifinfo.Xresolution/m_exifinfo.ResolutionUnit));
	if ((m_exifinfo.Yresolution != 0.0) && (m_exifinfo.ResolutionUnit != 0))
		SetYDPI((long)(m_exifinfo.Yresolution/m_exifinfo.ResolutionUnit));
#endif
	} else {
		if (cinfo.density_unit==2){
			SetXDPI((long)floor(cinfo.X_density * 254.0 / 10000.0 + 0.5));
			SetYDPI((long)floor(cinfo.Y_density * 254.0 / 10000.0 + 0.5));
		} else {
			SetXDPI(cinfo.X_density);
			SetYDPI(cinfo.Y_density);
		}
	}

	if (cinfo.out_color_space==JCS_GRAYSCALE){
		SetGrayPalette();
		head.biClrUsed =256;
	} else {
		if (cinfo.quantize_colors==TRUE){
			SetPalette(cinfo.actual_number_of_colors, cinfo.colormap[0], cinfo.colormap[1], cinfo.colormap[2]);
			head.biClrUsed=cinfo.actual_number_of_colors;
		} else {
			head.biClrUsed=0;
		}
	}

	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	iter.Upset();
	while (cinfo.output_scanline < cinfo.output_height) {

		if (info.nEscape) longjmp(jerr.setjmp_buffer, 1); // <vho> - cancel decoding

		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		// info.nProgress = (long)(100*cinfo.output_scanline/cinfo.output_height);
		//<DP> Step 6a: CMYK->RGB */
		if ((cinfo.num_components==4)&&(cinfo.quantize_colors==FALSE)){
			BYTE k,*dst,*src;
			dst=iter.GetRow();
			src=buffer[0];
			for(long x3=0,x4=0; x3<(long)info.dwEffWidth && x4<row_stride; x3+=3, x4+=4){
				k=src[x4+3];
				dst[x3]  =(BYTE)((k * src[x4+2])/255);
				dst[x3+1]=(BYTE)((k * src[x4+1])/255);
				dst[x3+2]=(BYTE)((k * src[x4+0])/255);
			}
		} else {
			/* Assume put_scanline_someplace wants a pointer and sample count. */
			iter.SetRow(buffer[0], row_stride);
		}
			iter.PrevRow();
	}

	/* Step 7: Finish decompression */
	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	//<DP> Step 7A: Swap red and blue components
	// not necessary if swapped red and blue definition in jmorecfg.h;ln322 <W. Morrison>
	if ((cinfo.num_components==3)&&(cinfo.quantize_colors==FALSE)){
		BYTE* r0=GetBits();
		for(long y=0;y<head.biHeight;y++){
			if (info.nEscape) longjmp(jerr.setjmp_buffer, 1); // <vho> - cancel decoding
			RGBtoBGR(r0,3*head.biWidth);
			r0+=info.dwEffWidth;
		}
	}

	/* Step 8: Release JPEG decompression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	/* And we're done! */
	return true;
}
////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
bool CxImageJPG::Encode(CxFile * hFile)
{
	if (EncodeSafeCheck(hFile)) return false;

	if (head.biClrUsed!=0 && !IsGrayScale()){
		strcpy(info.szLastError,"JPEG can save only RGB or GreyScale images");
		return false;
	}

	// necessary for EXIF, and for roll backs
	long pos=hFile->Tell();

	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	//struct jpeg_error_mgr jerr;
	/* We use our private extension JPEG error handler. <CSC> */
	struct jpg_error_mgr jerr;
	jerr.buffer=info.szLastError;
	/* More stuff */
	int row_stride;		/* physical row width in image buffer */
	JSAMPARRAY buffer;		/* Output row buffer */

	/* Step 1: allocate and initialize JPEG compression object */
	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/
	//cinfo.err = jpeg_std_error(&jerr); <CSC>
	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = ima_jpeg_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		strcpy(info.szLastError, jerr.buffer); //<CSC>
		jpeg_destroy_compress(&cinfo);
		return 0;
	}

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);
	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */
	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/

	//jpeg_stdio_dest(&cinfo, outfile);
	CxFileJpg dest(hFile);
    cinfo.dest = &dest;

	/* Step 3: set parameters for compression */
	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = GetWidth(); 	// image width and height, in pixels
	cinfo.image_height = GetHeight();

	if (IsGrayScale()){
		cinfo.input_components = 1;			// # of color components per pixel
		cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */
	} else {
		cinfo.input_components = 3; 	// # of color components per pixel
		cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
	}

	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/

//#ifdef C_ARITH_CODING_SUPPORTED
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_ARITHMETIC) != 0)
		cinfo.arith_code = TRUE;
//#endif

//#ifdef ENTROPY_OPT_SUPPORTED
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_OPTIMIZE) != 0)
		cinfo.optimize_coding = TRUE;
//#endif

	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_GRAYSCALE) != 0)
		jpeg_set_colorspace(&cinfo, JCS_GRAYSCALE);

	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_SMOOTHING) != 0)
		cinfo.smoothing_factor = m_nSmoothing;

	jpeg_set_quality(&cinfo, GetJpegQuality(), (GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_BASELINE) != 0);

//#ifdef C_PROGRESSIVE_SUPPORTED
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_PROGRESSIVE) != 0)
		jpeg_simple_progression(&cinfo);
//#endif

#ifdef C_LOSSLESS_SUPPORTED
	if ((GetCodecOption(CXIMAGE_FORMAT_JPG) & ENCODE_LOSSLESS) != 0)
		jpeg_simple_lossless(&cinfo, m_nPredictor, m_nPointTransform);
#endif

	cinfo.density_unit=1;
	cinfo.X_density=(unsigned short)GetXDPI();
	cinfo.Y_density=(unsigned short)GetYDPI();

	/* Step 4: Start compressor */
	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */
	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = info.dwEffWidth;	/* JSAMPLEs per row in image_buffer */

	//<DP> "8+row_stride" fix heap deallocation problem during debug???
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, 8+row_stride, 1);

	CImageIterator iter(this);

	iter.Upset();
	while (cinfo.next_scanline < cinfo.image_height) {
		// info.nProgress = (long)(100*cinfo.next_scanline/cinfo.image_height);
		iter.GetRow(buffer[0], row_stride);
		// not necessary if swapped red and blue definition in jmorecfg.h;ln322 <W. Morrison>
		if (head.biClrUsed==0){				 // swap R & B for RGB images
			RGBtoBGR(buffer[0], row_stride); // Lance : 1998/09/01 : Bug ID: EXP-2.1.1-9
		}
		iter.PrevRow();
		(void) jpeg_write_scanlines(&cinfo, buffer, 1);
	}

	/* Step 6: Finish compression */
	jpeg_finish_compress(&cinfo);

	/* Step 7: release JPEG compression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

/*#if CXIMAGEJPG_SUPPORT_EXIF
	if (m_exif && m_exif->m_exifinfo->IsExif){
		// discard useless sections (if any) read from original image
		m_exif->DiscardAllButExif();
		// read new created image, to split the sections
		hFile->Seek(pos,SEEK_SET);
		m_exif->DecodeExif(hFile,EXIF_READ_IMAGE);
		// save back the image, adding EXIF section
		hFile->Seek(pos,SEEK_SET);
		m_exif->EncodeExif(hFile);
	}
#endif*/

	/* And we're done! */
	return true;
}

////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_ENCODE
unsigned char* CxImageJPG::IppEncode(int * ippLen)
{
	bool bres = TRUE;
	CMemBuffOutput out;
	PARAMS_JPEG param;

	int          Channels;
	int          LineStep;
	IppiSize     roi;
	JSS          sampling  = JS_411;
	JCOLOR       in_color;
	JCOLOR       out_color;
	Ipp8u*       pJPEG;
	int          JPEGSize;
	JERRCODE     jerr;
//	CFile        jpeg; //no use, deleted by qiansen for linux
//	CString      string; //no use. deleted by qiansen

	roi.width  = head.biWidth;
	roi.height = head.biHeight;
	Channels   = head.biBitCount >> 3;

	LineStep = roi.width*Channels + DIB_PAD_BYTES(roi.width,Channels);
	JPEGSize = roi.width * roi.height * Channels;

	RevertScanLinesOrder(info.pImage,LineStep,Channels,roi);

	CJPEGEncoder encoder;

	// just for too small images
	if(JPEGSize < 1024)
	{
	JPEGSize = 4096;
	}

	pJPEG = (Ipp8u*)malloc(JPEGSize);
	if(0 == pJPEG)
	{
	return FALSE;
	}

	switch(Channels)
	{
	case 1:
	in_color  = JC_GRAY;
	out_color = JC_GRAY;
	sampling  = JS_444;
	break;

	case 3:
	in_color  = JC_BGR;
	out_color = JC_YCBCR;
	break;

	case 4:
	in_color  = JC_CMYK;
	out_color = JC_YCCK;
	break;

	default:
	in_color  = JC_UNKNOWN;
	out_color = JC_UNKNOWN;
	}

	jerr = encoder.SetSource(info.pImage,LineStep,roi,Channels,in_color);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	jerr = out.Open(pJPEG,JPEGSize);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	jerr = encoder.SetDestination(&out);
	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	param.color            = out_color;
	param.huffman_opt      = 0;
	param.mode             = JPEG_BASELINE;
	//param.point_transform  = 0;
	//param.predictor        = 1;
	param.quality          = 50;
	param.restart_interval = 0;
	param.sampling         = sampling;

	jerr = encoder.SetParams(
		param.mode,
		param.color,
		param.sampling,
		param.restart_interval,
		param.huffman_opt,
		param.quality);

	if(JPEG_OK != jerr)
	{
		bres = FALSE;
		goto Exit;
	}

	if(in_color == JC_CMYK)
	{
	BGRA_to_RGBA(info.pImage,roi.width,roi.height);
	}

	jerr = encoder.WriteHeader();
	if(JPEG_OK != jerr)
	{
	  bres = FALSE;
	  goto Exit;
	}

	jerr = encoder.WriteData();
	if(JPEG_OK != jerr)
	{
	  bres = FALSE;
	  goto Exit;
	}

	* ippLen = out.GetBufLen();

Exit:

	if(bres)
	{
	  return pJPEG;
	}
	return NULL;
}

bool CxImageJPG::IppEncode(unsigned char* pImgInput, int imgWidth, int imgHeight,int imgChannel, int * ippLen,unsigned char* pImgOutput,int quality)
{
  //  printf("====imgWidth=%d,imgHeight=%d,ippLen=%d\n",imgWidth,imgHeight,*ippLen);
  bool bres = TRUE;
  CMemBuffOutput out;
  PARAMS_JPEG param;

  int          Channels;
  int          LineStep;
  IppiSize     roi;
  JSS          sampling  = JS_411;
  JCOLOR       in_color;
  JCOLOR       out_color;
  Ipp8u*       pJPEG;
  int          JPEGSize;
  JERRCODE     jerr;
//  CFile        jpeg; //no use. deleted by qiansen
//  CString      string; //no use. deleted by qiansen

  roi.width  = imgWidth;
  roi.height = imgHeight;
  Channels   = imgChannel;

  if(*ippLen == 0)
  {
        LineStep = roi.width*Channels + DIB_PAD_BYTES(roi.width,Channels);
  }
  else
  {
        LineStep = *ippLen;
  }
  JPEGSize = roi.width * roi.height;
//printf("====LineStep=%d\n",LineStep);
  //RevertScanLinesOrder(imgData,LineStep,Channels,roi);

  CJPEGEncoder encoder;

  // just for too small images
  if(JPEGSize < 1024)
  {
    JPEGSize = 4096;
  }

  //pJPEG = (Ipp8u*)malloc(JPEGSize);
  pJPEG = (Ipp8u*)pImgOutput;
  if(0 == pJPEG)
  {
    return FALSE;
  }

  switch(Channels)
  {
  case 1:
    in_color  = JC_GRAY;
    out_color = JC_GRAY;
    sampling  = JS_444;
    break;

  case 3:
    in_color  = JC_RGB;
    out_color = JC_YCBCR;
    break;

  case 4:
    in_color  = JC_CMYK;
    out_color = JC_YCCK;
    break;

  default:
    in_color  = JC_UNKNOWN;
    out_color = JC_UNKNOWN;
  }

  jerr = encoder.SetSource(pImgInput,LineStep,roi,Channels,in_color);
  if(JPEG_OK != jerr)
  {
	  bres = FALSE;
	  goto Exit;
  }

  jerr = out.Open(pJPEG,JPEGSize);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

  jerr = encoder.SetDestination(&out);
  if(JPEG_OK != jerr)
  {
	  bres = FALSE;
	  goto Exit;
  }

  param.color            = out_color;
  param.huffman_opt      = 0;
  param.mode             = JPEG_BASELINE;
  //param.point_transform  = 0;
  //param.predictor        = 1;
  param.quality          =  quality;
  param.restart_interval = 0;
  param.sampling         = sampling;

  jerr = encoder.SetParams(
	  param.mode,
	  param.color,
	  param.sampling,
	  param.restart_interval,
	  param.huffman_opt,
	  param.quality);

  if(JPEG_OK != jerr)
  {
	  bres = FALSE;
	  goto Exit;
  }

  jerr = encoder.WriteHeader();
  if(JPEG_OK != jerr)
  {
	  bres = FALSE;
	  goto Exit;
  }

  jerr = encoder.WriteData();
  if(JPEG_OK != jerr)
  {
	  bres = FALSE;
	  goto Exit;
  }


  * ippLen = encoder.NumOfBytes();//out.GetBufLen();


Exit:

  /*if(0 != encoder)
  {
	  //encoder->Clean();
	  delete encoder;
	  encoder = 0;
  }*/

 /* if(bres)
  {
	  memcpy(pImgOutput,pJPEG,*ippLen);
  }
  if(0 != pJPEG)
  {
	  free(pJPEG);
	  pJPEG = 0;
  }*/

  return bres;
}


unsigned char* CxImageJPG::IppDecode(const char* filename,int * ippLen,int& sz)
{
    int        channels;
	int        precision;
	JCOLOR     color;
	JSS        sampling;
	int        imageStep;
	int        imageSize;
	JCOLOR     imageColor;
	IppiSize   roi;
	JERRCODE   jerr;
	int m_nWidth,m_nHeight;
	bool       bres     = TRUE;
	unsigned char* m_pDIBBuffer = NULL;
	CMemBuffInput in;
/*	// read jpeg file
	CFile jpeg;
	//打开文件
	bres = jpeg.Open(filename,CFile::modeRead|CFile::typeBinary);
	if(FALSE == bres) return FALSE;
	//文件长度
	int JPEGSize = (int)jpeg.GetLength();
	if(0 == JPEGSize) return FALSE;
	//分配内存
	Ipp8u*	pJPEG = (Ipp8u*)malloc(JPEGSize);
	if(0 == pJPEG)	return FALSE;

	jpeg.Read(pJPEG,JPEGSize);
	jpeg.Close(); //*/

	////////////////////////////////////////////////////////////
	//revised by qiansen for linux
	FILE *jpeg=NULL;
	//打开文件
	jpeg=fopen(filename,"rb");
	if(!jpeg) return FALSE;
	//文件长度
	int qs_position1=ftell(jpeg);
	fseek(jpeg,0,SEEK_END);
	int qs_position2=ftell(jpeg);
	int JPEGSize=qs_position2-qs_position1;//此变量下面要用到
	if(0 == JPEGSize) return FALSE;
	//分配内存
	Ipp8u*	pJPEG = (Ipp8u*)malloc(JPEGSize);
	if(0 == pJPEG)	return FALSE;
	//读文件数据
	fseek(jpeg,0,SEEK_SET);
	fread(pJPEG,sizeof(unsigned char),JPEGSize,jpeg);
	fseek(jpeg,0,SEEK_SET);//读完记得还原
	fclose(jpeg);
	//end of qiansen's read code
	////////////////////////////////////////////////////////////

	//粗略判断是否是JPEG图片
	if(pJPEG[0] != 255 || pJPEG[1] != 216 || pJPEG[2] != 255)
	{
		if(0 != pJPEG)
		{
			free(pJPEG);
			pJPEG = 0;
		}

		return FALSE;
	}

	CJPEGDecoder    decoder;
	{
		//
		jerr = in.Open(pJPEG,JPEGSize);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetSource(&in);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}
		//读JPG头
		jerr = decoder.ReadHeader(&m_nWidth,&m_nHeight,&channels,&color,&sampling,&precision);

		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		sz = sizeof(BITMAPINFO);

		switch(channels)
		{
		case 1:
			imageColor    = JC_GRAY;
			sz += sizeof(RGBQUAD)*256;
			break;

		case 3:
			imageColor    = JC_BGR;
			break;

		case 4:
			imageColor    = JC_CMYK;
			bres = FALSE;
			goto Exit;

		default:
			color         = JC_UNKNOWN;
			imageColor    = JC_UNKNOWN;
			break;
		}

		imageStep = m_nWidth*channels + DIB_PAD_BYTES(m_nWidth,channels);
		imageSize = imageStep*m_nHeight;

		* ippLen = imageSize + sz;

		roi.width  = m_nWidth;
		roi.height = m_nHeight;

		//分配BMP数据内存
		m_pDIBBuffer = (unsigned char*)malloc(* ippLen);

		if(NULL == m_pDIBBuffer)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.SetDestination(m_pDIBBuffer+sz,imageStep,roi,channels,imageColor,JS_444,8);
		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		jerr = decoder.ReadData();

		if(JPEG_OK != jerr)
		{
			bres = FALSE;
			goto Exit;
		}

		//原图高宽
		m_ippScrInfo.Height = m_nHeight;
		m_ippScrInfo.Width = m_nWidth;

		int iMax = 160;
		//判断大小
		if(m_nWidth <= iMax && m_nHeight <= iMax)
		{
			//填充头
			BITMAPINFO* m_imageInfo = reinterpret_cast<BITMAPINFO*>(m_pDIBBuffer);
			m_imageInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
			m_imageInfo->bmiHeader.biBitCount      = (WORD)(channels << 3);
			m_imageInfo->bmiHeader.biWidth         = m_nWidth;
			m_imageInfo->bmiHeader.biHeight        = m_nHeight;
			m_imageInfo->bmiHeader.biPlanes        = 1;
			m_imageInfo->bmiHeader.biCompression   = BI_RGB;
			m_imageInfo->bmiHeader.biSizeImage     = 0;
			m_imageInfo->bmiHeader.biClrUsed       = channels == 1 ? 256 : 0;
			m_imageInfo->bmiHeader.biClrImportant  = channels == 1 ? 256 : 0;
			m_imageInfo->bmiHeader.biXPelsPerMeter = 0;
			m_imageInfo->bmiHeader.biYPelsPerMeter = 0;
			head.biWidth = m_nWidth;
			head.biHeight = m_nHeight;
			if(channels == 1)
			{
				for(int i = 0; i < 256; i++)
				{
					m_imageInfo->bmiColors[i].rgbBlue     = (Ipp8u)i;
					m_imageInfo->bmiColors[i].rgbGreen    = (Ipp8u)i;
					m_imageInfo->bmiColors[i].rgbRed      = (Ipp8u)i;
					m_imageInfo->bmiColors[i].rgbReserved = (Ipp8u)0;
				}
			}
			//翻转
			RevertScanLinesOrder(m_pDIBBuffer+sz,imageStep,channels,roi);
			goto Exit;

		}
		//RESIZE
		double   m_ratio ;
		int      dstHeight ;
		int      dstWidth ;
		if(m_nWidth >= m_nHeight)
		{
			m_ratio = (double)iMax/(double)m_nWidth;
			dstWidth = m_nWidth * m_ratio;
			dstHeight = m_nHeight * m_ratio;
		}
		else
		{
			m_ratio = (double)iMax/(double)m_nHeight;
			dstWidth = m_nWidth * m_ratio;
			dstHeight = m_nHeight * m_ratio;
		}
		int dstImgStep = dstWidth*channels + DIB_PAD_BYTES(dstWidth,channels);

		IppiRect srcroi={0,0,m_nWidth,m_nHeight};
		IppiSize   dstRoi;

		dstRoi.width = dstWidth;
		dstRoi.height = dstHeight;


		/*if(channels == 3)
			ippiResize_8u_C3R( m_pDIBBuffer+ sz, roi, imageStep, srcroi, m_pDIBBuffer+sz, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );
		else if(channels == 1)
			ippiResize_8u_C1R( m_pDIBBuffer+ sz, roi, imageStep, srcroi, m_pDIBBuffer+sz, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );
		else if(channels == 4)
			ippiResize_8u_C4R( m_pDIBBuffer+ sz, roi, imageStep, srcroi, m_pDIBBuffer+sz, dstImgStep, dstRoi,m_ratio, m_ratio, IPPI_INTER_SUPER );
*/


		BITMAPINFO* m_imageInfo = reinterpret_cast<BITMAPINFO*>(m_pDIBBuffer);
		m_imageInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
		m_imageInfo->bmiHeader.biBitCount      = (WORD)(channels << 3);
		m_imageInfo->bmiHeader.biWidth         = dstWidth;
		m_imageInfo->bmiHeader.biHeight        = dstHeight;
		m_imageInfo->bmiHeader.biPlanes        = 1;
		m_imageInfo->bmiHeader.biCompression   = BI_RGB;
		m_imageInfo->bmiHeader.biSizeImage     = 0;
		m_imageInfo->bmiHeader.biClrUsed       = channels == 1 ? 256 : 0;
		m_imageInfo->bmiHeader.biClrImportant  = channels == 1 ? 256 : 0;
		m_imageInfo->bmiHeader.biXPelsPerMeter = 0;
		m_imageInfo->bmiHeader.biYPelsPerMeter = 0;
		head.biWidth = dstWidth;
		head.biHeight = dstHeight;

		if(channels == 1)
		{
			for(int i = 0; i < 256; i++)
			{
				m_imageInfo->bmiColors[i].rgbBlue     = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbGreen    = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbRed      = (Ipp8u)i;
				m_imageInfo->bmiColors[i].rgbReserved = (Ipp8u)0;
			}
		}

		* ippLen = dstImgStep * dstHeight + sz;
		//翻转
		RevertScanLinesOrder(m_pDIBBuffer+sz,dstImgStep,channels,dstRoi);
	}
Exit:

	if(0 != pJPEG)
	{
		free(pJPEG);
		pJPEG = 0;
	}

//	decoder.Clean();

	if(bres)
	{
		//返回数据
		return m_pDIBBuffer;
	}

	* ippLen = -1;

	return NULL;
}


void CxImageJPG::BGRA_to_RGBA(BYTE* data,int width,int height)
{
	int   i;
	int   j;
	int   pad;
	int   line_width;
	BYTE  r, g, b;
	BYTE* ptr;

	ptr = data;
	pad = DIB_PAD_BYTES(width,4);
	line_width = width * 4 + pad;

	for(i = 0; i < height; i++)
	{
		ptr = data + line_width*i;
		for(j = 0; j < width; j++)
		{
			b = ptr[0];
			g = ptr[1];
			r = ptr[2];
			ptr[0] = r;
			ptr[1] = g;
			ptr[2] = b;
			ptr += 4;
		}
	}

	return;
} // CxImageJPG::BGRA_to_RGBA()
////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_JPG











































