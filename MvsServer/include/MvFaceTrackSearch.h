#ifndef MV_FACE_TRACK_SEARCH_H
#define MV_FACE_TRACK_SEARCH_H

#include <cv.h>
#include <vector>
#include <string>

using namespace std;

class MvFaceSearchImpl;

#ifndef MV_ST_FS_PARAM
#define MV_ST_FS_PARAM
//人脸搜索参数
typedef struct stFaceSearchParam
{
	string  m_strFaceDetectorFileName; //"./FaceDetector.xml"
	string	m_strASModelFileName; //"./asmmodel";
	string	m_strFeatModelFileName; //"./Gabor_model.txt"
} mv_stFSParam;
#endif

#ifndef MV_ST_FACE_FEATURE
#define MV_ST_FACE_FEATURE
//人脸特征
typedef struct stFaceFeature
{
	char* 	m_pFaceFeature;
	int 	m_nSize;

	stFaceFeature()
	{
		m_pFaceFeature= NULL;
		m_nSize= 0;
	}

} mv_stFaceFeature;

//人脸搜索结果
typedef struct stFaceMathRes
{
	float 	m_fDistance; //与参考图像的距离
	CvRect 	m_rect;     //在匹配图中的位置
	stFaceMathRes()
	{
		m_fDistance 	= -1.0f;
		m_rect			= cvRect(0,0,0,0);
	}
} mv_stFaceMathRes;
#endif

//人脸轨迹搜索接口
class MvFaceTrackSearch
{
public:
	MvFaceTrackSearch();
	~MvFaceTrackSearch();

	//初始化接口
	bool mv_Initial(mv_stFSParam& param);

	//释放
	bool mv_UnInitial();

	//人脸检测接口,人脸宽高不小于120x120
	bool mv_DetectFaces(IplImage* pImage, vector<CvRect>& facePos);

	//特征提取接口, bExt=true时会提取更多的特征，但速度会变慢
	//在视频特征提取时bExt=false,
	//在匹配特征提取时可选择bExt=true以提高匹配准确度
	//**vImages和vFacePos里面的图像和矩形是一一对应的，且这些数据必须来自于同一个轨迹的人脸序列**
	bool mv_ExtractFeatureFromTrackImages(vector<IplImage*>& vImages, vector<CvRect>& vFacePos,
										  mv_stFaceFeature& faceFeatures, vector<CvRect> vOriFacePos=vector<CvRect>(), 
										  bool bTrack=true, bool bExt=false);

	//人脸轨迹序列图像特征搜索接口
	bool mv_MatchTrackFeature(vector<mv_stFaceFeature>& targetFeatures, 
							  mv_stFaceFeature& queryFeature,
							  vector<mv_stFaceMathRes>& matchRes, bool bTrack=true);

private:
	MvFaceSearchImpl*	m_pFaceSearchImpl;	
};


#endif