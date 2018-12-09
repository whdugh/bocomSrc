#ifndef DSP_DATA_MANAGE
#define DSP_DATA_MANAGE
#include "global.h"
#ifndef ALGORITHM_YUV
#ifndef ALGORITHM_DL
#include "CarnumParameter.h"
#else
#include "Mv_CarnumDetect.h"
#endif
#include "MvVehicleClassify.h"
#include "MvFindTargetByVirtualLoop.h"
#include "CvxText.h"
#include "LabelRecognition.h"
#include "ColorRecognition.h"
#ifndef ALGORITHM_DL
#include "Mv_CarnumDetect.h"
#endif
#include "DspDataProcess.h"

typedef std::list<string> listPicData;
typedef std::map<UINT32, listPicData> mapPicData; //车辆编号-图片数据映射
typedef std::map<Picture_Key,listPicData> mapPKPicData;//（相机编号，图片帧号）-图片数据映射

#define  IMAGEBUFUNIT	80 //Jpg缓冲大小
#define  MAX_TIME_DIS	420	//最大有效时间间隔,500s
#define MAX_TIME_DEAL  1200 //最大强制输出时间间隔,1200s

class CDspDataProcess;

typedef struct tagImageBuf
{
	int nImgWidth;
	int nImgHeight;
	IplImage* imgSnap;//卡口
	IplImage* imgComposeSnap;// 违章
	IplImage* imgComposeResult;// 违章结果
	bool bUsed;

}SImageBuf,*PSImageBuf;

typedef struct tagDataProcess
{
	UINT32 uCameraId;
	UINT16 nCameraType;
	UINT16 nType;// 2:卡口 10:违章

	SImageBuf imgBufForSimplePic;
	SImageBuf imgBufForComposePic;
	SImageBuf imgComposeResultPic;

	BYTE* pBuffer;
}SDataProcess,*PSDataProcess;

typedef struct tagDspDataManage
{
	int nIndex;
	void* pManage;
}SDspDataManage,*PDspDataManage;

typedef struct tagDspOutPutKeyTemp
{
	Picture_Key key;
	int64_t ts;
	int64_t tsRecv;	//接收数据时间
	CDspDataProcess* pDataProc;

	bool operator < (const tagDspOutPutKeyTemp& other) const
	{
		if(tsRecv < other.tsRecv)
		{
			return true;
		}
		else if (ts < other.ts)        //按时间戳排序
		{
			return true;
		}
		else if (ts == other.ts)  //如果帧号相同，按比相机ID升序排序
		{
			return key < other.key;
		}

		return false;
	}
} SDspOutPutKeyTemp, *PDspOutPutKeyTemp;

typedef std::map<SDspOutPutKeyTemp, string> CarnumMap;

class CDspDataManage{
public:
	CDspDataManage(int nDataCount = 10);
	~CDspDataManage();

public:
	//初始化
	bool InitDspData(int nChannelId, CHANNEL_DETECT_KIND nDetectType = DETECT_NONE);
	bool AddFrame(char* pBuffer);
	int DataProcessThreadFor200W();
	int DataProcessThreadFor500W();
	int DoDataProcessFor200W(int nIndex);
	int DoDataProcessFor500W(int nIndex);

	int DataProcessThreadOutPutTemp();
	int DoDataProcessOutPutTemp(int nIndex);

	bool GetImageByJpgKey(const Picture_Elem &PicElem,PLATEPOSITION *pTimeStamp,IplImage* pImage);
	int GetServerIpgCount();

	//核查是否输出图片
	bool CheckCarNumOutPut(const Picture_Key &Pic_Key, const int64_t &ts);
	//根据Picture_Key,时间戳,在JpgMap取图
	bool FindPicFromJpgMap(Picture_Key key, int64 ts, string &strPic);
	//确认是否,能根据Picture_Key,时间戳,在JpgMap取到图
	bool IsFindPicFromJpgMap(Picture_Key key, int64 ts);

private:
	bool AddJpgFrame(BYTE* pBuffer);
	//void GetPicByKeyFromMap(Picture_Key pickeyDst, string& strPic);
	int RemoveServerJpg(Image_header_dsp *pHeader);

private:
	int m_nChannelId;
	int m_nDataCount;
	int m_nThreadCntFor200W;
	int m_nThreadCntFor500W;

	int m_nThreadCntForTemp;

	//文本区域高度
	int m_nExtentHeight;
	int m_nWordPos;

	//jpg互斥锁
	pthread_mutex_t m_JpgFrameMutex;
	pthread_mutex_t m_WaitFor200WMutex;
	pthread_mutex_t m_WaitFor500WMutex;

	pthread_mutex_t m_OutPutMutex;

	//关键字为帧号和相机编号组成的结构体的JPG大图队列(检测器为服务端)
	//map<Picture_Key,string> m_ServerJpgFrameMap;
	 JpgMap m_ServerJpgFrameMap;

	static int               m_DataExit;//程序退出标识

	vector<string> m_mapWaitListFor200W;
	vector<string> m_mapWaitListFor500W;
	unsigned long long 	m_nWaitNumFor200W;
	unsigned long long 	m_nWaitNumFor500W;

	CarnumMap m_mapCarnumOut; //车牌输出缓存
	unsigned long long 	m_nWaitNumForTemp;
	int m_nTestSeq;
	CHANNEL_DETECT_KIND m_nDetectKind;
};


#endif
#endif
