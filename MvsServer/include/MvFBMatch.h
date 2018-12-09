#ifndef __MV_FBMATCH_H__
#define __MV_FBMATCH_H__

#include <vector>
#include <cv.h>
#include "structdef.h"
using namespace std;

//#define MV_LOCAL_DEBUG		//本地调试用，不用则关掉

#ifdef  MV_LOCAL_DEBUG
typedef unsigned int        UINT32;
#define MAX_VIDEO			128
#define MAX_PLATE			16

/*
//车牌记录结构
typedef struct _RECORD_PLATE
{
	UINT32 uSeq;						//序列号
#ifdef MV_LOCAL_DEBUG
	int nYear;
	int nMonth;
	int nDay;
	int nHour;
	int nMin;
	int nSec;
#endif
	UINT32 uTime;						//识别车牌时间(秒)
	UINT32 uMiTime;					//识别车牌时间(毫秒)
	char chText[MAX_PLATE];					//车牌文本
	UINT32 uColor;					//车牌类型（颜色）

	UINT32 uCredit;					//识别可靠度
	UINT32 uRoadWayID;				//车道号

	UINT32 uType;						//车辆类型(低16位大、中、小，高16位卡车、巴士、轿车等）

	UINT32 uSmallPicSize;				//车牌小图大小
	UINT32 uSmallPicWidth;			//车牌小图宽度
	UINT32 uSmallPicHeight;			//车牌小图高度

	UINT32 uPicSize;					//车牌全景图片大小
	UINT32 uPicWidth;					//车牌全景图片宽度
	UINT32 uPicHeight;				//车牌全景图片高度

	UINT32 uPosLeft;					//车牌在全景图片中的位置左
	UINT32 uPosTop;					//车牌在全景图片中的位置上
	UINT32 uPosRight;					//车牌在全景图片中的位置右
	UINT32 uPosBottom;				//车牌在全景图片中的位置下


	UINT32 uCarColor1;				//车身颜色
	UINT32 uSpeed;					//车速
	UINT32 uDirection;				//行驶方向
	UINT32 uCarBrand;				//产商标志
	char chPlace[64];				//经过地点
	char chVideoPath[MAX_VIDEO];				//录像路径
	char chPicPath[MAX_VIDEO];				//大图片路径

	UINT32 uCarColor2;                    //车身颜色2

	UINT32 uWeight1;                    //车身颜色权重1
	UINT32 uWeight2;                    //车身颜色权重2

	UINT32 uSeqID;                      //帧序号
	UINT32 uPlateType;            //车牌结构
	UINT32 uViolationType;       //违章类型(闯红灯等)

	UINT32 uTypeDetail;       //车型细分
	UINT32 uStatusType;       //记录类型
	UINT32 Position;                   // 图象帧在文件中的开始位置（单位为字节）

	UINT32 uChannelID;                  //通道编号

	UINT32 uLongitude;//地点纬度(实际精度*10000, 精确到小数点后四位)
	UINT32 uLatitude; //地点经度(实际精度*10000, 精确到小数点后四位)
	UINT32 uTime2;						//第二车牌时间(秒)
	UINT32 uMiTime2;					//第二车牌时间(毫秒)
	UINT32 uAlarmKind;            //黑白名单报警1黑名单；2白名单
	UINT32 uSignalTime;				//红灯时间
	UINT32 uRedLightBeginTime;                    //红灯开始时间(秒)
	UINT32 uRedLightBeginMiTime;                  //红灯开始时间(毫秒)
	UINT32 uRedLightEndTime;                    //红灯结束时间(秒)
	UINT32 uRedLightEndMiTime;                  //红灯结束时间(毫秒)
	UINT32 uLimitSpeed;                    //限速值
	UINT32 uOverSpeed;                  //超速起拍值

	UINT32 uCameraId;//相机ID
	char chReserved[124];//扩展

	_RECORD_PLATE()
	{
		uSeq = 0;
		uTime = 0;
		uMiTime = 0;
		memset(chText,0,MAX_PLATE);
		uColor = 1;

		uCredit = 90;
		uRoadWayID = 0;

		uType = 0;

		uSmallPicSize = 0;
		uSmallPicWidth = 0;
		uSmallPicHeight = 0;

		uPicSize = 0;
		uPicWidth = 0;
		uPicHeight = 0;

		uPosLeft = 0;
		uPosTop = 0;
		uPosRight = 0;
		uPosBottom = 0;

		uSpeed = 0;
		uCarColor1 = 11;//未知
		uDirection = 0;
		uCarBrand = 1000;
		memset(chPlace,0,64);
		memset(chVideoPath,0,MAX_VIDEO);
		memset(chPicPath,0,MAX_VIDEO);

		uCarColor2 = 11;
		uWeight1 = 100;
		uWeight2 = 0;

		uAlarmKind = 0;
		uPlateType = 0;
		uViolationType = 0;

		uSeqID = 0;
		uTypeDetail = 0;
		uStatusType = 0;
		Position = 0;

		uLongitude = 0;
		uLatitude = 0;

		uChannelID = 0;
		uTime2 = 0;
		uMiTime2 = 0;
		uSignalTime = 0;
		uRedLightBeginTime = 0;
		uRedLightBeginMiTime = 0;
		uRedLightEndTime = 0;
		uRedLightEndMiTime = 0;
		uLimitSpeed = 0;
		uOverSpeed = 0;

		uCameraId = 0;
		memset(chReserved, 0, 124);

	}
}RECORD_PLATE;
*/


#else
#include "structdef.h"
#endif


/*


typedef struct _RECORD_PLATE_DSP_MATCH
{
	RECORD_PLATE dspRecord;
	IplImage *pImg;//图片
	IplImage *pImgArray[3];//图片

	_RECORD_PLATE_DSP_MATCH()
	{
		pImg = NULL;
		pImgArray[0] = NULL;
		pImgArray[1] = NULL;
		pImgArray[2] = NULL;
	}

}RECORD_PLATE_DSP_MATCH;

//匹配结果输出
typedef struct __MatchPair
{
	RECORD_PLATE_DSP_MATCH A;
	vector<RECORD_PLATE_DSP_MATCH> vecB;

	vector<int> vecDis;
}MatchPair;

//待匹配的信息
typedef struct _ObjMatchInfo
{
	RECORD_PLATE_DSP_MATCH plate;
	int nMatchTimes;

	_ObjMatchInfo()
	{
		nMatchTimes = 0;
	}
}ObjMatchInfo;
*/

class MvFBMatch
{
public:
	MvFBMatch(void);
	~MvFBMatch(void);

private:
	int m_nAChnl, m_nBChnl;										//记录匹配的通道号
	vector<ObjMatchInfo> m_vecA, m_vecB;						//队列
	vector<int> m_vecDiff;										//记录时间差	
	unsigned int m_uLastMatchTimeA, m_uLastMatchTimeB;			//最近匹配时间

private:
	int GetCarNumDis(const char * str1, const char * str2);
	int IsSimilar(const char c1, const char c2);
	int GetSimiDis(const char * str1, const char * str2);

	void CleanCache(vector<ObjMatchInfo> &vecCache, const unsigned int ts);											//清理队列中数据

#ifdef MV_LOCAL_DEBUG
public:
	void LogFile(vector<ObjMatchInfo> vecObj, const char *Path);
	void Match(vector<ObjMatchInfo> &vecA,vector<ObjMatchInfo> &vecB,const char *path, const char *unsure);
#endif


public:
	void mvInput(RECORD_PLATE_DSP_MATCH plate);
	vector<MatchPair> mvOutput();
	
};

#endif