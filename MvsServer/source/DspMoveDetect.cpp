// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifdef DSPMOVEDETECT

#include "DspMoveDetect.h"
#include "Common.h"
#include "CommonHeader.h"
#include "ippi.h"
#include "ippcc.h"
#include "ximage.h"
#include "XmlParaUtil.h"

#ifndef NOPLATE
#define IJL_DIB_ALIGN (sizeof(int) - 1)

#define IJL_DIB_UWIDTH(width,nchannels) \
  ((width) * (nchannels))

#define IJL_DIB_AWIDTH(width,nchannels) \
  ( ((IJL_DIB_UWIDTH(width,nchannels) + IJL_DIB_ALIGN) & (~IJL_DIB_ALIGN)) )

#define IJL_DIB_PAD_BYTES(width,nchannels) \
  ( IJL_DIB_AWIDTH(width,nchannels) - IJL_DIB_UWIDTH(width,nchannels) )


//车牌检测线程
void* ThreadDspMoveCarnumDetect(void* pArg)
{
    CDspMoveDetect* pDspMoveDetect = (CDspMoveDetect*)pArg;

    if(pDspMoveDetect == NULL) return pArg;

    pDspMoveDetect->DealCarnumDetect();

    pthread_exit((void *)0);
    printf("=================ThreadDspMoveCarnumDetect end\n");
    return pArg;
}

//构造
CDspMoveDetect::CDspMoveDetect()
{
    //存取信号互斥
    pthread_mutex_init(&m_FrameMutex,NULL);

    //线程ID
    m_nThreadId = 0;

    m_nChannelId = -1;

    m_bConnect = false;

    m_imgSnap = NULL;

    m_imgComposeSnap = NULL;

    m_img = NULL;

    m_nDetectKind = DETECT_CARNUM;

    m_nDetectTime = DETECT_AUTO;

    m_nExtentHeight = 60;
    m_nSaveImageCount = 1;
    m_nSmallPic = 0;
    m_nWordPos = 0;
    m_nWordOnPic = 0;
    m_nDayNight = 1;

    m_nCameraID = 0;
    m_nCameraType = 0;
    m_nDetectDirection = 0;//检测方向

    m_pCurJpgImage = NULL;

    m_pComposeJpgImage = NULL;

	m_mapVTSData.clear();
}

//析构
CDspMoveDetect::~CDspMoveDetect()
{
    //存取信号互斥
    pthread_mutex_destroy(&m_FrameMutex);

}

bool CDspMoveDetect::Init(int nChannelId,UINT32 uWidth,UINT32 uHeight,int nWidth, int nHeight)
{
    printf("=====CDspMoveDetect::Init()====\n");
    printf("==nChannelId=%d==uWidth=%d==uHeight=%d=nWidth=%d==nHeight=%d=\n", nChannelId, uWidth, uHeight, nWidth, nHeight);
    m_bReloadROI = true;

    m_bInitCarNumLib = 0;

    m_nChannelId = nChannelId;

    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxSmall[i]=0;
        uFluxMiddle[i]=0;
        uFluxBig[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
    }


    m_nExtentHeight = g_PicFormatInfo.nExtentHeight;

    m_nSaveImageCount = g_nSaveImageCount;
    m_nSmallPic = g_PicFormatInfo.nSmallPic;
    m_nWordPos = g_PicFormatInfo.nWordPos;
    m_nWordOnPic = g_PicFormatInfo.nWordOnPic;

    //文本初始化
    int nFontSize = 25;
    if(uWidth < 1000)
    {
       nFontSize = 15;
    }
    else if(uWidth < 2000)
    {
       nFontSize = g_PicFormatInfo.nFontSize;
    }
    else
    {
        if(m_nExtentHeight < 80)
        {
			if(g_nServerType == 4)//江宁电警
			{
				m_nExtentHeight = 100;
				nFontSize = 50;
			}
			else
			{
				nFontSize = g_PicFormatInfo.nFontSize;
			}
        }
        else
        {
            m_nExtentHeight = 80;
            nFontSize = 40;
        }
    }


        m_ratio_x = 1.0;
        m_ratio_y = 1.0;


		m_pCurJpgImage = new BYTE[uWidth*uHeight/4];

        m_pComposeJpgImage = new BYTE[uWidth*uHeight];

        printf("=====m_img==cvCreateImageHeader==uWidth=%d=uHeight=%d==\n", uWidth, uHeight);
		//m_img = cvCreateImageHeader(cvSize(uWidth,uHeight),8,3);
		m_img = cvCreateImage(cvSize(uWidth,uHeight),8,3);

        m_imgSnap = cvCreateImage(cvSize(uWidth,uHeight+m_nExtentHeight),8,3);


		if(m_imgComposeSnap == NULL)
       {
                int uWidth = m_imgSnap->width;
                int uHeight = m_imgSnap->height;

                if(m_nWordOnPic == 1)
                {
                    uHeight -= m_nExtentHeight;
                }

                //uHeight += m_nExtentHeight;

                printf("===CDspMoveDetect::Init()=m_nWordOnPic=%d=g_nVtsPicMode=%d==uWidth=%d=uHeight=%d===\n", g_nVtsPicMode, uWidth, uHeight);
                //=CDspMoveDetect::Init()===g_nVtsPicMode=0==uWidth=2448=uHeight=2128===

                //闯红灯检测三帧合成图像
				if(g_nVtsPicMode == 1) //2x2
                {
                    m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*2),8,3);
                }
                else if(g_nVtsPicMode == 2) //3x2
                {
                    m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*3),8,3);
                }
				else
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth,(uHeight)*3),8,3);
				}

				printf("m_nWordOnPic=%d,m_imgComposeSnap->height=%d,uHeight=%d,m_nExtentHeight=%d\n",m_nWordOnPic,m_imgComposeSnap->height,uHeight,m_nExtentHeight);
        }

    m_cvText.Init(nFontSize);

    m_cvBigText.Init(80);


    printf("===m_bReloadROI=%d====\n", m_bReloadROI);

    //重读车牌检测区域
    if(m_bReloadROI)
    {
        //读取车道标定信息
        vector<mvvideostd> vListStabBack;

        bool bLoadReadSetting = LoadRoadSettingInfo(1,vListStabBack);
        printf("====CDspMoveDetect::OnDspDetect===bLoadReadSetting=%d====\n", bLoadReadSetting);
        if(!bLoadReadSetting)
        {
			printf("CDspMoveDetect::OnDspDetect==== nDetect LoadRoadSettingInfo error \n");
            return false;
        }

        printf("=========CDspMoveDetect::OnDspDetect==333333===m_nDetectKind=%x==\n", m_nDetectKind);

		//线圈测速
        //if(((m_nDetectKind&DETECT_LOOP)==DETECT_LOOP))
        {
            printf("====DETECT_LOOP===\n");
			m_LoopParaMap.clear();

            CXmlParaUtil xml;
            xml.LoadLoopParameter(m_LoopParaMap,m_nChannelId);

            printf("=CDspMoveDetect::Init==m_nChannelId=%d===m_LoopParaMap.size()=%d==\n", m_nChannelId, m_LoopParaMap.size());
        }

        printf("=after==DETECT_LOOP==\n");

		//电警
		if(((m_nDetectKind&DETECT_VTS)==DETECT_VTS))
		{
			VTS_GLOBAL_PARAMETER vtsGlobalPara;

            CXmlParaUtil xml;
            xml.LoadVTSParameter(m_vtsObjectParaMap,m_nChannelId,vtsGlobalPara);
            m_vtsGlobalPara = vtsGlobalPara;
		}

		printf("=after==DETECT_VTS==\n");

		if(m_bInitCarNumLib == 1)
        {
            mvcarnumdetect.carnum_quit();
            m_bInitCarNumLib = 0;
        }

        //不同的场景载入不同的boost
        char buf[64] = {'\0'};
        sprintf(buf,"./BocomMv/%d",1);

printf("====before==mvcarnumdetect.carnum_init===\n");
        m_bInitCarNumLib = mvcarnumdetect.carnum_init(buf,homography_image_to_world,m_imgSnap->width,m_imgSnap->height-m_nExtentHeight);
printf("====after==mvcarnumdetect.carnum_init===\n");



        //计算标定
		if(g_nDetectMode == 2)
		{
			for(int i = 0; i<12; i++)
			{
				if(i%2==0)
				{
					//m_image_cord[i] *= m_fScaleX;
					m_image_cord[i] *= 1.0f;
				}
				else if(i%2==1)
				{
					//m_image_cord[i] *= m_fScaleY;
					m_image_cord[i] *= 1.0f;
				}
			}
		}
        mvfind_homography(m_image_cord,m_world_cord);//基于帧图象


        //设置车型检测标定
        m_vehicleClassify.mvSetTSAIData(m_imgSnap->width,m_imgSnap->height,6,m_image_cord, m_world_cord);

        //颜色库标定设置

        m_carColor.color_Destroy(); //
        m_carColor.color_Init( homography_image_to_world, m_nCameraType,m_imgSnap->width,m_imgSnap->height,0 );

        m_carLabel.carLabel_Destroy();
        bool bInitCarLabel = ((m_nDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE);
        m_carLabel.carLabel_Init( m_imgSnap->width,m_imgSnap->height,bInitCarLabel );

        m_bReloadROI = false;
    }

    printf("=before=BeginCarnumDetect==\n");

    //创建车牌及目标检测线程
    if(!BeginCarnumDetect())
    {
        return false;
    }

	m_mapVTSData.clear();

    return true;
}

bool CDspMoveDetect::BeginCarnumDetect()
{
    printf("======BeginCarnumDetect========\n");
    //线程结束标志
    m_bEndDetect = false;

    //线程属性
    pthread_attr_t   attr;
    //初始化
    pthread_attr_init(&attr);
    struct   sched_param   param;

    param.sched_priority   =   20;
    pthread_attr_setschedparam(&attr,   &param);
    //分离线程
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	//启动车牌检测线程
    int nret=pthread_create(&m_nThreadId,&attr,ThreadDspMoveCarnumDetect,this);

    //成功
    if(nret!=0)
    {
        //失败
        LogError("创建车牌检测线程失败,服务无法检测车牌！\r\n");
        return false;
    }

    pthread_attr_destroy(&attr);


    return true;
}

bool CDspMoveDetect::EndCarnumDetect()
{
    //线程结束标志
    m_bEndDetect = true;

    if(m_nThreadId != 0)
    {
        pthread_join(m_nThreadId,NULL);
        m_nThreadId = 0;
    }


    printf("EndCarnumDetect\n");
    //退出车牌检测程序
    if(m_bInitCarNumLib == 1)
    {
        mvcarnumdetect.carnum_quit();
        m_bInitCarNumLib = 0;
    }
    //释放文本资源
    m_cvText.UnInit();

    m_cvBigText.UnInit();

    //释放颜色库
    m_carColor.color_Destroy();
    m_carLabel.carLabel_Destroy();



	if(m_pCurJpgImage)
    {
        delete []m_pCurJpgImage;
        m_pCurJpgImage = NULL;
    }
    /*if(m_pComposeJpgImage)
    {

        delete []m_pComposeJpgImage;
        m_pComposeJpgImage = NULL;
    }*/
    //try()
    {
          printf("=================11111\n");
          if(m_img != NULL)
		  {
            //cvReleaseImageHeader(&m_img);
            cvReleaseImage(&m_img);
            m_img = NULL;
          }
         printf("=================777777777777\n");
        if(m_imgComposeSnap != NULL)
        {
            cvReleaseImage(&m_imgComposeSnap);
            m_imgComposeSnap = NULL;
        }
        if(m_imgSnap != NULL)
        {
            cvReleaseImage(&m_imgSnap);
            m_imgSnap = NULL;
        }
        //printf("EndCarnumDetect 11111111\n");
        /////////
    }

    return true;
}


