/*----------------------------------------------------------------------------

  LSD - Line Segment Detector on digital images

  Copyright 2007,2008,2009,2010 rafael grompone von gioi (grompone@gmail.com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.

  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** @file lsd.h
    @brief LSD module header
    @author rafael grompone von gioi (grompone@gmail.com)
 */
/*----------------------------------------------------------------------------*/
#ifndef LSD_HEADER
#define LSD_HEADER


/*----------------------------------------------------------------------------*/
/** @brief Chained list of coordinates.
 */
struct lsd_coorlist
{
  int x,y;
  struct lsd_coorlist * next;
};

/*----------------------------------------------------------------------------*/
/** @brief A point (or pixel).
 */
struct lsd_point {int x,y;};


/*----------------------------------------------------------------------------*/
/** @brief Rectangle points iterator.
 */
typedef struct
{
  double vx[4];
  double vy[4];
  double ys,ye;
  int x,y;
} lsd_rect_iter;


/*----------------------------------------------------------------------------*/
/*--------------------------- Rectangle structure ----------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** @brief Rectangle structure: line segment with width.
 */
struct lsd_rect
{
  double x1,y1,x2,y2;  /* first and second point of the line segment */
  double width;        /* rectangle width */
  double x,y;          /* center of the rectangle */
  double theta;        /* angle */
  double dx,dy;        /* vector with the line segment angle */
  double prec;         /* tolerance angle */
  double p;            /* probability of a point with angle within 'prec' */
};

/*----------------------------------------------------------------------------*/
/*----------------------- 'list of n-tuple' data type ------------------------*/
/*----------------------------------------------------------------------------*/
/** @brief 'list of n-tuple' data type

    The i component, of the n-tuple number j, of an n-tuple list 'ntl'
    is accessed with:

      ntl->values[ i + j * ntl->dim ]

    The dimension of the n-tuple (n) is:

      ntl->dim

    The number of number of n-tuples in the list is:

      ntl->size

    The maximum number of n-tuples that can be stored in the
    list with the allocated memory at a given time is given by:

      ntl->max_size
 */
typedef struct lsd_ntuple_list_s
{
  unsigned int size;
  unsigned int max_size;
  unsigned int dim;
  double * values;
} * lsd_ntuple_list;

/*----------------------------------------------------------------------------*/
/*----------------------------- Image Data Types -----------------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** @brief char image data type

    The pixel value at (x,y) is accessed by:

      image->data[ x + y * image->xsize ]

    with x and y integer.
 */


typedef struct lsd_image_char_s
{
  unsigned char * data;
  unsigned int xsize,ysize;
} * lsd_image_char;

/*----------------------------------------------------------------------------*/
/** @brief int image data type

    The pixel value at (x,y) is accessed by:

      image->data[ x + y * image->xsize ]

    with x and y integer.
 */

typedef struct lsd_image_int_s
{
  int * data;
  unsigned int xsize,ysize;
} * lsd_image_int;

/*----------------------------------------------------------------------------*/
/** @brief double image data type

    The pixel value at (x,y) is accessed by:

      image->data[ x + y * image->xsize ]

    with x and y integer.
 */
typedef struct lsd_image_double_s
{
  double * data;
  unsigned int xsize,ysize;
} * lsd_image_double;


class CLSD  
{
public:
	void free_image_double(lsd_image_double i);
	lsd_image_double new_image_double(unsigned int xsize, unsigned int ysize);
	void free_ntuple_list(lsd_ntuple_list in);

public:
	CLSD();
	virtual ~CLSD();

	/*----------------------------------------------------------------------------*/
	/*-------------------------- Line Segment Detector ---------------------------*/
	/*----------------------------------------------------------------------------*/

	/*----------------------------------------------------------------------------*/
	/* LSD Full Interface                                                         */
	/*----------------------------------------------------------------------------*/
	/** @brief LSD Full Interface

		@param image       Input image.

		@param scale       When different than 1.0, LSD will scale the image by
						   Gaussian filtering.
						   Example: is scale=0.8, the input image will be subsampled
						   to 80% of its size, and then the line segment detector
						   will be applied.
						   Suggested value: 0.8

		@param sigma_scale When scale!=1.0, the sigma of the Gaussian filter is:
						   sigma = sigma_scale / scale,   if scale <  1.0
						   sigma = sigma_scale,           if scale >= 1.0
						   Suggested value: 0.6

		@param quant       Bound to the quantization error on the gradient norm.
						   Example: if gray level is quantized to integer steps,
						   the gradient (computed by finite differences) error
						   due to quantization will be bounded by 2.0, as the
						   worst case is when the error are 1 and -1, that
						   gives an error of 2.0.
						   Suggested value: 2.0

		@param ang_th      Gradient angle tolerance in the region growing
						   algorithm, in degrees.
						   Suggested value: 22.5

		@param eps         Detection threshold, -log10(NFA).
						   The bigger, the more strict the detector is,
						   and will result in less detections.
						   (Note that the 'minus sign' makes that this
						   behavior is opposite to the one of NFA.)
						   The value -log10(NFA) is equivalent but more
						   intuitive than NFA:
						   - -1.0 corresponds to 10 mean false alarms
						   -  0.0 corresponds to 1 mean false alarm
						   -  1.0 corresponds to 0.1 mean false alarms
						   -  2.0 corresponds to 0.01 mean false alarms
						   .
						   Suggested value: 0.0

		@param density_th  Minimal proportion of region points in a rectangle.
						   Suggested value: 0.7

		@param n_bins      Number of bins used in the pseudo-ordering of gradient
						   modulus.
						   Suggested value: 1024

		@param max_grad    Gradient modulus in the highest bin. For example,
						   for images with integer gray levels in [0,255],
						   the maximum possible gradient value is 255.0.
						   Suggested value: 255.0

		@param region      Optional output: an int image where the pixels used
						   in some line support region are marked. Unused pixels
						   have the value '0' while the used ones have the
						   number of the line segment, numbered 1,2,3,...
						   If desired, a non NULL pointer to an image_int should
						   be used. The resulting image has the size of the image
						   used for the processing, that is, the size of the input
						   image scaled by the given factor 'scale'.
						   Suggested value: NULL

		@return            A 5-tuple list, where each 5-tuple corresponds to a
						   detected line segment. The five values are:
						   - x1,y1,x2,y2,width
						   .
						   for a line segment from (x1,y1) to (x2,y2) and
						   a width 'width'.
	 */
	lsd_ntuple_list LineSegmentDetection( lsd_image_double image, double scale,
									  double sigma_scale, double quant,
									  double ang_th, double eps, double density_th,
									  int n_bins, double max_grad,
									  lsd_image_int * region );

