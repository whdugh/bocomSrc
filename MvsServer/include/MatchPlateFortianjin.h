#ifndef FB_MATCH_PLATE_H
#define FB_MATCH_PLATE_H
//#ifdef FBMATCHPLATE

#include "structdef.h"
#include "CommonHeader.h"
#include "Thread.h"
#include "DspDataProcess.h"

//#ifdef MATCH_LIU_YANG
	#include "MvFBMatch2.h"
//#else
//	#include "MvFBMatch.h"
//#endif


//匹配结果输出
typedef struct __MatchPlate
{
	RECORD_PLATE_DSP_MATCH A;
	RECORD_PLATE_DSP_MATCH B;
}MatchPlate;

class CMatchPlateFortianjin
{
public:
	CMatchPlateFortianjin();
	~CMatchPlateFortianjin();

public:
	int InitMatchPlate();

	// 启动取得对比车牌检测线程
	int CheckMatchPlate();
	static threadfunc_t STDPREFIX BeginCheckPlate(threadparam_t lpParam);
	void CheckPlate();

	// 启动处理对比车牌检测线程
	int DealMatchPlate();
	static threadfunc_t STDPREFIX BeginDealPlate(threadparam_t lpParam);
	void DealPlate();

	//
	static void* DoDealPlate(void* lpParam);
	static void* DoDealNoMatchPlate(void* lpParam);
	// 输入卡口数据给对比处理
	void mvInput(RECORD_PLATE_DSP_MATCH &plate);

	//设置录像处理指针
	void SetTempRecord(CRoadRecordTemp &recordTemp) { m_pRecordTemp = &recordTemp; }

	//输出违章前后牌匹配录像,返回录像路径
	std::string DealOutPutMachVideo(RECORD_PLATE &plateA, RECORD_PLATE &plateB);

	//构造前后牌不匹配事件
	void SetMatchEvent(const std::string &strVideoPath, const RECORD_PLATE &plate, const bool &bVideoFlag, std::string &strEvent);

	//获取录像路径
	std::string GetVideoSavePath(RECORD_PLATE& plate);

	//
	bool PopMatchData(MatchPlate & sMatchPlate);
private:
	void DoDealPlateFunc(RECORD_PLATE_DSP_MATCH &A, RECORD_PLATE_DSP_MATCH &B);
	void DoDealNoMatchPlateFunc(RECORD_PLATE_DSP_MATCH &plate);
private:
	static int               m_exit;//程序退出标识
	vector<MatchPlate> m_vcMatchList;
	pthread_mutex_t m_MatchList_Mutex;

	CDspDataProcess* m_pDataProc200W[4];
	CDspDataProcess* m_pDataProc500W[4];

//#ifdef MATCH_LIU_YANG
//	MvFBMatch2* m_mvMatch2;
//#else
//	MvFBMatch* m_mvMatch;
//#endif

	CRoadRecordTemp* m_pRecordTemp;
};

extern CMatchPlateFortianjin g_matchPlateFortianjin;
//#endif //FBMATCHPLATE
#endif //FB_MATCH_PLATE_H
