// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   14:24
	filename: 	e:\BocomProjects\find_target_lib\include\MvRectTracker.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	MvRectTracker
	file ext:	h
	author:		Durong
	
	purpose:	记录一个rect的每一帧的位置。在电警中，这个rect可以是框住车牌的rect，也可以
	是框住整个车辆的rect。
*********************************************************************/


#ifndef MV_RECT_TRACKER
#define MV_RECT_TRACKER


#include <map>
#include "cxcore.h"


//using namespace std;

// 记录每一帧、每个车牌----从group开始算起
class MvRectTracker
{
public:

	// 添加新的车牌进行车牌位置跟踪。
	void AddNewRect(CvRect rectPos, int nRectId, unsigned int uFrameSeq);

	// 更新跟踪中的车牌位置
	void UpdateRect(int nRectId, CvRect pos, unsigned int uFrameSeq);


	// 获取车牌的位置。
	bool GetRectPos(unsigned int uFrameSeq, int nRectId, CvRect &ret) const;


	// 获取车牌最后一次更新的位置
	bool GetRectLastPos(int nRectId, CvRect &pos) const;

	//获得车牌最老的一次
     bool GetRectFristPos(int nRectId, CvRect &pos) const;

	 //获取同一车牌检出位置的个数
	 int  GetRectNum(int nRectId) const;

	// 获取车牌倒数第二次更新的位置
	bool GetRectSecLastPos(int nRectId, CvRect &pos) const;


	unsigned int GetFirstTrackFrameSeq(int nRectId) const;

	unsigned int GetLastTrackFrameSeq(int nRectId) const;


	// 删除某一ID的rect的记录
	void DeleteRectTrack(int nPlateId);


	// 判断某一ID的rect是否正在记录中
	bool IsRectTracking(int nRectId);

	// 将某一ID的rect的位置记录拷贝出来。
	void CopyRectPosTrack(int nRectId, std::map<unsigned int, CvRect> &t) const;

	// 将位置记录信息填充到某一ID的rect名义之下
	void FillRectTrack(int nRectId, const std::map<unsigned int, CvRect> &t);


	//void Delete(unsigned int uFrameSeq, CarManager *pCarManager);


private:

	


	// 帧号->位置
	typedef std::map<unsigned int, CvRect> RectTrackRecord;
	typedef std::map<unsigned int, CvRect>::const_iterator TrackRecordConstIt;
	typedef std::map<unsigned int, CvRect>::const_reverse_iterator RevTrackRecordConstIt;
	typedef std::map<unsigned int, CvRect>::iterator TrackRecordIt;
	typedef std::map<unsigned int, CvRect>::reverse_iterator TrackRecordRIt;
	typedef std::map<int, RectTrackRecord>::const_iterator TracksConstIt;
	typedef std::map<int, RectTrackRecord>::const_reverse_iterator RevTracksConstIt;
	typedef std::map<int, RectTrackRecord>::iterator TracksIt;


	// PlateId to its tracks
	std::map<int, RectTrackRecord> m_tracks;

	

	



};

#endif