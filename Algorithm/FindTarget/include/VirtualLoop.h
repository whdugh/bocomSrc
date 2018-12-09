// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


#ifndef VIRTUALLOOP_H
#define VIRTUALLOOP_H

#include <vector>
//#include <set>
#include <map>
#include <errno.h>

#include "SensedObject.h"
#include "disjoint-set1.h"
#include "Calibration.h"
#include "IBackgroundModel.h"
#include "CarManager.h"
#include "OpticalFlow.h"
#include "OpticalFlowSift.h"
#include "MvLineSegment.h"
#include "SynCharData.h"
#include "MvTrack.h"
#include "OpticalFlowHelper.h"
#include "VehicleFlowHelper.h"
#include "MvLine.h"
#include "MvLineStorage.h"
#include "MvBgLine.h"
#include "functions.h"
#include "MvLineMerge.h"

#define FTG_USE_HOG

#ifdef FTG_USE_HOG
#include "hog.h"
#endif


typedef struct _FloatRect
{
	float x;
	float y;
	float width;
	float height;
} MvFloatRect;




typedef struct _BUSTOPINF
{ 
	char SerSliSum;
	double fBusTopAvg;
	CvRect  BusVlpRect;
	bool   bBusRepted;
	bool   bBusAwayVlp;
	unsigned int  nFramNum;


	_BUSTOPINF()
	{
		BusVlpRect = cvRect(0,0,0,0);
		SerSliSum = 0;
		bBusRepted = false;
		bBusAwayVlp = false;  
	}


}MvBusTopInf;



class MvFindTargetByVirtualLoop;
class CarManager;
class VehicleFlowHelper;

class VirtualLoop  
{
private:
	VehicleFlowHelper *m_pVehicleFlowHelper;
	//MvLineStorage *m_pVirtualLoopLineStorage;

	#ifdef FTG_USE_HOG
		MvHOGDescriptor* m_pHogDet;
	#endif

    

	//NoCardColor *m_pNoCard_Color;

	#ifdef DO_DONGS_CODE
public:
		//区域个数
		int m_N;

		//记录历史的均值与方差
		//前一帧
		double *oldMean;
		double *oldDev;
		//前两帧
		double *oldMean2;
		double *oldDev2;

		
		//记录检测区域
		CvRect *subRoi;		
		
		//记录上次调用的帧号
		int m_oldFrame;

	#endif

public:
	std::vector<SensedObject*> objs;

	std::vector<SensedObject*> lastFrameObjs;

	int m_nCarWidth; 
	

	unsigned int m_nBuSApNum;
	CvRect       m_BuSRec;
	int          m_nVlpCarWidth;
	CvRect       m_AddParRec;
	CvRect       m_vlpRec;
	

	


	// 构造函数
	VirtualLoop(MvFindTargetByVirtualLoop *pFtgVlp, CarManager *pCarManager);
	
	// 析构函数
	~VirtualLoop();

	// 将适合用于电警跟踪的目标找出来
	void GetObjsForEle(unsigned int uFrameSeq, std::vector<SensedObject*> &objs, IplImage *dimg = NULL);


	// 断pObj
	void SealObject(SensedObject *pObj);

	// 将虚拟线圈上的rect转化到世界坐标系。
	// rect未缩小。
	MvFloatRect ConvertVlpRect2World(const CvRect &rect) const;

	// 将rect从图像坐标转到世界坐标。
	// rect相对于原图坐标（输入原图，没缩小。1600*1200）。
	MvFloatRect ConvertRectFromImageToWorld(const CvRect &rect) const;
	

	//将点从checkimg转换到世界坐标系
	CvPoint2D64f CvtPointFromCheckImg2World(CvPoint2D64f checkImgPoint);


	//将点从缩放过的roi转换到checkImg
	CvPoint2D64f CvtPointFromCheckImgRoi2CheckImg(CvPoint2D64f checkImgRoiPoint, CvRect roi);


	// 判断check image（缩小的）上的直线是否压线圈。
	bool IsLineCrossLoop(int x1, int y1, int x2, int y2, float decratio = 0);
	bool IsLineCrossLoop2(int x1, int y1, int x2, int y2, float dec);


	// 将原图上的rect转换到check image上。
	// 比如将车牌号码的rect转到check image上。
	CvRect CvtRectFrOriImgToChkImg(const CvRect &rect) const;


	// 将vlp上的rect转到Check上。tips：vlp、check都相对于原图。
	void ConvertRectFromVlpToCheck(CvRect &rect) const;

