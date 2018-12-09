/************************************************************************/
/*
 * abstract class for integrating algorithm
 * author: John
 * date: 2011/4/13
*/
/************************************************************************/

#ifndef _ABSTRACTDETECT_H_
#define _ABSTRACTDETECT_H_
#include "structdef.h"

typedef struct _TIME_PLATE
{
   char chPlate[8];
   unsigned int uSec;
   unsigned int uMiSec;
   int nTop;
   int nLeft;
   int nWidth;
   int nHeight;
   _TIME_PLATE()
   {
	   memset(chPlate, 0, 8);
	   uMiSec = 0;
	   uSec = 0;
	   nTop = nHeight = nWidth = nLeft = 0;
   }
}TimePlate;

class CAbstractDetect
{
public:
	virtual ~CAbstractDetect();
	//初始化检测数据，算法配置文件初始化
	virtual bool Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst)=0;
	//释放检测数据，算法数据释放
	virtual bool UnInit()=0;
	//向车牌缓冲区中增加数据
	virtual void AddCarNumInfo(MvCarNumInfo info)=0;
	//向车牌事件缓冲区中增加图片
	virtual void AddPic(TimePlate& tp, string& strPicData)=0;
protected:
	CAbstractDetect();

};
#endif
