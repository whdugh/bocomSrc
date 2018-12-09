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
#include "FaceDetect.h"
#include <time.h>

#include "ippi.h"
#include "ippcc.h"
#include "ximage.h"
#include "XmlParaUtil.h"
#include "MatchCommunication.h"

#ifndef NOFACEDETECT

//人脸特征提取线程
void* ThreadFaceFeatureExtract(void* pArg)
{
    //取类指针
    CFaceDetect* pFaceDetect = (CFaceDetect*)pArg;
    if(pFaceDetect == NULL) return pArg;

    pFaceDetect->DealFaceResult();
    pthread_exit((void *)0);
    return pArg;
}

//人脸检测线程
void* ThreadDetectFace(void* pArg)
{
    //取类指针
    CFaceDetect* pFaceDetect = (CFaceDetect*)pArg;
    if(pFaceDetect == NULL) return pArg;

    pFaceDetect->DealFrame();
    pthread_exit((void *)0);
    return pArg;
}
//构造
CFaceDetect::CFaceDetect()
{
    //线程ID
    m_nThreadId = 0;
    //通道ID初始
    m_nChannelID = -1;
    //检测帧列表互斥
    pthread_mutex_init(&m_Frame_Mutex,NULL);
    pthread_mutex_init(&m_FaceResult_Mutex,NULL);
    //最多缓冲3帧
    m_nFrameSize = 10;

	m_nFeatureThreadId = 0;

//	m_bNight = false;

    m_nDetectTime = DETECT_AUTO;
    //默认只检测事件
    m_nDetectKind = DETECT_FACE;

    m_bEventCapture = false;
    m_eCapType = CAPTURE_NO;

    m_bConnect = false;

   m_pJpgImage = NULL;

    m_nTrafficStatTime = 60;
    m_nCaptureTime = 5;

//	m_smImageData = NULL;

    m_imgSnap = NULL;
    m_imgPreSnap = NULL;
    m_imgComposeSnap = NULL;

    m_pResultFrameBuffer = NULL;

    m_img = NULL;
    m_imgPre = NULL;
    m_pExtentRegion = NULL;

    m_pImage = NULL;
	m_pImageFrame = NULL;

    m_nExtent = 60;
    m_nWordPos = 0;
    m_nDayNight = 1;


    m_nDeinterlace = 1;

    m_fScaleX  = 1;
    m_fScaleY  = 1;

 	m_nMonitorID = 0;

    return;
}
//析构
CFaceDetect::~CFaceDetect()
{
    //检测帧列表互斥
    pthread_mutex_destroy(&m_Frame_Mutex);
    pthread_mutex_destroy(&m_FaceResult_Mutex);
    return;
}

//初始化检测数据，算法配置文件初始化
bool CFaceDetect::Init(int nChannelID,int widthOrg,int heightOrg,int widthDst ,int heightDst)
{
    printf("bool CFaceDetect::Init\n");
    //终止检测
    m_bTerminateThread = false;
    //完成检测
//	m_bFinishDetect = true;
    //通道ID
    m_nChannelID = nChannelID;

    m_vFaceTracks.clear();

    if( (widthOrg/widthDst) == (heightOrg/heightDst) )
    {
        m_nDeinterlace = 1; //帧图像
    }
    else
    {
        m_nDeinterlace = 2; //场图像
    }

    m_imgSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
    m_imgPreSnap = cvCreateImage(cvSize(widthOrg,heightOrg*m_nDeinterlace+m_nExtent),8,3);
    m_imgComposeSnap = cvCreateImage(cvSize(widthOrg,(heightOrg*m_nDeinterlace+m_nExtent)*2),8,3);

    m_img =  cvCreateImageHeader(cvSize(widthOrg, heightOrg+m_nExtent/m_nDeinterlace), 8, 3);
    m_imgPre =  cvCreateImageHeader(cvSize(widthOrg, heightOrg+m_nExtent/m_nDeinterlace), 8, 3);

    m_pImage = cvCreateImageHeader(cvSize(widthOrg, heightOrg), 8, 3);
	m_pImageFrame = cvCreateImage(cvSize(widthOrg, heightOrg*m_nDeinterlace), 8, 3);

    int nSize = m_img->widthStep*m_nExtent/m_nDeinterlace*sizeof(char);
    m_pExtentRegion  = new char[nSize];
    memset(m_pExtentRegion,0,nSize);

    m_pJpgImage = new BYTE[widthOrg*heightOrg/2];

    //分配秒图缓冲区
    nSize = sizeof(SRIP_DETECT_HEADER)+m_img->imageSize;
    printf("=================================nSize=%d\n",nSize);
    int nPlateSize = FRAMESIZE;

    //获取xy方向缩放比
    m_fScaleX =  (double)widthOrg/widthDst;
    m_fScaleY = (double) (heightOrg*m_nDeinterlace)/heightDst;

    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
        uFluxVehicle[i]=0;
    }


    m_bReloadConfig = true;
	m_bReloadConfigFeature = true;

    //文本初始化
    int nFontSize = 25;

    if(widthOrg < 1000)
    {
        nFontSize = 15;
    }
    else
    {
        nFontSize = 25;
    }
    m_cvText.Init(nFontSize);
    m_nWordPos = g_PicFormatInfo.nWordPos;//文字显示位置

    //启动检测帧数据处理线程
    if(!BeginDetectThread())
    {
        LogError("创建采集帧数据检测线程失败!\r\n");
        return false;
    }
