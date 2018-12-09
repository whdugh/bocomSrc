#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "ConvNet.h"

#ifdef _WINDLL
#define LPLOC_API __declspec(dllexport)
#else
#define LPLOC_API
#endif

const int g_nLPNetNum = 6;  //用于车牌点定位的网络个数！
const int g_nLayerNum = 3;  //单双层标志输出类别数！
class licensePlateLoc
{
public:
	LPLOC_API licensePlateLoc();
	LPLOC_API ~licensePlateLoc();

	LPLOC_API int dlInit(char* filename_loc[g_nLPNetNum], char* filename_class, bool bUseFineTuning = true, int nIterNum = 3);
	LPLOC_API float dlForward( IplImage *pImgSrc, CvRect rectRoi, vector<CvPoint> &vecPts, int &nLayer);
	LPLOC_API CvMat* GetShape();

private:
	bool dlImgPreprocess(IplImage *pImgSrc, CvRect &rectRoi);
	bool dlSubImgPreprocess(IplImage *pImgSrc, CvPoint ptPredict, float fLPWidth, CvRect &rectRoi);
	bool dlGetRslt(float *pLocNet, CvRect rectRoi, int nPtNum, vector<CvPoint> &vecPts);
	//精调网络！
	bool dlFineTuning(IplImage *pImgSrc, const vector<CvPoint> &vecPts, vector<CvPoint> &vecRectfPts);
	//判断单层双层牌！0，单层，1，双层，-1，错误！
	int JudgeLayer(IplImage *pImgSrc, const vector<CvPoint> &vecPts, float *fSingleProb);
	//图像预处理，边界判断，resize，减均值！
	bool dlImgPreprocess4LayerNet(IplImage *pImgSrc, const vector<CvPoint> &vecPts, CvRect &rectRoi);
	//返回：0，单层，1，双层，2，不确定，-1，错误！
	int GetLayerLabel(float *fProb, string strLayer2Extr, float *fSingleProb);
	bool GetUnionRect1(CvRect r1, CvRect r2, CvRect &ret);
	bool expandRegion(CvRect region1, CvRect &region2, float fXScale, float fYScale);
private:
	vector<vector<ConvNet*> > m_convnet;
	ConvNet *m_convnet4LayerClass;//判别单双层；
	vector<CvPoint2D32f> m_point;
	CvMat* m_matFaceShape;
	vector<CvRect> m_rect;

	IplImage* m_imgShowResult;

	//wyj
	int m_nIterNum;
	bool m_bUseFineTuning;
};

#endif/*__DEEP_FACIAL_POINT_H__*/
