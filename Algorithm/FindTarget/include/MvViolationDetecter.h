#ifndef MV_VIOLATION_DETECTER_H
#define MV_VIOLATION_DETECTER_H

#include <vector>
#include "MvGroup.h"
#include "MvLine.h"
#include "MvChannelRegion.h"
#include "MvLightStatusMem.h"
#include "ViolationInfo.h"
#include "CarInfo.h"
#include "MvRelabelEventHandler.h"
#include "StdAfx.h"



//using namespace std;

class MvGroupLayer;
 typedef struct VIO_GROUP
 {
	 unsigned int GroupId;
	 bool bTurn; 
	 bool bTouchOtherLine;
	 MvLine *passLine;
	 bool bDir;
	 Car *pCar;
	 vector<MvCorner> Corner;
	 CvRect ParkRec;
	 bool   bMetPreLinout;
	 bool   bMetReverseDir; //满足逆行
	 VIO_GROUP()
	 {
		 GroupId = -1;
		 bTurn = false;
		 bTouchOtherLine = false;
		 passLine = NULL;
		 pCar = NULL;
		 bDir = false;//尾牌//1:前牌
		 bMetPreLinout = false;
		 bMetReverseDir = false;

	 }


 }VioGroup;

class MvViolationDetecter : public MvRelabelEventHandler
{
public:

	MvViolationDetecter(const MvFindTargetByVirtualLoop *pFtg, MvLightStatusMem *pLightStatusMem, MvGroupLayer* pGroupLayer, 
		MvLine *pLnStop, MvLine *pLnFore, MvLine *pLnRight, MvLine *pLnLeft, const std::vector<MvLine> &vecYellowLine,
		const std::vector<MvLine> &vecBianDaoXian, const std::vector<MvLine> &vecWhiteLine,
		const std::vector<ChannelRegion> &lstChannelsInfo, int nRedLightVioDelayFrames, bool doVioDetect, bool doVioRedLight);
	~MvViolationDetecter();


	// 进行工作。
	void Process( std::vector<ViolationInfo> &vis, unsigned int uFrameSeq, MyCalibration *pCalib, unsigned int uMinBufferFrm, unsigned int uMaxBufferFrm );

	
	// 当同一辆车的group重新选择时，通知MvViolationDetecter进行一些维护操作。
	// 主要用于逆行、压线事件的多报。
	virtual void OnRelabeled(int nOldId, int nNewId);


	// 检查每个在uFrameSeq帧得到更新的group是否闯红灯。
	//void CheckForRedLightViolation(std::vector<ViolationInfo> &rlvis, unsigned int uFrameSeq, MyCalibration* pCalib, int nRedLightViolationAllowance = 0);


	

	

	

	// 禁行，特点时间段，
	//void CheckFor();



	//得到车道的行驶方向！
	int GetChannelDriveDir(int nRoadIndex);

	//得到车道是否为待行区！
	bool GetChannelHoldStopReg(int nRoadIndex);

	int GetChannelRedLightTime(int nRoadIndex);

	//判断是否有单个车道的前行线！
	bool IsHaveChannelForeLine();

	//得到每个车道的前行线！
	MvLine * GetChannelForeLine(int nRoadIndex);

	MvLine *MvGetChannelStopLine( int nRoadIndex );

	CvRect *MvGetChannelrectMedianPos( int nRoadIndex );

	void MvSetHoldStopLine( int nRoadIndex ,MvLine **HoldForeLineFirst, MvLine **HoldForeLineSecond,
		MvLine **HoldStopLineFirst, MvLine **HoldStopLineSecond );

	MvLine* GetStopLine() const;

	MvLine* GetForeLine() const;


	MvLine* GetLeftLine() const;

	MvLine* GetRightLine() const;

	std::vector<MvLine> GetYellowLine() const;
	std::vector<MvLine> GetWhiteLine() const;
	std::vector<MvLine> GetBiandLine() const;


	int MvGetmapVioGroup(int id)
	{
		return m_mapVioToGroup[id].GroupId;
	}

	void  mvSetYelGridPoly(const MvPolygon Ploy);
    void MvMakeYelGridImg(CvSize ImgSize);
	map<unsigned int ,unsigned int> m_mapcodeImg;
	


private:
	
	// 判断一个group有没有闯红灯。
	// 在红灯亮起之后的nRedLightViolationAllowance帧内这一段时间闯红灯的不算。
	//bool CheckForRedLightViolation(_MvGroup &g, unsigned int uFrameSeq, std::vector<ViolationInfo> &vis, MyCalibration* pCalib, int nRedLightViolationAllowance = 0);

	//停止行驶
	void CheckForStopViolation( _MvGroup* pGroup, std::vector<ViolationInfo> &vis, unsigned int uFrameSeq );

	void UpdatParkViolation(_MvGroup* pGroup, std::vector<ViolationInfo> &vis);


	bool ParkState(vector<MvCorner> &ParkConer, CvRect ParkRec);


