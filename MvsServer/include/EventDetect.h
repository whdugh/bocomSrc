/************************************************************************/
/*
 * abstract class for integrating algorithm about event detecting
 * author: John
 * date: 2011/4/13
 * you can define some arguments for event detecting only here
*/
/************************************************************************/
#ifndef _EVENT_DETECT_H_
#define _EVENT_DETECT_H_
#include "AbstractDetect.h"
#include <vector>
#include <list>
#include <pthread.h>

class CEventDetect : public CAbstractDetect
{
public:
	CEventDetect();
	void AddCarNumInfo(MvCarNumInfo info);
	void PopCarNumInfo(vector<MvCarNumInfo> & carNums);
	void AddPic(TimePlate& tp, string& strPicData);
	virtual ~CEventDetect();
protected:
	vector<MvCarNumInfo> m_vecCarNum;
	list< pair<TimePlate, string> > m_Piclist;//±£´æ³µÅÆÍ¼Æ¬
	pthread_mutex_t m_muxCarNum;
	pthread_mutex_t m_muxPic;
};
#endif
