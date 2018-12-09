//// 博康智能视频检测识别软件 V2.0
//// Bocom Intelligent Video Detection & Recognition Software V2.0
//// 版权所有 2008-2009 上海博康智能信息技术有限公司
//// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
//// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
////
//#ifndef OPTICAL_FLOW_SIFT_H
//#define OPTICAL_FLOW_SIFT_H
//
//#include <cxcore.h>
//#include "OpticalFlow.h"
//#include <vector>
//#include "sift_descr1.h"
//#include "Calibration.h"
//#include "MvPolygon.h"
////#include "MyCorner.h"
//
//#include "CornerBackgroundModel.h"//角点背景模型
//
//using namespace std;
//
//class OpticalFlowSift
//{
//public:
//	OpticalFlowSift();
//	~OpticalFlowSift();
//	
//
//	
//	//void      ExtractCornersAndCompDescr(IplImage *pCurFrame, MyCorner** corners, int& cornerSize);
//
//
//	// 进行角点匹配，如果匹配上记录在vecFlows里面，并且pCurCorners继承与它匹配的角点的移动性。
//	//void      GenerateOpticalFlow(MyCorner* pLastCorners, int nLastCornerSize, MyCorner* pCurCorners, int nCurCornerSize, vector<Flow> &vecFlows);
//
//
//	// 
//
//	
//
//	void      GetOpticalFlowAndGroupBySift(IplImage *pCurFrame, int64 nCurTimestamp, int64 &dt, 
//									 vector<Flow>& flowFiled, int sizeThresh, float lengthThresh,unsigned int uFrameSeq);
//
//	// 中值滤波计算流经r的流量。
//	float     static CalcFlowInRegion(vector<Flow> flowField, CvRect r);
//
//
//	// 中值滤波计算流经r的流量。
//	float     static CalcFlowInRegion(vector<Flow> flowField, CvRect r, FindTargetElePolice::MyCalibration* pCalib,  CvPoint offset);
//
//
//	//Flow      static CalcFlowInRegion(const vector<Flow> flowField, CvRect r);
//
//	// 长度中值确定一些光流，再按角度中值，选择一个光流作为主要光流。
//	bool      static GetMainFlowInRegion(vector<Flow> flowField, CvRect r, Flow &ret);
//
//
//	bool      IsHaveMoveFeaturesInRegion(CvRect r) const;
//
//
//
//	void      GetFlowGroups(vector<CvRect> &groups) const;
//
//	// 长度中值去除噪声。没有考虑方向。    
//	void      static RemoveNoise(vector<Flow> &flowField);
//
//
//	void      static ClearFlowInRegion(vector<Flow> &flowField, const CvRect &rgn);
//
//
//	//void      static CalcSpeedByFlow
//
//	void      static GroupFlowField(const vector<Flow> &flowField, vector<CvRect> &rect, int sizeThresh, float lengthThresh);
//
//
//	CvPoint2D32f* GetCorners(int &nSize) const;
//	
//	//void      static GetFlowFieldRect(const vector<Flow> &flowField,  vector<CvRect> &rect, int sizeThresh, float lengthThresh);
//private:
//
//	void      GenOpticalFlowGroup(const vector<Flow> &flowField, vector<int> vecEndPointKey, 
//										  vector<CvRect> &rect, int sizeThresh, float lengthThresh);
//
//
//
//	CvPoint2D32f*     m_pLastFrameCorners;
//	int               m_nLastFrameCornerCount;
//	MyDescriptor*     m_pLastFrameDescriptor;
//
//	bool*             m_bCornerHasMoved;
//
//	int64             m_nLastFrameTimestamp;
//
//	vector<Flow>      m_vecFlowFiled;
//
//	vector<CvRect>    m_vecFlowFieldGroup;
//
//
//	CornerBackgroundModel<CvPoint2D32f> *m_pCornerBGM;
//
//
//};
//#endif
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