	// 将check roi上上的rect转换到原图。没有经过缩小
	void CvtRectFromCheckToOri(CvRect &rect) const;





	//从图像计算出rect
//	void Sense(IplImage *img0, IBackgroundModel *bgm, unsigned int imgId, int64 ts, vector<CvRect> &rects, IplImage **vlpImage);


//	void Update(/*CarManager* pCarManager, const MvFindTargetByVirtualLoop *pFtgVlp, */IplImage* checkOriRgb, 
//		vector<Flow> flowField, int64 nOpticalFlowDeltaTimestamp, vector<CvRect> &newRects,
//		vector<CvRect> &invalidRegion, unsigned int frameSeq, int64 ts, bool bBackgroundSuddenChanged, 
//		MyCalibration *pCali,  int imgIndex, unsigned int uTimestamp,CvRect vlpRect, CvRect checkImageRect,  
//		const OpticalFlowSift *pSift, IplImage* bkImg, const IplImage *imgCheck);

	void Update(unsigned int frameSeq, int64 ts, int imgIndex, unsigned int uTimestamp, MyCalibration *pCali, IplImage* imgCheckRgb, 
				std::vector<CvRect> &bgRects, bool bBackgroundSuddenChanged,IplImage* bkImg,  IplImage *imgCheck, const std::vector<MvTrack*> &tracks, 
				const std::vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSets, const std::vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSetsProx,
				std::vector<CvRect> &invalidRegion, MvCorner*pCorners, int nCount, std::vector<CvRect> vecAbsBg,vector<CvRect>VlpObjBacRec,
				IplImage *ShadowImg =NULL ,IplImage *VlpForEsImg =NULL,IplImage *DarkVlpForImg = NULL);
	void Update(unsigned int frameSeq, int64 ts, unsigned int uTimestamp
		,const std::vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSets,IplImage *imgCheckRgb
		,IplImage *imgCheck);
	void SetFakeObj(int nFlowGropId);

	void SetSenObjForType(int nFlowGropId,int nType);

	void SetObjNearCarState(unsigned int frameSeq);

	bool  JudgeShadow(CvRect ObjPos,IplImage *ShadowImg);

	bool  GroupByRoad(CvRect GroupRec,IplImage *RecImg);

	int  mvGetSensObjChanel(CvRect Rgn);



	// 判断目标是否属于某个大车。
	// 大车出现时有可能粉碎，如果后面几帧合起来了。则可以把以前分碎的合并进来。
	bool IsObjBelongToBigVeh(SensedObject* pObj);

	

	//void UpdateNight(CarManager *pCarManager, const MvFindTargetByVirtualLoop *pFtgVlp, IplImage* checkOriRgb,
	//				vector<Flow> flowFieldOri, int64 nOpticalFlowDeltaTimestamp,  vector<CvRect> &bgRects, 
	//				vector<CvRect> &invalidRegion, unsigned int frameSeq, int64 ts, bool bBackgroundSuddenChanged,
	//				MyCalibration *pCali, int imgIndex, unsigned int uTimestamp, CvRect vlpRect, CvRect checkImageRect,
	//				/*const OpticalFlowSift *pSift,*/ IplImage* bkImg, IplImage *imgCheck, const vector<MvTrack*> &tracks,
	//				const vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSets);
	
  
    void SegObjRect(const vector<OpticalFlowHelper::FlowSetInfo> &VlpFlowSets,const std::vector<CvRect> &newRects, std::vector<CvRect> ForRects, std::vector<CvRect> &DstRects);

	// 找最好的对应。基于Munkres算法
	static std::map<int, int> GetBestAssoc(const std::vector<CvRect> &newRects, const std::vector<SensedObject*> &objs);

	
	// 每帧调用，将SensedObject和车牌关联
	void Assoc(const vector<CvRect> &SrcUinVlpRec,unsigned int ts, CarManager* pCarManager, IplImage *Test =NULL);

	Car* Assoc(unsigned int uCarId,SensedObject *pSensObj);
	void mvSeTracksCarPlat(SensedObject *pSensObj,vector<MvTrack*>tracks,CvPoint PlateCen);
	
	//获得较近的Vlp目标！
	std::vector<SensedObject *> GetNearSensedObjs();

	// 每帧调用一次，删除一些不需要的信息
	void Delete(int64 ts, unsigned int uFrameSeq, VehicleFlowHelper *pFlowHelper);
	
	// 输出。每帧调用。如果有需要输出的东西通过ret输出。
	//void Output(int64 ts, unsigned int frameSeq, std::vector<CarInfo> &otput, std::vector<ObjSyncData> &vecSyncData );

