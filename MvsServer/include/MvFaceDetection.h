#ifndef MV_FACE_DETECTION_H
#define MV_FACE_DETECTION_H

#include <cv.h>
#include <highgui.h>
#include <vector>

using namespace std;

class MvFaceDetectionImpl;

//人脸检测参数
typedef struct stFaceDetectionParam
{
	int		m_nMinFaceSize;
	int		m_nMaxFaceSize;

	string  m_strFFaceDetectorModelFileName;	//正面
	string	m_strLPFaceDetectorModelFileName;	//左侧
	string	m_strRPFaceDetectorModelFileName;	//右侧
	string	m_strLRFaceDetectorModelFileName;	//左旋
	string	m_strRRFaceDetectorModelFileName;	//右旋
} mv_stFDParam;

//人脸轨迹
typedef struct stFaceTrackNode	//人脸轨迹节点
{
	int64		m_nTime;		//节点的人脸图像在视频中的时间(ms)；
	int64		m_nFrameNo;		//节点的人脸图像在视频中的帧号；
	IplImage*	m_pFaceImage;	//每个节点的人脸图像，为了后续的对齐处理，这些图像比检测器输出的图像略大
	CvRect		m_faceRect;		//每个节点的人脸在图像中的位置

	stFaceTrackNode() 
	{
		m_nTime		= -1;
		m_nFrameNo	= -1;
		m_pFaceImage= NULL;
	}
} mv_stFaceTrackNode;

typedef struct stFaceTrack		//人脸轨迹
{
	IplImage*			m_pRepImage;			//每个轨迹一张全图
	vector<CvRect>		m_vRepImageRects;		//跟踪时是轨迹人脸在全图中的位置，只有一个矩形；单帧检测时多个矩形位置
	int64				m_nTime;				//轨迹全图对应时间(毫秒)

	vector<mv_stFaceTrackNode>	m_vTrack;		//轨迹上每个节点人脸图像及在对应的位置信息

	stFaceTrack()
	{
		m_pRepImage = NULL;
	}
} mv_stFaceTrack;

bool mv_ReleaseFaceTrack(mv_stFaceTrack& track);
bool mv_ReleaseFaceTracks(vector<mv_stFaceTrack>& vTracks);

class MvFaceDetection
{
public:
	MvFaceDetection();
	~MvFaceDetection();

public:
	//初始化
	bool mv_Init(mv_stFDParam& param);

	//释放
	bool mv_UnInit();

	//人脸检测接口
	bool mv_DetectFaces(IplImage* pImage, int64 ts, int64 nFrameNo, vector<mv_stFaceTrack>& vFaceTracks, 
						bool bTrack=true, bool bLastFrame=false);
	
	//单帧人脸检测接口
	bool mv_DetectFaces(IplImage* pImage, vector<CvRect>& vFacePos);

private:
	MvFaceDetectionImpl*	m_pFaceDetectionImpl;
};

#endif