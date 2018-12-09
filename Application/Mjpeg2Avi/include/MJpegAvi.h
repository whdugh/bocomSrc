#ifndef SKP_MJPEG_H
#define SKP_MJPEG_H

#include <string>
#include "byteswap.h"
#include <stdio.h>
#include "avifmt.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <list>


typedef struct _Jpeg_Data
{
  unsigned int size;
  unsigned int offset;
}Jpeg_Data;

typedef std::list<Jpeg_Data> ListJpeg;


class CMJpegAvi
{
public:
	CMJpegAvi();
	virtual ~CMJpegAvi();

//公有方法
public:
	//打开视频文件
	bool OpenFile(const char* chFileName);
	//关闭视频文件
	bool CloseFile();
	//添加一帖数据
	bool AddFrame(const unsigned char* pBuffer,const unsigned int nLength);
	//是否开始编码
	bool IsEncoding() {return m_EncodeFile != 0;}
	//设置录像宽高
	void SetWidth(unsigned int uWidth) { m_uWidth = uWidth;}
	void SetHeight(unsigned int uHeight){ m_uHeight = uHeight;}
	//返回录像文件名
	const char*  GetEncodingFileName() { return m_strFileName.c_str();}

protected:
	void print_quartet(unsigned int i);
	long get_file_sz();
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

	//
	ListJpeg m_list;

	//
	int m_nFrame;
};

#endif // SKP_MJPEG_H
