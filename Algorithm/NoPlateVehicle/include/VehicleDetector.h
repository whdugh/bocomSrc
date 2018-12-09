#ifndef VEHICLEDETECTOR_H_
#define VEHICLEDETECTOR_H_

using namespace std;

struct Target;
class MvMulObjDetect;

enum VehicleClass
{
	VEHICLE = 0,                /* = 0 机动车     */
	BICYCLE_VEHICLE ,           /* = 1 非机动车   */
	PEDESTRIANER,               /* = 2 行人       */	
	ERROR_VEHICLE               /* = 3 错误       */
};

#ifndef MV_VEHICLE_
#define MV_VEHICLE_
class Mv_Vehicle
{

public:
	Mv_Vehicle() : m_dConfidece( 0.0 ), m_dPlate_Confidece(0.0), m_dTaxi_Confidece(0.0), m_Plate( -1 ), m_Taxi( -1 )
	{
		memset( &m_Position, 0, sizeof(CvRect) );
		memset( &m_Plate_Position, 0, sizeof(CvRect) );
		memset( &m_Taxi_Position, 0, sizeof(CvRect) );
		m_VehicleClass = VEHICLE;
	}

	CvRect m_Position;			// 车辆位置
	double m_dConfidece;		// 车辆可信度
	int m_Plate;				// -1 未检测；0 无牌；1 有牌
	CvRect m_Plate_Position;	// 车牌位置 全局坐标 非相对于车辆的坐标
	double m_dPlate_Confidece;	// 车牌可信度
	int m_Taxi;				    // 出租车 -1 未检测；0 无牌；1 有牌
	CvRect m_Taxi_Position;	    // 出租车位置 全局坐标 非相对于车辆的坐标
	double m_dTaxi_Confidece;	// 出租车可信度

	VehicleClass m_VehicleClass;//新增类别标签，类别2及以上暂无车牌信息

};
#endif

class Mv_VehicleDetector
{

public:

	Mv_VehicleDetector();

	~Mv_VehicleDetector();

	int mvInit( char* );

	int mvUninit();

	virtual int mvDetectVehicle( const IplImage* src, const CvRect detect_area,	vector<Mv_Vehicle>& Vehicle_Result, double thresh, bool bresize = true  );

	//new add
	virtual int mvDetectVehicle(const IplImage* src,vector<Mv_Vehicle>& Vehicle_Result, double* thresh);

	virtual char* mvGetVersion();

protected:

	MvMulObjDetect *m_pVehicleDetector;

	double m_Resize_Rate;

protected:

	// 合并目标框序列
	int TargetGroup( vector<Target>& );         

	// 是否满足重叠条件
	bool isOverLap( const cv::Rect, const cv::Rect );
	
	// 合并目标框
	Target CombinTarget( const Target, const Target );

	// 获取车辆位置
	int GetVehicles( const const vector<Target>, vector<Mv_Vehicle>& );


	//new add 
	int GetVehicles( const const const vector<vector<Target> >, vector<Mv_Vehicle>& );

};

class Mv_NoPlateVehicleDetector : public Mv_VehicleDetector
{

public:

	Mv_NoPlateVehicleDetector();

	~Mv_NoPlateVehicleDetector();

	int mvInit( char*, char* );
	int mvInitTaxiDetect( char* );

	//重载新初始化接口，nDetectClass默认为1，最大为3，默认只检测车
	int mvInit( char*,int nDetectClass=1);

	int mvUninit();

	//车辆单类检测
	int mvDetectNoPlateVehicle( const IplImage* src, const CvRect detect_area, vector<Mv_Vehicle>& Vehicle_Result, double thresh );

	//重载无牌车功能,多类检测，最后两个参数与原来不同
	int mvDetectNoPlateVehicle( const IplImage* src, const CvRect detect_area,vector<Mv_Vehicle>& Vehicle_Result, double* thresh,bool bDetectMotoPlate = false );

	//重载无牌车功能，输入了车辆信息，不再做车检
	int mvDetectNoPlateVehicle( const IplImage* src, const CvRect detect_area, vector<Mv_Vehicle>& Vehicle_Result );

	
	int mvDetectPlate( const IplImage* src, const CvRect detect_area, vector<Mv_Vehicle>&, double thresh );

	int mvDetectPlate( const IplImage* src, Mv_Vehicle&, double thresh );

	int mvDetectPlateSupply( const IplImage* src, const CvRect detect_area, vector<Mv_Vehicle>&, double thresh );

	int mvSetNotTaxiDetect();


protected:

	MvMulObjDetect *m_pPlateDetector;

	MvMulObjDetect *m_pTaxiDetector;

	double m_Vehicle_Resize_Rate;

	bool NeedDetectPlate( const CvRect detect_area,  const Mv_Vehicle _Vehicle );


//Add by ly 2015年6月23日,新重载的函数
private:

	int nClasses; 

	//new add for multiclass plate detect
	int mvDetectPlates( const IplImage* src, const CvRect detect_area,vector<Mv_Vehicle>&, bool bDetectMotoPlate);

	//为不同放缩尺寸做重载
	int mvDetectPlates( const IplImage* src, Mv_Vehicle&, double thresh ,bool bIsMotor=false );

	//判断出租车
	int mvJudgeTaxi( const IplImage* src, const CvRect detect_area, vector<Mv_Vehicle>&_vVehicles );

};


#endif