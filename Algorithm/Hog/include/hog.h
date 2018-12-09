/****************************************************************************************\
*            HOG (Histogram-of-Oriented-Gradients) Descriptor and Object Detector        *
\****************************************************************************************/
#ifndef __HOG_H
#define __HOG_H

#include <vector>
#include <memory>
//#include <algorithm>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#include "HoGMacro.h"

using namespace std;


class MvHOGDescriptor
{
public:
	int  m_nStaticHogCalNum;
    enum { L2Hys=0 };

#ifdef	DEBUG_HOG_CALGRADIENT
	bool    m_bGetFN;
	char    m_cGradFN[1024], m_cAngleFN[1024];	
#endif

	MvHOGDescriptor(  CvSize _winSize=cvSize(WIN_SIZE_X, WIN_SIZE_Y), 
					CvSize _blockSize=cvSize(BLOCK_SIZE, BLOCK_SIZE), 
					CvSize _blockStride=cvSize(BLOCK_STRIDEX, BLOCK_STRIDEY), 
					CvSize _cellSize=cvSize(CELL_SIZE, CELL_SIZE) )  : 
					m_nbins(9),
					m_derivAperture(1),
					m_winSigma(-1),
					m_histogramNormType(L2Hys),
					m_L2HysThreshold(0.2),
					m_gammaCorrection(true),
					m_bVailidHOG(true), 
					m_nFrameCount(0), 
					m_nImgCount(0)
    {
		m_winSize = _winSize;
		m_blockSize = _blockSize;
		m_blockStride = _blockStride;
		m_cellSize = _cellSize;

#ifdef	DEBUG_HOG_CALGRADIENT
		m_bGetFN  = false;
#endif

#if 1
		m_nNewBins = m_nbins + 1;
#else
		m_nNewBins = m_nbins;
#endif

	}


	void mvAddOneDirToHoGArc( ) 
	{								
		m_nNewBins = m_nbins + 1;
	};


    virtual ~MvHOGDescriptor() {}

    size_t mvGetDescriptorSize() const;
	size_t mvGetHogLineDescriptorSize() const;

    bool mvCheckDetectorSize(bool bUseLineHog=false) const;
    double mvGetWinSigma() const;

    virtual void mvSetSVMDetector( const vector<float>& _svmdetector, bool bUseLineHog=false );

     virtual void mvLoad(const string& filename, vector<float>& detector);
	 virtual void mvLoadHogSvmFile(const string& filename, vector<float>& detector );

    virtual void mvCompute(const IplImage* img, vector<float>& descriptors,
							 CvSize winStride=cvSize(0,0), CvSize padding=cvSize(0,0),
							 const vector<CvPoint>& locations=vector<CvPoint>()) const;
	virtual int  mvComputeHogFeature(const IplImage* img, float fADescriptors[], CvPoint  windowLtPt[],
                            	CvSize winStride=cvSize(0,0), CvSize padding=cvSize(0,0),
                             	const vector<CvPoint>& locations=vector<CvPoint>()) const;
	virtual void mvDetect_forGetSample( const IplImage* img,  vector<CvPoint>& detectWindowLtPts,
								vector<CvPoint>& foundLocations, 	double hitThreshold=0, 
								CvSize winStride=cvSize(0,0),	CvSize padding=cvSize(0,0),
								const vector<CvPoint>& searchLocations=vector<CvPoint>() ) const;
	virtual void mvDetect_withHog( const IplImage* img,  vector<CvPoint>& detectWindowLtPts, vector<float>& fScore,
								 CvSize winStride=cvSize(0,0), CvSize padding=cvSize(0,0),
								 const vector<CvPoint>& searchLocations=vector<CvPoint>() ) const;
    virtual void mvDetect(const IplImage* img, vector<CvPoint>& foundLocations,
					double hitThreshold=0, CvSize winStride=cvSize(0,0), CvSize padding=cvSize(0,0),
					const vector<CvPoint>& searchLocations=vector<CvPoint>()) const;

	 virtual void mvCalHogFeature( float fHitThreshold, const IplImage* img, vector<CvPoint> &vctPt, 
					vector< vector<float> > &vctMulWinHogVal, vector<float> &vctScore, 
					CvSize winStride=cvSize(0,0), CvSize padding=cvSize(0,0),
					const vector<CvPoint>& searchLocations=vector<CvPoint>()) const;

