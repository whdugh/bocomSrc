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

#include "Common.h"
#include "CommonHeader.h"
#include "FileManage.h"
#include "XmlParaUtil.h"
#include "CSeekpaiDB.h"
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "RoadCommunication.h"
#include "FtpCommunication.h"
#define DEBUG_FILEMANAGE

#define DISK_LIMIT_90	90

CFileManage g_FileManage;

//构造
CFileManage::CFileManage()
{
	m_nThreadId = 0;
	m_nCurDeletePicID = -1;
	m_nCurDeleteVideoID = -1;
	m_strH264FileName = "";
	m_uExtendPicID = 0;
	m_uExtendMaxPicID = 0;
	m_uExtendMinPicID = 0;
	m_tDetetePicTime = 0;
	m_tDeteteVideoTime = 0;
	m_uRemovePicLoopCount = 0;
	m_uRemoveVideoLoopCount = 0;
	m_uGetPicIdLoopCount = 0;
	m_uGetVideoIdLoopCount = 0;

	m_strDspH264FileName = "";

	m_nThreadTableId = 0;
	m_nThreadBmpId = 0;
	m_nThreadCleanDskId = 0;
}

//析构
CFileManage::~CFileManage()
{
}

//删除图片及录象文件(bVideo ==0图片,bVideo==1录象,nMaxCount:每次删除的图片数量,fileTime:写入第一个有效文件的时间戳)
void CFileManage::RemoveFile(bool bVideo,int nMaxCount,time_t *fileTime)
{
	//如果文件序列都已清空，就不再做删除
	if ((bVideo && g_uVideoId == 0) ||
		(bVideo == false && g_uPicId == 0))
	{
		//cerr<<(bVideo?"Video":"Pic")<<" is empty"<<endl;
		return ;
	}

    unsigned int uId;
    unsigned int uMaxVideoId;
    unsigned int uMaxPicId;

    pthread_mutex_lock(&g_Id_Mutex);
    uMaxVideoId = g_uMaxVideoId;
    uMaxPicId = g_uMaxPicId;

	//if (bVideo)
	//{
	//	cerr<<"Jump! "<<m_uRemoveVideoLoopCount<<" ?< "<<m_uGetVideoIdLoopCount
	//		<<"; "<<g_uVideoId<<" ?>= "<<m_nCurDeleteVideoID<<endl;
	//}
	//else
	//{
	//	cerr<<"Jump! "<<m_uRemovePicLoopCount<<" ?< "<<m_uGetPicIdLoopCount
	//		<<"; "<<g_uPicId<<" ?>= "<<m_nCurDeletePicID<<endl;
	//}

    if(bVideo)
    {
		//第一次做遍历 || 没有做过跳转并且系统Id>=删除过的Id的情况下
        if (m_nCurDeleteVideoID == -1 ||
			m_uRemoveVideoLoopCount < m_uGetVideoIdLoopCount ||
			m_uRemoveVideoLoopCount == m_uGetVideoIdLoopCount && (long)g_uVideoId >= m_nCurDeleteVideoID)
		{
			uId = g_uVideoId+100;
			m_uRemoveVideoLoopCount = 0;
			m_uGetVideoIdLoopCount = 0;
		}
        else
		{
			uId = m_nCurDeleteVideoID;
		}
    }
    else
    {
		//第一次做遍历 || 没有做过跳转并且系统Id>=删除过的Id的情况下
		if (m_nCurDeletePicID == -1 ||
			m_uRemovePicLoopCount < m_uGetPicIdLoopCount ||
			m_uRemovePicLoopCount == m_uGetPicIdLoopCount && (long)g_uPicId >= m_nCurDeletePicID)
		{
			uId = g_uPicId+100;
			m_uRemovePicLoopCount = 0;
			m_uGetPicIdLoopCount = 0;
		}
        else
		{
			uId = m_nCurDeletePicID;
		}
    }
    pthread_mutex_unlock(&g_Id_Mutex);

	//uId+100后,如果跳转后仍然在序列之外 就从0开始删除
	long maxId = bVideo? uMaxVideoId:uMaxPicId;
	if ((long)uId - maxId >= maxId)
	{
		//cerr<<" maxId too small ";
		uId = 0;
	}
	
    int m,n,k;
    char buf[128]={0};
    int nCount = 0;
	int breakId = 0;
	bool bOnce = false;

    while(true)
    {
		//如果超过最大ID从头循环
		if(bVideo)
		{
			if(uId > uMaxVideoId)
			{
				uId = uId - uMaxVideoId -1;
				m_uRemoveVideoLoopCount++;
			}
		}
        else
		{
			if(uId > uMaxPicId)
			{
				uId = uId - uMaxPicId -1;
				m_uRemovePicLoopCount++;
			}
		}

		//cerr<<" "<<uId;
		if (bOnce == false)
		{
			breakId = uId;
			bOnce = true;
		}
		else
		{
			//文件列循环满一次(全部为空文件)，跳出循环
			if (uId == breakId)
			{
				if (bVideo)
				{
					//违章录像全部为空文件时，序列从0开始
					g_uVideoId = 0;
					m_nCurDeleteVideoID == -1;
				}
				else
				{	//图片全部为空文件时，序列从0开始
					g_uPicId = 0;
					m_nCurDeletePicID == -1;
				}
				//cerr<<(bVideo? "Video":"Pic")<<" all clear!";
				return;
			}
		}

        m = uId/10000;
        n = (uId-m*10000)/100;
        k = (uId-m*10000-n*100);

        if(bVideo)
        {
            sprintf(buf,"%s/%02d",g_strVideo.c_str(),m);
        }
        else
        {
            sprintf(buf,"%s/%02d",g_strPic.c_str(),m);
        }
        std::string strSubPath1(buf);

        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%s/%02d",strSubPath1.c_str(),n);
        std::string strSubPath2(buf);

        if(bVideo)
        {
            if(g_nEncodeFormat == 2||g_nEncodeFormat == 1)
            {
                sprintf(buf,"%s/%02d.mp4",strSubPath2.c_str(),k);
            }
            else
            {
                sprintf(buf,"%s/%02d.avi",strSubPath2.c_str(),k);
            }
        }
        else
        {
            sprintf(buf,"%s/%02d.jpg",strSubPath2.c_str(),k);
        }
        std::string strPath(buf);
        //用一张空图片覆盖下一张图片
        if(access(strPath.c_str(),F_OK) == 0)
        {
            struct stat s;
            stat(strPath.c_str(), &s);
            int nSize = s.st_size;

            if(nSize > 0)
            {
				if (fileTime)
				{
					//cerr<<strPath;
					*fileTime = s.st_mtime;
					break;
				}
                FILE* fp = fopen(strPath.c_str(),"wb");
				if (fp != NULL)
				{
					fwrite("",0,1,fp);
					fclose(fp);
				}

                if(bVideo == false)
                {
                    //删除对应的记录
                    g_skpDB.DeleteOldRecord(strPath,true,bVideo);
                }
                nCount++;
            }
        }

        if(nCount >= nMaxCount)
        {
           // sync();
            break;
        }
		uId++;
    }
	if (bVideo)
	{
		 m_nCurDeleteVideoID = uId;
	}
	else
	{
		m_nCurDeletePicID = uId;
	}
}


