#include "MvFBMatch2.h"
#include <algorithm>
#include <highgui.h>
#include <cv.h>

#include "Common.h"
MvFBMatch2 g_MvFBMatch2;
MvFBMatch2::MvFBMatch2(void)
{
	pthread_mutex_init(&m_OutMutex,NULL);
	pthread_mutex_init(&m_MapMutex,NULL);
	pthread_mutex_init(&m_OutMutex2,NULL);
	m_uUpdateFlag = 0;
}

MvFBMatch2::~MvFBMatch2(void)
{
	pthread_mutex_destroy(&m_OutMutex);
	pthread_mutex_destroy(&m_MapMutex);
	pthread_mutex_destroy(&m_OutMutex2);
	m_ChanIDAndFPlateMap.clear();
}


//读取车牌
void MvFBMatch2::mvInput(RECORD_PLATE_DSP_MATCH &plate)
{	
#ifdef DEBUG_LIUYANG
	LogNormal("MvFBMatch2::mvInput uKey:%d,%d,%d,%d car:%s", \
		plate.uKeyArray[0], plate.uKeyArray[1], plate.uKeyArray[2], plate.uKeyArray[3], plate.dspRecord.chText);
#endif
	if(plate.dspRecord.chText[0] == '*')//无牌车,及目标不处理
	{
		return;
	}

	ObjMatchInfo tmp;
	tmp.nMatchTimes = 0;
	memcpy((void *)&tmp.plate, (void *)&plate, sizeof(RECORD_PLATE_DSP_MATCH ));

#ifdef MATCH_LIU_YANG_DEBUG
	LogNormal("id:%d, uSeq:%d uSeqId:%lld dir:%d %s vio:%d", \
		plate.dspRecord.uChannelID, plate.dspRecord.uSeq, plate.dspRecord.uSeqID, \
		plate.dspRecord.uDetectDirection, plate.dspRecord.chText, plate.dspRecord.uViolationType);
#endif
	
	//判断Log是否存在，不存在则创建
	if(access("./Log",0) != 0) //目录不存在
	{
		mkdir("./Log",S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
	}

	LogTrace("./Log/chanId.txt","%s,Dirid:%d,type=%d,%s, chanid=%d,seq=%d\n",
		plate.dspRecord.chText,
		plate.dspRecord.uDetectDirection,
		plate.dspRecord.uViolationType, 
		plate.dspRecord.chPlace,
		plate.dspRecord.uChannelID,plate.dspRecord.uSeq);

	if(1 == plate.dspRecord.uDetectDirection) //尾牌	
	{
		pthread_mutex_lock(&m_OutMutex);

		if(m_vecA.size() > MAX_MATCH_SIZE)
		{
			LogError("CleanCache m_vecA.size=%d\n",m_vecA.size());
			//CleanCache(m_vecA, m_uLastMatchTimeA);
		}

		m_vecA.push_back(tmp);
		pthread_mutex_unlock(&m_OutMutex);

		m_uUpdateFlag = 1;

		LogTrace("./Log/VecA.txt", "add %s,%d into m_vecA,size=%d,id=%d\n",
			plate.dspRecord.chText,plate.dspRecord.uViolationType,m_vecA.size(),plate.dspRecord.uChannelID);
	}
	else
	{
		m_uUpdateFlag = 3;	

		//核查m_ChanIDAndFPlateMap各个通道前牌数量<MAX_MATCH_SIZE
		WriteFPlateToMap(tmp);
		
		LogTrace("./Log/VecMapTmp.txt", "%d:add %s,%d before insert map,size=%d,id=%d\n",
			plate.dspRecord.uTime,plate.dspRecord.chText,plate.dspRecord.uViolationType,m_ChanIDAndFPlateMap.size(),plate.dspRecord.uChannelID);
	}
	
}

//高16位车道逻辑编号，低16位车道类型
//uVerRoadID = uVerRoadID<<16;
//statistic.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;
//进行匹配，长时间没有匹配的，强行输出
vector<MatchPair> MvFBMatch2::mvOutput()
{
	vector<MatchPair> vecSure;
	//LogNormal("[%s]:m_vecA.size = %d,map.size = %d\n",__FUNCTION__,m_vecA.size(),m_ChanIDAndFPlateMap.size());//8,1
	if(m_vecA.size() == 0 || m_ChanIDAndFPlateMap.size() == 0 || (0 == m_uUpdateFlag))
	{
		return vecSure;
	}

	//清理长期没有匹配的结果
	vector<ObjMatchInfo>::iterator it_A, it_B;
	int nMid = 60;		//存在一分钟差距	
	bool bFlag = false;

	for(it_A = m_vecA.begin(); it_A!=m_vecA.end();)
	{
		if ( it_A->nMatchTimes > 0 && it_A->bMatchSuccess)	//匹配过的尾牌
		{
			pthread_mutex_lock(&m_OutMutex);
			LogTrace("./Log/vecDelete.txt", "erase %s,%d  from vecA4444\n",it_A->plate.dspRecord.chText,it_A->plate.dspRecord.uViolationType);
			it_A = m_vecA.erase(it_A);	
			pthread_mutex_unlock(&m_OutMutex);
			continue;
		}

		if (m_ChanIDAndFPlateMap.size() > 0) //有前牌数据
		{
			map<int,vector<ObjMatchInfo> >::iterator iterMap = m_ChanIDAndFPlateMap.begin();
			
			for (;iterMap != m_ChanIDAndFPlateMap.end();++iterMap)
			{
				vector<ObjMatchInfo>::iterator iterVec = iterMap->second.begin();
			    bFlag = false;

				if (iterMap->second.size() > 0)
				{
					for (;iterVec != iterMap->second.end();++iterVec)
					{
						//计算时间差
						int nDiff = it_A->plate.dspRecord.uTime - iterVec->plate.dspRecord.uTime;
						if(abs(nDiff)>nMid*3) //大于3
						{
							continue;
						}

						int nDis = GetCarNumDis(it_A->plate.dspRecord.chText, iterVec->plate.dspRecord.chText); //去掉车牌汉字
						int nSimDis = 0;
						if (0 != nDis)
						{
							nSimDis = mvDspCalcCarNumDis(it_A->plate.dspRecord.chText, iterVec->plate.dspRecord.chText,1);
						}
						
						if((nDis == 0) || (6 == nSimDis))	//车牌号相同
						{

							int nDiff = it_A->plate.dspRecord.uTime - iterVec->plate.dspRecord.uTime;				

							if(abs(nDiff) > nMid*3) //超过3分钟
							{
								continue;
							}

							it_A->nMatchTimes = 1;				

							/*if(abs(nDiff) < nMid*2) //2分钟以内
							{
								m_vecDiff.push_back(nDiff);
							}

							if(m_vecDiff.size() > MAX_MATCH_SIZE)
							{
								vector<int>::iterator it = m_vecDiff.begin();
								m_vecDiff.erase(it);
							}*/			

							if(m_uLastMatchTimeA < iterVec->plate.dspRecord.uTime)
							{
								m_uLastMatchTimeA = it_A->plate.dspRecord.uTime;
							}
							/*if(m_uLastMatchTimeB < iterVec->plate.dspRecord.uTime)
							{
								m_uLastMatchTimeB = iterVec->plate.dspRecord.uTime;
							}*/

							if((abs(nDiff) <= nMid*3) )
							{
								iterVec->nMatchTimes = 1;

								MatchPair tmp;
								memcpy((void *)&tmp.A, &(it_A->plate),sizeof(RECORD_PLATE_DSP_MATCH));
								tmp.vecB.push_back(iterVec->plate);

								if(it_A->plate.dspRecord.uChannelID != iterVec->plate.dspRecord.uChannelID)
								{
									//LogNormal("A:seq: %d B:seq: %d car1:%s, car2:%s ", \
										tmp.A.dspRecord.uSeqID, iterVec->plate.dspRecord.uSeqID, tmp.A.dspRecord.chText, iterVec->plate.dspRecord.chText);
									LogTrace("./Log/vecSure.txt","A:seq: %d B:seq,%d A:%s, B:%s,%d\n",
										tmp.A.dspRecord.uSeqID, iterVec->plate.dspRecord.uSeqID, tmp.A.dspRecord.chText, 
										iterVec->plate.dspRecord.chText,iterVec->plate.dspRecord.uViolationType);
									it_A->bMatchSuccess = true;
									vecSure.push_back(tmp);

									if (!bFlag)
									{
										bFlag = true;
										break;
									}
									
								}				
								else
								{
									LogNormal("A: %d B: %d car1:%s, car2:%s chanA:%d chanB:%d SameChannel", \
										tmp.A.dspRecord.uSeqID, iterVec->plate.dspRecord.uSeqID, tmp.A.dspRecord.chText, iterVec->plate.dspRecord.chText,
										it_A->plate.dspRecord.uChannelID, iterVec->plate.dspRecord.uChannelID);
								}
							}
						}
						else
						{
							//AddPlateNoMatch(iterVec->plate);
							continue;
						}
					}
				}
				if (bFlag)
				{
					break;
				}
			}//End of for (;iterMap != m_ChanIDAndFPlateMap.end();++iterMap)
			
			if (iterMap == m_ChanIDAndFPlateMap.end()) //未找到前牌删除该尾牌
			{
				it_A->nNoMathTimes ++;
				if(it_A->nNoMathTimes >= 2)
				{
					LogTrace("./Log/vecDelete.txt", "erase %s,%d from Map111\n",it_A->plate.dspRecord.chText,it_A->plate.dspRecord.uViolationType);
					LogError("前后牌未匹配上强制输出尾牌[%s]",it_A->plate.dspRecord.chText);
					
					AddPlateNoMatch(it_A->plate);
					pthread_mutex_lock(&m_OutMutex);							
					it_A = m_vecA.erase(it_A);
					pthread_mutex_unlock(&m_OutMutex);
					continue;
				}
			}
			
		}
		++it_A;
   }	

	m_uUpdateFlag = 0;

	return vecSure;
}