#ifdef _DEBUG
    LogNormal("通道检测算法模块初始化完成，处理线程启动!\r\n");
#endif

    if(m_bEventCapture)
    {
        m_skpRoadRecorder.Init();
    }

    return true;
}

//释放检测数据，算法数据释放
bool CFaceDetect::UnInit()
{
    //设置停止标志位
    m_bTerminateThread = true;

    //停止检测帧数据处理线程
    EndDetectThread();

    //停止事件录象线程
    if(m_bEventCapture)
    m_skpRoadRecorder.UnInit();

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);
    //清空检测列表
    m_ChannelFrameList.clear();
    //m_ResultFrameList.clear();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);

    //释放文本资源
    m_cvText.UnInit();
	
	//删除人脸数据
	pthread_mutex_lock(&m_FaceResult_Mutex);
    m_vFaceTracks.clear();
    //解锁
    pthread_mutex_unlock(&m_FaceResult_Mutex);

    

    if(m_imgSnap)
    {
        cvReleaseImage(&m_imgSnap);
        m_imgSnap = NULL;
    }

    if(m_imgComposeSnap)
    {
        cvReleaseImage(&m_imgComposeSnap);
        m_imgComposeSnap = NULL;
    }

    if(m_imgPreSnap)
    {
        cvReleaseImage(&m_imgPreSnap);
        m_imgPreSnap = NULL;
    }

    if(m_img)
    {
        cvReleaseImageHeader(&m_img);
        m_img = NULL;
    }

    if(m_imgPre)
    {
        cvReleaseImageHeader(&m_imgPre);
        m_imgPre = NULL;
    }

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

    if(m_pExtentRegion)
    {
        delete []m_pExtentRegion;
        m_pExtentRegion = NULL;
    }

    if(m_pJpgImage != NULL)
    {
        delete []m_pJpgImage;
        m_pJpgImage = NULL;
    }


    return true;
}


//启动数据处理线程
bool CFaceDetect::BeginDetectThread()
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
    int nret=pthread_create(&m_nThreadId,&attr,ThreadDetectFace,this);


    //成功
    if(nret!=0)
    {
        //失败
        LogError("创建采集帧检测处理线程失败,服务无法检测采集帧！\r\n");
        pthread_attr_destroy(&attr);
        return false;
    }

	nret=pthread_create(&m_nFeatureThreadId,&attr,ThreadFaceFeatureExtract,this);

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
void CFaceDetect::EndDetectThread()
{
    //停止线程
    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }

	m_FaceDetection.mv_UnInit();

	//停止线程
    if(m_nFeatureThreadId != 0)
    {
        pthread_join(m_nFeatureThreadId,NULL);
        m_nFeatureThreadId = 0;
    }

	m_FaceTrackSearch.mv_UnInitial();
}

//事件检测处理
void CFaceDetect::DealFrame()
{
    printf("m_bTerminateThread=%d\n",m_bTerminateThread);
    int count = 0;
    while(!m_bTerminateThread)
    {
        //自动判断白天还是晚上
        if(m_nDetectTime == DETECT_AUTO)
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
        }
        //////////////////////////////////////////

        //处理一条数据
        DetectFrame();

        //1毫秒
        usleep(1000*1);
    }
}

//检测数据
bool CFaceDetect::DetectFrame()
{
    //弹出一条帧图片
    std::string result = PopFrame();
    //无检测帧
    if(result.size() == 0) return true;

    //检测图片，并提交检测结果
    return OnFrame(result);

}

//添加一帧数据
bool CFaceDetect::AddFrame(std::string& frame)
{
    //添加到检测列表
    if(!m_bTerminateThread)
    {
        SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)frame.c_str();

        //加锁
        pthread_mutex_lock(&m_Frame_Mutex);


        if(m_ChannelFrameList.size()>=m_nFrameSize)
        {
            m_ChannelFrameList.pop_front();
            printf("=======================exceed CFaceDetect::AddFrame\r\n");
        }

    #ifdef DEBUG_INFO
        printf("                 AddDetect %d\r\n",sDetectHeader->uChannelID);
        //		printf("                      %d\r\n",sDetectHeader->uSeq);
    #endif

        m_ChannelFrameList.push_back(frame);

        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);

        return true;
    }
    return false;
}


//弹出一帧数据
std::string CFaceDetect::PopFrame( )
{
    std::string response;

    // ID错误或者实例为空则返回
    if(this == NULL || m_nChannelID <= 0 )
        return response;

    //加锁
    pthread_mutex_lock(&m_Frame_Mutex);

    //判断是否有采集的数据帧
    if(m_ChannelFrameList.size() == 0)
    {
        //解锁
        pthread_mutex_unlock(&m_Frame_Mutex);
        return response;
    }

    //取最早命令
    ListFrame::iterator it = m_ChannelFrameList.begin();
    //保存数据
    response = *it;
    //删除取出的采集帧
    m_ChannelFrameList.pop_front();
    //解锁
    pthread_mutex_unlock(&m_Frame_Mutex);
    return response;
}

