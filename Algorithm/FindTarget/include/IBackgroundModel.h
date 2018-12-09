// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef IBACKGROUNDMODEL_H
#define IBACKGROUNDMODEL_H

#include <cxcore.h>
#include <vector>

class IBackgroundModel
{
public:
    virtual ~IBackgroundModel();

	virtual  void Invalidate() = 0;
	virtual  bool IsInitialized() = 0;
	virtual  void Init(IplImage *img) = 0;
	virtual  void Update(IplImage *img, int count, const CvRect * ObjRect, bool updateInRect) = 0;
	
	virtual  void Update(IplImage *img, IplImage* mask) = 0;

	virtual  void GetBackground(IplImage **background) = 0;
	virtual  void GetForeGround(IplImage *frame, IplImage **foreground) = 0;
	//virtual  void RemoveShadows() = 0;

	//virtual  void SetROI(const CvRect &roiRect) = 0;
	virtual  void SetObjectRgn(const int count, const CvRect * ObjRect) = 0;
	
	virtual  bool IsReGenBackground() = 0;
	virtual  bool ReGenBackground(IplImage *img) = 0;
	virtual  void SetUpdateWeight(double weight) = 0;

	void Update(IplImage *img, const std::vector<CvRect> &rgns, bool updateInRect);

};

#endif