//返回两个车牌之间的距离
int MvFBMatch2::GetCarNumDis(const char * str1, const char * str2)
{
		int nDis = 0;
		
		for(int i=1; i<7; i++)
		{
			if(str1[i] != str2[i])
			{
				nDis++;
			}
		}
//#ifdef MATCH_LIU_YANG_DEBUG
//		if(0 == nDis)
//		{
//			LogNormal("str1:%s str2:%s nDis:%d", str1, str2, nDis);
//		}		
//#endif
		return nDis;
}

int MvFBMatch2::GetSimiDis(const char * str1, const char * str2)
{
	int nDis = 0;	

	for(int i=1; i<7; i++)
	{
		if(str1[i] != str2[i])
		{
			if(str1[i] == '#' || str2[i] == '#'
				|| IsSimilar(str1[i], str2[i]))
			{

			}
			else
			{
				nDis++;
			}

		}
	}

	return nDis;

}

int MvFBMatch2::IsSimilar(const char c1, const char c2)
{
	int i;
	char similar[][2] = {{'E','F'},{'B','8'},{'Z','2'},{'0','D'},
	{'S','5'},{'T','7'},{'A','4'},{'0','C'},
	{'Y','V'},{'F','P'},{'C','G'},{'0','Q'},
	{'R','K'},{'N','M'},{'0','U'},{'0','6'},
	{'1','7'},{'5','6'},{'Y','T'},{'3','8'},
	{'V','W'},{'B','G'},{'B','H'}};

	for(i = 0; i < 23; i++)
	{
		if(similar[i][0] == c1 && similar[i][1] == c2 ||
			similar[i][0] == c2 && similar[i][1] == c1)
		{
			return 1;
		}
	}

	return 0;
}


