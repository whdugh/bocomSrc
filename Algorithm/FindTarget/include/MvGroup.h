#ifndef MV_GROUP_H
#define MV_GROUP_H



#include <map>
#include <vector>
#include "cxcore.h"
#include "Car.h"
//#include "SensedObject.h"
#include "MvTrackLayer.h"
#include "MvViolationDetecter.h"
#include "ViolationInfo.h"



//using namespace std;

//struct SensedObject;

#define  BINDA_MAX_NUM 10


typedef struct _MvGroup
{

public:
	
	_MvGroup() {};
	// 构造函数
	_MvGroup(MvTrackLayer *pTrackLayer, unsigned int uFrameSeq/*, CvRect rectCarPosWhenGroupFormed*/);


	// 获取group的编号。每个group有一个唯一的编号。
	int GetGroupId() const;

	inline void SubId()
	{
		_MvGroup::nGroupIdUsed--;
	}
	inline void SetId(int id)
	{
		_MvGroup::nGroupIdUsed = id;
		m_nGroupId = id;

	}

	// 根据轨迹估计车辆之前位置
	void GoBackward(FindTargetElePolice::MyCalibration *pCalib, MvMaxSize *pMaxSize);


	// 设置group所关联的车牌。
	void SetCar(Car *pCar);

	// 
	Car* GetCar() const;


	SensedObject* GetAssocSensedObj() const;

	void SetAssocSensedObj(SensedObject* pObj)
	{
		m_pAssocSensedObj = pObj;
	}


	// 在Group被删除前，group需要做的善后工作。
	void Delete();


	// 设置group在uFrameSeq帧的位置、所包含的轨迹。并设置轨迹的m_nGroupId.
	void Update(unsigned int uFrameSeq, CvRect rectGroup, const std::vector<MvTrack*> &vecTracks);

	//获取指定帧的Group位置！
	bool GetGroupPos(unsigned int uFrmSeq, CvRect &rectPos) const;

	// 获取group最后一次更新时在图像上所处位置。
	CvRect GetGroupLastPos() const;


	// 获取group第一次更新时的位置
	CvRect GetFirstPos() const;


	// 取得group生成时刻的帧号。
	unsigned int GetGroupGenFrameSeq() const;


	// Group形成那一帧Group在图像中的位置。
	CvRect GetGroupGenPos() const;


	// Group形成那一帧车辆在图像中的位置（根据车牌猜测的）。
	//CvRect GetGroupGenCarPos() const;
	

	// 获取group倒数第二次更新时的位置
	CvRect GetGroupSecLastPos() const;


	// 获取最长的轨迹。
	MvTrack* GetLongestTrack() const;

    


	// 获取group上一次更新时所包含的轨迹。
	void GetGroupLastTracks(std::vector<int> &vecTracks) const;

	
	// 
	void GetGroupLastTracks(std::vector<MvTrack*> &vecTracks) const;


	


	// 
	int GetGroupLastTrackCount() const;

	unsigned int GetFirstFrameSeq() const;
	unsigned int GetlastSecdFrameSeq() const;
	


	int GetGroupHistorySize() const;


	// 获取group上一次更新时的帧号。
	unsigned int GetLastUpdateFrmSeq() const;

	// 将group的框框画到img上，并将其包含的轨迹画到图上。
	void DrawGroup(IplImage *img, unsigned int uFrameSeq) const;

	void DrawGroupRet( IplImage *img,CvPoint point =cvPoint(0,0) ) const;

	// 拷贝group位置历史记录。
	void CopyHistory(std::map<unsigned int, CvRect> &mapHist);

	// 将mapHist里面的记录填充到m_history里面，如果m_history已经有的就不填充了。
	void FillHist(std::map<unsigned int, CvRect> &mapHist);


	


	friend class MvViolationDetecter;

private:
	
	// Group形成那一帧车辆在图像中的位置（根据车牌猜测的）。
	//CvRect m_rectCarPosWhenGroupFormed;

	// group id产生器。
	static int nGroupIdUsed;


	// group的编号。从1开始。
	int m_nGroupId;


	// group产生的帧号。它并不一定是m_history的begin().
	// 因为group产生时，它会往更早的时间倒退，猜测其在较早时刻位置。	
	unsigned int uGenFrameSeq;


	// 上一次更新时的帧号。
	unsigned int m_uLastUpdateFrmSeq;


	// Group所关联的车牌号码。
	Car *m_pCar;


	// 关联的虚拟线圈目标。
	SensedObject *m_pAssocSensedObj;


	// Group所包含的轨迹
	std::vector<int> m_vecTracks;





	// 
	//bool m_bViolationDetectIgnore;


	// 逆行检测忽略。
	//bool m_bReverseViolationIgnore;


	// 轨迹层对象
	MvTrackLayer *m_pTrackLayer;

	CvPoint   m_LastTrackVec;

	
	
	// group各帧在图像上的位置的记录。
	std::map<unsigned int, CvRect> m_history;
    int  m_BiandDaoDir[BINDA_MAX_NUM];

	


	// 
	typedef std::map<unsigned int, CvRect>::iterator GroupHistoryIt;
	typedef std::map<unsigned int, CvRect>::const_iterator GroupHistoryConstIt;


	// 设置group id
	//void SetGroupId(int nId);

public:
	// Group所包含的轨迹
	std::vector<MvTrack*> m_vecTracksPointer;
	bool m_bDelayedOutput;  //Group当前是否处于闯红灯等待输出阶段！
	bool m_bMetReServe;//是否满足逆行
	bool m_bUseCarFram;

	

} MvGroup;


#endif