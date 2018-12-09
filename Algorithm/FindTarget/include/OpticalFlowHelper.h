#ifndef OPTICAL_FLOW_HELPER_H
#define OPTICAL_FLOW_HELPER_H


#include <vector>
#include "MvTrack.h"
#include "disjoint-set1.h"

#include "OpticalFlow.h"
#include "Mode.h"
#include "Car.h"
class MvTrackLayer;
class VirtualLoop;

using namespace std;

#ifdef TRACK_GROUP_LGW

#ifndef  MAXINT
#define MAXINT 65536
#endif

#define MIN_GROUP_TRACKNUM 3 //Group包含最小的轨迹个数
#define PERMIT_DISAPE_MAXFRAMNUM  12 //允许消失的帧数

enum GroupType
{
    Motocycl_Group=0,
    CAR_Group,
	MIDBUS_Group,
	BUS_Group
};

typedef struct _ForObj
{
	CvRect ForRec; //前景目标
	unsigned int upDatDetFramSeq;
	unsigned int uFristFramSeq;
	unsigned int uId;
	unsigned int type;
	
	_ForObj()
	{
		ForRec = cvRect(0,0,0,0);
		upDatDetFramSeq = 0;
		uFristFramSeq = 0;
		uId = 0;
		type = 0;
	}

}ForObj;
typedef struct _GroupFlow
{

	unsigned int nGropFolwId;//Group编号
	CvRect   GropRec;//Group外界框
	bool bTrackUpdat;//是否得到轨迹更新
	int  nPlatcolor;//车牌颜色
	unsigned int  uCarId;//当前车牌编号
	unsigned int  uHistoyCarId;//历史车牌编号
	CvRect CarRec;//车辆整体位置
	CvRect CarPlateRec;//车牌的位置
	unsigned int ulstCarFremseq;//车牌最后更新的帧号
	Drive_Dir Dir;//0为由远即近（卡口前牌）,1为由近即远（电警尾牌）,2水平方向
	CvPoint  PreditPlatePoi;//车牌位置
	unsigned int uPrediPlateFram;
	bool     bPrediState;//车辆是否得到预测车牌
	float    fPlateSlope; //车牌的偏移斜率
	int      nHitMidBus;//中型车
	int      nSegY;//聚类分割线
	int      nDisAppearNum;//卡口目标车头驶离图像区域的次数
	int      nHitVehicl;//认为是机动车的次数
	int     nEnterBusTop;//大车进入车辆顶部
	bool    bPark;//车是否存在停靠状态
	bool    bMov;//车是否存在运动状态
	//std::vector<CvPoint> GroupTrack; //目标稳定后的Track。一旦突变就进行清除
	CvRect HistRec;
	std::vector<MvTrack*> *pvecTracks;
	bool  bFake;//是否为假目标
	unsigned int uLastupFram;//更新的最后一次帧号
	int       nLigNum;//夜间灯的个数
    unsigned int nAppearNum;
	bool     bReported; //只针对尾牌情况，因为其关联车牌


	_GroupFlow()
	{

		nGropFolwId = 0;
		nPlatcolor = -1;
		bTrackUpdat = false;
		Dir = UnKnow;
		uCarId = 0;
		uHistoyCarId = 0;
		CarRec = cvRect(0,0,1,1);
		CarPlateRec = cvRect(0,0,1,1);
		PreditPlatePoi = cvPoint(MAX_INT,MAX_INT);
		ulstCarFremseq = 0;
		fPlateSlope = 1;
		nHitMidBus = 0;
		nSegY = 0;
		bPrediState = false;
		nDisAppearNum = 0;
		nHitVehicl = 0;
		nEnterBusTop = 0;
		bPark = false;
		bMov  = false;
		pvecTracks = NULL;
		bFake = false;
		uLastupFram = 0;
		nLigNum = 0;
		nAppearNum = 0;
		uPrediPlateFram = 0;
		bReported = false;

	}

}GroupFlow;

typedef struct _MotionVect
{
   int dx;
   int dy;
   _MotionVect()
   {
	   dx = 0 ;
	   dy = 0 ;
   }

}MotionVect; 