//添加图像数据帧(pBuffer由3部分构成：yuv_video_buf+Image_header_dsp+imagedata)
bool  CDspMoveDetect::AddFrame(BYTE* pBuffer)
{
	if(!m_bEndDetect)
	{
        //加锁
		pthread_mutex_lock(&m_FrameMutex);

	    //printf("===in===CDspMoveDetect::AddFrame==\n");

	    yuv_video_buf* pHeader = (yuv_video_buf*)pBuffer;
		string strPic;
		if(pHeader->size > 0)
		{
		    //printf("==1111=----CDspMoveDetect::AddFrame===pHeader->size=%d====\n", pHeader->size);
		    strPic.append((char*)pBuffer, sizeof(yuv_video_buf)+pHeader->size);
		}
		else
		{
		    //printf("==2222=-----CDspMoveDetect::AddFrame===pHeader->size=%d====\n", pHeader->size);
		    return false;
		}

		//for test BEGIN
		/*
		printf("==111==for test BEGIN==m_mapVTSData.size()=%d=pHeader->nSeq=%d==\n", m_mapVTSData.size(), pHeader->nSeq);
		for(mapPicData::iterator it_b = m_mapVTSData.begin(); it_b != m_mapVTSData.end(); it_b++)
		{
		    if( it_b->second.size() > 0)
		    {
		        printf("==it_b->second.size()=%d==\n", it_b->second.size());
		        for(listPicData::iterator it_list_b = it_b->second.begin(); it_list_b != it_b->second.end(); it_list_b++)
		        {
		            Image_header_dsp* pDspImageHeader = (Image_header_dsp*)(it_list_b->c_str()+sizeof(yuv_video_buf));
                    printf("=pDspImageHeader->nSeq=%d=pDspImageHeader->nCount=%d==pDspImageHeader->nType=%d==\n", \
                           pDspImageHeader->nSeq, pDspImageHeader->nCount, pDspImageHeader->nType);
		        }
		    }
		}
		printf("==111==for test END==m_mapVTSData.size()=%d==\n", m_mapVTSData.size());
		*/
		//for test END

		Image_header_dsp* pDspImageHeader = (Image_header_dsp*)(pBuffer+sizeof(yuv_video_buf));
		//printf("==CDspMoveDetect::AddFrame==pDspImageHeader->nCount=%d====\n", pDspImageHeader->nCount);
		/*
		Picture_Key PicKey;
		PicKey.uSeq = pDspImageHeader->nSeq;
		PicKey.uCameraId = pDspImageHeader->uCameraId;
		*/

		/*mapSeqPicData::iterator it = m_mapVTSData.find(pDspImageHeader->nSeq);
		if(it != m_mapVTSData.end())
		{
		    //ERROR!! 此处发现有超过40个的链表长度（对应nCount号相同）
			it->second.push_back(strPic);
			//printf("=###=2222====AddFrame=it->second.size()=%d===\n", it->second.size());
			printf("=###=2222====AddFrame=\n");
		}*/
		//else
		//{
			listPicData listPic;
			listPic.push_back(strPic);
			Picture_Key PicKey;
			PicKey.uSeq = pDspImageHeader->nSeq;
			PicKey.uCameraId = pDspImageHeader->uCameraId;

			//printf("==CDspMoveDetect::AddFrame=pDspImageHeader->nCount=%d====listPic.size()=%d===\n", pDspImageHeader->nCount, listPic.size());
			m_mapVTSData.insert(mapPKPicData::value_type(PicKey,listPic));
			printf("=###=3333====AddFrame=\n");
		//}

		//printf("==CDspMoveDetect::AddFrame====m_mapVTSData.size()=%d===\n", m_mapVTSData.size());

		//for test BEGIN
		/*
		printf("==222==for test BEGIN==m_mapVTSData.size()=%d==\n", m_mapVTSData.size());
		for(mapPicData::iterator it_b = m_mapVTSData.begin(); it_b != m_mapVTSData.end(); it_b++)
		{
		    if( it_b->second.size() > 0)
		    {
		        printf("==it_b->second.size()=%d==\n", it_b->second.size());
		        for(listPicData::iterator it_list_b = it_b->second.begin(); it_list_b != it_b->second.end(); it_list_b++)
		        {
		            Image_header_dsp* pDspImageHeader = (Image_header_dsp*)(it_list_b->c_str()+sizeof(yuv_video_buf));
                    printf("=pDspImageHeader->nSeq=%d=pDspImageHeader->nCount=%d==pDspImageHeader->nType=%d==\n", \
                           pDspImageHeader->nSeq, pDspImageHeader->nCount, pDspImageHeader->nType);
		        }
		    }
		}
		printf("==222==for test END==m_mapVTSData.size()=%d==\n", m_mapVTSData.size());
		*/
		//for test END

		//解锁
		pthread_mutex_unlock(&m_FrameMutex);

		return true;
	}
	return false;
}


//从待检队列中弹出一帧数据
int CDspMoveDetect::PopFrame(listPicData& listPic)
{
    //printf("=in==CDspMoveDetect::PopFrame===\n");
    int nSize = 0;
    //加锁
    pthread_mutex_lock(&m_FrameMutex);

        if(m_mapVTSData.size() > 0)
        {
            //printf("======m_mapVTSData.size()=%d====\n", m_mapVTSData.size());
			mapPKPicData::iterator it = m_mapVTSData.begin();

			//printf("====PopFrame it->second.size()=%d\n", it->second.size());

			while(it != m_mapVTSData.end())
			{
			    //printf("==== CDspMoveDetect::PopFrame 11111111====\n");
			    //printf("==111==it->second.size()=%d===\n", it->second.size());

			    bool bFind = ToBeOutPut(it->second);

				//if(it->second.size() >= 4)
				if(it->second.size() > 0)
				{
				    /*
				    printf("==== CDspMoveDetect::PopFrame 1111----2222====\n");
					bool bFind = false;

					listPicData::iterator it_b = it->second.begin();
					listPicData::iterator it_e = it->second.end();
					it_e--;

					//while(it_b != it_e)
					while(it_b != it_e)
					{
					    if(it_e == NULL)
					    {
					        return 0;
					    }

					    printf("==== CDspMoveDetect::PopFrame 22222222====\n");
						//Image_header_dsp* pDspImageHeader = (Image_header_dsp*)(it_b->c_str()+sizeof(yuv_video_buf));
						Image_header_dsp* pDspImageHeader = (Image_header_dsp*)(it_e->c_str()+sizeof(yuv_video_buf));

						printf("=111==nSeq=%d=pDspImageHeader->nSize=%d===\n", pDspImageHeader->nSeq, pDspImageHeader->nSize);

						printf("=22==88888888###===pDspImageHeader->nSeq=%d, pDspImageHeader->nCount=%d, pDspImageHeader->nOrder=%d=pDspImageHeader->nType=%d=\n", \
                                pDspImageHeader->nSeq, pDspImageHeader->nCount, pDspImageHeader->nOrder, pDspImageHeader->nType);

						if(pDspImageHeader->nType == 3)//卡口图片
						{
						    printf("======KAKOU==pDspImageHeader->nType=%d=\n", pDspImageHeader->nType);
							if(it->second.size() == 2)//取出结果
							{
								bFind = true;
								break;
							}
						}
						else if(pDspImageHeader->nType == 4 || pDspImageHeader->nType == 5)//电警图片
						{
						    printf("======DIANJING==pDspImageHeader->nType=%d=\n", pDspImageHeader->nType);
							if(it->second.size() >= 4)//取出结果
							{
								bFind = true;
								break;
							}
						}
						else
						{
							printf("=======NOT===KAKOU====DIANJING====\n");
						}

						it_e--;
					}//End of while
                    */

					if(bFind)
					{
					    listPicData::iterator it_b = it->second.begin();
                        listPicData::iterator it_e = it->second.end();

						//取出结果
						it_b = it->second.begin();
						it_e = it->second.end();

						while(it_b != it_e)
						{
						    printf("==== CDspMoveDetect::PopFrame 3333333====\n");
							Image_header_dsp* pDspImageHeader = (Image_header_dsp*)(it_b->c_str()+sizeof(yuv_video_buf));

							if(pDspImageHeader->nType == DSP_IMG_LOOP_INFO)//线圈触发信息
							{
								//listLoop.push_back(*it_b);
							}
							else//图片信息
							{
								listPic.push_back(*it_b);
							}

							it_b++;
						}
						printf("====CDspMoveDetect::PopFrame444444444====\n");

						it->second.clear();

						m_mapVTSData.erase(it);

						nSize = 1;

						break;
					}

				}//End of if(it->second.size() >= 2)

				//printf("==== CDspMoveDetect::PopFrame 55555====\n");

				it++;
			}//End of while(it != m_mapVTSData.end())

			//printf("==== CDspMoveDetect::PopFrame 66666====\n");


			/*if(m_mapVTSData.size() > 60)
			{
				it = m_mapVTSData.begin();

				if(it->second.size() <= 1)
				{
					it->second.clear();
					m_mapVTSData.erase(it);

					LogNormal("累积数据过多,取消输出=m_mapVTSData.size()=%d", m_mapVTSData.size());
				}
			}
			*/
        }

    if(nSize > 0)
    {
        printf("==CDspMoveDetect::PopFrame====listPic.size()=%d===\n", listPic.size());
    }
    //解锁
    pthread_mutex_unlock(&m_FrameMutex);

    //printf("=out==CDspMoveDetect::PopFrame===\n");
    return nSize;
}


//车牌检测处理
void CDspMoveDetect::DealCarnumDetect()
{
    //初始化各个车道检测区域大小
    int count = 0;
    while(!m_bEndDetect)
    {
        //自动判断白天还是晚上
        if(m_nDetectTime == DETECT_AUTO)
        {
            if(count>1000)
            {
                count = 0;
            }
            if(count==0)
            {
                m_nDayNight = DayOrNight();
            }
            count++;
        }
        ///////////////////

		listPicData listPic;
		//listPicData listLoop;

        //printf("=DealCarnumDetect==Before PopFrame======\n");

        int nSize = PopFrame(listPic);

        if(nSize>0)
        {
            //LogNormal("=After PopFrame==nSize=%d---===\n", nSize);
          OnDspDetect(listPic);
        }
		else
		{
			usleep(1000);
		}

        //usleep(1000*500);
    }

    return;
}




//电警叠加文本信息
void CDspMoveDetect::PutVtsTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp,PLATEPOSITION* pSignalTimeStamp)
{
    //for test...
    //return;

    printf("===in==CDspMoveDetect::PutVtsTextOnImage===\n");
    if(pTimeStamp != NULL)
    {

        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nStartX = 0;
        int nWidth = 10;
        int nHeight = 0;

printf("=====111111===\n");

        if(m_nWordOnPic == 1) //针对江宁项目，字直接叠加在图上
        {
            /*
            if(pImage->width > m_imgSnap->width)
            {
                nWidth += (m_imgSnap->width)*(nIndex%2);
                nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex/2);

                //经过地点,行驶方向,车道编号
                std::string strDirection = GetDirection(plate.uDirection);
                sprintf(chOut,"%s %s 车道%d",m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID);
                memset(wchOut,0,sizeof(wchOut));
                UTF8ToUnicode(wchOut,chOut);
                nHeight += (m_nExtentHeight/2);
                m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(255,255,255));


                //经过时间(第二行)
                std::string strTime;
                strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
                sprintf(chOut,"抓拍时间: %s:%03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
                memset(wchOut,0,sizeof(wchOut));
                UTF8ToUnicode(wchOut,chOut);
                nHeight += (m_nExtentHeight/2);
                m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

                //红灯时间(第三行)
                if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
                {
                    UINT32 uTimestamp = pSignalTimeStamp->uTimestamp;
                    strTime = GetTime(uTimestamp,0);
                    sprintf(chOut,"红灯时间: %s:%03d",strTime.c_str(),(((pSignalTimeStamp)->ts)/1000)%1000);
                    memset(wchOut,0,sizeof(wchOut));
                    UTF8ToUnicode(wchOut,chOut);
                    nHeight += (m_nExtentHeight/2);
                    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
                }
            }
            else//天津电警
            {
				CvxText cvText;
				cvText.Init(50);

                nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex);

                //经过时间(第一行)
                std::string strTime;
                strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
                sprintf(chOut,"捕获时间: %s %03d",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
                memset(wchOut,0,sizeof(wchOut));
                UTF8ToUnicode(wchOut,chOut);
                nHeight += (100/2);
                cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));


                if(nIndex == 2)
                {
                     //经过地点
                    sprintf(chOut,"%s",m_strLocation.c_str());
                    memset(wchOut,0,sizeof(wchOut));
                    UTF8ToUnicode(wchOut,chOut);
                    nHeight += (100/2);
                    cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(255,255,255));

                      //行驶方向
                    std::string strDirection = GetDirection(plate.uDirection);
                    sprintf(chOut,"%s",strDirection.c_str());
                    memset(wchOut,0,sizeof(wchOut));
                    UTF8ToUnicode(wchOut,chOut);
                    nHeight += (100/2);
                    cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(255,255,255));

                    //红灯时间
                    if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
                    {
                        UINT32 uTimestamp = pSignalTimeStamp->uTimestamp;
                        strTime = GetTime(uTimestamp,0);
						int nMiTime = (((pSignalTimeStamp)->ts)/1000)%1000;
                        sprintf(chOut,"红灯开始时间: %s %03d",strTime.c_str(),nMiTime);
                        memset(wchOut,0,sizeof(wchOut));
                        UTF8ToUnicode(wchOut,chOut);
                        nHeight += (100/2);
                        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

                        strTime = GetTime(pSignalTimeStamp->uTimestamp+m_vtsGlobalPara.nRedLightTime,0);
                        sprintf(chOut,"红灯结束时间: %s %03d",strTime.c_str(),(nMiTime+6)%1000);
                        memset(wchOut,0,sizeof(wchOut));
                        UTF8ToUnicode(wchOut,chOut);
                        nHeight += (100/2);
                        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
                    }
                }

				cvText.UnInit();
            }
            */
        }
        else
        {
            printf("====###22233333==========\n");
            if(pImage->width > m_imgSnap->width)
            {
                nWidth += (m_imgSnap->width)*(nIndex%2);
                if(m_nWordPos == 0)
                {
                    nHeight = (m_imgSnap->height)*(nIndex/2+1) - m_nExtentHeight;
                }
                else
                {
                    nHeight = (m_imgSnap->height)*(nIndex/2);
                }
            }
            else
            {
                if(pImage->height == m_imgSnap->height)
                {
                    if(m_nWordPos == 0)
                    nHeight = m_imgSnap->height - m_nExtentHeight;
                    else
                    nHeight = 0;
                }
                else
                {
                    if(m_nWordPos == 0)
                    nHeight = (m_imgSnap->height)*(nIndex+1) - m_nExtentHeight;
                    else
                    nHeight = (m_imgSnap->height)*(nIndex);
                }
            }
            nStartX = nWidth;
printf("====###222==444444444=\n");

            //设备编号
            std::string strDirection = GetDirection(plate.uDirection);
            sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str());//
            string strText(chOut);

            //车道编号
            if(g_PicFormatInfo.nRoadIndex == 1)
            {
                sprintf(chOut,"车道编号:%d  ",plate.uRoadWayID);//
                string strTmp(chOut);
                strText += strTmp;
            }

            //车牌号码
            if(g_PicFormatInfo.nCarNum == 1)
            {
                std::string strCarNum(plate.chText);
                sprintf(chOut,"车牌号码:%s  ",strCarNum.c_str());
                string strTmp(chOut);
                strText += strTmp;
            }
printf("====###222==5555555=\n");
            //车速
            if(g_PicFormatInfo.nCarSpeed == 1)
            {
                //行驶速度
                sprintf(chOut,"速度:%dkm/h",plate.uSpeed);
                string strTmp(chOut);
                strText += strTmp;
            }

            memset(wchOut,0,sizeof(wchOut));
            UTF8ToUnicode(wchOut,(char*)strText.c_str());
            if(pImage->width < 1000)
            {
                nHeight += 20;
            }
            else
            {
                nHeight += (m_nExtentHeight/2);
            }
            m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

printf("=====22222===\n");
            /////////////////////////////////////////////第二行
            strText.clear();
            //经过时间
            std::string strTime;
            strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
            sprintf(chOut,"抓拍时间:%s:%03d  ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
            {
                string strTmp(chOut);
                strText += strTmp;
            }


            /*//红灯时间
            if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
            {
                UINT32 uTimestamp = pSignalTimeStamp->uTimestamp;
                strTime = GetTime(uTimestamp,0);
                {
                    sprintf(chOut,"红灯时间:%s:%03d  ",strTime.c_str(),(((pSignalTimeStamp)->ts)/1000)%1000);
                    string strTmp(chOut);
                    strText += strTmp;
                }
            }
            */
printf("=====333333333===\n");
            if(g_PicFormatInfo.nViolationType == 1)
            {
               //违章行为
                if(plate.uViolationType != 0)
                {
                    std::string strViolationType;
                    if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
                    {
                        strViolationType = "闯红灯";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_PARKING_VIOLATION)
                    {
                        strViolationType = "违章停车";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_FORBID_LEFT)
                    {
                        strViolationType = "禁止左拐";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_FORBID_RIGHT)
                    {
                        strViolationType = "禁止右拐";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_FORBID_STRAIGHT)
                    {
                        strViolationType = "禁止前行";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_RETROGRADE_MOTION)
                    {
                        strViolationType = "逆行";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_PRESS_LINE)
                    {
                        strViolationType = "压线";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_NO_PASSING)
                    {
                        strViolationType = "禁止行驶";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_SMALL_IN_FORBIDDEN_ROAD)
                    {
                        strViolationType = "禁行小车";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD)
                    {
                        strViolationType = "禁行大车";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_BIG_IN_FORBIDDEN_TIME)
                    {
                        strViolationType = "大货禁行";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_ELE_EVT_BIANDAO)
                    {
                        strViolationType = "变道";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_OBV_TAKE_UP_BUSWAY)
                    {
                        strViolationType = "占用公交道";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_CYC)
                    {
                        strViolationType = "柴油车出现";
                    }
                    else if(plate.uViolationType == DETECT_RESULT_NOCARNUM)
                    {
                        strViolationType = "无牌车出现";
                    }
					else if(plate.uViolationType == DETECT_RESULT_NOT_LOCAL_CAR)
                    {
                        strViolationType = "出现非本地车";
                    }
                    sprintf(chOut,"违章行为:%s",strViolationType.c_str());
                    string strTmp(chOut);
                    strText += strTmp;
                }
            }

            memset(wchOut,0,sizeof(wchOut));
            UTF8ToUnicode(wchOut,(char*)strText.c_str());
            nWidth = 10;
            if(pImage->width > m_imgSnap->width)
            {
                nWidth += (m_imgSnap->width)*(nIndex%2);
            }
            nStartX = nWidth;

            if(pImage->width < 1000)
            {
                nHeight += 20;
            }
            else
            {
                nHeight += (m_nExtentHeight/2);
            }
            m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
        }
    }
    else //空白区域
    {
        printf("====###white==444444444=\n");
        CvxText cvText;
        cvText.Init(120);

        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = pImage->width/2+50;
        int nHeight = pImage->height/2+400;

        if(g_nVtsPicMode > 1)
        {
            nHeight = pImage->height*2/3+400;
        }
printf("=====4444444===\n");
        sprintf(chOut,"路口编号:%s",g_strDetectorID.c_str());//
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

        nHeight += 120;
        //经过地点
        sprintf(chOut,"路口名称:");
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

        nHeight += 120;
        //经过地点
        sprintf(chOut,"%s",m_strLocation.c_str());
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

        nHeight += 120;
         //行驶方向
        memset(wchOut,0,sizeof(wchOut));
        std::string strDirection = GetDirection(plate.uDirection);
        sprintf(chOut,"车道方向:%s",strDirection.c_str());
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

        nHeight += 120;
        //经过时间
        std::string strTime = GetTime(plate.uTime);
        sprintf(chOut,"抓拍时间: %s",strTime.c_str());
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

        /*nHeight += 120;
        //车牌号码
        std::string strCarNum(plate.chText);
        memset(wchOut,0,sizeof(wchOut));
        sprintf(chOut,"车牌号码: %s",strCarNum.c_str());
        UTF8ToUnicode(wchOut,chOut);
        cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));*/
printf("=====555555===\n");
        cvText.UnInit();
    }

    printf("===out==CDspMoveDetect::PutVtsTextOnImage===\n");

}


