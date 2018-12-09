#ifndef MATCH_PLATE_H
#define MATCH_PLATE_H

#include "structdef.h"
#include "CommonHeader.h"
#include "Thread.h"
#include "DspDataProcess.h"

class CDspDataProcess;
class CMatchPlate
{
public:
	CMatchPlate();
	~CMatchPlate();

public:
	// 启动检测线程
	int InitMatchPlate();
	static threadfunc_t STDPREFIX BeginCheckPlate(threadparam_t lpParam);
	void CheckPlate();

	// 插入前排检测结果到list
	void PushFrontPlate(int nChannel, RECORD_PLATE_DSP_MATCH data);
	// 插入后排检测结果到待检查List
	void PushBackPlate(RECORD_PLATE_DSP_MATCH data);

private:
	void DealPlate(RECORD_PLATE_DSP_MATCH record_plate);
	// 返回值：-1:没有找到；0:找到；1：找到但都有车牌
	int SearchFrontPlate1(RECORD_PLATE_DSP_MATCH record_plate, RECORD_PLATE_DSP_MATCH& foundRecord);
	// 返回值：-1:没有找到；0:找到；1：找到但都有车牌
	int SearchFrontPlate2(RECORD_PLATE_DSP_MATCH record_plate, RECORD_PLATE_DSP_MATCH& foundRecord);	
	void OutPutPlate(RECORD_PLATE_DSP_MATCH record_plate, RECORD_PLATE_DSP_MATCH foundRecord);
	void SendResult(RECORD_PLATE& plate,unsigned int uSeq);

	//超过15s前的车牌记录删除
	bool IsLastPlate(UINT32 uPlateTime);

private:
	static int               m_exit;//程序退出标识

	std::list<RECORD_PLATE_DSP_MATCH> m_FrontPlateList1; //车道1,前排检测结果
	std::list<RECORD_PLATE_DSP_MATCH> m_FrontPlateList2; //车道2,前排检测结果

	std::list<RECORD_PLATE_DSP_MATCH> m_BackPlateList;	// 后排检测结果

	pthread_mutex_t m_FrontPlate1_Mutex;
	pthread_mutex_t m_FrontPlate2_Mutex;
	pthread_mutex_t m_BackPlate_Mutex;

	CDspDataProcess* m_pDataProc200W;
	CDspDataProcess* m_pDataProc500W;
};

extern CMatchPlate g_matchPlate;

#endif
