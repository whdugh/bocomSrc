#ifndef MV_TRACK_LAYER_H
#define MV_TRACK_LAYER_H

#include <fstream>
#include <vector>
#include "MvCornerLayer.h"
#include "MvTrack.h"

#include "MvPolygon.h"
#include "disjoint-set1.h"
#include "OpticalFlowHelper.h"
#include "StdAfx.h"

using namespace std;

#define TRACK_MAX_NUM 1024


class MvTrackLayer
{
public:

	MvTrackLayer(MvCornerLayer *pCornerLayer);

	~MvTrackLayer();

	// 更新、新增track
	void UpdateTracks(const std::vector<MvCorner*> &vecCornerIndex, const unsigned int uFrameSeq,const  int64 ts, const unsigned int uTimestamp,
		const MyCalibration *pCalib,const  MvMaxSize *p_Maxsize);

	void Speed_UpdateTracks(MvCorner** pCors, int nCount, const unsigned int uFrameSeq, const int64 ts,const  unsigned int uTimestamp,
		const MyCalibration *pCalib, const MvMaxSize *p_Maxsize);

	void UpdateTracks(MvCorner** pCors, int nCount, const unsigned int uFrameSeq, const int64 ts,const  unsigned int uTimestamp,
		const MyCalibration *pCalib, const MvMaxSize *p_Maxsize);

	// 删除不合理的轨迹。
	void RmErrorTrack(const unsigned int uDelta,const unsigned int uFrameSeq,const CvRect MaskRect);

	// 删除没用的track
	void RmUselessTracks();

	//
	void SetPreitCornImgs(IplImage *PredImg, const unsigned int uFrameSeq, const int64 ts,
		const unsigned int uTimestamp,
		const FindTargetElePolice::MyCalibration *pCalib,const  MvMaxSize *p_Maxsize,CvRect CarRioRec
		,const vector<CvRect> &GrouArea);


	// 删除不合理的轨迹。
	void RmUnreasonableTrack();

	// 删除最后一次跟新比uFrameSeq小的轨迹
	void RmTracksBefore(unsigned int uDelta, unsigned int uFrameSeq);

	void RmTracksBeyondMask(CvRect MaskRect);
	
	void MoveTrackAddress(int nTrackIndex);


	// 画tracks
	void DrawTracks(IplImage* img, unsigned int uFrameSeq);

	void DrawCurrenTracks(IplImage* img, unsigned int uFrameSeq);


	// 获取最后一次更新轨迹时，活跃（得到更新,不含新加和估计的）的轨迹的索引
	//void GetLastTracks(std::vector<int> &lstTrackIndex);

	
	// 获取最后一次更新轨迹时，活跃（得到更新,不含新加和估计的）的轨迹的指针。
	
	void GetLastTracks(const  MvMaxSize *p_Maxsize,unsigned int uFrameSeq, std::vector<_MvTrack*> &lstTrackPointer, std::vector<_MvTrack*> &lstEleTrackPointer,bool bIsDay);


	// 根据索引号取得轨迹
	  _MvTrack* GetTrackPointerByIndex(int index);

	// GetLastFrameTracks();

	// 

	void WriteTracksToTxt(std::ofstream &ofs);


	// 计算轨迹之间距离不变性。同一辆车上的非噪声轨迹应该具有较好的距离不变性。
	static void DisKeeplity(std::vector<MvTrack*> &tracks, float& vec, float &scalar);


	// 任一个track在universe里的下标与在tracks里的下标相同。
	static Universe* GenTracksUniverse(const std::vector<MvTrack*> &tracks);


	


	// 计算每个track最后一次位置点组成的集合的外截rect
	static CvRect GetBoudingRect(const std::vector<MvTrack*> &tracks);

	// 计算每个track最后一次位置点组成的集合的外截rect
	static CvRect GetBoudingRect(const std::vector<MvTrack*> &tracks1, const std::vector<MvTrack*> &tracks2);
	
	// 计算每个track倒数第二个位置点组成的集合的外截rect
	static CvRect GetSecBoudingRect(const std::vector<MvTrack*> &tracks);

	// 从轨迹集合src中提取出最晚一个角点在rgn区域里的轨迹。
	static void GetTracksInRegion(const std::vector<MvTrack*> &src, std::vector<MvTrack*> &dest, CvRect rgn);
	static CvRect GetRecTracksInRegion(const std::vector<MvTrack*> &src, CvRect rgn);

	
#ifdef TRACK_GROUP_LGW

