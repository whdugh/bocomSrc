/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
graph-based track clustering
*/
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

#include "libHeader.h"

#ifndef DISJOINT_SET
#define DISJOINT_SET

// disjoint-set forests using union-by-rank and path compression (sort of).

typedef struct _point4D{
	float minx, miny, maxx, maxy;
	_point4D()
	{
		minx=-1;
		miny=-1;
		maxx=-1;
		maxy=-1;
	}
} point4D;

typedef struct
{
	int rank;
	int p;		//连通域的索引号
	int size;	//连通域包含的顶点数
	int minx,miny,maxx,maxy;  //four corners of the area
	int minx45,miny45,maxx45,maxy45;  //45 degress rotated four corners of the area
//	int area;	//最小包围矩形的面积
} uni_elt;

class universe
{
public:
	universe(int elements, point4D *coors);		//elements为总顶点数
	~universe();

	int		find(int x);		//查找x顶点所属的连通域


	int    is_canJoin( int x, int y, const CvSize &szWHThresh );

	int    is_canJoin(int x, int y, IplImage *max_sizex, IplImage *max_sizey, 
						IplImage *max_sizex45, IplImage *max_sizey45, 
						IplImage *oriImage, float cam_height, float fExt=1.1f ); //判断顶点x和顶点y是否可合并
	void    join(int x, int y ) ;


	bool	join(int x, int y, IplImage *max_sizex, IplImage *max_sizey, 
				  IplImage *max_sizex45, IplImage *max_sizey45,
				  IplImage *oriImage,float cam_height, float fExt=1.1f );	//合并顶点x和顶点y

	int		size(int x) const { return elts[x].size; }
	int		num_sets() const { return num; }

private:
	uni_elt *elts;	//结构体元素指针
	int num;		//合并后剩余的区域数
};

#endif