typedef struct _Group //用来记录历史目标信息
{
	GroupType type; //车的类型
	MotionVect vec;  //车的运动矢量
	float   fWDis;  //邻近MIN_TRACK_NUM下车的运动世界距离-
	CvRect  Rgn;   //区域
	unsigned int uLastUpFram;//最后一次更新的时间
	unsigned int uFrisFram;//第一次一次更新的时间
	unsigned int  uCarId;//历史车牌编号
	unsigned int  nGropFolwId;//历史车牌编号
	_Group()
	{
		type = CAR_Group;
		fWDis = 0.0;
		Rgn = cvRect(0,0,1,1);
		uLastUpFram = 0;
		uFrisFram = 0;
		uCarId = 0;
		nGropFolwId = 0;

	}

}Group;

typedef struct _TrackCluster
{
	vector<MvTrack*> pVecTrack;
	int nGropFolwId; 
	_TrackCluster()
	{
		nGropFolwId = 0;
	}

}TrackCluster;

#endif

// 一些轨迹的（光流的）组成的集合。


class OpticalFlowHelper
{
public:
	typedef struct _FlowSetInfo
	{

		std::vector<MvTrack*> vecTracks;
		std::vector<Flow>     vecFlows;
		CvRect           rgn;
		double           dMainFlowX;
		double           dMainFlowY;
		int              nGropFolwId; 
		unsigned int    uCarId;
		CvRect          CarRec;
		Drive_Dir       Dir;
		float    fPlateSlope; //
		int       nPaletCol;
		bool      bFake;
		int       nAppearNum;
		bool      bTouchBom;
		int       nType;//-1未知，0非机动车，1机动车，2是中型车，3大车
		int      nLigNum;
		bool     bPark;
		CvPoint  PlatePoi;//车牌位置

		_FlowSetInfo()
		{
			dMainFlowX = 0.0;
			dMainFlowY = 0.0;
			nGropFolwId = 0;
			uCarId = 0;
			rgn = cvRect(0,0,0,0);
			CarRec = cvRect(0,0,1,1);
			Dir = Come;
			fPlateSlope = 1;
			nPaletCol = 0;
			bFake = false;
			nAppearNum = 0;
			bTouchBom = false;
			nType = -1;
			nLigNum = 0;
			bPark =false;
			PlatePoi = cvPoint(0,0);

		}

		void GetMainDirection(float &dDirX, float &dDirY);
	} FlowSetInfo;

#ifdef  TRACK_GROUP_LGW
	 static unsigned int m_GenId;
	 vector<GroupFlow> m_GroupFlow;
	 map<unsigned int,Group> m_HistGroup;
	 vector<Group> m_HistBusGroup;
	 MvMaxSize* m_pMaxSize;
	 MvTrackLayer* m_pTrackLayer;
	 unsigned int m_uFramSeq;
	 int m_nCarWidth;
#endif
	 VirtualLoop *m_pVirLoop;
	//static void GetMotionRegion( const std::vector<MvTrack*> &tracks, std::vector<FlowSetInfo> &vecFlowSetsInfo,
	//							float fDisXThresh = 0.6, float fDisYThresh = 1.5, Time_Mode mode = DAY);

	 OpticalFlowHelper();
	 ~ OpticalFlowHelper();
	 void GetMotionRegion(MvMaxSize* pMaxSize,const std::vector<MvTrack*> &tracks, IplImage* img, 
		std::vector<FlowSetInfo> &vecFlowSetsInfo1, std::vector<FlowSetInfo> &vecFlowSetsInfo2,
		float fDisXThresh1 = 0.6, float fDisYThresh1 = 1.5, float fSimility1 = 0.3, 
		float fDisXThresh2 = 0.8, float fDisYThresh2 = 1.8, float fSimility2 = 0.4, 
		Time_Mode mode = DAY, bool bDoTwice = false,bool bBuSNear = false);


	 void GetCompensRegion(const std::vector<FlowSetInfo> &vecFlowSrcInfo1, 
		const std::vector<FlowSetInfo> &vecFlowSrcInfo2, std::vector<FlowSetInfo> &vecFlowDstInfo2);


	 void GetMainFlow(const std::vector<Flow> &vecFlows, double &dx, double &dy);

