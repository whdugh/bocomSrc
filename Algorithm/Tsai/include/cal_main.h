/*******************************************************************************\
*										*
* This file contains operating constants, data structure definitions and I/O	*
* variable declarations for the calibration routines contained in the file	*
* cal_main.c.						                        *
*                                                                               *
* An explanation of the basic algorithms and description of the variables	*
* can be found in "An Efficient and Accurate Camera Calibration Technique for	*
* 3D Machine Vision", Roger Y. Tsai, Proceedings of IEEE Conference on Computer	*
* Vision and Pattern Recognition, 1986, pages 364-374.  This work also appears	*
* in several other publications under Roger Tsai's name.			*
*                                                                               *
*                                                                               *
* Notation                                                                      *
* --------                                                                      *
*                                                                               *
* The camera's X axis runs along increasing column coordinates in the           *
* image/frame.  The Y axis runs along increasing row coordinates.               *
*                                                                               *
* pix == image/frame grabber picture element                                    *
* sel == camera sensor element                                                  *
*                                                                               *
*                                                                               *
* History                                                                       *
* -------                                                                       *
*                                                                               *
* 01-Apr-95  Reg Willson (rgwillson@mmm.com) at 3M St. Paul, MN                 *
*       Filename changes for DOS port.                                          *
*                                                                               *
* 04-Jun-94  Reg Willson (rgwillson@mmm.com) at 3M St. Paul, MN                 *
*       Added alternate macro definitions for less common math functions.       *
*                                                                               *
* 25-Mar-94  Torfi Thorhallsson (torfit@verk.hi.is) at the University of Iceland*
*       Added definitions of parameters controlling MINPACK's lmdif()           *
*       optimization routine.                                                   *
*                                                                               *
* 30-Nov-93  Reg Willson (rgw@cs.cmu.edu) at Carnegie-Mellon University         *
*       Added declaration for camera type.                                      *
*                                                                               *
* 07-Feb-93  Reg Willson (rgw@cs.cmu.edu) at Carnegie-Mellon University         *
*       Original implementation.                                                *
*                                                                               *
*										*
\*******************************************************************************/

#ifndef _CAL_MAIN_
#define _CAL_MAIN_

#include "cal_pub.h"
#include "matrix.h"
#include "f2c.h"

/*extern int lmdif_( int (*fcn) (integer*,  integer*,
doublereal *, doublereal *, int),
integer *m, integer *n,
doublereal *x, doublereal *fvec, doublereal *ftol, doublereal *xtol, doublereal *gtol,
integer *maxfev,
doublereal *epsfcn, doublereal *diag,
integer *mode,
doublereal *factor,
integer *nprint, integer *info, integer *nfev,
doublereal *fjac,
integer *ldfjac, integer *ipvt,
doublereal *qtf, doublereal *wa1, doublereal *wa2, doublereal *wa3, doublereal *wa4);
*/
class cal
{
public:
//	cal();
	~cal(){};

	void      test_load_cd_data ();
	void      print_error_stats (FILE     *fp);
	void      print_cp_cc_data (FILE     *fp);
	void      dump_cp_cc_data (FILE     *fp);
	void      load_cp_cc_data (FILE     *fp);
	void      dump_cd_data (FILE     *fp);
	void      load_cd_data (FILE     *fp);
	void      mv_load_cd_data (int count, float *ptImage, float *ptWorld);

/* Forward declarations for the calibration routines */
	void  initialize_photometrics_parms ();
	void  initialize_photometrics_parms_widthheight (int width, int height);
	void  initialize_general_imaging_mos5300_matrox_parms ();
	void  initialize_panasonic_gp_mf702_matrox_parms ();
	void  initialize_sony_xc77_matrox_parms ();
	void  initialize_sony_xc57_androx_parms ();

	void  coplanar_calibration ();
	void  coplanar_calibration_with_full_optimization ();

	void  noncoplanar_calibration ();
	void  noncoplanar_calibration_with_full_optimization ();

	void  coplanar_extrinsic_parameter_estimation ();
	void  noncoplanar_extrinsic_parameter_estimation ();

	void  world_coord_to_image_coord ( double    xw, double yw, double zw, 
				 double *Xf, double *Yf);
	void  image_coord_to_world_coord (double    Xfd, double Yfd, double zw,
				 double *xw, double *yw);
	void  world_coord_to_camera_coord (double    xw, double yw, double zw,
				 double *xc, double *yc, double *zc);
	void  camera_coord_to_world_coord (double xc, double yc, double zc,
				 double *xw, double *yw, double *zw);
	void  distorted_to_undistorted_sensor_coord (double    Xd, double Yd,
				 double *Xu, double *Yu);
	void  undistorted_to_distorted_sensor_coord (double    Xu, double Yu,
				 double *Xd, double *Yd);
	void  distorted_to_undistorted_image_coord (double    Xfd, double Yfd,
				 double *Xfu, double *Yfu);
	void  undistorted_to_distorted_image_coord (double    Xfu, double Yfu,
				 double *Xfd, double *Yfd);

	void  distorted_image_plane_error_stats (double  *mean, double *stddev, double *max, double *sse);
	void  undistorted_image_plane_error_stats (double   *mean, double *stddev, double *max, double *sse);
	void  object_space_error_stats (double   *mean, double *stddev, double *max, double *sse);
	void  normalized_calibration_error (double   *mean,  double *stddev);

	// the following two functions, get the calibration parameters cc, cp.
    // added by baozheng, 2009.12.02
	struct calibration_constants  get_cc();
	struct camera_parameters get_cp();
//	void get_cc( struct calibration_constants camparams );

