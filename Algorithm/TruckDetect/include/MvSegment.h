#pragma once

class opencv;

class CSegment
{

public:
	CSegment();
	~CSegment();

	static IplImage* createROI(IplImage* img, CvRect position, float r_col = 8, float r_row = 8, float r_height = 7);

	static CvRect resizeRect(IplImage* img, int default_x, int default_y, int default_width, int default_height);
};