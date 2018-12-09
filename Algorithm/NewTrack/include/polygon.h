// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


#include <stdio.h>
//typedef float real; // could be double.
#define C const
#define bigReal 1.E38  // FLT_MAX, DBL_MAX if double above
#include <limits.h>
#include <stdlib.h>


//#define _DEBUG_OUPOUT

//typedef int64_t int;
typedef int64_t hp;

//typedef long int;
//typedef long hp;

#ifndef EVENT_POINT
	#define EVENT_POINT
	typedef struct
	{
		float x; 
		float y;
	} point;
#endif


typedef struct {
	point min; 
	point max;
} box;

typedef struct {
	long x; 
	long y;
} ipoint;

typedef struct {
	long mn; 
	long mx;
} rng;

typedef struct {
	ipoint ip; 
	rng rx; 
	rng ry; 
	short in;
} vertex;

const float gamut = 50000000., mid = gamut/2.f;

void mvbd(float * X, float y) //min
{
	*X = *X<y ? *X:y;
}

void mvbu(float * X, float y)//max
{
	*X = *X>y ? *X:y;
}

void mvrange(box& B,point * x, int c)
{
	while(c--)
	{
		mvbd(&B.min.x, x[c].x); 
		mvbu(&B.max.x, x[c].x);
		mvbd(&B.min.y, x[c].y); 
		mvbu(&B.max.y, x[c].y);
	}
}

void mvfit(const box& B, float sclx, float scly,
		point * x, int cx, vertex * ix, int fudge)
{

	int c=cx; 
	while(c--)
	{
		ix[c].ip.x = (long)((x[c].x - B.min.x)*sclx - mid)&~7|fudge|c&1;
		ix[c].ip.y = (long)((x[c].y - B.min.y)*scly - mid)&~7|fudge;
	}


	ix[0].ip.y += cx&1;
	ix[cx] = ix[0];

	c=cx; 
	while(c--) 
	{
		/*          
		ix[c].rx = ix[c].ip.x < ix[c+1].ip.x ?
			(rng){ix[c].ip.x,ix[c+1].ip.x} : (rng){ix[c+1].ip.x,ix[c].ip.x};

		ix[c].ry = ix[c].ip.y < ix[c+1].ip.y ?
			(rng){ix[c].ip.y,ix[c+1].ip.y} : (rng){ix[c+1].ip.y,ix[c].ip.y};
		*/
		if (ix[c].ip.x < ix[c+1].ip.x)
		{
			ix[c].rx.mn = ix[c].ip.x;
			ix[c].rx.mx = ix[c+1].ip.x;
		}
		else
		{
			ix[c].rx.mn = ix[c+1].ip.x;
			ix[c].rx.mx = ix[c].ip.x;
		}

		if (ix[c].ip.y < ix[c+1].ip.y)
		{
			ix[c].ry.mn = ix[c].ip.y;
			ix[c].ry.mx = ix[c+1].ip.y;
		}
		else
		{
			ix[c].ry.mn = ix[c+1].ip.y;
			ix[c].ry.mx = ix[c].ip.y;
		}

		ix[c].in=0;
	}

}

hp mvarea(ipoint a, ipoint p, ipoint q)
{
	hp ar = (hp)p.x*q.y - (hp)p.y*q.x 
		     + (hp)a.x*(p.y - q.y) + (hp)a.y*(q.x - p.x);
#ifdef _DEBUG_OUPOUT
	printf("px=%d py=%d qx=%d qy=%d ax=%d ay=%d ar=%ld\n",
		   p.x,p.y,q.x,q.y,a.x,a.y,ar);
#endif
	return ar;
}

void mvcntrib(hp& s, ipoint f, ipoint t, short w)
{
	s += (hp)w*(t.x-f.x)*(t.y+f.y)/2;
#ifdef _DEBUG_OUPOUT
	printf("cntrib s=%d %d\n",s,&s);
#endif
}

int mvovl(rng p, rng q)
{
	return p.mn < q.mx && q.mn < p.mx;
}

void mvcross(hp& s, vertex * a, vertex * b, vertex * c, vertex * d,
		double a1, double a2, double a3, double a4)
{
	float r1=(float)(a1/((float)a1+a2)), r2 = (float)(a3/((float)a3+a4));
	/* 
	cntrib(s,(ipoint){a->ip.x + r1*(b->ip.x-a->ip.x), 
		a->ip.y + r1*(b->ip.y-a->ip.y)},
			b->ip, 1);

	cntrib(s,d->ip, (ipoint){
	c->ip.x + r2*(d->ip.x - c->ip.x), 
	c->ip.y + r2*(d->ip.y - c->ip.y)}, 1);
	*/


	ipoint ipt;
	ipt.x = a->ip.x + (long)(r1*(b->ip.x-a->ip.x));
	ipt.y = a->ip.y + (long)(r1*(b->ip.y-a->ip.y));
	mvcntrib(s,ipt,b->ip, 1);

	ipt.x = c->ip.x + (long)(r2*(d->ip.x - c->ip.x));
	ipt.y = c->ip.y + (long)(r2*(d->ip.y - c->ip.y));
	mvcntrib(s,d->ip, ipt, 1);

	++a->in; 
	--c->in;
}

