/*
 *类的功能：在更新直线背景时记录帧号及直线，用于在其他地方提取直线时复用，以优化运算性能！
 */
#ifndef MV_LINE_STORAGE_H
#define MV_LINE_STORAGE_H

#include "MvLineSegment.h"


class MvLineStorage
{
public:
	MvLineStorage();
	~MvLineStorage();

public:
	void Update(unsigned int uFrm, const BLine *pAllLine, unsigned int lineNum);
	unsigned int GetCurrentFrm();
	bool GetLine(CvRect rectRoi, BLine **pRetLine, int &lineNum);

private:
	unsigned int m_uCurrentFrm;
	BLine *m_pLine;
	unsigned int m_uLineNum;

};

#endif