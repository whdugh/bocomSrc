// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef MVFRONTWINDPOS_H 
#define MVFRONTWINDPOS_H

#include <vector>
#include <map>

#include "MvLineSegment.h"
using namespace std;





#ifndef MAX_OF_TWO
#define MAX_OF_TWO(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN_OF_TWO
#define MIN_OF_TWO(a,b)            (((a) < (b)) ? (a) : (b))
#endif


typedef struct _MVQUAD {
	uchar    rgbBlue;
	uchar    rgbGreen;
	uchar    rgbRed;
	uchar    rgbReserved;
} MvQuad;



typedef struct _MVHORLINE
{
	CvPoint  pStar;
	CvPoint  pEnd; //后期添的，前面有些地方未必计算其值
	uchar LinGrayVal;
	int nLeng;
	_MVHORLINE()
	{
		 nLeng = 0;
	}
}MvHorline;




class  MvFrontWindPos
{

public:

	MvFrontWindPos();

	~MvFrontWindPos(void);

     void mvDetcCarShieldByPlat( IplImage *oriColorImg, int colorType, CvRect CarPlate ,bool bYelPlate,vector<CvRect> &CarShield);


private:
	void mvShiledEdge(const IplImage *WinImg,CvRect Carplat,vector<MvHorline> &ObjShiledLine,vector<MvHorline>&LargerLine);

	int mvGetmainGray(const IplImage *oriColorImg, CvRect rect);

	void mvCombineHorEdge( vector<MvHorline> &SrcLin ,vector<MvHorline> &DstLine,CvRect CarPlat);

	void mvCombineHorEdge( vector<MvHorline> &SrcLin ,vector<MvHorline> &DstLine,int nXdis,int nYdis);
	void mvCombineLin( const vector<MvHorline> &SrcLin ,vector<MvHorline> &DstLine ,int nLengTrd[2],const IplImage*pWinImg);

	//判读前后相似性
	bool  mvFHSim(IplImage *oriColorImg,CvRect CarShield,bool bDeWindByLine,bool bLef );

	//求遮阳板边与边框距离
	 int mvShiledDisSide(const vector<LSDLine> &Slopeline,MvHorline ShildLine, bool bRigSide= true);
	 bool mvShiledNextWindEgde(const vector<LSDLine> &Slopeline,CvRect ShiledRec,bool bRig = true);
	 int  mvnSimHig(CvRect ShiledRec);


	//求最长线段
	bool mvGetLongLine( IplImage *ThdImg);

	//otsu阈值
	unsigned int mvgetotsuthershold( const IplImage *simg, const unsigned int nWidSample,const unsigned int nHigSample );

    //区域分析
	void AreaCharRect( const CvRect winRt, const vector<MvHorline> &SameLine,vector<CvRect>&CarShield,
		CvRect CarPlate, CvPoint PTrans);

	//获得水平和斜线--根据linseg得到的线段
	void mvGetHorSLopLine(vector <LSDLine> &HorLine,vector <LSDLine> &SlopeLine
		,const LSDLine *pLines, const int &nLines,bool *pLinesNeed,const IplImage * lineSegImg);

	void mvSetLinNeedState(const LSDLine *pLines,const int &nLines,bool *pLinesNeed,CvRect *Rec);


	bool mvJudgingBackWindowPosition(IplImage *lineSegImg, LSDLine *pLines, const int &nLineNum,bool *pLinesNeed ,CvRect &winPosRt,
		const CvRect *rectOri, const CvRect *rectMedian,bool bFiledLin =false,int nYStaTrd = 0) const;

	bool  mvWindowPosition(vector <LSDLine> &HorLine,vector <LSDLine> &SlopeLine, CvRect &winRt);

	void mvRGBToHSL(MvQuad *rgb, MvQuad *hsl); 

    //纠正车窗位置；
	void mvCorrecWindPos( CvRect &winRt,const  CvRect CarPlate ,const CvRect rectOri,
		                  const vector <LSDLine> &HorLine,const vector <LSDLine> &SlopeLine);
        