void mvinness(hp& s, vertex * P, int cP, vertex * Q, int cQ)
{
#ifdef _DEBUG_OUPOUT
	printf("cP=%d cQ=%d\n",cP,cQ);
#endif
	int s2=0, c=cQ; 
//	s2=0;
//	int c=cQ; 
	//ipoint p = P[0].ip;
	ipoint p;
	p.x = P[0].ip.x;
	p.y = P[0].ip.y;

	while(c--)
	{
		if(Q[c].rx.mn < p.x && p.x < Q[c].rx.mx)
		{
			int sgn = 0 < mvarea(p, Q[c].ip, Q[c+1].ip);
			s2 += sgn != Q[c].ip.x < Q[c+1].ip.x ? 0 : (sgn?-1:1); 
#ifdef _DEBUG_OUPOUT
			printf("inness c=%d s2=%d\n",c,s2);
#endif
		}
	}
#ifdef _DEBUG_OUPOUT
	printf("inness s2=%d %d\n",s2,&s2);
#endif
	
	int j; 
	for(j=0; j<cP; ++j)
	{
		if(s2) 
			mvcntrib(s,P[j].ip, P[j+1].ip, s2);
		s2 += P[j].in;
	}
#ifdef _DEBUG_OUPOUT
	printf("inness s2=%d %d\n",s2,&s2);
#endif
}

float inter(point * a, int na, point * b, int nb)
{
	vertex *ipa = (vertex*)malloc(sizeof(vertex)*(na+1));
	vertex *ipb = (vertex*)malloc(sizeof(vertex)*(nb+1));

	box B = {{(float)bigReal, (float)bigReal},
	         {-(float)bigReal, -(float)bigReal}};
	double ascale;

#ifdef _DEBUG_OUPOUT
	printf("sizeof(hp)=%d\n",sizeof(hp));
#endif

	if(na < 3 || nb < 3) 
	{
		free(ipa);//qiansen
		free(ipb);
		return 0;
	}

	mvrange(B, a, na); 
	mvrange(B, b, nb);
#ifdef _DEBUG_OUPOUT
	printf("MIN(%.2f,%.2f), max (%.2f,%.2f)\n",
		    B.min.x,B.min.y,B.max.x,B.max.y);
#endif

	float rngx = B.max.x - B.min.x, sclx = gamut/rngx,
		rngy = B.max.y - B.min.y, scly = gamut/rngy;
#ifdef _DEBUG_OUPOUT
	printf("rngx = %.2f sclx = %.2f, rngy = %.2f scly = %.2f\n",
		    rngx,sclx,rngy,scly);
#endif

	mvfit(B, sclx, scly, a, na, ipa, 0); 
	mvfit(B, sclx, scly, b, nb, ipb, 2);

#ifdef _DEBUG_OUPOUT
	printf("sclx = %.2f, scly = %.2f\n",sclx,scly);
#endif
	ascale = sclx*scly;
#ifdef _DEBUG_OUPOUT
	printf("ascale = %.2f\n",ascale);
#endif

	hp s = 0; 
	int j, k;


	for(j=0; j<na; ++j) 
		for(k=0; k<nb; ++k)
		{
#ifdef _DEBUG_OUPOUT
			printf("j=%d,k=%d: %d<%d && %d<%d\n",
				    j,k,ipa[j].rx.mn,ipb[k].rx.mx,ipb[k].rx.mn,ipa[j].rx.mx);
#endif
			if(mvovl(ipa[j].rx, ipb[k].rx) && mvovl(ipa[j].ry, ipb[k].ry))
			{
#ifdef _DEBUG_OUPOUT
				printf("enter j=%d k=%d\n",j,k);
#endif
				hp a1 = -mvarea(ipa[j].ip, ipb[k].ip, ipb[k+1].ip),
				a2 = mvarea(ipa[j+1].ip, ipb[k].ip, ipb[k+1].ip);

				int o = a1<0; 
				if(o == a2<0)
				{
					hp a3 = mvarea(ipb[k].ip, ipa[j].ip, ipa[j+1].ip),
					a4 = -mvarea(ipb[k+1].ip, ipa[j].ip, ipa[j+1].ip);
#ifdef _DEBUG_OUPOUT
					printf("j=%d k=%d a3=%d a4=%d\n",j,k,a3,a4);
#endif
					if(a3<0 == a4<0) 
					{
						if(o) 
						{	
							mvcross(s,&ipa[j], &ipa[j+1], &ipb[k], &ipb[k+1], 
								(double)a1, (double)a2, (double)a3, (double)a4);
						}
						else 
						{	
							mvcross(s,&ipb[k], &ipb[k+1], &ipa[j], &ipa[j+1], 
								(double)a3, (double)a4, (double)a1, (double)a2);
						}	
					}
				}
			}
	}

	mvinness(s, ipa, na, ipb, nb);
#ifdef _DEBUG_OUPOUT
	printf("s=%d %d\n",s,&s);
#endif
	mvinness(s, ipb, nb, ipa, na);
#ifdef _DEBUG_OUPOUT
	printf("s=%d %d\n",s,&s);
#endif
	free(ipa);
	free(ipb);

	return (float)(s/ascale);
}
