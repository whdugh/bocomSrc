// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

/********************************************************************
	created:	2010_10_8   10:28
	filename: 	e:\BocomProjects\find_target_lib\include\CornerBackgroundModel.h
	file path:	e:\BocomProjects\find_target_lib\include
	file base:	CornerBackgroundModel
	file ext:	h
	author:		Durong
	
	purpose:	角点背景模型，基本思想：统计过去一段时间内各个像素处角点出现的
	次数，次数多的地方很可能是背景图像变化剧烈的地方，这些地方的角点被认为是背景
	角点。
*********************************************************************/

#ifndef CORNER_BACKGOUND_MODEL_H
#define CORNER_BACKGOUND_MODEL_H

#include <vector>
#include <list>
//using namespace std;

#define LIST_SIZE 300

#define CORNER_TEST

template <class _Point>
class CornerBackgroundModel
{
public:

	CornerBackgroundModel(int nXRes, int nYRes);

	~CornerBackgroundModel();

	void Input(_Point* pPts, int nPtSize);

	void GetForegroundCorners(_Point *pPts, int &nPtSize,const std::vector<CvRect> &CarGroupPark) const;

	void UpdateBackground();
	

private:

	int    GetMaxValueInMap() const;


	int    m_nXRes;  //X、Y方向的图像大小
	int    m_nYRes;

	//
	int    m_nPointBufferMaxSize;

	int    m_nPointCount;
	
	// 输入buffer
	_Point *m_pPointBuffer;

	// 标记输入buffer可否用于更新背景模型。
	// 更新完之后，该值设为false。防止一组值被重复更新进模型。
	bool m_bPointBufferUpdateAble;

	// 标记每个图像像素位置出现较角点的次数。
	int    *m_pMap;

	int    m_nMapMaxValue;

	typedef std::vector<_Point> _Pts;
	//typedef typename vector<_Point>::iterator _PtsIt;

	std::list<_Pts> m_lstPoints;
};






template <class _Point>
CornerBackgroundModel<_Point>::CornerBackgroundModel(int nXRes, int nYRes)
{
	m_nXRes        = nXRes;
	m_nYRes        = nYRes;
	m_nPointBufferMaxSize = nYRes * nXRes;
	m_pPointBuffer = new _Point[m_nPointBufferMaxSize];
	m_nPointCount  = 0;
	m_pMap         = new int[nXRes * nYRes];
	m_nMapMaxValue = 0;
	m_bPointBufferUpdateAble = false;

	for (int i=0; i<nXRes*nYRes; i++)
	{
		m_pMap[i] = 0;
	}
}

template <class _Point>
void CornerBackgroundModel<_Point>::GetForegroundCorners(_Point *pPts, int &nPtSize,const std::vector<CvRect> &CarGroupPark) const
{
	nPtSize = 0;
	std::vector<CvRect>::const_iterator CarParkIter; //违章车辆停车区域
	for (int i=0; i< m_nPointCount; i++)
	{
		int x = m_pPointBuffer[i].x;
		int y = m_pPointBuffer[i].y;

		assert(y < m_nYRes && x < m_nXRes );

		bool InPark = false; //对停靠区域内的角点信息不进行过滤
		for (CarParkIter = CarGroupPark.begin();CarParkIter !=  CarGroupPark.end(); CarParkIter++)
		{
			if (x >= CarParkIter->x && x <= CarParkIter->x+CarParkIter->width 
				&& y >= CarParkIter->y && y <=CarParkIter->y + CarParkIter->height)
			{
				InPark = true;
				break;
			}
		}
		

		//如果该角点的历史位置上出现的次数》=0.2 * m_nMapMaxValue而且大于LIST_SIZE * 0.08 =24则进行过滤掉
		//m_nMapMaxValueda值表示当前m_pMap中出现次数最多的，在UpdateBackground获取
		if (m_pMap[y * m_nXRes + x] < 0.2 * m_nMapMaxValue || 
			m_pMap[y * m_nXRes + x] < LIST_SIZE * 0.08
			|| InPark)
		{
			pPts[nPtSize++] = m_pPointBuffer[i];
		}

	}
}

template <class _Point>
void CornerBackgroundModel<_Point>::Input(_Point* pPts, int nPtSize)
{
	for (int i=0; i<nPtSize; i++)
	{
		
		m_pPointBuffer[i] = pPts[i];
	}
	m_nPointCount = nPtSize;
	m_bPointBufferUpdateAble = true;
}


template <class _Point>

void CornerBackgroundModel<_Point>::UpdateBackground()
{
	int i;



	if (m_bPointBufferUpdateAble == false || m_nPointCount == 0)
	{	

		return;
	}
	//当链表的个数大于LIST_SIZE时，则进行对链表中第一个进行数据覆盖
	//同时重新更新map里面的数据！---只记录最邻近的LIST_SIZE个数据信息


	if (m_lstPoints.size() == LIST_SIZE)
	{
		_Pts pts = m_lstPoints.front();


		
		typename std::vector<_Point>::iterator it;
		for (it = pts.begin(); it!=pts.end(); it++)
		{
			int x = it->x;
			int y = it->y;

			assert(y < m_nYRes && x <m_nXRes);



			m_pMap[y * m_nXRes + x] -= 1;  
			assert(m_pMap[y * m_nXRes + x] >= 0);
		}



		m_lstPoints.pop_front();   


	}
	
	_Pts pts;



	//把当前更新得到角点的位置放入map，来统计在图像中在同一位置出现的次数
	for (i=0; i< m_nPointCount; i++)
	{
		int x = m_pPointBuffer[i].x;
		int y = m_pPointBuffer[i].y;
		m_pMap[y * m_nXRes + x] += 1;



		pts.push_back(m_pPointBuffer[i]);
	}	
	//把当前帧更新得到的背景角点组成的一组放入链条m_lstPoints中去
	m_lstPoints.push_back(pts);	




    //得到m_pMap中出现的次数（就是角点在图像同一点出现的次数）最多的一次
	m_nMapMaxValue = GetMaxValueInMap();


	

	
}


template <class _Point>
int CornerBackgroundModel<_Point>::GetMaxValueInMap() const
{
	int max = 0;
	for (int i=0; i<m_nXRes * m_nYRes; i++)
	{
		if (m_pMap[i] > max)
			max = m_pMap[i];
	}
	return max;
}


template <class _Point>
CornerBackgroundModel<_Point>::~CornerBackgroundModel()
{
	if (m_pPointBuffer != NULL)
	{
		delete [] m_pPointBuffer;
		m_pPointBuffer = NULL;
	}

	if (m_pMap)
	{
		delete [] m_pMap;
		m_pMap = NULL;
	}
}

#endif