//添加人脸检测结果数据
bool CFaceDetect::AddFaceResult(vector<mv_stFaceTrack>& vFaceTracks)
{
	if(!m_bTerminateThread)
    {
		 //加锁
		pthread_mutex_lock(&m_FaceResult_Mutex);
		
		vector<mv_stFaceTrack>::iterator it = vFaceTracks.begin();
		while(it != vFaceTracks.end())
		{
			m_vFaceTracks.push_back(*it);

			it++;
		}

		 //解锁
		pthread_mutex_unlock(&m_FaceResult_Mutex);

		return true;
	}
	
	return false;
}

//弹出人脸检测结果数据
bool CFaceDetect::PopFaceResult(mv_stFaceTrack& stFaceTrackResult)
{
    //加锁
    pthread_mutex_lock(&m_FaceResult_Mutex);

    //判断是否有采集的数据帧
    if(m_vFaceTracks.size() == 0)
    {
        //解锁
        pthread_mutex_unlock(&m_FaceResult_Mutex);
        return false;
    }

    //取最早命令
    vector<mv_stFaceTrack>::iterator it = m_vFaceTracks.begin();
    //保存数据
    stFaceTrackResult = *it;
    //解锁
    pthread_mutex_unlock(&m_FaceResult_Mutex);
    return true;
}

//删除人脸检测结果数据
bool CFaceDetect::DeleteFaceResult()
{
    //加锁
    pthread_mutex_lock(&m_FaceResult_Mutex);

    //判断是否有采集的数据帧
    if(m_vFaceTracks.size() == 0)
    {
        //解锁
        pthread_mutex_unlock(&m_FaceResult_Mutex);
        return false;
    }

    //取最早命令
    vector<mv_stFaceTrack>::iterator it = m_vFaceTracks.begin();
    //保存数据
    if(it != m_vFaceTracks.end())
	{
		vector<mv_stFaceTrackNode>::iterator it_b = it->m_vTrack.begin();

		while(it_b != it->m_vTrack.end())
		{
			if(it_b->m_pFaceImage != NULL)
			{
				cvReleaseImage(&it_b->m_pFaceImage);
				it_b->m_pFaceImage = NULL;
			}
			it_b++;
		}

		if(it->m_pRepImage != NULL)
		{
			cvReleaseImage(&it->m_pRepImage);
			it->m_pRepImage = NULL;
		}

		//删除取出的采集帧
		 m_vFaceTracks.erase(it);
	}
    
    //解锁
    pthread_mutex_unlock(&m_FaceResult_Mutex);
    return true;
}

//人脸特征提取
bool CFaceDetect::DealFeatureExtract(mv_stFaceTrack& stFaceTrackResult)
{
	if(m_bReloadConfigFeature)
	{
		m_FaceTrackSearch.mv_UnInitial();

		mv_stFSParam param;
		param.m_strFaceDetectorFileName = "./FrontFaceDetector.xml";
		param.m_strASModelFileName = "./asmmodel";
		param.m_strFeatModelFileName = "./FaceFeatureModel";
		m_FaceTrackSearch.mv_Initial(param);
		m_bReloadConfigFeature = false;
	}

	vector<IplImage*> vImages;
	vector<CvRect> vFacePos;

	vector<mv_stFaceTrackNode>::iterator it = stFaceTrackResult.m_vTrack.begin();

	while(it != stFaceTrackResult.m_vTrack.end())
	{
		vImages.push_back(it->m_pFaceImage);
		vFacePos.push_back(it->m_faceRect);

		it++;
	}
	
	printf("before mv_ExtractFeatureFromTrackImages,vImages.size()=%d,vFacePos.size()=%d\n",vImages.size(),vFacePos.size());

	vector<CvRect> vRepImageRects = stFaceTrackResult.m_vRepImageRects;

	mv_stFaceFeature faceFeatures;
	if(m_FaceTrackSearch.mv_ExtractFeatureFromTrackImages(vImages,vFacePos,faceFeatures,vRepImageRects,false))
	{
		printf("after mv_ExtractFeatureFromTrackImages,faceFeatures.m_nSize=%d\n",faceFeatures.m_nSize);

		//printf("strFeature data= %x,%x,%x,%x\n", *(faceFeatures.m_pFaceFeature+4), *(faceFeatures.m_pFaceFeature+5), *(faceFeatures.m_pFaceFeature+6), *(faceFeatures.m_pFaceFeature+7));
			
		//输出人脸特征
		mv_stFaceFeatureResult faceFeaturesResult;
		faceFeaturesResult.uTime64 = stFaceTrackResult.m_nTime;
		faceFeaturesResult.pRepImage = stFaceTrackResult.m_pRepImage;
		//存第一个矩形框
		CvRect repImageRect = *(stFaceTrackResult.m_vRepImageRects.begin());
		faceFeaturesResult.repImageRect = repImageRect;
		faceFeaturesResult.stFaceFeature = faceFeatures;
		OutPutFaceFeature(faceFeaturesResult);

		if(faceFeatures.m_pFaceFeature != NULL)
		{
			delete []faceFeatures.m_pFaceFeature;
			faceFeatures.m_pFaceFeature = NULL;
		}

		return true;
	}

	printf("11 after mv_ExtractFeatureFromTrackImages,faceFeatures.m_nSize=%d\n",faceFeatures.m_nSize);
	
	return false;
}