	virtual bool mvDetectMultiScale_forGetSample(const IplImage* img,						   //待检测的行人区域图像
					vector<CvRect>& foundLocations,  //检测到的行人rect区域
					double hitThreshold=0,					   //检测到的阈值
					CvSize winStride=cvSize(0,0),		   //窗口滑动大小
					CvSize padding=cvSize(0,0),            //留白大小
					double scale0=1.08					       //图像每次缩放的比率
					);	     
	virtual bool mvDetectMultiScale_withHogLine( CvPoint ltPtInWorkImg,                     //待检测图像在工作图中的左上角位置
					float      fScaleXWorkToDetectImg,     //工作图与检测图像的宽缩放比率
					float      fScaleYWorkToDetectImg,     //工作图与检测图像的高缩放比率
					int         nLongLineNum,        //较长线段的数目
					CvPoint linePt1A[ ],     //较长线段的端点1
					CvPoint linePt2A[ ],     //较长线段的端点2
					const IplImage* img,	 //待检测的行人区域图像
					vector<CvRect>& foundLocations,   //检测到的行人rect区域
					double hitThreshold,	 //检测到的阈值
					CvSize winStride,		 //窗口滑动大小
					CvSize padding,			 //留白大小
					double scale0,			 //图像每次缩放的比率			
					int groupThreshold		 //检测到的行人数目阈值
				);
    virtual void mvDetectMultiScale( 
					const IplImage* img, vector<CvRect>& foundLocations,
					  double hitThreshold=0, CvSize winStride=cvSize(0,0),
					  CvSize padding=cvSize(0,0), double scale=1.05,
					  int groupThreshold=2);
	virtual void mvDetectHumanWithMultiScale( const IplImage* img,							     //待检测的行人区域图像
					vector<CvRect>& vctFoundLocations, //检测到的行人rect区域
					bool    bASureIsPeople[3],               
					double hitThreshold,			   //检测到行人的阈值
					int		  nDetectdPeopleNumThres,  //检测到的行人数目阈值
					float    fMergerThres,          //各行人合并阈值
					CvSize winStride=cvSize(0,0),	//窗口滑动大小
					CvSize padding=cvSize(0,0),	 //留白大小
					double scale0=1.05		     //图像每次缩放的比率
				);

    virtual void mvComputeGradient(const IplImage* img, CvMat* grad, CvMat* angleOfs,
							 CvSize gradSize=cvSize(0,0),   CvSize paddingTL=cvSize(0,0), CvSize paddingBR=cvSize(0,0));
	virtual void mvComputeGradientOptimize(const IplImage* img, CvMat* grad, CvMat* angleOfs,
	  	                       CvSize gradSize=cvSize(0,0),   CvSize paddingTL=cvSize(0,0), CvSize paddingBR=cvSize(0,0));
	virtual void mvComputeGradientOptimizeNew( const IplImage* img, CvMat* grad, CvMat* angleOfs,
		             CvSize gradSize=cvSize(0,0),   CvSize paddingTL=cvSize(0,0), CvSize paddingBR=cvSize(0,0) );
	virtual void mvComputeGradientOptimizeRgbNew( const IplImage* img, CvMat* grad, CvMat* qangle, 
		             CvSize gradSize=cvSize(0,0),   CvSize paddingTL=cvSize(0,0), CvSize paddingBR=cvSize(0,0) ) ;

    static vector<float> mvGetDefaultPeopleDetector( void );

	int mvBorderInterpolate( int p, int len, int borderType );

	void mvGroupRectangles(vector<CvRect>& rectList, int groupThreshold, double eps);

	// 存图函数
	void mvSaveTestImage( char cTopDir[256], char cType[10], char *infoStr, long &nImgCount );

	//判断线与矩形相交
	static bool mvisPointInsideRectangle(double x0, double y0, double x1, double y1, double x, double y);
	
	static bool mvisBetween(double a, double b, double c);

	static int mvsameSide(double x0, double y0, double x1, double y1, double px0, double py0, double px1, double py1);

	static bool mvisLineIntersectingLine(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3);

	static bool mvisLineIntersectingRectangle(double lx0, double ly0, double lx1, double ly1, double x0, double y0, double x1, double y1);

	static void getWindowLineFeature( CvPoint winLtPt, CvPoint winRbPt, int nLongLineNum, CvPoint linePt1A[], CvPoint linePt2A[],
								     int nFeatureDim, float fAFeature[] );

    CvSize m_winSize;
    CvSize m_blockSize;
    CvSize m_blockStride;
    CvSize m_cellSize;
    int m_nbins;
	int m_nNewBins;
    int m_derivAperture;
    double m_winSigma;
    int m_histogramNormType;
    double m_L2HysThreshold;
    bool m_gammaCorrection;
    vector<float> m_svmDetector;
	vector<float> m_svmDetectorLevel[2];

    bool m_bVailidHOG;
	long m_nFrameCount;
	long m_nImgCount;

};
#endif
