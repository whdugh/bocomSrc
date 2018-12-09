// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "Common.h"
#include "CommonHeader.h"
#include "FeatureSearch.h"
#include "MatchCommunication.h"
#include "XmlParaUtil.h"
#include "ximage.h"

#ifndef NOFEATURESEARCH

//特征提取线程
void* ThreadFeatureSearch(void* pArg)
{
    //取类指针
    CFeatureSearchDetect* pFeatureSearch = (CFeatureSearchDetect*)pArg;
    if(pFeatureSearch == NULL) return pArg;

    pFeatureSearch->DealFrame();
    pthread_exit((void *)0);
    return pArg;
}

//构造
CFeatureSearchDetect::CFeatureSearchDetect()
{
        //线程ID
    m_nThreadId = 0;

    //通道ID初始
    m_nChannelID = -1;
    //检测帧列表互斥
    pthread_mutex_init(&m_Frame_Mutex,NULL);
    //最多缓冲10帧
    m_nFrameSize = 10;

    m_pJpgImage = NULL;

    m_pImage = NULL;
	m_pLogoImage = NULL;
	m_pImageFrame = NULL;

    m_nDayNight = 1;

    m_searchinterface = NULL;

    m_nDeinterlace = 1;

    m_fScaleX  = 1;
    m_fScaleY  = 1;

	m_nVideoType = 0;//实时视频

	m_nInterval = 2;
	m_lCount = 0;
}

//析构
CFeatureSearchDetect::~CFeatureSearchDetect()
{
    //检测帧列表互斥
    pthread_mutex_destroy(&m_Frame_Mutex);
}

//初始化检测数据，算法配置文件初始化
bool CFeatureSearchDetect::Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst)
{
    //终止检测
    m_bTerminateThread = false;
    //通道ID
    m_nChannelID = nChannelID;
	m_uSeq = 0;

    if( (widthOrg/widthDst) == (heightOrg/heightDst) )
    {
        m_nDeinterlace = 1; //帧图像
    }
    else
    {
        m_nDeinterlace = 2; //场图像
    }

    m_pImage = cvCreateImageHeader(cvSize(widthOrg, heightOrg), 8, 3);

	m_pImageFrame = cvCreateImage(cvSize(widthOrg, heightOrg*m_nDeinterlace), 8, 3);

    m_pJpgImage = new BYTE[widthOrg*heightOrg];


	m_cvText.Init(25);

     //获取xy方向缩放比
    m_fScaleX =  (double)widthOrg/widthDst;
    m_fScaleY = (double) (heightOrg*m_nDeinterlace)/heightDst;

    m_bReloadConfig = true;

	m_farrect = cvRect(0,0,0,0);

	m_nearrect = cvRect(0,0,0,0);

    m_searchinterface = new MvPeopleSearch();

	std::string strPath = "./logo.bmp";
	if(access(strPath.c_str(),F_OK) == 0)
	{
		m_pLogoImage = cvLoadImage(strPath.c_str(),-1);
		if(m_pLogoImage != NULL)
		{
			cvConvertImage(m_pLogoImage,m_pLogoImage,CV_CVTIMG_SWAP_RB);
		}
	}

	string strInterval;
	if (GetProfileStringName("SkipPicNum", strInterval, "update.ini"))
	{
		m_nInterval = atoi(strInterval.c_str());
	}

	//启动检测帧数据处理线程
	if(!BeginDetectThread())
	{
		LogError("创建采集帧数据检测线程失败!\r\n");
		return false;
	}
	
     return true;
}

//释放检测数据，算法数据释放
bool CFeatureSearchDetect::UnInit()
{
     //设置停止标志位
    m_bTerminateThread = true;

    //停止检测帧数据处理线程
    EndDetectThread();

	//释放文本资源
	m_cvText.UnInit();

        //加锁
    pthread_mutex_lock(&m_Frame_Mutex);
    //清空检测列表
    m_FrameList.clear();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);

    if(m_pImage)
    {
        cvReleaseImageHeader(&m_pImage);
        m_pImage = NULL;
    }

	if(m_pImageFrame)
	{
		cvReleaseImage(&m_pImageFrame);
		m_pImageFrame = NULL;
	}

    if(m_pJpgImage != NULL)
    {
        delete []m_pJpgImage;
        m_pJpgImage = NULL;
    }
	
	if(m_pLogoImage != NULL)
	{
		cvReleaseImage( &m_pLogoImage );
		m_pLogoImage = NULL;
	}

    if(m_searchinterface != NULL)
    {
        m_searchinterface->mv_UnInitial();

        delete m_searchinterface;
        m_searchinterface = NULL;
    }

    return true;
}