//3分钟后仍未匹配，强制删除
void MvFBMatch2::CleanCache(vector<ObjMatchInfo> &vecCache, const unsigned int ts)
{
	if(vecCache.size() < MAX_MATCH_SIZE)
	{
		return;
	}

	vector<ObjMatchInfo>::iterator it;

	for(it=vecCache.begin(); it!=vecCache.end();)
	{
		if(vecCache.size() < MAX_MATCH_SIZE/2)
		{
			break;
		}

		if(it == vecCache.begin() && it->nMatchTimes > 0)
		{
			//LogNormal("CleanCache  1:%s id:%d", it->plate.dspRecord.chText, it->plate.dspRecord.uChannelID);
			LogTrace("./Log/vecDelete.txt", "%d:erase %s,%d from m_vec1111",
				it->plate.dspRecord.uTime,it->plate.dspRecord.chText,it->plate.dspRecord.uViolationType);
			it = vecCache.erase(it);
			
			continue;
		}

		if(vecCache.size() > MAX_MATCH_SIZE)
		{
			int nDiff = 0;
			if(ts > 0 && it->plate.dspRecord.uTime > 0)
			{
				nDiff = ts - it->plate.dspRecord.uTime;
			}

			if(it == vecCache.begin() && nDiff > 180)
			{
				//LogNormal("CleanCache  2:%s id:%d", it->plate.dspRecord.chText, it->plate.dspRecord.uChannelID);
				LogTrace("./Log/vecDelete.txt", "%d:erase %s,%d from m_vec22222\n",
				it->plate.dspRecord.uTime,it->plate.dspRecord.chText,it->plate.dspRecord.uViolationType);
				it = vecCache.erase(it);
				continue;
			}
			else if(it->nMatchTimes > 0 && (1 != it->plate.dspRecord.uDetectDirection))//删除比较过的尾牌
			{
				//LogNormal("CleanCache  3:%s id:%d", it->plate.dspRecord.chText, it->plate.dspRecord.uChannelID);
				LogTrace("./Log/vecDelete.txt", "%d:erase %s,%d from m_vec33333\n",
					it->plate.dspRecord.uTime,it->plate.dspRecord.chText,it->plate.dspRecord.uViolationType);
				it = vecCache.erase(it);
				continue;		
			}
		}

		it++;
	}
}

