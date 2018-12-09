#ifndef MVGEMLINE_H
#define MVGEMLINE_H

#include "MvLineSegment.h"
#include <vector>
using namespace std;

#ifndef MAX_OF_TWO
#define MAX_OF_TWO(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN_OF_TWO
#define MIN_OF_TWO(a,b)            (((a) < (b)) ? (a) : (b))
#endif


class MvGemLine
{
public:
	MvGemLine();
	~MvGemLine();

	static inline bool mvPoinInRec(CvPoint poin, CvRect r1)
	{
		if (poin.x > r1.x &&  poin.x < r1.x +r1.width &&
			poin.y > r1.y &&  poin.y < r1.y + r1.height)
		{
			return true;
		}
		else
		{
			return false;
		}

	}

	static int mvLinInRec(LSDLine Line, CvRect r1)
	{
		//mvPoinInRec(cvPoint(Line.x1,Line.y1),r1)  && mvPoinInRec(cvPoint(Line.x2,Line.y2),r1)
		int nx = (Line.x1+ Line.x2)/2;
		int ny = (Line.y1+ Line.y2)/2;
		if ( mvPoinInRec(cvPoint(nx,ny),r1)   )
		{
			return true;
		}
		else
		{
			return false;
		}


	}

	//对水平线段Y进行排序
	static void mvXorYSortLine(vector <LSDLine> &HorLine ,int nYsort);

	//合并水平线段
	static void mvSegmentUnite(vector <LSDLine> &lpBline, int nNoBgLineNum,bool bYewPlat = false);

	//快速排序
	static void mvquickSort(double *data,int *index, int l, int r, bool bSmallToBig = true);

	// 计算两个Rect的交集Rect             
	inline static bool    GetUnionRect(CvRect r1, CvRect r2, CvRect &ret)
	{
		int lx = MAX_OF_TWO(r1.x, r2.x);
		int rx = MIN_OF_TWO(r1.x+r1.width-1, r2.x+r2.width-1);
		int ty = MAX_OF_TWO(r1.y, r2.y);
		int by = MIN_OF_TWO(r1.y+r1.height-1, r2.y+r2.height-1);

		if (rx <= lx || ty >= by)
		{
			return false;
		}

		ret.x = lx;
		ret.y = ty;
		ret.width = rx-lx + 1;
		ret.height= by-ty + 1;
		return true;
	}

	inline static bool    lineLapShild(LSDLine line, CvRect r1, float fLapxtrd)
	{
		int lx = MIN_OF_TWO( MIN_OF_TWO(line.x1,line.x2),r1.x);
		int rx = MAX_OF_TWO(r1.x+r1.width-1,MAX_OF_TWO(line.x1,line.x2));
		
		float fLapLeng =  (r1.width + line.length) - (rx - lx );

		if ( fLapLeng/(r1.width +0.01) > fLapxtrd )
		{
			return true;
		}

	
		return false;
	}


	inline static bool    GetBoundRect(CvRect r1, CvRect r2, CvRect &ret)
	{
		int lx = MIN_OF_TWO(r1.x, r2.x);
		int rx = MAX_OF_TWO(r1.x+r1.width-1, r2.x+r2.width-1);
		int ty = MIN_OF_TWO(r1.y, r2.y);
		int by = MAX_OF_TWO(r1.y+r1.height-1, r2.y+r2.height-1);

		ret.x = lx;
		ret.y = ty;
		ret.width = rx-lx + 1;
		ret.height= by-ty + 1;
		return true;
	}

	static int mvRandom(const int range_min, const int range_max)
	{
		int randValue = static_cast<int>((double)rand() / ((double)RAND_MAX+1)*(range_max-range_min)+range_min);

		return randValue;
	}




	/* 限制rectangle越图像界, 它的高度与宽度可能会变小  */
	static inline void mvRestrictROIByImage(CvRect &rect, const IplImage *image)
	{
		rect.width = (rect.x<0) 
			? ((rect.width+rect.x>image->width) ? image->width-1 : rect.width+rect.x) 
			: ((rect.x+rect.width>image->width) ? (image->width-rect.x-1) : rect.width);
		rect.height = (rect.y<0) 
			? ((rect.height+rect.y>image->height) ? image->height-1 : rect.height+rect.y)
			: ((rect.y+rect.height>image->height) ? (image->height-rect.y-1) : rect.height);

		rect.x = rect.x > image->width-1 ? image->width-2 : (rect.x<0 ? 0 : rect.x);
		rect.y = rect.y > image->height-1 ? image->height-2 : (rect.y<0 ? 0 : rect.y);
		rect.width = rect.width <= 0 ? 1 : (rect.x+rect.width>image->width ? image->width-rect.x : rect.width);
		rect.height = rect.height <= 0 ? 1 : (rect.y+rect.height>image->height ? image->height-rect.y : rect.height);
	}


};
























#endif