//事件检测处理
void CFeatureSearchDetect::DealFrame()
{
    int count = 0;
    while(!m_bTerminateThread)
    {
        if(count>200)
        {
            count = 0;
        }
        if(count==0)
        {
            m_nDayNight = DayOrNight();
        }
        count++;

        //////////////////////////////////////////

        //处理一条数据
        DetectFrame();

        //1毫秒
        usleep(1000*1);
    }
}

//检测数据
bool CFeatureSearchDetect::DetectFrame()
{
    //弹出一条帧图片
    std::string result = PopFrame();
    //无检测帧
    if(result.size() == 0) return true;

    //检测图片，并提交检测结果
    return OnFrame(result);

}

//启动数据处理线程
bool CFeatureSearchDetect::BeginDetectThread()
{
    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    struct   sched_param   param;
    param.sched_priority   =   20;
    pthread_attr_setschedparam(&attr,   &param);
    //分离线程
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    //启动监控线程
    int nret=pthread_create(&m_nThreadId,&attr,ThreadFeatureSearch,this);


    //成功
    if(nret!=0)
    {
        //失败
        LogError("创建采集帧检测处理线程失败,服务无法检测采集帧！\r\n");
        pthread_attr_destroy(&attr);
        return false;
    }
    pthread_attr_destroy(&attr);
    return true;
}

//停止处理线程
void CFeatureSearchDetect::EndDetectThread()
{
    //停止线程
    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }
}

//添加一帧数据
bool CFeatureSearchDetect::AddFrame(std::string& frame)
{
    //添加到检测列表
    if(!m_bTerminateThread)
    {
        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)frame.c_str();

        //加锁
        pthread_mutex_lock(&m_Frame_Mutex);


        if(m_FrameList.size()>=m_nFrameSize)
        {
            m_FrameList.pop_front();
        }

        m_FrameList.push_back(frame);

        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);
        return true;
    }
    return false;
}

//弹出一帧数据
std::string CFeatureSearchDetect::PopFrame( )
{
    std::string response;

    // ID错误或者实例为空则返回
    if(this == NULL || m_nChannelID <= 0 )
        return response;

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);

    //判断是否有采集的数据帧
    if(m_FrameList.size() == 0)
    {
        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);
        return response;
    }

    //取最早命令
    std::list<std::string>::iterator it = m_FrameList.begin();
    //保存数据
    response = *it;
    //删除取出的采集帧
    m_FrameList.pop_front();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);
    return response;

}