	//在线段组合无法检测的基础上，依据线段检测车窗位置；
	void mvGetWindPosByLine( CvRect &winRt, const  CvRect CarPlate ,const  CvRect  rectOri,vector <LSDLine> &HorLine,
		const vector <LSDLine> &SlopeLine,IplImage* grayLineImg,float frio,bool &bDarkFind);

	void  mvFilterShiled(vector<CvRect> &CarShield,const  CvRect CarPlate,const  CvRect rectOri,
		IplImage* grayLineImg,IplImage*oriColorImg, bool bDarkFind,CvPoint pointSrc,bool bIsGetWindowPos,
		CvRect winRt,const vector<LSDLine>&HorStorLine,const vector<LSDLine>&SlopeLine);

	void mvGetShieldPos(IplImage *oriGrayImg, CvRect cardrect,  CvRect &winRt ,CvPoint &PTrans,
		vector<CvRect> &CarShield,const vector <LSDLine> &HorLine,const vector <LSDLine> &SolpeLine,
		bool bBirght= true,bool FindWind = false);


	bool mvShildNearLine(const IplImage *grayLineImg, const vector <LSDLine> &HorLine,CvRect winRt,CvRect &ShildRec);

	void mvGetParaLine(const vector <LSDLine> &HorLine,int CarPlateWid,CvRect winRt,CvPoint &PTrans,vector<CvRect> &CarShield);
	bool mvRecognitionBackwindow(IplImage *lineSegImg, vector<LSDLine> &line_H, 
										vector<LSDLine> &line_L, vector<LSDLine> &line_R, CvRect &winRt) const;

	void mvGetBackWindowByOtherLines(IplImage *lineSegImg, vector<LSDLine> &line_H, vector<LSDLine> &line_L,
		                             vector<LSDLine> &line_R, vector<LSDLine> &threeWindowLines, 
		                             vector<CvRect> &threeVec, vector<CvRect> &threeeDisVec) const;
	
	void mvComputeLSDLinePara(LSDLine *pLine, const int &nLineNum) const;

	void mvFilterNotBackWindows(IplImage *lineSegImg, const vector<LSDLine> &windowLines, vector<CvRect> &rtOriVec, 
							vector<CvRect> &disVec, const int &DIS_THRESHOLD, vector<CvRect> &windowVec) const;

	bool mvGetLineUpDownRect(const IplImage *lineSegImg, const int &pos, const int &IMG_HEIGHT, vector<CvRect> &rectTwo) const;

	void mvFindWindowByPoRelation(const IplImage *lineSegImg, const vector<CvRect> &rtOriVec, vector<CvRect> &windowVec) const;
	
	inline void mvWindowCreate(IplImage *lineSegImg,LSDLine *horiLine, LSDLine *horiLineNext,LSDLine *leftLine, LSDLine *rightLine, const int dis1,
		                     const int dis2, const int dis3, const int dis4,vector<CvRect> &windowRtVec, 
		                     vector<CvRect> &windowDisVec, vector<LSDLine> &windowLines) const;

	inline void mvOtherWindowCreate(LSDLine *horiLine, LSDLine *horiLineNext,LSDLine *leftLine, LSDLine *rightLine, const int dis1,
										const int dis2, const int dis3, const int dis4,vector<CvRect> &windowRtVec, 
											vector<CvRect> &windowDisVec, vector<LSDLine> &windowLines) const;

	 void mvGetMedianRectangle( IplImage *Img, CvRect cardrect, CvRect &rectOri, CvRect &rectMedian, float &scale_w,
		float &scale_H );

	IplImage * mvGetHLSImage(IplImage *srcImg, int colorMode) const;

	//提取sobel结果
	void mvExtractSobel( IplImage* grayImg, int nMod, 
		IplImage *pSImg, IplImage *pMaskImg=NULL );

	

private:
	int m_PlateResizedWidth;

	int m_PlateResizedHeight;

	float m_WindowHightLow;//车窗高度上限

	float m_WindowHightHigh;//车窗高度下限

	int m_ColorType; //颜色通道顺序
	IplImage *m_TesLineImg;
	vector<CvRect> m_SimRec;//区域相似的区域块；

    MvHorline m_WinLin;
	CvRect m_CarPlat;
	bool   m_bYelPlate;
	bool m_Dark;

};



#endif
