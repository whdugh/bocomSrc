// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//





#ifndef MY_SIFT1_H
#define MY_SIFT1_H

#include <map>
#include "sift_descr.h"
#include "MvCorner.h"



class MySIFT1: public MySIFT
{
public:
	MatchResult mvCheckForMatchResult(CvPoint2D32f &pt, unsigned char *key, MyDescriptor* klist,  CvPoint2D32f *points, int keypt_count, const CvRect &searchArea/*by dur*/);

	bool CheckForMatch(unsigned char key[128], unsigned char **keyList, int nKeyListSize, int &nMatchedIndex, bool bUseUniqueMatch = false);

	bool CheckForMatch(const MvCorner *pCor, const std::map<int, MvCorner*> &mapIndex2Corner, int &nMatchedIndex, bool bUseUniqueMatch = false);


	bool CheckForMatch(const MvCorner *pCor, const std::vector<MvCorner*> &vecCorners, int &nMatchedIndex, int &nDis,/*std::ofstream &ofs,*/ bool bUseUniqueMatch = false);


	bool CheckForMatch(const MvCorner *pCor, MvCorner** pCors, int nCount, int &nMatchedIndex, int &nDis, bool bUseUniqueMatch = false);

	MvCorner * CheckForMatch(const MvCorner *pCor, const std::vector<MvCorner*> &vecCorners);

	MvCorner *Speed_CheckForMatch(const MvCorner *pCor, const std::vector<MvCorner*> &vecCorners);

	int Speed_SurfDist(unsigned char *p1, unsigned char *p2);

};
#endif
