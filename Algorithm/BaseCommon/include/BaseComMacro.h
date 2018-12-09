#ifndef __BASE_COM_MACRO_H
#define __BASE_COM_MACRO_H

	//----------------定义一些常用的宏-------------------//
	#ifndef MV_MAX
		#define MV_MAX(a,b)    (((a) > (b)) ? (a) : (b))
	#endif

	#ifndef MV_MIN
		#define MV_MIN(a,b)    (((a) < (b)) ? (a) : (b))
	#endif

	#ifndef MIN
		#define MIN(a,b)     (((a) < (b)) ? (a) : (b))
	#endif

	#ifndef MAX
		#define MAX(a,b)     (((a) > (b)) ? (a) : (b))
	#endif

	//-----------PI-----------//
	#ifndef PIx2
		#define PIx2 6.28318530717958647692
	#endif

	#ifndef PI
		#define PI 3.14159265358979323846
	#endif

	#ifndef HALF_PI
		#define HALF_PI 1.57079632679489661923
	#endif

	#ifndef PI_DIVIDE_2
		#define PI_DIVIDE_2  1.57079632679  //pi/2
	#endif

	#ifndef PI_DIVIDE_3
		#define PI_DIVIDE_3  1.047197551197 //pi/3
	#endif

	#ifndef PI_DIVIDE_4
		#define PI_DIVIDE_4  0.785398163397 //pi/4
	#endif

	#ifndef PI_DIVIDE_5
		#define PI_DIVIDE_5  0.628318530718 //pi/5
	#endif

	#ifndef PI_DIVIDE_6
		#define PI_DIVIDE_6  0.523598775598 //pi/6
	#endif

	#ifndef PI_DIVIDE_8
		#define PI_DIVIDE_8  0.392699075000 //pi/8
	#endif

	#ifndef PI_DIVIDE_9
		#define PI_DIVIDE_9  0.349065844444 //pi/9
	#endif

	#ifndef PI_DIVIDE_10
		#define PI_DIVIDE_10  0.314159265359 //pi/10
	#endif

	#ifndef PI_DIVIDE_12
		#define PI_DIVIDE_12  0.26179938799 //pi/12
	#endif

	#ifndef PI_DIVIDE_20
		#define PI_DIVIDE_20  0.157079632679 //*.05
	#endif

	#ifndef PI_DIVIDE_30
		#define PI_DIVIDE_30  0.104719755119 //6°
	#endif

	#ifndef PI15
		#define PI15 0.471238898025 //*.15
	#endif

	#ifndef ONEDIVPI
		#define ONEDIVPI 0.318309886  //1/PI
	#endif

	#ifndef PI_ONE_DEGREE
		#define  PI_ONE_DEGREE  0.0174533
	#endif

#endif