	void Output(int64 ts, unsigned int uFrameSeq,  VehicleFlowHelper *pFlowHelper, std::vector<SensedObject*> &vecOutput, std::map<int, _CarInfo> m_mapOutputedVehicle, Time_Mode mode);

	//判断*****大车目标是否属于同车道前后大车！
	bool CheckObjBelongToBigVeh(SensedObject *pObj, ObjType vehType, std::map<int, _CarInfo> m_mapOutputedVehicle, unsigned int frameSeq, int64 ts);

	//void TestSobel(IplImage *img0);
	
	// 找vlp区域与光流场区域的最好对应。用于传递给颜色。
	static std::map<int, int> GetBestVlpRectToFlowFieldRectAssign(const std::vector<CvRect> &vlpRects, const std::vector<CvRect> &flowRects);
	
	// 返回最近压线（并且最后更新不早于uSeq）的车是不是占满整个虚拟线圈的大车。
	bool IsLastBigVehAndTakeFullVlp(unsigned int uSeq);

	MvFindTargetByVirtualLoop *m_pFtgVlp;
private:

	//MvLineMerge *m_pLineMerge;
	unsigned int m_nBusNigfram;
	unsigned int m_BusNowAway;
	map<int,MvBusTopInf>  m_MapBusInf;
	map<int,vector< unsigned int > > m_ChalCarFram;
	void RecodNearVehicFram(int nChanl,unsigned int frameSeq);
	bool GetNearVehicl(int nChanel, unsigned int frameSeq,bool bCom =true );
	void mvCorrecHisTrackRec(SensedObject*pSenObj,const vector<MvTrack*> &pTrack,CvRect &SensObjRec);
	void StorelasTrackIndex(SensedObject*pSenObj,const vector<MvTrack*> &pTrack);
	

	

	CarManager *m_pCarManager;
	
	// 获取object。flag=-1所有目标。flag=0没有断的目标。flag=1断了的目标。
	void GetObjects(std::vector<SensedObject*> &objs, int flag);

	// 更新所有目标的目标头位置。
	void UpdateObjectsHeadPos(const std::vector<MvTrack*> &tracks/*, CvPoint offset*/);

	
	// 车牌强断，自然断，流量断
	void SealObjectsByFlowCarNumNat(CarManager *pCarManager, unsigned int frameSeq,
		const MvFindTargetByVirtualLoop *pFtgVlp, const IplImage *imgCheck, const IplImage *imgCheckRgb);
	
	// 把有许多0流量轨迹的目标断掉。
	void SealObjectWithManyZeroFlowTrack(unsigned int uFrameSeq, const IplImage *imgCheck, 
		const std::vector<MvTrack*> &vecTracks, bool bIsDay, std::vector<CvRect> &sealedRgn);

	//void SealByCarNumAndFLow(CarManager *pCarManager);

	
	// 统计图像img在roi区域内RGB三个通道的均值与方差。
	//static void StColor(IplImage *img, CvRect roi, float mean[3], float var[3]);

	
	void SealObjectByVlpColor();



	void CloseToMergeObjs();


	// 如果车牌在本帧有检出，并且他关联了一个sensed obj。
	// 如果这个obj左右边知道（车型部分得到），判断左右边
	// 和车牌是否对称，如果对称，则尝试用左右边位置切当前感知到的区域。
	// 如果切出来的区域比较大则真切，如果切出来的比较小，则放弃。
	void SplitUsingPlateAndVehicleRgn(std::vector<CvRect> &vlpRects, unsigned int uFrameSeq);


	//晚间判断车灯的个数
    int DetectLigSum(IplImage *imgN,int threshold);

	// 晚间，车牌出现时用车灯判断车型。-2判不出、1大车、2小车。
	int DetectVeichleTypeByHeadLight(IplImage *imgN, CvRect rectCarNum, unsigned int uFrameSeq, int threshold = 200);


	// 用已经很确定的目标去分裂前景。
	void SplitForeRectsByObject(std::vector<CvRect> &rects, std::vector<CvRect> rectsByCarNum, const std::vector<MvTrack*> &tracks) const;



	void SplitForeAccordingMotionDiff(const std::vector<MvTrack*> &tracks, 
		const std::vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSets, 
		const std::vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSetsProx, 
		std::vector<CvRect> &rects, MyCalibration *pCalibCheckImg, 
		unsigned int uFrameSeq, IplImage *imgCheck);


	// 当新车牌出现时，整理一些objs，进行一些分裂与合并。
	void ReviseByCarNum(unsigned int uFrameSeq);