/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CDspMoveDetect::PutTextOnComposeImage(IplImage* pImage,RECORD_PLATE plate,int nOrder)
{
    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;
    if( m_nWordPos== 0)
    {
        nHeight = m_imgSnap->height - m_nExtentHeight;
    }

    nStartX = nWidth;


    //设备编号,经过地点,行驶方向,经过时间
    std::string strDirection = GetDirection(plate.uDirection);
    std::string strTime = GetTime(plate.uTime,0);
    //sprintf(chOut,"设备编号:%s    地点名称:%s    方向:%s    时间:%s.%03d",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime);//
	sprintf(chOut,"设备编号:%d    地点名称:%s    方向:%s    时间:%s.%03d",plate.uChannelID,m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime);//
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    nHeight += 90;
    m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

     //第一时间
    if(m_imgSnap->width > 2000)
    nWidth = 800;
    else
    nWidth = 400;

    nHeight += 100;
    if( m_nWordPos== 0)
    {
        nHeight = 100;
    }
	if(nOrder == 0)
	{
		sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);
	}
	else
	{
		std::string strTime2 = GetTime(plate.uTime2,0);
		sprintf(chOut,"%s.%03d",strTime2.c_str(),plate.uMiTime2);
	}
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));


    //第二时间
    if(m_imgSnap->width > 2000)
    nWidth = 3200;
    else
    nWidth = 2000;
	if(nOrder == 0)
	{
		std::string strTime2 = GetTime(plate.uTime2,0);
		sprintf(chOut,"%s.%03d",strTime2.c_str(),plate.uMiTime2);
	}
	else
	{
		sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);
	}
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);
    m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

    if(m_nSmallPic == 1)
    {
        //小图时间
        if(m_imgSnap->width > 2000)
        nWidth = 5400;
        else
        nWidth = 3400;
        sprintf(chOut,"%s.%03d",strTime.c_str(),plate.uMiTime);
        memset(wchOut,0,sizeof(wchOut));
        UTF8ToUnicode(wchOut,chOut);
        m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
    }
}


/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CDspMoveDetect::PutTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp)
{
    wchar_t wchOut[255] = {'\0'};
    char chOut[255] = {'\0'};

    int nStartX = 0;
    int nWidth = 10;
    int nHeight = 0;

    if(pImage->width > m_imgSnap->width)
    {
        nWidth += (m_imgSnap->width)*(nIndex%2);
        if(m_nWordPos == 0)
        {
            nHeight = (m_imgSnap->height)*(nIndex/2+1) - m_nExtentHeight;
        }
        else
        {
            nHeight = (m_imgSnap->height)*(nIndex/2);
        }
    }
    else
    {
        if(pImage->height == m_imgSnap->height)
        {
            if(m_nWordPos == 0)
            nHeight = m_imgSnap->height - m_nExtentHeight;
            else
            nHeight = 0;
        }
        else
        {
            if(m_nWordPos == 0)
            nHeight = (m_imgSnap->height)*(nIndex+1) - m_nExtentHeight;
            else
            nHeight = (m_imgSnap->height)*(nIndex);
        }
    }
    nStartX = nWidth;


	if(g_nServerType == 7 || g_nServerType == 10)
	{
		std::string strDirection = GetDirection(plate.uDirection);
		std::string strCarNum(plate.chText);
		std::string strTime;
		strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);

		sprintf(chOut,"路口名称:%s  行驶方向:%s  车道号:%d  车牌号码:%s  抓拍时间:%s %03d",m_strLocation.c_str(),strDirection.c_str(),plate.uRoadWayID,strCarNum.c_str(),strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);//
		string strText(chOut);

		memset(wchOut,0,sizeof(wchOut));
		UTF8ToUnicode(wchOut,(char*)strText.c_str());
		if(pImage->width < 1000)
		{
			nHeight += 20;
		}
		else
		{
			nHeight += (m_nExtentHeight/2);
		}
		m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
		return;
	}


    //设备编号
    std::string strDirection = GetDirection(plate.uDirection);
	//LogNormal("ready to add 设备编号\n");
	//sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s   经度:%.4f  纬度:%.4f",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str(),((plate.uLatitude)*1.0/10000),((plate.uLongitude)*1.0/10000));
    sprintf(chOut,"相机编号:%d  地点名称:%s  方向:%s   经度:%.4f  纬度:%.4f",plate.uChannelID,m_strLocation.c_str(),strDirection.c_str(),((plate.uLatitude)*1.0/1000000),((plate.uLongitude)*1.0/1000000));
	//LogNormal("add 设备编号 end\n");
	string strText(chOut);
    //车道编号
    if(g_PicFormatInfo.nRoadIndex == 1)
    {
        sprintf(chOut,"车道编号:%d  ",plate.uRoadWayID);//
        string strTmp(chOut);

        strText += strTmp;
    }

    //车牌号码
    if(g_PicFormatInfo.nCarNum == 1)
    {
        std::string strCarNum(plate.chText);
        sprintf(chOut,"车牌号码:%s",strCarNum.c_str());
        string strTmp(chOut);
        strText += strTmp;
    }

    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());
    if(pImage->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
        nHeight += (m_nExtentHeight/2);
    }
    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));



    /////////////////////////////////////////////第二行
    strText.clear();
    //经过时间
    std::string strTime;

    strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);
    sprintf(chOut,"时间:%s.%03d  ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);
    {
        string strTmp(chOut);
        strText += strTmp;
    }

    if(g_PicFormatInfo.nCarSpeed == 1)
    {
        //行驶速度
        sprintf(chOut,"速度:%dkm/h  ",plate.uSpeed);
        string strTmp(chOut);
        strText += strTmp;
    }

    if(g_PicFormatInfo.nCarColor == 1)
    {
        //车身颜色
        std::string strCarColor = GetObjectColor(plate.uCarColor1);
        std::string strCarColor2 = GetObjectColor(plate.uCarColor2);
        if(plate.uCarColor2 == UNKNOWN)
        sprintf(chOut,"颜色:%s  ",strCarColor.c_str());
        else
        sprintf(chOut,"颜色:%s,%s  ",strCarColor.c_str(),strCarColor2.c_str());
        string strTmp(chOut);
        strText += strTmp;
    }

    if(g_PicFormatInfo.nCarType == 1)
    {
        //车辆类型
        std::string strCarType;
        switch(plate.uTypeDetail)
        {
        case BUS_TYPE:
            strCarType = "大型客车";
            break;
        case TRUCK_TYPE:
            strCarType = "大型货车";
            break;
        case MIDDLEBUS_TYPE:
            strCarType = "中型客车";
            break;
        case TAXI:
            strCarType = "小型客车";
            break;
        case MINI_TRUCK:
            strCarType = "小型货车";
            break;
        default:
            if(plate.uType == SMALL_CAR)
            {
                strCarType = "小车";
            }
            else if(plate.uType == MIDDLE_CAR)
            {
                strCarType = "中车";
            }
            else if(plate.uType == BIG_CAR)
            {
                strCarType = "大车";
            }
            else
            {
                strCarType = "";
            }
            break;
        }
        sprintf(chOut,"车辆类型:%s  ",strCarType.c_str());
        string strTmp(chOut);
        strText += strTmp;
    }

    if(g_PicFormatInfo.nCarBrand == 1)
    {
        //车标
       if(plate.uCarBrand != 100)
       {
           if(plate.uCarBrand != OTHERS)
           sprintf(chOut,"车标:%s",g_strCarLabel[plate.uCarBrand]);
           else
           sprintf(chOut,"车标:");

           string strTmp(chOut);
           strText += strTmp;
       }
    }

    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,(char*)strText.c_str());
    nWidth = 10;
    if(pImage->width > m_imgSnap->width)
    {
        nWidth += (m_imgSnap->width)*(nIndex%2);
    }
    nStartX = nWidth;

    if(pImage->width < 1000)
    {
        nHeight += 20;
    }
    else
    {
        nHeight += (m_nExtentHeight/2);
    }
    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
}

/* 函数介绍：添加数字水印
 * 输入参数：
 * 输出参数：无
 * 返回值：无
 */
void CDspMoveDetect::GetWaterMark(const char* pData,int nSize,std::string& strWaterMark)
{
    //先添加0xFFD8FFE0
    UINT32 nMarkFlag = 0xE0FFD8FF;
    strWaterMark.append((char*)&nMarkFlag,sizeof(UINT32));
    //再从图像1k开始每10k取一个字节，一共取20次，如果不足20次则补充满20次，补充的字节为0x56
    //对这20个字节分别与0x94进行异或
    int nCount = 0;
    for(UINT32 i = 1000; i< nSize; i += 10000)
    {
        char  chWaterMark = pData[i]^(0x94);
        strWaterMark.append(&chWaterMark,1);
        nCount++;

        if(nCount >=20)
            break;
    }

    //printf("===****nSize=%d***nCount=%d****strWaterMark.size=%d\n",nSize,nCount,strWaterMark.size());
    if(nCount < 20)
    {
        char uComplement = 0x56;
        for(int j =0; j<20 - nCount; j++)
        {
            char  chWaterMark = uComplement^(0x94);
            strWaterMark.append(&chWaterMark,1);
        }
    }
    //printf("===************strWaterMark.size=%d\n",strWaterMark.size());
}


