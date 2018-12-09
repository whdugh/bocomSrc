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
#ifndef _FACTORY_H_
#define _FACTORY_H_

#include "global.h"
#include "AbstractCamera.h"
class AbstractCamera;
class Factory
{
	public:
		Factory();
		 ~Factory();
		AbstractCamera* Create(int nCameraType,int nPixelFormat = 0);
	protected:
        void Destory();
	private:
       AbstractCamera* m_pAbstractCamera;
};
#endif

