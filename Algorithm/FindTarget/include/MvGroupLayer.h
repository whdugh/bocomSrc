#ifndef MV_GROUP_LAYER_H
#define MV_GROUP_LAYER_H

#include <list>
#include <vector>
#include <map>
#include "MvTrackLayer.h"
#include "MvGroup.h"
#include "MvMaxSize.h"
#include "disjoint-set1.h"
#include "MvPolygon.h"
#include "MvRectTracker.h"
#include "MvFindTargetByVirtualLoop.h"
#include "MvRelabelEventHandler.h"


//using namespace std;
using namespace PRJ_NAMESPACE_NAME;

//class MvRectTracker;
class CarManager;
class MvFindTargetByVirtualLoop;
//struct _MvGroup;

class MvGroupLayer
{

public:
	
	// 构造函数。
	MvGroupLayer(MvCornerLayer *pCornerLayer, MvTrackLayer* m_pTrackLayer, MvMaxSize *pMaxSize,
		MyCalibration *pCalib, MvFindTargetByVirtualLoop *pFtg);
	
	// 析构函数
	~MvGroupLayer();

	
	// 轨迹集合类。存储了一些轨迹及其终点区域。
	typedef struct _TrackSet
	{
		std::vector<MvTrack*> tracks;//所包含的轨迹
		CvRect rect;// 轨迹的区域

		// 计算集合中轨迹所属的group。
		bool GetTrackSetGroupId(int &nGroupId, int &nCount) const;

		int GetTracksCount() const;

		void mvGetMedainDirection( double &fx, double &fy ) const;

	} TrackSet;


	// 将tracks进行分割。分割成一些集合。
	void SegTracks( const std::vector<MvTrack*> &tracks, std::vector<TrackSet> &sets, 
		int nLeastCommonFrames = 3, float fPhyDis = 3.0, float fMotionSimility = 0.4 ,bool bPlate =false,CvRect 
		 PlateRec =cvRect(0,0,1,1),unsigned int uCarId =-1);



	// 车辆检出。
	_MvGroup* OnCarDetected( const std::vector<MvTrack*> &tracks, CvRect rgn, unsigned int uFrameSeq);


	// 车牌出现。
	void OnCarNumDetected(std::vector<ViolationInfo> &vis,const std::vector<MvTrack*> &vecTracks, const std::vector<Car*> vecCars, 
		unsigned int uFrameSeq, float fScaleX, float fScaleY, CarManager *pCarManager, 
		std::list<CvRect> &lstRectForDebug, std::list<MvPolygon> &lstPolyForDebug,const IplImage *img =NULL);


	// 删除一些没用的Group
	void DeleteGroups(unsigned int uFrameSeq, unsigned int uInterval,std::vector<ViolationInfo> &vis);


	void MergeGroups(CarManager *pCarManager, unsigned int uFrameSeq,std::vector<ViolationInfo> &vis);


	_MvGroup* GetGroupById(int nGroupId);


	// 用trackset更新groups。
	void UpdateGroups(unsigned int uFrameSeq, const std::vector<MvTrack*> &tracks, IplImage *img);

	//得到Group运动方向，与停止线法线的夹角！要求停止线要画好！
	bool GetGroupMotionDir(_MvGroup *pGroup, /*unsigned int uStartFrmSeq, unsigned int uFinishFrmSeq,*/ const MvLine *stopLine, float fMinDis, float &motionAngle);
	// 
	bool GetGroupMotDirByRectTracker(MvGroup* pGroup, float fMinDis, float &vx, float &vy) const;

	bool GetGroupMotDir(_MvGroup *pGroup, double &fTrackMotX, double &fTrackMotY) const;

	bool IsGroupExist(int nGroupId);


	// 判断group最后一次更新时，是不是压着某根线
	MvLine* IsGroupOnLine(MvLine *pForeLine, MvLine *pLeftLine, MvLine *pRightLine, _MvGroup* pGroup,bool &bTurnState) const;

	bool IsGroupOnLine(_MvGroup *pGroup, const MvLine *pLine) const;

	// 求group所代表的车辆，是从哪个车道开出来的。
	bool GetGroupRoadIndex(_MvGroup *pGroup, std::vector<ChannelRegion> m_lstChannelsInfo, int &nRoadIndex, bool bFlag = false ) const;

	
	// 画Groups
	void DrawGroups(IplImage *img, unsigned int uFrameSeq,const std::vector<MvTrack*> &vecTracks);