	// 从轨迹集合src中提取出最晚一个角点在rgn区域里的轨迹。
	static void GetTracksInRegion(const std::vector<MvTrack*> &src,std::vector<MvTrack*> &RemainTrack,
		std::vector<MvTrack*> &dest, CvRect rgn);

	// 从轨迹集合src中提取出最晚一个角点在rgn区域里的轨迹。
	static void GetTracksInRegion(const std::vector<MvTrack*> &src, std::vector<MvTrack*> &dest, CvRect rgn,int nID);
	static void SetTracksDisToPlate( std::vector<MvTrack*> &src,CvPoint Plate);
	static void GetPlateByTracksDis(const MvMaxSize *pMaxSize,const std::vector<MvTrack*> &src,
		GroupFlow *pGrowFlow,CvPoint &Plate);
	static void SetTracksCarIdAndPlateDis(std::vector<MvTrack*> &src,unsigned int uCarId,CvPoint Plate,bool bPark =false);


#endif
	
	//获取轨迹中最左或者左右的轨迹，false是去最左边的
	static _MvTrack* GetSideTrack( std::vector<MvTrack*> &tracks,bool RigTrack= true);

	// 获取轨迹中最长的一条。角点数最多。
	static _MvTrack* GetLongestTrack(const std::vector<MvTrack*> &tracks);

	// 从轨迹集合src中提取出最晚一个角点在poly区域里的轨迹。
	static void GetTracksInRegion(const std::vector<MvTrack*> &src, std::vector<MvTrack*> &dest, const MvPolygon &poly);


	static void RemoveTracksInRgn(const std::vector<MvTrack*> &src, std::vector<MvTrack*> &dest, CvRect rgn);

	// 对轨迹按照候选次数进行排序
	static void SortTrackByInformalTimes(std::vector<MvTrack*> &tracks);

	// 判断pTrackA的候选次数是不是大于pTrackB
	static bool IsTrackAHaveMoreInformalTimesThanTrackB(const MvTrack* pTrackA, const MvTrack *pTrackB);

	// 判断pTrack按X从小往大排序
	static bool IsTrackXUp(const MvTrack* pTrackA, const MvTrack *pTrackB);
	

	// 获取ts中轨迹最后一个角点到pt的y方向距离的平均值。
	static float GetTracksToPointDis(const std::vector<MvTrack*> &ts, CvPoint pt);


	 void DrawTracks(const std::vector<MvTrack*> &tracks, IplImage *img, CvScalar color=CV_RGB(0, 0, 0)
		,bool bDiscFront = false);

	// 获取轨迹运动方向的方差
	static float GetTrackDirVar(const std::vector<MvTrack*> &tracks);

	// 统计tracks中，travelLength大于fTravLen，并且角点个数大于fCorCount的轨迹个数。
	static int CountTracks(const std::vector<MvTrack*> &tracks, float fTravLen, float fCorCount);


	// 已知某个目标在x帧的位置rect，以及目标在x帧所包含的轨迹，计算目标在x-1，x-2，x-3...帧的位置。
	static std::map<unsigned int, CvRect> GoBack(MvMaxSize *pMaxSize, unsigned int x, CvRect rect, const std::vector<MvTrack*> &tracks);

	

private:

	MvTrackLayer(const MvTrackLayer& l);

	MvTrackLayer & operator = (const MvTrackLayer &l);

	// 添加新的轨迹。nIndex角点。返回新轨迹的索引。
	int AddNewTrackByCornerIndex(int nIndex);

	// 
	int AddTrack(const _MvTrack &t);

	void SetTrackIndex(_MvTrack *pTrack, int nIndex);
	


private:
	
		
	// 存储最后一次更新轨迹时，活跃（得到更新,不含新加和估计的）的轨迹的索引
	std::vector<int>     m_vecLastTrackIndex;

	// 存储最后一次更新轨迹时，活跃（得到更新,不含新加和估计的）的轨迹的指针。
	std::vector<_MvTrack*> m_vecLastTrackPointer;


	// 存储最后一次更新轨迹时，活跃（得到更新,不含新加和估计的）的轨迹的指针。
#ifdef SPEED_UP
       _MvTrack** m_vecLastTracksAll;
	   int m_LastTracksNum;
#else
	std::vector<_MvTrack*> m_vecLastTracksAll;
#endif
	


	// 角点层对象
	MvCornerLayer *m_pCornerLayer;
	


	// memory storage
	CvMemStorage *m_pMem;

	// 存储轨迹
	CvSet        *m_pTrackSet;

	int  *g_pTrack_Index;//存储轨迹索引号（0~1024）

	_MvTrack  *g_Track;
	int m_TrackPos; //指向轨迹待存储的下一个位置
	
};



#endif