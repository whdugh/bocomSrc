#ifndef _MV_DIST_GRAPH_H_
#define _MV_DIST_GRAPH_H_

#include "libHeader.h"

//基于图论的分割
namespace MV_AN_DISTGRAPH
{
	//聚类所用的4D点(左上，右下)
	typedef struct MV_SEGM_POINT4D
	{
		int minx, miny;
		int maxx, maxy;
		MV_SEGM_POINT4D()
		{
			minx = -1;
			miny = -1;
			maxx = -1;
			maxy = -1;
		}
	}MvSegmPoint4D;

	//聚类所用的无向图的边
	typedef struct MV_SEGM_EDGE
	{			
		int	  a;
		int	  b;
		float fXDist2CarW;
		float fYDist2CarH;
		float fMoveUncons;
		float fPtDistUncons;
		float fDiffVectorVal;

		MV_SEGM_EDGE( )
		{
			fMoveUncons = 100.0f;
		}
	}MvSegmEdge;


	//聚类所用的无向图的边
	typedef struct MV_SEGM_DIFF
	{			
		int	  nIdx;
		float w;
		MV_SEGM_DIFF( )
		{			
			w = 1000.0f;		
		}
	}MvSegmDiff;

	typedef struct 
	{
		int rank;
		int p;
		int size;

		int minx;
		int miny;
		int maxx;
		int maxy;
	}myUni_elt;


	class CSegmUniverse 
	{
	public:
		CSegmUniverse( int elements );
		~CSegmUniverse( );

		CSegmUniverse( int elements, MvSegmPoint4D *coors );

		int find( int x );  

		void join( int x, int y );
		bool joinWithMaxsize( int x, int y, 
				IplImage *pSzX, IplImage *pSzY,
				float fExt ); 

		int size( int x ) const { return elts[x].size; };
		int num_sets( ) const { return num; };

	private:
		myUni_elt *elts;
		int num;
	};

}

#endif