#pragma once
#include <cv.h>

using namespace cv;



typedef struct Target
{
	Rect rRect;//目标位置信息

	double dConfidece; //目标置信度
};


typedef struct TargetInfo
{
	int nClassType;//目标类别，从1开始，按照模型加载顺序编号

	Target tTarget;//目标信息
};


class MvDetect;

class MvMulObjDetect
{

public:

	MvMulObjDetect(void);

	~MvMulObjDetect(void);


	//原始接口，单类两类检测
public:

	//获取版本号
	static char* mvGetVision();
	
	bool mvInitDetectModel(char* );

	void mvUnInitDetectModel();


	/*稍早版本接口，对应张伟*/

	//单类别检测接口：输入图像，检测到目标位置，置信度，置信度阈值,检测框合并参数，默认不合并
	void mvSingleClassDetect(Mat mImg,vector<Target>&vTarget,double dThres=30.0,double dCover=2.0);


	//加载两类检测模型
	bool mvInitMultiModel(char*,char*);

	void mvUnInitMultiModel();

	//两类检测接口：输入图像，检测到目标位置，置信度，置信度阈值，检测框合并参数，默认不合并
	void mvMultiClassDetect(Mat mImg,vector<Target>&vClass1,
		vector<Target>&vClass2,double dThres=30,double dThres1=30,double dCover=2.0);



/*新加N类检测接口，输入多少个检测模型，检测多少个类别 */
public:

	//加载N类检测模型,Classifier为分类器路径名称，nClass为类别数
	bool mvInitMulti(char**Classifier ,int nClass);

	//对象释放函数
	void mvUnInitMulti();


	//N类目标检测接口：输入图像，检测到目标信息，置信度阈值数组，目标合并参数，图像长边缩放长度，1.0不缩放
	void mvNClassDetect(Mat mImg,vector<vector<Target> >&vTarget,
		double* dThres,double dCover=2.0,double dMaxLength=1.0);




private:

	MvDetect* mMultiDetect;

	//边界保护
	vector<Target> mvRationality( vector<Target>& V, const Mat pImage );

	double mvRectCoverRate( Rect r1, Rect r2 );   //计算两个矩形的重叠率

	Rect mvRectCombineBig( Rect r1, Rect r2 );

	//目标合并函数
	int mvRectGroup( vector<Target>& V,double dThres);

};