//保存全景图像
int CDspMoveDetect::SaveImage(IplImage* pImg,std::string strPicPath,int nIndex,UINT16* features)
{
    #ifdef LogTime
    struct timeval tv;
    double t = (double)cvGetTickCount();
    gettimeofday(&tv,NULL);
    LogTrace("time-test.log","before SaveImage==time = %s.%03d\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
    #endif

    BYTE* pOutImage = NULL;

    pOutImage = m_pCurJpgImage;


    CxImage image;
    int srcstep = 0;
    if(image.IppEncode((BYTE*)pImg->imageData,pImg->width,pImg->height,3,&srcstep,pOutImage,g_PicFormatInfo.nJpgQuality))
    {
        #ifdef LogTime
        t = (double)cvGetTickCount() - t;
        double dt = t/((double)cvGetTickFrequency()*1000.) ;
        gettimeofday(&tv,NULL);
        LogTrace("time-test.log","after IppEncode==time = %s.%03d,dt=%d ms\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
        t = (double)cvGetTickCount();
        #endif
        FILE* fp = NULL;
        if(nIndex == 1)
        {
           fp = fopen(strPicPath.c_str(),"a");
        }
        else
        {
           fp = fopen(strPicPath.c_str(),"wb");
        }
        if(fp!=NULL)
        {
            fwrite(pOutImage,srcstep,1,fp);

            //叠加数字水印
            #ifdef WATER_MARK
            std::string strWaterMark;
            GetWaterMark((char*)pOutImage,srcstep,strWaterMark);
            //printf("before write WaterMark,strWaterMark.size()=%d\n",strWaterMark.size());
            //fflush(stdout);
            fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
            //printf("after write WaterMark\n");
            //fflush(stdout);
            srcstep += strWaterMark.size();
            #endif

            #ifdef OBJECT_FEATURE//将目标特征追加在图片后面
            if(features!=NULL)
            {
                int nSizeTexture = DIM_FEATURE * sizeof(UINT16);
                fwrite(features,nSizeTexture,1,fp);
                srcstep += nSizeTexture;
            }
            #endif

            fclose(fp);

        }
        else
        {
            printf("canot open file\n");
        }
    }
    return srcstep;
}

//流量统计
void CDspMoveDetect::VehicleStatistic(StatResultList& listStatResult,UINT32 uTimestamp)
{
    //车道编号
    UINT32 uRoadID;
    UINT32 uVerRoadID;
    UINT32 uObjectNum;

    //统计保存
    RECORD_STATISTIC statistic;

    StatResultList::iterator it_b = listStatResult.begin();
    StatResultList::iterator it_e = listStatResult.end();
    while(it_b != it_e)
    {
        uRoadID = it_b->nChannelIndex;       //车道编号

        vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
        while(it != m_vtsObjectRegion.end())
        {
            if(uRoadID == -1||
               uRoadID == it->nRoadIndex)
            {
                uVerRoadID = it->nVerRoadIndex;//车道逻辑编号
                uRoadID = it->nRoadIndex;
                break;
            }
            it++;
        }

        //高16位车道逻辑编号，低16位车道类型
        uVerRoadID = uVerRoadID<<16;
        statistic.uRoadType[uRoadID-1] = uVerRoadID | VEHICLE_ROAD;


        switch(it_b->sRtype)
        {
        case STAT_FLUX:
            //高16位大车总数，低16位车辆总数
            uObjectNum = uFluxBig[uRoadID-1];
            uObjectNum = uObjectNum<<16;
            uFluxAll[uRoadID-1] = (UINT32) (it_b->value);
            statistic.uFlux[uRoadID-1] = uObjectNum | (uFluxAll[uRoadID-1]);
            break;
        case STAT_SPEED_AVG:
            statistic.uSpeed[uRoadID-1] = (UINT32) (it_b->value);
            break;
        case STAT_ZYL:
            statistic.uOccupancy[uRoadID-1] = (UINT32) (it_b->value);
            break;
        case STAT_QUEUE:
            statistic.uQueue[uRoadID-1] = (UINT32) (it_b->value);
            break;
        case STAT_CTJJ:
            statistic.uSpace[uRoadID-1] = (UINT32) (it_b->value);
            break;
        }

        //高16位中型车总数，低16位小车总数
        uObjectNum = uFluxMiddle[uRoadID-1];
        uObjectNum = uObjectNum<<16;
        statistic.uFluxCom[uRoadID-1] = uObjectNum|(uFluxSmall[uRoadID-1]);
        statistic.uTime = uTimestamp;
        statistic.uStatTimeLen = m_nTrafficStatTime;

        it_b++;
    }

    //记录检测数据,保存统计
    g_skpDB.SaveStatisticInfo(m_nChannelId,statistic,VEHICLE_ROAD);

    //流量统计初始化
    for(int i = 0; i<MAX_ROADWAY; i++)
    {
        uFluxAll[i]=0;
        uFluxSmall[i]=0;
        uFluxMiddle[i]=0;
        uFluxBig[i]=0;
        uFluxPerson[i]=0;
        uFluxNoneVehicle[i]=0;
    }
}

//输出违章检测结果
void CDspMoveDetect::OutPutVTSResult(ViolationInfo& infoViolation, PLATEPOSITION *TimeStamp)
{
    printf("===in==CDspMoveDetect::OutPutVTSResult==\n");

        RECORD_PLATE plate;
         //违章类型
        if(infoViolation.evtType == ELE_RED_LIGHT_VIOLATION)// 闯红灯
        {
            plate.uViolationType = DETECT_RESULT_RED_LIGHT_VIOLATION;
        }
        else if(infoViolation.evtType == ELE_PARKING_VIOLATION)// 违章停车
        {
            plate.uViolationType = DETECT_RESULT_PARKING_VIOLATION;
        }
        else if(infoViolation.evtType == ELE_FORBID_LEFT)// 禁止左拐
        {
            plate.uViolationType = DETECT_RESULT_FORBID_LEFT;
        }
        else if(infoViolation.evtType == ELE_FORBID_RIGHT)// 禁止右拐
        {
            plate.uViolationType = DETECT_RESULT_FORBID_RIGHT;
        }
        else if(infoViolation.evtType == ELE_FORBID_STRAIGHT)// 禁止前行
        {
            plate.uViolationType = DETECT_RESULT_FORBID_STRAIGHT;
        }
        else if(infoViolation.evtType == ELE_RETROGRADE_MOTION)// 逆行
        {
            plate.uViolationType = DETECT_RESULT_RETROGRADE_MOTION;
        }
        else if(infoViolation.evtType == ELE_PRESS_LINE)// 压线
        {
            plate.uViolationType = DETECT_RESULT_PRESS_LINE;
        }
        else if(infoViolation.evtType == ELE_NO_PASSING)// 禁行
        {
            plate.uViolationType = DETECT_RESULT_NO_PASSING;
        }
        else if(infoViolation.evtType == ELE_EVT_BIANDAO)//变道
        {
            plate.uViolationType = DETECT_RESULT_ELE_EVT_BIANDAO;
        }
        else if(infoViolation.evtType == OBV_TAKE_UP_BUSWAY)//占用公交道
        {
            plate.uViolationType = DETECT_RESULT_OBV_TAKE_UP_BUSWAY;
        }
        else if(infoViolation.evtType == EVT_CYC_APPEAR)//柴油车出现
        {
            plate.uViolationType = DETECT_RESULT_CYC;
        }
        else if(infoViolation.evtType == EVT_FORBID_TRUCK)//大货禁行
        {
            plate.uViolationType = DETECT_RESULT_BIG_IN_FORBIDDEN_TIME;
        }
        else if(infoViolation.evtType == EVT_GO_FAST)//车辆超速
        {
            plate.uViolationType = DETECT_RESULT_EVENT_GO_FAST;
        }
		else if(infoViolation.evtType == EVT_NON_LOCAL_PLATE)//非本地车
        {
            plate.uViolationType = DETECT_RESULT_NOT_LOCAL_CAR;
        }


        std::string strCarNum = infoViolation.carInfo.strCarNum;
        //判断是否有车牌的车
        bool bCarNum = true;
        if( (infoViolation.carInfo.strCarNum[0] == '*')&& (infoViolation.carInfo.strCarNum[6] == '*') )
        {
            bCarNum = false;
        }

        if(bCarNum)
        {
            //车牌号码转换
            CarNumConvert(strCarNum,infoViolation.carInfo.wj);
        }

        //PLATEPOSITION  TimeStamp[6];
		PLATEPOSITION  SignalTimeStamp;


        //经过时间(秒)
        plate.uTime = infoViolation.carInfo.uTimestamp;
        //毫秒
        plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;
        //车牌号码
        memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
        //地点
        memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
        //行驶方向
		//LogNormal("m_nDetectDirection=%d,infoViolation.carInfo.nDirection=%d",m_nDetectDirection,infoViolation.carInfo.nDirection);
        if(m_nDetectDirection != infoViolation.carInfo.nDirection)
        {
            if(infoViolation.carInfo.nDirection != -1)
            {
                if(m_nDirection%2==0)
                {
                    plate.uDirection = m_nDirection - 1;
                }
                else
                {
                    plate.uDirection = m_nDirection + 1;
                }
            }
            else
            {
                plate.uDirection = m_nDirection;
            }
        }
        else
        {
            plate.uDirection = m_nDirection;
        }
        //车牌结构
        plate.uPlateType = infoViolation.carInfo.carnumrow;
        //车牌颜色
        plate.uColor = infoViolation.carInfo.color;
        //车辆类型
        plate.uType =  infoViolation.carInfo.vehicle_type;

        //车型细分
        if(((m_nDetectKind&DETECT_TRUCK)==DETECT_TRUCK))
        plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);

        if(plate.uType==SMALL)
        {
            plate.uType = SMALL_CAR;
        }
        else if(plate.uType==MIDDLE)
        {
            plate.uType = MIDDLE_CAR;
        }
        else if(plate.uType==BIG)
        {
            plate.uType = BIG_CAR;
        }
        else if(plate.uType==OTHER)
        {
            plate.uType = OTHER_TYPE;
        }
        else if(plate.uType==PERSON)
        {
            plate.uType = PERSON_TYPE;
        }
        else if(plate.uType == TWO_WHEEL)
        {
            plate.uType = OTHER_TYPE;
            plate.uTypeDetail = TWO_WHEEL_TYPE;
        }

        //车道编号
        vector<ChannelRegion>::iterator it_b = m_vtsObjectRegion.begin();
        while(it_b != m_vtsObjectRegion.end())
        {
            if(-1 == infoViolation.nChannel)
            {
                plate.uRoadWayID = it_b->nVerRoadIndex;
                break;
            }
            else if(it_b->nRoadIndex == infoViolation.nChannel)
            {
                plate.uRoadWayID = it_b->nVerRoadIndex;
                break;
            }
            it_b++;
        }

        //车速
        double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
        plate.uSpeed = (UINT32)(dSpeed+0.5);

        //发生位置(在当前图片上的)
        plate.uPosLeft  = infoViolation.carInfo.ix;
        plate.uPosTop   = infoViolation.carInfo.iy;
        plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
        plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

        //获取图片路径
        std::string strPicPath,strVideoPath,strTmpPath;
        pthread_mutex_lock(&g_Id_Mutex);

        ////////////////////
        //需要判断磁盘是否已经满
        g_FileManage.CheckDisk(false,false);

        //存储大图片
        strPicPath  = g_FileManage.GetPicPath();

        int nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,0);
        //解锁
        pthread_mutex_unlock(&g_Id_Mutex);

        //删除已经存在的记录
        g_skpDB.DeleteOldRecord(strPicPath,false,false);
        //大图存储路径
        memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

        /////////////////////////////叠加文字信息(需要叠三次)



            if(m_imgComposeSnap != NULL)
            {
                //大图宽高
                plate.uPicWidth = m_imgComposeSnap->width;
                plate.uPicHeight = m_imgComposeSnap->height;

                for(int i=0; i<6; i++)
                {
                    printf("=====TimeStamp[%d]=%lld===\n", i, TimeStamp[i].uTimestamp);
                }

                for(int nIndex = 0; nIndex <3 ; nIndex++)
                {
                    printf("=nIndex=%d====\n", nIndex);
                    PutVtsTextOnImage(m_imgComposeSnap,plate,nIndex,TimeStamp);
                }

                printf("=Before 2222=PutVtsTextOnImage==\n");

                //叠加额外信息
                if( (g_nVtsPicMode > 0)&& (g_nVtsPicMode < 3))
                {
                    PutVtsTextOnImage(m_imgComposeSnap,plate);
                }


                plate.uPicSize = SaveImage(m_imgComposeSnap,strPicPath,2);

                printf("=OutPutVTSResult=After SaveImage==\n");
            }


        //保存闯红灯记录
        if(nSaveRet>0)
        g_skpDB.SavePlate(m_nChannelId,plate,0);

        //将车牌信息送客户端
        if(m_bConnect)
        {
            SRIP_DETECT_HEADER sDetectHeader;
            sDetectHeader.uChannelID = m_nChannelId;
            sDetectHeader.uDetectType = SRIP_CARD_RESULT;
            sDetectHeader.uTimestamp = infoViolation.carInfo.uTimestamp;
            sDetectHeader.uSeq = infoViolation.carInfo.uSeq;

            std::string result;
            //车牌号码
            memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
            result.append((char*)&plate,sizeof(plate));

            result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
            g_skpChannelCenter.AddResult(result);
        }

    printf("===out==CDspMoveDetect::OutPutVTSResult==\n");
}