	/*----------------------------------------------------------------------------*/
	/* LSD Simple Interface with Scale                                            */
	/*----------------------------------------------------------------------------*/
	/** @brief LSD Simple Interface with Scale

		@param image Input image.

		@param scale When different than 1.0, LSD will scale the image by
					 Gaussian filtering.
					 Example: is scale=0.8, the input image will be subsampled
					 to 80% of its size, and then the line segment detector
					 will be applied.
					 Suggested value: 0.8

		@return a 5-tuple list of detected line segments.
	 */
	lsd_ntuple_list lsd_scale(lsd_image_double image, double scale);

	/*----------------------------------------------------------------------------*/
	/* LSD Simple Interface                                                       */
	/*----------------------------------------------------------------------------*/
	/** @brief LSD Simple Interface

		@param image Input image.

		@return a 5-tuple list of detected line segments.
	 */
	lsd_ntuple_list lsd(lsd_image_double image);

private:
	void error(char * msg);

	int double_equal(double a, double b);

	double dist(double x1, double y1, double x2, double y2);

	void enlarge_ntuple_list(lsd_ntuple_list n_tuple);

	void add_5tuple( lsd_ntuple_list out, double v1, double v2,
							double v3, double v4, double v5 );

	void gaussian_kernel(lsd_ntuple_list kernel, double sigma, double mean);

	lsd_image_double gaussian_sampler( lsd_image_double in, double scale,
										  double sigma_scale );

	lsd_image_double ll_angle( lsd_image_double in, double threshold,
						  struct lsd_coorlist ** list_p, void ** mem_p,
						  lsd_image_double * modgrad, unsigned int n_bins,
						  double max_grad );
	int isaligned( int x, int y, lsd_image_double angles, double theta,
						  double prec );

	double angle_diff(double a, double b);

	double angle_diff_signed(double a, double b);

	double log_gamma_lanczos(double x);

	double log_gamma_windschitl(double x);

	double nfa(int n, int k, double p, double logNT);

	void rect_copy(struct lsd_rect * in, struct lsd_rect * out);

	double inter_low(double x, double x1, double y1, double x2, double y2);

	double inter_hi(double x, double x1, double y1, double x2, double y2);

	void ri_del(lsd_rect_iter * iter);

	int ri_end(lsd_rect_iter * i);

	void ri_inc(lsd_rect_iter * i);

	lsd_rect_iter * ri_ini(struct lsd_rect * r);

	double rect_nfa(struct lsd_rect * rec, lsd_image_double angles, double logNT);

	double get_theta( struct lsd_point * reg, int reg_size, double x, double y,
					 lsd_image_double modgrad, double reg_angle, double prec );

	void region2rect( struct lsd_point * reg, int reg_size,
					 lsd_image_double modgrad, double reg_angle,
					 double prec, double p, struct lsd_rect * rec );

	void region_grow( int x, int y, lsd_image_double angles, struct lsd_point * reg,
					 int * reg_size, double * reg_angle, lsd_image_char used,
					 double prec );

	double rect_improve( struct lsd_rect * rec, lsd_image_double angles,
								double logNT, double eps );

	int reduce_region_radius( struct lsd_point * reg, int * reg_size,
							 lsd_image_double modgrad, double reg_angle,
							 double prec, double p, struct lsd_rect * rec,
							 lsd_image_char used, lsd_image_double angles,
							 double density_th, double logNT, double eps );
	int refine( struct lsd_point * reg, int * reg_size, lsd_image_double modgrad,
			   double reg_angle, double prec, double p, struct lsd_rect * rec,
			   lsd_image_char used, lsd_image_double angles, double density_th,
			   double logNT, double eps );
	
private:
	void free_image_char(lsd_image_char i);
	lsd_image_char new_image_char(unsigned int xsize, unsigned int ysize);
	lsd_image_char new_image_char_ini( unsigned int xsize, unsigned int ysize,
								   unsigned char fill_value );

	void free_image_int(lsd_image_int i);
	lsd_image_int new_image_int(unsigned int xsize, unsigned int ysize);
	lsd_image_int new_image_int_ini( unsigned int xsize, unsigned int ysize,
								 int fill_value );

	lsd_image_double new_image_double_ini( unsigned int xsize, unsigned int ysize,
									   double fill_value );

	lsd_ntuple_list new_ntuple_list(unsigned int dim);
};






#endif /* !LSD_HEADER */
/*----------------------------------------------------------------------------*/