	// 闯红灯
	void CheckForRedLightViolation2( _MvGroup* pGroup, std::vector<ViolationInfo> &rlvis, std::vector<ViolationInfo> &vis,
									unsigned int uFrameSeq, MyCalibration* pCalib, vector<_MvGroup *>&Viogroup,
									int nRedLightViolationAllowance = 0, unsigned int uMinBufferFrm = 0, unsigned int uMaxBufferFrm = 0);

	// 禁左禁右。
	void CheckForNoTurnLeftRightViolation( _MvGroup* pGroup, std::vector<ViolationInfo> &vis,
									unsigned int uFrameSeq, MyCalibration* pCalib, unsigned int uMinBufferFrm = 0, unsigned int uMaxBufferFrm = 0);



	// 逆行违章
	void CheckForRetrogradeViolation( _MvGroup* pGroup, std::vector<ViolationInfo> &vis, unsigned int uFrameSeq );
	int  LocalInChanleNum(CvRect GroupRec,int nChaneIndex);

	//黄网格停靠
	void CheckYelGridForParkViolation(_MvGroup* pGroup, std::vector<ViolationInfo> &vis, unsigned int uFrameSeq );


	//更新违章黄网格状态
	 void UpdatRushYelGridVio(_MvGroup* pGroup, std::vector<ViolationInfo> &vis,unsigned int uFramseq);

	// 压线违章
	void CheckForPressLineViolation( _MvGroup* pGroup, 
					std::vector<ViolationInfo> &vis,
					unsigned int uFrameSeq,
					bool isYellowLine = true
					);

	// 变道违章
	void CheckForBianDaoViolation( _MvGroup* pGroup, 
					std::vector<ViolationInfo> &vis,
					unsigned int uFrameSeq,
					MyCalibration* pCalib);

	void mvRecDirToLine(CvPoint p1,CvPoint p2,CvRect rect,int *nDriNum);


	// 禁行违章检测
	void CheckForNoPassingViolation( _MvGroup* pGroup, std::vector<ViolationInfo> &vis, unsigned int uFrameSeq );





	// 确定group是从哪个车道开过来的。false表示不知道。
	//bool GetGroupChannelSource(const _MvGroup &g, int &nChnIndex);

	//是否闯红灯暂缓输出！
	bool IsViolationWait(_MvGroup *pGroup);

	// 判断某一个group之前是否已经检测出某种类型的违章。
	bool IsVoilationIgnored(int nGroupId, VIO_EVENT_TYPE evt);

	// 判断某一个group之前是否已经记录压线。
	bool IsVoiPressRecode(int nGroupId, VIO_EVENT_RECORD evt);

	// 删除Group对应的违章。
	bool DelVoilationIgnored(int nGroupId, VIO_EVENT_TYPE evt);

	// 删除Group对应的违章。
	bool DelVoilationHist(int nGroupId, VIO_EVENT_TYPE evt);

    
	bool IsViolationDetected(int nGroupId, VIO_EVENT_TYPE evt) const;

	//找电警第一张图的合理位置！
	bool FindProperFstPos(bool bBreakTrafficLight, _MvGroup *pGroup, unsigned int uFstFrame, unsigned int uMinBufFrm, unsigned int uMaxBufFrm, \
		unsigned int uFirstRedLight, unsigned int &uProperFrame, unsigned int &uUpperFrm, unsigned int &uLowerFrm);
	
private:// 红灯状态相关
	
	// 红灯true，绿灯黄灯false  黄灯为false吗？不一定吧？！
	bool GetLightStatus(MvChannelFrameSignalStatus cfss, MvLine* pTouchLine) const;

	bool GetLightStatus(MvLine* pTouchLine, unsigned int uSeq, int nRoadIndex) const;
	
	//得到某一车道某一帧某个方向的红灯信号！nDriveDir = -1,左转灯，0，直行灯，1，右转灯；
	bool GetLightStatus(int nRoadIndex, unsigned int uSeq, int nDriveDir) const;

	unsigned int GetFirstRedLightFrameSeq(MvLine* pTouchLine, unsigned int uSeq, int nRoadIndex) const;

private: //位置相关。

	bool IsGroupUnderStopLine(unsigned int uFrameSeq, _MvGroup* pGroup, MvLine *pStopLine, MyCalibration* pCalib, CvRect *pRectMedain,
		float *pFirstPlateDist = NULL, bool bFlagViolation = false,bool bNoTurnLefRig = false) const;

	bool IsGroupPassStopLineXm(unsigned int uFrameSeq, _MvGroup* pGroup, MvLine *pStopLine, MyCalibration* pCalib, float fFirstPlateDist, float fPassLen = 1.0) const;

	bool IsGroupInChannel(unsigned int uSeq, _MvGroup* pGroup, int nRoadIndex);

	unsigned int GetFrameSeq( _MvGroup* pGroup, unsigned int uStartSeq, unsigned int uFinishSeq, float fRatio, unsigned int uUnderStopLineSeq) const;

