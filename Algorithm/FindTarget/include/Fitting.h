// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef FITTING_H
#define FITTING_H

class Fitting
{
public:

	// y = kx + b.拟合直线参数k，b。如果直线是垂直线。ver=true,直线方程为x=b
	static void FittingLine(double *x, double *y, int size, bool &ver, double &k, double &b);


	static void TestFittingLine();
};


#endif