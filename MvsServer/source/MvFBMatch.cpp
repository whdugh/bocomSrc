#include "MvFBMatch.h"
#include <algorithm>
#include <highgui.h>
#include <cv.h>


MvFBMatch::MvFBMatch(void)
{
	m_nAChnl = -1;
	m_nBChnl = -1;
}

MvFBMatch::~MvFBMatch(void)
{
}


//读取车牌
void MvFBMatch::mvInput(RECORD_PLATE_DSP_MATCH plate)
{
	//前牌不处理
	if(plate.dspRecord.uDirection == 0 && plate.dspRecord.chText[0] == '*')	//前牌
	{
		return;
	}

	if(m_nAChnl == -1)
	{
		m_nAChnl = plate.dspRecord.uChannelID;		//
	}

	if(m_nBChnl == -1 && plate.dspRecord.uChannelID != m_nAChnl)
	{
		m_nBChnl = plate.dspRecord.uChannelID;	
	}

	ObjMatchInfo tmp;
	tmp.nMatchTimes = 0;
	memcpy((void *)&tmp.plate, (void *)&plate, sizeof(RECORD_PLATE_DSP_MATCH ));

	if(plate.dspRecord.uChannelID == m_nAChnl)
	{
		m_vecA.push_back(tmp);
	}
	else
	{
		m_vecB.push_back(tmp);
	}

}

//高16位车道逻辑编号，低16位车道类型
//uVerRoadID = uVerRoadID<<16;
//statistic.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;

//进行匹配，长时间没有匹配的，强行输出
vector<MatchPair> MvFBMatch::mvOutput()
{
	vector<MatchPair> vecSure;
	vector<MatchPair> vecUnsure;
	
	//清理长期没有匹配的结果
	vector<ObjMatchInfo>::iterator it_A, it_B;

	int nMid = 60;		//存在一分钟差距
	

	for(it_A = m_vecA.begin(); it_A!=m_vecA.end(); it_A++)
	{
		//if(it_A->plate.dspRecord.chText[0] == '*')		//暂时不处理无牌车
		//{
		//	continue;
		//}
	
		if(it_A->nMatchTimes > 0)	//匹配过的，不再匹配
		{
			continue;
		}

		MatchPair tmp;

		for(it_B = m_vecB.begin(); it_B!= m_vecB.end(); it_B++)
		{
			if(it_A->plate.dspRecord.chText[0] == '*')
			{
				break;
			}

			if(it_B->plate.dspRecord.chText[0] == '*')
			{
				continue;
			}

			//计算时间差
			int nDiff = it_A->plate.dspRecord.uTime - it_B->plate.dspRecord.uTime;

			if(abs(nDiff)>600 && abs(nDiff)>nMid*10)
			{
				continue;
			}

			int nDis = GetCarNumDis(it_A->plate.dspRecord.chText, it_B->plate.dspRecord.chText);
			int nSimDis = GetSimiDis(it_A->plate.dspRecord.chText, it_B->plate.dspRecord.chText);

			if(nDis <= 2 || (nDis<=2 && nSimDis<=1))	//可能相似
			{
				int nDiff = it_A->plate.dspRecord.uTime - it_B->plate.dspRecord.uTime;

				if(nMid < 5)
				{
					if(abs(nDiff) > nMid*5 && nDis>1)
					{
						continue;
					}
				}
				else if(abs(nDiff) > nMid*5 && nDis>1)
				{
					continue;
				}

				if(nMid < 5)
				{
					if(abs(nDiff) > nMid*3 && nDis>2)
					{
						continue;
					}
				}
				else if(abs(nDiff) > nMid*3 && nDis>2)
				{
					continue;
				}

				if(abs(nDiff) > 3600 || abs(nDiff) > nMid*10)	//超过1个小时
				{
					break;
				}

				if(abs(nDiff) < nMid*2)
				{
					m_vecDiff.push_back(nDiff);
				}

				if(m_vecDiff.size() > 128)
				{
					vector<int>::iterator it = m_vecDiff.begin();
					m_vecDiff.erase(it);
				}

				if(nDis == 0)
				{
					it_A->nMatchTimes = 1;
					it_B->nMatchTimes = 1;
					tmp.vecB.clear();
				}
				else
				{
					it_A->nMatchTimes++;
					it_B->nMatchTimes++;
				}

				tmp.vecB.push_back(it_B->plate);				

				int nM = cvFloor(m_vecDiff.size()/2);
				if(nM > 0)
				{
					nth_element(m_vecDiff.begin(), m_vecDiff.begin()+nM, m_vecDiff.end());
					if(abs(m_vecDiff[nM]) >240)
					{
						nMid =abs(m_vecDiff[nM]);	
					}
				}

				
				if(m_uLastMatchTimeB < it_B->plate.dspRecord.uTime)
				{
					m_uLastMatchTimeB = it_B->plate.dspRecord.uTime;
				}

				if(nDis == 0)
				{
					break;
				}

			}
			else
			{
				continue;
			}
		}

		if(it_A->nMatchTimes > 0)
		{
			tmp.A = it_A->plate;
			vecSure.push_back(tmp);
		}

	}
	

	vector<MatchPair>::iterator itMatch;
	for(it_A = m_vecA.begin(); it_A!=m_vecA.end(); it_A++)
	{
		MatchPair tmp;
		tmp.A = it_A->plate;

		if(it_A->nMatchTimes <= 0)
		{
			//查找和A最近匹配的时间差
			if(vecSure.size() <= 0)
			{
				continue;
			}

			int64 nBegin = 0;
			int64 nEnd = 0;
			for(itMatch=vecSure.begin(); itMatch!=vecSure.end(); itMatch++)
			{			
			    if(itMatch->vecB.size()<=0)
				{
				   continue;
				}
				RECORD_PLATE_DSP_MATCH mA = itMatch->A;
				if(mA.dspRecord.uTime > nBegin && mA.dspRecord.uTime < it_A->plate.dspRecord.uTime)
				{
					nBegin = itMatch->vecB.front().dspRecord.uTime;
				}

				if( mA.dspRecord.uTime >= it_A->plate.dspRecord.uTime)
				{
					nEnd = itMatch->vecB.front().dspRecord.uTime;
					vector<RECORD_PLATE_DSP_MATCH>::iterator itV;
					for(itV=itMatch->vecB.begin(); itV!=itMatch->vecB.end(); itV++)
					{
						if(itV->dspRecord.uTime < nEnd)
						{
							nEnd = itV->dspRecord.uTime;
						}
					}

					break;
				}
			}

			if(nEnd == 0 || nBegin == 0)
			{
				continue;
			}

			//在B中查找附近的图片
			vector<ObjMatchInfo>::iterator it;
			for(it=m_vecB.begin(); it!=m_vecB.end(); it++)
			{
				if(it->plate.dspRecord.uTime <= (nBegin - 2))
				{
					continue;
				}

				if(it->plate.dspRecord.uTime >= (nEnd + 2))
				{
					continue;		//其实应该为break,但是这里时间还没有排序
				}

				if(it->nMatchTimes > 0)
				{
					continue;
				}

				int nDiff = it_A->plate.dspRecord.uTime - it->plate.dspRecord.uTime;

				if(abs(nDiff) > 6)
				{
					continue;
				}

				tmp.vecB.push_back(it->plate);

				
			}

			if(tmp.vecB.size() > 0)
			{
				if(m_uLastMatchTimeA < it_A->plate.dspRecord.uTime)
				{
					m_uLastMatchTimeA = it_A->plate.dspRecord.uTime;
				}

				memcpy((void *)&tmp.A, &(it_A->plate),sizeof(RECORD_PLATE_DSP_MATCH));

				vecUnsure.push_back(tmp);
			}


		}
		else
		{
			vecSure.push_back(tmp);
		}

	
	}

	CleanCache(m_vecA, m_uLastMatchTimeA);
	CleanCache(m_vecB, m_uLastMatchTimeB);

	return vecUnsure;
}


