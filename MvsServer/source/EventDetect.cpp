#include "EventDetect.h"

CEventDetect::CEventDetect()
{
  pthread_mutex_init(&m_muxCarNum, NULL);
  pthread_mutex_init(&m_muxPic, NULL);
}

CEventDetect::~CEventDetect()
{
   pthread_mutex_destroy(&m_muxCarNum);
   pthread_mutex_destroy(&m_muxPic);
}

void CEventDetect::AddCarNumInfo(MvCarNumInfo info)
{
   pthread_mutex_lock(&m_muxCarNum);
	m_vecCarNum.push_back(info);
   pthread_mutex_unlock(&m_muxCarNum);
}

void CEventDetect::PopCarNumInfo(vector<MvCarNumInfo> & carNums)
{
	pthread_mutex_lock(&m_muxCarNum);
	if (m_vecCarNum.size() > 0)
	{
		carNums.insert(carNums.begin(), m_vecCarNum.begin(), m_vecCarNum.end());
		m_vecCarNum.clear();
	}
	pthread_mutex_unlock(&m_muxCarNum);
}
//有车牌的图片数据存入缓冲区中
void CEventDetect::AddPic(TimePlate& tp, string& strPicData)
{
  pthread_mutex_lock(&m_muxPic);
  if (m_Piclist.size() > 5)
  {
	  m_Piclist.pop_front();
  }
  m_Piclist.push_back(make_pair(tp, strPicData));
  pthread_mutex_unlock(&m_muxPic);
}