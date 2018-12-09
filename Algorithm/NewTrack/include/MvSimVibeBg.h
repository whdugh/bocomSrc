// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef	SIM_VIBEMODEL_H
#define SIM_VIBEMODEL_H

#include "libHeader.h"

class MvSimVibeModel
{
public:
	MvSimVibeModel(void);

	~MvSimVibeModel(void);

	void mvUninitVibeModel(void);
	
	// 函数功能：得到当前的vibe模型
	bool mvGetVibeModel(
			IplImage *pCurRgbImg,			//输入的当前彩色图像
			const int &nSameColorDist=2,	//颜色相似的距离阈值
			const float &fSameColorRate=0.1f, //颜色相似的比率阈值
			const int &nBgUpdateFre=16,		//背景样本点更新频率
			const int &nBkPtCntTh=1,	    //认为为背景时相似的点数阈值
			const int &nSampleCnt=5,	    //样本点数目
			const int &nNerbor=2,		    //邻域宽度
			IplImage *pFkMaskImg=NULL       //传递进来的前景mask
		);

	// 函数功能：得到当前的vibe模型的前景
	IplImage *mvGetFkImg4VibeModel(
				IplImage *pCurRgbImg,
				bool bAddFkMask=false );

public:
	IplImage *m_bgrImg; //当前帧彩色图像
	IplImage *m_fkImg;  //当前帧前景图像
	
private:
	void mvInitVibeModelVar( );

	void mvCreateImages(void);

	void mvAllocVariable(const IplImage *pImg, 
		const int &nSameColorDist, const float &fSameColorRate,
		const int &nBgUpdateFre, const int &nBkPtCntTh, 
		const int &nSampleCnt, const int &nNerbor);

	//初次初始化背景模型的m_N幅背景样本图像  
	bool mvInitSampleBgImgs(IplImage *img);

	//背景与前景更新
	void mvUpdateVibeModel(IplImage *pCurRgbImg, 
		          IplImage *pFkMaskImg=NULL);

	//彩色图像的背景模型更新
	void mvUpdate8uC3VibeModel(IplImage *pBgrImg,
			 IplImage *foreImg, int nBgUpdateFre);

	//计算彩色距离
	inline unsigned short mvEuclidDist_L_Inf(const uchar *pSrcB,
		const uchar *pSrcG, const uchar *pSrcR, const uchar *pOutB,	
		const uchar *POutG, const uchar *pOutR) const;

private:
	//函数功能：彩色图像的背景模型更新
	void mvUpdateBgSamples(IplImage *pBgrImg, int nBgUpdateFre,
		                   IplImage *pFkMaskImg );
		
	//函数功能：根据模型来获取当前彩色图所对应的前景图
	void mvGetFkImgFromModel(IplImage *pBgrImg, IplImage *foreImg);


private:
	bool m_bHadInit; //是否进行过初始化了

	unsigned short m_nNerborRadius; //邻域半径

	short m_BOUND_UP;  //邻域随机数的开上界
	short m_BOND_DOWN; //邻域随机数的闭下界

	IplImage **m_samImgs; //背景模型样本图像集



	IplImage *m_rndNbrImg;		//邻域点 随机数图像
	IplImage *m_rndSampleImg;   //抽样点 随机数图像
    IplImage *m_rndUpdateImg;   //背景更新 随机数图像

	CvSize m_imgSize;      
	int m_width, m_height; //背景建模图像宽度,高度
	bool m_bIsResize;      //是否需要缩放

	int m_chnnls;       //颜色通道数目

	int m_nSampleCnt;   //每个像素的样本个数
	int m_nBgUpdateFre; //背景更新的频率(多少帧一次)
	int m_nMinN;		//最小的基数

	int   m_nSameColorDist;  //认为颜色相似的距离阈值
	float m_fSameColorRate;  //认为颜色相似的比率阈值

	int m_nRunCount;    //运行帧数的统计

	CvRNG m_rnd_state; //随机数生成器		
};

#endif