//检测采集帧，并返回结果
bool CFeatureSearchDetect::OnFrame(std::string& frame)
{
	FeatureSearchHeader *pHeader = (FeatureSearchHeader*)frame.c_str();
    SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)(frame.c_str()+sizeof(FeatureSearchHeader)));
    sDetectHeader.uChannelID = m_nCameraId;
	sDetectHeader.uHeight *= m_nDeinterlace;
	if(!pHeader)
	{
		LogTrace("rc_feature.log", "OnFrame::cannot parse header!");
		return false;
	}
	//static long lCount = 0;//计数器，每隔一段时间发送图片
    // 重新加载配置
    if(m_bReloadConfig)
    {
        m_searchinterface->mv_UnInitial();

        mvSearchInterfaceParamer para;
        para.m_iface_width= sDetectHeader.uWidth;
        para.m_iface_height = sDetectHeader.uHeight;

		vector<CvPoint> ptDirection;

        //初始化车道配置
        if(!LoadRoadSettingInfo(para.v_iface_poly,para.m_iface_farrect,para.m_iface_nearrect,ptDirection))
        {
            return false;
        }
		
		LogTrace("rc_feature.log", "%d,%d,%d,%d\n",para.m_iface_farrect.width,para.m_iface_nearrect.width,para.m_iface_farrect.height, para.m_iface_nearrect.height);
		if(para.m_iface_farrect.width <= 0 || para.m_iface_nearrect.width <= 0 || para.m_iface_farrect.height <= 0|| para.m_iface_nearrect.height <= 0)
		{
			return false;
		}
       if (pHeader->sMode == 0)//使用标定提特征
       {
		   m_searchinterface->mv_SetParamer( para );
		   m_searchinterface->mv_Initial(true);
		   m_searchinterface->mv_SetRoadDirecton( ptDirection );
       }

		m_farrect = para.m_iface_farrect;

		m_nearrect = para.m_iface_nearrect;

		if(1 == g_nServerType|| g_AmsHostInfo.uHasAmsHost > 0)//应用服务器
		{
			g_MatchCommunication.OutPutCalibration(m_farrect,m_nearrect,&sDetectHeader);
		}

		m_bReloadConfig = false;
    }

	CvRect ObjRect = cvRect(0, 0, 0, 0);//目标框
	char* data = (char*)(frame.c_str() + sizeof(FeatureSearchHeader) + sizeof(SRIP_DETECT_HEADER));
	//char* data = (char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER));
	cvSetData(m_pImage,data,m_pImage->widthStep);

	cvConvertImage(m_pImage,m_pImage,CV_CVTIMG_SWAP_RB);
	/*CvRect rtPos;
	rtPos.x = 0;
	rtPos.y = m_pImage->height/2;
	rtPos.width = m_pImage->width;
	rtPos.height = m_pImage->height/8;
	cvSetImageROI(m_pImage,rtPos);
	cvSet(m_pImage, cvScalar( 255,0, 0 ));
	cvResetImageROI(m_pImage);
	static int nIndex = 0;
	char buf[64] = {0};
	sprintf(buf,"test/%d.jpg",nIndex);
	cvSaveImage(buf,m_pImage);
	nIndex++;*/



	/*if (pHeader->sMode == 1)//使用目标框提特征
	{
		mvSearchInterfaceParamer para;
		ObjRect.x = pHeader->uLeft;
		ObjRect.y = pHeader->uTop;
		ObjRect.width = pHeader->uWidth;
		ObjRect.height = pHeader->uHeight;

		para.m_iface_width = m_pImage->width;
		para.m_iface_height = m_pImage->height;

		CvPoint point = cvPoint(ObjRect.x,ObjRect.y);
		para.v_iface_poly.push_back(point);
		point = cvPoint(ObjRect.x+ObjRect.width-1,0);
		para.v_iface_poly.push_back(point);
		point = cvPoint(ObjRect.x+ObjRect.width-1,ObjRect.y+ObjRect.height-1);
		para.v_iface_poly.push_back(point);
		point = cvPoint(0,ObjRect.y+ObjRect.height-1);
		para.v_iface_poly.push_back(point);

		para.m_iface_farrect = m_farrect;
		para.m_iface_nearrect = m_nearrect;
		

		m_searchinterface->mv_SetParamer( para );
		m_searchinterface->mv_Initial(false);
		m_searchinterface->mv_SetIsObjectFeature(true);
		m_searchinterface->mv_SetEveryFrame( m_pImage, m_uSeq, &ObjRect,true );
	}
	else*/ if (pHeader->sMode == 0)
	{
		printf("before m_searchinterface mv_SetEveryFrame,m_uSeq=%d,sDetectHeader->uSeq=%d\n",m_uSeq,sDetectHeader.uSeq);

		struct timeval tv1,tv2;
		if(g_nPrintfTime == 1)
		{
				gettimeofday(&tv1,NULL);
		}

		if(m_nDeinterlace == 2)
		{
			cvResize(m_pImage,m_pImageFrame);
			m_searchinterface->mv_SetEveryFrame( m_pImageFrame, m_uSeq,NULL,false );
		}
		else
		{
			m_searchinterface->mv_SetEveryFrame( m_pImage, m_uSeq,NULL,false );
		}
		printf("after m_searchinterface mv_SetEveryFrame\n");
		if(g_nPrintfTime == 1)
		{
				gettimeofday(&tv2,NULL);
				FILE* fp = fopen("time.log","ab+");
				fprintf(fp,"mv_SetEveryFrame==t1=%lld,t2=%lld,dt = %lld\n",(tv1.tv_sec*1000000+tv1.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec),(tv2.tv_sec*1000000+tv2.tv_usec)-(tv1.tv_sec*1000000+tv1.tv_usec));
				fclose(fp);
		}
	}
	
	
    m_searchinterface->mv_ExtractFeatureFromImage();

    //特征向量
    feature* mfeature = NULL;
    m_searchinterface->mv_GetFeature( &mfeature );
     if(m_nDeinterlace == 2)
	{
		cvConvertImage(m_pImageFrame,m_pImageFrame,CV_CVTIMG_SWAP_RB);
	}
	else
	{
		cvConvertImage(m_pImage,m_pImage,CV_CVTIMG_SWAP_RB);
	}

    //输出特征结果
    if(mfeature != NULL)
    {
		if(mfeature->nBlobSize > 0 || mfeature->nSurfSize > 0 ||mfeature->nEdgeSize > 0)
		{
			if (pHeader->sMode == 0)
			{
				/*if(0 == lCount%m_nInterval)
				 OutPutResult(mfeature,&sDetectHeader, true);
				else
                 OutPutResult(mfeature,&sDetectHeader, false);

				lCount++;*/
				OutPutResult(mfeature, &sDetectHeader);
			}
			/*else
			{
				m_nInterval = 1;
				OutPutResult(mfeature, &sDetectHeader);
			}*/
		}

		m_uSeq++;
    }
    /*if (pHeader->sMode == 1)
    {
		m_searchinterface->mv_UnInitial();
    }*/

    return true;
}