bool CFaceDetect::DealFaceResult()
{
	int count = 0;
    while(!m_bTerminateThread)
    {
        //自动判断白天还是晚上
        if(m_nDetectTime == DETECT_AUTO)
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
        }
        //////////////////////////////////////////

        mv_stFaceTrack stFaceTrack;
		
		if(PopFaceResult(stFaceTrack))
		{
			DealFeatureExtract(stFaceTrack);

			//删除人脸轨迹中的小图
			DeleteFaceResult();

		}

        //1毫秒
        usleep(1000*1);
    }
}

//输出人脸特征结果
void CFaceDetect::OutPutFaceFeature(mv_stFaceFeatureResult& faceFeatures)
{
		char buf[256] = {0};
        XMLNode FeatureNode,TimeNode,PicNode,PosNode,ModeNode,TempNode;
        XMLCSTR strText;

		//////////////////////////////
		XMLNode FeatureInfoNode;//特征数据
		FeatureInfoNode = XMLNode::createXMLTopNode("FaceFeatureInfo");

		ModeNode = FeatureInfoNode.addChild("WorkMode");
		sprintf(buf,"%d",0);
		ModeNode.addText(buf);

		TimeNode = FeatureInfoNode.addChild("Time");
		string strTime = GetTime(faceFeatures.uTime64/1000);
		sprintf(buf,"%s:%03d",strTime.c_str(),(faceFeatures.uTime64)%1000);
		TimeNode.addText(buf);
		printf("Time=%s\n",buf);
		   
		
		int nImageSize = 0;
		CJpgToAvi JpgToAvi;
		JpgToAvi.Encode((unsigned char*)faceFeatures.pRepImage->imageData,faceFeatures.pRepImage->width,faceFeatures.pRepImage->height,&nImageSize,m_pJpgImage);
		
		/*static int nIndex = 0;
		sprintf(buf,"%d.jpg",nIndex);
		nIndex++;
		cvSaveImage(buf,faceFeatures.pRepImage);*/

		printf("faceFeatures.pRepImage->width=%d,faceFeatures.pRepImage->height=%d,nImageSize=%d\n",faceFeatures.pRepImage->width,faceFeatures.pRepImage->height,nImageSize);

		string strPic;
		EncodeBase64(strPic,(unsigned char*)(m_pJpgImage),nImageSize);
		PicNode = FeatureInfoNode.addChild("Pic");
		if(strPic.size() > 0)
			PicNode.addText(strPic.c_str());

		PosNode = FeatureInfoNode.addChild("FacePos");
		TempNode = PosNode.addChild("PosX");
		sprintf(buf,"%d",faceFeatures.repImageRect.x);
		TempNode.addText(buf);

		TempNode = PosNode.addChild("PosY");
		sprintf(buf,"%d",faceFeatures.repImageRect.y);
		TempNode.addText(buf);

		TempNode = PosNode.addChild("PosW");
		sprintf(buf,"%d",faceFeatures.repImageRect.width);
		TempNode.addText(buf);

		TempNode = PosNode.addChild("PosH");
		sprintf(buf,"%d",faceFeatures.repImageRect.height);
		TempNode.addText(buf);


	    FeatureNode = FeatureInfoNode.addChild("FaceFeature");
		string strFeature;
		if(faceFeatures.stFaceFeature.m_pFaceFeature != NULL)
		{
			EncodeBase64(strFeature,(unsigned char*)faceFeatures.stFaceFeature.m_pFaceFeature,faceFeatures.stFaceFeature.m_nSize);
		}

		if(strFeature.size() > 0)
		FeatureNode.addText(strFeature.c_str());

		FEATURE_DETECT_HEADER header;
		header.uCameraID = m_nCameraId;
		header.uCmdID = FACE_FEATURE_INFO;
			
		string strXmlResult;//特征数据
		strXmlResult.append((char*)&header,sizeof(FEATURE_DETECT_HEADER));

		int nSize;
		XMLSTR strData = FeatureInfoNode.createXMLString(1, &nSize);
		if(strData)
		{
			strXmlResult.append(strData, sizeof(XMLCHAR)*nSize);
			freeXMLString(strData);
		}
		printf("strXmlResult.size()=%d\n",strXmlResult.size());

		g_MatchCommunication.AddResult(strXmlResult);
}


//保存全景图像
int CFaceDetect::SaveImage(IplImage* pImg,std::string strPicPath,int nIndex)
{
    CxImage image;
    int srcstep = 0;

    if(image.IppEncode((BYTE*)pImg->imageData,pImg->width,pImg->height,3,&srcstep,m_pJpgImage,g_PicFormatInfo.nJpgQuality))
    {
        FILE* fp = NULL;
        if(nIndex > 0)
        {
            fp = fopen(strPicPath.c_str(),"a");//追加在上一张图的后面
        }
        else
        {
            fp = fopen(strPicPath.c_str(),"wb");
        }

		if(fp)
		{
			fwrite(m_pJpgImage,srcstep,1,fp);

			//叠加数字水印
			#ifdef WATER_MARK
			std::string strWaterMark;
			GetWaterMark(m_pJpgImage,srcstep,strWaterMark);
			fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
			srcstep += strWaterMark.size();
			#endif

			fclose(fp);
		}
    }

    return srcstep;
}