	// 上一次调用Update的seq
	unsigned int m_uLastFrameSeq;


	// 车牌修补。如果检出车牌，则在vlp上必有一块压线
	void FixByCarNum(const CarManager *pCarManager, std::vector<CvRect> &rects) ;


	void FixByHistory(IplImage* imgCheckRgb, IplImage* imgCheck, const std::vector<CvRect> &vlpRects, std::vector<CvRect> &rects, const std::vector<MvTrack*> &tracks,
		const std::vector<OpticalFlowHelper::FlowSetInfo> &vecFlowSets,const vector<CvRect> &foreRgnByFlow,IplImage *ForVlpImg
		,const std::vector<CvRect> &VlpObjBacRec) const;
	

	void GroupContourRects(CvSeq *contour, std::vector<CvRect> &bigRectContainer);


	void GroupRectsInRects(std::vector<CvRect> &rects, std::vector<CvRect> bigRects);

	
	SensedObject* GetLastFrameObject(CvRect rect, int64 ts);

	// 根据车牌位置，车宽计算其压线位置
	CvRect CalcVeichlePosInVlpByCarNum(CvRect rectCarNum, CvRect vlpRoi, FindTargetElePolice::MyCalibration *pCalib, float veichleWidth) const;

	// 根据车牌位置、车在vlp上的压线位置计算整个车身在图像中的位置
	// TODO: 车高度和宽的的关系
	CvRect CalcVeichlePosInImgByCarNum(CvRect rectCarNum, CvRect vlpRoi, FindTargetElePolice::MyCalibration *pCalib, 
							CvRect rectCarOnVlp, float fVeichleLengthWidthRatio = 3.0f) const;


	// 根据车流量、车在vlp上的压线位置计算整个车身在图像中的位置
	// TODO: 车高度和宽的的关系
	CvRect CalcVeichlePosInImgByFlow(CvRect vlpRoi, CvRect rectCarOnVlp, FindTargetElePolice::MyCalibration* pCalib, float flow, float fVeiLenWidRat = 3.0f);


	

	

	//IplImage *GetVlpImage(IplImage *frame);
	void GetSobelEdgeGroups(IplImage *frame, std::vector<CvRect> &sobelRects);

	// 已知上一帧某个物体的压线位置。根据本帧流量。预测其本帧压线位置。
	// 对于直行的目标。前后两帧位置应该差不多。所以这个功能是为了解决非
	// 直行目标。
	void PredictVlpRectByFlow(CvRect &rect, const Flow &flow) const;


	// 看psobj在不在lastFrameObj向量中
	bool IsLastFrameObj(SensedObject *psobj);

	// 判断虚拟线圈上两个rect是否靠近
	bool IsTwoVlpRectsClose(const CvRect &r1, const CvRect &r2);


	void  GroupRectsOnVlp(std::vector<CvRect> src, std::vector<CvRect> &dest);



	


	void  GroupFlowField(const std::vector<Flow> &flowFieldWorld, std::vector<CvRect> &rects);

#ifdef TRACK_GROUP_LGW
		void DetObjCol(SensedObjectTrackData * psotd,IplImage *pChecRgbImg);
#endif

    void DetectVehicType(SensedObject *pSensObj,IplImage *imgCheckRgb,IplImage *imgCheck,unsigned int frameSeq,
		const std::vector<MvTrack*> &track);


	// 用两垂直线一水平线判断车辆类型
	static int DetectVeichleTypeByLines( IplImage* img, CvRect roi, MvLine *pLines, int nLineCount, MvFindTargetByVirtualLoop *pFtg, int direction, bool bRealCar, CvRect &rectVehicleRgn, unsigned int uFrameSeq) ;
	

	//
	static int DetectVehicleTypeByLinesWeak(IplImage *img, MvLine *lines, int nLineCount, MvFindTargetByVirtualLoop *pFtg);
	
	
	static bool   IsBigVeichleByLines(MyCalibration *pCalib, int nVlpY, MvLine *pLines, int nLineCount, MvLine **pRetHorLine, MvLine **pRetLeftVerLine, MvLine **pRetRightVerLine);

	
	
	static bool   IsSmallVeichleByLines(MvLine *pLines, int nLineCount, MvLine **pRetHorLine, MvLine **pRetLeftVerLine, MvLine **pRetRightVerLine);

	
	
	static bool   IsBigVehicleByBackLines(MvLine *pLines, int nLineCount, MvLine **pRetUpperHorLine, \
		                                    MvLine **pRetLowerHorLine, MvLine **pRetLeftSkewLine, MvLine **pRetRightSkewLine, MvFindTargetByVirtualLoop *pFtg, bool bRealCar);
	
