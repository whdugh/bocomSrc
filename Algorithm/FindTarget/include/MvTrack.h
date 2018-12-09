#ifndef MV_TRACK_H
#define MV_TRACK_H

#include <list>
#include <map>
#include <fstream>
#include "MvCorner.h"
#include "MvCornerLayer.h"

#include "MyKalman.h"
#include "MvMaxSize.h"



#define TRACK_MAX_CORNERS 200
#define MIN_TRACK_CORNUM 3 //轨迹包含的最少角点数目



enum Drive_Dir
{
	Come =0,
	Leave,
	Horizon,
	UnKnow

};

typedef struct _RelativDis
{
	float fWorDis;
	int   nImgDis;
	_RelativDis()
	{
		fWorDis = 0.0; 
		nImgDis = 0;
	}

}RelativDis;






typedef struct _MvTrack
{

public:

	_MvTrack()
	{

	}
	// 构造函数
	_MvTrack(MvCornerLayer *pCorLayer)
	{
		m_nTakedPosCount   = 0;
		m_nNextStorPos     = 0;
		m_nTrackIndex      = -1;
		m_pCornerLayer     = pCorLayer;
		memset(m_CornerIndex, 0, TRACK_MAX_CORNERS * sizeof(int));

		m_uLastUpdateSeq   = 0;

		m_fTravX           = 0.0;
		m_fTravY           = 0.0;

		m_nGroupId         = 0;

		m_nInformalTimes   = 0;
		m_bFormal          = true;
		m_nFormNum         = 0;


		m_nEstTimes        = 0;


		m_nTotalEstTimes   = 0;


#ifdef TRACK_GROUP_LGW
		 m_fJoinGroupSim =  100.0;
		 m_nGropFolwId  = 0;
		 m_nCarCol = -1;
		 m_uCarId = 0;
		 m_DisPlatPoint = cvPoint(MAX_INT,MAX_INT);
		 m_bBusTop = false;
#endif

	}

#ifdef TRACK_GROUP_LGW
	
	inline void SetBusTop(bool bBusTop) 
	{
		m_bBusTop = bBusTop;
	}


	inline void ClearGroupPropter()
	{
		m_nGropFolwId  = 0;
		m_fJoinGroupSim = 100.0;
		m_nCarCol = -1;
		m_uCarId = 0;
		m_DisPlatPoint = cvPoint(MAX_INT,MAX_INT);
		m_bBusTop = false;

	}

	inline void ClearTrackPropter()
	{
		m_nTakedPosCount   = 0;
		m_nNextStorPos     = 0;
		m_nTrackIndex      = -1;
		memset(m_CornerIndex, 0, TRACK_MAX_CORNERS * sizeof(int));

		m_uLastUpdateSeq   = 0;

		m_fTravX           = 0.0;
		m_fTravY           = 0.0;

		m_nGroupId         = 0;

		m_nInformalTimes   = 0;
		m_bFormal          = true;
		m_nFormNum         = 0;


		m_nEstTimes        = 0;


		m_nTotalEstTimes   = 0;

		m_nGropFolwId  = 0;
		m_fJoinGroupSim = 100.0;
		m_nCarCol = -1;
		m_uCarId = 0;
		m_DisPlatPoint = cvPoint(MAX_INT,MAX_INT);
		m_bBusTop = false;

	}

	inline bool GetBusTop() const
	{
		 return m_bBusTop;
	}

	inline int GetCarCol() const
	{
		return m_nCarCol;
	}
	inline void SetCarCol(int nCol) 
	{
		 m_nCarCol = nCol;
	}

	inline CvPoint GetPlatDis() const
	{
		return m_DisPlatPoint;
	}
	void SetPlatDis( CvPoint PlatPoint);

	inline unsigned int GetTrackCarId() const
	{	 
		return m_uCarId;
	}
	inline void SetTrackCarId(unsigned int uCarId) 
	{
		m_uCarId = uCarId;
	}

	// 获取轨迹所属的目标检测的Group编号。
	inline int GetFlowGroupId() const
	{
		return m_nGropFolwId;
	}
	inline void  SetTrackGrouFlowId(int nID)
	{ 
		m_nGropFolwId = nID;
	}


	inline float GetJoinGroupSim() const
	{
		return m_fJoinGroupSim;
	}

	void  SetJoinGroupSim(float fsim) 
	{
		 m_fJoinGroupSim = fsim;
	}

	void GetMotionDirection(double &fDx, double &fDy, CvRect GroupRec) const;
#endif
	// 预测轨迹在uFrameSeq帧时的位置。
	MvCorner Predict(const unsigned int uFrameSeq,const int64 ts, const unsigned int uTimestamp, const MyCalibration *pCalib, 
		int &nGateSize, const MvMaxSize *p_Maxsize);

	void Speed_Predict(const unsigned int uFrameSeq,const int64 ts, const unsigned int uTimestamp, const MyCalibration *pCalib, 
		int &nGateSize, const MvMaxSize *p_Maxsize ,MvCorner &PreditCor);

	// 用索引为nCornerIndex的角点更新轨迹。
	// 更新时有可能将很老的轨迹点从轨迹记录里删除。如果发生则让角点层删除相应的角点。
	// 估计的角点也通过该函数更新进轨迹。
	void Update(int nCornerIndex);


	// 在角点vecCors里找匹配最好的
	bool FindMatch(const std::vector<MvCorner*> &vecCors, const MvCorner &center, 
		int radius, int &nIndex, int &nDis, MySIFT1 *pSift);

	bool Speed_FindMatch( MvCorner** vCorPointer, MvCorner** tmp_cors, int nCorCount, 
		const MvCorner *pcenter,int radius, int &nIndex, int &nDis );

	bool FindMatch(MvCorner** pCors, MvCorner** tmp_cors, int nCorCount, const MvCorner &center,
		int radius, int &nIndex, int &nDis, MySIFT1 *pSift);

	bool Speed_CheckForMatch( const MvCorner *pCor, MvCorner** pCors, int nCount, int &nMatchedIndex, int &nDis);

	bool CheckForMatch( const MvCorner *pCor, MvCorner** pCors, int nCount, int &nMatchedIndex, int &nDis);

	 int SurfDist(unsigned char *p1, unsigned char *p2);
	// 获取最后一个非估计点的索引。
	bool GetLastNonEstimateCorIndex(int &nIndex);

	// 获取连续两个角点间运动距离大于dis（米）的次数。
	int GetMovedCornerCount(float dis) const;
	


	// 获取轨迹最后一个角点的索引。
	int GetLastCorner() const;
	

	// 获取轨迹第一个角点的索引。
	int GetFirstCorner() const;


	// 获取轨迹的第倒数N个角点。（最早出现的是第一个，最晚出现的是最后一个）。
	// 如果轨迹没有那么多角点，或者已经没有记录了，返回false
	bool GetReverseNthCorner(int n, MvCorner **pCor) const;


	// 获取记录中倒数出现的角点的指针。
	MvCorner* GetRecerseNCornerPointer(int n) const;

	// 获取轨迹总共走了多远，单位米。每次有新角点加入都会累加该值。
	float GetTravelLength() const;


	// 获取轨迹图像距离。
	int GetTravelImgDis() const;


	// 获取轨迹在图像上的运动方向。最后一个角点的ix, iy与第一个角点的ix, iy相比较。
	void GetMotionDirection(double &fDx, double &fDy, int nRecFrames = -1) const;


	// 获取轨迹在世界坐标系中的运动方向。
	void GetMotionDirectionWorld(float &fDx, float &fDy, int nRecFrames = -1) const;

	//MvCorner* GetCornerByFrameSeq()


	// 找出轨迹某一帧时的角点数据。如果找不到返回false
	bool GetCornerByFrameSeq(unsigned int uFrameSeq, MvCorner **pCorner, bool bAllowEstimate = true) const;


	// 获取记录中最早出现的角点的指针。
	MvCorner* GetFirstCornerPointer() const;

	// 获取最后一个角点的指针。
	MvCorner* GetLastCornerPointer() const;


	// 增加轨迹估计次数。
	void IncEstTimes()
	{
		// 总估计次数
		m_nTotalEstTimes++;

		// 连续估计次数
		m_nEstTimes++;
	}

	// 获取连续估计次数
	int GetEstTimes() const
	{
		return m_nEstTimes;
	}

	// 获取总估计次数
	int GetTotalEstTimes() const
	{
		return m_nTotalEstTimes;
	}

	bool GetFormTrack()const
	{
		return m_bFormal;
	}
	// 清除连续估计次数。当有非估计角点加入时，就会清除连续估计次数。
	void ClearEstTimes()
	{
		m_nEstTimes = 0;
	}


	// 计算轨迹当前的瞬时运动方向
	//bool GetTrackInstantMotionDirectionWorld(float &vx, float &vy, int nFrames=4);

	

	// 获取倒数第二个角点的指针。
	MvCorner* GetSecLastCornerPointer() const;


	
	// 获取当前轨迹和轨迹t共帧的角点的起点和终点。
	bool GetCommonFrameStartEndCorners(const _MvTrack &t, MvCorner *vMyCors[2], MvCorner *vTCors[2]);




	// 获取当前轨迹和轨迹t共帧的角点
	//void GetCommonFrameCorners(const _MvTrack &t, MvCorner *vMyCors[TRACK_MAX_CORNERS], 
	//	MvCorner *vTCors[TRACK_MAX_CORNERS], int &nCommonCount);


	// 给定两个轨迹共帧部分的起点和终点。比较两个轨迹。如果两个轨迹都不动，则返回0.
	// 没有用到。
	//static double CompareMotion1(MvCorner *vMyCors[2], MvCorner *vTCors[2]);


	// 看两条轨迹的角点之间距离是否保持恒定。
	//static double CompareMotionLongTerm(MvCorner *vMyCors[TRACK_MAX_CORNERS], MvCorner *vTCors[TRACK_MAX_CORNERS],
	//	int nCommonCount, int nLeastCommonFrames = 2, int nFrom = 0);

	// 
	//static double CompareMotionLength(MvCorner *vMyCors[TRACK_MAX_CORNERS], MvCorner *vTCors[TRACK_MAX_CORNERS],
	//	int nCommonCount, int nLeastCommonFrames = 2, int nFrom = 0);


	// 比较两个轨迹的运动长度，从倒数第nBack个角点到最新的一个角点。
	double CompareMotionLength2(const _MvTrack &t, int nBack);

	int   CalTrackDis(const _MvTrack *t,bool bXDir =false);


	// 比较两个轨迹的运动一致性。各小段分别对应比较。返回各段差异的最大值。
	// nFrom指定从哪个角点开始比较。
	// 返回值在[0-1]，越大表明差异越大。
	// tips: 两个不动的轨迹返回值为0
	/*static double CompareMotionShortTerm(MvCorner *vMyCors[TRACK_MAX_CORNERS], MvCorner *vTCors[TRACK_MAX_CORNERS], 
		int nCommonCount, int nLeastCommonFrames = 2, int nFrom = 0);*/

	
	// 比较两个轨迹的运动方向一致性。从第nFrom角点，到nCommonCount-1的角点的向量相比较。
	//static double ComareMotionByDirection(MvCorner *vMyCors[TRACK_MAX_CORNERS], MvCorner *vTCors[TRACK_MAX_CORNERS],
	//	int nCommonCount, int nLeastCommonFrames = 2, int nFrom = 0);

	
	double ComareMotionByDirection2(const _MvTrack &t, int nBack,bool bNoVehicle = true);


    double ComareMotionByDis( const _MvTrack &t,const MvMaxSize *p_Maxsize);



	// 逐帧比较运动速度方向一致性。[0-1]表示差异大小
	//double CompareMotion2(const _MvTrack &t, int nLeastCommonFrames = 5);



	// 比较两个轨迹最新的nSeg个小段的运动一致性。各小段分别对应比较。返回各段差异的最大值。
	double CompareMotionShortTerm2(const _MvTrack &t, int nSeg = -1);


	// 看两条轨迹的角点之间距离是否保持恒定。
	double CompareMotionLongTerm2(const _MvTrack &t, int nCors = -1);


	//速度关系
	double MotionSpeed(const _MvTrack *t);

	void  GetTrackMotinVec(int &ndx, int &ndy,bool bFul =true);
	void  GetTrackMotinDis(float &fWodis,int n);


	// 按段比较。
	// 首先看最新的角点的帧号是否一致，如果不一致则认为不相似， 返回。
	// 然后，取出共帧的总长度。如果总长度小于nSize，则认为不相似，返回。
	// 再然后，从当前帧开始，以nSize为段大小，取出两个轨迹在这一段时间内的运动。
	// 如果最新的段两个轨迹都没动,则放大nSize到nSize + nStep，再取运动。
	// 如果一个动一个不动则认为不相似，返回。
	// 如果两个都动，则比较这两段的运动相似性，并返回。
	// nSize 段大小。
	double CompareMotion3(const _MvTrack &t, int nSize = 5, int nStep = 2, int nRecursTimes = 0);


	





	// 获取轨迹的角点序列。
	void GetTrackCorners(int corners[TRACK_MAX_CORNERS], int &nCornerCount) const;

	// 
	void GetTrackCorners(MvCorner* vCorners[TRACK_MAX_CORNERS], int &nCount) const;


	// 获取轨迹所包含的角点个数。
	int GetTrackCornerCount() const;
	
	// 
	void Destroy();


	// 获取最后一次更新的帧号。
	unsigned int GetLastUpdateFrameSeq() const;


	// 判断轨迹是否合理。
	bool IsTrackReasonable(int nBack=-1);


	// 画轨迹。
	void DrawTrack(IplImage *img,  int nTrackIndex, CvScalar color) const;

	// 画轨迹。
	void DrawTrack(IplImage *img, CvScalar color ,CvPoint offpoint = cvPoint(0,0),bool bShowIndex=false ) const;


	// 
	void WriteTrackToTxt(std::ofstream &ofs);


	// 获取轨迹连续估计的次数。
	int GetContEstTimes();


	// 获取本track在cvset里的index
	int GetTrackIndex()const;

	int GetIndex();

	// 获取轨迹所属的Group编号。
	int GetGroupId() const;


	// 对估计点重新修正。
	void RefineCorners(const MyCalibration *pCalib);

	// 获取最近n帧内运动量
	bool GetRecentNFrameMovingDis(int n, float &idis, float &wdis) const;

	// 看两条轨迹的角点之间相对运动距离。
	RelativDis RelativeMotionWDis(const _MvTrack *t);




	//
	//bool IsFormalTrack() const;


	//void ResetInformalTimes();


	//void IncInformalTimes();


	//void SetFormal(bool bFormal);


	// MvTrackLayer是track的友元。
	friend class MvTrackLayer;
	friend struct _MvGroup;
	friend class MvGroupLayer;
	
private:
	
	// 设置轨迹所归属的group编号。
	void SetGroupId(int nId);
	
	// 通知角点层删除本轨迹的所有角点。
	void RemoveAllCorners();
	


	


	// 设置是否可以被删除。
	//void SetCanDel(bool bCanDel);
	

private:

#ifdef TRACK_GROUP_LGW

	float m_fJoinGroupSim;
	int m_nGropFolwId;
	int m_nCarCol;
	unsigned int m_uCarId;
	CvPoint m_DisPlatPoint;//距离车牌的的偏移量
	bool  m_bBusTop;

#endif

	MvCornerLayer *m_pCornerLayer;

	// 该track是否可以被删除。
	//bool m_bCanDel;


	// track在CvSet里面的编号。
	int m_nTrackIndex;


	// 记录一条轨迹的角点个数。
	int m_nTakedPosCount;


	// 下一个角点在corners里应该占的位置。
	int m_nNextStorPos;

	
	// 轨迹数据。记录组成轨迹的角点的索引。
	// corners是个循环队列，当队列满时会将
	// 老数据删除。记得在删除时通知角点层删除角点。
	int m_CornerIndex[TRACK_MAX_CORNERS];


	//int m_CornerIndexL[];
	//int m_nTakedPosCountL;
	//int m_nNextStorPosL;


	// 轨迹估计的次数，如果得到非估计角点则清0
	int m_nEstTimes;



	// 轨迹总共估计的次数，不清0.
	int m_nTotalEstTimes;


	unsigned int m_uLastUpdateSeq;

	// 轨迹所属的Group的编号。
	// 初始值为0
	int m_nGroupId;


	// 是否是轨迹的正式轨迹。如果不是正式轨迹，还有一个属性m_nInformalTimes
	bool m_bFormal;
	int  m_nFormNum;

	// 非正式轨迹的次数。
	int m_nInformalTimes;

	bool m_bUpData;

	// 轨迹移动距离。世界坐标。(m)
	float m_fTravX;
	float m_fTravY;

	MvCorner m_PredCorn;


	

	

} MvTrack;


// MvTrack在CvSet里面的存储结构。
typedef struct _MvTrackElem : CvSetElem
{
	_MvTrack track;
	_MvTrackElem(const _MvTrack &t):track(t)
	{
	}

} MvTrackElem;



#endif