//载入通道设置
bool CFeatureSearchDetect::LoadRoadSettingInfo(vector<CvPoint>& v_iface_poly,CvRect& farrect,CvRect& nearrect,vector<CvPoint> &ptDirection)
{
    LIST_CHANNEL_INFO list_channel_info;
    CXmlParaUtil xml;
    if(xml.LoadRoadSettingInfo(list_channel_info,m_nChannelID))
    {
        bool bLoadPerson = false;

        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator it_begin;
        Point32fList::iterator it_end;

        while(it_b != it_e)
        {
            int i = 0;

            CHANNEL_INFO channel_info = *it_b;

            CvPoint pt;

            //车道区域
            it_begin = channel_info.chRegion.listPT.begin();
            it_end = channel_info.chRegion.listPT.end();
            for(; it_begin != it_end; it_begin++)
            {
                pt.x = it_begin->x;
                pt.y = it_begin->y;

                v_iface_poly.push_back(pt);
            }
			
			//车道方向
			pt.x = channel_info.chProp_direction.ptBegin.x;
            pt.y = channel_info.chProp_direction.ptBegin.y;
			ptDirection.push_back(pt);

            pt.x = channel_info.chProp_direction.ptEnd.x;
            pt.y = channel_info.chProp_direction.ptEnd.y;
			ptDirection.push_back(pt);

            if(!bLoadPerson)
            {
                //远处行人框
                it_rb = channel_info.RemotePersonRegion.listRegionProp.begin();
                it_re = channel_info.RemotePersonRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    it_begin = it_rb->listPt.begin();
                    it_end = it_rb->listPt.end();
                    i = 0;
                    for( ; it_begin!=it_end; it_begin++)
                    {
                        if(i==0)
                        {
                            farrect.x = (it_begin->x);
                            farrect.y = it_begin->y;
                        }
                        else if(i == 2)
                        {
                            farrect.width = it_begin->x - farrect.x;
                            farrect.height = it_begin->y - farrect.y;
                        }

                        i++;
                    }
                }
                //近处行人框
                it_rb = channel_info.LocalPersonRegion.listRegionProp.begin();
                it_re = channel_info.LocalPersonRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    i = 0;
                    it_begin = it_rb->listPt.begin();
                    it_end = it_rb->listPt.end();
                    for( ; it_begin!=it_end; it_begin++)
                    {
                         if(i==0)
                        {
                            nearrect.x = (it_begin->x);
                            nearrect.y = it_begin->y;
                        }
                        else if(i == 2)
                        {
                            nearrect.width = it_begin->x - nearrect.x;
                            nearrect.height = it_begin->y - nearrect.y;
                        }

                        i++;

                    }
                }

                bLoadPerson = true;
            }
            it_b++;
        }

        return true;
    }
    else
    {
        printf("读取车道参数失败!\r\n");
        return false;
    }
}

