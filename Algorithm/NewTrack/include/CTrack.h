

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//           Class used for tracking objects for gray scaled images
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//---------------------------------------------------------------------------
#ifndef CTrackH
#define CTrackH
//---------------------------------------------------------------------------

#include <string>
#include <math.h>
#include <stdio.h>
//#include "cmyimage.h"
#include "CPixel.h"

#include "libHeader.h"

class CTrack
{
	// Declaration of the variables
 	private:
    	IplImage *FLIR_image;
		IplImage *FLIR_imageR, *FLIR_imageG, *FLIR_imageB;

        CPixel 	target_center;
        CPixel 	candidate_center;

		float 	constant_ch; //normalization constant

        //int		model_window_radius;
        //int		candidate_window_radius;

		CvPoint2D32f		model_window_radius;
        CvPoint2D32f		candidate_window_radius;

        int		number_of_bins;
        float	**FLIR_model_color_probabilities;
        float	**FLIR_candidate_color_probabilities;
        float	**FLIR_weights;

		bool    init_flag;

		//信息量定义
		CvRect   statepoint[2];
		int nFrameNumber;
		int nOri_Feature1, nOri_Feature2, nOri_Feature3;
	// Declaration of the functions
    public:
    	CTrack( int nWindow_radius, int nColor_bins, int nWidth, int nHeight, float nScaleWidth_Height );

    	~CTrack();
//	    void track_target(char*,char*,char*,char*,int,int,CPixel,int,int);

		CvPoint2D32f  mv_Calnextcenter_bymeanshift( IplImage *pBGRimage );

		bool    init_model( IplImage *pBGRimage, CvPoint2D32f centerpoint );
		
		CvPoint2D32f    get_model_window_radius( ){	 return model_window_radius;	}

    private:
        float 	kronecker_delta_function			(float,float);

        float	eponechnikov_profile				(float);
        float	derivative_of_eponechnikov_profile	(float);

        float	constant_C 							();		//for model
        float	constant_Ch			  				();		//for candidate

        void	model_density_function_q			();//for model
        void	candidate_density_function_p		();//for candidate

        float	bhattacharyya_coefficient();
        void	calculate_weights( );
        void	determine_new_candidate_location();

        float 	mv_track_target_in_consecutive_frames();

		int     mvCalFeaturePoints( IplImage* pColorImage, CvRect RoiRect ); //计算感兴趣区域内的特征点个数，返回第一类和第二类特征点数目		

		void    mvConvertImage( IplImage *pBGRimage );

		
};


#endif