	static bool   IsSmallVehicleByBackLines(MvLine *pLines, int nLineCount, MvLine **pRetUpperHorLine, \
											MvLine **pRetLowerHorLine, MvLine **pRetLeftSkewLine, MvLine **pRetRightSkewLine, MvFindTargetByVirtualLoop *pFtg, bool bRealCar);
	static bool   IsVehicleByBackLines(MvLine *pLines, int nLineCount, bool bIsCar);

	static bool   IsGotBackWindow(MvLine *pLines, int nLineCount, MvLine **pRetUpperHorLine, \
								  MvLine **pRetLowerHorLine, MvLine **pRetLeftSkewLine, MvLine **pRetRightSkewLine, float fHorLenThr);
	static bool   IsGotTwoSidelinesAndOneHorline(MvLine *pLines, int nLineCount, MvLine **pRetUpperHorLine, \
												 MvLine **pRetLowerHorLine, MvLine **pRetLeftSkewLine, MvLine **pRetRightSkewLine,  float fHorLineLenThr, float fSideLineLenThr, MvFindTargetByVirtualLoop *pFtg, bool bRealCar);
	static bool   IsHaveManySideLines(MvLine *pLines, int nLineCount, MvLine **pRetUpperHorLine, \
												 MvLine **pRetLowerHorLine, MvLine **pRetLeftSkewLine, MvLine **pRetRightSkewLine,float fSideLineLenThr, MvFindTargetByVirtualLoop *pFtg);
	
	static float  CalcObjInstantMotionDirection(const std::vector<MvTrack*> &track, CvRect trackRegion, IplImage *img, unsigned int uFrameSeq);  //计算目标运动的主方向！
	
     static  float CalcObjInstantMotionDirection(const std::vector<MvTrack*> &track, IplImage *img, unsigned int uFrameSeq);
	#ifdef FTG_USE_HOG
	static int DetectVehicleTypeByHog(IplImage* detectImg, MvHOGDescriptor* hog, std::vector<CvRect> &found);
	#endif


	#ifdef DO_DONGS_CODE
		
		// 董恒志
		//用直线判断边缘大车
		//static bool   IsBigVeichleByLinesSide(unsigned int uFrameSeq, bool bIsDay, const IplImage* img, CvRect roi);
		bool   IsBigVeichleByLinesSide(unsigned int uFrameSeq,
											  bool bIsDay, const IplImage* img, 
											  CvRect roi, const MvFindTargetByVirtualLoop *pFtgVlp,
											  CvRect MyVirtualLoopRect,
  											  float &fRemainFlow);



		// 董恒志
		//白天情况下检查大车遮挡小车的情况
		//如果返回值为真，则断开这个大车
		//如果返回值为假，则根据返回的流量值来判断该不该断，何时候断
		bool IsCutBigCarTail(unsigned int uFrameSeq, 
								bool bIsDay, const IplImage* img,
								CvRect roi, const MvFindTargetByVirtualLoop *pFtgVlp,
								CvRect MyVirtualLoopRect,
								float &fRemainFlow);


		// 董恒志
		//晚上检查大车遮挡小车的情况
		//如果返回值为真，则断开这个大车   
		//如果返回值为假，则根据返回的流量值来判断该不该断，何时候断.

		bool IsCutBigCarTailNight(unsigned int uFrameSeq,
												  bool bIsDay, const IplImage* img, 
												  CvRect roi,const MvFindTargetByVirtualLoop *pFtgVlp,
												  CvRect MyVirtualLoopRect,
												  float &fRemainFlow);

		// 董恒志
		// 晚上检查大车遮挡小车的情况
		// 如果返回值为真，则断开这个大车   
		// 注：此函数根据灰度的均值和方差进行计算
		bool IsCutBigCarTailNight2(unsigned int uFrameSeq,
												  bool bIsDay, const IplImage* img, 
												  CvRect roi,const MvFindTargetByVirtualLoop *pFtgVlp,
												  CvRect MyVirtualLoopRect
												  );


	#endif

public:
	// 结果在外部释放。
	static void GetLines( MyCalibration *pCalib,MvBgLine *pLineBGM, IplImage* img, CvRect roi, MvLine **pLines, int &nCount, \
							MvLine **pMvBgLine, int &nBgLineNum, unsigned int uFrameSeq, MvLineStorage *pLineStorage, MvLineMerge *pLineMerge);
};

#endif
