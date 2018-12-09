/// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2010 上海博康智能信息技术有限公司
// Copyright 2008-2010 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary


#ifndef FASTHESSIAN_H
#define FASTHESSIAN_H

#include <cv.h>
#include <vector>
#include <algorithm>

//#ifdef min
//#undef min
//#endif
//
//#ifdef max
//#undef max
//#endif
#ifndef mv_max
#define mv_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef mv_min
#define mv_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

class ResponseLayer;
static const int OCTAVES = 5;
static const int INTERVALS = 4;
static const float THRES = 0.0004f;
static const int INIT_SAMPLE = 2;

class Ipoint; // Pre-declaration
typedef std::vector<Ipoint> IpVec;
typedef std::vector<std::pair<Ipoint, Ipoint> > IpPairVec;

//! Computes the sum of pixels within the rectangle specified by the top-left start
//! co-ordinate and size
inline float BoxIntegral(IplImage *img, int row, int col, int rows, int cols) 
{  
	float *data = (float *) img->imageData;
	int step = img->widthStep/sizeof(float);

	// The subtraction by one for row/col is because row/col is inclusive.
	int r1 = mv_min(row,          img->height) - 1;
	int c1 = mv_min(col,          img->width)  - 1;
	int r2 = mv_min(row + rows,   img->height) - 1;
	int c2 = mv_min(col + cols,   img->width)  - 1;

	float A(0.0f), B(0.0f), C(0.0f), D(0.0f);
	if (r1 >= 0 && c1 >= 0)
		A = data[r1 * step + c1];
	if (r1 >= 0 && c2 >= 0)
		B = data[r1 * step + c2];
	if (r2 >= 0 && c1 >= 0) 
		C = data[r2 * step + c1];
	if (r2 >= 0 && c2 >= 0) 
		D = data[r2 * step + c2];

	return mv_max(0.f, A - B - C + D);
}

class Ipoint {

public:

	//! Destructor
	~Ipoint() {};

	//! Constructor
	Ipoint() : orientation(0) {};

	//! Gets the distance in descriptor space between Ipoints
	float operator-(const Ipoint &rhs)
	{
		float sum=0.f;
		for(int i=0; i < 64; ++i)
			sum += (this->descriptor[i] - rhs.descriptor[i])*(this->descriptor[i] - rhs.descriptor[i]);
		return sqrt(sum);
	};

	//! Coordinates of the detected interest point
	float x, y;

	//! Detected scale
	float scale;

	//! Orientation measured anti-clockwise from +ve x-axis
	float orientation;

	//! Sign of laplacian for fast matching purposes
	int laplacian;

	//! Vector of descriptor components
	float descriptor[64];

	//! Placeholds for point motion (can be used for frame to frame motion analysis)
	float dx, dy;

	//! Used to store cluster index
	int clusterIndex;
};

class FastHessian {
  
  public:
   
    //! Constructor without image
    FastHessian(std::vector<Ipoint> &ipts, 
                const int octaves = OCTAVES, 
                const int intervals = INTERVALS, 
                const int init_sample = INIT_SAMPLE, 
                const float thres = THRES);

    //! Constructor with image
    FastHessian(IplImage *img, 
                std::vector<Ipoint> &ipts, 
                const int octaves = OCTAVES, 
                const int intervals = INTERVALS, 
                const int init_sample = INIT_SAMPLE, 
                const float thres = THRES);

    //! Destructor
    ~FastHessian();

    //! Save the parameters
    void saveParameters(const int octaves, 
                        const int intervals,
                        const int init_sample, 
                        const float thres);

    //! Set or re-set the integral image source
    void setIntImage(IplImage *img);

    //! Find the image features and write into vector of features
    void getIpoints();
    
  private:

    //---------------- Private Functions -----------------//

    //! Build map of DoH responses
    void buildResponseMap();

    //! Calculate DoH responses for supplied layer
    void buildResponseLayer(ResponseLayer *r);

    //! 3x3x3 Extrema test
    int isExtremum(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);    
    
    //! Interpolation functions - adapted from Lowe's SIFT implementation
    void interpolateExtremum(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);
    void interpolateStep(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b,
                          double* xi, double* xr, double* xc );
    CvMat* deriv3D(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);
    CvMat* hessian3D(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);

    //---------------- Private Variables -----------------//

    //! Pointer to the integral Image, and its attributes 
    IplImage *m_pImg;
    int m_nWidth, m_nHeight;

    //! Reference to vector of features passed from outside 
    std::vector<Ipoint> &ipts;

    //! Response stack of determinant of hessian values
    std::vector<ResponseLayer *> m_responseMap;

    //! Number of Octaves
    int octaves;

    //! Number of Intervals per octave
    int intervals;

    //! Initial sampling step for Ipoint detection
    int init_sample;

    //! Threshold value for blob resonses
    float thresh;
};


#endif