void MvFBMatch2::WriteFPlateToMap(ObjMatchInfo & ObjMat )
{
	pthread_mutex_lock(&m_MapMutex);
	VecMatchMap::iterator iterMap = m_ChanIDAndFPlateMap.find(ObjMat.plate.dspRecord.uChannelID);
	if (iterMap == m_ChanIDAndFPlateMap.end())
	{
		vector<ObjMatchInfo> vecTmp;
		vecTmp.push_back(ObjMat);
			m_ChanIDAndFPlateMap.insert(make_pair(ObjMat.plate.dspRecord.uChannelID,vecTmp));
			LogTrace("./Log/VecMap.txt", "add %s,%d into map111,id=%d,size=%d\n",
				ObjMat.plate.dspRecord.chText,ObjMat.plate.dspRecord.uViolationType,ObjMat.plate.dspRecord.uChannelID,m_ChanIDAndFPlateMap.size());
	}
	else
	{
		if (iterMap->second.size() >= (MAX_MATCH_SIZE )) //如果通道对应前牌数大于60,删除第一个元素并插入该前牌
		{
			vector<ObjMatchInfo>::iterator iterVec = iterMap->second.begin();

			LogTrace("./Log/vecDelete.txt", "overload %s,%d,%d from map\n",
				iterVec->plate.dspRecord.chText,iterVec->plate.dspRecord.uViolationType,iterMap->second.size());

			if ((iterVec->nMatchTimes == 0) )  //未匹配过且为前后牌方式时输出
			{
				LogError("前后牌未匹配上强制输出前牌[%s]",iterVec->plate.dspRecord.chText);
				AddPlateNoMatch(iterVec->plate);
			}
			iterMap->second.erase(iterVec);

		    iterMap->second.push_back(ObjMat);
			LogTrace("./Log/VecMap.txt", "add %s,%d into map333,size=%d,id=%d,mapsize:%d\n",
				ObjMat.plate.dspRecord.chText,ObjMat.plate.dspRecord.uViolationType,iterMap->second.size(),ObjMat.plate.dspRecord.uChannelID,m_ChanIDAndFPlateMap.size());
		}
		else
		{
			iterMap->second.push_back(ObjMat);
			LogTrace("./Log/VecMap.txt", "add %s,%d into map222,size=%d,id=%d,msize:%d\n",
				ObjMat.plate.dspRecord.chText,ObjMat.plate.dspRecord.uViolationType,iterMap->second.size(),ObjMat.plate.dspRecord.uChannelID,m_ChanIDAndFPlateMap.size());
		}
	}
	pthread_mutex_unlock(&m_MapMutex);
	/*LogTrace("./Log/mapsize.txt","size : %d\n",m_ChanIDAndFPlateMap.size());*/
//#ifdef DEBUG_LIUYANG
//	LogNormal("WriteFPlateToMap size : %d\n",m_ChanIDAndFPlateMap.size());
//#endif
}