//输出车牌检测结果
void CDspMoveDetect::CarNumOutPut(CarInfo& cardNum)
{
    //LogNormal("====CDspMoveDetect::CarNumOutPut====\n");
    CvRect rtRoi;
    {
        /////
        {
            //////////////////////////////
            std::string strCarNum;
            strCarNum = cardNum.strCarNum;
            //LogNormal("cardNum.uSeq=%d=strCarNum=%s\r\n", cardNum.uSeq, strCarNum.c_str());

            //判断是否有车牌的车
            bool bCarNum = true;
            bool bLoop = false;
            if( (cardNum.strCarNum[0] == '*') && (cardNum.strCarNum[6] == '*') )
            {
                bCarNum = false;
                if(cardNum.strCarNum[1] == '+')
                {
                    bLoop = true;
                }
            }

            if(bCarNum)
            {
                //车牌号码转换
                CarNumConvert(strCarNum, cardNum.wj);
            }
            ////////////////////////////////////////////////////
//                printf("cardNum.uSeq=%d,cardNum.imgIndex=%d\n",cardNum.uSeq,cardNum.imgIndex);

            PLATEPOSITION  TimeStamp[2];
            TimeStamp[0].uTimestamp = cardNum.uTimestamp;
            TimeStamp[0].uFieldSeq = cardNum.uSeq;


            //设置图像实际大小（不包含下面的黑色区域,）
            CvRect rtRealImage;
            rtRealImage.x = 0;
            rtRealImage.y = 0;
            rtRealImage.width = m_imgSnap->width;
            rtRealImage.height = m_imgSnap->height-m_nExtentHeight;
            if(m_nWordPos == 1)
            {
                rtRealImage.y += m_nExtentHeight;
            }

            RECORD_PLATE plate;
            //经过时间(秒)
            plate.uTime = cardNum.uTimestamp;

            //毫秒
            plate.uMiTime = (cardNum.ts/1000)%1000;
            //车牌号码
            memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());


			//车牌世界坐标 add by wantao
			plate.uLongitude = (UINT32)(cardNum.wx*10000);
			plate.uLatitude = (UINT32)(cardNum.wy*10000);
			//相机ID
			plate.uChannelID = cardNum.id;

            //地点
            memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
            //行驶方向
             plate.uDirection = m_nDirection;

            //车牌结构
            plate.uPlateType = cardNum.carnumrow;
            //车牌颜色
            plate.uColor = cardNum.color;
            if(plate.uColor <= 0)
            {
               plate.uColor =  CARNUM_OTHER;
            }
            //帧号
            plate.uSeqID = TimeStamp[0].uFieldSeq;

            //      printf("222cardNum.x=%d,y=%d,w=%d,h=%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);

            //车牌区域
            CvRect rtCarnum;
            rtCarnum.x = cardNum.ix;
            rtCarnum.y = cardNum.iy;
            rtCarnum.width = cardNum.iwidth;
            rtCarnum.height = cardNum.iheight;
            if(rtCarnum.x+rtCarnum.width>=m_imgSnap->width)
            {
                rtCarnum.width = m_imgSnap->width - rtCarnum.x-1;
            }
            if(rtCarnum.y+rtCarnum.height>=m_imgSnap->height-m_nExtentHeight)
            {
                rtCarnum.height = m_imgSnap->height-m_nExtentHeight - rtCarnum.y-1;
            }

            //车牌检测区域
            rtRoi.x = m_rtVlpRoi.x;
            rtRoi.width = m_rtVlpRoi.width;
            rtRoi.y = m_rtVlpRoi.y;
            rtRoi.height = m_rtVlpRoi.height;

            object_color objColor;
            carnum_context context;
            context.position =  rtCarnum;
            context.vehicle_type = cardNum.vehicle_type;
            context.color = cardNum.color;
            context.mean  = cardNum.mean;
            context.stddev = cardNum.stddev;

            context.VerticalTheta  = cardNum.VerticalTheta;
            context.HorizontalTheta = cardNum.HorizontalTheta;

            memcpy(context.carnum,cardNum.strCarNum,sizeof(context.carnum));

            context.smearnum = cardNum.smearCount;
            memcpy(context.smearrect,cardNum.smear,sizeof(CvRect)*(cardNum.smearCount));
            context.nCarNumDirection = cardNum.nDirection;//目标运动方向

            //获取机动车颜色
            if((m_nDetectKind&DETECT_CARCOLOR)==DETECT_CARCOLOR)
            {
                if(bCarNum||bLoop)
                {
                    #ifdef LogTime
                    struct timeval tv;
                    double t = (double)cvGetTickCount();
                    gettimeofday(&tv,NULL);
                    LogTrace("time-test.log","before mvGetCardCarColor==time = %s.%03d\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
                    #endif

                    //cvSetImageROI(m_imgSnap,rtRealImage);
                    //m_carColor.mvGetCardCarColor(m_imgSnap->imageData,context,rtRoi,&objColor,m_nDayNight,bCarNum);
                    int nExtentHeight = 0;
                    if(m_nWordPos == 1)
                    {
                        nExtentHeight = m_nExtentHeight;
                    }

                    printf("=m_imgSnap->widthStep=%d=nExtentHeight=%d,m_nDayNight=%d\n", m_imgSnap->widthStep, nExtentHeight, m_nDayNight); //
                    //m_carColor.mvGetCardCarColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNight,bCarNum);


                objColor.nColor1 = 1;
                printf("before=objColor.nColor1=%d\n", objColor.nColor1);

                    m_carColor.mvGetCardCarColor(m_img->imageData,context,rtRoi,&objColor,m_nDayNight,bCarNum);

            //存图测试--begin
                /*BYTE* pOutImage = NULL;
                pOutImage = new BYTE[m_img->width*m_img->height/4];

                printf("=222==m_imgSnap->width=%d=m_imgSnap->height=%d\n", m_imgSnap->width, m_imgSnap->height);
                printf("==g_PicFormatInfo.nJpgQuality=%d", g_PicFormatInfo.nJpgQuality);

                char jpg_name[256] = {0};
                //sprintf(jpg_name, "./text/264-img-hea.avi");
                sprintf(jpg_name, "./text/%d_TT_seq.jpg", cardNum.uSeq);
                CxImage image;
                int srcstep = 0;
                if(image.IppEncode((BYTE*)m_img->imageData, m_img->width, m_img->height, 3, &srcstep, pOutImage, g_PicFormatInfo.nJpgQuality))
                //if(image.IppEncode((BYTE*)m_imgSnap->imageData, m_imgSnap->width, m_imgSnap->height, 3, &srcstep, pOutImage, g_PicFormatInfo.nJpgQuality))
                {
                    FILE* fp = NULL;
                    fp = fopen(jpg_name, "a");
                    if(fp!=NULL)
                    {
                        fwrite(pOutImage, srcstep, 1, fp);
                    }
                }

                if(pOutImage != NULL)
                {
                    delete pOutImage;
                    pOutImage = NULL;
                }
                printf("=After save image...==\n");*/
            //存图测试--end


                printf("after=objColor.nColor1=%d\n", objColor.nColor1);

                    printf("after mvGetCarColor\n");

                   //cvResetImageROI(m_imgSnap);
                    #ifdef LogTime
                    t = (double)cvGetTickCount() - t;
                    double dt = t/((double)cvGetTickFrequency()*1000.) ;
                    gettimeofday(&tv,NULL);
                    LogTrace("time-test.log","after mvGetCardCarColor==time = %s.%03d,dt=%d ms\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
                     #endif

                    plate.uCarColor1 = objColor.nColor1;
                    plate.uWeight1 = objColor.nWeight1;
                    plate.uCarColor2 = objColor.nColor2;
                    plate.uWeight2 = objColor.nWeight2;

                    cardNum.objColor = objColor;
                }
                else//获取非机动车及行人颜色
                {
                    plate.uCarColor1 = cardNum.objColor.nColor1;
                    plate.uWeight1 = cardNum.objColor.nWeight1;
                    plate.uCarColor2 = cardNum.objColor.nColor2;
                    plate.uWeight2 = cardNum.objColor.nWeight2;
                }
            }


            //提取车身特征向量
            if((m_nDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE)
            {
                //提取车身纹理特征向量
                if(bCarNum)
                {
                    #ifdef LogTime
                    struct timeval tv;
                    double t = (double)cvGetTickCount();
                    gettimeofday(&tv,NULL);
                    LogTrace("time-test.log","before mvGetClassifyEHDTexture==time = %s.%03d\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
                    #endif

                    //cvSetImageROI(m_imgSnap,rtRealImage);
                    ushort uCarBrand;
                    //mvGetClassifyEHDTexture( m_imgSnap,context,rtRoi,uCarBrand,m_nDayNight);
                    int nExtentHeight = 0;
                    if(m_nWordPos == 1)
                    {
                        nExtentHeight = m_nExtentHeight;
                    }
                    m_carLabel.mvGetClassifyEHDTexture( m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNight );
                    plate.uCarBrand = uCarBrand;

					printf("after mvGetClassifyEHDTexture\n");

                    //cvResetImageROI(m_imgSnap);

                   #ifdef LogTime
                    t = (double)cvGetTickCount() - t;
                    double dt = t/((double)cvGetTickFrequency()*1000.) ;
                    gettimeofday(&tv,NULL);
                    LogTrace("time-test.log","after mvGetClassifyEHDTexture==time = %s.%03d,dt=%d ms\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
                    #endif
                }
            }

            //获取车辆类型（高16位卡车、巴士、轿车等，低16位大、中、小车）
            if((m_nDetectKind&DETECT_TRUCK)==DETECT_TRUCK)
            {
                int nRet = 0;
                if( bCarNum)
                {
                    printf("=333=cardNum.nDirection=%d",cardNum.nDirection);
                    printf("m_nDetectDirection=%d", m_nDetectDirection);


                    if(cardNum.nDirection == 0)
                    {
                        std::string strNumber;
                        strNumber = cardNum.strCarNum;
                        if(*(strNumber.c_str()+strNumber.size()-1)!='$')
                        {
                            #ifdef LogTime
                            struct timeval tv;
                            double t = (double)cvGetTickCount();
                            gettimeofday(&tv,NULL);
                            LogTrace("time-test.log","before mvTruckDetect==time = %s.%03d\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000);
                            #endif
                        printf("before mvTruckDetect m_imgSnap(%d,%d),m_nDayNight=%d\n",m_imgSnap->width,m_imgSnap->height,m_nDayNight);

                            //cvSetImageROI(m_imgSnap,rtRealImage);

							bool IsForeCarnum=true;
							if(m_nDetectDirection == 1)
							{
								IsForeCarnum = false;
							}
                            nRet = m_vehicleClassify.mvTruckDetect(m_imgSnap,context,m_nDayNight,IsForeCarnum);

                        printf("==after mvTruckDetect nRet=%d=\n");

                            //cvResetImageROI(m_imgSnap);

                            #ifdef LogTime
                            t = (double)cvGetTickCount() - t;
                            double dt = t/((double)cvGetTickFrequency()*1000.) ;
                            gettimeofday(&tv,NULL);
                            LogTrace("time-test.log","after mvTruckDetect==time = %s.%03d,dt=%d ms\n",GetTime(tv.tv_sec).c_str(),tv.tv_usec/1000,(int)dt);
                             #endif
                        }
                        cardNum.subVehicleType = nRet;
                    }
                    else
                    {
                        nRet = cardNum.subVehicleType;
                    }

                    plate.uTypeDetail = GetTypeDetail(nRet);
                }
                //大货禁行直接在此输出
                //GetVtsResult(cardNum,plate);
            }


            //车道逻辑编号(暂时取第一个)
            printf("=cardNum.RoadIndex=%d=\n", cardNum.RoadIndex);
			//LogNormal("11111=cardNum.RoadIndex=%d=\n", cardNum.RoadIndex);
            vector<ChannelRegion>::iterator it = m_vtsObjectRegion.begin();
            while(it != m_vtsObjectRegion.end())
            {
				//LogNormal("333 it->nRoadIndex = %d\n",it->nVerRoadIndex);
                if(-1 == cardNum.RoadIndex)
                {
                    plate.uRoadWayID = it->nVerRoadIndex;
                    cardNum.RoadIndex = it->nRoadIndex;
                    break;
                }
                else if(it->nRoadIndex == cardNum.RoadIndex)
                {
                    plate.uRoadWayID = it->nVerRoadIndex;
                    break;
                }

				//plate.uRoadWayID = it->nRoadIndex;
                it++;
            }

			//LogNormal("2222= plate.uRoadWayID=%d=\n", cardNum.RoadIndex);

            //车辆类型
            //plate.uType =  cardNum.vehicle_type;
            //printf("========plate.uType===%d\n",plate.uType);
            if(cardNum.vehicle_type==SMALL)
            {
                uFluxSmall[cardNum.RoadIndex-1]++;
                uFluxAll[cardNum.RoadIndex-1]++;
                plate.uType = SMALL_CAR;
            }
            else if(cardNum.vehicle_type==MIDDLE)
            {
                uFluxMiddle[cardNum.RoadIndex-1]++;
                uFluxAll[cardNum.RoadIndex-1]++;
                plate.uType = MIDDLE_CAR;
            }
            else if(cardNum.vehicle_type==BIG)
            {
                uFluxBig[cardNum.RoadIndex-1]++;
                uFluxAll[cardNum.RoadIndex-1]++;
                plate.uType = BIG_CAR;
            }
            else if(cardNum.vehicle_type==OTHER)
            {
                //uFluxNoneVehicle[cardNum.RoadIndex-1]++;
                plate.uType = OTHER_TYPE;
            }
            else if(cardNum.vehicle_type==PERSON)
            {
                //uFluxPerson[cardNum.RoadIndex-1]++;
                plate.uType = PERSON_TYPE;
            }
            else if(cardNum.vehicle_type == TWO_WHEEL)
            {
                //uFluxNoneVehicle[cardNum.RoadIndex-1]++;
                plate.uType = OTHER_TYPE;
                plate.uTypeDetail = TWO_WHEEL_TYPE;
            }
            //车速
            double dSpeed =   sqrt(cardNum.vx*cardNum.vx+cardNum.vy*cardNum.vy);


            plate.uSpeed = (UINT32)(dSpeed+0.5f);

            /////////////////////////////

            if(m_nWordPos == 1)
            {
                rtCarnum.y += m_nExtentHeight;
            }

            //车牌位置
            plate.uPosLeft = rtCarnum.x;
            plate.uPosTop = rtCarnum.y;
            plate.uPosRight = rtCarnum.x+rtCarnum.width-1;
            plate.uPosBottom = rtCarnum.y+rtCarnum.height-1;

            //图片尺寸
            plate.uPicWidth = m_imgSnap->width;
            plate.uPicHeight = m_imgSnap->height;


            //检测是否布控报警--add by ywx
            if(g_nDetectSpecialCarNum == 1)
            {
                plate.uAlarmKind = g_skpDB.IsSpecialCard(strCarNum);
                if(plate.uAlarmKind == 1)
                {
                    plate.uViolationType = DETECT_RESULT_BLACK_PLATE;
                    LogNormal("黑名单内车辆[%s]出现!\r\n", strCarNum.c_str());
                }
                else if(plate.uAlarmKind == 2)
                {
                    plate.uViolationType = DETECT_RESULT_WHITE_PLATE;
                    LogNormal("白名单内车辆[%s]出现!\r\n", strCarNum.c_str());
                }
            }

            if(cardNum.nNoCarNum == 1)
            plate.uViolationType = DETECT_RESULT_NOCARNUM;

			//不进行回写直接写数据库并发送给客户端
            {
                {
                     //获取图片路径
                    std::string strPicPath;
                    int nSaveRet = GetPicPathAndSaveDB(strPicPath);
                    printf("====CDspMoveDetect::CarNumOutPut===strPicPath=%s=nSaveRet=%d=\n", strPicPath.c_str(), nSaveRet);

                    //大图存储路径
                    memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());

            #ifdef ROSEEK_SAVE_INFO
                        FILE *fpOut = NULL;
                        char jpg_name[256] = "./text/Seq_DSP_pic.txt";
                        char charContant[512] = {0};
                        //if(pDsp_header->nType == 1)
                        {
                            //sprintf(jpg_name, "./text/Seq_DSP_ORDER.txt");
                            sprintf(charContant, "seq:%lld==pic:%s==\n", plate.uSeqID, strPicPath.c_str());
                            fpOut = fopen(jpg_name, "a+");
							if(fpOut)
							{
								fwrite(charContant, 1, sizeof(charContant), fpOut);
								fclose(fpOut);
							}
                        }
            #endif

					//LogNormal("line2296 相机ID=%d,帧号=%d\n",plate.uChannelID,plate.uSeqID);
                    PutTextOnImage(m_imgSnap,plate,0,TimeStamp);
                   #ifdef OBJECT_FEATURE//将目标特征追加在图片后面
                  plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0,cardNum.features);
                   #else
                   plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0,NULL);
                   #endif


                    //保存车牌记录
                    if(nSaveRet>0)
                    {
                        printf("======before==g_skpDB.SavePlate==\n");
                        g_skpDB.SavePlate(m_nChannelId,plate,0,NULL);
                        printf("======after==g_skpDB.SavePlate==\n");
                    }

                    //将车牌信息送客户端
                    if(m_bConnect)
                    {
                        SRIP_DETECT_HEADER sDetectHeader;
                        sDetectHeader.uChannelID = m_nChannelId;
                        //车牌检测类型
                        sDetectHeader.uDetectType = SRIP_CARD_RESULT;
                        sDetectHeader.uTimestamp = cardNum.uTimestamp;
                        sDetectHeader.uSeq = cardNum.uSeq;

                        std::string result;
                        //车牌号码
                        memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
                        //判断车牌位置是否需要扩充
                        GetCarPostion(plate);
                        result.append((char*)&plate,sizeof(plate));

                        result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
                        g_skpChannelCenter.AddResult(result);
                    }
                }
            }
        }

    }
}

//车牌检测
void CDspMoveDetect::DetectCarNum(IplImage* img, CvRect rtROI, UINT32 uTimestamp, int64_t ts, unsigned long  nSeq, std::vector<CarInfo>& vResult)
{
    printf("====in CDspMoveDetect::DetectCarNum====\n");

        CarInfo carnums;
		int nCarNumResult=0;
		char tmp[32] = {0};
		IplImage* tmpimg = NULL;

		CvRect rect = rtROI;//检测区域

		carnum_context vehicle_result[CARNUMSIZE];
		road_context context;


		img->ID = m_nChannelId;

		loop_parmarer LoopParmarer;
		LoopParmarer.pStart_point = cvPoint( 0, 0 );
        LoopParmarer.pEnd_point = cvPoint( 0, 0 );

printf("==before=mvcarnumdetect.find_carnum========\n");

        printf("===rect.width=%d=rect.height=%d====\n", rect.width, rect.height);
		//车牌检测
		if(rect.width > 0 && rect.height > 0)
		{
		    try
		    {
		        printf("===find_carnum ING...=\n");
		        printf("==img->ID=%d===img=%x==\n", img->ID, img);
                nCarNumResult = mvcarnumdetect.find_carnum(NULL,img,tmp,&tmpimg,rect,1,vehicle_result,&context,NULL,LoopParmarer);
		    }
		    catch(...)
		    {
		        printf("==mvcarnumdetect.find_carnum=ERROR!!!==\n");
		    }
		    printf("==nCarNumResult=%d==\n", nCarNumResult);
		}


printf("=====2222==DetectCarNum====\n");
		 if(tmpimg!=NULL)
		{
			cvReleaseImage(&tmpimg);
			tmpimg = NULL;
		}


	    printf("======nCarNumResult=%d====\n", nCarNumResult);

		if(nCarNumResult > 0)
		{

printf("=====44444==DetectCarNum====\n");
				for(int i = 0; i<nCarNumResult; i++)
				{
                    memcpy(carnums.strCarNum,vehicle_result[i].carnum,7);
                    memcpy(carnums.wj,vehicle_result[i].wjcarnum,2);
                    carnums.color = vehicle_result[i].color;
                    carnums.vehicle_type=vehicle_result[i].vehicle_type;

                    carnums.ix      = vehicle_result[i].position.x;
                    carnums.iy      = (vehicle_result[i].position.y);
                    carnums.iwidth  = vehicle_result[i].position.width;
                    carnums.iheight = (vehicle_result[i].position.height);

                    carnums.iscarnum= vehicle_result[i].iscarnum;
                    carnums.mean    = vehicle_result[i].mean;
                    carnums.stddev  = vehicle_result[i].stddev;

                    carnums.smearCount  = vehicle_result[i].smearnum;
                    memcpy(carnums.smear,vehicle_result[i].smearrect,sizeof(CvRect)*(carnums.smearCount));

                    carnums.VerticalTheta=vehicle_result[i].VerticalTheta;
                    carnums.HorizontalTheta=vehicle_result[i].HorizontalTheta;

                    carnums.uTimestamp = uTimestamp;
                    carnums.ts      = ts;
                    carnums.uSeq   = nSeq;
                    carnums.carnumrow = vehicle_result[i].carnumrow;//车牌结构


                    carnums.nDirection = m_nDetectDirection;

                    vResult.push_back(carnums);
				}//End of for(int i = 0; i<nCarNumResult; i++)

		}
		else if(nCarNumResult == 0)
		{
			//LogNormal("in DspMoveDetect.cpp line2442 no carNum");
			carnums.ix = 100;
			carnums.iy = 100;
			carnums.iwidth = 100;
			carnums.iheight = 100;

			memcpy(carnums.strCarNum,"*******",8);
			carnums.uTimestamp = uTimestamp;

			carnums.ts = ts;
			carnums.uSeq = nSeq;
			vResult.push_back(carnums);
		}
		else
		{
		}
    printf("====out CDspMoveDetect::DetectCarNum====\n");
}


//获取图像坐标到世界坐标的变换矩阵
void CDspMoveDetect::mvfind_homography(float *image, float *world)
{
    //only use the first 4 points to calculate homography
    CvMat src_points = cvMat(4,2,CV_32FC1,image);
    CvMat dst_points = cvMat(4,2,CV_32FC1,world);

    CvMat *homo = cvCreateMat(3,3,CV_32FC1);
    cvFindHomography(&src_points, &dst_points, homo);//src*H=dst

    int i,j;
    for(i=0; i<3; i++) //存到数组里，transform时更快
    {
        for(j=0; j<3; j++)
        {
            homography_image_to_world[i][j] = cvmGet(homo,i,j);
            //printf("%.2f ",homography_image_to_world[i][j]);
        }
        //printf("\n");
    }
    //printf("\n");
    cvReleaseMat(&homo);
}

//载入通道设置(0:目标检测读取配置;1:车牌检测读取配置)
bool CDspMoveDetect::LoadRoadSettingInfo(int nType,vector<mvvideostd>& vListStabBack)
{
    LIST_CHANNEL_INFO list_channel_info;
    CXmlParaUtil xml;
    if(xml.LoadRoadSettingInfo(list_channel_info,m_nChannelId))
    {
        float image_cord[12];
        float world_cord[12];
        CvPoint pt,pt1,pt2;
        CvPoint2D32f ptImage;

        m_vtsObjectRegion.clear();

        m_cnpMap.clear();

        LIST_CHANNEL_INFO::iterator it_b = list_channel_info.begin();
        LIST_CHANNEL_INFO::iterator it_e = list_channel_info.end();

        bool bLoadCalibration = false;
        bool bLoadCardArea = false;


        std::list<REGION_PROPERTY>::iterator it_rb;
        std::list<REGION_PROPERTY>::iterator it_re;

        Point32fList::iterator it_begin;
        Point32fList::iterator it_end;

        Point32fList::iterator it_32fb;
        Point32fList::iterator it_32fe;

        Point32fList::iterator item_b;
        Point32fList::iterator item_e;

        while(it_b != it_e)
        {
            int i = 0;
            int j = 0;
            CHANNEL_INFO channel_info = *it_b;
            ChannelRegion  vtsRegion;

            carnum_parm_t cnp;
            //车道编号
            vtsRegion.nRoadIndex = channel_info.chProp_index.value.nValue;

            //车道逻辑编号
            vtsRegion.nVerRoadIndex = channel_info.chProp_name.value.nValue;
            //车道方向
            int nDirection = channel_info.chProp_direction.value.nValue;
            if(nDirection < 180)
            {
                vtsRegion.nDirection = 0;
                cnp.direction = 0;
            }
            else
            {
                vtsRegion.nDirection = 1;
                cnp.direction = 1;
            }
            vtsRegion.vDirection.start.x = channel_info.chProp_direction.ptBegin.x;
            vtsRegion.vDirection.start.y = channel_info.chProp_direction.ptBegin.y;
            vtsRegion.vDirection.end.x = channel_info.chProp_direction.ptEnd.x;
            vtsRegion.vDirection.end.y = channel_info.chProp_direction.ptEnd.y;

            //标定区域
            if(!bLoadCalibration)
            {
                printf("channel_info.calibration.length=%f,channel_info.calibration.width=%f\n",channel_info.calibration.length,channel_info.calibration.width);

                //矩形区域（4个点）
                it_begin = channel_info.calibration.region.listPT.begin();
                it_end = channel_info.calibration.region.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    image_cord[2*i] = (it_begin->x)*m_ratio_x;
                    image_cord[2*i+1] = (it_begin->y)*m_ratio_y;

                    if(i==0)
                    {
                        world_cord[2*i] = 0;
                        world_cord[2*i+1] = 0;
                    }
                    else if(i==1)
                    {
                        world_cord[2*i] = channel_info.calibration.length;
                        world_cord[2*i+1] = 0;
                    }
                    else if(i==2)
                    {
                        world_cord[2*i] = channel_info.calibration.length;
                        world_cord[2*i+1] = channel_info.calibration.width;
                    }
                    else if(i==3)
                    {
                        world_cord[2*i] = 0;
                        world_cord[2*i+1] = channel_info.calibration.width;
                    }
                    printf("image_cord[2*i]=%f,image_cord[2*i+1]=%f,world_cord[2*i]=%f,world_cord[2*i+1]=%f\n",image_cord[2*i],image_cord[2*i+1],world_cord[2*i],world_cord[2*i+1]);
                    i++;
                }
                //printf("channel_info.calibration.length=%f,channel_info.calibration.width=%f\n",channel_info.calibration.length,channel_info.calibration.width);
                //辅助标定点
                it_begin = channel_info.calibration.listPT.begin();
                it_end = channel_info.calibration.listPT.end();

                it_32fb = channel_info.calibration.list32fPT.begin();
                it_32fe = channel_info.calibration.list32fPT.end();
                while(it_begin!=it_end&&it_32fb!=it_32fe)
                {
                    //image cor
                    image_cord[2*i] = (it_begin->x)*m_ratio_x;
                    image_cord[2*i+1] = (it_begin->y)*m_ratio_y;

                    //world cor
                    world_cord[2*i] = it_32fb->x;
                    world_cord[2*i+1] = it_32fb->y;

                    printf("image_cord[2*i]=%f,image_cord[2*i+1]=%f,world_cord[2*i]=%f,world_cord[2*i+1]=%f\n",image_cord[2*i],image_cord[2*i+1],world_cord[2*i],world_cord[2*i+1]);


                    it_32fb++;
                    it_begin++;
                    i++;
                }


                mvfind_homography(image_cord,world_cord);//基于帧图象


                bLoadCalibration = true;
            }
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
                        pt.x = (int) (item_b->x*m_ratio_x + 0.5);
                        pt.y = (int) (item_b->y*m_ratio_y + 0.5);
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
                    }

                    {
                        //设置车牌检测区域
                        m_rtCarnumROI.x = pt1.x;
                        m_rtCarnumROI.y = pt1.y;
                        m_rtCarnumROI.width = pt2.x - pt1.x;
                        m_rtCarnumROI.height = pt2.y - pt1.y;

						m_rtVlpRoi = m_rtCarnumROI;
                        //printf("m_rtCarnumROI.x=%d,m_rtCarnumROI.y=%d,m_rtCarnumROI.width=%d,m_rtCarnumROI.height=%d\n",m_rtCarnumROI.x,m_rtCarnumROI.y,m_rtCarnumROI.width,m_rtCarnumROI.height);
                    }
                }
                bLoadCardArea = true;
            }

                //车道区域
                it_begin = channel_info.chRegion.listPT.begin();
                it_end = channel_info.chRegion.listPT.end();
                for(; it_begin != it_end; it_begin++)
                {
                    CvPoint   ptChanRgn;
                    ptChanRgn.x = it_begin->x*m_ratio_x;
                    ptChanRgn.y = it_begin->y*m_ratio_y;
                    vtsRegion.vListChannel.push_back(ptChanRgn);
                    printf("ptChanRgn.x=%d,ptChanRgn.y=%d\n",ptChanRgn.x,ptChanRgn.y);

                    if( (ptChanRgn.x > m_imgSnap->width) || (ptChanRgn.y > m_imgSnap->height))
                    {
                        printf("==ptChanRgn.x=%d==ptChanRgn.y=%d==m_imgSnap->width=%d=m_imgSnap->height=%d===\n",\
                                ptChanRgn.x, ptChanRgn.y, m_imgSnap->width, m_imgSnap->height);
                        return false;
                    }
                }

                m_vtsObjectRegion.push_back(vtsRegion);

                m_cnpMap.insert(CnpMap::value_type(vtsRegion.nRoadIndex,cnp));

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



//设置检测类型
void CDspMoveDetect::SetDetectKind(CHANNEL_DETECT_KIND nDetectKind)
{
    LogNormal("SetDetectKind=nDetectKind=0x:%x=\n", nDetectKind);
    if(m_nDetectKind != nDetectKind)
    {
        bool bNeedReloadROI = false;
        CHANNEL_DETECT_KIND preDetectKind = m_nDetectKind;


        /////////////闯红灯检测
        if((preDetectKind&DETECT_VTS)==DETECT_VTS)//原先需要进行闯红灯检测
        {
            if((nDetectKind&DETECT_VTS)!= DETECT_VTS)//现在不需要进行闯红灯检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_VTS)!= DETECT_VTS)//原先不需要进行闯红灯检测
        {
            if((nDetectKind&DETECT_VTS)==DETECT_VTS)//现在需要进行闯红灯检测
            {
                bNeedReloadROI = true;
            }
        }


		/////////////线圈检测
        if((preDetectKind&DETECT_LOOP)==DETECT_LOOP)//原先需要进行线圈检测
        {
            if((nDetectKind&DETECT_LOOP)!= DETECT_LOOP)//现在不需要进行线圈检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_LOOP)!= DETECT_LOOP)//原先不需要进行线圈检测
        {
            if((nDetectKind&DETECT_LOOP)==DETECT_LOOP)//现在需要进行线圈检测
            {
                bNeedReloadROI = true;
            }
        }


        //车身颜色检测
        if((preDetectKind&DETECT_CARCOLOR)==DETECT_CARCOLOR)//原先需要进行车身颜色检测
        {
            if((nDetectKind&DETECT_CARCOLOR)!= DETECT_CARCOLOR)//现在不需要进行车身颜色检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_CARCOLOR)!= DETECT_CARCOLOR)//原先不需要进行车身颜色检测
        {
            if((nDetectKind&DETECT_CARCOLOR)==DETECT_CARCOLOR)//现在需要进行车身颜色检测
            {
                bNeedReloadROI = true;
            }
        }

        //车辆特征检测
        if((preDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE)//原先需要进行车辆特征检测
        {
            if((nDetectKind&DETECT_TEXTURE)!= DETECT_TEXTURE)//现在不需要进行车辆特征检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_TEXTURE)!= DETECT_TEXTURE)//原先不需要进行车辆特征检测
        {
            if((nDetectKind&DETECT_TEXTURE)==DETECT_TEXTURE)//现在需要进行车辆特征检测
            {
                bNeedReloadROI = true;
            }
        }

        //车型检测
        if((preDetectKind&DETECT_TRUCK)==DETECT_TRUCK)//原先需要进行车型检测
        {
            if((nDetectKind&DETECT_TRUCK)!= DETECT_TRUCK)//现在不需要进行车型检测
            {
                bNeedReloadROI = true;
            }
        }
        else if((preDetectKind&DETECT_TRUCK)!= DETECT_TRUCK)//原先不需要进行车型检测
        {
            if((nDetectKind&DETECT_TRUCK)==DETECT_TRUCK)//现在需要进行车型检测
            {
                bNeedReloadROI = true;
            }
        }

        m_nDetectKind = nDetectKind;
        if(bNeedReloadROI)
        {
            m_bReloadROI = true;
        }
    }
}

//设置白天晚上还是自动判断
void CDspMoveDetect::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
{
    if (dType == DETECT_AUTO)
    {
        m_nDetectTime = DETECT_AUTO;
    }
    else if (dType == DETECT_DAY)
    {
        m_nDetectTime = DETECT_DAY;
        m_nDayNight =1;
    }
    else
    {
        m_nDetectTime = DETECT_NIGHT;
        m_nDayNight =0;
    }
}

//获取图片路径并将图片编号存储在数据库中
int CDspMoveDetect::GetPicPathAndSaveDB(std::string& strPicPath)
{

    pthread_mutex_lock(&g_Id_Mutex);
    ////////////////////
    //需要判断磁盘是否已经满
    g_FileManage.CheckDisk(false,false);

    //存储大图片
    strPicPath  = g_FileManage.GetPicPath();

    int nSaveRet = g_skpDB.SavePicID(g_uPicId);
    //解锁
    pthread_mutex_unlock(&g_Id_Mutex);

    //删除已经存在的记录
    g_skpDB.DeleteOldRecord(strPicPath,false,false);


    return nSaveRet;
}


//获取车身位置
CvRect CDspMoveDetect::GetCarPos(RECORD_PLATE plate)
{
    int x = 0;
    int y = 0;
    int w = 0;//宽度
    int h = 0;//高度

    CvRect rtCar;

    if(plate.chText[0] != '*')//有牌车
    {
        w = plate.uPosRight - plate.uPosLeft;//宽度
        h = plate.uPosBottom - plate.uPosTop;//高度
        int dw = 2*w;
        if(plate.uType == SMALL_CAR)
        {
            dw = 3*w;
            rtCar.width = 7*w;
        }
        else
        {
            dw = 4*w;
            rtCar.width = 9*w;
        }
        rtCar.height = rtCar.width;
        x = plate.uPosLeft-dw;

        if(m_nDetectDirection == 0)
        {
            y = plate.uPosTop-rtCar.height+6*h;
        }
        else
        {
            y = plate.uPosTop-rtCar.height+8*h;
        }
    }
    else
    {
        CvPoint point;
        point.x = (plate.uPosRight + plate.uPosLeft)/2.0;
        point.y = (plate.uPosBottom + plate.uPosTop)/2.0;

        if(m_imgSnap->width > 2000)
        {
            rtCar.width = 800;
            rtCar.height = 800;
        }
        else
        {
            rtCar.width = 500;
            rtCar.height = 500;
        }

        x = point.x - rtCar.width/2;
        y = point.y - rtCar.height/2;
    }

    if(x > 0)
    {
        rtCar.x = x;
    }
    else
    {
        rtCar.x = 0;
    }

    int nExtentHeight = 0;
    if(m_nWordPos == 1)
    {
        nExtentHeight = m_nExtentHeight;
    }

    if(y > nExtentHeight)
    {
        rtCar.y = y;
    }
    else
    {
        rtCar.y = nExtentHeight;
    }

    //printf("0000========%d==%d,%d,%d,%d=============================\n",plate.uPosLeft-3*w,rtCar.x,rtCar.y,rtCar.width,rtCar.height);


    if(rtCar.x+rtCar.width>=m_imgSnap->width)
    {
        rtCar.x = m_imgSnap->width - rtCar.width-1;
    }

    if(rtCar.y+rtCar.height>=m_imgSnap->height)
    {
        rtCar.y = m_imgSnap->height - rtCar.height-1;
    }

    return rtCar;
}

//获取车型细分类型
int CDspMoveDetect::GetTypeDetail(int nRet)
{
    int nRetType = NO_JUDGE;
    if(nRet == 1)
    {
        nRetType = BUS_TYPE;
    }
    else if(nRet == 2)
    {
        nRetType = TRUCK_TYPE;
    }
    else if(nRet == 3)
    {
        nRetType = MIDDLEBUS_TYPE;
    }
    else if(nRet == 4)
    {
        nRetType = TAXI;//OTHER_TYPE;
    }
    else if(nRet == 5)
    {
        nRetType = NO_JUDGE;
    }
    else if(nRet == 6)
    {
        nRetType = WRONG_POS;
    }
    else if(nRet == 7)
    {
        nRetType = MINI_TRUCK;
    }

    return nRetType;
}


//对输入车牌进行处理
bool CDspMoveDetect::OnDspDetect(listPicData& listPic)
{
	printf("==in==CDspMoveDetect::OnDspDetect===\n");

	//设置车牌检测参数
    carnum_parm_t cnp;
    CnpMap::iterator it = m_cnpMap.begin();
    if(it != m_cnpMap.end())
    {
        cnp = it->second;
    }
    cnp.isday = m_nDayNight;
    mvcarnumdetect.set_carnum_parm(&cnp);

    printf("=========listPic.size()=%d=\n", listPic.size());

    if(listPic.size() < 1)
    {
        //printf("=========listPic.size()=%d,listLoop.size()=%d=\n", listPic.size(),listLoop.size());
        return false;
    }
    else
    {

    }

	std::string result;
	listPicData::iterator it_b = listPic.begin();
	result = *it_b;

	//车牌记录
	std::vector<CarInfo> vResult;

	PLATEPOSITION plateposition[6]; //记录时间

//===============第一张图做卡口处理======begin=============================================//
	//解码Jpg->RGB
    yuv_video_buf* buf = (yuv_video_buf*)result.c_str();
    Image_header_dsp *img_header = (Image_header_dsp*)(result.c_str()+sizeof(yuv_video_buf));
    int nSizeInput = buf->size - sizeof(Image_header_dsp); //光Jpg图大小

    printf("====buf->size=%d==img_header->nSize=%d==nSizeInput=%d===\n", buf->size, img_header->nSize, nSizeInput);

    int nSize = 0; //输出解码后图像大小

    //IplImage* m_img_Tmp = cvCreateImageHeader(cvSize(buf->width*2,buf->height*2),8,3);
    //DecodeJpgToRgb((BYTE*)result.c_str()+sizeof(yuv_video_buf)+sizeof(Image_header_dsp), \
    //               buf->width, buf->height, nSizeInput, nSize, (BYTE*)m_img_Tmp->imageData);

    //BYTE * img_Temp = new BYTE[buf->width * buf->height * 3];

    //DecodeJpgToRgb((BYTE*)result.c_str()+sizeof(yuv_video_buf)+sizeof(Image_header_dsp), \
    //               buf->width, buf->height, nSizeInput, nSize, img_Temp);

    printf("==m_img->width=%d=m_img->height=%d===\n", m_img->width, m_img->height);
    DecodeJpgToRgb((BYTE*)result.c_str()+sizeof(yuv_video_buf)+sizeof(Image_header_dsp), \
                buf->width, buf->height, nSizeInput, nSize, (BYTE*)m_img->imageData);

    printf("=after====DecodeJpgToRgb===\n");



printf("===CDspMoveDetect::OnDspDetect=before==DetectCarNum==\n");
    CvRect rtCarnumROI;
    GetCarnumROIByRoadIndex(img_header->nRoadIndex, rtCarnumROI);


    printf("=x:%d,y:%d,width:%d,height:%d==\n", m_rtCarnumROI.x, m_rtCarnumROI.y, m_rtCarnumROI.width, m_rtCarnumROI.height);
    printf("=x:%d,y:%d,width:%d,height:%d==\n", rtCarnumROI.x, rtCarnumROI.y, rtCarnumROI.width, rtCarnumROI.height);
	//检测车牌
    DetectCarNum(m_img, rtCarnumROI, buf->uTimestamp, buf->ts, buf->nSeq, vResult);

    if(vResult.size() >= 1)
    {
        //LogNormal("==CDspMoveDetect=vResult.size()=%d=buf->nSeq=%d\n", vResult.size(), buf->nSeq);
    }
    else
    {
    }


printf("===CDspMoveDetect::OnDspDetect=after==DetectCarNum==\n");

	//计算车速
	/*if(listLoop.size() >= 2)
	{
		listPicData::iterator it_p1 = listLoop.begin();
		listPicData::iterator it_p2 = it_p1++;

	    yuv_video_buf* pHeader1 = (yuv_video_buf*)it_p1->c_str();
		Image_header_dsp* pDspImageHeader1 = (Image_header_dsp*)(it_p1->c_str()+sizeof(yuv_video_buf));

		yuv_video_buf* pHeader2 = (yuv_video_buf*)it_p2->c_str();
		Image_header_dsp* pDspImageHeader2 = (Image_header_dsp*)(it_p2->c_str()+sizeof(yuv_video_buf));

		LoopParaMap::iterator it_p = m_LoopParaMap.find(pDspImageHeader1->nRoadIndex);

		if(it_p != m_LoopParaMap.end())
		{
			float fDistance = it_p->second.fDistance;

			int64_t dt = fabs(pHeader2->ts - pHeader1->ts);

			if(dt > 0)
			carnums.vy = fDistance*(1e6)/dt;
		}
	}
	*/
//===============第一张图做卡口处理======END=============================================//


    //for(std::vector<CarInfo>::iterator it_b_car = vResult.begin(); it_b_car != vResult.end(); it_b_car++)

    CarInfo carnums;
    carnums.RoadIndex = img_header->nRoadIndex;
    int nResultsCount = vResult.size();
    std::vector<CarInfo>::iterator it_b_car = vResult.begin();
    do
    {
            //if(listLoop.size() >= 1)
           //{
                if(nResultsCount > 0)
                {
                    carnums = *it_b_car;
                    carnums.RoadIndex = img_header->nRoadIndex;
					//carnums.vx = 3;//暂时固定
					//carnums.vy = 4;//暂时固定
					//carnums.wx = 0.2000; //暂时固定
					//carnums.wy = 0.1000; //暂时固定
                }
                else//对于未检测出车牌的也需要输出
                {
                   // LogNormal("=====No Car Num=====\n");
                    memset(carnums.strCarNum,0,8);
                    memcpy(carnums.strCarNum,"*******",7);
                    carnums.uTimestamp = buf->uTimestamp;
                    carnums.ts      = buf->ts;
                    carnums.uSeq   = buf->nSeq;

                    /*carnums.ix      = 100;
                    carnums.iy      = 100;
                    carnums.iwidth  = 50;
                    carnums.iheight = 50;*/

                    carnums.ix = rtCarnumROI.x + rtCarnumROI.width * 0.75f;
                    carnums.iy = rtCarnumROI.y;
                    carnums.iwidth  = 100;
                    carnums.iheight = 50;
                }
/*
                printf("=in==cacu Speed==listLoop.size()=%d==\n", listLoop.size());

                listPicData::iterator it_p1 = listLoop.begin();

                yuv_video_buf* pHeader1 = (yuv_video_buf*)it_p1->c_str();
                Image_header_dsp* pDspImageHeader1 = (Image_header_dsp*)(it_p1->c_str()+sizeof(yuv_video_buf));
                InductorInfoForPC* pLoopInfo = (InductorInfoForPC*)(it_p1->c_str()+sizeof(yuv_video_buf)+sizeof(Image_header_dsp));
                printf("===plateposition[0].uTimestamp=%lld=\n", plateposition[0].uTimestamp);


                #ifdef ROSEEK_SAVE_INFO
                                FILE *fpOut = NULL;
                                char jpg_name[256] = "./text/Seq_DSP_order.txt";
                                char charContant[512] = {0};
                                //if(pDsp_header->nType == 1)
                                {
                                    //sprintf(jpg_name, "./text/Seq_DSP_ORDER.txt");
                                    sprintf(charContant, "seq:%lld==order:%d==\n", pHeader1->nSeq, pHeader1->nFrameRate);
                                    fpOut = fopen(jpg_name, "a+");
                                    fwrite(charContant, 1, sizeof(charContant), fpOut);
                                    fclose(fpOut);
                                }
                #endif

                LoopParaMap::iterator it_p = m_LoopParaMap.find(pDspImageHeader1->nRoadIndex);

                printf("====pDspImageHeader1->nRoadIndex=%d===m_LoopParaMap.size()=%d===\n", pDspImageHeader1->nRoadIndex, m_LoopParaMap.size());

                if(it_p != m_LoopParaMap.end())
                {
                    float fDistance = it_p->second.fDistance;
                    printf("===fDistance=%f=\n", fDistance);

                    float dt = 0;

                    printf("====pLoopInfo->Inductor1edge1TickCount=%lld====\n", pLoopInfo->Inductor1edge1TickCount);
                    printf("====pLoopInfo->Inductor1edge2TickCount=%lld====\n", pLoopInfo->Inductor1edge2TickCount);
                    printf("====pLoopInfo->Inductor2edge1TickCount=%lld====\n", pLoopInfo->Inductor2edge1TickCount);
                    printf("====pLoopInfo->Inductor2edge2TickCount=%lld====\n", pLoopInfo->Inductor2edge2TickCount);

                    //float dt1 = fabs( (pLoopInfo->Inductor1edge1TickCount*0.001f) - (pLoopInfo->Inductor1edge2TickCount*0.001f) ); //单位ms毫秒
                    //float dt2 = fabs( (pLoopInfo->Inductor2edge1TickCount*0.001f) - (pLoopInfo->Inductor2edge2TickCount*0.001f) );

                    float dt1 = fabs( (pLoopInfo->Inductor1edge2TickCount*0.001f) - (pLoopInfo->Inductor2edge2TickCount*0.001f) ); //单位ms毫秒
                    float dt2 = fabs( (pLoopInfo->Inductor2edge1TickCount*0.001f) - (pLoopInfo->Inductor1edge1TickCount*0.001f) );

                    printf("=====dt1=%f====dt2=%f===\n", dt1, dt2);

                    if(dt1 > 0.000001f && dt2 > 0.00001f)
                    {
                        printf("==dt1 > 0 && dt2 > 0=\n");
                        dt = dt1*0.5f + dt2*0.5f;
                        //dt = dt1;
                    }
                    else if(dt1 > 0.000001f)
                    {
                        dt = dt1;
                    }
                    else if(dt2 > 0.000001f)
                    {
                        dt = dt2;
                    }
                    else
                    {
                        //...
                    }

                    if(dt > 0.000001f)
                    {
                        printf("=fDistance=%f=dt=%f=====\n", fDistance, dt);
                        float fTemp1 = dt * 0.001f; //时间m秒
                        printf("===fTemp1=%f=\n", fTemp1);
                        carnums.vy = (fDistance*3.6f)/fTemp1;

                        //carnums.vy = fDistance/(dt*(1e9));
                        //carnums.vy = fDistance*(1e9)/dt;

                        printf("==carnums.vy=%f===\n", carnums.vy);
                    }
                }*/
            //}

            printf("=####8888####=before DIANJING====listPic.size()=%d===\n", listPic.size());

            if(listPic.size() > 0 )
            {
                //1.卡口数据-输出
                {
                    printf("========333###=====CDspMoveDetect::OnDspDetect====KAKOU===\n");
                    cvSet(m_imgSnap, cvScalar( 0,0,0));

                    CvRect rt;
                    rt.x = 0;
                    rt.y = 0;
                    rt.width = m_img->width;
                    rt.height = m_img->height;

                    if(m_nWordPos == 1 && m_nWordOnPic == 0)
                    {
                        rt.y += m_nExtentHeight;
                    }

                    printf("===CDspMoveDetect::OnDspDetect==\n");

                printf("===before==cvSetImageROI=\n");
                    cvSetImageROI(m_imgSnap,rt);

                    cvCopy(m_img,m_imgSnap);
                    cvResetImageROI(m_imgSnap);

                    //cvCopyImage(m_img, m_imgSnap);

                printf("===after==cvSetImageROI=\n");
/*
                //存图测试--begin
                BYTE* pOutImage = NULL;
                pOutImage = new BYTE[m_img->width*m_img->height/4];

                printf("m_imgSnap->width=%d=m_imgSnap->height=%d\n", m_imgSnap->width, m_imgSnap->height);
                printf("==g_PicFormatInfo.nJpgQuality=%d", g_PicFormatInfo.nJpgQuality);

                char jpg_name[256] = {0};
                //sprintf(jpg_name, "./text/264-img-hea.avi");
                sprintf(jpg_name, "./text/%d_TT_seq.jpg", buf->nSeq);
                CxImage image;
                int srcstep = 0;
                //if(image.IppEncode((BYTE*)m_img->imageData, m_img->width, m_img->height, 3, &srcstep, pOutImage, g_PicFormatInfo.nJpgQuality))
                if(image.IppEncode((BYTE*)m_imgSnap->imageData, m_imgSnap->width, m_imgSnap->height, 3, &srcstep, pOutImage, g_PicFormatInfo.nJpgQuality))
                {
                    FILE* fp = NULL;
                    fp = fopen(jpg_name, "a");
                    if(fp!=NULL)
                    {
                        fwrite(pOutImage, srcstep, 1, fp);
                    }
                }
                printf("=After save image...==\n");
                //存图测试--end
                */

                    //LogNormal("==before==CarNumOutPut====\n");
                    //输出卡口记录
					carnums.id = img_header->uCameraId;
                    CarNumOutPut(carnums);
                    //LogNormal("==after==CarNumOutPut====\n");
                }
			}


        if(it_b_car != vResult.end())
        {
            it_b_car++;
        }
        else
        {
            LogNormal("==End of vResult.end...!\n");
        }


    }while(it_b_car != vResult.end());//End of do


    //LogNormal("Out=OnDspDetect=\n");

    return true;
}

//解码jpg->rgb
bool CDspMoveDetect::DecodeJpgToRgb(BYTE *pBuf, const int nWidth, const int nHeight, const int nSizeInput, int &srcstep, BYTE *pBufOut)
{
    printf("==CDspMoveDetect::DecodeJpgToRgb==nWidth=%d====nHeight=%d===\n", nWidth, nHeight);

    //jpg->rgb解码
    if(pBuf != NULL)
    {
        CxImage image;

        int lineStep = nWidth*3 + IJL_DIB_PAD_BYTES(nWidth,3);
        srcstep = lineStep * nHeight;

        //yuv_video_buf* header = (yuv_video_buf*)pBuf;

        //printf("====header->size=%d===\n", header->size);

        printf("===before=Decode==nSizeInput=%d=\n", nSizeInput);
        //先解码jpg->rgb
        image.Decode(pBuf, nSizeInput, 3); //解码
        //printf("==image.GetBits()=%x====\n", image.GetBits());

        printf("===after=Decode==!!\n");

        if(image.GetSrcSize() > 100)
        {
            //memcpy(pBufOut, image.GetBits(), srcstep);
            memcpy(pBufOut, image.GetBits(), image.GetSrcSize());
            srcstep = image.GetSrcSize();
        }
        else
        {
            printf("====Decode=ERROR!!!=====\n");
        }

        printf("==CDspMoveDetect::DecodeJpgToRgb==nSizeInput=%d=image.GetSrcSize()=%d===srcstep=%d=\n", nSizeInput, image.GetSrcSize(), srcstep);
    }
    else
    {
        return false;
    }

    return true;
}

//是否输出结果判断
bool CDspMoveDetect::ToBeOutPut(listPicData &listInput)
{
    //printf("====in CDspMoveDetect::ToBeOutPut==listInput.size()=%d=\n", listInput.size());
    bool bRet = false;
    bool bFlagKa, bFlagLoop, bFlagDj;
    bFlagKa = false;
    bFlagLoop = false;
    bFlagDj = false;

	if(listInput.size() > 0)
	{
		return true;
	}

    if(listInput.size() < 2)
    {
        return false;
    }

    listPicData::iterator it_b = listInput.begin();
    listPicData::iterator it_e = listInput.end();
    Image_header_dsp* pDspImageHeader = NULL;

    while(it_b != it_e)
    {
        pDspImageHeader = (Image_header_dsp*)(it_b->c_str()+sizeof(yuv_video_buf));

        //printf("=111==nSeq=%d=pDspImageHeader->nSize=%d===\n", pDspImageHeader->nSeq, pDspImageHeader->nSize);

        //printf("=22==88888888###===pDspImageHeader->nSeq=%d, pDspImageHeader->nCount=%d, pDspImageHeader->nOrder=%d=pDspImageHeader->nType=%d=\n", \
        //        pDspImageHeader->nSeq, pDspImageHeader->nCount, pDspImageHeader->nOrder, pDspImageHeader->nType);

        if(pDspImageHeader->nType == DSP_IMG_GATE_PIC)//卡口图片
        {
            //printf("======KAKOU==pDspImageHeader->nType=%d=\n", pDspImageHeader->nType);
            bFlagKa = true;
            /*if(it->second.size() == 2)//取出结果
            {
                bFind = true;
                break;
            }*/
        }
        else if(pDspImageHeader->nType == DSP_IMG_VTS_PIC || pDspImageHeader->nType == DSP_IMG_CONVERS_PIC)//电警图片
        {
            printf("======DIANJING==pDspImageHeader->nType=%d=\n", pDspImageHeader->nType);
            bFlagDj = true;
            /*if(it->second.size() >= 4)//取出结果
            {
                bFind = true;
                break;
            }*/
        }
        else if(pDspImageHeader->nType == DSP_IMG_LOOP_INFO)//线圈信息
        {
            printf("======LOOP==pDspImageHeader->nType=%d=\n", pDspImageHeader->nType);
            bFlagLoop = true;
        }
        else
        {
            /*
            printf("=222==nSeq=%d=pDspImageHeader->nSize=%d===\n", pDspImageHeader->nSeq, pDspImageHeader->nSize);
            if(it->second.size() >= 1)//取出单张图片或线圈信息
            {
                bFind = true;
                break;
            }
            */
            //printf("=======NOT===KAKOU====DIANJING====\n");
        }

        it_b++;
    }//End of while

    if(listInput.size() == 2)
    {
        if(bFlagLoop && bFlagKa)
        {
            printf("==oooOUT====KAKOU===\n");
            bRet = true;
        }
    }
    else if(listInput.size() == 4)
    {
        if(bFlagLoop && bFlagDj)
        {
            printf("==oooOUT====DIANJING===\n");
            bRet = true;
        }
    }

    return bRet;
}

//获取对应车道，车牌检测区域
bool CDspMoveDetect::GetCarnumROIByRoadIndex(int nRoadIndex, CvRect &rtDest)
{
    float fX = 0.0f;
    float fWidth = 0.0f;
    float fW1, fW2, fW3;
    fW1 = m_rtCarnumROI.width * 0.25f;
    fW2 = m_rtCarnumROI.width * 0.5f;
    fW3 = m_rtCarnumROI.width * 0.333f;
    //fW4 = m_rtCarnumROI.width * 1.0f;
    //int nRoadIndex = img_header->nRoadIndex;
    printf("==nRoadIndex=%d\n", nRoadIndex);
    switch(nRoadIndex)
    {
        case 3:
            {
                fX = m_rtCarnumROI.x;
                //fWidth = fW2;
                fWidth = fW3;
                break;
            }
        case 2:
            {
                fX = m_rtCarnumROI.x + fW1;
                fWidth = fW2;
                break;
            }
        case 1:
            {
                //fX = m_rtCarnumROI.x + fW2;
                fX = m_rtCarnumROI.x + m_rtCarnumROI.width * 0.6f;
                //fWidth = fW2;
                fWidth = m_rtCarnumROI.width * 0.4f;
                break;
            }
        default:
            {
                fX = m_rtCarnumROI.x;
                fWidth = m_rtCarnumROI.width;
            }
            break;
    }//End of switch
    //m_rtCarnumROI.x = (int)fX;
    //m_rtCarnumROI.width = (int)fWidth;

    rtDest.x = (int)fX;
    rtDest.y = (int)m_rtCarnumROI.y;
    rtDest.width = (int)fWidth;
    rtDest.height = m_rtCarnumROI.height;

    return true;
}

#endif //NOPLATE

#endif
