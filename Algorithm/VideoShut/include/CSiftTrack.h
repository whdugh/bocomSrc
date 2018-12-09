#ifndef _MV_CSIFTRACK_H
#define _MV_CSIFTRACK_H

#include "declare.h"
#include "Mem_Alloc.h"
#include "Corner.h"
#include "MvSift.h"
//#include "MvTrack.h"
#include <vector>
#include "MvUtility.h"

using namespace std;

#define TRACKCOL

using namespace Shut_Corner;
namespace Shut_Track
{

	//记录角点信息
	typedef struct _MV_TRACK_POINT
	{
		int64 ts;
		short nStatus;  //0代表估计点
		CvPoint corner;

		_MV_TRACK_POINT()
		{
			ts = 0;
			nStatus = 0;
			corner = cvPoint(0,0);

		}
	} MvTrackPoint;

	//记录轨迹的信息
	typedef struct _MV_TRACK
	{
		int nTrackId;   //轨迹的ID

		short nEstTimes;  //轨迹中估计角点的数量
		CvPoint ptPredict;  //轨迹的预测角点
		unsigned char *feature;

		MvTrackPoint *pTrackPoint;
		short nTrackLen;
		bool  bShut;
	

#ifdef TRACKCOL
		CvScalar TrackCol;
		bool     bColSet;
#endif

		_MV_TRACK()
		{
			nTrackId = -1;
			nEstTimes = 0;
			feature = NULL;
			pTrackPoint = NULL;
			nTrackLen = 0;
			bShut    = false;
			ptPredict = cvPoint(0,0);
			

#ifdef TRACKCOL
			bColSet = false;;
#endif
		}

	} MvTrack;



	class CsiftTrack
	{

	public:
		void mvInit(CvSize ImgSize,int **pnMaxSize,int nSizW);
		void mvProcessTrack(int nTrackLenTresh );
		void mvGetConerData( MvCornerParm ConerPam);
		void mvSetTrackShutStatInRegi(CvRect RegionRec);
		void mvGetTrackIdInRegi(CvRect RegionRec,int &nTrackNum ,int &nTrackShutNum);
		void mvGetInRegiPoint(CvRect RegionRec,int nDisTrd,vector<CvPoint> &TrackPoint);
		void DrawTrack(IplImage *pImg,MvTrack *pTrack,CvScalar col);
		void DrawAllTrack(IplImage *pImg,CvScalar col);
		void mvUnInit();
		CsiftTrack();
		~CsiftTrack();

	private:
		

		void mvRemoveUnreasonableTracks();
		void   mvPredictTrack(MvTrack *pTrack,CvSize Size);
		CvRect mvFindMatchRect(MvTrack *pTrack);
		int    mvFindMatchTrackPoint(MvTrack *pTrack, CvRect rt, int *pCornerPos, int *pCornerTrackDis);


		bool    mvIsTrackReasonable(MvTrack *pTrack);
		int    mvGenTrackId(void);
		void mvMacthTrackOri();//轨迹寻找匹配点
		void mvUpdataTrackOri();//更新轨迹
		void mvSetTrackFil(int nTrackLenTresh);
		void mvInitMacPoi();
		void mvGenerateNewTrack();
		

	private:
		ushort **m_MaxSizeX;  //
		ushort **m_MaxSizeY;   //
		CvSize m_ImgSize;

		int m_nPointsCount;  //角点数量
		CvPoint *m_pCornPoints;  //角点坐标位置 
		uchar **m_pFeature;  //角点的sift描述符
		char  *m_pMatchStatus;  //角点的匹配状态



		int m_nTrackIndex;
		int *m_pCornerPos;
		int *m_pTrackPos;
		int *m_pCornerTrackDis;
		int m_nCornerTrackCount;  //角点和轨迹的匹配信息


		MvTrack *m_pTrackOri;
		int m_nTrackCntOri;  //过滤前轨迹的完整信息

		MvTrack *m_pTrackFlt;
		int m_nTrackCntFlt;  //过滤后的轨迹信息 



		//轨迹
		MvTrack *m_pTrack;
		int m_nTrackCnt;  //轨迹的完整信息

		int  m_GroupCnt;


		bool m_bInitSucs;

	};

};
#endif
