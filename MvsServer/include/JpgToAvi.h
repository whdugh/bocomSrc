// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//
#ifndef SKP_JPGTOAVI_H
#define SKP_JPGTOAVI_H

#include "global.h"

typedef std::list<AVI_FrameInfo> Jpeg_List;
typedef std::list<AVI_EventInfo> Event_List;


class CJpgToAvi
{
public:
	CJpgToAvi();
	virtual ~CJpgToAvi();

//公有方法
public:
	//打开视频文件
	bool OpenFile(const char* chFileName);
	//关闭视频文件
	bool CloseFile();
	//添加一帖数据
	bool AddFrame(unsigned char* pBuffer,unsigned int nLength,const std::string& strEvent);
	//添加一帖数据
    bool AddOneFrame(unsigned char* pBuffer,unsigned int nLength,SRIP_DETECT_HEADER* sDHeader);
	//是否开始编码
	bool IsEncoding() {return m_EncodeFile != 0;}
	//设置录像宽高
	void SetWidth(unsigned int uWidth) { m_uWidth = uWidth;}
	void SetHeight(unsigned int uHeight){ m_uHeight = uHeight;}
	//返回录像文件名
	const char*  GetEncodingFileName() { return m_strFileName.c_str();}

	//写录像头
    void WriteHeader();

    //add ext-header data and big pictures.(zhangyaoyao)
    void AddBigPics(const string &strEvents);
    //添加车牌记录和图片
    void AddPlatePics(int nChannel,UINT32 uBeginTime,UINT32 uEndTime);

    //rgb->jpg
    bool Encode(BYTE* pBuf,int nWidth,int nHeight,int* srcstep,BYTE* pOutBuf);

protected:

//私有变量
private:

	//目标文件
	FILE* m_EncodeFile;

	//宽
	unsigned int m_uWidth;
	//高
	unsigned int m_uHeight;

	//文件名
	std::string m_strFileName;

	//录象帧数列表
	Jpeg_List m_list;
	//事件列表
	Event_List m_elist;
	//车牌记录总数
	int m_PlateCount;

	//录象总帧数
	unsigned int m_nFrame;

	//postion of picture.
    UINT32 m_uPicPos;
	int m_nChannelId;
};

#endif // SKP_JPGTOAVI_H