	const MvRectTracker* GetPlateTracker() const;

	const MvRectTracker* GetCarTracker() const;


	void AddRelabelEvtListener(MvRelabelEventHandler *listener);


	MvMaxSize* GetMaxSize() const;
	void InformTrackTrans( unsigned int uFrameSeq, const vector<MvTrack*> &allTracks, IplImage *img );


	friend class MvViolationDetecter;
	// 
	//void BeforeTrackDeleteEventHandler(int nTrackIndex);

private:


	

	// 更新不带车牌的groups
	void _UpdateNonPlateGroups(unsigned int uFrameSeq, const std::vector<MvTrack*> &allTracks, IplImage *img);

	// 更新带车牌的groups
	void _UpdatePlateGroups(unsigned int uFrameSeq, const std::vector<MvTrack*> &allTracks, IplImage *img);

	//
	bool SelectTracks(const Car *pCar, const std::vector<TrackSet> &sets, const CvRect &rectSelSetRgn,
		const std::vector<MvTrack*> &tracks, const MvPolygon &polySelTracksRgn, TrackSet &selSet);



	// 按照区域位置选择set。从一系列set中选择一个和rectSelRgn位置比较匹配的set。
	bool SelectSet(const std::vector<TrackSet> &sets, CvRect rectSelRgn, TrackSet &set ,bool MotoStyle = false);


	// 对tracks进行分割。并选择轨迹最多的set输出。
	bool FormAndSelectSet(const std::vector<MvTrack*> &tracks, TrackSet &set, float dx = 0.0f, float dy = 0.0f);

	// 将set形成MvGroup
	void FormAddGroupByCarNum(TrackSet &set, unsigned int uFrameSeq, Car *pCar, CvRect carNumPos);


	
	// 删除group,返回值标记group是否存在。
	bool DeleteGroup(int nGroupId,std::vector<ViolationInfo> &vis);

	// 
	//static Universe* GenTrackUniverse(const vector<MvTrack*> &tracks);

	// 估计group应该具有的轨迹数量。假定车辆在total中的位置为pos，且在total标识的区域中提取nMaxCorners个角点。
	static int EstimateGroupTrackCount(CvRect pos, CvRect total);


	// 根据车牌估计车在图像里的多边形。
	MvPolygon EstimateCarPolyByCarNum(CvRect rectCarNum, float fWidth, float fLength);


	
private:

	// group层所持有的groups。int表示group的编号。
	std::map<int, _MvGroup> m_Groups;


	// 轨迹层对象。
	MvTrackLayer* m_pTrackLayer;


	// 角点层对象
	MvCornerLayer *m_pCornerLayer;


	//
	MvMaxSize *m_pMaxSize;


	// Group形成时m_pCarPlateTracker开始记录车牌位置。Group删除时相应的plate记录删除。
	MvRectTracker *m_pCarPlateTracker;

	// 
	MvRectTracker *m_pCarTracker;


	MyCalibration *m_pEleCalib;


	MvFindTargetByVirtualLoop *m_pFtg;



private:


	// 维护m_mapLastTryGenGroupSeq记录，将被删除的车牌，从m_mapLastTryGenGroupSeq记录里删掉。
	// 防止一些记录在m_mapLastTryGenGroupSeq一直得不到删除。
	// m_mapLastTryGenGroupSeq里的数据相对于CarManager里的Car允许有一些延时，没有很高的及时性要求。
	// 即：车牌已经删除了m_mapLastTryGenGroupSeq里还存在该车的数据也可以。
	// 所以调用该方法没有严格的位置要求。
	void KeepCarLastTryGenGroupSeq(CarManager* pCarManager);


	// CarId, frameSeq. 上次为id为carid的车尝试产生group的帧号（不管有没有产生Group）
	// car删除时相应的记录要删除。
	std::map<int, unsigned int> m_mapLastTryGenGroupSeq;



private:// 重新标记相关

	// 重新标记侦听着。
	std::vector<MvRelabelEventHandler*> m_vecRelabelEvtListener;
	void BroadCastRelabelEvt( int nOldId, int nNewId );

};

#endif