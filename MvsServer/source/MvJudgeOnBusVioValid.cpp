#include "MvJudgeOnBusVioValid.h"
#include <highgui.h>


MvJudgeOnBusVioValid::MvJudgeOnBusVioValid(void)
{
	m_pPlateCascade = NULL;
}


MvJudgeOnBusVioValid::~MvJudgeOnBusVioValid(void)
{
	if(m_pPlateCascade != NULL)
	{
		mvOnBusVioValidUnInit();
	}
}

bool MvJudgeOnBusVioValid::mvOnBusVioValidInit(const char *pName)
{
	bool bRet = true;

	m_pPlateCascade = (CvHaarClassifierCascade*)cvLoad(pName, 0, 0, 0);   //载入分类器       
	if( !m_pPlateCascade )    
	{   
		bRet = false;
		printf("Load XML Failed"); 
	} 

	return bRet;
}

void MvJudgeOnBusVioValid::mvOnBusVioValidUnInit()
{
	if(m_pPlateCascade)
	{
		cvReleaseHaarClassifierCascade(&m_pPlateCascade);
		m_pPlateCascade = 0;
	}
}

vector<CvRect> MvJudgeOnBusVioValid::mvDetectPlatePos(const IplImage *pImage)
{
	vector<CvRect> vecRet;

	if(!m_pPlateCascade || pImage == NULL || pImage->width < 15 || pImage->height < 15)
	{
		return vecRet;
	}

	CvMemStorage* storage = 0;
	double ThresholdRate=0.1;
	storage = cvCreateMemStorage(0);         
	cvClearMemStorage( storage ); 

	
	CvSeq* targets = cvHaarDetectObjects( pImage, m_pPlateCascade, storage,
		                     1.1, 2, 0,cvSize(14, 14));                        //多尺度检测函数

	for( int i = 0; i < (targets ? targets->total : 0); i++ )  
	{  
		CvRect* r = (CvRect*)cvGetSeqElem( targets, i );
		CvRect rt = *r;

		vecRet.push_back(rt);
	}

	cvReleaseMemStorage(&storage);

	return vecRet;
}

bool MvJudgeOnBusVioValid::verifySizesRt(CvRect rt)
{
	if(rt.width<30 || rt.width>300)
	{
		return false;
	}

	if ((rt.y+rt.height)<150)
	{
		return false;
	}

	float r = (float)rt.width / (float)rt.height;
	if(r<2 || r>8)
	{
		return false;
	}

	return true;
}

bool MvJudgeOnBusVioValid::IsHaveCarInPlateRight(const IplImage *pImg, vector<CvRect> &vecRt, CvRect rt)
{
	Mat input(pImg);

	Mat SmallImage;
	SmallImage.create(600,800, CV_8UC3);
	resize(input,SmallImage,SmallImage.size());
	
	Mat img_gray;
	cvtColor(SmallImage, img_gray, CV_BGR2GRAY);

	Mat img_blur;
	//Finde vertical lines. Car plates have high density of vertical lines
	bilateralFilter(img_gray, img_blur,5, 10,5/2);

	IplImage *pImgIn = &IplImage(img_blur);

	vector<CvRect> vecRet = mvDetectPlatePos(pImgIn);

	float xRatio = pImg->width/(float)800;
	float yRatio = pImg->height/(float)600;

	vector<CvRect>::iterator it;

	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		CvRect itRt;
		if (verifySizesRt(*it)==0)
		{
			continue;
		}
		itRt.x = it->x*xRatio;
		itRt.y = it->y*yRatio;
		itRt.width = it->width*xRatio;
		itRt.height = it->height*yRatio;

		vecRt.push_back(itRt);
	}

	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		if (verifySizesRt(*it)==0)
		{
			continue;
		}

		if(it->x*xRatio > (rt.x+rt.width))
		{
			return false;
		}
	}
	
	return true;	
}


bool MvJudgeOnBusVioValid::IsHavePlateInImgRight(const IplImage *pImg, vector<CvRect> &vecRt)
{
	Mat input(pImg);

	Mat SmallImage;
	SmallImage.create(600,800, CV_8UC3);
	resize(input,SmallImage,SmallImage.size());

	Mat img_gray;
	cvtColor(SmallImage, img_gray, CV_BGR2GRAY);

	Mat img_blur;
	//Finde vertical lines. Car plates have high density of vertical lines
	bilateralFilter(img_gray, img_blur,5, 10,5/2);

	IplImage *pImgIn = &IplImage(img_blur);

	vector<CvRect> vecRet = mvDetectPlatePos(pImgIn);

	float xRatio = pImg->width/(float)800;
	float yRatio = pImg->height/(float)600;

	vector<CvRect>::iterator it;

	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		CvRect itRt;
		if (verifySizesRt(*it)==0)
		{
			continue;
		}
		itRt.x = it->x*xRatio;
		itRt.y = it->y*yRatio;
		itRt.width = it->width*xRatio;
		itRt.height = it->height*yRatio;

		vecRt.push_back(itRt);
	}

	for(it=vecRet.begin(); it!=vecRet.end(); it++)
	{
		if (verifySizesRt(*it)==0)
		{
			continue;
		}

		if (it->x < (pImgIn->width>>1))
		{
			continue;
		}

		return true;
	}

	return false;
}

bool MvJudgeOnBusVioValid::mvOnBusVioValidJudge(const IplImage *pImage1,vector<CvRect> &vecRt1,
	                                         	const IplImage *pImage2,vector<CvRect> &vecRt2,
												const IplImage *pImage3,vector<CvRect> &vecRt3,
												CvRect plateRt)
{
	bool bValid1 = IsHaveCarInPlateRight(pImage1, vecRt1, plateRt);
	if (!bValid1)
	{
		return false;
	}

	bool bValid2 = IsHavePlateInImgRight(pImage2, vecRt2);
	if (!bValid2)
	{
		return false;
	}

	bool bValid3 = IsHavePlateInImgRight(pImage3, vecRt3);

	if (bValid1 && bValid2 && bValid3)
	{
		return true;
	}

	return false;
}