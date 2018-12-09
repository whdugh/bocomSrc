#ifndef _MVPLR_NEW_H_
#define _MVPLR_NEW_H_

#include "Interface.h"
#include "Mv_CarnumDetect.h"

//using namespace caffe;

#define DEBUG
//#define ZRG_DEBUG

#ifdef DEBUG
//#define TIME_START
//#define SAVE_RESULT 
//#define SAVE_FAKE_PLATE
//#define SAVE_SAMPLE //保存样本
//#define MVPLR_NEW_SAVEIMAGE
//#define MVPLR_NEW_SHOWIMAGE
//#define PLATELOCATIONOLDMETHOD_DEBUG //老方法车牌定位算法开关
//#define LIUYANG_SHOW_CARandPLATE
//#define PRINTF_DETECT_AND_KEYPOINT
//#define DEBUG_SHOW_FINAL_RESULT
#endif

#ifdef TIME_START
#define TIME_COUNT
#ifndef TIME_COUNT
#define TIME_ALL_COUNT
#endif
#endif

#define CHANGE_O_T_D //强制改变'O'成'D'

enum plate_colors { blue, yellow, army, police, white, unknown, wj, yue, sg, ling, falsecolor };

static char charLabels[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','1', 'J',
	'K', 'L', 'M', 'N','0', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z','-','+','#','$',
	'2', '3','4', '5', '6', '7', '8', '9' 
};

static char charLabels_cn[] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't','u', 'v',
	'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E'
};

#ifdef CHANGE_O_T_D
static char charLabels_letter[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','I', 'J',
	'K', 'L', 'M', 'N', 'D','P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z'
};
#else
static char charLabels_letter[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','I', 'J',
	'K', 'L', 'M', 'N', 'O','P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z'
};
#endif

typedef struct MV_PLATEINFO
{
	char    carnum[7];             //车牌号码

	double	confidence[7];		   //位置信度

	CvRect  position;              //车牌位置,相对于原图

	plate_colors   color;                 //车牌颜色

	int     vehicle_type;          //车辆类型	

	float	iscarnum;              //从匹配结果看是否是车牌

	double     mean;               //车牌亮度

	double     stddev;				//车牌方差	

	char    wjcarnum[2];           //武警牌中间的两个小字

	int     carnumrow;             //单排或者双排车牌

	CvRect    smearrect[20];         //光柱所在位置

	int       smearnum;				//光柱的数目

	int      VerticalTheta;          //垂直倾斜角,以角度为单位

	int      HorizontalTheta;          //水平倾斜角,以角度为单位

	int      nCarNumDirection;         //车牌的方向，通知安发是前牌还是尾牌

	bool     bIsMotorCycle;          //是否为摩托车！

}MvPlateinfo;

class Mv_VehicleDetector;
class Mv_NoPlateVehicleDetector;
//class licensePlateLoc;
class ConvNet;
class Mv_Vehicle;
class MVPLR_Memory;


class Mv_PLR_New
{
public:
	Mv_PLR_New();
	~Mv_PLR_New();

	int mv_Init( char* strPath );
	int mv_UnInit();
	//int mv_PLRmain(  IplImage* InImg, CvRect detectRect,vector<MvPlateinfo> &PLR_result);
	//int mv_PLRmain(  IplImage* InImg, Mv_Vehicle *VehicleDet,int VehicleNum,vector<MvPlateinfo> &PLR_result );
	//int mv_PLRmain(  IplImage* InImg, CvRect *PlateRect,int PlateNum,vector<MvPlateinfo> &PLR_result);
	int mv_PLRmain(  IplImage* InImg, CvRect detectRect,vector<carnum_context> &PLR_result);
	int mv_PLRmain(  IplImage* InImg, Mv_Vehicle *VehicleDet,int VehicleNum,vector<carnum_context> &PLR_result );
	int mv_PLRmain(  IplImage* InImg, CvRect *PlateRect,int PlateNum,vector<carnum_context> &PLR_result);

private:
	//int mv_PLR_Recognition( IplImage*, CvRect*, int, MvPlateinfo& );
	int mv_PLR_Recognition( IplImage*, CvRect*, int, carnum_context& );
	int mv_PLR_Judge( carnum_context& PLRresult );

	// 创建注销DL网络
	void initializeDL();
	void initializeDL_letter();
	void initializeDL_cn();

	IplImage *pCovnetImage;
	int nImgSize;
	// CNN网络参数


	// 数字和字母DL分类器
	ConvNet* mv_DbnRec;	
	CvMat* convetdst;

	// 字母DL分类器
	ConvNet* mv_DbnRec_letter;	
	CvMat* convetdst_letter;
	
	// 汉字DL分类器对象
	ConvNet* mv_DbnRec_cn;
	CvMat* convetdst_cn;


	Mv_NoPlateVehicleDetector* mv_detector;
	licensePlateLoc/*<float>*/ pll;
	//MVPLR_Memory *pLocMemory;
};

#endif