//设置相机编号
void CFeatureSearchDetect::SetCameraID(int nCameraID)
{
    m_nCameraId = nCameraID;
}

//保存全景图像
int CFeatureSearchDetect::EncodeImage(IplImage *pImage)
{
    CxImage image;
    int srcstep = 0;

	image.IppEncode((BYTE*)pImage->imageData,pImage->width,pImage->height,3,&srcstep,m_pJpgImage,g_PicFormatInfo.nJpgQuality);
    
    return srcstep;
}

//输出特征结果
/*void CFeatureSearchDetect::OutPutResult(feature* mfeature,SRIP_DETECT_HEADER* sDetectHeader, bool bIsSendPic)
{
    int nImageSize = 0;
	if(m_nDeinterlace == 2)
	{
		PutTextOnImage(m_pImageFrame,sDetectHeader);
		nImageSize = EncodeImage(m_pImageFrame);
	}
	else
	{
		PutTextOnImage(m_pImage,sDetectHeader);
		nImageSize = EncodeImage(m_pImage);
	}
	
    
    string strResult;

	FEATURE_DETECT_HEADER header;
	header.uCameraID = sDetectHeader->uChannelID;
	header.uCmdID = FEATURE_INFO;
	header.uTime64 = sDetectHeader->uTime64;
	header.uWidth = sDetectHeader->uWidth;
	header.uHeight = sDetectHeader->uHeight;
	header.uPicSize = nImageSize;
	header.farrect = m_farrect;
	header.nearrect = m_nearrect;
	header.nWorkMode = m_nVideoType;
	header.uSeq = sDetectHeader->uSeq;

	//printf("=000==OutPutResult nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",mfeature->nBlobSize,mfeature->nSurfSize,mfeature->nEdgeSize);
	if (bIsSendPic)//发送图片
	{
		strResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));
		strResult.append((char*)m_pJpgImage,nImageSize);
	}
	else
	{
		header.uPicSize = 0;
		strResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));
	}
	
	//printf("=111==OutPutResult nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",mfeature->nBlobSize,mfeature->nSurfSize,mfeature->nEdgeSize);
	
	strResult.append((char*)(&(mfeature->nBlobSize)),sizeof(int));

	if(mfeature->nBlobSize > 0)
	{
		if(mfeature->fBlobFeature)
		{
			strResult.append((char*)mfeature->fBlobFeature,sizeof(blobFeatureType)*mfeature->nBlobSize);
		}
		else
		{
			return;
		}
		
	}
	//printf("=222==OutPutResult nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",mfeature->nBlobSize,mfeature->nSurfSize,mfeature->nEdgeSize);
	
	strResult.append((char*)(&(mfeature->nSurfSize)),sizeof(int));
	if(mfeature->nSurfSize > 0)
	{
		
		if(mfeature->fSurfFeature)
		{
			strResult.append((char*)mfeature->fSurfFeature,sizeof(featureType)*mfeature->nSurfSize);
		}
		else
		{
			return;
		}
	}
	
	//printf("=333==OutPutResult nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",mfeature->nBlobSize,mfeature->nSurfSize,mfeature->nEdgeSize);
	strResult.append((char*)(&(mfeature->nEdgeSize)),sizeof(int));
	if(mfeature->nEdgeSize > 0)
	{
		
		if(mfeature->fEdgeFeature)
		{
			strResult.append((char*)mfeature->fEdgeFeature,sizeof(int)*mfeature->nEdgeSize);
		}
		else
		{
			return;
		}
	}
	//printf("=444==OutPutResult nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",mfeature->nBlobSize,mfeature->nSurfSize,mfeature->nEdgeSize);

    g_MatchCommunication.AddResult(strResult);
	return;
}*/


