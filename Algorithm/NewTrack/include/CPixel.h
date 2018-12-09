
#ifndef CPixelH
#define CPixelH

#include <math.h>

class CPixel
{
    public:
    	float y;
  		float x;
    public:
    	CPixel				( );
    	CPixel				(int _y,int _x);
    	float length		( );
    	float distance		(CPixel a);
    	float distance		(int _y,int _x);
    	CPixel& operator = 	(const CPixel &a);
    	CPixel  operator - 	(const CPixel &a);
    	CPixel  operator + 	(const CPixel &a);
    	CPixel  operator / 	(const float a);
};

#endif
