#ifndef _MV_LINE_H_
#define _MV_LINE_H_



#include "math.h"

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif


bool mvisBetween(double a, double b, double c);
bool mvequals (double a, double b, double limit);
bool mvequals (double a, double b);

bool mvisLineIntersectingLine(
							  double x0, double y0, double x1, double y1,
							  double x2, double y2, double x3, double y3);

int mvsameSide(double x0, double y0, double x1, double y1,
			   double px0, double py0, double px1, double py1);

int mvfindLineSegmentIntersection( 
								  double x0, double y0,  double x1, double y1,
								  double x2, double y2,  double x3, double y3, 
								  double* intersection );


#endif