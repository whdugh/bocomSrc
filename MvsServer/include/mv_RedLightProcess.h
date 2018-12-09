// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//



#ifndef MV_REDLIGHT_PROCESS_H_
#define MV_REDLIGHT_PROCESS_H_
#include <stdio.h>
#include "cxcore.h"
#include "cv.h"
#include "ml.h"
#include "highgui.h"

#ifndef MAX_OF_TWO
#define MAX_OF_TWO(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN_OF_TWO
#define MIN_OF_TWO(a,b)            (((a) < (b)) ? (a) : (b))
#endif


typedef struct RGBCOLOR {
	uchar    rgbBlue;
	uchar    rgbGreen;
	uchar    rgbRed;	
} RGBCOLOR;
class mv_RedLightProcess
{
public:

	mv_RedLightProcess(float fscale);
	~mv_RedLightProcess();

	void ProcessSinglePic_ForDsp( IplImage* img0, bool bTurnLeft, bool bFoward, bool bTurnRight,
		CvRect roiLeftLight_red, CvRect roiLeftLight_green,
		CvRect roiMidLight_red, CvRect roiMidLight_green,
		CvRect roiRightLight_red, CvRect roiRightLight_green, CvPoint m_affparams );


private:
	void LigCorrect(IplImage * ImgData, CvRect RecLig, int nLghtSort);
	void mvFillHoleImage( IplImage *pImage );

	void mvHSLToRGB(RGBCOLOR *hsl, RGBCOLOR *rgb);
	void mvRGBToHSLnew(RGBCOLOR *rgb, RGBCOLOR *hsl); 
	bool Filter( IplImage *pimg, CvRect vRect, long* kernel, long Ksize, long Kfactor, long Koffset );



	int VarIniRedH;
	int VarIniRedL;
	int VarIniGenH;
	int VarIniGenL;

	float fScale; 

};































#endif
