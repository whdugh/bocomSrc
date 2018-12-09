#ifndef MV_TYPE_CLASSIFY__H__
#define MV_TYPE_CLASSIFY__H__

#include "MvTypeUtility.h"
#include "MvTypeDetector.h"

#include <cv.h>
#include <vector>

using namespace std;

class MvTypeDetector
{
	friend class MvVehicleClassifyDetail;

public:
	MvTypeDetector();
	~MvTypeDetector();

protected:
private:
	bool m_bDay;
	
	bool m_bIsForCarnum;

	bool m_bGangao;			//港澳车牌
	int m_nBrandColor;
	CvRect m_rtBrand;

	float m_fZXScale;
	int m_nMinBusWidth;		//

	vector<TypicalEdge> m_vecHLines;
	vector<TypicalEdge> m_vecVLines;

	//涉及的数据
	bool m_pbCheck[4];
	TypicalEdge m_tpEdge[4];
	int m_nWidth;

	int m_nVehicleType;

	int m_nStatus;

	int ClassifyYellowTailBrand();	//10200-10269
	int ClassifyOtherTailBrand();	//10270-10299
public:
	int mvGetType();
};

#endif