//返回两个车牌之间的距离
int MvFBMatch::GetCarNumDis(const char * str1, const char * str2)
{
		int nDis = 0;

		if(str1[0] != str2[0] || str1[1] != str2[1])
		{
			nDis++;
		}

		for(int i=2; i<8; i++)
		{
			if(str1[i] != str2[i])
			{
				nDis++;
			}
		}

		return nDis;

}

int MvFBMatch::GetSimiDis(const char * str1, const char * str2)
{
	int nDis = 0;

	if(str1[0] != str2[0] || str1[1] != str2[1])
	{
		nDis++;
	}

	for(int i=2; i<8; i++)
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

int MvFBMatch::IsSimilar(const char c1, const char c2)
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


//10分钟后仍未匹配，强制删除
void MvFBMatch::CleanCache(vector<ObjMatchInfo> &vecCache, const unsigned int ts)
{
	vector<ObjMatchInfo>::iterator it;

	for(it=vecCache.begin(); it!=vecCache.end();)
	{
		if(vecCache.size() < 32)
		{
			break;
		}

		if(it == vecCache.begin() && it->nMatchTimes > 0)
		{
			it = vecCache.erase(it);
			continue;
		}

		int nDiff = ts - it->plate.dspRecord.uTime;

		if(vecCache.size() > 512)
		{
			if(it == vecCache.begin() && nDiff > 1200)
			{
				it = vecCache.erase(it);
				continue;
			}
			else if(it->nMatchTimes > 0)
			{
				it = vecCache.erase(it);
				continue;		
			}
		}
		

		it++;
	}
}