	//below from CAL_UTIL
	void      print_cp_cc_data (FILE     *fp,
		struct camera_parameters *cp,
		struct calibration_constants *cc);
	void      dump_cp_cc_data (FILE     *fp,
		struct camera_parameters *cp,
		struct calibration_constants *cc);

	void      mv_load_cd_data (int count, float *ptImage, float *ptWorld,
							   struct calibration_data *cd);

	int      ncc_full_optimization_error (
		integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,		/* vector of error from data */
		integer  *flag);
	int      ncc_nic_optimization_error (
			integer  *m_ptr,		/* pointer to number of points to fit */
			integer  *n_ptr,		/* pointer to number of parameters */
			doublereal *params,		/* vector of parameters */
			doublereal *err,		/* vector of error from data */
			integer  *flag);

private:
	void      ncc_full_optimization ();
	void      ncc_three_parm_optimization ();
	void      ncc_compute_exact_f_and_Tz ();
	void      initialize_sony_xc75_matrox_parms ();
	void      initialize_xapshot_matrox_parms ();
	void      solve_RPY_transform ();
	void      apply_RPY_transform ();
	void      cc_compute_Xd_Yd_and_r_squared ();
	void      cc_compute_U ();
	void      ncc_compute_better_R ();
	void      cc_compute_Tx_and_Ty ();
	void      cc_compute_R ();
	void      ncc_nic_optimization ();
	void      cc_compute_approximate_f_and_Tz ();
	int      cc_compute_exact_f_and_Tz_error ( integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,
		integer* flag);		/* vector of error from data */
	void      cc_compute_exact_f_and_Tz ();
	void      cc_three_parm_optimization ();
	void      cc_remove_sensor_plane_distortion_from_Xd_and_Yd ();
	int      cc_five_parm_optimization_with_late_distortion_removal_error (
		integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,		/* vector of error from data */
		integer  *flag);
	void      cc_five_parm_optimization_with_late_distortion_removal ();
	int      cc_five_parm_optimization_with_early_distortion_removal_error (
		integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,		/* vector of error from data */
		integer  *flag);
	void      cc_five_parm_optimization_with_early_distortion_removal ();
	int      cc_nic_optimization_error (
		integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,		/* vector of error from data */
		integer  *flag);
	void      cc_nic_optimization ();
	int      cc_full_optimization_error (
		integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,		/* vector of error from data */
		integer  *flag);
	void      cc_full_optimization ();
	void      ncc_compute_Xd_Yd_and_r_squared ();
	void      ncc_compute_U ();
	void      ncc_compute_Tx_and_Ty ();
	void      ncc_compute_sx ();
	void      ncc_compute_R ();
	void      ncc_compute_approximate_f_and_Tz ();
	int      ncc_compute_exact_f_and_Tz_error (
		integer  *m_ptr,		/* pointer to number of points to fit */
		integer  *n_ptr,		/* pointer to number of parameters */
		doublereal *params,		/* vector of parameters */
		doublereal *err,
		integer* flag);		/* vector of error from data */

	int lmdif_(
			   /* Subroutine */ int (cal::*fcn) (integer*,  integer*,
		doublereal *, doublereal *, integer*),
		integer *m, integer *n,
		doublereal *x, doublereal *fvec, doublereal *ftol, doublereal *xtol, doublereal *gtol,
		integer *maxfev,
		doublereal *epsfcn, doublereal *diag,
		integer *mode,
		doublereal *factor,
		integer *nprint, integer *info, integer *nfev,
		doublereal *fjac,
		integer *ldfjac, integer *ipvt,
		doublereal *qtf, doublereal *wa1, doublereal *wa2, doublereal *wa3, doublereal *wa4);

	/* Subroutine */ int fdjac2_(/* Subroutine */ int (cal::*fcn) (integer*,  integer*,
		doublereal *, doublereal *, integer*),
		integer *m, integer *n,
		doublereal *x, doublereal *fvec, doublereal *fjac,
		integer *ldfjac, integer *iflag,
		doublereal *epsfcn, doublereal *wa);

/* Subroutine */ int lmpar_(integer *n,
		doublereal *r,
		integer *ldr, integer *ipvt,
		doublereal *diag, doublereal *qtb, doublereal *delta, doublereal *par,
		doublereal *x, doublereal *sdiag, doublereal *wa1, doublereal *wa2);

		int qrsolv_(integer *n,
		doublereal *r,
		integer *ldr, integer*ipvt,
		doublereal *diag, doublereal*qtb, doublereal*x, doublereal*sdiag, doublereal*wa);

		int qrfac_(integer *m, integer *n,
		doublereal *a,
		integer *lda,
		logical *pivot,
		integer *ipvt, integer *lipvt,
		doublereal *rdiag, doublereal *acnorm, doublereal *wa);

		doublereal enorm_(integer *n, doublereal *x);

		doublereal dpmpar_(integer *i);

	double    CBRT (double    x);
	double hypot(double myd1,double myd2);
public:
	int m_nErrorCode;
private:

/* External declarations for variables used by the subroutines for I/O */
struct camera_parameters cp;
struct calibration_data cd;
struct calibration_constants cc;
char   camera_type[256];

/* Local working storage */
double    Xd[MAX_POINTS],
          Yd[MAX_POINTS],
          r_squared[MAX_POINTS],
          U[7];
int dWarningCounter;

};



#endif

//#ifdef __cplusplus
//}
//#endif
