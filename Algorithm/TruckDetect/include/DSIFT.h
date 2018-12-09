#ifndef DSIFT_H_
#define DSIFT_H_

class opencv;

class CDSIFT
{

public:
	CDSIFT();
	~CDSIFT();

	int m_ScrWidth;					// 样本图像/扫描框图宽
	int m_ScrHeight;				// 样本图像/扫描框图高
	int m_GridWidth;				// 网格宽
	int m_GridHeight;				// 网格高
	int m_gridstep;					// 网格步长
	int m_PatchSize;				// Patch尺度

	CvMat* m_grad_x;				// x方向梯度
	CvMat* m_grad_y;				// y方向梯度
	CvMat* m_DSIFT_Mat;				// 特征Mat
	CvMat* m_Kernal;				// 权值核
	CvMat* ydest;					// 角度x加权
	CvMat* xdest;					// 角度y加权
	CvMat** m_EeightOrient_Mat;		// 八方向梯度

	
	/*
	函数功能：通过输入初始化除m_DSIFT_Mat 外的成员变量。
	输入：一幅图像中每个像素的x方向的梯度矩阵grad_x,
	     和y方向的梯度矩阵grad_y, 以及网格步长gridstep.
	输出：无。
	返回值：0表示： 正常。
			-1表示： 输入参数有误。
	*/
	int init(CvMat* grad_x, CvMat* grad_y, int gridstep);

	/*
	函数功能：获得图片网格行列数
	输入：无
	输出：行列数
	返回值：0
	*/
	int getGrid(int& GridWidt, int& GridHeight); // 0, 0, 0, 0, 0.125, 0.375, 0.625, 0.875

	/*
	函数功能：DSIFT特征提取API
	输入：无
	输出：DSIFT特征Mat
	返回值：0表示： 特征提取正常
			-1表示：输入参数有误 
	*/
	int getDSIFT(CvMat* DSIFT_Mat);

	/*
	函数功能：保存m_DSIFT_Mat数组到传入的文件名中。
	输入：文件名filename.
	输出：无。
	返回值：-2表示：输入的参数为空。
			-1表示：文件不能打开。
			0表示：要处理的成员变量m_DSIFT_Mat为空。
			1表示：存储成功。
	*/
	int save(char* dir);

	/*
	函数功能：释放init中开辟的内存。
	输入：无。
	输出：无。
	返回值：无意义。
	*/
	int clean();

	/*
	函数功能：计算把方向梯度大小。
	输入：无
	输出：无。
	返回值：-1表示：梯度图为空
			0表示：处理成功
	*/
	int getEeightOrientation();

	/*
	函数功能：计算梯度像素权值
	输入：无
	输出：无
	返回值：0表示：权值计算完成
			1表示：八方向图组为空

	*/
	int calculateWeight();

	/*
	函数功能：归一化
	输入：无
	输出：无
	返回值：0表示：归一化完成

	*/
	int Normalization();


};


#endif