/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：MvLineSegment.h
* 摘要: 简要描述本文件的内容
* 版本: V1.1
* 作者: 王福健 
* 完成日期: 2009年8月21日
*/

#ifndef MVLINESEGMENT__H__
#define MVLINESEGMENT__H__

#include <ippi.h>
#include <ippcc.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>

#ifndef FALSE
#define	FALSE false
#endif

typedef struct LSDRect {
	/** Coordinates of endpoints. */
	float x1, y1, x2, y2;
	/** Length of line. */
	float length;
	/** Number of pixels in the line's support. */
	unsigned int num_pixels;
	/** Number of pixels who voted for line. */
	unsigned int num_votes;
	/** Slope in the x-direction. */
	float tangent_x;
	/** Slope in the y-direction. */
	float tangent_y;
	/** Midpoint in the x-direction. */
	float midpoint_x;
	/** Midpoint in the y-direction. */
	float midpoint_y;
	float orientation;	//为了和BLine兼容
	/** theta                                */
	float theta;        /* angle */
	float width;        /* rectangle width */
	//	float prec;         /* tolerance angle */
	//  float p;
	//double p;           /* probability of a point with angle within prec */
} LSDLine,LSDRect,BLine;

typedef struct
{
	float vx[4];
	float vy[4];
	float ys,ye;
	int x,y;
} rect_iter;

typedef struct Image16s {
	
	/** The actual pixel data of the image. */
	
	Ipp16s* pixels;
	
	/**
	
	  * Interval in pixels between consecutive rows. Multiply this by
	  
		* sizeof(Ipp16s) before using in ippi functions.
		
	*/
	
	int step;
	
	IppiSize roi;
	
} Image16s;



/**

  * Convenient way to group image parameters which tend to go together.
  
*/

typedef struct Image8u {
	
	/** The actual pixel data of the image. */
	
	Ipp8u* pixels;
	
	/**
	
	  * Interval in bytes between consecutive rows. This may be different
	  
		* than the image width because ippiMalloc adds padding for better
		
		  * alignment.
		  
	*/
	
	int step;
	
	/**
	
	  * Region of interest (image size).
	  
	*/
	
	IppiSize roi;
	
} Image8u;

typedef struct image_float_s
{
	Ipp32f *data;//float * data;
	int xsize,ysize;
} * image_float;

typedef struct ntuple_float_s
{
	int size;
	int max_size;
	int dim;
	float * values;
} * ntuple_float;

struct coorlist
{
	int x,y;
	struct coorlist * next;
};

struct LSDPoint 
{
	int x,y,local;
};

class MvLineSegment  
{
private:
	int reg_size2;                   /* size of region in reg2 */
	float * sum_l;                   /* weight sum on longitudinal direction */
	float * sum_w;                   /* weight sum on lateral direction */
	int sum_res ;                 /* resolution factor on weight sum */
	int sum_offset;                  /* offset to center of weight sum */

private:
	double p1evl(double x, double coef[], int N );
	double polevl( double x, double coef[], int N );
	float inter_low(float x, float x1, float y1, float x2, float y2);
	float inter_hi(float x, float x1, float y1, float x2, float y2);
	double __lgamma_r(double x, int* sgngam);
	double lgamma(double x);

	int isaligned(int x, int y, Ipp32f *ippAngles, float theta, float prec,int xsize);
	void ri_del(rect_iter * iter);
	int ri_end(rect_iter * i);
	void ri_inc(rect_iter * i);
	rect_iter * ri_ini(struct LSDRect * r);
	float nfa(int n, int k, float p, Ipp32f logNT);
	float rect_nfa(struct LSDRect * rec, Ipp32f *ippAngles, Ipp32f logNT,int xsize,int ysize,float p, float prec);
	void rect_copy(struct LSDRect * in, struct LSDRect * out);
	float rect_improve(struct LSDRect * rec, float theta, Ipp32f* ippAngles,
                    Ipp32f logNT, float eps,int xsize,int ysize,float p, float prec);
	float angle_diff(float a, float b);
	float get_theta( struct LSDPoint * reg, int reg_size, float x, float y,
		Ipp32f* modgrad, float reg_angle, float prec,
                 float * elongation );
	float region2rect(struct LSDPoint * reg, int reg_size, Ipp32f* modgrad,
                  float reg_angle, float prec, float p, struct LSDRect * rec);
	void region_grow( int x, int y, Ipp32f *ippAngles, struct LSDPoint * reg,
		int * reg_size, float * reg_angle, IplImage *imgused,float prec, int radius,
        Ipp32f*  modgrad, float p, int min_reg_size );
	Ipp32f* ll_angle( IplImage *in, float threshold,struct coorlist ** list_p, void ** mem_p,
        Ipp32f ** ippModgrad, int n_bins, int max_grad,int fx =0);
	image_float gaussian_sampler( IplImage *in, float scale, float sigma_scale);
	void gaussian_kernel( ntuple_float kernel, float sigma, float offset);
	void free_ntuple_float(ntuple_float in);
	ntuple_float new_ntuple_float(int dim);
	image_float new_image_float(int xsize,int ysize);
	void free_image_float(image_float i);
public:
	MvLineSegment();
	virtual ~MvLineSegment();

	void line_extraction(IplImage *pImage,CvRect rect,int nDirections, int min_gradient, float min_length,
						   LSDLine** line, int* num_lines);
	void burns_line_extraction(Ipp8u* pixels, int step, int width, int height,
		int nDirections, int min_gradient, float min_length,
						   LSDLine** line, int* num_lines,int nFx =0);

};

#endif 