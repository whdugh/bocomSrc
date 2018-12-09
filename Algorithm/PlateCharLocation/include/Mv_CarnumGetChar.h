#ifndef _MV_CARNUMGETCHAR_H_
#define _MV_CARNUMGETCHAR_H_

#include<vector>

using namespace std;
//////

#define JING_PLATE_DEBUG //警牌处理 
#define WHITE_PLATE_DEBUG //白牌处理

//#define JING_PLATE_DEBUG_DEBUG //警牌处理 
//#define WHITE_PLATE_DEBUG_DEBUG //白牌处理

#ifdef WHITE_PLATE_DEBUG_DEBUG
#define WHITE_PLATE_DEBUG_SHOWIMAGE
#endif

//#define SAMPLE161616

//#define SAVE_PLATE_SAMPLE

//#define TIME_COMPUTE
//#define TIME_COMPUTE_SIM
//#define TIME_COMPUTE_DR
//  

//#define MV_CARNUMGETCHAR_DEBUG
//#define MV_DR_CARNUMGETCHAR_DEBUG //双层黄牌调试开关

//#define TEST_YELLOWPLATE_JUDGE //黄色牌判断容易错误，将两头字符的相似性权重提高

//#define COLOREDGE_DEBUG

#ifdef MV_CARNUMGETCHAR_DEBUG
//#define CARNUMGETCHAR_DEBUG_SHOW_IMAGE
#define CARNUMGETCHAR_DEBUG_FINALCUT //最终切分效果
#define CARNUMGETCHAR_DEBUG_SHOW_IMAGE_COSTFUNCTION
#define CARNUMGETCHAR_DEBUG_SHOW_IMAGE_COSTFUNCTION_DETAIL //过程细节显示
#define CARNUMGETCHAR_DEBUG_SHOW_RGBGRADIENTIMAGE 
#define CARNUMGETCHAR_DEBUG_SHOW_FIVEPOINT //显示五个点位置
//#define CARNUMGETCHAR_DEBUG_SAVE_IMAGE
#define CARNUMGETCHAR_DEBUG_PRINTFINFO
#define CARNUMGETCHAR_DEBUG_HORANGLE
#define CARNUMGETCHAR_DEBUG_VERANGLE
#define CARNUMGETCHAR_DEBUG_COLOR_REJUDGE //颜色再判断
#define CARNUMGETCHAR_DEBUG_COLOR_YandB 
#define CARNUMGETCHAR_DEBUG_SHOW_MORECUT_DETAIL //再次切分开关 过程
#define CARNUMGETCHAR_DEBUG_SHOW_MORECUT //再次切分开关 结果
#endif

//#define SAVE_COLOR_CHAR

//#define CARNUMGETCHAR_DEBUG_PRINTF_FIVEKEYPOINT_ANGLE

#ifdef MV_DR_CARNUMGETCHAR_DEBUG
#define MV_DR_CARNUMGETCHAR_DEBUG_SHOW_IMAGE_DETECTEDGE //双层黄牌的边缘检测
#endif

//#define CARNUMGETCHAR_DEBUG_SHOW_FIVEPOINT //显示五个点位置

#ifdef JING_PLATE_DEBUG_DEBUG
#define SHOW_RED_AREA
//#define CARNUMGETCHAR_DEBUG_SHOW_FIVEPOINT //显示五个点位置
#define CARNUMGETCHAR_DEBUG_SHOW_IMAGE_COSTFUNCTION_JING
//#define CARNUMGETCHAR_DEBUG_SHOW_IMAGE_COSTFUNCTION_DETAIL_JING //过程细节显示
#ifndef MV_CARNUMGETCHAR_DEBUG
#define CARNUMGETCHAR_DEBUG_SHOW_IMAGE_COSTFUNCTION
#endif
#endif

//#define CARNUMGETCHAR_DEBUG_HOUGH_CHECK_LOCATION

enum COLORS { c_blue, c_yellow, c_white, c_black, c_othercolor, c_falsecolor };
enum TYPES { t_normal,t_doublerowyellow,t_army,t_doublerowarmy,t_wj,t_doublerowwj, t_police, t_yue, t_sg, t_ling, t_othertype };
typedef struct MV_CUTINFO
{
	int x;
	int len;
	int cost_val;
}mv_cutinfo;

typedef struct MV_DR_CUTINFO
{
	CvPoint p;
	int len;
	int cost_val;
}mv_DR_cutinfo;

typedef struct MV_BLOB
{
	short x;
	short y;
	short width;
	short height;
} mv_blob;

typedef struct MV_BLOB_LINE 
{
	short sta;
	short end;
} mv_blob_line;

