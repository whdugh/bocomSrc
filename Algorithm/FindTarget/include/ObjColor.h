// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


#ifndef OBJCOLOR_H
#define OBJCOLOR_H

#ifndef OBJECTCOLOR
#define OBJECTCOLOR
typedef struct OBJECT_COLOR
{
	int  nColor1;       //颜色1
	int  nColor2;	    //颜色2
	int  nColor3;       //颜色3
	
	int  nWeight1;    //颜色权重1
	int  nWeight2;    //颜色权重2
	int  nWeight3;    //颜色权重3
	
	OBJECT_COLOR()
	{
		nColor1 = 11;
		nColor2 = 11;
		nColor3 = 11;
		nWeight1 = 0;
		nWeight2 = 0;
		nWeight3 = 0;
	}
} object_color;
#endif

#endif