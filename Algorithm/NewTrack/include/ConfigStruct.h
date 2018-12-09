#ifndef __CONFIG_STRUCT_H

#define __CONFIG_STRUCT_H

#include <string>
#include <list>
#include "structdef.h"
#include "libHeader.h"

using namespace std;

//--------------------车道绘制所用到的结构-------------------//
typedef struct _mvRgnStru {
	int nKind;   //方向
	int	nPoints; //total number of points
	CvPoint2D32f *pPoints; //各个点集的坐标 
	CvPoint2D32f center;   //方向中心点

	bool is_person_road;

	_mvRgnStru()
	{
		nKind=-1;
		nPoints=0;
		pPoints=NULL;
		is_person_road = false; //true;//
	}
}mvRgnStru; 

//区域链表
typedef std::list<mvRgnStru> RgnList;

typedef struct _VEHICLE_PARAM
{
	int			nIndex;			//车道的序号

	mvRgnStru	chanRgn;		//车道区域
	mvRgnStru	oriRgn;			//车道方向区域

	mvRgnStru*	stopRgn;		//停车监测区域
	int			stopRgnNum;
	mvRgnStru*	perRgn;			//行人监测区域
	int			perRgnNum;
	mvRgnStru*	dropRgn;		//丢弃物监测区域
	int			dropRgnNum;
	mvRgnStru*	skipRgn;		//忽略区域
	int			skipRgnNum;

	mvRgnStru*	bargeInRgn;		//闯入区域
	int			bargeInRgnNum;

	Line*	amountLine;			//流量监测线
	int		amountLineNum;

	Line*	refLine;			//参考线
	int		refLineNum;

	Line*	turnRoadLine;		//变道线
	int		turnRoadLineNum;

	Line*	yellowLine;			//黄线
	int		yellowLineNum;      //目前客户端将整个道路的黄线均压入到每个车道中

	Line*	leadStreamLine;		//导流线
	int		leadStreamLineNum;  //目前客户端将整个道路的导流线均压入到每个车道中

	Line*	beyondMarkLine;		//越界线
	int		beyondMarkLineNum;

	float*	homography_src;		//标定矩阵
	float*	homography_dst;
	float	cam_height;

	char	strChanName[32];	//车道的名称
	bool	is_person_channel;	//非机动车道

	RgnList::iterator it_road;

	_VEHICLE_PARAM()
	{
		stopRgnNum = 0;
		perRgnNum = 0;
		dropRgnNum = 0;
		skipRgnNum = 0;
		amountLineNum = 0;
		refLineNum = 0;
		turnRoadLineNum = 0;
		bargeInRgnNum = 0;
		beyondMarkLineNum = 0;
		yellowLineNum = 0;
		leadStreamLineNum = 0;

		//指针也初始化为NULL，释放时可判断//qiansen
		stopRgn	= NULL;
		perRgn	= NULL;
		dropRgn	= NULL;
		skipRgn = NULL;
		amountLine	= NULL;
		refLine		= NULL;
		turnRoadLine= NULL;
		beyondMarkLine = NULL;
		bargeInRgn = NULL;
		beyondMarkLine = NULL;
		yellowLine = NULL;
		leadStreamLine = NULL;

		homography_src = NULL;
		homography_dst = NULL;		

		cam_height = 20;
		is_person_channel = false;//true;
	}
}VEHICLE_PARAM;

typedef struct _GLOBAL_SETING_PARAM
{
	mvRgnStru*	 stabRgn;//稳态区域
	int          stabRgnNum;
	_GLOBAL_SETING_PARAM( )
	{
		stabRgn = NULL;  
		stabRgnNum = 0;
	}
}GLOBAL_SETING_PARAM;


#endif  //#ifndef __CONFIG_STRUCT_H