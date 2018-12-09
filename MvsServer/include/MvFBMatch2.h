#ifndef __MV_FBMATCH2_H__
#define __MV_FBMATCH2_H__

#include <vector>
#include <cv.h>
#include "structdef.h"
using namespace std;

#define MAX_MATCH_SIZE 60 //< MAX_IMG_TAG_NUM/3
typedef std::map<int, vector<ObjMatchInfo> > VecMatchMap;

class MvFBMatch2
{
public:
	MvFBMatch2(void);
	~MvFBMatch2(void);

private:
	//const int m_nADirection, m_nBDirection;					//记录匹配的方向，A,放尾牌，B放前牌
	vector<ObjMatchInfo> m_vecA/*, m_vecB*/;						//队列
	vector<int> m_vecDiff;										//记录时间差	
	unsigned int m_uLastMatchTimeA, m_uLastMatchTimeB;			//最近匹配时间
	VecMatchMap m_ChanIDAndFPlateMap;							//通道和前牌对应表
	vector<RECORD_PLATE_DSP_MATCH> m_NoMatchList;				//保存未找到的车牌
private:
	int GetCarNumDis(const char * str1, const char * str2);
	int IsSimilar(const char c1, const char c2);
	int GetSimiDis(const char * str1, const char * str2);

	void CleanCache(vector<ObjMatchInfo> &vecCache, const unsigned int ts);	//清理队列中数据
	void WriteFPlateToMap(ObjMatchInfo & ObjMat);//写入前牌
	int mvDspCalcCarNumDis(const char *num1, const char *num2, int useSimilarChar); //模糊匹配 

#ifdef MV_LOCAL_DEBUG
public:
	void LogFile(vector<ObjMatchInfo> vecObj, const char *Path);
	void Match(vector<ObjMatchInfo> &vecA,vector<ObjMatchInfo> &vecB,const char *path, const char *unsure);
#endif

public:
	void mvInput(RECORD_PLATE_DSP_MATCH &plate);
	vector<MatchPair> mvOutput();	
	
	//输出互斥
	pthread_mutex_t m_OutMutex;
	pthread_mutex_t m_MapMutex;
	unsigned int m_uUpdateFlag;//更新记录状态标记 0:未更新 1:更新m_vecA 3:更新m_vecB 

	void AddPlateNoMatch(RECORD_PLATE_DSP_MATCH &plate);
	bool PopPlateNoMatch(RECORD_PLATE_DSP_MATCH &plate);
	pthread_mutex_t m_OutMutex2;
};
extern MvFBMatch2 g_MvFBMatch2;
#endif