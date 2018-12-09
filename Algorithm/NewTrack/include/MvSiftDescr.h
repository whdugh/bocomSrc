#ifndef _MV_SIFT_DESCR_H_
	#define _MV_SIFT_DESCR_H_

	#ifndef  LINUX
		#include "opencv2/opencv.hpp"
		#include "opencv2/objdetect/objdetect.hpp" 
		#include "opencv2/features2d/features2d.hpp" 
		#include "opencv2/highgui/highgui.hpp" 
		#include "opencv2/calib3d/calib3d.hpp" 
		#include "opencv2/imgproc/imgproc_c.h" 
		#include "opencv2/imgproc/imgproc.hpp"   
		#include "opencv2/nonfree/features2d.hpp"
		#include "opencv2/legacy/legacy.hpp"
		using namespace cv; 
	#else
		#include "cv.h"
		#include "highgui.h"
		#include "cxcore.h"
	#endif

	#ifndef USE_MAG_CORRECT
		#define USE_MAG_CORRECT  //计算梯度幅值时，使用小幅值惩罚
	#endif

	#ifndef PIx2
		#define PIx2 6.2831853071
	#endif

	//determines the size of a single descriptor orientation histogram
	#ifndef DESCR_SCALE_FACTOR
		#define DESCR_SCALE_FACTOR 3.0
	#endif

	//threshold on magnitude of elements of descriptor vector 
	#ifndef DESCR_MAG_THRESH
		#define DESCR_MAG_THRESH 0.2
	#endif

	//factor used to convert floating-point descriptor to unsigned char
	#ifndef INT_DESCR_FACTOR
		#define INT_DESCR_FACTOR 512.0
	#endif

	#ifndef MY_SQUARE
		#define MY_SQUARE
		#define MV_SQUARE_SUM(a,b)  ((a)*(a)+(b)*(b)) 
	#endif


	//简单SIFT特征描述计算的结构体
	typedef struct StruSimSiftDescr
	{
	public:
		StruSimSiftDescr( )
		{
			m_bInit = false;
		}

		//初始化SIFT特征描述符计算的结构体
		bool mvInitStruSimSiftDescr( 
				int nWinRadius = 7,  //窗口的半径长度 8
				int nCellCnt = 4,	 //子单元的数目 4
				int nOriBin = 8		 //梯度方向的bin数 8
			);


		//释放SIFT特征描述符计算的结构体
		bool mvUninitStruSimSiftDescr( );


		//计算给定点的Sift描述符
		void mvComputePtsDescriptors(
				uchar **descriptor, IplImage* img,
				int nPtCnt, CvPoint2D32f *pPtLoc 
			);
		
		//求sift特征描述符的欧氏距离
		static int mvDistSquared( 
				unsigned char *pk1, unsigned char *pk2 );

		//进行简单的sift匹配
		static void mvSiftMatch(
				int n1, CvPoint2D32f *ptA1, uchar **desA1, 
				int n2, CvPoint2D32f *ptA2, uchar **desA2, 	  
				int &nPair, CvPoint ptAPair[],
				float fMatchHalfWinX = 20, //匹配窗口的一半 
				float fMatchHalfWinY = 20, //匹配窗口的一半 
				float fMinSecRate = 0.64f, //最小次小距离的比率
				int   nMinDistThres = 100000  //最小距离的阈值
			); 

	private:		
		bool  m_bInit;		//是否进行过初始化
		int	  m_nWinRadius; //窗口的半径长度  
		int   m_nCellCnt;	//子单元的数目    
		int	  m_nOriBin;	//梯度方向的bin数 
		float m_fBinsPerRad;  //一方向bin对应的弧度

		int   m_nWidth;		//窗口一行的点数

		float ***m_fHistos;   //用于sift计算的描述符
		float *m_fDescr;	  //最终的sift特征描述符
		float **m_pWinGaussWeight;  //窗口的gauss权重 
		float *m_pWinCellVal; //窗口中的点对应的cell值

		float **m_pSqrtTable;
		float **m_pTan2Table;

	private:
		//计算梯度的幅值和角点
		bool mvCalcGradMagOri(
				IplImage* img, //图像
				int r, int c,  //点的位置
				float* mag, float* ori, //幅值,方向
				bool bCheckBorder  //是否检查边界
			);

		//计算sift直方图
		void mvCalcDescrHistos( 
				IplImage* img,	  //图像
				int row, int col  //点的位置
			);

		//对hist进行插值
		void mvInterpHistEntry( float*** hist, 
				float rbin, float cbin, float obin, 
				float mag );


		//将直方图装换为描述符
		void mvHistosToDescr( uchar *uchar_descr );


		//归一化向量
		void mvNormalizeVector( float* vec, int n );

	}SimSiftDescr;



	//简单SIFT特征描述计算的结构体
	typedef struct StruSimpleSIFTDescr
	{
	public:
		void mvCalcGaussWight( int n, int radius, 
				float fAWeight[30][30] );		

		bool mvcalc_grad_mag_ori( IplImage* img, 
				int r, int c, float* mag, float* ori );

		void mvcalc_grad_mag_ori_simpled( IplImage* img,
				int r, int c, float* mag, float* ori );

		void interp_hist_entry( float*** hist, 
				float rbin, float cbin,	float obin,
				float mag, int d, int n );

		float*** mvdescr_histos_simpled( IplImage* img,
				int row, int col, int n, int bins, int radius,
				float fAWeight[30][30], float ***histos );

		void mvrelease_descr_histos( float**** histos, int n );

		void mvnormalize_vec( float* vec, int n );

		void mvhistos_to_descr( uchar *uchar_descr, 
				float*** histos, int n, int bins, 
				float* float_descr );	

		void mvcompute_descriptors( uchar **descriptor,
				IplImage* image, int num_keypts,
				CvPoint2D32f *corners, int n, int bins );

	}SimpleSIFTDesc;

#endif