	//取得闯红灯和禁左、禁右、禁前的第二张图的帧号！
	bool GetMedianFrame(_MvGroup *pGroup, unsigned int uFirstFrm, unsigned int uLastFrm, unsigned int uRoadIndex, unsigned int &uMedianFrm, bool bFlagStopLine = false);

private:

	bool GetCarInfo(_MvGroup* pGroup, unsigned int uFrameSeq, CarInfo &ci) const;

	void KeepHistAndIgnoreRecord(std::vector<ViolationInfo> &vis);


	void _UpdateDelayedGroups(unsigned int uMinBufferFrm, std::vector<ViolationInfo> &rlvis);


	bool GetDelayFrameSeqStatus( unsigned int uFrameSeq, ViolationInfo *pViolation );



	
	bool MvGetTrackTrunState (_MvGroup *pGroup);//判断目标是否存在拐弯状态
	bool mvGetCarTurn(Car *pCar,_MvGroup *pGroup,bool bTurnLef = true);
	int  mvGetOutState(const int nGrouId,const VIO_EVENT_TYPE &evtType);

	void mvCodeImg(std::vector<ViolationInfo> &vis,VIO_EVENT_TYPE evtType,unsigned int Fram0,unsigned int Fram1,unsigned int Fram2);
	

	



public:
	  void GetParkViol(std::vector<ViolationInfo> &vis, unsigned int uFrameSeq);

	  //为压线、变道的违章延时取第三张图
	  void MvGetVioFrame(std::vector<ViolationInfo> &vis,unsigned int uFrameSeq);

	  //在目标消失时将违章报出--涉及违章分级
	  void MvGetViolat (std::vector<ViolationInfo> &vis,unsigned int uFrameSeq);

	  void mvDelCode(std::vector<ViolationInfo> &vis,unsigned int Fram0,unsigned int Fram1,unsigned int Fram2);
		float  m_fScalWidht;
		VioGroup* GetVioId(int VioId)
		{
          return &m_mapVioToGroup[VioId];
		}
		std::vector<ViolationInfo>* GetParkVec()
		{
			return &m_villegParkOutput;
		}
		inline 	MvPolygon MvGetYelGridPoly()
		{
			return m_YelGridPoly;
		}

private:



	//vector<ViolationInfo> vStopCheck;//为无锡测试用，无任何实用价值



	std::vector<ViolationInfo> vDelayRedLightOutput;

	std::vector<ViolationInfo> m_villegParkOutput;

	

	MvGroupLayer *m_pGroupLayer;

	int m_nRedLightDelay;

	// 四条线。
	MvLine *m_pLnStop;
	MvLine *m_pLnFore;
	MvLine *m_pLnRight;
	MvLine *m_pLnLeft;


	//待行区四条线
	MvLine *m_pLnHoldForeFirst;
	MvLine *m_pLnHoldForeSecond;
	MvLine *m_pLnHoldStopFirst;
	MvLine *m_pLnHoldStopSecond;


	



	// 黄线
	std::vector<MvLine> m_vecYellowLines;

	// 白线
	std::vector<MvLine> m_vecWhiteLines;


	// 变道线
	std::vector<MvLine> m_vecBianDaoXian;

	// 车道信息
	std::vector<ChannelRegion> m_lstChannelsInfo;

	bool m_bHaveChannelForeLine;

	// 红灯状态记录器
	MvLightStatusMem *m_pLightStatusMem;

	//是否做了违章检测！
	bool m_bViolationDetection;
	//是否做了闯红灯！
	bool m_bViolateRedLight;

	// 忽略标，用于防止对一辆车进行多次违章输出。
	// 注意：需要选择一个恰当的时刻将特别老的记录的删除
	std::map< int, std::vector<VIO_EVENT_TYPE> > m_mapViolationIgnore; 

	// 违章记录，防止多次输出
	std::map< int, std::vector<VIO_EVENT_TYPE> > m_mapViolationHist;

	// 记录Group压线情况
	std::map< int, std::vector<VioPressRecode> > m_mapVioPressRecode;

	//为了解决左转或右转车辆先碰到前行线而误报闯红灯的问题，增加如下变量和设置！
	bool m_bUseNewViolationRules;
	bool m_bUseChongHongDeng;

	std::map<_MvGroup *, ViolationInfo> m_mapVioRedLightDelay;

	std::map<int, VioGroup> m_mapVioToGroup;//违章编号与Group的ID映射关系；（只记录不含待转区的闯红灯、禁左、禁右、禁前）
	

	const MvFindTargetByVirtualLoop *m_pFtg;

	std::vector<ViolationInfo> m_Violation;//（该缓冲队列，只记录不含待转区的闯红灯、禁左、禁右、禁前）


	unsigned int m_unID;
	int nTestFlag;
	IplImage *m_pYelGridImg;
	MvPolygon m_YelGridPoly;
};




#endif
