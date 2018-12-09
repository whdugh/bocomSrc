
#ifndef _MV_UTILITY_H_
#define _MV_UTILITY_H_

#include "declare.h"
#include "VideoShutter.h"

#define RESIZE_RECT(rect, fx, fy) \
	cvRect((rect.x) * (fx), (rect.y) * (fy), (rect.width) * (fx), (rect.height) * (fy));



int     mvSiftDist(unsigned char *p1, unsigned char *p2);
int     mvInRect(const CvPoint pt, const CvRect rt);
double GetAngleBetweenTwoVector(double vx1, double vy1, double vx2, double vy2);

bool   JudgelapInXDir(CvRect Src, CvRect Dst,float flapRio = 0.0);
void    GetInterseRect(CvRect r1, CvRect r2, CvRect &ret);
bool    GetUnionRect(CvRect r1, CvRect r2, CvRect &ret);


#endif