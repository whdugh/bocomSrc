#ifndef BASIC_SSE
#define BASIC_SSE

#include <xmmintrin.h>
#include <emmintrin.h>

#ifndef MY_ALIGNED
	#define MY_ALIGNED

	#ifdef LINUX
		#define MY_DECL_ALIGNED(x) __attribute__ ((aligned (x)))
		#elif defined WIN32
			#define MY_DECL_ALIGNED(x) __declspec(align(x))
		#else
			#define MY_DECL_ALIGNED(x)
	#endif
#endif

class Basic_SSE
{
public:
	Basic_SSE(void);
	~Basic_SSE(void);
public:

static void FastExp_32f(float *dest, float *src, int len);
static void FastAtan2_32f(const float *Y, const float *X, float *angle, int len, bool angleInDegrees = false);


#ifndef PI
	#define PI 3.14159265358979323846
#endif

#ifndef PIx2
	#define PIx2 6.28318530717958647692
#endif

static MY_DECL_ALIGNED(16) int _pi32_0x7f[4];
static __m128 _ps_1;
static __m128 _ps_0p5;
static __m128 _ps_exp_hi;
static __m128 _ps_exp_lo;
static __m128 _ps_cephes_LOG2EF;
static __m128 _ps_cephes_exp_C1;
static __m128 _ps_cephes_exp_C2;
static __m128 _ps_cephes_exp_p0;
static __m128 _ps_cephes_exp_p1;
static __m128 _ps_cephes_exp_p2;
static __m128 _ps_cephes_exp_p3;
static __m128 _ps_cephes_exp_p4;
static __m128 _ps_cephes_exp_p5;
};


#endif