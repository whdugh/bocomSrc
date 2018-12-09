#ifndef MV_LIGHT_TWINKLING_PREVENTER
#define MV_LIGHT_TWINKLING_PREVENTER


#include <map>
#include <vector>
#include <string>
#include "cxcore.h"

// 防止红绿灯闪烁的类,对一个灯进行判断，是否亮。
class MvLightTwinklingPreventer
{
public:
	MvLightTwinklingPreventer(CvRect lightRgn);

	MvLightTwinklingPreventer(CvRect lightRgn, float fThreshold);

	bool Process(unsigned int uSeq, IplImage* img);

	bool GetNearestLightOnFrame(unsigned int src, unsigned int &ret, bool bBigOrSmall);


	CvRect GetRgn() const;

	void SetName(std::string name);

private:

	// 帧号，灯的状态。
	std::map<unsigned int, bool> m_mapLightStatus;

	// 灯区域灰度历史记录。
	std::vector<int> m_vecLightRgnGrayRecord;


	CvRect m_rgn;

	// 均值
	float m_fLighRgnGrayMean;

	// 标准差
	float m_fLightRgnGrayStdVar;

	// 
	float m_fThreshold;

private:
	std::string m_strName;

};

#endif