//检测采集帧，并返回结果
bool CFaceDetect::OnFrame(std::string& frame)
{
    //取类型
    SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());

	printf("CFaceDetect::OnFrame\n");

    // 重新加载配置
    if(m_bReloadConfig)
    {
        vector<CvPoint> vImagePoint;
        vector<CvPoint3D32f> vWorldPoint;
        //vector<event_param> DectorRegn;
        vector<CvPoint> pointstore;

        CvRect farrect = cvRect(0,0,0,0);
        CvRect nearrect = cvRect(0,0,0,0);
		m_vDetectRoi = cvRect(0,0,0,0);
		
		m_DetectPointList.clear();

        //初始化车道配置
       /* if(!LoadRoadSettingInfo(vImagePoint,vWorldPoint,pointstore,farrect,nearrect))
        {
            return false;
        }*/

		//printf("farrect.width=%d,nearrect.width=%d,farrect.height=%d,nearrect.height=%d",farrect.width,nearrect.width,farrect.height,nearrect.height);

		/*if(farrect.width <= 0 || nearrect.width <= 0 || farrect.height <= 0|| nearrect.height <= 0)
		{
			return false;
		}*/

		m_FaceDetection.mv_UnInit();

		mv_stFDParam param;
		param.m_nMinFaceSize = 40;
		param.m_nMaxFaceSize = 300;
		param.m_strFFaceDetectorModelFileName  = "./FrontFaceDetector.xml";
		param.m_strLPFaceDetectorModelFileName = "./LeftProfileDetectModel.xml";
		param.m_strRPFaceDetectorModelFileName = "./HS.xml";
		param.m_strLRFaceDetectorModelFileName = "./LeftRotDetectModel.xml";
		param.m_strRRFaceDetectorModelFileName = "./RightRotDetectModel.xml";
		m_FaceDetection.mv_Init(param);
       
        //重新配置完成
        m_bReloadConfig = false;
        LogNormal("通道[%d]重新加载配置\r\n",m_nChannelID);
    }
    ///////////////////////////////////
    printf("before mv_Detecteveryframe\n");
    //检测分析
    char* data = (char*)(frame.c_str() + sizeof(SRIP_DETECT_HEADER));
    cvSetData(m_pImage,data,m_pImage->widthStep);
    cvConvertImage(m_pImage,m_pImage,CV_CVTIMG_SWAP_RB);
	
	if(m_nDeinterlace == 2)
	{
		cvResize(m_pImage,m_pImageFrame);
	}

	vector<mv_stFaceTrack> vFaceTracks;

	if(m_nDeinterlace == 2)
	{
		m_FaceDetection.mv_DetectFaces(m_pImageFrame,sDetectHeader.uTime64/1000,sDetectHeader.uSeq,vFaceTracks,false);
	}
	else
	{
		m_FaceDetection.mv_DetectFaces(m_pImage,sDetectHeader.uTime64/1000,sDetectHeader.uSeq,vFaceTracks,false);
	}

    printf("after mv_Detecteveryframe,vFaceTracks.size()=%d\n",vFaceTracks.size());


    //检测完成，处理结果
	if(vFaceTracks.size()>0)
    {

	   vector<mv_stFaceTrack>::iterator it_b = vFaceTracks.begin();
       vector<mv_stFaceTrack>::iterator it_e = vFaceTracks.end();

        RECORD_EVENT event;
        std::string strPicPath,strPath,strTmpPath,strEvent,strStat;
        //
        sDetectHeader.uDetectType = SRIP_DETECT_EVENT;

        UINT32 uRoadIndex = 1;
        //事件地点
        memcpy(event.chPlace,m_strLocation.c_str(),m_strLocation.size());

        DETECT_RESULT_TYPE eType;

        while(it_b!=it_e)
        {
            //根据车道号获取车道逻辑编号

            //人脸识别结果
            {
                //行驶方向
                event.uDirection = m_nDirection;
                event.uRoadWayID = uRoadIndex;//车道编号

				CvRect repImageRect = *(it_b->m_vRepImageRects.begin());

				UINT32 uPosX = repImageRect.x + repImageRect.width/2;
				UINT32 uPosY = repImageRect.y + repImageRect.height/2;

                //存数据库的坐标需要乘缩放比
                event.uPosX = (int)uPosX;     //事件发生横坐标
                event.uPosY = (int)uPosY;     //事件发生纵坐标
                if(m_nWordPos == 1)
                event.uPosY += m_nExtent;

                event.uSpeed = 0;

                //类型
                event.uCode = DETECT_RESULT_FACE;

                event.uVideoBeginTime = it_b->m_nTime/1000-5;//事件发生前5秒
                event.uMiVideoBeginTime = 0;
                event.uVideoEndTime = it_b->m_nTime/1000 + m_nCaptureTime - 5;//
                event.uMiVideoEndTime = 0;
                event.uEventBeginTime = it_b->m_nTime/1000;
                event.uMiEventBeginTime = (it_b->m_nTime)%1000;
                event.uEventEndTime = it_b->m_nTime/1000+5;
                event.uMiEventEndTime = 0;

				event.uPicWidth = repImageRect.width;
				event.uPicHeight = repImageRect.height;

                {
					cvConvertImage(it_b->m_pRepImage,it_b->m_pRepImage,CV_CVTIMG_SWAP_RB);

					/*CvPoint center;
					center.x= repImageRect.x + repImageRect.width/2.0;
					center.y = repImageRect.y + repImageRect.height/2.0;
					int radius = 25;
					CvScalar color = cvScalar(255,0,0);
					cvCircle(it_b->m_pRepImage,center,radius,color,2);*/

                    //加锁
                    pthread_mutex_lock(&g_Id_Mutex);

                    //需要判断磁盘是否已经满
                    g_FileManage.CheckDisk(true,false);
                    //获取快照图片名称及大小
                    strPicPath = g_FileManage.GetPicPath();
                    memcpy(event.chPicPath,strPicPath.c_str(),strPicPath.size()); //事件图片路径

                    int nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,m_bEventCapture?1:0);
                    //解锁
                    pthread_mutex_unlock(&g_Id_Mutex);

                    //需要判断检测结果类型，如果是违法行为需要保存两张图片
                    //保存图片
                    event.uPicSize = SaveImage(it_b->m_pRepImage,strPicPath);
                    //删除已经存在的记录
                        g_skpDB.DeleteOldRecord(strPicPath,false,false);
                    //记录检测数据,保存事件
                    if(nSaveRet>0)
                        g_skpDB.SaveTraEvent(sDetectHeader.uChannelID,event,0,0,true);
                }
                //发往客户端的坐标需要乘缩放比
                event.uPosX = (int)uPosX/m_fScaleX;     //事件发生横坐标
                event.uPosY = (int)uPosY/m_fScaleY;     //事件发生纵坐标
				bool bShow = false;
				memcpy(event.chReserved,&bShow,sizeof(bool));
                strEvent.append((char*)&event,sizeof(RECORD_EVENT));
            }
            
            it_b++;
        }

        if(strEvent.size()>0)
            strEvent.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));


        //printf("======================================m_bConnect=%d,strEvent.size()=%d\n",m_bConnect,strEvent.size());
        //////有连接才发送
        if(m_bConnect)
        {
            //发送事件
            if(strEvent.size()>0)
                g_skpChannelCenter.AddResult(strEvent);
        }

	   //添加人脸检测结果数据供人脸特征提取线程调用
		AddFaceResult(vFaceTracks);
    }

    return true;
}