//生成特征数据
void CFeatureSearchDetect::OutPutResult(feature* mfeature,SRIP_DETECT_HEADER* sDetectHeader)
{		
		char buf[256] = {0};
        XMLNode FeatureNode,TimeNode,PicNode,BlobNode,SurfNode,EdgeNode,ModeNode,SeqNode;
        XMLCSTR strText;

		//////////////////////////////
		if(0 == m_lCount%m_nInterval)
		{
			if(m_lCount > 0)
			{
				m_lCount = 0;
			}
			
		   m_strXmlResult.clear();
		   m_FeatureInfoNode.deleteNodeContent();

		   m_FeatureInfoNode = XMLNode::createXMLTopNode("FeatureInfo");


		   ModeNode = m_FeatureInfoNode.addChild("WorkMode");
		   sprintf(buf,"%d",m_nVideoType);
		   ModeNode.addText(buf);
		   

		    int nImageSize = 0;
			if(m_nDeinterlace == 2)
			{
				PutTextOnImage(m_pImageFrame,sDetectHeader);
				nImageSize = EncodeImage(m_pImageFrame);
			}
			else
			{
				PutTextOnImage(m_pImage,sDetectHeader);
				nImageSize = EncodeImage(m_pImage);
			}

			string strPic;
			EncodeBase64(strPic,(unsigned char*)(m_pJpgImage),nImageSize);
			PicNode = m_FeatureInfoNode.addChild("Pic");
			if(strPic.size() > 0)
				PicNode.addText(strPic.c_str());

			FEATURE_DETECT_HEADER header;
			header.uCameraID = m_nCameraId;
			header.uCmdID = FEATURE_INFO;

			m_strXmlResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));
	
	   }
		m_lCount++;
		

	    FeatureNode = m_FeatureInfoNode.addChild("Feature");

        TimeNode = FeatureNode.addChild("Time");
        string strTime = GetTime(sDetectHeader->uTime64/1000/1000);
        sprintf(buf,"%s:%03d",strTime.c_str(),(sDetectHeader->uTime64/1000)%1000);
        TimeNode.addText(buf);

		SeqNode = FeatureNode.addChild("Seq");
		sprintf(buf,"%d",m_uSeq);
		SeqNode.addText(buf);

		printf("m_uSeq=%d,sDetectHeader->uSeq=%d\n",m_uSeq,sDetectHeader->uSeq);

        #ifndef NOFEATURESEARCH 
		
		int nBlobSize = mfeature->nBlobSize;
		if(nBlobSize> 0)
		{
				unsigned char* pBlobFeature = (unsigned char*)(mfeature->fBlobFeature);
				string strBlobFeature;
				EncodeBase64(strBlobFeature,pBlobFeature,sizeof(blobFeatureType)*nBlobSize);
				BlobNode = FeatureNode.addChild("BlobFeature");
				if(strBlobFeature.size() > 0)
				BlobNode.addText(strBlobFeature.c_str());
		}
		
		
		int nSurfSize = mfeature->nSurfSize;
		if(nSurfSize> 0)
		{
				unsigned char* pSurfFeature = (unsigned char*)(mfeature->fSurfFeature);
				
				string strSurfFeature;
				EncodeBase64(strSurfFeature,pSurfFeature,sizeof(featureType)*nSurfSize);
				SurfNode = FeatureNode.addChild("SurfFeature");
				if(strSurfFeature.size() > 0)
					SurfNode.addText(strSurfFeature.c_str());
		}
		
		
		int nEdgeSize = mfeature->nEdgeSize;
		if(nEdgeSize > 0)
		{
				unsigned char* pEdgeFeature = (unsigned char*)(mfeature->fEdgeFeature);

				string strEdgeFeature;
				EncodeBase64(strEdgeFeature,pEdgeFeature,sizeof(int)*nEdgeSize);
				EdgeNode = FeatureNode.addChild("EdgeFeature");
				if(strEdgeFeature.size() > 0)
				EdgeNode.addText(strEdgeFeature.c_str());
		}
		
		printf("nBlobSize=%d,nSurfSize=%d,nEdgeSize=%d\n",nBlobSize,nSurfSize,nEdgeSize);
        #endif

		if(m_lCount == m_nInterval)
		{
			int nSize;
			XMLSTR strData = m_FeatureInfoNode.createXMLString(1, &nSize);
			if(strData)
			{
				m_strXmlResult.append(strData, sizeof(XMLCHAR)*nSize);
				freeXMLString(strData);
			}

			g_MatchCommunication.AddResult(m_strXmlResult);
		}
}

