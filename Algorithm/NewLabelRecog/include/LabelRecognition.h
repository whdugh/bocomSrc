// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef CLASSIFY_EHD_TEXTURE
#define CLASSIFY_EHD_TEXTURE

#include <vector>
#include "cv.h"
#include "cxcore.h"
#include "ml.h"
#include "Mv_CarnumDetect.h"//车牌库里面的头文件
#include "CarLabel.h"

using namespace cv;
using namespace std;

#define LABELNUM 809  //DL可识别的车标类别总数
#define TruckDetect   //老算法黄牌车检测宏开关
#define OLDCARDETECT  //老算蓝牌车标检测宏开关（前牌 or 尾牌）
#define Labelpf

class Carlogo;
//class Carlabel;

//车型输出枚举
enum TRUCK_LABEL
{
	LARGEBUS = 1,                //大型客车     1
	LARGEVAN,                    //大型货车     2
	MEDIUMBUS,                   //中型客车     3
	SMALLCAR,					//小型客车		4
	UNKNOW,						//未知			5
	MISS,						//未知			6
	SMALLVAN,                   //小型货车		7
	SUV,                        //suv           8
	MBC,                        //面包车         9
	YVK,                        //依维柯        10
	MPV,                        //MPV          11

};

#ifndef OBJECTLABEL
#define OBJECTLABEL
typedef struct OBJ_LABEL
{
	int plabel; //车标类别标号

	float fCLP; //置信度

	OBJ_LABEL()
	{
		plabel = OTHERS;

		fCLP = 0.f;
	}

}object_label;
#endif


#define MV_SAVEIMG_DEBUG

class CarLabel
{

public:
	CarLabel();

	~CarLabel();
	
	/*车标初始化*/
	void label_Init();	

	/*车标释放函数*/
	void label_Destroy();
	

	/*设置图像宽和高	
	mvSetImageWidthandHeight：设置图像宽和高，仅在mvGetClassifyEHDTexture输入参数imageData为char*时使用
	int nWidth：待检测图像的宽
	int nHeight：待检测图像的高
	*/
	void mvSetImageWidthandHeight( int nWidth, int nHeight );

	/*DL车标识别主函数（二次识别调用接口）
	IplImage *img：输入的图像数据
	carnum_context *vehicle_Info：输入的车牌信息（若无则输入NULL）
	CvRect *rtRoi：输入的车检区域（若无则输入NULL）
	object_label *objLabel：输出的车标类别
	int isDayBytime：使用默认值1（白天）
	*/
	int mvDLCardCarLabel( IplImage *img, carnum_context *vehicle_Info, CvRect *rtRoi, 
		object_label *objLabel, int isDayByTime=1);
	
	/*DL车标识别主函数（於峰的x86系统用）
	IplImage *img：输入的图像数据
	carnum_context *vehicle_Info：输入的车牌信息（若无则输入NULL）
	CvRect *rtRoi：备用无牌车区域输入接口（若无则输入NULL）
	vector<Target> vTarget：输入的车检区域
	object_label *objLabel：输出的车标类别
	int isDayBytime：使用默认值1（白天）
	*/
	/*int mvDLCardCarLabel( IplImage *img, carnum_context *vehicle_Info, CvRect *rtRoi, 
		vector<Target> vTarget, object_label *objLabel, int isDayByTime=1);

	vector<Target> mvCarDetection(IplImage *img);

	CvRect mvCarLocation(IplImage *img, vector<Target> CarIn, CvRect vPlate);
*/
	bool mvIntersect(const CvRect rt1, const CvRect rt2, float *fThresh = NULL);


	/*
	GetVersion：获得版本号	
	返回：static char Version[] = { "color Recognition Version x.x.x.x" " "  __DATE__ " " __TIME__ };
	*/
	static char* GetVersion();

	//设置模型初始化路径
	void mvSetPath(char* strPath);

	IplImage* mvCreateROI(IplImage *src, CvRect Brand, CvRect &m_RectROI, float fsize = 200);

	/*存图（按照不同种类和数量上限）
	输入：
	IplImage *img：原始图像
	IplImage *psubimage：局部图像（车辆）
	carnum_context *vehicle_Info：输入的车牌信息（若无则输入NULL）
	char path[512]：存图路径
	int nsort：存图类别号
	CvRect rtV：局部图像在原始图像中的位置（车辆）
	int nTh：数量上限（每种1000）
	*/
	void mvSaveImage( IplImage *img, IplImage *psubimage,carnum_context *vehicle_Info, char path[512], int nsort, CvRect rtV, int nTh = 10000 );

	//车型判别输出函数
	void TruckOutStyleput( int classfication, int &nTruckDetect);

	//车标类别输出明细表
	//int LabelOutput_sub(int classfication);

#ifdef TruckDetect
	//大类truck型输出
	int TruckOutput(int classfication);	
	int LabelOutput_sub(int classfication);
	int LabelOutput_bsub(int classfication);
#endif
	

protected:	


private:
	char buffer_path[512];
	int label[LABELNUM];	
private:

	//控制初始化
	int first;
	int m_nWidht;
	int m_nHeight;
	IplImage *m_oriImg;

#ifdef TruckDetect
	int n;
	IplImage* Original;
	HOGDescriptor *hogT;
	int nImgNumT;
	int dimT;
	////样本矩阵，nImgNum：横坐标是样本数量， WIDTH * HEIGHT：样本特征向量，即图像大小  
	int PCAdiT;
	CvSVM svmT;
	CvMat* pMeanT;
	CvMat* pEigVecsT;
#ifdef OLDCARDETECT
	//前牌模型加载
	HOGDescriptor *hog;
	int nImgNum;
	int dim;
	////样本矩阵，nImgNum：横坐标是样本数量， WIDTH * HEIGHT：样本特征向量，即图像大小  
	int PCAdi;
	CvSVM svm;
	CvMat* pMean;
	CvMat* pEigVecs;

	//尾牌模型加载
	HOGDescriptor *hogB;
	int nImgNumB;
	int dimB;
	////样本矩阵，nImgNum：横坐标是样本数量， WIDTH * HEIGHT：样本特征向量，即图像大小  
	int PCAdiB;
	CvSVM svmB;
	CvMat* pMeanB;
	CvMat* pEigVecsB;
#endif
#endif

	
	//DLL
	Carlogo *net;

	
};
#endif