	// 长度中值去除噪声。没有考虑方向。    
	 void RemoveNoise(std::vector<Flow> &flowField);


	 bool HaveMovedFeaturesInRegion(const std::vector<FlowSetInfo> &vecFlowSetsInfo, CvRect rgn, int nSizeThresh);

	//static bool HaveMovedFeaturesInRegion(const vector<MvTrack*> &tracks, CvRect rgn, int nSizeThresh);

	 bool GetMainFlowInRegion(const std::vector<MvTrack*> &tracks, CvRect rgn, Flow &ret,Time_Mode mode = DAY);


	 void GetMaxLengFlowInRegion(const std::vector<MvTrack*> &tracks,Flow &ret);


	// 判断区域是不是车灯打在地上形成的。
	 bool IsRgnLight(IplImage *img, MyCalibration*pCalib, CvRect rgn);

#ifdef TRACK_GROUP_LGW

	 void  mvDiffImg();
	 bool mvNearBus(CvRect rec,unsigned int nRecAppearFramSeq);
	 //由Track到Group生产的主函数
	 void mvProcessFlowGroup(VirtualLoop *pVriLoop,MvTrackLayer* pTrackLayer,MvMaxSize* pMaxSize,vector<MvTrack*> vecTracks,
		                     unsigned int uFrameSeq,int64 ts,vector<FlowSetInfo> &vecFlowSets,Time_Mode mode,
							 bool bTailMove = false); 

	 RelativDis mvCalGroupStaEndVarWDis(const MvTrack* pSrcTrack,const MvTrack *pDstTrack);

	 //生产新的Group
	 void mvGenerateGroup(vector<MvTrack*> &vecNoGroupTracks,vector<TrackCluster> &ObjCluterTrack);

	 void mvSetFlowSetInfo(FlowSetInfo &vecFlowSets,GroupFlow *pGrowFlow);

	 bool mvCombineOterhGroup(FlowSetInfo SrcFlowSets);
	 void mvOterhCarGroup(GroupFlow *pCarGroup);

	 CvPoint mvGetCenPoin(const vector<MvTrack*> &ObjTrack);
	 
	 void mvDelSmallDisTrack(vector<MvTrack*> &Tracks);

	 Drive_Dir mvGetDirByTrack(const vector<MvTrack*> &Track);

	 void     mvCountGroupDisApearNum(GroupFlow *pGrowFlow);//记录目标消失的次数；

	 //将JoinGroupTrack中编号与加入到与pDst聚类中轨迹编号一致的
	 void mvUpdatGroupByTrack(TrackCluster *pDst,GroupFlow *pGrowFlow,vector<MvTrack*> &JoinGroupTrack);

	 //统计新加入到Group的轨迹，同时更新当前Group是否有新轨迹加入的状态
	 void mvGetJoinGroupTrack(TrackCluster *pSrc, const map<int ,int> &MapGroupToPos,vector<MvTrack*> &GroupTrack,unsigned int uFramSeq);

	 CvRect mvGetGroupSearcRec(GroupFlow *pGrpFlow,unsigned int uFramSeq,TrackCluster *pGroupTrack,CvRect &EnterBusopRec
		 ,int &nDx);

	 bool  mvGetBusToptrackCen(const vector<MvTrack*> &Track,CvPoint &point);

	 //删除Group下不合理的轨迹
	 void mvDelUnResonGrpTrck(TrackCluster *pSrc,GroupFlow *pGroFlow);

	 //设置轨迹的车辆属性
	 void  mvSetTrackCarInf(vector<MvTrack*> &Track,GroupFlow *pGroFlow,bool bCurrectCar =false
		 ,CvPoint Plat =cvPoint(MAX_INT,MAX_INT));

	 void mvSetTrackVirtualCarInf(vector<MvTrack*> &Track,GroupFlow *pGroFlow,bool bHeadToBom = false);

	 void mvKmeanHalfTrack(TrackCluster *pSrc,bool bXSeg ,GroupFlow *pGroFlow,int nMaxSize,bool Bicycel = false);

	 bool mvTrackSegDis(const vector<MvTrack*> pSrcTrack);