//取视频文件路径 格式: ./video/20081104/10/10/vod10_100.mp4
std::string CFileManage::GetVideoPath()
{
    time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
    time( &now );
    localtime_r(&now,newTime);

	// 判断目录是否存在,不存在则建立目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}


    char buf[128]={0};
    sprintf(buf,"%s/%4d%02d%02d",g_strVideo.c_str(),newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
	std::string strDayPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strDayPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strDayPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	sprintf(buf,"%s/%02d",strDayPath.c_str(),newTime->tm_hour);
	std::string strMinPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strMinPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strMinPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

    if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2)
    sprintf(buf, "%s/%02d_%02d.mp4", strMinPath.c_str(),newTime->tm_min,newTime->tm_sec);
    else
    sprintf(buf, "%s/%02d.avi", strMinPath.c_str(),newTime->tm_min);
    std::string strTime(buf);

    printf("=========H264_GetVideoPath=======strMinPath.c_str()=%s=====strTime=%s\n",strMinPath.c_str(),strTime.c_str());

    m_strH264FileName = strTime;
	return strTime;
}

/* 函数介绍：获取车牌图片路径：(格式./pic/channel_1/sub1/sub2/100_plate.jpg)
 * 输入参数：nChannel-通道编号，uPicId-图片编号
 * 输出参数：无
 * 返回值：车牌图片路径
 */
