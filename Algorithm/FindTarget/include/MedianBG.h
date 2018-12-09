// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//


/* 
* 文件名称：MedianBG.h
* 摘要: 中值背景建模
* 版本: V2.2
* 作者: 方辉
* 完成日期: 2009年8月7日
*/

#ifndef __MEDIANBG2_H
#define __MEDIANBG2_H

#include <cv.h>
#include <highgui.h>

class CMedianBG2
{

public:

	bool m_bGenBackground; // 告知外部程序初始背景是否建立完

	CMedianBG2();
	virtual ~CMedianBG2();

	/* 设定取景框 */
	void FillViewRgn(CvPoint pt[4]);	

	/* 输入当前帧，如果是第一帧则初始化 */
	void Input(const IplImage *frame);

	/* 初始化背景（基于中值滤波）*/
	bool GenBackGround(int frameNum, int sampleRate);

	/* 初始化背景（直接将当前帧作为背景，并由外部程序更新改进）*/
	bool GenBackGround_Simple();

	/* 针对拥挤情况下的背景初始化，采用帧间差分法 */
	bool GenBackGround_InterFrameDiff(int frameNum, int sampleRate, int Thd_diff);

	/* 背景更新 */
	void Update(void);

	/* 根据指定区域信息对背景进行更新 */
	void Update_ShortTermBg(const int count, const CvRect * ObjRect, bool updateInRect);

	/* 根据指定区域信息对背景进行更新 */
	void Update_LongTermBg(const int count, const CvRect * ObjRect, bool updateInRect);

	
	IplImage * GetBackground();
	IplImage * GetBackground_LongTrem();

	IplImage * GetForeground();
	IplImage * GetProFg();
	IplImage * GetCurrentFrame();
	IplImage * GetFilteredFg();
	IplImage * GetLastFrame();

	// 获得背景像素的标准差值
	IplImage * GetPixelSDV();
	
	/* 去除阴影，将前景中阴影像素赋值为(0,0,ShadowValue) */
	void RemoveShadow(int ShadowValue);

	void RemoveShadow(int nShadowValue, IplImage* imgForeground);

	/* 提取前景中的物体 */
	bool ExtractObject(int Thd_bgSub, int Thd_RectSize, bool shadowRemove, int *RectNum, CvRect *rects);

	/* 对提取的物体进行分析 */
	int CheckObject(CvRect *rects, int rectNum, int Thd_InterDiff, int min_cornerNum);

	/* 设置背景更新速度（当前帧权重）*/
	void SetUpdateWeight(float alpha);

	/* 从外部程序中获取前景物体信息 */
	void ObjectRgn(const int count, const CvRect * ObjRect);

	/* 获得当前帧序号 */
	long GetFrameNum();

	/* 背景差分生成前景图，前景可通过GetForeground()获得 */
	bool bgSubtraction( int Thd_bgSub );

	/* 获取区域角点信息 */
	CvPoint *GetCorners(int *cornerNum);



private:

	CvPoint m_pViewRgn[4];		// 取景框

	IplImage * m_pMaskImg;		//
	IplImage * m_pBackGround;	//背景模型（短期）
	IplImage * m_pBackGround_LongTerm; // 长期背景模型
	IplImage * m_pForeGround;	//前景
	IplImage * m_pFilteredFg;	//经过处理的前景
	IplImage * m_pFrame;			//用于保留当前帧
	IplImage * m_pLastFrame;		//


	// 用于背景更新
	IplImage * m_pNewFgImg;
	IplImage * m_pNewBgImg;
	IplImage * pFgProImg;

	int m_nWidth,m_nHeight;		//长和宽

	long m_nFrames;				//用于统计接收到的视频帧数（由于在初始化背景前需调用两次input()，所以其初始值设为-2）

	int m_nMaskPixels;	//mask像素数目

	//背景建模参数
	int m_nInitFrames;	// 总共用于背景建模的帧数
	int m_nDT;			// 背景初始化中采用的帧率（每隔m_nDT帧）
	int m_nBuffLen;     // 缓冲区长度

	bool m_bObjects; // 前景中是否有物体
	int m_nObjectsNum; // 上层来的物体个数
	CvRect* m_pObjRects; // 物体区域
	
	//背景更新系数
	float m_fbgalpha;

	//建模区域的buff
	int *indexj;
	int *indexi;
	int **buffb, **buffg, **buffr;

	// 角点参数
	int m_nCornerNum;
	CvPoint *m_pCorners;


	int Median(int** buffb, int** buffg, int** buffr, long k, int n);
	int max3(int a, int b, int c);
	int min3(int a, int b, int c);
	float max3f(float a, float b, float c);
	float min3f(float a, float b, float c);
	int minArr(int* dis, int n);
	bool thresholdFg();	
	void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v );
	bool interROIDiff(CvRect Rect, int Ti_roi);
	bool checkROI(CvRect Rect);
	void setFrame(IplImage *pForeGround);
	void updateBgModel(CvRect Rect);
	void getIntensityChange(int* mdv_b,  int* mdv_g, int* mdv_r);
	bool cornerTest(CvRect Rect, int min_cornerNum);
	

	/* 自动二值化图像 基于Otsu法 */
	float AutoThersholding(int minvalue, int maxvalue);

};

#endif // 
