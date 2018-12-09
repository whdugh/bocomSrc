#ifndef _MV_TRANSFORM_
#define _MV_TRANSFORM_

//#define CARNUMGETCHAR_DEBUG_PRINTF_TRANSFER_PROCESS //打印变换矩阵计算过程

int mv_Saveimage( char *cFileName, uchar *pData, int nWidth, int nHeight );
int mv_Showimage( char *cFileName, unsigned char *pData, int nWidth, int nHeight, int flag );

enum IMAGE_FORMAT{GRAY_FORMAT,RGB_FORMAT,THREE_CHANNEL,YUV420,YUV422};
int mv_getPerspectiveTransform(CvPoint *SrcPoint, CvPoint *DstPoint ,double *Coef );
//int mv_getPerspectiveTransform_0(MvPoint *SrcPoint, MvPoint *DstPoint ,double *Coef );
//int mv_transformImg(unsigned char *SrcImg, int SrcW, int SrcH,unsigned char *DstImg, int DstW, int DstH, double *TransformMat );
int mv_transformImg(unsigned char *SrcImg,int SrcW,	int SrcH,unsigned char *DstImg,	int DstW,int DstH,double *TransformMat,	IMAGE_FORMAT imageformat);
int mv_IplImageTransformImg(IplImage *SrcImg, IplImage *DstImg, double *TransformMat);
int mv_IplImageTransformImg_new(IplImage *SrcImage, IplImage *DstImage, double *TransformMat);
CvPoint mv_PointTransform(CvPoint oriPoint, double *TransformMat );
int Mv_Threshold_OTUS(unsigned char *in_img,int img_width,CvRect rect );
int mv_Getippsum( int* image, int nWidth, int nHeight, CvRect rect );
void mv_GetInteImage( unsigned char *pCimage, int *pInteImage, int nWidth, int nHeight );

#endif