	 bool mvSplitGroup(TrackCluster *pSrc,Drive_Dir Dir,int nMaxSize,TrackCluster *pDst);

	 void mvYDirCombinGroup(TrackCluster *pSrc,GroupFlow*pSrcGrFlow,TrackCluster *pDst,GroupFlow*pDstGrFlow,CvRect SrcMaxSize);

	 //校正Group
	 void mvCorrectGroupTrack(TrackCluster *pSrc, const map<int ,int> &MapGroupToPos);

	 //Group之间的轨迹合并
	 void mvCombineGroupTrack(TrackCluster *pSrc,TrackCluster *pDst);

	 //相似的轨迹合并
	 void mvCombineSimTrack(TrackCluster *pSrc,TrackCluster *pDst,bool bYelPal =false);

	 int  mvGetMapGrouPos(const map<int ,int> &MapGroupToPos,int nGropFolwId);

	 void mvCorrectBigSizeGroup(TrackCluster *pSrc, const map<int ,int> &MapGroupToPos);

	 float mvCalSimTrack( MvTrack* pSrcTrack, const vector<MvTrack*> &vecCompaTracks,CvRect GropRec, CvRect MaxSizeRec);

	 void  mvTrackSetFlowGroupId(vector<MvTrack*> &Track,int nId);

	 bool  mvGroupTrackParkState( TrackCluster *pSrcTrack,bool &bMov);

	 bool mvAddToHistoryGroup(vector<MvTrack*> &ObjTrack,const GroupFlow &ObjGroup);

	 //进行图论分割
	 void mvSegmentGraph(Universe *pUniver, int num_vertices, int num_edges, Edge *edges);

	 //利用二分法按w升序
	 void mvBinSort(Edge *data, int n);

	 void mvSetClusterTrackGroupId( Universe *pUniver,vector<TrackCluster> &ObjCluterTrack);

	 void mvSetGroupFlowByCar( GroupFlow *pGroFlow, vector<MvTrack*> pTrack,
		 int GropFolwId,vector<OpticalFlowHelper::FlowSetInfo>&vecFlowSets, Car *pCar=NULL ,CvRect CarRec =cvRect(0,0,1,1),CvRect paltRec= cvRect(0,0,1,1) );

	 void  mvClearTrackLoacInCar(vector<MvTrack*> &Track,CvRect CarRec);
	 
	 void  mvFindFakeGroup(vector<FlowSetInfo> &vecFlowSets);//寻找伪目标；主要是晚上车灯前引起路面的假目标

	  float  mvCalBrightRate(CvRect Rec,int nBrightness);

	 void  mvUpadtHistGroup();//更新历史目标

	  void  mvDelHistGroup();//删除历史目标--针对很近没更新的

	  bool mvDetLig(const CvRect &Rec, vector<CvRect> &LigRec);
	  
	  void upDateDiffObj(const CvRect &Rec,bool bMayAdd = true);

	  void DelDiffObjByFram();
	  
	  bool bOutDiffArea(CvRect Rec);

	  void DelDiffObjByGrowId(unsigned int uId);

	  void SetDiffObjId(FlowSetInfo &vecFlowSets);
	  void SetDiffType();
	  ForObj *DiffObExis(unsigned int uId);
	  void  DelDiffObExis(unsigned int uId);

	  
	  void DrawDiffObj();

	  void SetObjFakeByCar(CvRect CarRec);
	  unsigned int mvSameCarWithDiffGrop(unsigned int uCarId);
	  void AssoCarByCarCoren(FlowSetInfo &vecFlowSets,GroupFlow &ObjNewGroup);
	  void SetGrowFlowReported(unsigned int uGrouFlowId);


	 IplImage *m_pRgbImg;
	 IplImage *m_pGrayImg;
	 CvRect  m_DiffRec;
	 IplImage *m_pLastGrayImg;
	 int   m_StartTrackYpos;
	 Time_Mode m_Mode;
	 vector<int> m_relivGroup;
	 int m_CrossFrisY;
	 int m_EleCrossSedY;
	 CvRect m_CarNumDetRgn;
	 vector<ForObj> m_ForObj;//尾牌记录的帧差目标；
#endif


};



#endif