//添加事件录象缓冲数据
bool CFaceDetect::AddVideoFrame(std::string& frame)
{
    {
       if(m_bEventCapture)
        m_skpRoadRecorder.AddFrame(frame);
    }

    return true;
}

//修改事件录像
void CFaceDetect::ModifyEventCapture(bool bEventCapture)
{
    //启动事件录像线程
    if(m_bEventCapture)
    {
        m_skpRoadRecorder.UnInit();
    }

    m_bEventCapture = bEventCapture;

    if(m_bEventCapture)
    {
        m_skpRoadRecorder.Init();
    }
}

//设置白天晚上还是自动判断
void CFaceDetect::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
{
    if (dType == DETECT_AUTO)
    {
        m_nDetectTime = DETECT_AUTO;
#ifdef DEBUG_DETECT
        printf("===auto day and night\r\n");
#endif
    }
    else if (dType == DETECT_DAY)
    {
        m_nDetectTime = DETECT_DAY;
        m_bReloadConfig = true;
		m_bReloadConfigFeature = true;
        m_nDayNight =1;
    }
    else
    {
        m_nDetectTime = DETECT_NIGHT;
        m_bReloadConfig = true;
		m_bReloadConfigFeature = true;
        m_nDayNight =0;
    }
}


/* 函数介绍：在事件结果图像上叠加文本信息（日期、地点等）
 * 输入参数：uTimestamp-时间戳
 * 输出参数：无
 * 返回值：无
 */
void CFaceDetect::PutTextOnImage(IplImage* pImg,RECORD_EVENT event,SRIP_DETECT_HEADER* sPreHeader,int nIndex)
{
	if(m_nExtent <= 0)
	{
		return;
	}

    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    //判断卡口行为还是违章行为
    //int nType = (event.uCode != DETECT_RESULT_EVENT_APPEAR);
    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;

    if(pImg->width > m_imgSnap->width)
    {
        nWidth += (m_imgSnap->width)*(nIndex%2);
        if(m_nWordPos == 0)
        nHeight = (m_imgSnap->height)*(nIndex/2+1) - m_nExtent;
        else
        nHeight = (m_imgSnap->height)*(nIndex/2);
    }
    else
    {
        if(m_nWordPos == 0)
        nHeight = m_imgSnap->height-m_nExtent;
        else
        nHeight = 0;
    }
    nStartX = nWidth;
    //设备编号
    std::string strDirection = GetDirection(event.uDirection);
    sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str());//
    string strText(chOut);

    //车道编号
    if(g_PicFormatInfo.nRoadIndex == 1)
    {

        sprintf(chOut,"车道编号:%d",event.uRoadWayID);//
        string strTmp(chOut);
        strText += strTmp;
    }

    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());
    if(pImg->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
        {
            nHeight += (g_PicFormatInfo.nExtentHeight/2);
        }
    }
    m_cvText.putText(pImg, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));


    //////////////////////////////////第二行
    strText.clear();
    //经过时间
    std::string strTime;

    if(nIndex == 0)
    {
        strTime = GetTime(event.uEventBeginTime,0);
        sprintf(chOut,"时间:%s.%03d  ",strTime.c_str(),event.uMiEventBeginTime);
        string strTmp(chOut);
        strText += strTmp;
    }
    else
    {
        UINT32 uPreEventTime = sPreHeader->uTimestamp;
        strTime = GetTime(uPreEventTime,0);
        sprintf(chOut,"时间:%s.%03d  ",strTime.c_str(),((sPreHeader->uTime64)/1000)%1000);
        string strTmp(chOut);
        strText += strTmp;
    }

    //行驶速度
    if(g_PicFormatInfo.nCarSpeed == 1)
    {
        sprintf(chOut,"速度:%dkm/h  ",event.uSpeed);
        string strTmp(chOut);
        strText += strTmp;
    }

    //车身颜色
    if(g_PicFormatInfo.nCarColor == 1)
    {
        std::string strCarColor = GetObjectColor(event.uColor1);
        std::string strCarColor2 = GetObjectColor(event.uColor2);

        if(event.uColor2 == UNKNOWN)
        {
            sprintf(chOut,"颜色:%s  ",strCarColor.c_str());
        }
        else
        {
            sprintf(chOut,"颜色:%s,%s  ",strCarColor.c_str(),strCarColor2.c_str());
        }
        string strTmp(chOut);
        strText += strTmp;
    }


    nWidth = 10;
    if(pImg->width > m_imgSnap->width)
    {
        nWidth += (m_imgSnap->width)*(nIndex%2);
    }
    nStartX = nWidth;

    if(pImg->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
      nHeight += (g_PicFormatInfo.nExtentHeight/2);
    }
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());
    m_cvText.putText(pImg, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
}

