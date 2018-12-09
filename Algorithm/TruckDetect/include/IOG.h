#ifndef IOG_H_
#define IOG_H_

class opencv;

class CIOG 
{
	
public:

	static CIOG* getInstance(void);					// 静态s实例化函数
	static void deleteInstance(void);				// 静态释放对象

	int m_Width;
	int m_Height;

	IplImage* m_pGray;								// BGR转GRAY的灰度图像
	CvMat* m_pGrayMat;								// 灰度Mat	
	CvMat* m_pKxMat;								// x方向梯度滤波核
	CvMat* m_pKyMat;								// y方向梯度滤波核

	
	int init();										// 初始化内存分配
	int getIOG(IplImage* img, CvMat* x, CvMat* y);	// 计算x,y方向梯度，其中img 是输入 ，x,y是输出。
	int clean();									// 释放内存

private:
	CIOG();
	~CIOG();

	CIOG (const CIOG&) {}

	static CIOG* m_ciog;							// 类内对象

};


#endif