//int mv_PlateCharLocation_Main( IplImage *InImg, vector<CvPoint> &fivekeyPts, int rowFlag, IplImage *PlateImg, CvRect *CharLoc );
int mv_PlateCharLocation_Main( IplImage *InImg, vector<CvPoint> &fivekeyPts, int rowFlag, IplImage *outPlateImg, CvRect *CharLoc, COLORS& platecolor, TYPES& platetype );
void mv_sort(int *arr, int arrlen, int flag );
int mv_GetChar( IplImage *PlateImg, IplImage *CharImg, CvRect charRect );
int mv_GetCharloc(IplImage *PlateImg, CvRect *charRect, int referCharWidth );
int mv_GetCharloc_new(IplImage *PlateImg, CvRect *charRect, CvRect candPlateRect,int referCharWidth );
int mv_GetCharloc_new_new(IplImage *PlateImg, CvRect *charRect, CvRect candPlateRect,int referCharWidth );
int mv_GetCharloc_new_new_new(IplImage *PlateImg, CvRect *charRect,int referCharWidth );
int mv_GetCharloc_new_new_new_new(IplImage *PlateImg, CvRect *charRect, CvPoint *plateKeypoint, int referCharWidth );
int mv_RotateImage(IplImage *RGBimg, IplImage *Grayimg, CvPoint *Oripoint,CvPoint *Newpoint,CvPoint Centerpoint,int Horangle, int Verangle );
int mv_CutPlateChar( IplImage *Grayimg, CvRect *CharLoc ,int colorflag );
int mv_CutPlateChar_new( IplImage *Grayimg, CvRect *CharLoc ,int colorflag );
int mv_GetCorpoint(CvPoint *SrcPoint, int scale, CvPoint *DstPoint );
void mv_getHorlinebinimg( IplImage *HorlineImg,IplImage *HorlinebinImg);
void mv_getHorlineImage(IplImage *InImg,IplImage *HorlineImg1,IplImage *HorlineImg2 ) ;
//CvRect mv_GetPlateRect( IplImage *InImg, CvPoint *quadranglePoint );
void mv_Canny(IplImage *Inimg,IplImage *Cannyimg,IplImage *CannyimgHor, IplImage *CannyimgVert,int low_threshold, int uper_threshold);
void mv_Canny_Vertical(IplImage *Inimg,IplImage *Cannyimg,int low_threshold, int uper_threshold);
void mv_Canny_Horizon(IplImage *Inimg,IplImage *Cannyimg,int low_threshold, int uper_threshold);
void mv_Canny_N0direction(IplImage *Inimg,IplImage *Cannyimg,int low_threshold, int uper_threshold);
void mv_sobel(IplImage *InImg, IplImage *xSobel, IplImage *ySobel );
void mv_GetImageEdge( IplImage *InImg, IplImage *EdgeImg0,IplImage *EdgeImg1,IplImage *EdgeImg2, CvPoint *quadranglePoint );
CvRect mv_GetPlateRect(IplImage *InImg,IplImage *EdgeImg0,IplImage *EdgeImg1,IplImage *EdgeImg2, CvPoint *OriPoint ,CvPoint *NewPoint);
void mv_JudgeFourQuadranglePoint( IplImage *InImg, CvPoint *quadranglePoint );
int mv_EetOptimalline(IplImage *BinImg,CvPoint *line, int num,int finalnum,int Fflag );
int mv_GetPossibleline(CvPoint *line, int Lnum, CvPoint *Point );
int mv_Hough(IplImage *BinImg, int minAngelRang, int maxAngelRang, CvPoint *line, int loop );
int mv_GetTwoPointTheta(CvPoint point1, CvPoint Point2);
int mv_GetLeftandRight(IplImage *VsobleImg,CvPoint *Vline, int Vlinenum,CvPoint *HLine,CvPoint *fourPoint);
int mv_Getcolor_YandB( IplImage *inImg,IplImage *grayImg,CvRect rect );//弱判断
int mv_Getcolor_StrongYandB( IplImage *inImg,IplImage *grayImg,CvRect rect, int& BC );//强判断
int mv_Getcolor_Red( IplImage *inImg,IplImage *grayImg,CvRect *charRect,int charNo );
void mv_GetCannyEdge( IplImage *InImg, IplImage *EdgeImg0,IplImage *EdgeImg1,IplImage *EdgeImg2, CvPoint *quadranglePoint );
CvRect mv_GetCandidateInfo( IplImage *PlateImg, CvPoint *quadranglePoint );
int mv_CheckJingPlate( IplImage *inImg,IplImage *grayImg, CvRect *charRect );
int mv_CheckJunPlate( IplImage *inImg,IplImage *grayImg, CvRect *charRect );
int mv_CheckRedArea(IplImage *RGBImg, IplImage *grayImg,CvRect &RedRect, int *redarr );
int mv_GetRedWeight(CvRect *charloc, int *redarr );
int mv_GetJingOptiCut(double *Intiproj,CvRect maxredRect, mv_cutinfo &jing_max, int imgw, int maxred );
int mv_JudgeJingCut(IplImage *PlateImg, double *Intiproj, mv_cutinfo jing_max, mv_cutinfo yellowmax );

//双层黄牌
int mv_DR_GetCorpoint(CvPoint *SrcPoint, int scale, CvPoint *DstPoint );
int mv_DR_GetCharloc(IplImage *PlateImg, CvRect *charRect, CvPoint *plateKeypoint, int referCharWidth );
CvPoint mv_DR_DetectEdgevtc(IplImage *sobelImg, IplImage *colorEdge,CvRect rect );
int mv_DR_GetCutLocation(mv_DR_cutinfo cInfo, CvRect *cLoc ,int maxWidth, int maxHeight);
void mv_GetLowrowCost(mv_cutinfo *LocCost, double *Intiproj,int rangW,int rangL,int maxEdge,int stanCentW,int stanCentL );
int mv_GetCharLoc_Downrow(mv_cutinfo LocInf, CvRect *CharLoc, int top,int bot, int maxEdge );
int mv_DR_ComputeRotateAngle0(IplImage *SobelImg, IplImage *BinImg, IplImage *RotatedImg, CvRect *CharRect );
CvRect mv_DR_GetEdgeHorUprow(IplImage *RGBsobelImg, CvRect *CharRect, int Marky );
CvRect mv_DR_DetectEdge(IplImage *sobelImg, IplImage *colorEdge,CvRect upRect,CvRect downRect );

//白牌处理
//int mv_GetWhitePlateChar(IplImage *Inimg, int *proj, mv_cutinfo& OptiCost, int upline ,CvRect *whiteLoc);
int mv_GetWhitePlateChar(IplImage *Inimg, int *proj,CvRect *whiteLoc, int Y_shift, int Y_height );

#endif