std::string CFileManage::GetPicPath()
{
    unsigned int  uPicId = g_uPicId;
     // 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

		int m = uPicId/10000;
		int n = (uPicId-m*10000)/100;
		int k = (uPicId-m*10000-n*100);

		sprintf(buf,"%s/%02d",g_strPic.c_str(),m);
		std::string strSubPath1(buf);

		sprintf(buf,"%s/%02d",strSubPath1.c_str(),n);
		std::string strSubPath2(buf);

		sprintf(buf,"%s/%02d.jpg",strSubPath2.c_str(),k);
		std::string strPicPath(buf);

		//printf("======================strPicPath=%s\n",strPicPath.c_str());

		// 判断目录是否存在,不存在则建立图片目录
		if(access(g_strPic.c_str(),0) != 0) //目录不存在
		{
			mkdir(g_strPic.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
			sync();
		}

		// 判断目录是否存在,不存在则建立第一层目录
		if(access(strSubPath1.c_str(),0) != 0) //目录不存在
		{
			mkdir(strSubPath1.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
			sync();
		}

		// 判断目录是否存在,不存在则建立第二层目录
		if(access(strSubPath2.c_str(),0) != 0) //目录不存在
		{
			mkdir(strSubPath2.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
			sync();
		}

		if(g_uMaxPicId < g_uPicId)
		{
			g_uMaxPicId = g_uPicId;
		}

		g_uPicId++;

		if(g_uPicId %100 == 0)//每100张更新一次磁盘缓冲区
		{
			sync();
		}

		return strPicPath;
}

//检测器之间通讯，获取用于存储从上一个点位发送过来的图片的路径(用于区间测速)
std::string CFileManage::GetRecvPicPath()
{
	unsigned int  uPicId = g_strMvsRecvPicId;
	// 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

	int m = uPicId/10000;
	int n = (uPicId-m*10000)/100;
	int k = (uPicId-m*10000-n*100);

	sprintf(buf,"%s/%02d",g_strMvsRecvPic.c_str(),m);
	std::string strSubPath1(buf);

	sprintf(buf,"%s/%02d",strSubPath1.c_str(),n);
	std::string strSubPath2(buf);

	sprintf(buf,"%s/%02d.jpg",strSubPath2.c_str(),k);
	std::string strPicPath(buf);

	//printf("======================strPicPath=%s\n",strPicPath.c_str());

	// 判断目录是否存在,不存在则建立图片目录
	if(access(g_strMvsRecvPic.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strMvsRecvPic.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	// 判断目录是否存在,不存在则建立第一层目录
	if(access(strSubPath1.c_str(),0) != 0) //目录不存在
	{
		mkdir(strSubPath1.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	// 判断目录是否存在,不存在则建立第二层目录
	if(access(strSubPath2.c_str(),0) != 0) //目录不存在
	{
		mkdir(strSubPath2.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	g_strMvsRecvPicId++;
	if(g_strMvsRecvPicId > 50000)//防止产生过多目录
	{
		g_strMvsRecvPicId = 0;
	}

	if(g_strMvsRecvPicId %100 == 0)//每100张更新一次磁盘缓冲区
	{
		sync();
	}

	return strPicPath;
}


/* 函数介绍：获取图片路径
* 输入参数：nType-0:卡口，1：违章，2：事件
* 输出参数：无
* 返回值：图片路径
*/
std::string CFileManage::GetSpecialPicPath(int nType)
{
	std::string strPicPath;

	// 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

	// 判断目录是否存在,不存在则建立图片目录
	if(access(g_strPic.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strPic.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}
	
	if(nType == 0)
	{
		sprintf(buf,"%s/卡口",g_strPic.c_str());
		strPicPath = (buf);
	}
	else if(nType == 1)
	{
		sprintf(buf,"%s/违章",g_strPic.c_str());
		strPicPath = (buf);
	}
	else if(nType == 2)
	{
		sprintf(buf,"%s/事件",g_strPic.c_str());
		strPicPath = (buf);
	}

	g_skpDB.UTF8ToGBK(strPicPath);

	// 判断目录是否存在,不存在则建立图片目录
	if(access(strPicPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strPicPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}
	
	return strPicPath;
}


//获取随机时间
int GetRandTime()
{
    srand( (unsigned)time( NULL ) );
    int nRandTime = 1 + (int)(600.0*rand()/(RAND_MAX+1.0)) - 300.0;
    return nRandTime;
}

//磁盘管理线程
void* ThreadDiskManage(void* pArg)
{
	//取类指针
	//CFileManage* pFileManage = (CFileManage*)pArg;
	//if(pFileManage == NULL) return pArg;
	CXmlParaUtil XmlParaUtil;
	bool bLoad = false;//当天是否载入过xml文件
    int nCount = 0;
	int nLoadCount = 0;
    int nRandTime = 0;
    int nTime = 0;
    int nMaxCount = 2880;//每4小时判断一次
	int nMaxLoadCount = 720;//每小时判断一次
	if(g_nLoadBasePlateInfo == 1 && (g_nServerType != 13))
	{
		for(int i=0; i<3; i++)//共尝试3次更新
		{
			bool bLoadOK = g_FtpCommunication.DoGet("yellowcar.xml", "yellowcar/yellowcar.xml");//开机启动时要先加载一次
			if (bLoadOK)//只有当ftp更新yellowcar.xml成功才去更新数据库
			{
				XmlParaUtil.LoadBasePlateInfo();
				g_bLoadXml = 0;//表示可以去更新数据库了，
				LogNormal("yellowcar.xml开机下载成功!");
				break;
			}
		}
	}

	while(!g_bEndThread)
	{
        if(nCount > nMaxCount)//每4小时判断一次
        {
            g_skpRoadCommunication.RoadCommunication();

            nCount = 0;
        }

		if(g_nLoadBasePlateInfo == 1 && (g_nServerType != 13))
		{
			if(nLoadCount > nMaxLoadCount)//每小时判断一次
			{
				if(g_bLoadXml == 2)//如果当天没有更新成功（或者是数据库已经在当天被更新了）
				{
					for(int i=0; i<3; i++)//共尝试3次更新
					{
						bool bLoadOK = g_FtpCommunication.DoGet("yellowcar.xml", "yellowcar/yellowcar.xml");//开机启动时要先加载一次
						if (bLoadOK)//只有当ftp更新yellowcar.xml成功才去更新数据库
						{
							XmlParaUtil.LoadBasePlateInfo();
							g_bLoadXml = 0;
							LogNormal("yellowcar.xml重新连网后下载成功!");
							break;
						}
					}
					nLoadCount = 0;
				}
			}
			nLoadCount++;
		}

        if(nCount == 0)
        {
            //数据库清理
            g_FileManage.RunManage();

            nRandTime = GetRandTime();
            nTime = 14400+nRandTime;
            nMaxCount  = nTime/5;
        }
        nCount++;

		if(g_nLoadBasePlateInfo == 1 && (g_nServerType != 13))//正常情况下每天零点更新
		{
			int nTime = GetHourAndMinute();

			if(nTime != 0)
			{
				bLoad = false;
			}
			else if(nTime == 0)//0时0分
			{
				if(bLoad == false)
				{
					nLoadCount = 0;//防止每天更新的时间点和每小时的更新的重合。
					g_bLoadXml = 2;//每天零点,在更新以前先把g_bLoadXml = 2，表示当天还没有更新成功
					for(int i=0; i<3; i++)//共尝试3次更新
					{
						bool bLoadOK = g_FtpCommunication.DoGet("yellowcar.xml", "yellowcar/yellowcar.xml");
						if (bLoadOK)//只有当ftp更新yellowcar.xml成功才去更新数据库
						{
							XmlParaUtil.LoadBasePlateInfo();
							bLoad = true;
							g_bLoadXml = 0;//更新成功了
							LogNormal("yellowcar.xml零点下载成功!");
							break;
						}
						bLoad = true;//表示当天更新了，但是并没有成功						
					}
				}
			}
		}
	
        sleep(5);
	}
	return pArg;
}

//磁盘检查并清理线程
void* ThreadCleanDsk(void* pArg)
{
	CFileManage *pCFileManage = (CFileManage*)pArg;
	while(!g_bEndThread)
	{
		pCFileManage->CleanUpDisk();
		sleep(5);
	}
	return NULL;
}

bool CFileManage::BeginManage()
{
	//线程属性
	pthread_attr_t   attr;
	//初始化
	pthread_attr_init(&attr);
	//分离线程
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	//启动磁盘管理线程
	int nret=pthread_create(&m_nThreadId,&attr,ThreadDiskManage,this);
	//成功
	if(nret!=0)
	{
		//失败
		LogError("创建磁盘管理线程失败,无法进行磁盘管理！\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
   //启动数据库优化线程
/*	nret = pthread_create(&m_nThreadTableId, &attr, ThreadOptimizeTable, NULL);
   if (nret != 0)
   {
	   //失败
	   LogError("创建数据库优化线程失败!\r\n");
	   pthread_attr_destroy(&attr);
	   return false;
   }*/
	//启动BMP文件管理线程
/*	nret = pthread_create(&m_nThreadBmpId, &attr, ThreadBmpManage, this);
	if (nret != 0)
	{
		//失败
		LogError("创建BMP文件管理线程失败!\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}*/

	//启动检查并清理磁盘的线程
	nret = pthread_create(&m_nThreadCleanDskId, &attr, ThreadCleanDsk, this);
	if (nret != 0)
	{
		//失败
		LogError("创建磁盘清理线程失败!\r\n");
		pthread_attr_destroy(&attr);
		return false;
	}
	
	pthread_attr_destroy(&attr);
	return true;
}

void CFileManage::RunManage()
{
    if(g_sysInfo.fDisk>=DISK_LIMIT_90)
    {
        //删除DB内容
        g_skpDB.DeleteOldContent();
    }
}

bool CFileManage::EndManage()
{
	return true;
}

bool CFileManage::Init()
{
     //获取最大图片编号
    g_uMaxPicId = GetMaxID(false);
    //获取最大录像编号
    g_uMaxVideoId = GetMaxID(true);

//    m_uExtendMinPicID = GetMinID(false);
	printf("======g_uMaxPicId=%d,g_uMaxVideoId=%d\n",g_uMaxPicId,g_uMaxVideoId);
	return BeginManage();
}

bool CFileManage::UnInit()
{
	return EndManage();
}


/* 函数介绍：获取事件录像路径：(格式./video/channel_1/sub1/sub2/100.avi)
 * 输入参数：uVideoId-录像编号
 * 输出参数：无
 * 返回值：事件录像路径
 */
std::string CFileManage::GetEventVideoPath(unsigned int&  uVideoId)
{
	//LogTrace("GetEventVideoPath.log", "==111=GetEventVideoPath==uVideoId=%d=\n", uVideoId);

    // 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

	int m = uVideoId/10000;
	int n = (uVideoId-m*10000)/100;
	int k = (uVideoId-m*10000-n*100);

	// 判断目录是否存在,不存在则建立目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	sprintf(buf,"%s/%02d",g_strVideo.c_str(),m);
	std::string strSubPath1(buf);

	sprintf(buf,"%s/%02d",strSubPath1.c_str(),n);
	std::string strSubPath2(buf);

    if(g_nEncodeFormat == 2||g_nEncodeFormat == 1)
    {
       sprintf(buf,"%s/%02d.mp4",strSubPath2.c_str(),k);
    }
    else
    {
       sprintf(buf,"%s/%02d.avi",strSubPath2.c_str(),k);
    }

	std::string strVideoPath(buf);
	printf("strVideoPath=%s\n",strVideoPath.c_str());

	// 判断目录是否存在,不存在则建立录像目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	// 判断目录是否存在,不存在则建立第一层目录
	if(access(strSubPath1.c_str(),0) != 0) //目录不存在
	{
		mkdir(strSubPath1.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	// 判断目录是否存在,不存在则建立第二层目录
	if(access(strSubPath2.c_str(),0) != 0) //目录不存在
	{
		mkdir(strSubPath2.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	if(g_uMaxVideoId < uVideoId)
    {
        g_uMaxVideoId = uVideoId;
    }

    uVideoId++;

	return strVideoPath;
}

/* 函数介绍：检查硬盘使用率,使图片序列或事件录像序列实现循环
* 输入参数：bVideo=0图片,bVideo=1录像;bEvent无效参数
* 输出参数：无
* 返回值：
*/
void CFileManage::CheckDisk(bool bEvent,bool bVideo)
{
	time_t now;
	time(&now);
	if(bVideo)
	{
		//当id达到序列尾 同时 上一次删除时间小于1小时 序列从头开始循环
		if(g_uVideoId > g_uMaxVideoId &&
			now - m_tDeteteVideoTime < 3600)	
		{
			g_uVideoId = 0;
			m_uGetVideoIdLoopCount++;
		}
	}
	else
	{
		//当id达到序列尾 同时 上一次删除时间小于1小时 序列从头开始循环
		if(g_uPicId > g_uMaxPicId &&
			now - m_tDetetePicTime < 3600)
		{
			g_uPicId = 0;
			m_uGetPicIdLoopCount++;
		}
	}
}

//获取指定目录下的最大编号
int CFileManage::GetMaxIDFromPath( const char* dirname)
{
   DIR* dir=NULL;
   struct dirent* ptr=NULL;
   dir = opendir(dirname);

   // 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

    int nMaxID = 0;
    int nID = 0;

    std::string strDir(dirname);

    if(dir)
    {
       while((ptr=readdir(dir))!=NULL)
       {
         if((strcmp(".",ptr->d_name)!=0)&&
             (strcmp("..",ptr->d_name)!=0))
         {
             std::string strPathName(ptr->d_name);

             if(strPathName.size() < 8)//全天录像不统计
             {
                 sprintf(buf,"%s/%s",dirname,ptr->d_name);
                 std::string strPath(buf);

                 strPath.erase(0,strDir.size()+1);

                int nPosPoint = strPath.find(".");



                if(nPosPoint == -1)
                {
                    nID = atoi(strPath.c_str());

                    //printf("strPath=%s nPosPoint=%d,nID=%d\n",strPath.c_str(),nPosPoint,nID);
                }
                else
                {
                    std::string strID = strPath.substr(0,nPosPoint);
                    nID = atoi(strID.c_str());
                }

                if(nID > nMaxID)
                {
                    nMaxID = nID;
                }
             }
         }
       }
       closedir(dir);
    }

   //printf("GetMaxIDFromPath nMaxID=%d\n",nMaxID);

	return nMaxID;
}

//获取最大图片或录像编号
UINT32 CFileManage::GetMaxID(bool bVideo)
{
    char buf[256] = {0};

    std::string strPath;
    if(bVideo)
    {
        strPath = g_strVideo;
    }
    else
    {
        strPath = g_strPic;
    }

    int nMaxID1 = GetMaxIDFromPath(strPath.c_str());

    sprintf(buf,"%s/%02d",strPath.c_str(),nMaxID1);
    std::string strPath1(buf);

    printf("GetMaxIDFromPath strPath1=%s\n",strPath1.c_str());

    int nMaxID2 = GetMaxIDFromPath(strPath1.c_str());
    sprintf(buf,"%s/%02d",strPath1.c_str(),nMaxID2);
    std::string strPath2(buf);

    printf("GetMaxIDFromPath strPath2=%s\n",strPath2.c_str());

    int nMaxID3 = GetMaxIDFromPath(strPath2.c_str());

    UINT32 nMaxID = nMaxID1*10000+nMaxID2*100+nMaxID3;

    printf("GetMaxPicID nMaxID=%u\n",nMaxID);

    return nMaxID;
}


//获取指定目录下的最小编号
int CFileManage::GetMinIDFromPath( const char* dirname,int nIndex)
{
   DIR* dir=NULL;
   struct dirent* ptr=NULL;
   dir = opendir(dirname);

   // 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

    int nMinID = 1000;
    int nID = 0;

    std::string strDir(dirname);

    if(dir)
    {
       while((ptr=readdir(dir))!=NULL)
       {
         if((strcmp(".",ptr->d_name)!=0)&&
             (strcmp("..",ptr->d_name)!=0))
         {
             std::string strPathName(ptr->d_name);

             if(strPathName.size() < 8)//全天录像不统计
             {
                 sprintf(buf,"%s/%s",dirname,ptr->d_name);
                 std::string strPath(buf);

                 strPath.erase(0,strDir.size()+1);

                int nPosPoint = strPath.find(".");



                if(nPosPoint == -1)
                {
                    nID = atoi(strPath.c_str());

                    //printf("strPath=%s nPosPoint=%d,nID=%d\n",strPath.c_str(),nPosPoint,nID);
                }
                else
                {
                    std::string strID = strPath.substr(0,nPosPoint);
                    nID = atoi(strID.c_str());
                }

                if(nIndex == 0)
                {
                    if(nID >= (g_nMaxPicFileCount/10000))
                    {
                        if(nID < nMinID)
                        {
                            nMinID = nID;
                        }
                    }
                }
                else
                {
                    if(nID < nMinID)
                    {
                       nMinID = nID;
                    }
                }
             }
         }
       }
       closedir(dir);
    }

    if(nMinID >= 1000)
    {
        nMinID = 0;
    }

	return nMinID;
}


//获取最小图片或录像编号
UINT32 CFileManage::GetMinID(bool bVideo)
{
    if(g_sysInfo_ex.fTotalDisk > g_nMaxDisk)//磁盘容量大于200G
    {
        char buf[256] = {0};

        std::string strPath;
        if(bVideo)
        {
            strPath = g_strVideo;
        }
        else
        {
            strPath = g_strPic;
        }

        int nMinID1 = GetMinIDFromPath(strPath.c_str(),0);

        sprintf(buf,"%s/%02d",strPath.c_str(),nMinID1);
        std::string strPath1(buf);

        printf("GetMinIDFromPath strPath1=%s\n",strPath1.c_str());

        int nMinID2 = GetMinIDFromPath(strPath1.c_str(),1);
        sprintf(buf,"%s/%02d",strPath1.c_str(),nMinID2);
        std::string strPath2(buf);

        printf("GetMinIDFromPath strPath2=%s\n",strPath2.c_str());

        int nMinID3 = GetMinIDFromPath(strPath2.c_str(),2);

        UINT32 nMinID = nMinID1*10000+nMinID2*100+nMinID3;

        printf("GetMinID m_uExtendMinPicID=%u\n",m_uExtendMinPicID);

        return nMinID;
    }
    else
    {
        return 0;
    }
}

//写入最旧有效文件的时间戳,bVideo:true录像,false图片;fileTime时间戳指针
void CFileManage::GetOldTime(bool bVideo, time_t *fileTime)
{
	//cerr<<"GetOldTime() in -"<<(bVideo? "Video":"Pic")<<endl;
	RemoveFile(bVideo, 1, fileTime);
	//cerr<<endl<<"GetOldTime() out -"<<(bVideo? "Video":"Pic")<<endl;
}

//取得最旧的全天录像时间
void CFileManage::GetOldDayVideoTime(time_t *fileTime)
{
	string strFileName = g_skpDB.GetOldDayVideoPath();
	if (strFileName != "")
	{
		struct stat s;
		stat(strFileName.c_str(), &s);
		if (s.st_size > 0)
		{
			*fileTime = s.st_mtime;
		}
	}
}

//定时磁盘清理
void CFileManage::CleanUpDisk()
{
	int nDiskLimit = DISK_LIMIT_90;
	if((g_nFtpServer == 1) &&(g_nServerType == 13))
	{
		nDiskLimit = 85;
	}

	if((g_nServerType == 29))
	{
		return;
	}

	if(g_sysInfo.fDisk <= nDiskLimit)
	{
		return;
	}

	if(g_nPicSaveMode != 0)
	{
		return;
	}

	//cerr<<"CleanUpDisk() in"<<endl;

	//取得硬盘状态
	char *path="/";
	if (IsDataDisk())//存在挂载硬盘并且图片不存在"/"分区下
	{
		path = "/detectdata";
	}

	struct statvfs diskstat;
	statvfs(path,&diskstat);


	UINT32 savedUsedDisk = ((float)diskstat.f_blocks - (float)diskstat.f_bavail)*4;
	UINT32 nowUsedDisk = savedUsedDisk;
	while (!g_bEndThread&&(savedUsedDisk - nowUsedDisk < 100000))	//一次删除100MB(全部删除完毕之后是否还继续删除)
	{
		time_t picTime = 0, videoTime = 0, dayVideoTime = 0;

		//取得最旧文件的修改时间
		GetOldTime(false, &picTime);
		GetOldDayVideoTime(&dayVideoTime);
		GetOldTime(true, &videoTime);

		//比较时间，取最小值
		time_t timeArr[] = {picTime, videoTime, dayVideoTime, 0};
		std::sort(&timeArr[0], &timeArr[3]);


		time_t minTime = 0;
		for (int i = 0; i < 3; i++)
		{
			if (timeArr[i] != 0)
			{
				minTime = timeArr[i];
				break;
			}
		}

		//删除最旧的文件
		if (minTime == 0)
		{
			break;
		}
		else if (minTime == picTime)
		{
			//cerr<<"Remove Pic in"<<endl;
			RemoveFile(false, 20);
			//cerr<<endl<<"Remove Pic out"<<endl;
			time(&m_tDetetePicTime);
		}
		else if (minTime == videoTime)
		{
			//cerr<<"Remove Video in"<<endl;
			RemoveFile(true, 2);
			//cerr<<endl<<"Remove Video out"<<endl;
			time(&m_tDeteteVideoTime);
		}
		else if (minTime == dayVideoTime)
		{
			g_skpDB.DeleteVideoFile(2);
		}
		statvfs(path, &diskstat);
		nowUsedDisk = ((float)diskstat.f_blocks - (float)diskstat.f_bavail)*4;
	}
	//cerr<<"CleanUpDisk() out"<<endl;
}

/*
//删除超过最大数量的图片文件(500G硬盘)
bool CFileManage::RemoveExtendFile(int nMaxCount)
{
    int m,n,k;
    unsigned int uId;
    char buf[128]={0};

    if(m_uExtendPicID > 0)
    {
        uId = m_uExtendPicID+1;
    }
    else
    {
        uId = m_uExtendMinPicID;
    }


    int nCount = 0;
    while(true)
    {

        m = uId/10000;
        n = (uId-m*10000)/100;
        k = (uId-m*10000-n*100);


        sprintf(buf,"%s/%02d",g_strPic.c_str(),m);
        std::string strSubPath1(buf);

        memset(buf, 0, sizeof(buf));
        sprintf(buf,"%s/%02d",strSubPath1.c_str(),n);
        std::string strSubPath2(buf);

        sprintf(buf,"%s/%02d.jpg",strSubPath2.c_str(),k);

        std::string strPath(buf);
        //用一张空图片覆盖下一张图片
        if(access(strPath.c_str(),F_OK) == 0)
        {
                FILE* fp = fopen(strPath.c_str(),"wb");
                fwrite("",0,1,fp);
                fclose(fp);

                //删除对应的记录
                g_skpDB.DeleteOldRecord(strPath,true,false);
        }

        m_uExtendPicID = uId;
        nCount++;

        //printf("====m_uExtendPicID=%d\n",m_uExtendPicID);

        if(k == 99)//此时需要删除上一级子文件夹
        {
            if(access(strSubPath2.c_str(),0) == 0)
            {
                sprintf(buf,"rm -rf %s",strSubPath2.c_str());
                system(buf);
            }
        }

        if((n == 99) && (k == 99))//此时需要删除上一级子文件夹
        {
            if(access(strSubPath1.c_str(),0) == 0)
            {
                sprintf(buf,"rm -rf %s",strSubPath1.c_str());
                system(buf);
            }
        }

        //到达最后一个文件则退出
        if(m_uExtendPicID == m_uExtendMaxPicID)
        {
            if(access(strSubPath1.c_str(),0) == 0)
            {
                sprintf(buf,"rm -rf %s",strSubPath1.c_str());
                system(buf);
            }

            m_uExtendMaxPicID = 0;
            break;
        }

        if(nCount >= nMaxCount)
        {
            break;
        }

        uId++;
    }

   return true;
}
*/

void * CFileManage::ThreadOptimizeTable(void *param)
{
	while(!g_bEndThread)
	{
		//LogTrace("optimize_table.log", "begin to optimize talbe...");
		string strSql = "optimize table SYSTEM_EVENT_INFO,TRAFFIC_EVENT_INFO, TRAFFIC_STATISTIC_INFO, NUMBER_PLATE_INFO;";
		MysqlDB db;
		db.initCondition(g_strDbHost,g_nDbPort,g_strDatabase,g_strUser,g_strPw);
		//LogTrace("optimize_table.log","g_strDbHost=%s,g_nDbPort=%d,g_strDatabase=%s,g_strUser=%s,g_strPw=%s\r\n",g_strDbHost.c_str(),g_nDbPort,g_strDatabase.c_str(),g_strUser.c_str(),g_strPw.c_str());
		if(db.connectDB())
		{
		//	LogTrace("optimize_table.log", "connect db success...");
			if (db.execSQL(strSql) == 0)
			{
				LogTrace("optimize_table.log", "optimize db success!");
			}
			db.closeDB();
		}
		//LogTrace("optimize_table.log", "thread optimize over.");
	  sleep(3600*24*7);//每周优化一次
	}
	pthread_exit(0);
}

int CFileManage::GetDigitalPathList(const char *dirname, vector<long> &pathList)
{
	DIR* dir=NULL;
	struct dirent* ptr=NULL;
	dir = opendir(dirname);

	if(dir)
	{
		pathList.clear();
		while((ptr=readdir(dir))!=NULL)
		{
			if (!isdigit(ptr->d_name[0]))
				continue;

			long nDir = atol(ptr->d_name);
			pathList.push_back(nDir);
		}
		closedir(dir);

       if(pathList.size() > 0)
		 std::sort(pathList.begin(), pathList.end());
	}
	return pathList.size();
}

void * CFileManage::ThreadBmpManage(void *param)
{
	int nRet;
	CFileManage *pManage = (CFileManage *)param;
	if (pManage)
	{
		LogTrace("bmp.log", "Start bmp manage thread.");
		while(!g_bEndThread)
		{
          vector<long> pathList;
		   if (pManage->GetDigitalPathList("./bmp", pathList) >= 300)
		   {
			   char chFile[128]={0};
			   sprintf(chFile, "./bmp/%ld.bmp", pathList[0]);
			   remove(chFile);
			   LogTrace("bmp.log", "remove file %s", chFile);
		   }

			usleep(1000*50);
		}
	}
	pthread_exit(&nRet);
}

//通过通道编号，取视频文件路径 格式: ./video/channel_id/20081104/10/10/vod10_100.mp4
std::string CFileManage::GetDspVideoPath(int nChannelId)
{
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r(&now,newTime);


	// 判断目录是否存在,不存在则建立目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	char buf[128]={0};
	sprintf(buf,"%s/%4d%02d%02d",\
		g_strVideo.c_str(), newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
	std::string strDayPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strDayPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strDayPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	sprintf(buf,"%s/%02d",strDayPath.c_str(),newTime->tm_hour);
	std::string strMinPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strMinPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strMinPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2)
		sprintf(buf, "%s/%02d-%02d.mp4", strMinPath.c_str(),newTime->tm_min,nChannelId);
	else
		sprintf(buf, "%s/%02d-%02d.avi", strMinPath.c_str(),newTime->tm_min,nChannelId);
	std::string strTime(buf);

	printf("=========H264_GetVideoPath=======strMinPath.c_str()=%s=====strTime=%s\n",strMinPath.c_str(),strTime.c_str());
	m_strH264FileName = strTime;
	return strTime;
}

/* 函数介绍：获取Dsp事件录像路径：(格式./video/channel_1/sub1/sub2/100.avi)
 * 输入参数：uVideoId-录像编号
 * 输出参数：无
 * 返回值：事件录像路径
 */
std::string CFileManage::GetDspEventVideoPath(unsigned int&  uVideoId)
{
	//LogTrace("GetEventVideoPath.log", "===GetEventVideoPath==uVideoId=%d=\n", uVideoId);

    // 初始化为0
	char buf[128];
	memset(buf, 0, sizeof(buf));

	int m = uVideoId/10000;
	int n = (uVideoId-m*10000)/100;
	int k = (uVideoId-m*10000-n*100);

	// 判断目录是否存在,不存在则建立目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	sprintf(buf,"%s/%02d",g_strVideo.c_str(),m);
	std::string strSubPath1(buf);

	sprintf(buf,"%s/%02d",strSubPath1.c_str(),n);
	std::string strSubPath2(buf);

    if(g_nEncodeFormat == 2||g_nEncodeFormat == 1)
    {
       sprintf(buf,"%s/%02d.mp4",strSubPath2.c_str(),k);
    }
    else
    {
       sprintf(buf,"%s/%02d.avi",strSubPath2.c_str(),k);
    }

	std::string strVideoPath(buf);
	printf("strVideoPath=%s\n",strVideoPath.c_str());

	// 判断目录是否存在,不存在则建立录像目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	// 判断目录是否存在,不存在则建立第一层目录
	if(access(strSubPath1.c_str(),0) != 0) //目录不存在
	{
		mkdir(strSubPath1.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	// 判断目录是否存在,不存在则建立第二层目录
	if(access(strSubPath2.c_str(),0) != 0) //目录不存在
	{
		mkdir(strSubPath2.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	if(g_uMaxVideoId < uVideoId)
    {
        g_uMaxVideoId = uVideoId;
    }

    uVideoId++;

	m_strDspH264FileName = strVideoPath;
	return strVideoPath;
}
//通过相机编号，取视频文件路径 格式: ./video/channel_id/20081104/10/10/11_22.mp4
std::string CFileManage::GetMulDspVideoPath(UINT32 uCamId)
{
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r(&now,newTime);

	// 判断目录是否存在,不存在则建立目录
	if(access(g_strVideo.c_str(),0) != 0) //目录不存在
	{
		mkdir(g_strVideo.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	char buf[128]={0};
	sprintf(buf,"%s/%4d%02d%02d",\
		g_strVideo.c_str(), newTime->tm_year+1900, newTime->tm_mon+1, newTime->tm_mday);
	std::string strDayPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strDayPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strDayPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	sprintf(buf,"%s/%02d",strDayPath.c_str(),newTime->tm_hour);
	std::string strMinPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strMinPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strMinPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	if(g_nEncodeFormat == 1 || g_nEncodeFormat == 2)
		sprintf(buf, "%s/%02d-%02d.mp4", strMinPath.c_str(),newTime->tm_min,uCamId);
	else
		sprintf(buf, "%s/%02d-%02d.avi", strMinPath.c_str(),newTime->tm_min,uCamId);
	std::string strTime(buf);

	printf("=========H264_GetVideoPath=======strMinPath.c_str()=%s=====strTime=%s\n",strMinPath.c_str(),strTime.c_str());

	m_strH264FileName = strTime;
	return strTime;
}

//取缓存录像路径,格式:./video/temp/00-01~59.mp4 [Hour-ChannelID]
std::string CFileManage::GetDspVideoTempPath(int nChannelId)
{
	time_t now;
	struct tm *newTime,timenow;
	newTime = &timenow;
	time( &now );
	localtime_r(&now,newTime);

	//是否是有数据盘
	bool bDataDisk = IsDataDisk();
	std::string strDisTemp = "";

	//重设录像路径
	if(bDataDisk)
	{
		strDisTemp = "/detectdata";
	}
	else
	{
		strDisTemp = "/home/road/server";
	}


	// 判断目录是否存在,不存在则建立目录
	if(access(strDisTemp.c_str(),0) != 0) //目录不存在
	{
		mkdir(strDisTemp.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	char buf[128]={0};
	sprintf(buf,"%s/temp", strDisTemp.c_str());
	std::string strTempPath(buf);

	// 判断目录是否存在,不存在则建立目录
	if(access(strTempPath.c_str(),0) != 0) //目录不存在
	{
		mkdir(strTempPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
		sync();
	}

	//g_strVideoTemp = strTempPath;

	sprintf(buf, "%s/%02d_%02d.h264", strTempPath.c_str(), newTime->tm_min, nChannelId);
	std::string strMinPath(buf);

	printf("=========H264_GetDspVideoTempPath========strMinPath=%s\n",\
		strMinPath.c_str());

	return strMinPath;
}