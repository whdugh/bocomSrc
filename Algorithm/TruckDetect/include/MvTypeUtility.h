#include <vector>

using namespace std;

#ifndef MVTYPEUTILITY_H__
#define MVTYPEUTILITY_H__

typedef struct stTypicalEdge	//该结构仿照BusEdge构造，参数含义也类似
{
	int    a, b;	
	double x,y;
	double x1,y1;
	double x2,y2;
	int	   type;	//1太近，2只有孤立一行，3距离合适

	bool bBusBottomLine;  //是否大车底线
	bool bWinBottomLine;
	bool bWinTopLine;
	stTypicalEdge()
	{
		x = 0;
		y = 0;
		x1 = 0;
		x2 = 0;
		y1 = 0;
		y2 = 0;
		bBusBottomLine = false;
		bWinBottomLine = false;
		bWinTopLine = false;
	}
}TypicalEdge;

enum _CARNUM_COLOR
{
	_CARNUM_BLUE = 1,
	_CARNUM_BLACK,
	_CARNUM_YELLOW,
	_CARNUM_WHITE,
	_CARNUM_OTHER
};

namespace MvTypeUtility
{
	int mvfindLineSegmentIntersection (double x0, double y0,double x1, double y1,
		double x2, double y2,double x3, double y3,double* intersection);

	bool mvequals (double a, double b);
	bool mvequals (double a, double b, double limit);

	int StatLines(vector<TypicalEdge> vecLines, int nLeft,int nRight, int nTop, int nBottom, float fThresh=0);
}

#endif