//输出特征特征标定信息
/*void CFeatureSearchDetect::OutPutCalibration(CvRect& farrect,CvRect& nearrect,SRIP_DETECT_HEADER* sDetectHeader)
{
}*/

//获取图片文件名称
string CFeatureSearchDetect::GetPicFileName(const string& strTime)
{
	char buf[256] = {0};

	string strPicPath = g_strPic;

	string strDate = GetDate(strTime);
	sprintf(buf,"%s/%s",strPicPath.c_str(),strDate.c_str());
	std::string strSubPath1(buf);


	sprintf(buf,"%s/%s",strSubPath1.c_str(),g_ServerHost.c_str());
	std::string strSubPath2(buf);


	sprintf(buf,"%s/%d",strSubPath2.c_str(),m_nCameraId);
	std::string strSubPath3(buf);
	

	string strHour = GetHMS(strTime,0);
	sprintf(buf,"%s/%s",strSubPath3.c_str(),strHour.c_str());
	std::string strSubPath4(buf);
	

	string strMinute = GetHMS(strTime,1);
	sprintf(buf,"%s/%s",strSubPath4.c_str(),strMinute.c_str());
	std::string strSubPath5(buf);
	

	string strHMS = GetHMS(strTime,2);
	sprintf(buf,"%s/%s.jpg",strSubPath5.c_str(),strHMS.c_str());
	string strPicFileName(buf);

	return strPicFileName;
}

/*
* 函数介绍：获取日期字符串
* 输入参数：时间字符串(2011-05-20 12:00:00:235)
* 输出参数：无
* 返回值 ：日期字符串(20110520)
*/
string CFeatureSearchDetect::GetDate(const string& strTime)
{
	string strDate;

	strDate.append(strTime.c_str(),4);//year
	strDate.append(strTime.c_str()+5,2);//month
	strDate.append(strTime.c_str()+8,2);//day

	return strDate;
}

/*
* 函数介绍：获取时分秒字符串
* 输入参数：时间字符串(2011-05-20 12:00:00:235)
* 输出参数：无
* 返回值 ：时分秒字符串(120000235)
*/
string CFeatureSearchDetect::GetHMS(const string& strTime,int nType)
{
	string strHMS;
	if(nType == 0)//获取小时字符串
	{
		strHMS.append(strTime.c_str()+11,2);//hour
	}
	else if(nType == 1)//获取分钟字符串
	{
		strHMS.append(strTime.c_str()+14,2);//minute
	}
	else if(nType == 2)//获取秒毫秒字符串
	{
		strHMS.append(strTime.c_str()+17,2);//second
		strHMS.append(strTime.c_str()+20,3);//misecond
	}
	return strHMS;
}

//叠加文本信息
void CFeatureSearchDetect::PutTextOnImage(IplImage*pImage,SRIP_DETECT_HEADER* sDetectHeader)
{
	wchar_t wchOut[255] = {'\0'};
	char chOut[255] = {'\0'};

	int nStartX = 0;
	int nWidth = 10;
	int nHeight = 0;

	//经过时间
	std::string strTime;
	strTime = GetTime(sDetectHeader->uTimestamp,0);
	sprintf(chOut,"抓拍时间: %s:%03d",strTime.c_str(),((sDetectHeader->uTime64)/1000)%1000);
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	nHeight += 30;
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

	//经过地点
	sprintf(chOut,"地点: %s",m_strLocation.c_str());
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	nHeight += 30;
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));
	
	//叠加LOGO信息
	PutLogoOnImage();
}

//叠加LOGO信息
void CFeatureSearchDetect::PutLogoOnImage()
{
		if(m_pLogoImage != NULL && m_pImage != NULL)
		{
			int nWidth = m_pLogoImage->width;
			int nHeight = m_pLogoImage->height;

			CvRect rect;
            rect.x = m_pImage->width - nWidth - 10;
            rect.y = 10;
            rect.width = nWidth;
            rect.height = nHeight;
			
			if(nWidth > 0 && nHeight > 0)
			{
				cvSetImageROI(m_pImage,rect);
				cvCopy(m_pLogoImage,m_pImage);
				cvResetImageROI(m_pImage);
			}
		}
}

#endif
