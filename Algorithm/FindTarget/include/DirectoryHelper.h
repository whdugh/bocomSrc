// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//



#if !defined(AFX_DIRECTORYHELPER_H__0DBAF89F_E552_49EC_AB1C_CE51DFBEA6FD__INCLUDED_)
#define AFX_DIRECTORYHELPER_H__0DBAF89F_E552_49EC_AB1C_CE51DFBEA6FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>

class DirectoryHelper  
{
public:
	static bool CreateDirAbsoluteLinux(std::string path);
	static bool CreateDirRelativeLinux(std::string path);
	DirectoryHelper();
	virtual ~DirectoryHelper();

};

#endif // !defined(AFX_DIRECTORYHELPER_H__0DBAF89F_E552_49EC_AB1C_CE51DFBEA6FD__INCLUDED_)