void MvFBMatch2::AddPlateNoMatch(RECORD_PLATE_DSP_MATCH &plate)
{
	pthread_mutex_lock(&m_OutMutex2);

////#ifdef DEBUG_LIUYANG
//	LogNormal("AddPlateNoMatch id:%d, uKey:%lld, car:%s ", \
//		plate.dspRecord.uChannelID, plate.uKeyArray[0], plate.dspRecord.chText);
////#endif
	m_NoMatchList.push_back(plate);
	/*LogTrace("./Log/send.txt","plate=%s,type=%d,id:%d,loction=%s\n",
		plate.dspRecord.chText,plate.dspRecord.uViolationType,plate.dspRecord.uDetectDirection,plate.dspRecord.chPlace);*/
	//LogNormal("m_NoMatchList=%d\n",m_NoMatchList.size());
	pthread_mutex_unlock(&m_OutMutex2);
}

bool MvFBMatch2::PopPlateNoMatch(RECORD_PLATE_DSP_MATCH &plate)
{
	bool bRet = false;

	pthread_mutex_lock(&m_OutMutex2);
	if (m_NoMatchList.size() > 0)
	{
		vector<RECORD_PLATE_DSP_MATCH>::iterator iterVec = m_NoMatchList.begin();

		plate = *iterVec;

#ifdef DEBUG_LIUYANG
		LogNormal("PopPlateNoMatch id:%d, uKey:%lld, car:%s size:%d ", \
			plate.dspRecord.uChannelID, plate.uKeyArray[0], plate.dspRecord.chText, m_NoMatchList.size());
#endif
		iterVec = m_NoMatchList.erase(iterVec);
		bRet = true;
	}
	pthread_mutex_unlock(&m_OutMutex2);

	return bRet;
}

int MvFBMatch2::mvDspCalcCarNumDis(const char *num1, const char *num2, int useSimilarChar) 
{ 
	int i; 
	int nCount, nEqual, nSimilar; 

	nCount = 0; 
	nEqual = 0; 
	nSimilar = 0; 

	//for(i = 0; i < 7; i++) 
	for(i = 1; i < 7; i++)  //去汉字
	{ 
		if(num1[i] == num2[i]) 
		{ 
			nEqual++; 
			continue; 
		} 

		if(useSimilarChar && IsSimilar(num1[i], num2[i])) 
		{ 
			nSimilar++; 
		} 
	} 

	nCount = nEqual+nSimilar; 

	return nCount; 
} 