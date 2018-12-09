// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   11:56
	filename: 	e:\BocomProjects\find_target_lib\include\MvCornerLayer.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvCornerLayer
	file ext:	h
	author:		Durong
	
	purpose:	角点层
*********************************************************************/


#ifndef MV_CORNER_LAYER_H
#define MV_CORNER_LAYER_H

#include <list>
#include <vector>
#include "cxcore.h"
#include "cv.h"

#include "sift_descr1.h"
#include "MvCorner.h"
#include "MvCornerBGM.h"
#include "Calibration.h"
#include "CornerBackgroundModel.h"

//#include <xmmintrin.h>



//using namespace std;

using namespace PRJ_NAMESPACE_NAME;

// 每次提取角点最多提这么多个
#define FRAME_MAX_CORNER_COUNT 200
#define FRAME_MAX_CORNER_COUNT_FOR_ELE 300

#define SIFT_DESCR_WIDTH 4
#define SIFT_BINS        8 //SIFT_DESCR_WIDTH * SIFT_DESCR_WIDTH * SIFT_BINS = 128
//
//typedef struct _CorIndexMap
//{
//	int nMapWidth;
//	int nMapHeight;
//	int *pIndex;
//
//	_CorIndexMap(int nW, int nH)
//	{
//		pIndex      = new int[nW*nH];
//		if (!pIndex)
//		{
//			printf("");
//			exit(-1);
//		}
//		nMapWidth   = nW;
//		nMapHeight  = nH;
//	}
//
//
//	~_CorIndexMap()
//	{
//		if (pIndex)
//		{
//			delete [] pIndex;
//			pIndex = NULL;
//		}
//	}
//
//
//	void SetMap(int val)
//	{
//		for (int i=0; i<nMapWidth * nMapHeight; i++)
//		{
//			pIndex[i] = val;
//		}
//	}
//
//}CorIndexMap;



typedef struct  _MV_CORNER_HARIS
{
	int x;
	int y;
	int val;
	_MV_CORNER_HARIS()
	{
		x = 0;
		y = 0;
		val = 0;

	}
} MvCornerHaris;

class MvCornerLayer
{
public:

	// 构造函数
	//MvCornerLayer(MvCornerBGM *pCornerBGM/*, MyCalibration *pCalib*/);

	MvCornerLayer(CornerBackgroundModel<CvPoint2D32f> *pCornerBGM);



	// 析构函数
	~MvCornerLayer();

	// 从图像上提取角点形成MvCorner
	void ExtractCorners( IplImage *imgGrayFrm, const unsigned int uFrameSeq,const int64 ts, const unsigned int uTimestamp,bool bDay,
		const  MyCalibration *pCalib, MvCorner corners[FRAME_MAX_CORNER_COUNT_FOR_ELE], int &nPtsCount,const bool bVio_Target,
		const std::vector<CvRect> &CarGroupPark,const IplImage* predimg =NULL);
	
	// 保存角点
	//void SaveCorners(MvCorner corners[FRAME_MAX_CORNER_COUNT], int nCount);

	// 保存角点.并返回被保存图像在set里的index
	//void SaveCorners(MvCorner corners[FRAME_MAX_CORNER_COUNT], int nCount, std::vector<int> &vecIndex);
	void SaveCorners(MvCorner corners[FRAME_MAX_CORNER_COUNT], int nCount, std::vector<MvCorner*> &vecCorners);


	void SaveCorners(MvCorner corners[FRAME_MAX_CORNER_COUNT], int nCount, MvCorner* cor_pointer[FRAME_MAX_CORNER_COUNT]);


	// 保存一个角点，返回其索引。
	int SaveOneCorner(const MvCorner &corner);

	// 按索引号取角点。
	//MvCorner GetCorner(int nIndex);

	// 按索引号取角点的指针。
	MvCorner* GetCornerPointer(int nIndex);

	// 按索引号删除角点。
	void RemoveCorner(int nIndex);

	
	// 按帧号取角点。耗时
	//void GetCorners(unsigned int uFrameSeq, vector<MvCorner> &ret);

	// 按帧号取角点。耗时
	//void GetCorners(unsigned int uFrameSeq, vector<int> &ret);

	// 返回标定对象。
	//MyCalibration* GetCalibrationObj();

	// 获取角点层所保存的角点个数。
	int GetCornerCount() const;


	void DebugCorners(const std::vector<int> &vecIndex);


	static void DrawCorners(const std::vector<int> &vecIndex, IplImage* img, MvCornerLayer *pCorLayer);

	// 测试用
	void MyFastFeaturesToTrack(const IplImage* pSrcImg, 
		CvPoint2D32f* _corners,
		int* _corner_count, double  quality_level,
		double  min_distance,
		int block_size,  //FileBox过滤用的
		const IplImage* pMaskImg CV_DEFAULT(NULL) //感兴趣区域图
		,const IplImage* predimg CV_DEFAULT(NULL));//轨迹预测位置图

	void MycalcMinEigenVal(const CvMat* _cov,CvMat *_dst);

	void MycornerMinEigenVal(const IplImage * src, CvMat* dst, int blockSize);

private:
	
	// memory storage
	CvMemStorage *m_pMem;

	// 存储角点
	CvSet        *m_pCornerSet;
	int        m_nCornerThresh;



private:
	
	// 角点背景模型
	CornerBackgroundModel<CvPoint2D32f> *m_pCornerBGM;
	void CalculateHarrisResponse(const IplImage *pGrayImg,const IplImage *pWeightimg, MvCornerHaris *harris_response,int num);
	void CalculateHarrisResponse(const IplImage *pGrayImg,const IplImage *pWeightimg,const int *tp,MvCornerHaris *harris_response,int num);
	int CornerResponseRestrain(MvCornerHaris * corner_pass,
		MvCornerHaris ** corner_max,int num_pass,int numMax);
	int CornerResponseRestrain2(MvCornerHaris * corner_pass,
		MvCornerHaris ** corner_max,int num_pass,int numMax);
	void compute_descriptors(uchar *descriptor,IplImage* image,CvPoint2D32f *corners,int nPtCnt ); 
	void ComputeSingleSurfDescriptor(IplImage *puImgGray,  CvPoint2D32f *pPoint, unsigned char *descriptor);
	
	 



public:
	bool  m_bEleDec;//针对尾牌做电警检测
	bool  m_bDay;  //白天


	//// sift临时变量
	//float***   histgram;
	//float*     float_descr;
	//int*       gradFlag;
	//float*     gradmag;
	//float*     gradori;

	

	// 标定对象
	//MyCalibration   *m_pCalib;
};


#endif