//查找5秒前的图片数据
SRIP_DETECT_HEADER CFaceDetect::GetPreImage(UINT32 uTimeStamp,DETECT_RESULT_TYPE  dType)
{
    SRIP_DETECT_HEADER sPreHeader;

    return sPreHeader;
}

//设置检测类型
void CFaceDetect::SetDetectKind(CHANNEL_DETECT_KIND nDetectKind)
{
    if(m_nDetectKind != nDetectKind)
    {
        m_nDetectKind = nDetectKind;
        m_bReloadConfig = true;
		m_bReloadConfigFeature = true;
    }
}

//载入通道设置
bool CFaceDetect::LoadRoadSettingInfo(vector<CvPoint>& vImagePoint, vector<CvPoint3D32f>& vWorldPoint,vector<CvPoint>& PoinStore,CvRect& farrect,CvRect& nearrect)
{
    LIST_CHANNEL_INFO list_channel_info;
    CXmlParaUtil xml;
    if(xml.LoadRoadSettingInfo(list_channel_info,m_nChannelID))
    {
        bool bLoadCalibration = false;
        bool bLoadRoad = false;
        bool bLoadPerson = false;
		bool bLoadCardArea = false;

        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator it_begin;
        Point32fList::iterator it_end;

        Point32fList::iterator it_32fb;
        Point32fList::iterator it_32fe;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        CPoint32f pt32f;
		
		CvPoint pt,pt1,pt2;
        CvPoint ptImage;
        CvPoint3D32f ptWorld;

        CvPoint2D32f ptEvent;


        m_roadMap.clear();
        printf("list_channel_info.size=%d\n",list_channel_info.size());
        while(it_b != it_e)
        {
            //mvrgnstru rgnStru;
            //mvline  lineStru;
           // event_param paramEvent;

            int i = 0;
            int j = 0;
            printf("m_fScaleX=%f,m_fScaleY=%f,m_nDeinterlace=%d\n",m_fScaleX,m_fScaleY,m_nDeinterlace);
            CHANNEL_INFO channel_info = *it_b;

            ROAD_INDEX_INFO road_index_info;
            //车道编号
            //paramEvent.nIndex = channel_info.chProp_index.value.nValue;
            road_index_info.nVerRoadIndex = channel_info.chProp_name.value.nValue;
            //车道方向
            int nDirection = channel_info.chProp_direction.value.nValue;
            if(nDirection < 180)
            {
                road_index_info.nDirection = 0;
            }
            else
            {
                road_index_info.nDirection = 1;
            }
           // m_roadMap.insert(RoadIndexMap::value_type(paramEvent.nIndex,road_index_info));

            //车道方向中心点
            printf("channel_info.chProp_direction.point.x=%f,channel_info.chProp_direction.point.y=%f\n",channel_info.chProp_direction.point.x,channel_info.chProp_direction.point.y);

            //标定区域
            if(!bLoadCalibration)
            {
                i = 0;
                //矩形区域（4个点）
                it_begin = channel_info.calibration.region.listPT.begin();
                it_end = channel_info.calibration.region.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    //image cor
                    ptImage.x = it_begin->x;
                    ptImage.y = it_begin->y;
                    vImagePoint.push_back(ptImage);

                    //world cor
                    if(i==0)
                    {
                        ptWorld.x = 0;
                        ptWorld.y = 0;
                    }
                    else if(i==1)
                    {
                        ptWorld.x = channel_info.calibration.length;
                        ptWorld.y = 0;
                    }
                    else if(i==2)
                    {
                        ptWorld.x = channel_info.calibration.length;
                        ptWorld.y = channel_info.calibration.width;
                    }
                    else if(i==3)
                    {
                        ptWorld.x = 0;
                        ptWorld.y = channel_info.calibration.width;
                    }
                    ptWorld.z = 0;
                    vWorldPoint.push_back(ptWorld);
                    i++;
                }

                //辅助标定点
                it_begin = channel_info.calibration.listPT.begin();
                it_end = channel_info.calibration.listPT.end();

                it_32fb = channel_info.calibration.list32fPT.begin();
                it_32fe = channel_info.calibration.list32fPT.end();
                while(it_begin!=it_end&&it_32fb!=it_32fe)
                {
                    //image cor
                    ptImage.x = it_begin->x;
                    ptImage.y = it_begin->y;
                    vImagePoint.push_back(ptImage);

                    //world cor
                    ptWorld.x = it_32fb->x;
                    ptWorld.y = it_32fb->y;
                    ptWorld.z = 0;
                    vWorldPoint.push_back(ptWorld);

                    it_32fb++;
                    it_begin++;
                }
                bLoadCalibration = true;
            }

            //道路区域
            if(!bLoadRoad)
            {
                int nRoadIndex = channel_info.roadRegion.chProperty.value.nValue;
                it_begin = channel_info.roadRegion.listPT.begin();
                it_end = channel_info.roadRegion.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    ptImage.x = it_begin->x;
                    ptImage.y = it_begin->y;

                    PoinStore.push_back(ptImage);
                }
                bLoadRoad = true;
            }

            //车道方向
            //lineStru.vPList.clear();
            ptImage.x = channel_info.chProp_direction.ptBegin.x;
            ptImage.y = channel_info.chProp_direction.ptBegin.y;
            //lineStru.vPList.push_back(ptImage);
            ptImage.x = channel_info.chProp_direction.ptEnd.x;
            ptImage.y = channel_info.chProp_direction.ptEnd.y;
            //lineStru.vPList.push_back(ptImage);
            //paramEvent.oriRgn.push_back(lineStru);


            //车道区域
            /*rgnStru.pPoints.clear();
            it_begin = channel_info.chRegion.listPT.begin();
            it_end = channel_info.chRegion.listPT.end();
            for(; it_begin != it_end; it_begin++)
            {
                ptEvent.x = it_begin->x;
                ptEvent.y = it_begin->y;

                rgnStru.pPoints.push_back(ptEvent);
            }
            paramEvent.chanRgn.push_back(rgnStru);*/


			//车牌区域
            if(!bLoadCardArea)
            {
                it_rb = channel_info.carnumRegion.listRegionProp.begin();
                it_re = channel_info.carnumRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        pt.x = (int) (item_b->x + 0.5);
                        pt.y = (int) (item_b->y + 0.5);
                        if(i==0)
                        {
                            pt1.x = pt.x;
                            pt1.y = pt.y;
                            pt2.x = pt.x;
                            pt2.y = pt.y;
                        }
                        else
                        {
                            if(pt1.x>pt.x)
                            {
                                pt1.x=pt.x;
                            }
                            if(pt1.y>pt.y)
                            {
                                pt1.y=pt.y;
                            }
                            if(pt2.x<pt.x)
                            {
                                pt2.x=pt.x;
                            }
                            if(pt2.y<pt.y)
                            {
                                pt2.y=pt.y;
                            }
                        }
                        i++;

						m_DetectPointList.push_back(pt);
                    }

                    //设置车牌检测区域
                     m_vDetectRoi.x = pt1.x;
                     m_vDetectRoi.y = pt1.y;
                     m_vDetectRoi.width = pt2.x - pt1.x;
                     m_vDetectRoi.height = pt2.y - pt1.y;
                }
                bLoadCardArea = true;
            }

            if(!bLoadPerson)
            {
                //远处行人框
                it_rb = channel_info.RemotePersonRegion.listRegionProp.begin();
                it_re = channel_info.RemotePersonRegion.listRegionProp.end();
                for(; it_rb != it_re; it_rb++)
                {
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    i = 0;
                    for( ; item_b!=item_e; item_b++)
                    {
                        if(i==0)
                        {
                            farrect.x = (item_b->x);
                            farrect.y = item_b->y;
                        }
                        else if(i == 2)
                        {
                            farrect.width = item_b->x - farrect.x;
                            farrect.height = item_b->y - farrect.y;
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
                    item_b = it_rb->listPt.begin();
                    item_e = it_rb->listPt.end();
                    for( ; item_b!=item_e; item_b++)
                    {
                         if(i==0)
                        {
                            nearrect.x = (item_b->x);
                            nearrect.y = item_b->y;
                        }
                        else if(i == 2)
                        {
                            nearrect.width = item_b->x - nearrect.x;
                            nearrect.height = item_b->y - nearrect.y;
                        }

                        i++;

                    }
                }

                bLoadPerson = true;
            }


            //DectorRegn.push_back(paramEvent);

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

//获取区域中心点
void CFaceDetect::GetCenterPoint(Point32fList ptList,CPoint32f& ptCenter)
{
    if(ptList.size()>0)
    {
        Point32fList::iterator it_b = ptList.begin();
        Point32fList::iterator it_e = ptList.end();

        ptCenter.x = 0;
        ptCenter.y = 0;

        while(it_b != it_e)
        {
            CPoint32f point;

            point = *it_b;
            ptCenter.x += point.x;
            ptCenter.y += point.y;

            it_b++;
        }

        ptCenter.x = ptCenter.x/ptList.size();
        ptCenter.y = ptCenter.y/ptList.size();
    }
}

//设置相机编号
void CFaceDetect::SetCameraID(int nCameraID)
{
    m_nCameraId = nCameraID;
}

//设置Monitor编号
void CFaceDetect::SetMonitorID(int nMonitorID)
{
    m_nMonitorID = nMonitorID;
}

//设置相机类型
void CFaceDetect::SetCameraType(int  nCameraType)
{
    m_nCameraType = nCameraType;
}
#endif


