// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "DspDataProcess.h"
#include "Common.h"
#include "CommonHeader.h"
#include "ippi.h"
#include "ippcc.h"
#include "ximage.h"
#include "XmlParaUtil.h"
#include "RoadImcData.h"
#include "BrandSubSection.h"
#include "MatchPlateFortianjin.h"
#include "CarLabel.h"
//#include "MvFBMatch2.h"
#ifndef ALGORITHM_YUV

#ifndef DSP_GOJIAO_TEST
#define DSP_GOJIAO_TEST
#endif
//extern MvFBMatch2 g_MvFBMatch2;
/*
#ifndef VTS_TEST_SEQ
#define VTS_TEST_SEQ
#endif
*/

//构造
CDspDataProcess::CDspDataProcess(CDspDataManage* pDspManage)
{
	m_ServermapInPutResult.clear();
	m_pDspManage = pDspManage;

	//存取信号互斥
    pthread_mutex_init(&m_OutPutMutex,NULL);
	m_nOutPutThreadId = 0;
    m_nChannelId = 1;
    m_bConnect = false;
    m_imgSnap = NULL;
    m_imgComposeSnap = NULL;
	m_imgComposeMatch = NULL;
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
	m_bEndThread = false;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nDirection = 0;
#ifdef TWICE_DETECT
	m_imgVtsElem1 = NULL;
	m_imgVtsElem2 = NULL;
	m_imgVtsElem3 = NULL;
#endif

//#ifdef MATCH_LIU_YANG
	m_uPrevImgag = 0;
	m_uCurrImgtag = 0;
	pthread_mutex_init(&m_ImgTagMutex,NULL);
//#endif
	m_imgComposeResult = NULL;
	m_pMapRoadName = NULL;
	m_imgComposeMatchKaKou = NULL;
	m_imgComposeNoMatch = NULL;
	m_imgComposeVtsNoMatch = NULL;

	for(int i=0; i<MAX_IMG_MATCH_TAG_NUM; i++)
	{
		m_imgTagList[i].pImg = NULL;
	}
}

//析构
CDspDataProcess::~CDspDataProcess()
{
    ////存取信号互斥
    //pthread_mutex_destroy(&m_JpgFrameMutex);
	LogNormal("CDspDataProcess::~CDspDataProcess\n");
	Uninit();
	 //存取信号互斥
    pthread_mutex_destroy(&m_OutPutMutex);

//#ifdef MATCH_LIU_YANG
	pthread_mutex_destroy(&m_ImgTagMutex);
//#endif
}

bool CDspDataProcess::Init(int nChannelId,UINT32 uWidth,UINT32 uHeight,int nWidth, int nHeight, CHANNEL_DETECT_KIND nDetectKind)
{
    printf("=====CDspDataProcess::Init()====\n");
    printf("==nChannelId=%d==uWidth=%d==uHeight=%d=nWidth=%d==nHeight=%d=\n", nChannelId, uWidth, uHeight, nWidth, nHeight);
	m_nWidth = uWidth;
	m_nHeight = uHeight;
	m_nDetectKind = nDetectKind;

	SetDetectKind(m_nDetectKind);

	//LogNormal("CDspDataProcess id:%d m_nDetectKind:%d", nChannelId, m_nDetectKind);
	

    m_bReloadROI = true;

    m_nChannelId = nChannelId;

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
			/*if(g_nServerType == 4)//江宁电警
			{
				m_nExtentHeight = 100;
				nFontSize = 50;
			}
			else
			*/
			{
				nFontSize = g_PicFormatInfo.nFontSize;
			}
        }
        else
        {
            /*m_nExtentHeight = 80;
            nFontSize = 40;*/

			m_nExtentHeight = g_PicFormatInfo.nExtentHeight;
			nFontSize = g_PicFormatInfo.nFontSize;
        }
    }

	if (g_nServerType == 13)
	{
			//对于上海电警项目将字符高度固定
			if(uWidth < 2000)
			{
				m_nExtentHeight = 64;
			}
			else if(uWidth < 2500)
			{
				m_nExtentHeight = 96;
			}
			else 
			{
				m_nExtentHeight = 160;
			}
	}

		//m_pCurJpgImage = new BYTE[uWidth*uHeight/4];
        m_pComposeJpgImage = new BYTE[uWidth*uHeight];

		if(m_pComposeJpgImage == NULL)
		{
			LogNormal("new m_pComposeJpgImage fail!");
			return false;
		}

        m_imgSnap = cvCreateImage(cvSize(uWidth,uHeight+m_nExtentHeight),8,3);	

		LogNormal("m_imgSnap uWidth=%d,uHeight=%d\n",m_imgSnap->width,m_imgSnap->height);

		if(m_imgComposeSnap == NULL)
       {
                int uWidth = m_imgSnap->width;
				int uHeight = m_imgSnap->height;

				printf("---KKKKKK---m_imgSnap->width=%d, m_imgSnap->height=%d-\n", m_imgSnap->width, m_imgSnap->height);

				//////////
                printf("===CDspDataProcess::Init()=m_nWordOnPic=%d=g_nVtsPicMode=%d==uWidth=%d=uHeight=%d==m_nExtentHeight=%d=\n", \
					m_nWordOnPic, g_nVtsPicMode, uWidth, uHeight, m_nExtentHeight);
                //=CDspDataProcess::Init()===g_nVtsPicMode=0==uWidth=2448=uHeight=2128===

				m_imgComposeResult = cvCreateImage(cvSize(uWidth,uHeight),8,3);

#ifdef TWICE_DETECT
				//LogNormal("TWICE_DETECT init 111\n");
				
				m_imgVtsElem1 = cvCreateImage(cvSize(uWidth,uHeight),8,3);
				m_imgVtsElem2 = cvCreateImage(cvSize(uWidth,uHeight),8,3);
				m_imgVtsElem3 = cvCreateImage(cvSize(uWidth,uHeight),8,3);
				
				bool bInit = InitBusVioFilter();
				if(!bInit)
				{
					LogError("初始化占用公交道二次识别失败!\n");
					return false;
				}
				
				//LogNormal("TWICE_DETECT init 222\n");
#endif

    //            //闯红灯检测三帧合成图像

				if(g_nVtsPicMode == 0) //3x1
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth,(uHeight*3)),8,3);
				}
				else if(g_nVtsPicMode == 1) //2x2
                {
                    m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*2),8,3);
                }
                else if(g_nVtsPicMode == 2) //3x2
                {
                    m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)*3),8,3);
                }
				else if(g_nVtsPicMode == 3) //1x2
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth*2,(uHeight)),8,3);
				}
				else if(g_nVtsPicMode == 4) //1x1
				{
					m_imgComposeSnap = cvCreateImage(cvSize(uWidth,(uHeight)),8,3);
				}
				else if(g_nVtsPicMode == 5) //1x3
				{					
					if(g_nGongJiaoMode == 1)
					{
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*4,(uHeight)),8,3);
					}
					else
					{
						m_imgComposeSnap = cvCreateImage(cvSize(uWidth*3,(uHeight)),8,3);
					}
				}				
				else
				{
					LogNormal("err g_nVtsPicMode:%d \n", g_nVtsPicMode);
				}

				if(m_imgComposeSnap != NULL)
				{
					printf("m_nWordOnPic=%d,m_imgComposeSnap->height=%d,uHeight=%d,m_nExtentHeight=%d\n",\
						m_nWordOnPic,m_imgComposeSnap->height,uHeight,m_nExtentHeight);
				}
				else
				{
					return false;
				}				
        }

//#else ifdef  FBMATCHPLATE
		{
			int uWidth = m_imgSnap->width;
			int uHeight = m_imgSnap->height;
			//m_imgComposeMatch = cvCreateImage(cvSize(uWidth*3,(uHeight)),8,3);
//#ifdef MATCH_LIU_YANG
	#ifdef MATCH_LIU_YANG_DEBUG
			{
				LogNormal("CDspDataProcess::Init 2x2 !");

				m_imgComposeMatch = cvCreateImage(cvSize(uWidth*2,uHeight*2),8,3);
				for(int i=0; i<MAX_IMG_MATCH_TAG_NUM; i++)
				{
					m_imgTagList[i].pImg = cvCreateImage(cvSize(uWidth,uHeight),8,3);
				}
			}
	#endif //MATCH_LIU_YANG_DEBUG
	//#else
			if(1  == g_nDoSynProcess)	
			{
				//LogNormal("CDspDataProcess::Init 3x2 !");
				if (1 == g_nVtsPicMode) 
				{
					m_imgComposeMatch = cvCreateImage(cvSize(uWidth*2,uHeight*2),8,3);
				}
				else if (2 ==  g_nVtsPicMode) 
				{
					m_imgComposeMatch = cvCreateImage(cvSize(uWidth*2,uHeight*3),8,3);
				}
				for(int i=0; i<MAX_IMG_MATCH_TAG_NUM; i++)
				{
					m_imgTagList[i].pImg = cvCreateImage(cvSize(uWidth,uHeight),8,3);
				}

			LogNormal("CDspDataProcess::Init 1x3 !");

			LogNormal("m_imgComposeMatch uWidth=%d,uHeight=%d\n",m_imgComposeMatch->width,m_imgComposeMatch->height);


			m_imgComposeMatchKaKou = cvCreateImage(cvSize(uWidth*3,uHeight*1),8,3);

			m_imgComposeNoMatch = cvCreateImage(cvSize(uWidth*2,uHeight*1),8,3);

			m_imgComposeVtsNoMatch = cvCreateImage(cvSize(uWidth*2,uHeight*2),8,3);
			}
	//#endif //MATCH_LIU_YANG_DEBUG
//#else
//			//else
//			{
//				m_imgComposeMatch = cvCreateImage(cvSize(uWidth*2,(uHeight*2)),8,3);
//			}
//#endif	//MATCH_LIU_YANG
		}
//#endif	//MATCHPLATE

    //m_cvText.Init(nFontSize);

    //m_cvBigText.Init(80);


    printf("===m_bReloadROI=%d====\n", m_bReloadROI);
	
	for(int i = 0;i<3;i++)
	{
		m_nRandCode[i] = 0;
	}

	m_vtsResult.clear();

	m_pMapRoadName = new mapRoadName;

	//DoPopData();

    return true;
}

bool CDspDataProcess::Uninit()
{
    //线程结束标志
    m_bEndDetect = true;

    //释放文本资源
    //m_cvText.UnInit();
    //m_cvBigText.UnInit();

    //释放颜色库
    //m_carColor.color_Destroy();
    //m_carLabel.carLabel_Destroy();


	if(m_pCurJpgImage)
    {
        delete []m_pCurJpgImage;
        m_pCurJpgImage = NULL;
    }
	if(m_pComposeJpgImage)
	{
		delete []m_pComposeJpgImage;
		m_pComposeJpgImage = NULL;
	}
   
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

	 if(m_imgComposeMatch != NULL)
	 {
		 cvReleaseImage(&m_imgComposeMatch);
		 m_imgComposeMatch = NULL;
	 }

	 if (m_imgComposeMatchKaKou != NULL)
	 {
		 cvReleaseImage(&m_imgComposeMatchKaKou);
		 m_imgComposeMatchKaKou = NULL;
	 }
	 if (m_imgComposeNoMatch != NULL)
	 {
		 cvReleaseImage(&m_imgComposeNoMatch);
		 m_imgComposeNoMatch = NULL;
	 }
	 if (m_imgComposeVtsNoMatch != NULL)
	 {
		 cvReleaseImage(&m_imgComposeVtsNoMatch);
		 m_imgComposeVtsNoMatch = NULL;
	 }
	 if (m_imgComposeResult != NULL)
	 {
		 cvReleaseImage(&m_imgComposeResult);
		 m_imgComposeResult = NULL;
	 }
#ifdef TWICE_DETECT
	 m_OnBusVioFilter.mvOnBusVioValidUnInit();

	 if(m_imgVtsElem1 != NULL)
	 {
		 cvReleaseImage(&m_imgVtsElem1);
		 m_imgVtsElem1 = NULL;
	 }
	 if(m_imgVtsElem2 != NULL)
	 {
		 cvReleaseImage(&m_imgVtsElem2);
		 m_imgVtsElem2 = NULL;
	 }
	 if(m_imgVtsElem3 != NULL)
	 {
		 cvReleaseImage(&m_imgVtsElem3);
		 m_imgVtsElem3 = NULL;
	 }
#endif

//#ifdef MATCH_LIU_YANG
	 for(int i=0; i<MAX_IMG_MATCH_TAG_NUM; i++)
	 {
		 if (m_imgTagList[i].pImg != NULL)
		 {
			  cvReleaseImage(&m_imgTagList[i].pImg);
		 }		
		 m_imgTagList[i].pImg = NULL;
		 m_imgTagList[i].bUse = false;
	 }
//#endif

	 if(m_pMapRoadName)
	 {
		 delete m_pMapRoadName;
		 m_pMapRoadName = NULL;
	 }
    return true;
}

//保存全景图像
int CDspDataProcess::SaveImage(IplImage* pImg,std::string strPicPath,int nIndex)
{
	BYTE* pOutImage = NULL;
    pOutImage = m_pComposeJpgImage;

	IplImage* pSrcImg = pImg;
	
	//图像缩放
	IplImage* pImgResize = NULL;
	if(g_PicFormatInfo.nResizeScale < 100 && g_PicFormatInfo.nResizeScale > 0)
	{
		if (nIndex == 6 || nIndex == 4)//电警前后牌6合一 未匹配的4合一已先执行压缩
		{
			//pImgResize = cvCreateImage(cvSize(pImg->width*g_PicFormatInfo.nResizeScale/100.0,pImg->height*g_PicFormatInfo.nResizeScale/100.0),8,3);
			//cvResize(pImg,pImgResize);
			//pSrcImg = pImgResize;
		}
		else
		{
			if((m_imgComposeNoMatch->width != pImg->width) && (m_imgComposeNoMatch->height != pImg->height))//单张卡口不需要缩放
			{
				pImgResize = cvCreateImage(cvSize(pImg->width*g_PicFormatInfo.nResizeScale/100.0,pImg->height*g_PicFormatInfo.nResizeScale/100.0),8,3);
				cvResize(pImg,pImgResize);
				pSrcImg = pImgResize;
			}
		}	
	}

    CxImage image;
    int srcstep = 0;
    if(image.IppEncode((BYTE*)pSrcImg->imageData,pSrcImg->width,pSrcImg->height,3,&srcstep,pOutImage,g_PicFormatInfo.nJpgQuality))
    {

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
            std::string strWaterMark;
            GetWaterMark((char*)pOutImage,srcstep,strWaterMark);
            fwrite(strWaterMark.c_str(),strWaterMark.size(),1,fp);
            srcstep += strWaterMark.size();

            fclose(fp);
        }
        else
        {
            printf("canot open file\n");
        }
    }


	if(pImgResize != NULL)
	{
		cvReleaseImage(&pImgResize);
	}
	
    return srcstep;
}

//设置检测类型
void CDspDataProcess::SetDetectKind(CHANNEL_DETECT_KIND nDetectKind)
{
    //LogNormal("SetDetectKind=nDetectKind=0x:%x=\n", nDetectKind);
    if(m_nDetectKind != nDetectKind)
    {
        bool bNeedReloadROI = false;
        CHANNEL_DETECT_KIND preDetectKind = m_nDetectKind;

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
void CDspDataProcess::SetChannelDetectTime(CHANNEL_DETECT_TIME dType)
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
int CDspDataProcess::GetPicPathAndSaveDB(std::string& strPicPath)
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


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//添加车牌信息
bool CDspDataProcess::AddPlateFrame(BYTE* pBuffer)
{
	if(pBuffer != NULL)
    {		
			m_nDayNight = DayOrNight();
			m_nDayNightbyLight = DayOrNight(1);

			//RECORD_PLATE_DSP信息转换成CarInfo类型信息。
			RECORD_PLATE_DSP *pPlate = (RECORD_PLATE_DSP *)(pBuffer+sizeof(Image_header_dsp));

			Image_header_dsp * pHeader = (Image_header_dsp *)pBuffer;
			LogNormal("pCode=%s type=%d id=%d car:%s\n",\
				pHeader->szCameraCode,pHeader->nType,pHeader->uCameraId, pPlate->chText);
			//LogNormal("******platetype = %d\n",pPlate->uType);
			LogTrace("FindLog.txt", "OutDataM Out-[id,%d,type,%d,seq,%d, num:%s vio:%d camCode:%s]---", \
				pHeader->uCameraId, pHeader->nType, pHeader->nSeq, pPlate->chText, pPlate->uViolationType, pHeader->szCameraCode);

			CarInfo    carnums;
			char text[10] = "123456789";


			if(pPlate->chText[0] != 'L') //非武警牌照
			{
			    memset(text, 0, 10);
			    memcpy(text, pPlate->chText, 7);
			    memcpy(carnums.strCarNum, text, 7);
			}
			else
			{
			    memset(text, 0, 10);
			    memcpy(text, pPlate->chText, 9);
			    memcpy(carnums.strCarNum, text, 7);
			    memcpy(carnums.wj, text+7, 2); //武警牌下面的小数字
			}

			carnums.color = pPlate->uColor;
			carnums.vehicle_type = pPlate->uType;

			carnums.ix = (pPlate->uPosLeft);
			carnums.iy = (pPlate->uPosTop);
			carnums.iwidth = (pPlate->uPosRight - pPlate->uPosLeft);
			carnums.iheight = (pPlate->uPosBottom - pPlate->uPosTop);
			//LogNormal("-plate pos-seq:%d [%d,%d,%d,%d]-\n", \
				pPlate->uSeq, pPlate->uPosLeft, pPlate->uPosTop, pPlate->uPosRight, pPlate->uPosBottom);


			carnums.mean = -1;
			carnums.stddev = -1;

			carnums.VerticalTheta = pPlate->uVerticalTheta;
			carnums.HorizontalTheta = pPlate->uHorizontalTheta;

			//carnums.uTimestamp = pPlate->uTime;
			//carnums.ts = (int64_t)pPlate->uTime*1000000+pPlate->uMiTime*1000;
			carnums.uTimestamp = (pHeader->ts)/1000/1000;
			carnums.ts = pHeader->ts;
			carnums.uSeq = pPlate->uSeq;
			carnums.carnumrow = pPlate->uPlateType;

			carnums.wx = (double)((pPlate->uLongitude) * 0.000001);
			carnums.wy = (double)((pPlate->uLatitude) * 0.000001);

			//添加相机IDadd by wantao
			//carnums.id = pPlate->uCameraId;
			carnums.id = pHeader->uCameraId;

			carnums.vx = 0;
			carnums.vy = pPlate->uSpeed; //传入dsp相机线圈检测出速度
			carnums.RoadIndex = pPlate->uRoadWayID;

			carnums.m_CarWholeRec.x = pPlate->uCarPosLeft;
			carnums.m_CarWholeRec.y = pPlate->uCarPosTop;
			carnums.m_CarWholeRec.width = pPlate->uCarPosRight - pPlate->uCarPosLeft;
			carnums.m_CarWholeRec.height = pPlate->uCarPosBottom - pPlate->uCarPosTop;
			printf("11==DSP_SPEED=uSeq:%d uSpeed=%d,chText=%s,uViolationType=%d,id=%d,id1=%d id2=%d\n", \
				pPlate->uSeq, pPlate->uSpeed,pPlate->chText,pPlate->uViolationType, carnums.id, pHeader->uCameraId,pPlate->uCameraId);

#ifdef DSP_GOJIAO_TEST
			//测试取图用数据
			if(1 == g_nGongJiaoMode)
			{
				char szText[16] = "dTest88";
				if(0 == strcmp(carnums.strCarNum, szText))
				{
					//输出测试图片
					CarNumOutPutTest(carnums);
					return true;
				}					
			}
#endif
			if(pPlate->uViolationType == 99)
			{
				//LogNormal("卡口添加车牌信息成功帧号:%lld,相机编号:%d\n",carnums.uSeq,carnums.id);
				
				if (g_nGongJiaoMode == 1)
				{
					std::string strCameraCode = pHeader->szCameraCode;
					CarNumOutPut(carnums, strCameraCode);
				}
				else
				{
					CarNumOutPut(carnums);
				}				
				printf("******************YYYYYYYYYYYYY   卡口添加车牌信息成功\n");
			}
			else if(pPlate->uViolationType >= 0 && pPlate->uViolationType < 99)//电警数据
			{
						//LogNormal("违章车牌\n");
						carnums.ix = (pPlate->uPosLeft);
						carnums.iy = (pPlate->uPosTop);
						carnums.iwidth = (pPlate->uPosRight - pPlate->uPosLeft);
						carnums.iheight = (pPlate->uPosBottom - pPlate->uPosTop);

						ViolationInfo info;
						std::vector<ViolationInfo> vResult;
						memcpy(&info.carInfo,&carnums,sizeof(CarInfo));
						info.evtType = (VIO_EVENT_TYPE)pPlate->uViolationType;//违章类型
						info.nChannel = pPlate->uRoadWayID;//车道编号
						info.frameSeqs[0] = pPlate->uSeq;//取图帧号
						info.frameSeqs[1] = pPlate->uSeq2;
						info.frameSeqs[2] = pPlate->uSeq3;
						info.frameSeqs[3] = pPlate->uSeq4;//卡口图片帧号
						info.nPicCount = 3;//图片数量
						info.redLightStartTime = pPlate->uRedLightStartTime;//红灯开始时间
						info.uUpperFrm = pPlate->uUpperFrm;
						info.uLowerFrm = pPlate->uLowerFrm;
						info.dis[0]  = pPlate->uDis[0];
						info.dis[1]  = pPlate->uDis[1];
						info.dis[2]  = pPlate->uDis[2];
						
						//LogNormal("违章添加车牌信息成功帧号:%lld,相机编号:%d\n",carnums.uSeq,carnums.id);
						if (g_nGongJiaoMode == 1)
						{
							std::string strCameraCode = pHeader->szCameraCode;
							OutPutVTSResult(info,strCameraCode);
						}
						else
						{
							OutPutVTSResult(info);
						}					
						printf("******************YYYYYYYYYYYYY   违章添加车牌信息成功\n");
			}
			else
			{
						LogNormal("暂不支持该违章类型：%d\n",pPlate->uViolationType);
			}
			return true;
    }
    return false;
}

//输出车牌检测结果
void CDspDataProcess::CarNumOutPut(CarInfo& cardNum)
{
			if(m_pDspManage->GetServerIpgCount() < 1)
			{
				//LogNormal("=CarNumOutPut=error==m_ServerJpgFrameMap.size()=%d\n", m_ServerJpgFrameMap.size());
				return;
			}

			CvRect rtRoi;

            //////////////////////////////
            std::string strCarNum;
            strCarNum = cardNum.strCarNum;


            //判断是否有车牌的车
            bool bCarNum = true;
            bool bLoop = false;
            if( (cardNum.strCarNum[0] == '*')&& (cardNum.strCarNum[6] == '*') )
            {
                bCarNum = false;
            }

            if(bCarNum)
            {
                //车牌号码转换
                CarNumConvert(strCarNum,cardNum.wj);
            }

            PLATEPOSITION  TimeStamp[2];
            TimeStamp[0].uTimestamp = cardNum.uTimestamp;

			//获取过滤图片
			//Picture_Key Pic_Key;
			//Pic_Key.uSeq = cardNum.uSeq;
			//Pic_Key.uCameraId = cardNum.id;
			Picture_Elem picElem;
			picElem.key.uCameraId = cardNum.id;
			//memcpy(picElem.key.szCameraCode, strCameraCode.c_str(), strCameraCode.size());

			picElem.key.uSeq = cardNum.uSeq;
			picElem.ts = cardNum.ts;
			bool bRet = m_pDspManage->GetImageByJpgKey(picElem,TimeStamp,m_imgSnap);
			LogTrace("FindLog.txt", "OutDataM bRet:%d, seq:%d, uCam:%d ts:%d", \
				bRet, picElem.key.uSeq, picElem.key.uCameraId, (picElem.ts/1000)/1000);
			if(!bRet)
			{
				return;
			}
		
			RECORD_PLATE plate;

			/*if (g_nServerType == 13 || g_nServerType == 17)
			{
				//经过时间(秒)
				plate.uTime = TimeStamp[0].uTimestamp;
				//毫秒
				plate.uMiTime = ((TimeStamp[0].ts)/1000)%1000;
			}
			else*/
			{
				//经过时间(秒)
				plate.uTime = cardNum.uTimestamp;
				//毫秒
				plate.uMiTime = (cardNum.ts/1000)%1000;
			}

			//车牌号码
			memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());


			//车牌世界坐标 add by wantao
			plate.uLongitude = (UINT32)(cardNum.wx*10000*100);
			plate.uLatitude = (UINT32)(cardNum.wy*10000*100);
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

            carnum_context context;
            context.position =  rtCarnum;
            context.vehicle_type = cardNum.vehicle_type;
            context.color = (CARNUM_COLOR)cardNum.color;
            context.mean  = cardNum.mean;
            context.stddev = cardNum.stddev;

            context.VerticalTheta  = cardNum.VerticalTheta;
            context.HorizontalTheta = cardNum.HorizontalTheta;

            memcpy(context.carnum,cardNum.strCarNum,sizeof(context.carnum));

            context.smearnum = cardNum.smearCount;
            memcpy(context.smearrect,cardNum.smear,sizeof(CvRect)*(cardNum.smearCount));
            context.nCarNumDirection = (carnumdirection)1;//目标运动方向

			
			//获取机动车颜色
			if(m_nDetectKind&DETECT_CARCOLOR)
			{
				if(bCarNum)
				{
					CarColorDetect(cardNum,plate,context);
				}
			}

			//提取车身特征向量
			if(m_nDetectKind&DETECT_TEXTURE)
			{
				//提取车身纹理特征向量
				if(bCarNum)
				{
					CarLabelDetect(cardNum,plate,context);
				}
			}

			//获取车辆类型（高16位卡车、巴士、轿车等，低16位大、中、小车）
			if(m_nDetectKind&DETECT_TRUCK)
			{
				if(bCarNum)
				{
				    DetectTruck(cardNum,plate,context);
				}
			}

            //车道逻辑编号(暂时取第一个)
             plate.uRoadWayID = cardNum.RoadIndex;
           

			//车速
			double dSpeed =   sqrt(cardNum.vx*cardNum.vx+cardNum.vy*cardNum.vy);
			plate.uSpeed = (UINT32)(dSpeed+0.5);

            //车辆类型
			plate.uType = cardNum.vehicle_type;
			CarTypeConvert(plate);

            /////////////////////////////
            //车牌位置
			if(m_nWordPos == 1)
            {
                rtCarnum.y += m_nExtentHeight;
            }
            plate.uPosLeft = rtCarnum.x;
            plate.uPosTop = rtCarnum.y;
            plate.uPosRight = rtCarnum.x+rtCarnum.width-1;
            plate.uPosBottom = rtCarnum.y+rtCarnum.height-1;

			//图片尺寸
			plate.uPicWidth = m_imgComposeResult->width;
			plate.uPicHeight = m_imgComposeResult->height;

			/*
			if(m_imgDestSnap == NULL)
			{
				printf("------------m_imgDestSnap == NULL----------\n");
			}
			*/
			printf("--------------tt666---cc----Car-------------11111111-------------\n");
			//检测是否布控报警--add by ywx
			/*if(g_nDetectSpecialCarNum == 1)
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
			*/

            if(cardNum.nNoCarNum == 1)
            plate.uViolationType = DETECT_RESULT_NOCARNUM;


             //获取图片路径
             std::string strPicPath;
            int nSaveRet = GetPicSavePath(plate,strPicPath);

			plate.uCameraId = cardNum.id;

             //截取小图区域
              CvRect rtPos;
              if(m_nSmallPic == 1)
              {
                 rtPos = GetCarPos(plate);
              }
                     
			  //叠加图片
               if(g_nPicMode != 1)
               {
					SaveComposeImage(plate,rtPos,TimeStamp);
               }
               else   //分开
               {
					SaveSplitImage(plate,rtPos,TimeStamp);
               }


               //保存车牌记录
                if(nSaveRet>0)
                {
                        //LogNormal("==before save=plate.uRoadWayID=%d==\n", plate.uRoadWayID);
						 //相机同步信息
						 SYN_CHAR_DATA syn_char_data;

                        g_skpDB.SavePlate(m_nChannelId,plate,0,&syn_char_data);
                }
				else
				{
					printf("-------ERROR! nSaveRet=%d \n", nSaveRet);
				}
				printf("---------------------Car-------------33333333-------------\n");
                //将车牌信息送客户端
                if(m_bConnect)
                {
					//车牌号码
					memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

					SendResult(plate,cardNum.uSeq);
                }
}

void CDspDataProcess::CarNumOutPut( CarInfo& cardNum, std::string &strCameraCode )
{
	if(m_pDspManage->GetServerIpgCount() < 1)
	{
		//LogNormal("=CarNumOutPut=error==m_ServerJpgFrameMap.size()=%d\n", m_ServerJpgFrameMap.size());
		return;
	}

	CvRect rtRoi;

	//////////////////////////////
	std::string strCarNum;
	strCarNum = cardNum.strCarNum;


	//判断是否有车牌的车
	bool bCarNum = true;
	bool bLoop = false;
	if( (cardNum.strCarNum[0] == '*')&& (cardNum.strCarNum[6] == '*') )
	{
		bCarNum = false;
	}

	if(bCarNum)
	{
		//车牌号码转换
		CarNumConvert(strCarNum,cardNum.wj);
	}

	PLATEPOSITION  TimeStamp[2];
	TimeStamp[0].uTimestamp = cardNum.uTimestamp;

	//获取过滤图片
	//Picture_Key Pic_Key;
	//Pic_Key.uSeq = cardNum.uSeq;
	//Pic_Key.uCameraId = cardNum.id;
	Picture_Elem picElem;
	picElem.key.uCameraId = cardNum.id;
	memcpy(picElem.key.szCameraCode, strCameraCode.c_str(), strCameraCode.size());

	picElem.key.uSeq = cardNum.uSeq;
	picElem.ts = cardNum.ts;
	bool bRet = m_pDspManage->GetImageByJpgKey(picElem,TimeStamp,m_imgSnap);
	LogTrace("FindLog.txt", "OutDataM bRet:%d, seq:%d, uCam:%d ts:%d", \
		bRet, picElem.key.uSeq, picElem.key.uCameraId, (picElem.ts/1000)/1000);
	if(!bRet)
	{
		return;
	}

	RECORD_PLATE plate;

	/*if (g_nServerType == 13 || g_nServerType == 17)
	{
	//经过时间(秒)
	plate.uTime = TimeStamp[0].uTimestamp;
	//毫秒
	plate.uMiTime = ((TimeStamp[0].ts)/1000)%1000;
	}
	else*/
	{
		//经过时间(秒)
		plate.uTime = cardNum.uTimestamp;
		//毫秒
		plate.uMiTime = (cardNum.ts/1000)%1000;
	}

	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());


	//车牌世界坐标 add by wantao
	plate.uLongitude = (UINT32)(cardNum.wx*10000*100);
	plate.uLatitude = (UINT32)(cardNum.wy*10000*100);
	//相机ID
	plate.uChannelID = cardNum.id;
	//地点
	//memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
	memcpy(plate.chPlace,strCameraCode.c_str(),strCameraCode.size());
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

	carnum_context context;
	context.position =  rtCarnum;
	context.vehicle_type = cardNum.vehicle_type;
	context.color = (CARNUM_COLOR)cardNum.color;
	context.mean  = cardNum.mean;
	context.stddev = cardNum.stddev;

	context.VerticalTheta  = cardNum.VerticalTheta;
	context.HorizontalTheta = cardNum.HorizontalTheta;

	memcpy(context.carnum,cardNum.strCarNum,sizeof(context.carnum));

	context.smearnum = cardNum.smearCount;
	memcpy(context.smearrect,cardNum.smear,sizeof(CvRect)*(cardNum.smearCount));
	context.nCarNumDirection = (carnumdirection)1;//目标运动方向


	//获取机动车颜色
	if(m_nDetectKind&DETECT_CARCOLOR)
	{
		if(bCarNum)
		{
			CarColorDetect(cardNum,plate,context);
		}
	}

	//提取车身特征向量
	if(m_nDetectKind&DETECT_TEXTURE)
	{
		//提取车身纹理特征向量
		if(bCarNum)
		{
			CarLabelDetect(cardNum,plate,context);
		}
	}

	//获取车辆类型（高16位卡车、巴士、轿车等，低16位大、中、小车）
	if(m_nDetectKind&DETECT_TRUCK)
	{
		if(bCarNum)
		{
			DetectTruck(cardNum,plate,context);
		}
	}

	//车道逻辑编号(暂时取第一个)
	plate.uRoadWayID = cardNum.RoadIndex;


	//车速
	double dSpeed =   sqrt(cardNum.vx*cardNum.vx+cardNum.vy*cardNum.vy);
	plate.uSpeed = (UINT32)(dSpeed+0.5);

	//车辆类型
	plate.uType = cardNum.vehicle_type;
	CarTypeConvert(plate);

	/////////////////////////////
	//车牌位置
	if(m_nWordPos == 1)
	{
		rtCarnum.y += m_nExtentHeight;
	}
	plate.uPosLeft = rtCarnum.x;
	plate.uPosTop = rtCarnum.y;
	plate.uPosRight = rtCarnum.x+rtCarnum.width-1;
	plate.uPosBottom = rtCarnum.y+rtCarnum.height-1;

	//图片尺寸
	plate.uPicWidth = m_imgComposeResult->width;
	plate.uPicHeight = m_imgComposeResult->height;

	/*
	if(m_imgDestSnap == NULL)
	{
	printf("------------m_imgDestSnap == NULL----------\n");
	}
	*/
	printf("--------------tt666---cc----Car-------------11111111-------------\n");
	//检测是否布控报警--add by ywx
	/*if(g_nDetectSpecialCarNum == 1)
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
	*/

	if(cardNum.nNoCarNum == 1)
		plate.uViolationType = DETECT_RESULT_NOCARNUM;


	//获取图片路径
	std::string strPicPath;
	int nSaveRet = GetPicSavePath(plate,strPicPath);

	plate.uCameraId = cardNum.id;
	memcpy(plate.szCameraCode,strCameraCode.c_str(),strCameraCode.size());
	//LogNormal("[%s]:plate.szCameraCode = %s\n",__FUNCTION__,plate.szCameraCode);
	//截取小图区域
	CvRect rtPos;
	if(m_nSmallPic == 1)
	{
		rtPos = GetCarPos(plate);
	}

	//叠加图片
	if(g_nPicMode != 1)
	{
		SaveComposeImage(plate,rtPos,TimeStamp);
	}
	else   //分开
	{
		SaveSplitImage(plate,rtPos,TimeStamp);
	}

	//保存车牌记录
	if(nSaveRet>0)
	{
		//LogNormal("==before save=plate.uRoadWayID=%d==\n", plate.uRoadWayID);
		//相机同步信息
		SYN_CHAR_DATA syn_char_data;

		g_skpDB.SavePlate(m_nChannelId,plate,0,&syn_char_data);
	}
	else
	{
		printf("-------ERROR! nSaveRet=%d \n", nSaveRet);
	}
	printf("---------------------Car-------------33333333-------------\n");
	//将车牌信息送客户端
	if(m_bConnect)
	{
		//车牌号码
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

		SendResult(plate,cardNum.uSeq);
	}
}

//发送检测结果到客户端
void CDspDataProcess::SendResult(RECORD_PLATE& plate,unsigned int uSeq)
{
		SRIP_DETECT_HEADER sDetectHeader;
        sDetectHeader.uChannelID = m_nChannelId;
        //车牌检测类型
        sDetectHeader.uDetectType = SRIP_CARD_RESULT;
        sDetectHeader.uTimestamp = plate.uTime;
        sDetectHeader.uSeq = uSeq;

        std::string result;
        
        //判断车牌位置是否需要扩充
        GetCarPostion(plate);
        result.append((char*)&plate,sizeof(plate));

        result.insert(0,(char*)&sDetectHeader,sizeof(SRIP_DETECT_HEADER));
        g_skpChannelCenter.AddResult(result);
}

//获取车身位置
CvRect CDspDataProcess::GetCarPos(RECORD_PLATE plate)
{
        int x = 0;
        int y = 0;
        int w = 0;//宽度
        int h = 0;//高度

		int nWidth = m_imgSnap->width;
		int nHeight = m_imgSnap->height;

        CvRect rtCar;

		int nExtentHeight = 0;
		if(m_nWordPos == 1)
		{
			nExtentHeight = m_nExtentHeight;
		}


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

        if(y > nExtentHeight)
        {
            rtCar.y = y;
        }
        else
        {
            rtCar.y = nExtentHeight;
        }

        if(rtCar.x+rtCar.width>=nWidth)
        {
            rtCar.x = nWidth - rtCar.width-1;
        }

        if(rtCar.y+rtCar.height>=nHeight)
        {
            rtCar.y = nHeight - rtCar.height-1;
        }

		if(g_nGongJiaoMode == 1)
		{
			//if(rtCar.y <= 0 || rtCar.x <= 0)
			{
				int temp = plate.uPosLeft - 200;
				rtCar.x = abs(temp);

				temp = plate.uPosTop - 200;
				rtCar.y = abs(temp);

				rtCar.width = 500;
				rtCar.height = 700;

				if(rtCar.y + rtCar.height > DSP_200_BIG_HEIGHT_SERVER)
				{
					rtCar.height = DSP_200_BIG_HEIGHT_SERVER - rtCar.y;
				}

				if(rtCar.x + rtCar.width > DSP_200_BIG_WIDTH_SERVER)
				{
					rtCar.width = DSP_200_BIG_WIDTH_SERVER - rtCar.y;
				}
			}
			printf("--rtCar[%d, %d, %d, %d]", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
		}


        return rtCar;
}


//分开存储图像
void CDspDataProcess::SaveSplitImage(RECORD_PLATE& plate,CvRect rtPos,PLATEPOSITION* TimeStamp)
{
		std::string strPicPath;
		strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

		plate.uPicWidth = m_imgSnap->width;
		plate.uPicHeight = m_imgSnap->height;

       //存第一张图
       PutTextOnImage(m_imgSnap,plate,0,TimeStamp);
       plate.uPicSize = SaveImage(m_imgSnap,strPicPath);


	    if(m_nSaveImageCount == 2)
		{
			//存第二张图
			PutTextOnImage(m_imgPreSnap,plate,1,TimeStamp);
			SaveImage(m_imgPreSnap,strPicPath,1);

		}

		//存储小图
		if(m_nSmallPic == 1)
		{
				if( (rtPos.width > 0) && (rtPos.height > 0))
				{
						plate.uSmallPicSize = SaveSmallImage(m_imgSnap,strPicPath,rtPos,m_pSmallJpgImage);

						SaveExistImage(strPicPath,m_pSmallJpgImage,plate.uSmallPicSize);
				}
		}
}

//叠加存储图像
void CDspDataProcess::SaveComposeImage(RECORD_PLATE& plate,CvRect rtPos,PLATEPOSITION* TimeStamp)
{			
			std::string strPicPath;
			strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

			//是否需要存储小图
            if(m_nSmallPic == 1)
            {
                                CvRect rect;
                                rect.x = 0;
								if(m_nWordPos == 1)
								{
									rect.y = 0;
								}
								else
								{
									rect.y = m_imgDestSnap->height - m_nExtentHeight;
								}
                                rect.width = m_imgDestSnap->width;
                                rect.height = (m_imgDestSnap->height - m_imgSnap->height+m_nExtentHeight);

								if(rect.height > 0)
								{
									cvSetImageROI(m_imgDestSnap,rect);
									cvSet(m_imgDestSnap, cvScalar( 0,0, 0 ));
									cvResetImageROI(m_imgDestSnap);
								}

                                for(int i = 0;i<m_nSaveImageCount;i++)
                                {
                                    rect.x = m_imgSnap->width*i;
                                    if(m_nWordPos == 1)
                                    rect.y = (m_imgDestSnap->height - m_imgSnap->height);
                                    else
                                    rect.y = 0;
                                    rect.width = m_imgSnap->width;
                                    rect.height = m_imgSnap->height;

                                    cvSetImageROI(m_imgDestSnap,rect);

									if(m_nSaveImageCount == 2)
									{
										if(m_nDetectDirection == 0)
										{
											if(i == 1)
											cvCopy(m_imgSnap,m_imgDestSnap);
											else
											cvCopy(m_imgPreSnap,m_imgDestSnap);
										}
										else
										{
											if(i == 0)
											cvCopy(m_imgSnap,m_imgDestSnap);
											else
											cvCopy(m_imgPreSnap,m_imgDestSnap);
										}
									}
									else
									{
										cvCopy(m_imgSnap,m_imgDestSnap);
									}

                                    cvResetImageROI(m_imgDestSnap);
                                }

                                rect.x = m_nSaveImageCount*m_imgSnap->width;
                                if(m_nWordPos == 1)
                                rect.y = (m_imgDestSnap->height - m_imgSnap->height+m_nExtentHeight);
                                else
                                rect.y = 0;
                                rect.width = m_imgDestSnap->width - rect.x;
                                rect.height = m_imgSnap->height - m_nExtentHeight;

                                if(rect.width > 0)
                                {
                                    cvSetImageROI(m_imgDestSnap,rect);
                                    if( (rtPos.width > 0) && (rtPos.height > 0))
                                    {
                                        cvSetImageROI(m_imgSnap,rtPos);
                                        cvResize(m_imgSnap,m_imgDestSnap);
                                        cvResetImageROI(m_imgSnap);
                                    }
                                    cvResetImageROI(m_imgDestSnap);
                                }

								int nOrder = 0;

								if(m_nSaveImageCount == 2)
								{
									if(m_nDetectDirection == 0)//前牌
									{
										nOrder = 1;

										plate.uPosLeft += m_imgSnap->width;
										plate.uPosRight += m_imgSnap->width;
									}
								}

                                PutTextOnComposeImage(m_imgDestSnap,plate,nOrder);
                                plate.uPicSize = SaveImage(m_imgDestSnap,strPicPath,2);
         }
         else
         {
                 if(m_nSaveImageCount == 2)
                 {
                                    for(int nIndex = 0; nIndex <m_nSaveImageCount ; nIndex++)
                                    {
                                        CvRect rect;
                                        rect.x = m_imgSnap->width*nIndex;
                                        rect.y = 0;
                                        rect.width = m_imgSnap->width;
                                        rect.height = m_imgSnap->height;

                                        cvSetImageROI(m_imgDestSnap,rect);
                                        if(nIndex == 0)
                                        cvCopy(m_imgSnap,m_imgDestSnap);
                                        else
                                        cvCopy(m_imgPreSnap,m_imgDestSnap);
                                        cvResetImageROI(m_imgDestSnap);

                                        PutTextOnImage(m_imgDestSnap,plate,nIndex,TimeStamp);
                                    }
                                    plate.uPicSize = SaveImage(m_imgDestSnap,strPicPath,2);

                  }
				  else
				  {
									PutTextOnImage(m_imgSnap,plate,0,TimeStamp);
									plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0);

									if(g_nPicSaveMode != 0)
									{
										if(plate.uViolationType == DETECT_RESULT_BLACK_PLATE)
										{
											string strViolationPicPath;
											GetPlatePicPath(plate,strViolationPicPath);
											SaveExistImage(strViolationPicPath,m_pCurJpgImage,plate.uPicSize);
										}
									}
					}
          }
}

//获取图片路径
int CDspDataProcess::GetPlatePicPath(RECORD_PLATE& plate,std::string& strPicPath)
{
	char buf[256] = {0};
	//存储大图片
	std::string strPath;

	string strTime = GetTime(plate.uTime,1);
	g_skpDB.UTF8ToGBK(strTime);

	string strCarnum(plate.chText);

	if(plate.chText[0] == '*')
	{
		if(plate.chText[1] == '-')
		{
			strCarnum = "无牌车";
		}
		else
		{
			strCarnum = "非机动车";
		}
	}
	g_skpDB.UTF8ToGBK(strCarnum);

	if( plate.uViolationType > 0)
	{
		strPath = g_FileManage.GetSpecialPicPath(1);

		string strViolationType = GetViolationType(plate.uViolationType);

		sprintf(buf,"%s/%s",strPath.c_str(),strViolationType.c_str());
		std::string strSubPicPath(buf);

		// 判断目录是否存在,不存在则建立图片目录
		if(access(strSubPicPath.c_str(),0) != 0) //目录不存在
		{
			mkdir(strSubPicPath.c_str(),S_IRWXU|S_IRGRP|S_IROTH|S_IXOTH);
			sync();
		}
		sprintf(buf,"%s/%s_%s.jpg",strSubPicPath.c_str(),strTime.c_str(),strCarnum.c_str());
	}
	else
	{
		strPath = g_FileManage.GetSpecialPicPath(0);
		sprintf(buf,"%s/%s_%s.jpg",strPath.c_str(),strTime.c_str(),strCarnum.c_str());
	}

	strPicPath = buf;

	return 1;
}

/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CDspDataProcess::PutTextOnComposeImage(IplImage* pImage,RECORD_PLATE plate,int nOrder)
{
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvBigText;
	m_cvBigText.Init(g_PicFormatInfo.nFontSize);

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

	sprintf(chOut,"设备编号:%d    地点名称:%s    方向:%s    时间:%s.%03d",plate.uChannelID,m_strLocation.c_str(),strDirection.c_str(),strTime.c_str(),plate.uMiTime);//
	string strText(chOut);
	//GPRM协议中：纬度:ddmm.mmmm(度分格式)， 经度:(dddmm.mmmm)(度分格式)
	double Longitude = (double)(plate.uLongitude/1000000) + (double)(plate.uLongitude - (plate.uLongitude/1000000)*1000000)*1.0/(1000000);
	double Latitude = (double)(plate.uLatitude/1000000) + (double)(plate.uLatitude - (plate.uLatitude/1000000)*1000000)*1.0/(1000000);
	sprintf(chOut,"经度:%.4f  纬度:%.4f",Longitude,Latitude);
	string strTmp(chOut);
	strText += strTmp;
	memset(chOut,0,sizeof(chOut));
	memcpy(chOut,strText.c_str(),strText.size());
	
    memset(wchOut,0,sizeof(wchOut));
    UTF8ToUnicode(wchOut,chOut);

	if(m_nSaveImageCount == 2)
	{
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
	else
	{
		nHeight += (m_nExtentHeight/2);
		m_cvBigText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	}

	m_cvBigText.UnInit();
}

/* 函数介绍：在图像上叠加文本信息（车牌号码、日期、地点等）
 * 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
 * 输出参数：无
 * 返回值：无
 */
void CDspDataProcess::PutTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp)
{
	//printf("**********CamId:%d*********[%d:%d]\n", plate.uCameraId,pImage->width,pImage->height);
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);

    wchar_t wchOut[1024] = {'\0'};
    char chOut[1024] = {'\0'};

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

    //设备编号
    std::string strDirection = GetDirection(plate.uDirection);
	//LogNormal("plate.uLongitude = %u,uLatitude = %u\n",plate.uLongitude,plate.uLatitude);
	double Longitude = (double)(plate.uLongitude/1000000) + (double)(plate.uLongitude - (plate.uLongitude/1000000)*1000000)*1.0/(1000000);
	double Latitude = (double)(plate.uLatitude/1000000) + (double)(plate.uLatitude - (plate.uLatitude/1000000)*1000000)*1.0/(1000000);

	if (g_nGongJiaoMode == 1)
	{
		LogNormal("[%s]:plate = %s\n",__FUNCTION__,plate.chText);
		LogNormal("Longitude = %.6f,Latitude = %.6f\n",Longitude,Latitude);
		sprintf(chOut,"相机编号:%s  经度:%.6f  纬度:%.6f  方向:%s ", \
			plate.szCameraCode,Longitude,Latitude,strDirection.c_str());
	}
	else
	{
		sprintf(chOut,"设备编号:%d  地点名称:%s  方向:%s   经度:%.4f  纬度:%.4f",\
			plate.uChannelID,m_strLocation.c_str(),strDirection.c_str(),Longitude,Latitude);
	}
		
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

#ifdef VTS_TEST_SEQ
	//记录帧号
	sprintf(chOut,"Seq: [%d][%d,%d,%d,%d]", \
		((pTimeStamp)->uSeq), plate.uPosLeft, plate.uPosTop, plate.uPosRight, plate.uPosBottom);

	string strTmpTest(chOut);
	strText += strTmpTest;
#endif

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

	//LogTrace("PutVtsText.log","*****11****pImage:%x********sizeof(wchOut):%d**[%d:%d]--[%d:%d]",pImage, sizeof(wchOut), pImage->width,pImage->height,nWidth, nHeight);
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
		{
			sprintf(chOut,"车身颜色:%s  ",strCarColor.c_str());
		}
        else
		{
			if(g_PicFormatInfo.nSecondCarColor == 0)
			{
				sprintf(chOut,"车身颜色:%s  ",strCarColor.c_str());
			}
			else
			{
				sprintf(chOut,"车身颜色:%s,%s  ",strCarColor.c_str(),strCarColor2.c_str());
			}
		}
        string strTmp(chOut);
        strText += strTmp;

		//号牌颜色
		std::string strPlateColor = GetPlateColor(plate.uColor);
		sprintf(chOut,"号牌颜色:%s  ",strPlateColor.c_str());

		string strPlateTmp(chOut);
        strText += strPlateTmp;
    }

    if(g_PicFormatInfo.nCarType == 1)
    {
        //车辆类型
        std::string strCarType = GetDetailCarType(plate.uType,plate.uTypeDetail,plate.uDetailCarType);
       
        sprintf(chOut,"车辆类型:%s  ",strCarType.c_str());
        string strTmp(chOut);
        strText += strTmp;
    }

    if(g_PicFormatInfo.nCarBrand == 1)
    {
        //车标
       {
          if(plate.uCarBrand != OTHERS)
		   {
				#ifdef GLOBALCARLABEL
				CBrandSusection BrandSub;
				UINT32 uCarLabel = plate.uCarBrand + plate.uDetailCarBrand;
				sprintf(chOut,"车标:%s  ",BrandSub.GetCarLabelText(uCarLabel).c_str());
				#endif
		   }
           else
           sprintf(chOut,"车标:  ");

           string strTmp(chOut);
           strText += strTmp;
       }
    }

	if(g_PicFormatInfo.nViolationType == 1)
	{
		if(plate.uViolationType > 0)
		{
			std::string strViolationType = GetViolationType(plate.uViolationType,1);
			sprintf(chOut,"违章类型:%s",strViolationType.c_str());
			string strTmp(chOut);
			strText += strTmp;
		}
	}

	//防伪码
	if (g_nGongJiaoMode == 1)
	{
		int nRandCode = g_RoadImcData.GetRandCode();
		sprintf(chOut,"防伪码:%d",nRandCode);
		string strTmp(chOut);
		strText += strTmp;
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
	nHeight -= 8;

	//printf("***22****pImage:%x**********sizeof(wchOut):%d**[%d:%d]--[%d:%d] %s\n", \
	//	pImage, sizeof(wchOut), pImage->width,pImage->height,nWidth, nHeight,wchOut);
	try
	{
	    m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	}
	catch(...)
	{
		SaveImgTest(pImage,NULL);
	}

	m_cvText.UnInit();
}

//获取图片路径
int CDspDataProcess::GetPicSavePath(RECORD_PLATE& plate,string& strPicPath)
{					
					int nSaveRet = 0;

					if(g_nPicSaveMode == 0)
					{
								nSaveRet = GetPicPathAndSaveDB(strPicPath);
								//printf("==nSaveRet=%d\n",nSaveRet);
					}
					else
					{
								if(plate.uViolationType == DETECT_RESULT_BLACK_PLATE)
								{
									plate.uViolationType = 0;
									nSaveRet = GetPlatePicPath(plate,strPicPath);
									plate.uViolationType = DETECT_RESULT_BLACK_PLATE;
								}
								else
								{
									nSaveRet = GetPlatePicPath(plate,strPicPath);
								}
					}

					//大图存储路径
					if(g_nPicSaveMode == 0)
					{
								memcpy(plate.chPicPath,strPicPath.c_str(),strPicPath.size());
					}
					else
					{
								string strGBKPicPath = strPicPath;
								g_skpDB.GBKToUTF8(strGBKPicPath);

								memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
					}
					
					return nSaveRet;
}

//输出违章检测结果
void CDspDataProcess::OutPutVTSResult(ViolationInfo& infoViolation)
{
	string strEvent;
    SRIP_DETECT_HEADER sHeader;

	LogTrace("FindLog.txt", "--plate:%s uSeq:%d ", infoViolation.carInfo.strCarNum, infoViolation.frameSeqs[0]);
	OutPutVTSResultElem(infoViolation,strEvent,sHeader,false);
}

void CDspDataProcess::OutPutVTSResult( ViolationInfo& infoViolation,std::string &strCameraCode )
{
	string strEvent;
	SRIP_DETECT_HEADER sHeader;

	LogTrace("FindLog.txt", "--plate:%s uSeq:%d ", infoViolation.carInfo.strCarNum, infoViolation.frameSeqs[0]);
	OutPutVTSResultElem(infoViolation,strEvent,sHeader,false,strCameraCode);
}

//输出一条违章检测结果
bool CDspDataProcess::OutPutVTSResultElem(ViolationInfo &infoViolation, string &strEvent, SRIP_DETECT_HEADER &sHeader, bool bGetVtsImgByKey)
{
	//LogTrace("Vts-text.log", "--infoViolation.carnum=%s \n", infoViolation.carInfo.strCarNum);
	//经纬度为0不输出
	if(g_nGongJiaoMode == 1)
	{
		if(infoViolation.carInfo.wx == 0 || infoViolation.carInfo.wy == 0)
		{
			return false;
		}
	}
        RECORD_PLATE plate;
         //违章类型
		plate.uViolationType =infoViolation.evtType;
		VtsTypeConvert(plate);

        std::string strCarNum = infoViolation.carInfo.strCarNum;
        //判断是否有车牌的车
        bool bCarNum = true;
        if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
        {
            bCarNum = false;
        }

        if(bCarNum)
        {
            //车牌号码转换
            CarNumConvert(strCarNum,infoViolation.carInfo.wj);
        }
		//车牌号码
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

		int nPos = -1;
		string nStrNum = plate.chText;
		nPos = nStrNum.find("*");

	    //车辆类型
        plate.uType =  infoViolation.carInfo.vehicle_type;

        //车型细分
        if(m_nDetectKind&DETECT_TRUCK)
        plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);
		
		//车辆类型转换
		CarTypeConvert(plate);

        int nPicCount = infoViolation.nPicCount;//图片数量
        if(g_nVtsPicMode == 3)
        {
            if(nPicCount > 2)
            nPicCount = 2;

			if(1 == g_PicFormatInfo.nSmallViolationPic)
			{
				UINT32 frameSeqs = infoViolation.frameSeqs[0];
				infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
				infoViolation.frameSeqs[1] = frameSeqs;
			}
        }
		else if(g_nVtsPicMode == 4)
		{
			nPicCount = 1;
		}

		if ((g_nDetectMode == 2)&&(DETECT_RESULT_EVENT_GO_FAST == plate.uViolationType))//超速只要一张图
		{
			nPicCount = 1;
		}

        PLATEPOSITION  TimeStamp[6];
		PLATEPOSITION  SignalTimeStamp;


        TimeStamp[0].uTimestamp = infoViolation.carInfo.uTimestamp;
		//TimeStamp[0].nDirection = infoViolation.carInfo.nDirection;
         printf("===nPicCount=%d\n",nPicCount);
        if(nPicCount>=1)
        {
			UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];

			if(plate.uViolationType != DETECT_RESULT_RED_LIGHT_VIOLATION)
			{
				frameSeqs = 0;
			}
			else
			{
				printf("s1=%d,s2=%d,s3=%d,d1=%d,dis1=%d,dis2=%d,dis3=%d\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2],infoViolation.frameSeqs[0] - infoViolation.dis[0],infoViolation.dis[0],infoViolation.dis[1],infoViolation.dis[2]);
			}

			SignalTimeStamp.x = infoViolation.carInfo.ix;
			SignalTimeStamp.y = infoViolation.carInfo.iy;
			SignalTimeStamp.width = infoViolation.carInfo.iwidth;
			SignalTimeStamp.height = infoViolation.carInfo.iheight;
			SignalTimeStamp.nType = plate.uType;
			SignalTimeStamp.IsCarnum = bCarNum;


            bool bRet = false;

		//	LogNormal("f1=%lld,f2=%lld,f3=%lld\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);

			//获取违章检测结果图像


					//车牌世界坐标 add by wantao
					plate.uLongitude = (UINT32)(infoViolation.carInfo.wx*10000*100);
					plate.uLatitude = (UINT32)(infoViolation.carInfo.wy*10000*100);
					//相机ID
					plate.uCameraId = infoViolation.carInfo.id;
					
					//发生位置(在当前图片上的)
					plate.uPosLeft  = infoViolation.carInfo.ix;
					plate.uPosTop   = infoViolation.carInfo.iy;
					plate.uPosRight = infoViolation.carInfo.ix + infoViolation.carInfo.iwidth;
					plate.uPosBottom  = (infoViolation.carInfo.iy + infoViolation.carInfo.iheight);

					plate.uCarPosLeft = infoViolation.carInfo.m_CarWholeRec.x;
					plate.uCarPosTop= infoViolation.carInfo.m_CarWholeRec.y;
					plate.uCarPosRight = infoViolation.carInfo.m_CarWholeRec.x + infoViolation.carInfo.m_CarWholeRec.width;
					plate.uCarPosBottom = infoViolation.carInfo.m_CarWholeRec.y + infoViolation.carInfo.m_CarWholeRec.height;


					//重设车牌位置
					ReSetVtsPlatePos(infoViolation, plate);

					Picture_Key Pic_Key;
					Pic_Key.uSeq = infoViolation.carInfo.uSeq;
					Pic_Key.uCameraId = infoViolation.carInfo.id;
					bRet = GetVtsImageByJpgKey2(infoViolation,Pic_Key,TimeStamp);
					if(!bRet)
					{
						return false;
					}
					else //3张图都找到,调老杨的二次识别,做筛选,剔除不符合的违章
					{
#ifdef TWICE_DETECT
						//LogNormal("plate.uViolationType :%d ", plate.uViolationType);
						if(DETECT_RESULT_OBV_TAKE_UP_BUSWAY == plate.uViolationType)
						{
							CvRect platRt;
							platRt.x = infoViolation.carInfo.ix;
							platRt.y = infoViolation.carInfo.iy;
							platRt.width =  infoViolation.carInfo.iwidth;
							platRt.height =  infoViolation.carInfo.iheight;

							//Rect,pImage1,2,3
							m_vecRet1.clear();
							m_vecRet2.clear();
							m_vecRet3.clear();

							//调用的主函数
							bool bVioValid = m_OnBusVioFilter.mvOnBusVioValidJudge(m_imgVtsElem1, m_vecRet1, 
								m_imgVtsElem2, m_vecRet2, m_imgVtsElem3, m_vecRet3, platRt);

							if(!bVioValid)
							{
								LogNormal("二次识别,筛查剔除! plate:%s ", infoViolation.carInfo.strCarNum);
								//DrawVioFilter(plate);
								return false;
							}
							else
							{
								LogNormal("二次识别,筛查通过! plate:%s ", infoViolation.carInfo.strCarNum);
								//TODO 将检测到的车牌位置画出来
								//DrawVioFilter(plate);
							}
						}						
#endif
					}
        }

		/*if (g_nServerType == 13 || g_nServerType == 17)
		{
			//经过时间(秒)
			plate.uTime = TimeStamp->uTimestamp;
			//毫秒
			plate.uMiTime = ((TimeStamp->ts)/1000)%1000;
		}
		else*/
		{
			//经过时间(秒)
			plate.uTime = infoViolation.carInfo.uTimestamp;
			//毫秒
			plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;
		}

        //地点
        memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
        //行驶方向
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

		/*if(g_nServerType == 7 || g_nServerType == 10)
		{
			plate.uDirection = m_nDirection;
		}*/

        //车牌结构
        plate.uPlateType = infoViolation.carInfo.carnumrow;
        //车牌颜色
        plate.uColor = infoViolation.carInfo.color;


        //车道编号
		plate.uRoadWayID = infoViolation.nChannel;

        //车速
        double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
        plate.uSpeed = (UINT32)(dSpeed+0.5);

		
			//发生位置(在当前图片上的)
			plate.uPosLeft  = infoViolation.carInfo.ix;
			plate.uPosTop   = infoViolation.carInfo.iy;
			plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
			plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

			//重设车牌位置
			ReSetVtsPlatePos(infoViolation, plate);

			plate.uPicWidth = m_imgComposeSnap->width;
			plate.uPicHeight = m_imgComposeSnap->height;
			

			//获取录像以及图片路径
			 pthread_mutex_lock(&g_Id_Mutex);
			 //获取图片路径
			std::string strPicPath;
			////////////////////
			int nSaveRet = GetVtsPicSavePath(plate,nPicCount,strPicPath);

			pthread_mutex_unlock(&g_Id_Mutex);
			//删除已经存在的记录
			g_skpDB.DeleteOldRecord(strPicPath,false,false);


			if(nPicCount >= 1)
			{		
				//存储违章图像,并叠字
				SaveVtsImage(plate,nPicCount,TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime);
			}

			//保存闯红灯记录
			if(nSaveRet>0)
			{
				g_skpDB.SavePlate(m_nChannelId,plate,0);
			}


        //将车牌信息送客户端
        if(m_bConnect)
        {
			//车牌号码
			memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
			SendResult(plate,infoViolation.carInfo.uSeq);
        }

	    return true;
}

bool CDspDataProcess::OutPutVTSResultElem( ViolationInfo &infoViolation, string &strEvent, SRIP_DETECT_HEADER &sHeader,bool bGetVtsImgByKey,std::string &strCameraCode )
{
	//LogTrace("Vts-text.log", "--infoViolation.carnum=%s \n", infoViolation.carInfo.strCarNum);
	//经纬度为0不输出
	if(g_nGongJiaoMode == 1)
	{
		if(infoViolation.carInfo.wx == 0 || infoViolation.carInfo.wy == 0)
		{
			return false;
		}
	} 
	RECORD_PLATE plate;
         //违章类型
		plate.uViolationType =infoViolation.evtType;
		VtsTypeConvert(plate);

        std::string strCarNum = infoViolation.carInfo.strCarNum;
        //判断是否有车牌的车
        bool bCarNum = true;
        if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
        {
            bCarNum = false;
        }

        if(bCarNum)
        {
            //车牌号码转换
            CarNumConvert(strCarNum,infoViolation.carInfo.wj);
        }
		//车牌号码
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

		int nPos = -1;
		string nStrNum = plate.chText;
		nPos = nStrNum.find("*");

	    //车辆类型
        plate.uType =  infoViolation.carInfo.vehicle_type;
		//LogNormal("[%s]1:vehicle_type = %d\n",__FUNCTION__,infoViolation.carInfo.vehicle_type); //test for gongjiao
        //车型细分
        if(m_nDetectKind&DETECT_TRUCK)
        plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);
		
		//车辆类型转换
		CarTypeConvert(plate);

        int nPicCount = infoViolation.nPicCount;//图片数量
        if(g_nVtsPicMode == 3)
        {
            if(nPicCount > 2)
            nPicCount = 2;

			if(1 == g_PicFormatInfo.nSmallViolationPic)
			{
				UINT32 frameSeqs = infoViolation.frameSeqs[0];
				infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
				infoViolation.frameSeqs[1] = frameSeqs;
			}
        }
		else if(g_nVtsPicMode == 4)
		{
			nPicCount = 1;
		}

		if ((g_nDetectMode == 2)&&(DETECT_RESULT_EVENT_GO_FAST == plate.uViolationType))//超速只要一张图
		{
			nPicCount = 1;
		}

        PLATEPOSITION  TimeStamp[6];
		PLATEPOSITION  SignalTimeStamp;


        TimeStamp[0].uTimestamp = infoViolation.carInfo.uTimestamp;
		//TimeStamp[0].nDirection = infoViolation.carInfo.nDirection;
         printf("===nPicCount=%d\n",nPicCount);
        if(nPicCount>=1)
        {
			UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];

			if(plate.uViolationType != DETECT_RESULT_RED_LIGHT_VIOLATION)
			{
				frameSeqs = 0;
			}
			else
			{
				printf("s1=%d,s2=%d,s3=%d,d1=%d,dis1=%d,dis2=%d,dis3=%d\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2],infoViolation.frameSeqs[0] - infoViolation.dis[0],infoViolation.dis[0],infoViolation.dis[1],infoViolation.dis[2]);
			}

			SignalTimeStamp.x = infoViolation.carInfo.ix;
			SignalTimeStamp.y = infoViolation.carInfo.iy;
			SignalTimeStamp.width = infoViolation.carInfo.iwidth;
			SignalTimeStamp.height = infoViolation.carInfo.iheight;
			SignalTimeStamp.nType = plate.uType;
			SignalTimeStamp.IsCarnum = bCarNum;


            bool bRet = false;

		//	LogNormal("f1=%lld,f2=%lld,f3=%lld\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);

			//获取违章检测结果图像


					//车牌世界坐标 add by wantao
					plate.uLongitude = (UINT32)(infoViolation.carInfo.wx*10000*100);
					plate.uLatitude = (UINT32)(infoViolation.carInfo.wy*10000*100);
					//相机ID
					plate.uCameraId = infoViolation.carInfo.id;
					memcpy(plate.szCameraCode,strCameraCode.c_str(),strCameraCode.size());
					LogNormal("[%s]:plate.szCameraCode = %s\n",__FUNCTION__,plate.szCameraCode);
					//发生位置(在当前图片上的)
					plate.uPosLeft  = infoViolation.carInfo.ix;
					plate.uPosTop   = infoViolation.carInfo.iy;
					plate.uPosRight = infoViolation.carInfo.ix + infoViolation.carInfo.iwidth;
					plate.uPosBottom  = (infoViolation.carInfo.iy + infoViolation.carInfo.iheight);
					//重设车牌位置
					ReSetVtsPlatePos(infoViolation, plate);
					Picture_Key Pic_Key;
					Pic_Key.uSeq = infoViolation.carInfo.uSeq;
					Pic_Key.uCameraId = infoViolation.carInfo.id;
					memcpy(Pic_Key.szCameraCode,strCameraCode.c_str(),strCameraCode.size());

					bRet = GetVtsImageByJpgKey2(infoViolation,Pic_Key,TimeStamp);
					LogNormal("[%s]:bRet = %d\n",__FUNCTION__,bRet);
					if(!bRet)
					{
						return false;
					}
					else //3张图都找到,调老杨的二次识别,做筛选,剔除不符合的违章
					{
#ifdef TWICE_DETECT
						//LogNormal("plate.uViolationType :%d ", plate.uViolationType);
						if(DETECT_RESULT_OBV_TAKE_UP_BUSWAY == plate.uViolationType)
						{
							CvRect platRt;
							platRt.x = infoViolation.carInfo.ix;
							platRt.y = infoViolation.carInfo.iy;
							platRt.width =  infoViolation.carInfo.iwidth;
							platRt.height =  infoViolation.carInfo.iheight;

							//Rect,pImage1,2,3
							m_vecRet1.clear();
							m_vecRet2.clear();
							m_vecRet3.clear();

							//调用的主函数
							bool bVioValid = m_OnBusVioFilter.mvOnBusVioValidJudge(m_imgVtsElem1, m_vecRet1, 
								m_imgVtsElem2, m_vecRet2, m_imgVtsElem3, m_vecRet3, platRt);

							if(!bVioValid)
							{
								LogNormal("二次识别,筛查剔除! plate:%s ", infoViolation.carInfo.strCarNum);
								//DrawVioFilter(plate);
								return false;
							}
							else
							{
								LogNormal("二次识别,筛查通过! plate:%s ", infoViolation.carInfo.strCarNum);
								//TODO 将检测到的车牌位置画出来
								//DrawVioFilter(plate);
							}
						}						
#endif
					}
        }

		/*if (g_nServerType == 13 || g_nServerType == 17)
		{
			//经过时间(秒)
			plate.uTime = TimeStamp->uTimestamp;
			//毫秒
			plate.uMiTime = ((TimeStamp->ts)/1000)%1000;
		}
		else*/
		{
			//经过时间(秒)
			plate.uTime = infoViolation.carInfo.uTimestamp;
			//毫秒
			plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;
		}

        //地点
        //memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
		memcpy(plate.chPlace,strCameraCode.c_str(),strCameraCode.size());
        //行驶方向
       /* if(m_nDetectDirection != infoViolation.carInfo.nDirection)
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
        }*/
        /*else
        {
            plate.uDirection = m_nDirection;
        }*/

		/*if(g_nServerType == 7 || g_nServerType == 10)
		{
			plate.uDirection = m_nDirection;
		}*/

        //车牌结构
        plate.uPlateType = infoViolation.carInfo.carnumrow;
        //车牌颜色
        plate.uColor = infoViolation.carInfo.color;


        //车道编号
		plate.uRoadWayID = infoViolation.nChannel;

        //车速
        double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
        plate.uSpeed = (UINT32)(dSpeed+0.5);

		
			//发生位置(在当前图片上的)
			plate.uPosLeft  = infoViolation.carInfo.ix;
			plate.uPosTop   = infoViolation.carInfo.iy;
			plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
			plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

			//重设车牌位置
			ReSetVtsPlatePos(infoViolation, plate);

			plate.uPicWidth = m_imgComposeSnap->width;
			plate.uPicHeight = m_imgComposeSnap->height;
			

			//获取录像以及图片路径
			 pthread_mutex_lock(&g_Id_Mutex);
			 //获取图片路径
			std::string strPicPath;
			////////////////////
			int nSaveRet = GetVtsPicSavePath(plate,nPicCount,strPicPath);

			pthread_mutex_unlock(&g_Id_Mutex);
			//删除已经存在的记录
			g_skpDB.DeleteOldRecord(strPicPath,false,false);


			if(nPicCount >= 1)
			{		
				//存储违章图像,并叠字
				SaveVtsImage(plate,nPicCount,TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime);
			}

			//保存闯红灯记录
			if(nSaveRet>0)
			{
				g_skpDB.SavePlate(m_nChannelId,plate,0);
			}

		
        //将车牌信息送客户端
        if(m_bConnect)
        {
			//车牌号码
			memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
			SendResult(plate,infoViolation.carInfo.uSeq);
        }

	    return true;
}

//检测器-车载公交方案 获取三张图
bool CDspDataProcess::GetVtsImageByJpgKey2(ViolationInfo infoViolation,Picture_Key Pic_Key,PLATEPOSITION* pTimeStamp)
{
	printf("=in=GetVtsImageByJpgKey2==\n");

	if(m_pDspManage->GetServerIpgCount() < 1)
	{
		//printf("=err==m_ServerJpgFrameMap.size()=%d==\n", m_ServerJpgFrameMap.size());
		return false;
	}

	if(m_imgComposeResult == NULL)
	{
		printf("-----------m_imgComposeResult NULL!----\n");
		return false;
	}

	printf("--------aaa---11111111-----GetVtsImageByJpgKey2--m_imgComposeSnap=%x--\n", m_imgComposeSnap);

	//合成图片清空
	CvRect nFinalRt;
	nFinalRt.x = 0;
	nFinalRt.y = 0;
	nFinalRt.width = m_imgComposeSnap->width;
	nFinalRt.height = m_imgComposeSnap->height;

	printf("************m_imgSnap:%d :%d \n",m_imgSnap->width,m_imgSnap->height);
	printf("************m_imgComposeSnap:%d :%d \n",m_imgComposeSnap->width,m_imgComposeSnap->height);
	cvSet(m_imgComposeSnap, cvScalar( 0,0, 0 ));
	printf("******************** cvSet END\n");

	yuv_video_buf* buf = NULL;
	bool bRet = false;
	int nPicCount = 3;
	RECORD_PLATE plate_r;

	plate_r.uPosLeft  = infoViolation.carInfo.ix;
	plate_r.uPosTop   = infoViolation.carInfo.iy;
	plate_r.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
	plate_r.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

	plate_r.uCarPosLeft = infoViolation.carInfo.m_CarWholeRec.x;
	plate_r.uCarPosTop = infoViolation.carInfo.m_CarWholeRec.y;
	plate_r.uCarPosRight = infoViolation.carInfo.m_CarWholeRec.x + infoViolation.carInfo.m_CarWholeRec.width;
	plate_r.uCarPosBottom = infoViolation.carInfo.m_CarWholeRec.y + infoViolation.carInfo.m_CarWholeRec.height;

	UINT32 uViolationType = infoViolation.evtType;

	//LogNormal("--GetImageByJpgKey2 Seqs[%d,%d,%d]\n", infoViolation.frameSeqs[0], infoViolation.frameSeqs[1], infoViolation.frameSeqs[2]);

	Picture_Elem picElem;
	picElem.key.uCameraId = Pic_Key.uCameraId;
	picElem.key.uSeq = Pic_Key.uSeq;
	picElem.ts = infoViolation.carInfo.ts;

	bool bRet1 = false;
	bool bRet2 = false;
	bool bRet3 = false;	

	int nFrams = 10;
	int64_t tsTemp = 0;

	Pic_Key.uSeq = infoViolation.frameSeqs[0];
	tsTemp = picElem.ts;
	bRet1 = m_pDspManage->IsFindPicFromJpgMap(Pic_Key, tsTemp);
	
	Pic_Key.uSeq = infoViolation.frameSeqs[1];
	tsTemp = picElem.ts + abs((long long)(infoViolation.frameSeqs[1] - infoViolation.frameSeqs[0]))/nFrams;
	bRet2 = m_pDspManage->IsFindPicFromJpgMap(Pic_Key, tsTemp);

	Pic_Key.uSeq = infoViolation.frameSeqs[2];
	tsTemp = picElem.ts + abs((long long)(infoViolation.frameSeqs[2] - infoViolation.frameSeqs[0]))/nFrams;
	bRet3 = m_pDspManage->IsFindPicFromJpgMap(Pic_Key, tsTemp);

	bool bRet11 = false;
	bool bRet22 = false;
	bool bRet33 = false;	

	bool bGetNei1 = false;
	bool bGetNei2 = false;
	bool bGetNei3 = false;

	string strPic;
	for(int i = 0; i<nPicCount; i++)
	{
		//Pic_Key.uSeq = infoViolation.frameSeqs[i];
		picElem.key.uSeq = infoViolation.frameSeqs[i];

		if(0 == i)
		{
			bRet11 = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[0], m_imgComposeResult);
			//找临近图片
			if(!bRet11)
			{
				bGetNei1 = GetImgByJpgKeyNeighbor(picElem, pTimeStamp, m_imgComposeResult, i, bRet11, bRet2, bRet3);
			}

#ifdef TWICE_DETECT
			if(m_imgComposeResult != NULL)
			{
				cvCopy(m_imgComposeResult, m_imgVtsElem1);
			}	
#endif
		}
		else if(1 == i)
		{
			bRet22 = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[1], m_imgComposeResult);
			if(!bRet22)
			{
				bGetNei2 = GetImgByJpgKeyNeighbor(picElem, pTimeStamp, m_imgComposeResult, i, bRet11, bRet22, bRet3);
			}

#ifdef TWICE_DETECT
			if(m_imgComposeResult != NULL)
			{
				cvCopy(m_imgComposeResult, m_imgVtsElem2);
			}	
#endif
		}
		else if(2 == i)
		{
			bRet33 = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[2], m_imgComposeResult);
			if(!bRet33)
			{
				bGetNei3 = GetImgByJpgKeyNeighbor(picElem, pTimeStamp, m_imgComposeResult, i, bRet11, bRet22, bRet33);
			}

#ifdef TWICE_DETECT
			if(m_imgComposeResult != NULL)
			{
				cvCopy(m_imgComposeResult, m_imgVtsElem3);
			}	
#endif
		}
		else
		{
		}

		if(2 == i)
		{
			LogTrace("FindLog.txt","plate:%s-seq:%d bRet-[%d,%d,%d],bRetAA-[%d,%d,%d], seq[%d,%d,%d] bGetNei:[%d,%d,%d]", \
				infoViolation.carInfo.strCarNum, picElem.key.uSeq, bRet1, bRet2, bRet3, bRet11, bRet22, bRet33,\
				infoViolation.frameSeqs[0], infoViolation.frameSeqs[1], infoViolation.frameSeqs[2], bGetNei1,bGetNei2,bGetNei3);
					
			if(bRet11 && bRet22 && bRet33)
			{
				LogNormal("-bRet 123 ok!-\n");
			}
			else if(!bRet11 && !bRet22 && !bRet33)
			{
				LogNormal("-bRet112233 fail!\n");
				return false;
			}
			else
			{
				if(!bRet11)
				{
					if(!bGetNei1)
					{
						LogNormal("-bGetNei1 fail!\n");
						return false;
					}
				}
				if(!bRet22)
				{
					if(!bGetNei2)
					{
						LogNormal("-bGetNei2 fail!\n");
						return false;
					}
				}
				if(!bRet33)
				{
					if(!bGetNei3)
					{
						LogNormal("-bGetNei3 fail!\n");
						return false;
					}
				}	
			}					
		}		

		if(g_nVtsPicMode == 1) //2x2
		{
			ComposePic2x2(i, plate_r);		
		}
		else if(g_nVtsPicMode == 5) //1x3
		{
			ComposePic1x3(i, plate_r);
		}
		else
		{}
		//LogNormal("-m_nWordPos=%d,m_nWordOnPic=%d--\n", m_nWordPos, m_nWordOnPic);		
	}

	//LogNormal("EE-bRet-[%d,%d,%d]\n", bRet1, bRet2, bRet3);
	
	printf("=out=GetVtsImageByJpgKey2==m_imgComposeSnap:%d :%d\n",m_imgComposeSnap->width,m_imgComposeSnap->height);

	return true;
}

//获取违章图片路径
int CDspDataProcess::GetVtsPicSavePath(RECORD_PLATE& plate,int nPicCount,string& strPicPath)
{		
			int nSaveRet = 0;

			if(7 == g_nServerType)
			{
				m_nRandCode[0] = g_RoadImcData.GetRandCode();
				//m_nRandCode[1] = g_RoadImcData.GetRandCode();
				LogTrace("FBMach.log", "m_nRandCode:%d ", m_nRandCode[0]);

				g_RoadImcData.GetPlatePicPath(plate,strPicPath,1,m_nRandCode[0],0);
				//LogTrace("VtsPath.txt", "plate.chPicPath:%s \n GetVtsPicSavePath: path: %s", \
				//	plate.chPicPath, strPicPath.c_str());
			
				nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,1); //FIX videoId
				string strGBKPicPath = strPicPath;
				memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());
			}
			else
			{
				if(g_nPicSaveMode == 0)
				{
					//需要判断磁盘是否已经满
					g_FileManage.CheckDisk(false,false);
					//存储大图片
					strPicPath  = g_FileManage.GetPicPath();

					nSaveRet = g_skpDB.SavePicID(g_uPicId,g_uVideoId,0);
				}
				else
				{
					nSaveRet = GetPlatePicPath(plate,strPicPath);
				}

				//大图存储路径
				if(g_nPicSaveMode == 0)
				{
					//LogTrace("VtsPath.txt", "GetVtsPicSavePath plate.chPicPath:%s 111 strPicPath:%s ", \
					plate.chPicPath, strPicPath.c_str());

					memset(plate.chPicPath, 0, 128);
					memcpy(plate.chPicPath, strPicPath.c_str(),strPicPath.size());

					//LogTrace("VtsPath.txt", "GetVtsPicSavePath plate.chPicPath:%s 222 strPicPath:%s ", \
					plate.chPicPath, strPicPath.c_str());
				}
				else
				{
					//LogTrace("VtsPath.txt", "GetVtsPicSavePath plate.chPicPath:%s 333 strPicPath:%s ", \
					plate.chPicPath, strPicPath.c_str());

					string strGBKPicPath = strPicPath;
					g_skpDB.GBKToUTF8(strGBKPicPath);

					memset(plate.chPicPath, 0, 128);
					memcpy(plate.chPicPath,strGBKPicPath.c_str(),strGBKPicPath.size());

					//LogTrace("VtsPath.txt", "GetVtsPicSavePath plate.chPicPath:%s 444 strPicPath:%s ", \
					plate.chPicPath, strPicPath.c_str());
				}
			}			

		    return nSaveRet;
}

//存储违章图像
void CDspDataProcess::SaveVtsImage(
	RECORD_PLATE& plate,
	int nPicCount,
	PLATEPOSITION* TimeStamp,
	PLATEPOSITION& SignalTimeStamp,
	int64_t redLightStartTime)
{
	if(g_nVtsPicMode == 1)//2x2
	{
		std::string strPicPath;
		strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

		//大图宽高
		plate.uPicWidth = m_imgComposeSnap->width;
		plate.uPicHeight = m_imgComposeSnap->height;

		for(int nIndex = 0; nIndex <nPicCount ; nIndex++)
		{
			PutVtsTextOnImage(m_imgComposeSnap,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime, plate.uViolationType);
		}

		//叠加额外信息
		if( (g_nVtsPicMode > 0)&& (g_nVtsPicMode < 3))
		{
			if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
				PutVtsTextOnImage(m_imgComposeSnap,plate,nPicCount,TimeStamp,&SignalTimeStamp,redLightStartTime,plate.uViolationType);
			else
				PutVtsTextOnImage(m_imgComposeSnap,plate);
		}


		if((nPicCount == 1)&&(g_nDetectMode == 2))
		{
			IplImage* pImage = cvCreateImageHeader(cvSize(m_imgComposeSnap->width, m_imgComposeSnap->height/2), 8, 3);
			cvSetData(pImage,m_imgComposeSnap->imageData,pImage->widthStep);
			plate.uPicSize = SaveImage(pImage,strPicPath,2);
			cvReleaseImageHeader(&pImage);
		}
		else
		{
			plate.uPicSize = SaveImage(m_imgComposeSnap,strPicPath,2);
		}
	}
	else if(g_nVtsPicMode == 5)//1x3
	{
		PutVtsTextOnImage1x3(m_imgComposeSnap,plate,TimeStamp,SignalTimeStamp,redLightStartTime);
	}
	else{}	
}

//电警叠加文本信息
void CDspDataProcess::PutVtsTextOnImage(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp,PLATEPOSITION* pSignalTimeStamp,int64_t redLightStartTime, UINT32 uViolationType)
{
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);

	int timeIndex = nIndex;
	if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//有违章小图 并且 大图2x2叠加
	{
		//增加第一张小图的时间,与第二张图的时间相等
		if (timeIndex > 0)
			timeIndex -= 1;
		//逆行的情况,第一张图片时间从第3张图片中取得
		if ( (DETECT_RESULT_RETROGRADE_MOTION == uViolationType /*||
			DETECT_RESULT_PRESS_LINE == uViolationType || //压黄线
			DETECT_RESULT_PRESS_WHITELINE == uViolationType ||
			DETECT_RESULT_ELE_EVT_BIANDAO == uViolationType */) && //变道
			0 == nIndex)
		{
			timeIndex = 2;
		}
	}

    if(pTimeStamp != NULL)
    {

        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = 10;
        int nHeight = 0;


        if(m_nWordOnPic == 1)//字直接叠加在图上
        {
            if(pImage->width > m_imgSnap->width)
            {
				/*
				if(g_nServerType != 4 && uViolationType == DETECT_RESULT_EVENT_GO_FAST)//洛阳项目
				{
					nWidth += (m_imgSnap->width)*(nIndex%2);
					nHeight = (m_imgSnap->height- m_nExtentHeight)*(nIndex/2);

					if(nIndex == 0)
					{
						timeIndex = 3;
					}
					else if(nIndex == 1)
					{
						timeIndex = 0;
					}

					//时间
					std::string strTime;
					strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
					sprintf(chOut,"时间:%s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
				
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (m_nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

					
					//经过地点,行驶方向
					std::string strDirection = GetDirection(plate.uDirection);
					sprintf(chOut,"地点:%s  方向:%s",m_strLocation.c_str(),strDirection.c_str());
				
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (m_nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

					int nOnlyOverSpedMax = 10;
					nOnlyOverSpedMax = GetMaxSpeed(plate.uType, plate.uChannelID, plate.uRoadWayID);
					//限速值
					sprintf(chOut,"1限速:%dkm/h  车速:%dkm/h",nOnlyOverSpedMax,plate.uSpeed);

					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (m_nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
				}
				else//针对江宁项目
				*/
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
					
					if(uViolationType == DETECT_RESULT_EVENT_GO_FAST && nIndex == 0)
					{
						//取卡口时间
						strTime = GetTime((pTimeStamp+3)->uTimestamp,0);
						sprintf(chOut,"抓拍时间: %s:%03d",strTime.c_str(),(((pTimeStamp+3)->ts)/1000)%1000);
					}
					else
					{
						strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);
						sprintf(chOut,"抓拍时间: %s:%03d",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
					}

					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,chOut);
					nHeight += (m_nExtentHeight/2);
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					//红灯时间(第三行)
					if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
					{

						UINT32 uTimestamp = (redLightStartTime/1000)/1000;
						if(redLightStartTime <= 0)
						{
							if(pSignalTimeStamp != NULL)
							{
								uTimestamp = pSignalTimeStamp->uTimestamp;
							}
						}
						strTime = GetTime(uTimestamp,0);
						sprintf(chOut,"红灯时间: %s:%03d",strTime.c_str(),((redLightStartTime)/1000)%1000);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,chOut);
						nHeight += (m_nExtentHeight/2);
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					}
				}
            }
        }
        else
        {
			//LogTrace("PutVtsText.log", "222=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d=m_nWordPos=%d", \
				g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode, m_nWordPos);

			//武汉格式
			if (1 == g_PicFormatInfo.nSmallViolationPic && 3 == g_nVtsPicMode)//有违章小图 并且 大图1x2叠加
			{
				nWidth = 10+nIndex*400;
				string strText("");

				//路口名称
				sprintf(chOut,"路口名称:%s",m_strLocation.c_str());
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight = 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//经过时间
				std::string strTime;
				strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,1);
				sprintf(chOut,"经过时间:%s%03d毫秒",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//方向，限速
				int nOnlyOverSpedMax = 10;
				nOnlyOverSpedMax = GetMaxSpeed(plate.uType, plate.uChannelID, plate.uRoadWayID);
				std::string strDirection = GetDirection(plate.uDirection);
				sprintf(chOut,"方向:%s  车辆限速:%dkm/h",strDirection.c_str(),nOnlyOverSpedMax);//
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				//速度，违法名称，超速百分比
				int nOverSped = (plate.uSpeed-nOnlyOverSpedMax)*100/nOnlyOverSpedMax;
				std::string strViolationType = GetViolationType(plate.uViolationType,1);
				sprintf(chOut,"速度:%dkm/h  违法名称:%s  超速百分比:%d%%",plate.uSpeed,strViolationType.c_str(),nOverSped);
				strText = chOut;
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				nHeight += 15;
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

				return;
			}
			

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

			//LogTrace("PutVtsText.log", "====nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);

			string strText("");

			//设备编号
			//if (g_nGongJiaoMode == 1)
			{
				double Longitude = (double)(plate.uLongitude/1000000) + (double)(plate.uLongitude - (plate.uLongitude/1000000)*1000000)*1.0/(1000000);
				double Latitude = (double)(plate.uLatitude/1000000) + (double)(plate.uLatitude - (plate.uLatitude/1000000)*1000000)*1.0/(1000000);
				sprintf(chOut,"设备编号:%d  经度:%.4f  纬度:%.4f ", \
					plate.uChannelID,Longitude,Latitude);
			}
			/*else
			{
				std::string strDirection = GetDirection(plate.uDirection);
				if (10 == g_nServerType)
					sprintf(chOut,"设备编号:%s  违法地点:%s  方向:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str());//
				else if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//有违章小图 并且 大图2x2叠加
					sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ", g_strDetectorID.c_str(), m_strLocation.c_str(), strDirection.c_str());
				else
					sprintf(chOut,"设备编号:%s  地点名称:%s  方向:%s  ",g_strDetectorID.c_str(),m_strLocation.c_str(),strDirection.c_str());//
			}
			*/
			strText += chOut;

            //车道编号
            if(g_PicFormatInfo.nRoadIndex == 1)
            {
                sprintf(chOut,"车道编号:%d  ",plate.uRoadWayID);//
                string strTmp(chOut);
                strText += strTmp;
            }

			//车牌号码
			if(g_PicFormatInfo.nCarNum == 1/* && 1 != g_PicFormatInfo.nSmallViolationPic*/)
			{
				std::string strCarNum(plate.chText);
				sprintf(chOut,"车牌号码:%s  ",strCarNum.c_str());
				string strTmp(chOut);
				strText += strTmp;
			}

			//车速
			if(g_PicFormatInfo.nCarSpeed == 1 && 1 != g_PicFormatInfo.nSmallViolationPic)
			{
				//行驶速度
				sprintf(chOut,"速度:%dkm/h  ",plate.uSpeed);
				string strTmp(chOut);
				strText += strTmp;
			}

			if (1 != g_PicFormatInfo.nSmallViolationPic)
			{
				//LogTrace("PutVtsText.log", "====nWidth=%d=nHeight=%d==nIndex=%d=g_PicFormatInfo.nSmallViolationPic=%d", nWidth, nHeight, nIndex, g_PicFormatInfo.nSmallViolationPic);

				//乐清宝康中心端 黑条中文字只显示1行
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
			}

            //经过时间
            std::string strTime;
            strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);

			/*
			if (10 == g_nServerType)
				sprintf(chOut,"违法时间:%s:%03d  ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
			else*/
			{				
				sprintf(chOut,"抓拍时间:%s:%03d  ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
			}

            {
                string strTmp(chOut);
                strText += strTmp;
            }


            //红灯时间
            if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
            {
				if(pSignalTimeStamp != NULL && 1 != g_PicFormatInfo.nSmallViolationPic)
				{
					UINT32 uTimestamp = (redLightStartTime/1000)/1000;
					if(redLightStartTime <= 0)
					{
						uTimestamp = pSignalTimeStamp->uTimestamp;
					}
					strTime = GetTime(uTimestamp,0);
					int nMiTime = (redLightStartTime/1000)%1000;
					{
						sprintf(chOut,"红灯开始时间:%s:%03d  ",strTime.c_str(),nMiTime);
						string strTmp(chOut);
						strText += strTmp;
					}

					if(g_nPicSaveMode == 1)
					{
						 strTime = GetTime(uTimestamp+m_vtsGlobalPara.nRedLightTime,0);
						 sprintf(chOut,"红灯结束时间:%s:%03d ",strTime.c_str(),(nMiTime+6)%1000);
						 string strTmp(chOut);
						 strText += strTmp;
					}
				}
            }

            if(g_PicFormatInfo.nViolationType == 1)
            {
               //违章行为
                if(plate.uViolationType != 0)
                {
                    std::string strViolationType = GetViolationType(plate.uViolationType,1);

					/*
					if (10 == g_nServerType)
						sprintf(chOut,"违法行为:%s",strViolationType.c_str());
					else
					*/
						sprintf(chOut,"违章行为:%s",strViolationType.c_str());
                    string strTmp(chOut);
                    strText += strTmp;

                }
            }

			if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)//有违章小图 并且 大图2x2叠加
			{
				//文字一行显示
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
			}

            memset(wchOut,0,sizeof(wchOut));
            UTF8ToUnicode(wchOut,(char*)strText.c_str());
            nWidth = 10;
            if(pImage->width > m_imgSnap->width)
            {
                nWidth += (m_imgSnap->width)*(nIndex%2);
            }

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
		//LogTrace("PutVtsText.log", "333=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d", \
			g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode);

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

		/*
		if (10 != g_nServerType)//非鞍山交警的情况
		{
			sprintf(chOut,"路口编号: %s",g_strDetectorID.c_str());//
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,chOut);
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));
		}
		*/

        nHeight += 120;
		/*
        //经过地点
		if (10 == g_nServerType)
		{
			int num = 3 * 8;
			for (int i = 0; i < m_strLocation.size(); i += num)
			{
				if (i == 0)
					sprintf(chOut,"违法地点: %s", m_strLocation.substr(i, num));
				else
					sprintf(chOut,"        %s", m_strLocation.substr(i, num));

				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,chOut);
				cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight),CV_RGB(0,0,255));

				nHeight += 120;
			}
		}
		else
		*/
		{
			sprintf(chOut,"路口名称: ");
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

		}

		 //行驶方向
		memset(wchOut,0,sizeof(wchOut));
		std::string strDirection = GetDirection(plate.uDirection);
		sprintf(chOut,"车道方向: %s",strDirection.c_str());
		UTF8ToUnicode(wchOut,chOut);
		cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,0,255));

		nHeight += 120;

        //经过时间
        std::string strTime = GetTime(plate.uTime);
		/*
		if (10 == g_nServerType)
		{
			sprintf(chOut,"违法时间: %s",strTime.c_str());
		}
		else
		*/
		{
			sprintf(chOut,"抓拍时间: %s",strTime.c_str());
		}
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

        cvText.UnInit();
    }
	
	m_cvText.UnInit();
}

//输出测试图片，模拟车牌结果（测试用）
void CDspDataProcess::CarNumOutPutTest(CarInfo& cardNum)
{
	printf("------------------------CarNumOutPutTest----->>-\n");
	if(m_pDspManage->GetServerIpgCount() < 1)
	{
		//LogNormal("=CarNumOutPut=error==m_ServerJpgFrameMap.size()=%d\n", m_ServerJpgFrameMap.size());
		return;
	}

	CvRect rtRoi;
    //////////////////////////////
    std::string strCarNum;
    strCarNum = cardNum.strCarNum;

    //判断是否有车牌的车
    bool bCarNum = true;
    bool bLoop = false;
    if( (cardNum.strCarNum[0] == '*')&& (cardNum.strCarNum[6] == '*') )
    {
        bCarNum = false;
    }

    if(bCarNum)
    {
        //车牌号码转换
        CarNumConvert(strCarNum,cardNum.wj);
    }

    PLATEPOSITION  TimeStamp[2];
    TimeStamp[0].uTimestamp = cardNum.uTimestamp;

	//获取过滤图片
	//Picture_Key Pic_Key;
	//Pic_Key.uSeq = cardNum.uSeq;
	//Pic_Key.uCameraId = cardNum.id;

	Picture_Elem picElem;
	picElem.key.uCameraId = cardNum.id;
	picElem.key.uSeq = cardNum.uSeq;
	picElem.ts = cardNum.ts;

	int nret = m_pDspManage->GetImageByJpgKey(picElem,TimeStamp,m_imgSnap);
	if(!nret)
	{
		return;
	}
		
	RECORD_PLATE plate;
	//经过时间(秒)
	plate.uTime = cardNum.uTimestamp;
	//毫秒
	plate.uMiTime = (cardNum.ts/1000)%1000;
	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());

	//车牌世界坐标 add by wantao
	//plate.uLongitude = (UINT32)(cardNum.wx*10000*100);
	//plate.uLatitude = (UINT32)(cardNum.wy*10000*100);
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

	if(m_imgDestSnap == NULL)
	{
		printf("------------m_imgDestSnap == NULL----------\n");
	}
    if(cardNum.nNoCarNum == 1)
    plate.uViolationType = DETECT_RESULT_NOCARNUM;

        //获取图片路径
	std::string strPicPath;
	int nSaveRet = GetPicSavePath(plate,strPicPath);					

    //截取小图区域
    CvRect rtPos;
    if(m_nSmallPic == 1)
    {
        rtPos = GetCarPos(plate);
    }            
	
	SaveComposeImageForTest(plate,rtPos,TimeStamp);

    //保存车牌记录
    if(nSaveRet>0)
    {
		//LogNormal("==before save=plate.uRoadWayID=%d==\n", plate.uRoadWayID);
		//相机同步信息
		SYN_CHAR_DATA syn_char_data;
		g_skpDB.SavePlate(m_nChannelId,plate,0,&syn_char_data);
    }
	else
	{
		printf("-------ERROR! nSaveRet=%d \n", nSaveRet);
	}

    //将车牌信息送客户端
    if(m_bConnect)
    {
		//车牌号码
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
		SendResult(plate,cardNum.uSeq);
    }
}

//叠加存储图像（测试用）
void CDspDataProcess::SaveComposeImageForTest(RECORD_PLATE& plate,CvRect rtPos,PLATEPOSITION* TimeStamp)
{			
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));


	PutTextOnImageForTest(m_imgSnap,plate,0,TimeStamp);
	plate.uPicSize = SaveImage(m_imgSnap,strPicPath,0);

	if(g_nPicSaveMode != 0)
	{
		if(plate.uViolationType == DETECT_RESULT_BLACK_PLATE)
		{
			string strViolationPicPath;
			GetPlatePicPath(plate,strViolationPicPath);
			SaveExistImage(strViolationPicPath,m_pCurJpgImage,plate.uPicSize);
		}
	}

}

/* 函数介绍：在图像上叠加文本信息（测试用）
* 输入参数：uTimestamp-时间戳，strCarNum-车牌号码
* 输出参数：无
* 返回值：无
*/
void CDspDataProcess::PutTextOnImageForTest(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp)
{
	printf("*******************[%d:%d]\n",pImage->width,pImage->height);
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);

    wchar_t wchOut[1024] = {'\0'};
    char chOut[1024] = {'\0'};

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

	sprintf(chOut,"编号:%d 测试专用图片", plate.uChannelID);
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

	printf("*******************[%s][%d:%d]--[%d:%d]\n",wchOut,pImage->width,pImage->height,nWidth, nHeight);
	m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

	m_cvText.UnInit();
}

//输出车牌遮挡前后匹配图
void CDspDataProcess::OutPutVtsMatch(RECORD_PLATE_DSP_MATCH &record_plate, RECORD_PLATE_DSP_MATCH &foundRecord, RECORD_PLATE& plate)
{
//	LogNormal("-OutPutVtsMatch-[%s]-[%s]\n", record_plate.dspRecord.chText, foundRecord.dspRecord.chText);
	CarInfo    carnums;
	char text[10] = "123456789";
	//char chPlateText[MAX_PLATE] = {0};

	/*if (record_plate.dspRecord.chText[0] != '*')
	{
		sprintf(chPlateText, "%s", record_plate.dspRecord.chText);
	}
	else if (foundRecord.dspRecord.chText[0] != '*')
	{
		sprintf(chPlateText, "%s", foundRecord.dspRecord.chText);
	}
	else
	{
		sprintf(chPlateText, "%s", record_plate.dspRecord.chText);
	}*/

	if(foundRecord.dspRecord.chText[0] != 'L') //非武警牌照
	{
		memset(text, 0, 10);
		memcpy(text, foundRecord.dspRecord.chText, 7);
		memcpy(carnums.strCarNum, text, 7);
	}
	else
	{
		memset(text, 0, 10);
		memcpy(text, foundRecord.dspRecord.chText, 9);
		memcpy(carnums.strCarNum, text, 7);
		memcpy(carnums.wj, text+7, 2); //武警牌下面的小数字
	}

	carnums.color = record_plate.dspRecord.uColor;
	carnums.vehicle_type = record_plate.dspRecord.uType;

	carnums.ix = (record_plate.dspRecord.uPosLeft);
	carnums.iy = (record_plate.dspRecord.uPosTop);
	carnums.iwidth = (record_plate.dspRecord.uPosRight - record_plate.dspRecord.uPosLeft);
	carnums.iheight = (record_plate.dspRecord.uPosBottom - record_plate.dspRecord.uPosTop);

	carnums.mean = -1;
	carnums.stddev = -1;

	//carnums.VerticalTheta = record_plate.dspRecord.uVerticalTheta;
	//carnums.HorizontalTheta = record_plate.dspRecord.uHorizontalTheta;

	carnums.uTimestamp = record_plate.dspRecord.uTime;//(pHeader->ts)/1000/1000;
	carnums.ts = record_plate.dspRecord.uTime * 1000 * 1000 + record_plate.dspRecord.uMiTime * 1000;
	carnums.uSeq = record_plate.dspRecord.uSeqID;
	carnums.carnumrow = record_plate.dspRecord.uType;//?

	carnums.wx = 0; //(double)((pPlate->uLongitude) * 0.0001);
	carnums.wy = 0; //(double)((pPlate->uLatitude) * 0.0001);

	carnums.id = 0; //pHeader->uCameraId;

	carnums.vx = 0;
	carnums.vy = record_plate.dspRecord.uSpeed; //传入dsp相机线圈检测出速度
	carnums.RoadIndex = record_plate.dspRecord.uRoadWayID;

	carnums.nDirection = record_plate.dspRecord.uDirection;

	ViolationInfo infoViolation;
	//电警数据
	{		
		//std::vector<ViolationInfo> vResult;
		memcpy(&infoViolation.carInfo,&carnums,sizeof(CarInfo));
		infoViolation.evtType = (VIO_EVENT_TYPE)record_plate.dspRecord.uViolationType;//违章类型
		infoViolation.nChannel = record_plate.dspRecord.uRoadWayID;//车道编号
		//info.frameSeqs[0] = record_plate.dspRecord.uSeq;//取图帧号
		//info.frameSeqs[1] = record_plate.dspRecord.uSeq2;
		//info.frameSeqs[2] = record_plate.dspRecord.uSeq3;

		infoViolation.nPicCount = 2;//图片数量
		//info.redLightStartTime = pPlate->uRedLightStartTime;//红灯开始时间
		//info.uUpperFrm = pPlate->uUpperFrm;
		//info.uLowerFrm = pPlate->uLowerFrm;
		//info.dis[0]  = pPlate->uDis[0];
		//info.dis[1]  = pPlate->uDis[1];
		//info.dis[2]  = pPlate->uDis[2];

		//LogNormal("违章添加车牌信息成功帧号:%lld,相机编号:%d\n",carnums.uSeq,carnums.id);

		//OutPutVTSResult(info);
		printf("******************YYYYYYYYYYYYY   违章添加车牌信息成功\n");
	}

//	RECORD_PLATE plate;
	memcpy((char*)(&plate),(char*)(&record_plate.dspRecord),sizeof(RECORD_PLATE));
	plate.uCarColor1 = record_plate.dspRecord.uCarColor1;
	plate.uCarColor2 = foundRecord.dspRecord.uCarColor1;
	
	//违章类型
	plate.uViolationType = DETECT_MATCH_PLATE; //infoViolation.evtType;
	//VtsTypeConvert(plate); //强制类型，无需转换

	std::string strCarNum = infoViolation.carInfo.strCarNum;
//	LogNormal("-strCarNum=%s--carInfo[%s]\n", strCarNum.c_str(), infoViolation.carInfo.strCarNum);

	//判断是否有车牌的车
	bool bCarNum = true;
	if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
	{
		bCarNum = false;
	}
	
	if(bCarNum)
	{
		//车牌号码转换
		CarNumConvert(strCarNum,infoViolation.carInfo.wj);
	}
	
	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	//memcpy(plate.chText,infoViolation.carInfo.strCarNum,7);

//	LogNormal("-plate.chText=%s--\n", plate.chText);

	int nPos = -1;
	string nStrNum = plate.chText;
	nPos = nStrNum.find("*");

	//车辆类型
	plate.uType =  infoViolation.carInfo.vehicle_type;

	//车型细分
	if(m_nDetectKind&DETECT_TRUCK)
		plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);

	//车辆类型转换
	CarTypeConvert(plate);

	int nPicCount = infoViolation.nPicCount;//图片数量
	//if(g_nVtsPicMode == 3)
	{
		if(nPicCount > 2)
			nPicCount = 2;

		if(1 == g_PicFormatInfo.nSmallViolationPic)
		{
			UINT32 frameSeqs = infoViolation.frameSeqs[0];
			infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
			infoViolation.frameSeqs[1] = frameSeqs;
		}
	}

	if ((g_nDetectMode == 2)&&(DETECT_RESULT_EVENT_GO_FAST == plate.uViolationType))//超速只要一张图
	{
		nPicCount = 1;
	}

	PLATEPOSITION  TimeStamp[6];
	PLATEPOSITION  SignalTimeStamp;


	TimeStamp[0].uTimestamp = infoViolation.carInfo.uTimestamp;
	//TimeStamp[0].nDirection = infoViolation.carInfo.nDirection;
	printf("===nPicCount=%d\n",nPicCount);
	if(nPicCount>=1)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];

		if(plate.uViolationType != DETECT_RESULT_RED_LIGHT_VIOLATION)
		{
			frameSeqs = 0;
		}
		else
		{
			printf("s1=%d,s2=%d,s3=%d,d1=%d,dis1=%d,dis2=%d,dis3=%d\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2],infoViolation.frameSeqs[0] - infoViolation.dis[0],infoViolation.dis[0],infoViolation.dis[1],infoViolation.dis[2]);
		}

		SignalTimeStamp.x = infoViolation.carInfo.ix;
		SignalTimeStamp.y = infoViolation.carInfo.iy;
		SignalTimeStamp.width = infoViolation.carInfo.iwidth;
		SignalTimeStamp.height = infoViolation.carInfo.iheight;
		SignalTimeStamp.nType = plate.uType;
		SignalTimeStamp.IsCarnum = bCarNum;


		bool bRet = false;

		//	LogNormal("f1=%lld,f2=%lld,f3=%lld\n",infoViolation.frameSeqs[0],infoViolation.frameSeqs[1],infoViolation.frameSeqs[2]);

		//获取违章检测结果图像


		//车牌世界坐标 add by wantao
		plate.uLongitude = (UINT32)(infoViolation.carInfo.wx*10000*100);
		plate.uLatitude = (UINT32)(infoViolation.carInfo.wy*10000*100);
		//相机ID
		plate.uChannelID = infoViolation.carInfo.id;

		//发生位置(在当前图片上的)
		plate.uPosLeft  = infoViolation.carInfo.ix;
		plate.uPosTop   = infoViolation.carInfo.iy;
		plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
		plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

		Picture_Key Pic_Key;
		Pic_Key.uSeq = infoViolation.carInfo.uSeq;
		Pic_Key.uCameraId = infoViolation.carInfo.id;

		//找图
		//bRet = GetVtsImageByJpgKey2(infoViolation,Pic_Key,TimeStamp);
		/*
		if(!bRet)
		{
			return false;
		}*/

		printf("-----1111111---GetVtsImageMatch----\n");
		//SaveImgTest(record_plate.pImg);
		GetVtsImageMatch(record_plate.pImg, foundRecord.pImg);
		printf("-----22222222---GetVtsImageMatch----\n");
	}

	/*
	if (g_nServerType == 13 || g_nServerType == 17)
	{
		//经过时间(秒)
		plate.uTime = TimeStamp->uTimestamp;
		//毫秒
		plate.uMiTime = ((TimeStamp->ts)/1000)%1000;
	}
	else
	*/
	{
		//经过时间(秒)
		plate.uTime = infoViolation.carInfo.uTimestamp;
		//毫秒
		plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;
	}

	//地点
	memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
	//行驶方向
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
	plate.uDirection = (m_nDirection % 4 + 1);
//	LogNormal("---plate.uDirection=%d=m_nDirection=%d\n", plate.uDirection,m_nDirection);

	//车牌结构
	plate.uPlateType = infoViolation.carInfo.carnumrow;
	//车牌颜色
	plate.uColor = infoViolation.carInfo.color;


	//车道编号
	plate.uRoadWayID = infoViolation.nChannel;

	//车速
	double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
	plate.uSpeed = (UINT32)(dSpeed+0.5);


	//发生位置(在当前图片上的)
	plate.uPosLeft  = infoViolation.carInfo.ix;
	plate.uPosTop   = infoViolation.carInfo.iy;
	plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
	plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

	plate.uPicWidth = m_imgComposeMatch->width;
	plate.uPicHeight = m_imgComposeMatch->height;

	//获取录像以及图片路径
	pthread_mutex_lock(&g_Id_Mutex);
	//获取图片路径
	std::string strPicPath;
	////////////////////
	int nSaveRet = GetVtsPicSavePath(plate,nPicCount,strPicPath);

	pthread_mutex_unlock(&g_Id_Mutex);
	//删除已经存在的记录
	g_skpDB.DeleteOldRecord(strPicPath,false,false);

	

	if(nPicCount >= 1)
	{		
		//存储违章图像
		SaveVtsImageMatch(plate,nPicCount,TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime);
	}

	//保存记录
	if(nSaveRet>0)
	{
		LogNormal("Color:[%d:%d]",plate.uCarColor1,plate.uCarColor2);
//		LogNormal("--record_plate.[%s][%s]--tt[%s]-\n", \
			record_plate.dspRecord.chText, foundRecord.dspRecord.chText, plate.chText);
		g_skpDB.SavePlate(m_nChannelId,plate,0);
	}

	//将车牌信息送客户端
	//if(m_bConnect)
	{
		//车牌号码
		memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
//		LogNormal("--SendResult-plate.uChannelID=%d\n", plate.uChannelID);
		SendResult(plate,infoViolation.carInfo.uSeq);
	}
}

//拼车牌遮挡违章图
bool CDspDataProcess::GetVtsImageMatch(IplImage* pImage1, IplImage* pImage2)
{	
	//合成图片清空
	CvRect nFinalRt;
	nFinalRt.x = 0;
	nFinalRt.y = 0;
	nFinalRt.width = m_imgComposeMatch->width;
	nFinalRt.height = m_imgComposeMatch->height;

	printf("************m_imgSnap:%d :%d \n",m_imgSnap->width,m_imgSnap->height);
	printf("************m_imgComposeMatch:%d :%d \n",m_imgComposeMatch->width,m_imgComposeMatch->height);
	cvSet(m_imgComposeMatch, cvScalar( 0,0, 0 ));
	printf("******************** cvSet END\n");

	bool bRet = false;
	int nPicCount = 2;

	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;

	string strPic;
	for(int i = 0; i<nPicCount; i++)
	{		
		//bRet2 = m_pDspManage->GetImageByJpgKey(Pic_Key, &pTimeStamp[1], m_imgComposeResult);
		if(0 == i)
		{
			rectPic.width = pImage1->width;
			rectPic.height = pImage1->height;
			if (rectPic.width == DSP_500_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_500_BIG_HEIGHT_SERVER + 80))
				{
					rectPic.height = DSP_500_BIG_HEIGHT_SERVER + 80;
				}
			}
			else if (rectPic.width == DSP_200_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_200_BIG_HEIGHT_SERVER+80))
				{
					rectPic.height = DSP_200_BIG_HEIGHT_SERVER + 80;
				}
			}

			cvSetImageROI(pImage1,rectPic);
			cvCopy(pImage1, m_imgComposeResult);
			cvResetImageROI(pImage1);
		}
		else if(1 == i)
		{
			rectPic.width = pImage2->width;
			rectPic.height = pImage2->height;
			if (rectPic.width == DSP_500_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_500_BIG_HEIGHT_SERVER + 80))
				{
					rectPic.height = DSP_500_BIG_HEIGHT_SERVER + 80;
				}
			}
			else if (rectPic.width == DSP_200_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_200_BIG_HEIGHT_SERVER+80))
				{
					rectPic.height = DSP_200_BIG_HEIGHT_SERVER + 80;
				}
			}

			cvSetImageROI(pImage2,rectPic);
			cvCopy(pImage2, m_imgComposeResult);
			cvResetImageROI(pImage2);
		}
		else
		{
			//
		}

		//SaveImgTest(m_imgComposeResult);

		CvRect rect;
		CvRect rt;

		int height = m_imgComposeResult->height;
		if(m_nWordOnPic == 1)
		{
			height -= m_nExtentHeight;
		}

		if(m_imgComposeMatch->width == m_imgComposeResult->width)
		{
			rect.x = 0;
			rect.y = i*height;
		}
		else
		{
			rect.x = ((i)%2)*m_imgComposeResult->width;
			rect.y = 0;//((i+1)/2)*height;
		}

		rect.width = m_imgComposeResult->width;
		rect.height = m_imgComposeResult->height - m_nExtentHeight;		

		if(m_nWordPos == 1)
		{
			if(m_nWordOnPic == 0)
			{
				rect.y += m_nExtentHeight;
			}
		}

		rt.x = 0;
		rt.y = 0;
		rt.width = m_imgComposeResult->width;
		rt.height = m_imgComposeResult->height - m_nExtentHeight;

		printf("*********RECT:x:%d y:%d width:%d height%d\n",rect.x,rect.y,rect.width,rect.height);
		cvSetImageROI(m_imgComposeResult,rt);
		cvSetImageROI(m_imgComposeMatch,rect);
		cvResize(m_imgComposeResult, m_imgComposeMatch);
		cvResetImageROI(m_imgComposeMatch);
		cvResetImageROI(m_imgComposeResult);		
	}
	return true;
}

//存储违章图像
void CDspDataProcess::SaveVtsImageMatch(RECORD_PLATE& plate,int nPicCount,PLATEPOSITION* TimeStamp,PLATEPOSITION& SignalTimeStamp,int64_t redLightStartTime)
{					
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
	//大图宽高
	plate.uPicWidth = m_imgComposeMatch->width;
	plate.uPicHeight = m_imgComposeMatch->height;
					
	for(int nIndex = 0; nIndex <nPicCount ; nIndex++)
	{
		PutVtsTextOnImageMatch(m_imgComposeMatch,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime, plate.uViolationType);
	}

	//叠加额外信息
	/*
	if( (g_nVtsPicMode > 0)&& (g_nVtsPicMode < 3))
	{
		if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
		{
			PutVtsTextOnImageMatch(m_imgComposeMatch,plate,nPicCount,TimeStamp,&SignalTimeStamp,redLightStartTime,plate.uViolationType);
		}
		else
		{
			PutVtsTextOnImageMatch(m_imgComposeMatch,plate);
		}
	}
	 */             

	if((nPicCount == 1)&&(g_nDetectMode == 2))
	{
		IplImage* pImage = cvCreateImageHeader(cvSize(m_imgComposeMatch->width, m_imgComposeMatch->height/2), 8, 3);
		cvSetData(pImage,m_imgComposeMatch->imageData,pImage->widthStep);
		plate.uPicSize = SaveImage(pImage,strPicPath,2);
		cvReleaseImageHeader(&pImage);
	}
	else
	{
		//SaveImgTest(m_imgComposeMatch, plate.chText);
		plate.uPicSize = SaveImage(m_imgComposeMatch,strPicPath,2);
	}
}

//电警叠加文本信息
void CDspDataProcess::PutVtsTextOnImageMatch(IplImage*pImage,RECORD_PLATE plate,int nIndex,PLATEPOSITION* pTimeStamp,PLATEPOSITION* pSignalTimeStamp,int64_t redLightStartTime, UINT32 uViolationType)
{
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);

	int timeIndex = nIndex;
    if(pTimeStamp != NULL)
    {
        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = 10;
        int nHeight = pImage->height - m_nExtentHeight;
	
		//LogNormal("*********seq:%d\n",plate.uSeq);

		printf("--PutVtsTextOnImageMatch-------pImage->width=%d--height=%d-----\n",\
			pImage->width, pImage->height);

		string strText("");


        if(m_nWordOnPic == 1)//字直接叠加在图上
        {
			if(pImage->width > m_imgSnap->width)
			{
				//
			}
        }
        else
        {
			//LogTrace("PutVtsText.log", "222=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d=m_nWordPos=%d", \
				g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode, m_nWordPos);
			
			//if (3 == g_nVtsPicMode)//大图1x2叠加
			{
				if(0 == nIndex)
				{				
					nWidth = 10+nIndex*400;				
					string strTmp = "";

					//路口名称
					//sprintf(chOut,"路口名称:  [%s]",m_strLocation.c_str());	
					sprintf(chOut,"路口名称:  [%s]",plate.chPlace);
					strTmp = chOut;
					strText += strTmp;
				
					//经过时间
					std::string strTime;
					strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,1);
					sprintf(chOut,"经过时间:%s%03d毫秒",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
					strTmp = chOut;
					strText += strTmp;

					/*
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					nHeight += 30;
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					*/

					//车牌号码
					if(g_PicFormatInfo.nCarNum == 1/* && 1 != g_PicFormatInfo.nSmallViolationPic*/)
					{
						std::string strCarNum(plate.chText);
						sprintf(chOut,"车牌号码:  %s  ",strCarNum.c_str());
						strTmp = chOut;
						strText += strTmp;
					}

					//车速
					if(g_PicFormatInfo.nCarSpeed == 1 && 1 != g_PicFormatInfo.nSmallViolationPic)
					{
						//行驶速度
						sprintf(chOut,"速度:%dkm/h  ",plate.uSpeed);
						strTmp = chOut;
						strText += strTmp;
					}

					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					nHeight += 30;
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					//方向，限速
					/*
					int nOnlyOverSpedMax = 10;
					nOnlyOverSpedMax = GetMaxSpeed(plate.uType, plate.uChannelID, plate.uRoadWayID);
					std::string strDirection = GetDirection(plate.uDirection);
					sprintf(chOut,"方向:%s  车辆限速:%dkm/h",strDirection.c_str(),nOnlyOverSpedMax);//
					strText = chOut;
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					nHeight += 30;
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

					//速度，违法名称，超速百分比
					int nOverSped = (plate.uSpeed-nOnlyOverSpedMax)*100/nOnlyOverSpedMax;
					std::string strViolationType = GetViolationType(plate.uViolationType,1);
					sprintf(chOut,"速度:%dkm/h  违法名称:%s  超速百分比:%d%%",plate.uSpeed,strViolationType.c_str(),nOverSped);
					strText = chOut;
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					nHeight += 30;
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
					*/

				}//end of if
				return;
			}		
			
            if(g_PicFormatInfo.nViolationType == 1)
            {
               //违章行为
                if(plate.uViolationType != 0)
                {
                    std::string strViolationType = GetViolationType(plate.uViolationType,1);

					/*
					if (10 == g_nServerType)
						sprintf(chOut,"违法行为:%s",strViolationType.c_str());
					else
					*/
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
		//LogTrace("PutVtsText.log", "333=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d", \
			g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode);		
    }

	m_cvText.UnInit();
}


//存图测试
void CDspDataProcess::SaveImgTest(IplImage *pImg, char* strFileName = NULL)
{
	static int nId = 0;
	BYTE* pOutImage = NULL;
	pOutImage = new BYTE[pImg->width * pImg->height / 4];

	char jpg_name[256] = {0};
	
	if (strFileName == NULL)
	{
		sprintf(jpg_name, "./text/TD_%d_%d_%d_%d_%u-seq.jpg", \
			nId, pImg->width, pImg->height, pImg->imageSize, GetTimeStamp());
	}
	else
	{
		sprintf(jpg_name,"./text/%s.jpg",strFileName);
	}
	
	//sprintf(jpg_name, "./text/TD_%d_%d_%d_%d_TT_2-seq.jpg", nId, pImg->width, pImg->height, pImg->imageSize);
	CxImage image;
	int srcstep = 0;

	if(image.IppEncode((BYTE*)pImg->imageData, pImg->width, pImg->height, \
		3, &srcstep, pOutImage, g_PicFormatInfo.nJpgQuality))
	{
		FILE* fp = NULL;
		fp = fopen(jpg_name, "a+");
		if(fp!=NULL)
		{
			fwrite(pOutImage, srcstep, 1, fp);
		}
	}

	if(pOutImage != NULL)
	{
		delete [] pOutImage;
		pOutImage = NULL;
	}

	nId++;
}

//找临近图片
bool CDspDataProcess::GetImgByJpgKeyNeighbor(
	const Picture_Elem &picElem,
	PLATEPOSITION* pTimeStamp,
	IplImage* pImage,
	const int iIndex,
	const bool bRet1,
	const bool bRet2,
	const bool bRet3)
{
	bool bRet = false;
	bool bRetTemp = false;
	int iTemp = -1;
	if(0 == iIndex)
	{
		if(!bRet1)
		{
			if(bRet2)
			{
				bRetTemp = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[1], pImage);
				if(bRetTemp)//在次取图, 不一定取得到. FIXME ERROR!
				{
					iTemp = 1;
				}				
			}
			else if(bRet3)
			{				
				bRetTemp = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[2], pImage);
				if(bRetTemp)//在次取图, 不一定取得到. FIXME ERROR!
				{
					iTemp = 2;
				}	
			}
			else{}
		}
	}
	else if(1 == iIndex)
	{
		if(!bRet2)
		{
			if(bRet1)
			{
				bRetTemp = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[0], pImage);
				if(bRetTemp)//在次取图, 不一定取得到. FIXME ERROR!
				{
					iTemp = 0;
				}			
			}
			else if(bRet3)
			{				
				bRetTemp = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[2], pImage);
				if(bRetTemp)//在次取图, 不一定取得到. FIXME ERROR!
				{
					iTemp = 2;
				}
			}
			else{}
		}
		
	}
	else if(2 == iIndex)
	{
		if(!bRet3)
		{
			if(bRet1)
			{
				bRetTemp = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[0], pImage);
				if(bRetTemp)//在次取图, 不一定取得到. FIXME ERROR!
				{
					iTemp = 0;
				}
			}
			else if(bRet2)
			{				
				bRetTemp = m_pDspManage->GetImageByJpgKey(picElem, &pTimeStamp[1], pImage);
				if(bRetTemp)//在次取图, 不一定取得到. FIXME ERROR!
				{
					iTemp = 1;
				}
			}
			else{}
		}
	}
	else{}
	
	
	if(iTemp != -1)
	{
		LogTrace("FindLog.txt", "-bRet=%d -iIndex:%d iTemp:%d tsIndex:%d tsTemp:%d", \
			bRet, iIndex, iTemp, (pTimeStamp[iIndex].ts/1000)/1000, (pTimeStamp[iTemp].ts/1000)/1000);
		pTimeStamp[iIndex].ts			= pTimeStamp[iTemp].ts;
		pTimeStamp[iIndex].uTimestamp	= pTimeStamp[iTemp].uTimestamp;
		pTimeStamp[iIndex].uFieldSeq	= pTimeStamp[iTemp].uFieldSeq;
		pTimeStamp[iIndex].uSeq			= pTimeStamp[iTemp].uSeq;
		bRet = true;		
	}	

	return bRet;
}

//拼图2x2
bool CDspDataProcess::ComposePic2x2(const int iIndex, const RECORD_PLATE &plate)
{
	bool bRet = false;
	CvRect rect;
	CvRect rt;

	int height = m_imgComposeResult->height;
	if(m_nWordOnPic == 1)
	{
		height -= m_nExtentHeight;
	}

	if(m_imgComposeSnap->width == m_imgComposeResult->width)
	{
		rect.x = 0;
		rect.y = iIndex*height;
	}
	else
	{
		rect.x = ((iIndex+1)%2)*m_imgComposeResult->width;
		rect.y = ((iIndex+1)/2)*height;
	}

	rect.width = m_imgComposeResult->width;
	rect.height = m_imgComposeResult->height - m_nExtentHeight;

	if(m_nWordPos == 1)
	{
		if(m_nWordOnPic == 0)
		{
			//rect.y += m_nExtentHeight;
			rect.height = m_imgComposeResult->height;	
		}
	}

	rt.x = 0;
	rt.y = 0;
	rt.width = m_imgComposeResult->width;
	rt.height = m_imgComposeResult->height - m_nExtentHeight;	

	//LogNormal("*RECT:x:%d y:%d width:%d height%d\n",rect.x,rect.y,rect.width,rect.height);
	cvSetImageROI(m_imgComposeResult,rt);
	cvSetImageROI(m_imgComposeSnap,rect);
	cvResize(m_imgComposeResult, m_imgComposeSnap);
	cvResetImageROI(m_imgComposeSnap);
	cvResetImageROI(m_imgComposeResult);

	int picIndex = 0;
	/*
	if (DETECT_RESULT_RETROGRADE_MOTION == uViolationType)
		picIndex = 2;
	*/

	if (iIndex == picIndex)
	{
		//违章图片加小图 并且 2x2叠加图
		if (1 == g_PicFormatInfo.nSmallViolationPic)
		{
			CvRect srcRt,dstRt;
			dstRt = rect;

			dstRt.x = 0;
			dstRt.y = 0;
			if(m_nWordPos == 1)
			{
				if(m_nWordOnPic == 0)
				{
					dstRt.y = m_nExtentHeight;
				}
			}

			//取车身位置
			srcRt = GetCarPos(plate);
			//LogNormal("SrcRt x:%d,y:%d,width:%d,height:%d\n",srcRt.x,srcRt.y,srcRt.width,srcRt.height);
			//LogNormal("left:%d,Top:%d,right:%d,bottom:%d\n",plate_r.uPosLeft,plate_r.uPosTop,plate_r.uPosRight,plate_r.uPosBottom);	

			//按目标区域比例裁剪
			srcRt.width = srcRt.height * dstRt.width/dstRt.height;

			if(srcRt.x + srcRt.width >= dstRt.width)
			{
				srcRt.x = dstRt.width - srcRt.width-1;
			}			

			cvSetImageROI(m_imgComposeResult,srcRt);
			cvSetImageROI(m_imgComposeSnap,dstRt);
			cvResize(m_imgComposeResult, m_imgComposeSnap);
			cvResetImageROI(m_imgComposeSnap);
			cvResetImageROI(m_imgComposeResult);

			bRet = true;
		}
	}

	return bRet;
}

//拼图1x3
bool CDspDataProcess::ComposePic1x3(const int iIndex, const RECORD_PLATE &plate)
{
	bool bRet = false;

	//[pic1][pic2][pic3][pic4特]
	bool bRetPic[3] = { false, false, false};
	bool bRetTeXie = false;

	CvRect srcRt;
	CvRect dstRt;

	if(m_nWordOnPic == 1)
	{
		//
	}
	else if(m_nWordOnPic == 0)
	{
		//拼单张图 [pic1][pic2][pic3]
		if(1 == m_nWordPos)//WORD_POS_UP
		{
			srcRt.x = 0;
			srcRt.y = m_nExtentHeight;
			srcRt.width = m_imgComposeResult->width;
			srcRt.height = m_imgComposeResult->height - m_nExtentHeight;
		}
		else //WORD_POS_DOWN
		{
			srcRt.x = 0;
			srcRt.y = 0;
			srcRt.width = m_imgComposeResult->width;
			srcRt.height = m_imgComposeResult->height - m_nExtentHeight;
		}

		bool bGetSrcRt = false;
		bool bGetDstRt = false;

		bGetSrcRt = CheckRectInImg(srcRt, m_imgComposeResult);
		bGetDstRt = GetRectByIndex_1x3(iIndex, m_imgComposeResult, m_imgComposeSnap, dstRt);

		if(bGetSrcRt && bGetDstRt)
		{			
			//LogNormal("srcRt[%d,%d,%d,%d]", srcRt.x, srcRt.y, srcRt.width, srcRt.height);
			//LogNormal("dstRt[%d,%d,%d,%d]", dstRt.x, dstRt.y, dstRt.width, dstRt.height);
			bRetPic[iIndex] = ComposeImg(srcRt, m_imgComposeResult, dstRt, m_imgComposeSnap);		
		}	

		//拼特写图 [pic4特]
		if(0 == iIndex)//取第一张 FIX,后面需考虑逆行情况
		{
			CvRect srcRtTe;
			CvRect dstRtTe;
			bool bGetSrcRtTe = GetTeXieRect(m_imgComposeResult, plate, srcRtTe);
			bool bGetDstRtTe = GetRectByIndex_1x3(3, m_imgComposeResult, m_imgComposeSnap, dstRtTe);

			if(bGetSrcRtTe && bGetDstRtTe)
			{
				//拼特写图
				bRetTeXie = ComposeImg(srcRtTe, m_imgComposeResult, dstRtTe, m_imgComposeSnap);	
			}

			if(!bRetTeXie)
			{
				LogNormal("--ComposeTeXie fail!\n");
			}
		}
	}
	else{}

	if(bRetPic[0] && bRetPic[1] && bRetPic[2] && bRetTeXie)
	{
		bRet = true;
	}

	return bRet;
}

//获取合成图1x3,各张图位置
bool CDspDataProcess::GetRectByIndex_1x3(
	const int i,
	const IplImage* pImageSrc,
	const IplImage* pImageDst,
	CvRect &rect)
{
	bool bRet = false;	

	int nWidth = pImageSrc->width;
	int nHeight = pImageSrc->height - m_nExtentHeight;	

	if(1 == m_nWordPos)//WORD_POS_UP
	{
		rect.x = i * nWidth;		
		rect.y = m_nExtentHeight;

		rect.width = nWidth;
		rect.height = nHeight;
	}
	else //WORD_POS_DOWN
	{
		rect.x = i * nWidth;
		rect.y = 0;

		rect.width = nWidth;
		rect.height = nHeight;
	}

	//LogNormal("m_nWordPos:%d rectPic:[%d,%d,%d,%d]", m_nWordPos, rect.x, rect.y, rect.width, rect.height);

	bRet = CheckRectInImg(rect, pImageDst);

	return bRet;
}

//核查rect是否合法
bool CDspDataProcess::CheckRectInImg(const CvRect &rect, const IplImage* pImage)
{
	bool bRet = false;

	if(rect.x >= 0 && rect.y >= 0)
	{
		if(rect.width > 0 && rect.height > 0)
		{
			if(rect.x <= pImage->width && rect.y <= pImage->height)
			{
				if((rect.width <= (pImage->width - rect.x)) && 
					(rect.height <= (pImage->height - rect.y)))
				{
					bRet = true;			
				}		
			}
		}
	}	

	return bRet;
}

//拼违章图
/*
* rectSrc:源图关注区域
* rectDst:目标图区域
*/
//拼违章图
bool CDspDataProcess::ComposeImg(
	const CvRect &rectSrc, 
	IplImage* pImageSrc,
	const CvRect &rectDst,	
	IplImage* pImageDst)
{
	bool bRet = false;
	bool bCheck1 = CheckRectInImg(rectSrc, pImageSrc);
	bool bCheck2 = CheckRectInImg(rectDst, pImageDst);

	if(bCheck1 && bCheck2)
	{
		if(pImageSrc != NULL && pImageDst != NULL)
		{
			cvSetImageROI(pImageSrc,rectSrc);
			cvSetImageROI(pImageDst,rectDst);
			cvResize(pImageSrc, pImageDst);//resize src to dest
			cvResetImageROI(pImageSrc);
			cvResetImageROI(pImageDst);

			bRet = true;
		}		
	}

	return bRet;
}

//获取合成图,特写图区域
bool CDspDataProcess::GetComposeTeXieRect(
	const IplImage* pImageSrc,
	const IplImage* pImageDst,
	CvRect &rect)
{
	bool bRet = false;

	int iIndex = 0;

	if(1 == g_nVtsPicMode)//2x2
	{
		iIndex = 0;
	}
	else if(5 == g_nVtsPicMode)//1x3
	{
		iIndex = 3;
	}
	else{}

	bRet = GetRectByIndex_1x3(iIndex, pImageSrc, pImageDst, rect);

	return bRet;
}

//获取单张图,特写图区域
bool CDspDataProcess::GetTeXieRect(
	const IplImage* pImageSrc,
	const RECORD_PLATE &plate, 
	CvRect &rtDst)
{
	bool bRet = false;

	CvRect rtSrc;
	CvRect rtSrcImg;

	//取车牌Rect
	rtSrc.x = plate.uPosLeft;
	rtSrc.y = plate.uPosTop;
	rtSrc.width = plate.uPosRight - plate.uPosLeft;
	rtSrc.height = plate.uPosBottom - plate.uPosTop;

	//取图片Rect
	rtSrcImg.x = 0;
	rtSrcImg.y = 0;
	rtSrcImg.width = pImageSrc->width;
	rtSrcImg.height = pImageSrc->height;

	bool bPlateRect = CheckRectInRect(rtSrc, rtSrcImg);
	if(!bPlateRect)
	{
		rtDst.x = 500;
		rtDst.y = 500;
		rtDst.width = 800;
		rtDst.height = 600;

		LogNormal("plate pos err!");

		return false;
	}

	int nHeightC1 = 20;//参考固定高, eg:20(车牌高度常数)	//20
	int nWidthExtC1 = 600;//参考外扩区域宽, eg:800			//160 600 400
	int nHeightExtC2 = 480;//参考外扩区域高, eg:600			//40 480 320
	int nHeightExtC3 = 320;//参考外扩区域,抠图区域高, eg:400 //20 320 220

	bool bGetCarPosRect = GetCarPosRect(nHeightC1,nWidthExtC1,nHeightExtC2,nHeightExtC3,rtSrc,rtSrcImg,rtDst);
	if(bGetCarPosRect)
	{
		bRet = CheckRectInImg(rtDst, pImageSrc);
	}
	else
	{
		//LogNormal("GetCarPostion err!");

		if(rtDst.width < 10 || rtDst.height < 10)
		{
			rtDst.x = pImageSrc->width/8;
			rtDst.y = pImageSrc->height/8;
			rtDst.width = (pImageSrc->width*3)/8;
			rtDst.height = (pImageSrc->height*3)/8;
		}
	}
		
	LogNormal("plate[%d,%d,%d,%d] TeXieRect[%d,%d,%d,%d]", \
		rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height,
		rtDst.x, rtDst.y, rtDst.width, rtDst.height);

	return bRet;
}

//在1x3合成图上,叠字
void CDspDataProcess::PutVtsTextOnImage1x3(
	IplImage* pImage,
	RECORD_PLATE& plate,
	PLATEPOSITION* TimeStamp,
	PLATEPOSITION& SignalTimeStamp,
	int64_t redLightStartTime)
{
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));

	//大图宽高
	plate.uPicWidth = pImage->width;
	plate.uPicHeight = pImage->height;

	for(int nIndex = 0; nIndex <3 ; nIndex++)
	{
		//PutVtsTextOnImage(pImage,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime, plate.uViolationType);
		if ( 1 == g_nGongJiaoMode)
		{
			PutGJVtsTextOnImage1x3_Elem(pImage,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime);
		}
		else
		{
			PutVtsTextOnImage1x3_Elem(pImage,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime);
		}	
	}

	/*
	//叠加额外信息
	if( (g_nVtsPicMode > 0)&& (g_nVtsPicMode < 3))
	{
		if (1 == g_PicFormatInfo.nSmallViolationPic && 1 == g_nVtsPicMode)
			PutVtsTextOnImage(m_imgComposeSnap,plate,nPicCount,TimeStamp,&SignalTimeStamp,redLightStartTime,plate.uViolationType);
		else
			PutVtsTextOnImage(m_imgComposeSnap,plate);
	}
	*/

	//g_nDetectMode == 2
	/*
	if(nPicCount == 1)
	{
		IplImage* pImage = cvCreateImageHeader(cvSize(m_imgComposeSnap->width, m_imgComposeSnap->height/2), 8, 3);
		cvSetData(pImage,m_imgComposeSnap->imageData,pImage->widthStep);
		plate.uPicSize = SaveImage(pImage,strPicPath,2);
		cvReleaseImageHeader(&pImage);
	}
	else
	*/
	{
		plate.uPicSize = SaveImage(pImage,strPicPath,2);
	}
}

//1x3单张图,电警叠加文本信息
void CDspDataProcess::PutVtsTextOnImage1x3_Elem(
	IplImage*pImage,
	RECORD_PLATE &plate,
	int nIndex,
	PLATEPOSITION* pTimeStamp,
	PLATEPOSITION* pSignalTimeStamp,
	int64_t redLightStartTime)
{
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);

	UINT32 uViolationType = plate.uViolationType;

	int timeIndex = nIndex;	

    if(pTimeStamp != NULL)
    {
        if(m_nWordOnPic == 1)//字直接叠加在图上
        {
            //..
        }
        else
        {
			//LogTrace("PutVtsText.log", "222=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d=m_nWordPos=%d", \
				g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode, m_nWordPos);

			wchar_t wchOut[255] = {'\0'};
			char chOut[255] = {'\0'};

			int nWidth = 10;
			int nHeight = 0;

			string strText("");

			if(0 == nIndex)
			{
				//设备编号			
				{
					LogNormal("******plate.uLongitude = %u,uLatitude = %u\n",plate.uLongitude,plate.uLatitude);
					double Longitude = (double)(plate.uLongitude/1000000) + (double)(plate.uLongitude - (plate.uLongitude/1000000)*1000000)*1.0/(1000000);
					double Latitude = (double)(plate.uLatitude/1000000) + (double)(plate.uLatitude - (plate.uLatitude/1000000)*1000000)*1.0/(1000000);
									
					
					sprintf(chOut,"设备编号:%d  经度:%.4f  纬度:%.4f ", \
						plate.uChannelID,Longitude,Latitude);
					
					string strTmp(chOut);
					strText += strTmp;
				}			

				//车牌号码
				if(g_PicFormatInfo.nCarNum == 1/* && 1 != g_PicFormatInfo.nSmallViolationPic*/)
				{
					std::string strCarNum(plate.chText);
					sprintf(chOut,"车牌号码:%s  ",strCarNum.c_str());
					string strTmp(chOut);
					strText += strTmp;
				}				

				//违法时间
				{
					std::string strTime;
					strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);			
					sprintf(chOut,"违法时间:%s:%03d  ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);			        
					string strTmp(chOut);
					strText += strTmp;
				}                    

				if(g_PicFormatInfo.nViolationType == 1)
				{
				   //违章行为
					if(plate.uViolationType != 0)
					{
						std::string strViolationType = GetViolationType(plate.uViolationType,1);					
						sprintf(chOut,"违章行为:%s",strViolationType.c_str());
						string strTmp(chOut);
						strText += strTmp;
					}
				}
				
				//公交模式下叠加相机编号和防伪码
				if (g_nGongJiaoMode == 1)
				{					
					int nRandCode = g_RoadImcData.GetRandCode();
					sprintf(chOut,"  相机编号:%s 防伪码:%d",plate.szCameraCode,nRandCode);
					string strTmp(chOut);
					strText += strTmp;

				}
				//设置叠字位置
				{
					//LogTrace("PutVtsText.log", "=11===nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);

					nWidth = 20;
					nHeight = 10 + m_nExtentHeight/2;	

					if(0 == m_nWordPos)//down
					{
						nHeight = (pImage->height - m_nExtentHeight/2 + 10);
						//LogNormal("11t down:w:%d, h:%d", nWidth, nHeight);
					}
				}

				//写入图片
				{
					//LogTrace("PutVtsText.log", "=22===nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
				}


				//特写图片叠字
				{
					/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)			
					strText.clear();

					//设置叠字位置
					{
						nWidth = 20 + (m_imgSnap->width)*3;
						nHeight = 20 + m_nExtentHeight + m_nExtentHeight/2;

						if(0 == m_nWordPos)//down
						{
							nHeight = (pImage->height - m_nExtentHeight - 20);
							//LogNormal("22 down:w:%d, h:%d", nWidth, nHeight);
						}
					}

					//时间
					{
						std::string strTime;
						strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);			
						sprintf(chOut,"%s:%03d  ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);			        
						string strTmp(chOut);
						strText += strTmp;
					}   

					//写入图片
					{
						//LogTrace("PutVtsText.log", "=33===nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);
						memset(wchOut,0,sizeof(wchOut));
						UTF8ToUnicode(wchOut,(char*)strText.c_str());
						m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
					}
					/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)
				}
			}//End of if(0 == nIndex)

			/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)			
			strText.clear();

			//设置叠字位置
			{
				nWidth = 20 + (m_imgSnap->width)*nIndex;
				nHeight = 20 + m_nExtentHeight + m_nExtentHeight/2;

				if(0 == m_nWordPos)//down
				{
					nHeight = (pImage->height - m_nExtentHeight - 20);
					//LogNormal("33 down:w:%d, h:%d", nWidth, nHeight);
				}
			}

			//时间
			{
				std::string strTime;
				strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);			
				sprintf(chOut,"%s:%03d  ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);			        
				string strTmp(chOut);
				strText += strTmp;
			}   

#ifdef VTS_TEST_SEQ
			//记录帧号
			sprintf(chOut,"Seq:[%d]", \
				((pTimeStamp+nIndex)->uSeq));

			string strTmpTest(chOut);
			strText += strTmpTest;
#endif

			//写入图片
			{
				//LogTrace("PutVtsText.log", "=44===nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
			}
			/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)			
        }
    }
    else //空白区域
    {
		//LogTrace("PutVtsText.log", "333=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d", \
			g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode);
    }

	m_cvText.UnInit();
}

//重设违章图片车牌区域坐标
void CDspDataProcess::ReSetVtsPlatePos(const ViolationInfo& infoViolation, RECORD_PLATE &plate)
{
	if(g_nVtsPicMode == 1)//2x2
	{
		plate.uPosLeft += m_imgComposeResult->width;
		plate.uPosRight += m_imgComposeResult->width;
	}
	else if(g_nVtsPicMode == 5)//1x3
	{
		//默认取第一张
	}
	else
	{
		//默认取第一张
	}
}


//输出车牌遮挡前后匹配图-天津版本
void CDspDataProcess::OutPutVtsMatchForTianJin(
	RECORD_PLATE_DSP_MATCH &record_plate, 
	RECORD_PLATE_DSP_MATCH &foundRecord, 
	RECORD_PLATE& plate)
{
	//LogTrace("FBMach.log", "OutPutVts video1:%s ", record_plate.dspRecord.chVideoPath);
	//LogTrace("FBMach.log","OutPutVts video2:%s ", foundRecord.dspRecord.chVideoPath);
	//LogTrace("FBMach.log","OutPutVts plate:%s ", plate.chVideoPath);

	//LogTrace("FBMach.log","-OutPutVtsMatch-[%s]-[%s]\n", record_plate.dspRecord.chText, foundRecord.dspRecord.chText);
	CarInfo    carnums;
	char text[10] = "123456789";
	if(foundRecord.dspRecord.chText[0] != 'L') //非武警牌照
	{
		memset(text, 0, 10);
		memcpy(text, foundRecord.dspRecord.chText, 7);
		memcpy(carnums.strCarNum, text, 7);
	}
	else
	{
		memset(text, 0, 10);
		memcpy(text, foundRecord.dspRecord.chText, 9);
		memcpy(carnums.strCarNum, text, 7);
		memcpy(carnums.wj, text+7, 2); //武警牌下面的小数字
	}

	carnums.color = record_plate.dspRecord.uColor;
	carnums.vehicle_type = record_plate.dspRecord.uType;
	carnums.ix = (record_plate.dspRecord.uPosLeft);
	carnums.iy = (record_plate.dspRecord.uPosTop);
	carnums.iwidth = (record_plate.dspRecord.uPosRight - record_plate.dspRecord.uPosLeft);
	carnums.iheight = (record_plate.dspRecord.uPosBottom - record_plate.dspRecord.uPosTop);
	carnums.mean = -1;
	carnums.stddev = -1;

	//carnums.VerticalTheta = record_plate.dspRecord.uVerticalTheta;
	//carnums.HorizontalTheta = record_plate.dspRecord.uHorizontalTheta;

	carnums.uTimestamp = record_plate.dspRecord.uTime;//(pHeader->ts)/1000/1000;
	carnums.ts = record_plate.dspRecord.uTime * 1000 * 1000 + record_plate.dspRecord.uMiTime * 1000;
	carnums.uSeq = record_plate.dspRecord.uSeqID;
	carnums.carnumrow = record_plate.dspRecord.uType;//?
	carnums.wx = 0; //(double)((pPlate->uLongitude) * 0.0001);
	carnums.wy = 0; //(double)((pPlate->uLatitude) * 0.0001);
	carnums.id = 0; //pHeader->uCameraId;
	carnums.vx = 0;
	carnums.vy = record_plate.dspRecord.uSpeed; //传入dsp相机线圈检测出速度
	carnums.RoadIndex = record_plate.dspRecord.uRoadWayID;
	carnums.nDirection = record_plate.dspRecord.uDirection;

	ViolationInfo infoViolation;
	//电警数据
	{
		memcpy(&infoViolation.carInfo,&carnums,sizeof(CarInfo));
		infoViolation.evtType = (VIO_EVENT_TYPE)record_plate.dspRecord.uViolationType;//违章类型
		infoViolation.nChannel = record_plate.dspRecord.uRoadWayID;//车道编号
		infoViolation.nPicCount = 2;//图片数量
	}

	memcpy((char*)(&plate),(char*)(&record_plate.dspRecord),sizeof(RECORD_PLATE));
	plate.uCarColor1 = record_plate.dspRecord.uCarColor1;
	plate.uCarColor2 = foundRecord.dspRecord.uCarColor1;
	
	//违章类型
	plate.uViolationType = DETECT_MATCH_PLATE; //infoViolation.evtType;

	std::string strCarNum = infoViolation.carInfo.strCarNum;
	//判断是否有车牌的车
	bool bCarNum = true;
	if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
	{
		bCarNum = false;
	}
	
	if(bCarNum)
	{
		//车牌号码转换
		CarNumConvert(strCarNum,infoViolation.carInfo.wj);
	}
	
	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	int nPos = -1;
	string nStrNum = plate.chText;
	nPos = nStrNum.find("*");

	//车辆类型
	plate.uType =  infoViolation.carInfo.vehicle_type;

	//车型细分
	if(m_nDetectKind&DETECT_TRUCK)
		plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);

	//车辆类型转换
	CarTypeConvert(plate);

	int nPicCount = infoViolation.nPicCount;//图片数量

	if(nPicCount > 2)
	{
		nPicCount = 2;
	}

	if(1 == g_PicFormatInfo.nSmallViolationPic)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0];
		infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
		infoViolation.frameSeqs[1] = frameSeqs;
	}

	PLATEPOSITION  TimeStamp[6];
	PLATEPOSITION  SignalTimeStamp;
	TimeStamp[0].uTimestamp = record_plate.dspRecord.uTime;
	TimeStamp[1].uTimestamp = foundRecord.dspRecord.uTime;
	if(nPicCount>=1)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];
		SignalTimeStamp.x = infoViolation.carInfo.ix;
		SignalTimeStamp.y = infoViolation.carInfo.iy;
		SignalTimeStamp.width = infoViolation.carInfo.iwidth;
		SignalTimeStamp.height = infoViolation.carInfo.iheight;
		SignalTimeStamp.nType = plate.uType;
		SignalTimeStamp.IsCarnum = bCarNum;

		bool bRet = false;
		//获取违章检测结果图像
		//车牌世界坐标
		plate.uLongitude = (UINT32)(infoViolation.carInfo.wx*10000*100);
		plate.uLatitude = (UINT32)(infoViolation.carInfo.wy*10000*100);
		//相机ID
		plate.uChannelID = infoViolation.carInfo.id;

		//发生位置(在当前图片上的)
		plate.uPosLeft  = infoViolation.carInfo.ix;
		plate.uPosTop   = infoViolation.carInfo.iy;
		plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
		plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

		//LogNormal(":%s ,%d,%d,%d,%d record_plate", \
		//	record_plate.dspRecord.chText, record_plate.dspRecord.uPosLeft, record_plate.dspRecord.uPosTop, record_plate.dspRecord.uPosRight, record_plate.dspRecord.uPosBottom);
		//LogNormal(":%s ,%d,%d,%d,%d foundRecord", \
		//	foundRecord.dspRecord.chText, foundRecord.dspRecord.uPosLeft, foundRecord.dspRecord.uPosTop, foundRecord.dspRecord.uPosRight, foundRecord.dspRecord.uPosBottom);

		Picture_Key Pic_Key;
		Pic_Key.uSeq = infoViolation.carInfo.uSeq;
		Pic_Key.uCameraId = infoViolation.carInfo.id;	

		printf("-----1111111---GetVtsImageMatch----\n");
		//SaveImgTest(record_plate.pImg);

		//合成前后车牌匹配和特写图
		GetVtsImageMatchForTianJin(record_plate, foundRecord);
		printf("-----22222222---GetVtsImageMatch----\n");
	}

	//经过时间(秒)
	plate.uTime = infoViolation.carInfo.uTimestamp;
	//毫秒
	plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;

	//地点
	memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
	//行驶方向
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
	plate.uDirection = (m_nDirection % 4 + 1);
//	LogNormal("---plate.uDirection=%d=m_nDirection=%d\n", plate.uDirection,m_nDirection);

	//车牌结构
	plate.uPlateType = infoViolation.carInfo.carnumrow;
	//车牌颜色
	plate.uColor = infoViolation.carInfo.color;
	//车道编号
	plate.uRoadWayID = infoViolation.nChannel;
	//车速
	double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
	plate.uSpeed = (UINT32)(dSpeed+0.5);
	//发生位置(在当前图片上的)
	plate.uPosLeft  = infoViolation.carInfo.ix;
	plate.uPosTop   = infoViolation.carInfo.iy;
	plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
	plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);
	plate.uPicWidth = m_imgComposeMatch->width;
	plate.uPicHeight = m_imgComposeMatch->height;

	//获取录像以及图片路径
	pthread_mutex_lock(&g_Id_Mutex);
	//获取图片路径
	std::string strPicPath;
	////////////////////
	int nSaveRet = GetVtsPicSavePath(plate,nPicCount,strPicPath);
	pthread_mutex_unlock(&g_Id_Mutex);

	//删除已经存在的记录
	g_skpDB.DeleteOldRecord(strPicPath,false,false);
	if(nPicCount >= 1)
	{
		//存储违章图像
		SaveVtsImageMatchForTianJin(plate,nPicCount,TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime);
	}

	//保存记录
	if(nSaveRet>0)
	{
		//LogNormal("Color:[%d:%d]",plate.uCarColor1,plate.uCarColor2);
		//LogNormal("--record_plate.[%s][%s]--tt[%s]-\n", \
			record_plate.dspRecord.chText, foundRecord.dspRecord.chText, plate.chText);
		LogNormal("video:%s ", plate.chVideoPath);

		//LogTrace("FBMach.log","video:%s ", plate.chVideoPath);
		g_skpDB.SavePlate(m_nChannelId,plate,0);
	}

	//将车牌信息送客户端
	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	//LogNormal("--SendResult-plate.uChannelID=%d\n", plate.uChannelID);
	SendResult(plate,infoViolation.carInfo.uSeq);
}

//合成前后车牌匹配和特写图
bool CDspDataProcess::GetVtsImageMatchForTianJin(
	RECORD_PLATE_DSP_MATCH &record_plate, 
	RECORD_PLATE_DSP_MATCH &foundRecord)
{
	IplImage* pImage1 = record_plate.pImg;
	IplImage* pImage2 = foundRecord.pImg;

	//assert(pImage1 != NULL);
	//SaveImgTest(pImage1);	
	//assert(pImage2 != NULL);
	//SaveImgTest(pImage2);

	if(pImage1 == NULL || pImage2 == NULL)
	{
		return false;
	}

	//合成图片清空
	CvRect nFinalRt;
	nFinalRt.x = 0;
	nFinalRt.y = 0;
	nFinalRt.width = m_imgComposeMatch->width;
	nFinalRt.height = m_imgComposeMatch->height;

	printf("m_imgComposeMatch:%x ", m_imgComposeMatch);

	printf("************m_imgSnap:%d :%d \n",m_imgSnap->width,m_imgSnap->height);
	printf("************m_imgComposeMatch:%d :%d \n",m_imgComposeMatch->width,m_imgComposeMatch->height);

	cvSet(m_imgComposeMatch, cvScalar( 0,0, 0 ));
	printf("******************** cvSet END\n");

	bool bRet = false;
	int nPicCount = 2;

	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;

	string strPic;
	for(int i = 0; i<nPicCount; i++)
	{		
		CvRect rect;
		CvRect rt;

		printf("000************* i=%d \n", i);

		if(0 == i)
		{
			rectPic.width = pImage1->width;
			rectPic.height = pImage1->height;
			if (rectPic.width == DSP_500_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_500_BIG_HEIGHT_SERVER+m_nExtentHeight))
				{
					rectPic.height = DSP_500_BIG_HEIGHT_SERVER+m_nExtentHeight;
				}
			}
			else if (rectPic.width == DSP_200_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_200_BIG_HEIGHT_SERVER+m_nExtentHeight))
				{
					rectPic.height = DSP_200_BIG_HEIGHT_SERVER+m_nExtentHeight;
				}
			}
			printf("--rectPic[%d,%d]---\n", rectPic.width, rectPic.height);
			printf("--pImage1[%d,%d]---\n", pImage1->width, pImage1->height);
			printf("m_imgComposeResult:%x", m_imgComposeResult);
			printf("--m_imgComposeResult[%d,%d]---\n", m_imgComposeResult->width, m_imgComposeResult->height);

			cvSetImageROI(pImage1,rectPic);

			printf("--src.depth():%d dst: %d", pImage1->depth, m_imgComposeResult->depth);
			printf("--src.size():%d dst: %d", pImage1->imageSize, m_imgComposeResult->imageSize);

			cvResetImageROI(m_imgComposeResult);

			printf("-22-pImage1[%d,%d]---\n", pImage1->width, pImage1->height);
			printf("-22-m_imgComposeResult[%d,%d]---\n", m_imgComposeResult->width, m_imgComposeResult->height);

			cvCopy(pImage1, m_imgComposeResult);
			cvResetImageROI(pImage1);

			printf("111*************\n");

			rect.x = 0;
			rect.y = 0;
			rect.width = m_imgComposeResult->width;
			rect.height = m_imgComposeResult->height - m_nExtentHeight;		

			if(m_nWordPos == 1)
			{
				rect.y += m_nExtentHeight;
				/*
				if(m_nWordOnPic == 0)
				{
					rect.y += m_nExtentHeight/2;
				}
				*/
			}

			rt.x = 0;
			rt.y = 0;
			rt.width = m_imgComposeResult->width;
			rt.height = m_imgComposeResult->height - m_nExtentHeight;
			if(m_nWordPos == 1)
			{
				rt.y += m_nExtentHeight;
			}

			printf("*********RECT:x:%d y:%d width:%d height%d\n",rect.x,rect.y,rect.width,rect.height);
			cvSetImageROI(m_imgComposeResult,rt);
			cvSetImageROI(m_imgComposeMatch,rect);
			cvResize(m_imgComposeResult, m_imgComposeMatch);
			cvResetImageROI(m_imgComposeMatch);
			cvResetImageROI(m_imgComposeResult);

			//特写图
			rect.x = (m_imgComposeResult->width);
			rect.y = 0;
			rect.width = (m_imgComposeResult->width);
			rect.height = (m_imgComposeResult->height - m_nExtentHeight);		

			if(m_nWordPos == 1)//黑边在上
			{
				rect.y += m_nExtentHeight;				
			}
						
			GetCarRect2(record_plate.dspRecord, rt);

#ifdef TEST_MATCH
			//测试在原图上画矩形			
			CvPoint pt1;
			CvPoint pt2;
			pt1.x = rt.x;
			pt1.y = rt.y;

			pt2.x = rt.x + rt.width;
			pt2.y = rt.y + rt.height;

			CvScalar color = CV_RGB(0,0,255);
			cvRectangle(m_imgComposeMatch, pt1, pt2, color);
#endif
			//Test///////////////////////////////

			cvSetImageROI(m_imgComposeResult,rt);
			cvSetImageROI(m_imgComposeMatch,rect);
			cvResize(m_imgComposeResult, m_imgComposeMatch);
			cvResetImageROI(m_imgComposeMatch);
			cvResetImageROI(m_imgComposeResult);
		}
		else if(1 == i)
		{
			printf("222*************\n");
			rectPic.width = pImage2->width;
			rectPic.height = pImage2->height;
			if (rectPic.width == DSP_500_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_500_BIG_HEIGHT_SERVER + 80))
				{
					rectPic.height = DSP_500_BIG_HEIGHT_SERVER + 80;
				}
			}
			else if (rectPic.width == DSP_200_BIG_WIDTH_SERVER)
			{
				if (rectPic.height > (DSP_200_BIG_HEIGHT_SERVER+80))
				{
					rectPic.height = DSP_200_BIG_HEIGHT_SERVER + 80;
				}
			}

			cvSetImageROI(pImage2,rectPic);
			cvCopy(pImage2, m_imgComposeResult);
			cvResetImageROI(pImage2);

			printf("333*************\n");
			/*
			int height = m_imgComposeResult->height;
			if(m_nWordOnPic == 1)
			{
				height -= m_nExtentHeight;
			}
			*/

			rect.x = 0;
			rect.y = m_imgComposeResult->height;

			rect.width = m_imgComposeResult->width;
			rect.height = m_imgComposeResult->height - m_nExtentHeight;		

			if(m_nWordPos == 1)
			{
				rect.y += m_nExtentHeight;
				/*
				if(m_nWordOnPic == 0)
				{
					rect.y += m_nExtentHeight;
				}
				*/
			}

			rt.x = 0;
			rt.y = 0;
			rt.width = m_imgComposeResult->width;
			rt.height = m_imgComposeResult->height - m_nExtentHeight;
			if(m_nWordPos == 1)
			{
				rt.y += m_nExtentHeight;
			}

			printf("*********RECT:x:%d y:%d width:%d height%d\n",rect.x,rect.y,rect.width,rect.height);
			cvSetImageROI(m_imgComposeResult,rt);
			cvSetImageROI(m_imgComposeMatch,rect);
			cvResize(m_imgComposeResult, m_imgComposeMatch);
			cvResetImageROI(m_imgComposeMatch);
			cvResetImageROI(m_imgComposeResult);

			//特写图
			rect.x = (m_imgComposeResult->width);
			rect.y = (m_imgComposeResult->height);
			rect.width = (m_imgComposeResult->width);
			rect.height = (m_imgComposeResult->height - m_nExtentHeight);		

			if(m_nWordPos == 1)
			{
				rect.y += m_nExtentHeight;
				/*
				if(m_nWordOnPic == 0)
				{
					rect.y += m_nExtentHeight/2;
				}
				*/
			}

			/*
			GetCarPostion(foundRecord.dspRecord);
			rt.x = foundRecord.dspRecord.uPosLeft;
			rt.y = foundRecord.dspRecord.uPosTop;
			rt.width = foundRecord.dspRecord.uPosRight - foundRecord.dspRecord.uPosLeft;
			rt.height = foundRecord.dspRecord.uPosBottom - foundRecord.dspRecord.uPosTop;
			*/

			GetCarRect2(foundRecord.dspRecord, rt);

			printf("444*************\n");
			cvSetImageROI(m_imgComposeResult,rt);
			cvSetImageROI(m_imgComposeMatch,rect);
			cvResize(m_imgComposeResult, m_imgComposeMatch);
			cvResetImageROI(m_imgComposeMatch);
			cvResetImageROI(m_imgComposeResult);

			printf("555*************\n");
		}
		else
		{
			//
		}

		//SaveImgTest(m_imgComposeResult);
	}
	return true;
}

//存储前后车牌匹配图像
void CDspDataProcess::SaveVtsImageMatchForTianJin(
	RECORD_PLATE& plate,
	int nPicCount,
	PLATEPOSITION* TimeStamp,
	PLATEPOSITION& SignalTimeStamp,
	int64_t redLightStartTime)
{
	LogTrace("VtsPath.txt", "SaveVtsImageMatchForTianJin strPath:%s\n", plate.chPicPath);
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
	//大图宽高
	plate.uPicWidth = m_imgComposeMatch->width;
	plate.uPicHeight = m_imgComposeMatch->height;
					
	for(int nIndex = 0; nIndex <nPicCount ; nIndex++)
	{
		PutVtsTextMatchForTianJin(m_imgComposeMatch,plate,nIndex,TimeStamp,&SignalTimeStamp,redLightStartTime, plate.uViolationType);
	}

	if((nPicCount == 1)&&(g_nDetectMode == 2))
	{
		IplImage* pImage = cvCreateImageHeader(cvSize(m_imgComposeMatch->width, m_imgComposeMatch->height/2), 8, 3);
		cvSetData(pImage,m_imgComposeMatch->imageData,pImage->widthStep);
		plate.uPicSize = SaveImage(pImage,strPicPath,2);
		cvReleaseImageHeader(&pImage);
	}
	else
	{
		//SaveImgTest(m_imgComposeMatch, plate.chText);
		plate.uPicSize = SaveImage(m_imgComposeMatch,strPicPath,2);
	}
}

//电警叠加文本信息
void CDspDataProcess::PutVtsTextMatchForTianJin(
	IplImage*pImage,
	RECORD_PLATE plate,
	int nIndex,
	PLATEPOSITION* pTimeStamp,
	PLATEPOSITION* pSignalTimeStamp,
	int64_t redLightStartTime, 
	UINT32 uViolationType)
{
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);
	int timeIndex = nIndex;
    if(pTimeStamp != NULL)
    {
        wchar_t wchOut[255] = {'\0'};
        char chOut[255] = {'\0'};

        int nWidth = 10;
        int nHeight = m_imgSnap->height - m_nExtentHeight;

		if(m_nWordPos == 1)//黑边在上
		{
			nHeight = 0;
		}
	
		//LogNormal("*********seq:%d\n",plate.uSeq);

		string strText("");
        if(m_nWordOnPic == 1)//字直接叠加在图上
        {
        }
        else
        {
			//LogTrace("PutVtsText.log", "222=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d=m_nWordPos=%d", \
				g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode, m_nWordPos);
			string strTmp = "";

			//经过时间
			std::string strTime;
			strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,1);
			sprintf(chOut,"时间:%s%03d毫秒 ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);
			strTmp = chOut;
			strText += strTmp;

			//路口名称	
			sprintf(chOut,"路口名称:[%s] ",plate.chPlace);
			strTmp = chOut;
			strText += strTmp;

			//方向
			std::string strDirection = GetDirection(plate.uDirection);
			sprintf(chOut,"方向:[%s] ",strDirection.c_str());
			strTmp = chOut;
			strText += strTmp;

			/*
			//车牌号码
			if(g_PicFormatInfo.nCarNum == 1)
			{
				std::string strCarNum(plate.chText);
				sprintf(chOut,"车牌号码:%s  ",strCarNum.c_str());
				strTmp = chOut;
				strText += strTmp;
			}
			*/

			//车速
			if(g_PicFormatInfo.nCarSpeed == 1 && 1 != g_PicFormatInfo.nSmallViolationPic)
			{
				//行驶速度
				sprintf(chOut,"速度:%dkm/h  ",plate.uSpeed);
				strTmp = chOut;
				strText += strTmp;
			}

			//设备编号
			sprintf(chOut,"设备编号:%s ",g_strDetectorID.c_str());
			strTmp = chOut;
			strText += strTmp;

			//防伪码
			sprintf(chOut,"防伪码：%08x", m_nRandCode[0]);
			strTmp = chOut;
			strText += strTmp;

			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());

			if(0 == nIndex)
			{
				nWidth = 10;
				nHeight += m_nExtentHeight/2;
			}
			else if(1 == nIndex)
			{
				nWidth = 10;
				nHeight += m_imgSnap->height;
				nHeight += m_nExtentHeight/2;
			}
			else{}

			printf("--11-PutVtsTextMatchForTianJin m_cvText.putText nWidth:%d nHeight:%d pImage[%d,%d]\n", \
				nWidth, nHeight, pImage->width, pImage->height);

			try{
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
			}				
			catch(...)
			{
				printf("catch m_cvText.putText err!");
			}
			
        }
    }
    else //空白区域
    {
		//LogTrace("PutVtsText.log", "333=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d", \
			g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode);		
    }

	m_cvText.UnInit();
}

//核查plate 是否合法
bool CDspDataProcess::CheckPlateRect(const RECORD_PLATE &plate, CvRect &rtSrc)
{
	bool bRet = true;

	if(plate.uPosLeft < 0 || plate.uPosTop < 0 || plate.uPosRight < 10 || plate.uPosBottom < 10 ||
		((plate.uPosRight - plate.uPosLeft) < 10) || ((plate.uPosBottom - plate.uPosTop) < 10) )
	{
		bRet = false;
	}

	int nWidth = 120;
	int nHeight = 35;
	rtSrc.x = (plate.uPosLeft + plate.uPosRight)/2 - nWidth/2;
	rtSrc.y = (plate.uPosTop + plate.uPosBottom)/2 - nHeight/2;

	if(rtSrc.x < 0)
	{
		 rtSrc.x = 0;
	}
	if(rtSrc.y < 0)
	{
		rtSrc.y = 0;
	}

	rtSrc.width = nWidth;
	rtSrc.height = nHeight;

	return bRet;
}

//获取车身位置
 void CDspDataProcess::GetCarRect(const RECORD_PLATE &plate, CvRect &rtCar)
{
	//LogNormal("11 rtCar:%d,%d,%d,%d ", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
	int nWidth = m_imgSnap->width;
	int nHeight = m_imgSnap->height - m_nExtentHeight;

	int nHeightC1 = 30;//参考固定高, eg:20(车牌高度常数)	//20
	int nWidthC1 = 200;//参考固定高, eg:100(车牌高度常数)
	int nWidthExtC1 = 1000;//参考外扩区域宽, eg:800			//160 600 400
	int nHeightExtC2 = (int)((nWidthExtC1 * nHeight) / nWidth);//参考外扩区域高, eg:600			//40 480 320
	int nHeightExtC3 = (int)(nHeightExtC2 * 0.7f);//参考外扩区域,抠图区域高, eg:400 //20 320 220		

	CvRect rtSrc;
	//核查plate 是否合法
	bool bCheckPlate = CheckPlateRect(plate, rtSrc);

	if(!bCheckPlate)//对于不合法车牌位置,固定为下1/4,居中
	{
		rtSrc.x = (int)(nWidth * 0.5 - nWidthC1 * 0.5);
		rtSrc.y = (int)(nHeight * 0.75 - nHeightC1 * 0.5);
		rtSrc.width = nWidthC1;
		rtSrc.height = nHeightC1;
		LogNormal("CheckPlate rtSrc:%d,%d,%d,%d ", rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height);
	}

	//LogNormal("Plate rtSrc:%d,%d,%d,%d ", rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height);

	CvRect rtSrcImg;

	rtSrcImg.x = 0;
	rtSrcImg.y = 0;
	rtSrcImg.width = m_imgSnap->width;
	rtSrcImg.height = m_imgSnap->height - m_nExtentHeight;
	if(m_nWordPos == 1)//黑边在上
	{
		rtSrcImg.y += m_nExtentHeight;
	}

	GetCarPosRect(nHeightC1,nWidthExtC1,nHeightExtC2,nHeightExtC3,rtSrc,rtSrcImg,rtCar);

	/////////////////////////////////////////////
	if(rtCar.x+rtCar.width >= nWidth)
	{
		rtCar.x = nWidth - rtCar.width-1;
	}

	if(m_nWordPos == 1)//黑边在上
	{
		rtCar.y += m_nExtentHeight;
	}

	if(rtCar.y+rtCar.height>=nHeight)
	{
		rtCar.y = nHeight - rtCar.height-1;	
	}

//#ifdef TEST_MATCH
	//LogNormal("m_nWordPos:%d rtCar.y:%d ", m_nWordPos, rtCar.y);
	//LogNormal("plate:%d,%d,%d,%d ", plate.uPosLeft, plate.uPosRight, plate.uPosTop, plate.uPosBottom);
	//LogNormal("rtCar:%d,%d,%d,%d ", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
//#endif

}

 //获取车身位置,取图片下1/4位置.
 void CDspDataProcess::GetCarRect2(const RECORD_PLATE &plate, CvRect &rtCar)
 {
	 //LogNormal("11 rtCar:%d,%d,%d,%d ", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
	 int nWidth = m_imgSnap->width;
	 int nHeight = m_imgSnap->height - m_nExtentHeight;	

	 CvRect rtSrc;
	 //核查plate 是否合法
	 bool bCheckPlate = CheckPlateRect(plate, rtSrc);

	 if(!bCheckPlate)//对于不合法车牌位置,固定为下1/4,居中
	 {		 
		 LogNormal("CheckPlate rtSrc:%d,%d,%d,%d ", rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height);
	 }

	 //LogNormal("Plate rtSrc:%d,%d,%d,%d ", rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height);

	 CvRect rtSrcImg;

	 rtSrcImg.x = 0;
	 rtSrcImg.y = 0;
	 rtSrcImg.width = m_imgSnap->width;
	 rtSrcImg.height = m_imgSnap->height - m_nExtentHeight;
	 if(m_nWordPos == 1)//黑边在上
	 {
		 rtSrcImg.y += m_nExtentHeight;
	 }

	 //GetCarPosRect(nHeightC1,nWidthExtC1,nHeightExtC2,nHeightExtC3,rtSrc,rtSrcImg,rtCar);
	 int fDstWidth = rtSrcImg.width * 0.5f;
	 float fTempLeft = rtSrc.x - (fDstWidth - rtSrc.width) * 0.5f;
	 float fTempTop = rtSrcImg.height * 0.5f;
	 float fTempRight = rtSrc.x + fDstWidth;
	 float fTempBottom = rtSrcImg.height;

	 if(fTempLeft < 0)
	 {
		 fTempLeft = 0;
		 fTempRight = fDstWidth;
	 }
	 else if(fTempRight > rtSrcImg.width)
	 {
		 fTempRight = rtSrcImg.width;
		 fTempLeft = fTempRight - fDstWidth;
	 }
	 else{}

	 rtCar.x = (int)(fTempLeft + 0.5f);
	 rtCar.y = (int)(fTempTop + 0.5f);
	 rtCar.width = (int)(fDstWidth + 0.5f);
	 rtCar.height = (int)(rtSrcImg.height * 0.5f + 0.5f);

	 /////////////////////////////////////////////
	 if(rtCar.x+rtCar.width >= nWidth)
	 {
		 rtCar.x = nWidth - rtCar.width-1;
	 }	

	 if(rtCar.y+rtCar.height>=nHeight)
	 {
		 rtCar.y = nHeight - rtCar.height-1;	
	 }

	 if(m_nWordPos == 1)//黑边在上
	 {
		 rtCar.y += m_nExtentHeight;
	 }

#ifdef MATCH_LIU_YANG_DEBUG
	 //LogNormal("m_nWordPos:%d rtCar.y:%d ", m_nWordPos, rtCar.y);
	 LogNormal("plate:%d,%d,%d,%d ", plate.uPosLeft, plate.uPosRight, plate.uPosTop, plate.uPosBottom);
	 LogNormal("rtCar:%d,%d,%d,%d ", rtCar.x, rtCar.y, rtCar.width, rtCar.height);
#endif
 }

 #ifdef TWICE_DETECT
 //初始化二次识别
 bool CDspDataProcess::InitBusVioFilter()
 {
	bool bRet = false;
	//初始化-载入xml文件
	bRet = m_OnBusVioFilter.mvOnBusVioValidInit("./BusPlate.xml");

	return bRet;
 }
#endif

#ifdef TWICE_DETECT
//将检测到的车牌位置画出来
void CDspDataProcess::DrawVioFilter(const RECORD_PLATE &plate)
{	
	vector<CvRect>::iterator it;
	for(it=m_vecRet1.begin(); it!=m_vecRet1.end(); it++)
	{
		CvRect rt;
		rt.x = it->x;
		rt.y = it->y;
		rt.width = it->width;
		rt.height = it->height;
		myRectangle(m_imgVtsElem1, rt, CV_RGB(255, 0, 255), 5);
	}

	for(it=m_vecRet2.begin(); it!=m_vecRet2.end(); it++)
	{
		CvRect rt;
		rt.x = it->x;
		rt.y = it->y;
		rt.width = it->width;
		rt.height = it->height;
		myRectangle(m_imgVtsElem2, rt, CV_RGB(255, 0, 255), 5);
	}

	for(it=m_vecRet3.begin(); it!=m_vecRet3.end(); it++)
	{
		CvRect rt;
		rt.x = it->x;
		rt.y = it->y;
		rt.width = it->width;
		rt.height = it->height;
		myRectangle(m_imgVtsElem3, rt, CV_RGB(255, 0, 255),5);
	}
	
	char jpg_name[256] = {0};
	sprintf(jpg_name, "TD_%d_%d_%u-%s-seq-1", m_nChannelId, m_imgVtsElem1->imageSize, GetTimeStamp(), plate.chText);
	SaveImgTest(m_imgVtsElem1,jpg_name);	
	sprintf(jpg_name, "TD_%d_%d_%u-%s-seq-2", m_nChannelId, m_imgVtsElem2->imageSize, GetTimeStamp(), plate.chText);
	SaveImgTest(m_imgVtsElem2,jpg_name);	
	sprintf(jpg_name, "TD_%d_%d_%u-%s-seq-3", m_nChannelId, m_imgVtsElem3->imageSize, GetTimeStamp(), plate.chText);
	SaveImgTest(m_imgVtsElem3,jpg_name);
}
#endif

#ifdef TWICE_DETECT
//在图像上,画矩形
void CDspDataProcess::myRectangle(IplImage *pImg, const CvRect &rt, const CvScalar &color, const int &thickness)
{
	CvPoint pt1;
	CvPoint pt2;
	pt1.x = rt.x;
	pt1.y = rt.y;

	pt2.x = rt.x + rt.width;
	pt2.y = rt.y + rt.height;

	cvRectangle(pImg, pt1, pt2, color, thickness);
}
#endif

//输出车牌前后匹配合成图-浏阳版本
void CDspDataProcess::OutPutVtsMatch2x3(
	RECORD_PLATE_DSP_MATCH &record_plate, 
	RECORD_PLATE_DSP_MATCH &foundRecord, 
	RECORD_PLATE& plate)
{
	//LogTrace("FBMach.log", "OutPutVts video1:%s ", record_plate.dspRecord.chVideoPath);
	//LogTrace("FBMach.log","OutPutVts video2:%s ", foundRecord.dspRecord.chVideoPath);
	//LogTrace("FBMach.log","OutPutVts plate:%s ", plate.chVideoPath);
	//LogTrace("FBMach.log","-OutPutVtsMatch-[%s]-[%s]\n", record_plate.dspRecord.chText, foundRecord.dspRecord.chText);

	//LogNormal("OutPutVtsMatch2x3 car:%s ", record_plate.dspRecord.chText);

	/*
	//just test
	char chPicName[255] = {0};
	memset(chPicName, 0, 255);
	sprintf(chPicName, "A-%s.jpg",record_plate.dspRecord.chText);
	SaveImgTest(record_plate.pImgArray[0],chPicName);

	memset(chPicName, 0, 255);
	sprintf(chPicName, "B-%s.jpg",foundRecord.dspRecord.chText);
	SaveImgTest(foundRecord.pImg,chPicName);
	//end test
	*/
	UINT32 uWidth = record_plate.dspRecord.uPicWidth;
	UINT32 uHeight = record_plate.dspRecord.uPicHeight;

	if (m_imgComposeMatch == NULL)
	{
		LogNormal("DspDataProcess uWidth=%d,uHeight=%d\n",uWidth,uHeight);
		Init(record_plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
	}
#ifdef MATCH_LIU_YANG_DEBUG
	bool bDeal = OutPutVtsMatchFilter(record_plate);

	if(!bDeal)
	{
		return;
	}	
#endif

//#ifdef MATCH_LIU_YANG
	//解码图片
	bool bSetA = SetVtsImgMatch(record_plate, true);
	bool bSetB = SetVtsImgMatch(foundRecord, false);

	//LogNormal("pImgA:%x pImgB:%x OutPutVtsMatch2x3", record_plate.pImgArray[1], foundRecord.pImg);

	if(!bSetA || !bSetB)
	{
		LogNormal("2*3 SetVtsImgMatch:bSetA:%d , bSetB:%d err!",bSetA, bSetB);
		return;
	}
//#endif

//红绿灯增强
#ifdef REDADJUST
	RedLightAdjustMatch(record_plate);
#endif

	ViolationInfo infoViolation;

	//PLATEPOSITION* pTimeStamp;
	PLATEPOSITION TimeStamp[4];
	SetVtsInfo(record_plate, foundRecord, plate, infoViolation, TimeStamp);
	bool bIsReSize = true;
	//合成违章图片
	bool bSetVtsImg = SetVtsImage2x3(record_plate, foundRecord,bIsReSize);

	if(!bSetVtsImg)
	{
		LogNormal("OutPutVtsMatch2x3 error!");
		return;
	}

	//获取录像以及图片路径
	pthread_mutex_lock(&g_Id_Mutex);
	//获取图片路径
	std::string strPicPath;
	int nSaveRet = GetVtsPicSavePath(plate, 2,strPicPath);
	pthread_mutex_unlock(&g_Id_Mutex);

	//压缩6合一图片
	IplImage* pSrcImg = m_imgComposeMatch;
	IplImage* pImgResize = NULL;
	if(g_PicFormatInfo.nResizeScale < 100 && g_PicFormatInfo.nResizeScale > 0)
	{
			pImgResize = cvCreateImage(cvSize(m_imgComposeMatch->width*g_PicFormatInfo.nResizeScale/100.0,m_imgComposeMatch->height*g_PicFormatInfo.nResizeScale/100.0),8,3);
			cvResize(m_imgComposeMatch,pImgResize);
			pSrcImg = pImgResize;
	}

	//合成违章叠字
	SetVtsText2x3(pSrcImg, plate, infoViolation, TimeStamp, record_plate, foundRecord,bIsReSize);//TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime);
	//存储违章图像
	SaveComposeVtsImageMatch(plate,pSrcImg);

	if(pImgResize != NULL)
	{
		cvReleaseImage(&pImgResize);
		pImgResize = NULL;
	}

	//删除已经存在的记录
	g_skpDB.DeleteOldRecord(strPicPath,false,false);


	//保存记录
	if(nSaveRet>0)
	{
		//LogNormal("Color:[%d:%d]",plate.uCarColor1,plate.uCarColor2);
		//LogNormal("--record_plate.[%s][%s]--tt[%s]-\n", \
		record_plate.dspRecord.chText, foundRecord.dspRecord.chText, plate.chText);
		//LogNormal("video:%s ", plate.chVideoPath);
		//LogNormal("pic:%s car:%s", plate.chPicPath, plate.chText);

		//LogTrace("FBMach.log","video:%s ", plate.chVideoPath);
		//g_skpDB.SavePlate(m_nChannelId,plate,0);
		g_skpDB.SavePlate(record_plate.dspRecord.uChannelID,plate,0);
	}

	//将车牌信息送客户端
	//车牌号码
	//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	//LogNormal("--SendResult-plate.uChannelID=%d\n", plate.uChannelID);
	SendResult(plate,infoViolation.carInfo.uSeq);
}

//设置车牌记录
void CDspDataProcess::SetCarInfo(const RECORD_PLATE_DSP_MATCH &record_plate, CarInfo  &carnums)
{
	//CarInfo    carnums;
	char text[10] = "123456789";
	if(record_plate.dspRecord.chText[0] != 'L') //非武警牌照
	{
		memset(text, 0, 10);
		memcpy(text, record_plate.dspRecord.chText, 7);
		memcpy(carnums.strCarNum, text, 7);
	}
	else
	{
		memset(text, 0, 10);
		memcpy(text, record_plate.dspRecord.chText, 9);
		memcpy(carnums.strCarNum, text, 7);
		memcpy(carnums.wj, text+7, 2); //武警牌下面的小数字
	}

	carnums.color = record_plate.dspRecord.uColor;
	carnums.vehicle_type = record_plate.dspRecord.uType;
	carnums.subVehicleType = record_plate.dspRecord.uTypeDetail;
	carnums.ix = (record_plate.dspRecord.uPosLeft);
	carnums.iy = (record_plate.dspRecord.uPosTop);
	carnums.iwidth = (record_plate.dspRecord.uPosRight - record_plate.dspRecord.uPosLeft);
	carnums.iheight = (record_plate.dspRecord.uPosBottom - record_plate.dspRecord.uPosTop);
	carnums.mean = -1;
	carnums.stddev = -1;

	carnums.uTimestamp = record_plate.dspRecord.uTime;//(pHeader->ts)/1000/1000;
	carnums.ts = record_plate.dspRecord.uTime * 1000 * 1000 + record_plate.dspRecord.uMiTime * 1000;
	carnums.uSeq = record_plate.dspRecord.uSeqID;
	carnums.carnumrow = record_plate.dspRecord.uType;//?
	carnums.wx = 0; //(double)((pPlate->uLongitude) * 0.0001);
	carnums.wy = 0; //(double)((pPlate->uLatitude) * 0.0001);
	carnums.id = 0; //pHeader->uCameraId;
	carnums.vx = 0;
	carnums.vy = record_plate.dspRecord.uSpeed; //传入dsp相机线圈检测出速度
	carnums.RoadIndex = record_plate.dspRecord.uRoadWayID;
	carnums.nDirection = record_plate.dspRecord.uDirection;
}

//合成违章记录
void CDspDataProcess::SetVtsInfo(
	const RECORD_PLATE_DSP_MATCH &record_plate, 
	const RECORD_PLATE_DSP_MATCH &foundRecord, 
	RECORD_PLATE& plate,
	ViolationInfo &infoViolation,
	PLATEPOSITION* pTimeStamp)
{
	CarInfo    carnums;
	SetCarInfo(record_plate, carnums);
	
	//电警数据
	{
		memcpy(&infoViolation.carInfo,&carnums,sizeof(CarInfo));
		infoViolation.evtType = (VIO_EVENT_TYPE)record_plate.dspRecord.uViolationType;//违章类型
		infoViolation.nChannel = record_plate.dspRecord.uRoadWayID;//车道编号
		infoViolation.nPicCount = 2;//图片数量
	}

	memcpy((char*)(&plate),(char*)(&record_plate.dspRecord),sizeof(RECORD_PLATE));
	plate.uCarColor1 = record_plate.dspRecord.uCarColor1;
	plate.uCarColor2 = foundRecord.dspRecord.uCarColor1;

	//违章类型-TODO
	//plate.uViolationType = DETECT_MATCH_PLATE; //报前后牌匹配
	plate.uViolationType = infoViolation.evtType;//

	std::string strCarNum = infoViolation.carInfo.strCarNum;
	
#ifdef MATCH_LIU_YANG_DEBUG
	strCarNum = foundRecord.dspRecord.chText;
#endif

	//判断是否有车牌的车
	bool bCarNum = true;
	if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
	{
		bCarNum = false;
	}

	if(bCarNum)
	{
		//车牌号码转换
		CarNumConvert(strCarNum,infoViolation.carInfo.wj);
	}

	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	int nPos = -1;
	string nStrNum = plate.chText;
	nPos = nStrNum.find("*");

	//车辆类型
	plate.uType =  infoViolation.carInfo.vehicle_type;
	//LogNormal("SetVtsInfo 11 uType:%d ", plate.uType);
	CarTypeConvert(plate);
	//LogNormal("SetVtsInfo 22 uType:%d ", plate.uType);

	//车型细分
	if(m_nDetectKind&DETECT_TRUCK)
		plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);

	//车辆类型转换
	CarTypeConvert(plate);

	int nPicCount = infoViolation.nPicCount;//图片数量

	if(nPicCount > 2)
	{
		nPicCount = 2;
	}

	if(1 == g_PicFormatInfo.nSmallViolationPic)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0];
		infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
		infoViolation.frameSeqs[1] = frameSeqs;
	}
	plate.uSignalTime = record_plate.dspRecord.uSignalTime;
	//PLATEPOSITION  TimeStamp[6];
	PLATEPOSITION  SignalTimeStamp;

//#ifdef MATCH_LIU_YANG
	for(int i=0; i<3; i++)
	{
		pTimeStamp[i].ts = record_plate.pPlatePos[i+1].ts;
		//LogNormal("ts:%d, %lld", i, pTimeStamp[i].ts);
	}

	pTimeStamp[3].ts = foundRecord.pPlatePos[0].ts;
	//LogNormal("ts:3, %lld", pTimeStamp[3].ts);
//#endif

	if(nPicCount>=1)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];
		SignalTimeStamp.x = infoViolation.carInfo.ix;
		SignalTimeStamp.y = infoViolation.carInfo.iy;
		SignalTimeStamp.width = infoViolation.carInfo.iwidth;
		SignalTimeStamp.height = infoViolation.carInfo.iheight;
		SignalTimeStamp.nType = plate.uType;
		SignalTimeStamp.IsCarnum = bCarNum;

		bool bRet = false;
		//获取违章检测结果图像
		//车牌世界坐标
		plate.uLongitude = (UINT32)(infoViolation.carInfo.wx*10000*100);
		plate.uLatitude = (UINT32)(infoViolation.carInfo.wy*10000*100);
		//相机ID
		plate.uChannelID = infoViolation.carInfo.id;

		//发生位置(在当前图片上的)
		plate.uPosLeft  = infoViolation.carInfo.ix;
		plate.uPosTop   = infoViolation.carInfo.iy;
		plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
		plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

		//LogNormal(":%s ,%d,%d,%d,%d record_plate", \
		//	record_plate.dspRecord.chText, record_plate.dspRecord.uPosLeft, record_plate.dspRecord.uPosTop, record_plate.dspRecord.uPosRight, record_plate.dspRecord.uPosBottom);
		//LogNormal(":%s ,%d,%d,%d,%d foundRecord", \
		//	foundRecord.dspRecord.chText, foundRecord.dspRecord.uPosLeft, foundRecord.dspRecord.uPosTop, foundRecord.dspRecord.uPosRight, foundRecord.dspRecord.uPosBottom);

		Picture_Key Pic_Key;
		Pic_Key.uSeq = infoViolation.carInfo.uSeq;
		Pic_Key.uCameraId = infoViolation.carInfo.id;	

		//printf("-----1111111---GetVtsImageMatch----\n");
		//SaveImgTest(record_plate.pImg);		
		//printf("-----22222222---GetVtsImageMatch----\n");
	}

	//经过时间(秒)
	plate.uTime = infoViolation.carInfo.uTimestamp;
	//毫秒
	plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;

	//地点
	//memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
	/*
	//行驶方向
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
	plate.uDirection = (m_nDirection % 4 + 1);
	*/

	//LogNormal("m_nDirection:%d", m_nDirection);
	//LogNormal("plate.uDirection:%d ", plate.uDirection);
	//LogNormal("infoViolation.carInfo.nDirection:%d", infoViolation.carInfo.nDirection);
	//	LogNormal("---plate.uDirection=%d=m_nDirection=%d\n", plate.uDirection,m_nDirection);

	//车牌结构
	plate.uPlateType = infoViolation.carInfo.carnumrow;
	//车牌颜色
	plate.uColor = infoViolation.carInfo.color;
	//车道编号
	plate.uRoadWayID = infoViolation.nChannel;
	//车速
	double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
	plate.uSpeed = (UINT32)(dSpeed+0.5);
	//发生位置(在当前图片上的)
	plate.uPosLeft  = infoViolation.carInfo.ix;
	plate.uPosTop   = infoViolation.carInfo.iy;
	plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
	plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);
	plate.uPicWidth = m_imgComposeMatch->width;
	plate.uPicHeight = m_imgComposeMatch->height;
}


//合成违章叠字
void CDspDataProcess::SetVtsText2x3(
	IplImage * pImage,
	RECORD_PLATE  &plate,
	const ViolationInfo &vioInfo,
	PLATEPOSITION* pTimeStamp,
	const RECORD_PLATE_DSP_MATCH &record_plate,
	const RECORD_PLATE_DSP_MATCH &foundRecord,
	const bool bIsReSize)
{
	if(pImage == NULL)
	{
		return;
	}

	int nRow = 3;
	int nCol = 2;

	int nWidth = 0;
	int nHeight = 0;

	char chOut[255] = {'\0'};
	wchar_t wchOut[255] = {'\0'};	

	std::string strDeviceId = "";
	std::string strPlace = "";
	int64_t ts = 0;
	for(int i=0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			//叠字位置
			//GetTextPos(nWidth, nHeight, i, j);

			strDeviceId = std::string(record_plate.szDeviceID);
			strPlace = std::string(record_plate.dspRecord.chPlace);

			ts = 0;
			//取得时间戳
			if(i==0 && j==0)
			{
				ts = pTimeStamp[0].ts;
			}			
			else if(i==0 && j==1)
			{
				ts = pTimeStamp[0].ts;
				//ts = pTimeStamp[1].ts;
			}
			else if(i==1 && j==0)
			{
				ts = pTimeStamp[1].ts;
			}
			else if(i==1 && j==1)
			{
				ts = pTimeStamp[2].ts;
			}
			else if((i==2 && j==0) || (i==2 && j==1))
			{
				ts = pTimeStamp[3].ts;
				strDeviceId = std::string(foundRecord.szDeviceID);
				strPlace = std::string(foundRecord.dspRecord.chPlace);
				plate.uDirection = foundRecord.dspRecord.uDirection;
			}
			else{}

			//叠字内容
			std::string strText = GetVtsText(pImage,plate, vioInfo, i, j, ts, strDeviceId, strPlace,bIsReSize);
		}

		if (g_nServerType == 13)
		{
			std::string strText = "";
			int nFontSize = m_nExtentHeight/2;
			CvxText cvText;
			cvText.Init(nFontSize);

			//经过时间
			std::string strTime;
			ts = pTimeStamp[0].ts;
			UINT32 uTime = (UINT32)((ts/1000)/1000);
			strTime = GetTime(uTime,0);

			nWidth = 0;
			nHeight = (m_imgSnap->height)*(i+1)- m_nExtentHeight/2;
			
			if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
			{
				sprintf(chOut,"%s 车道%5s%02d %s 红灯时间:%d秒",strPlace.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uSignalTime);//
			}
			else
			{
				sprintf(chOut,"%s 车道%5s%02d %s",strPlace.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str());
			}
			strText += chOut;
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			//叠加第二行违章代码
			nHeight += m_nExtentHeight/2;
			strText = GetViolationType(plate.uViolationType,1);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			cvText.UnInit();
		}
	}

	return;
}

//获取叠字内容
std::string CDspDataProcess::GetVtsText(IplImage * pImage,
	const RECORD_PLATE  &plate, 
	const ViolationInfo &vioInfo, 
	const int nRow, 
	const int nCol,
	const int64_t ts,
	const std::string &strDeviceId,
	const std::string &strPlace,
	const bool bIsReSize)
{
	std::string strText = "";

	char chOut[255] = {'\0'};
	int nWidth = 0;
	int nHeight = 0;
	wchar_t wchOut[255] = {'\0'};	

	unsigned int uExtentHeight = 0;
	//文本初始化
	int nFontSize = g_PicFormatInfo.nFontSize;
	if (bIsReSize)
	{
		if (g_PicFormatInfo.nResizeScale < 100  && g_PicFormatInfo.nResizeScale >= 60)
		{
			nFontSize = g_PicFormatInfo.nFontSize * 4 / 5;
		}
		else if (g_PicFormatInfo.nResizeScale < 60  && g_PicFormatInfo.nResizeScale >= 5)
		{
			nFontSize = g_PicFormatInfo.nFontSize * 3 /5;
		}
		uExtentHeight = nFontSize * 2 + 10;   //文本扩展区域高度按照字体大小的 2倍加10 计算
	}

	if (g_nServerType == 13)
	{
		if( !((nRow == 0 || nRow == 2)&&(nCol == 0)))
		{
			nFontSize = m_nExtentHeight/2;
			CvxText cvText;
			cvText.Init(nFontSize);

			nWidth += (m_imgSnap->width)*nCol;
			nHeight = (m_imgSnap->height)*nRow + m_nExtentHeight;

			std::string strDirection = GetDirection(plate.uDirection);
			sprintf(chOut,"抓拍方向:%s",strDirection.c_str());//

			strText += chOut;
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());

			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));

			//抓拍时间
			UINT32 uTime = (UINT32)((ts/1000)/1000);
			UINT32 uMiTime = ((UINT32)(ts/1000)%1000);
			std::string strTime = GetTime(uTime);
			memset(chOut,0,sizeof(chOut));
			sprintf(chOut,"%s.%03d",strTime.c_str(), uMiTime);
			
			nHeight += (m_nExtentHeight/2);
			strText.clear();
			strText += chOut;
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());

			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
			
					
			cvText.UnInit();
		}
		return strText;
	}
	CvxText cvText;
	cvText.Init(nFontSize);
	
	GetTextPos(nWidth, nHeight, nRow, nCol,bIsReSize,uExtentHeight);
	//设备编号
	memset(chOut,0,sizeof(chOut));
	sprintf(chOut,"设备编号: %s ",strDeviceId.c_str());
	std::string strTemp0(chOut);
	strText += strTemp0;

	//地点
	memset(chOut,0,sizeof(chOut));
	sprintf(chOut,"地点名称: %s ", strPlace.c_str());//m_strLocation.c_str());
	std::string strTemp2(chOut);
	strText += strTemp2;

	//行驶方向
	memset(chOut,0,sizeof(chOut));
	std::string strDirection = GetDirection(plate.uDirection);
	sprintf(chOut,"方向: %s ",strDirection.c_str());
	std::string strTemp3(chOut);
	strText += strTemp3;

	//车道编号
	if(g_PicFormatInfo.nRoadIndex == 1)
	{
		memset(chOut,0,sizeof(chOut));
		sprintf(chOut,"车道名称:%d  ",plate.uRoadWayID);//
		std::string strTemp5(chOut);
		strText += strTemp5;
	}
	//车牌号码
	if(g_PicFormatInfo.nCarNum == 1)
	{
		std::string strCarNum(plate.chText);
		memset(chOut,0,sizeof(chOut));
		sprintf(chOut,"车牌号码: %s ",strCarNum.c_str());
		std::string strTemp4(chOut);
		strText += strTemp4;
	}
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,(char*)strText.c_str());
	cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	
	//第二行
	if (bIsReSize)
	{
		nHeight = nHeight + uExtentHeight / 2 -1;
	}
	else
	{
		nHeight = nHeight + m_nExtentHeight/2;
	}
	strText = "";

	//memset(wchOut,0,sizeof(wchOut));
	//UTF8ToUnicode(wchOut,(char*)strText.c_str());
	//cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	
	//抓拍时间
	UINT32 uTime = (UINT32)((ts/1000)/1000);
	UINT32 uMiTime = ((UINT32)(ts/1000)%1000);
	std::string strTime = GetTime(uTime);
	memset(chOut,0,sizeof(chOut));
	sprintf(chOut,"抓拍时间: %s:%03d ",strTime.c_str(), uMiTime);
	std::string strTemp1(chOut);
	strText += strTemp1;
	
	/*
	//红灯时间
	//if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
	{
		UINT32 uTimestamp = (vioInfo.redLightStartTime/1000)/1000;
		if(vioInfo.redLightStartTime <= 0)
		{
			if(pSignalTimeStamp != NULL)
			{
				uTimestamp = pSignalTimeStamp->uTimestamp;
			}
		}
		strTime = GetTime(uTimestamp,0);
		sprintf(chOut,"红灯时间: %s:%03d",strTime.c_str(),((vioInfo.redLightStartTime)/1000)%1000);
		strTemp6
	}
	*/

	//违法行为
	if(g_PicFormatInfo.nViolationType == 1)
    {
        //违章行为
        if(plate.uViolationType != 0)
        {
			if(nCol < 3)
			{
				//int nId = nRow*2 + nCol + 1;

				UINT32 uVtsCode;
				std::string strViolationType = "";
				bool bGet = GetVtsCode(plate.uViolationType, uVtsCode, strViolationType);
				if(bGet)
				{
					sprintf(chOut,"违法代码:%d ", uVtsCode);
					std::string strTemp9(chOut);
					strText += strTemp9;
				}
				else				
				{
					strViolationType = GetViolationType(plate.uViolationType,1);					
				}
				memset(chOut,0,sizeof(chOut));
				sprintf(chOut,"违法行为:%s ",strViolationType.c_str());
				std::string strTemp7(chOut);
				strText += strTemp7;
			}
        }
    }

	//防伪码
	m_nRandCode[0] = g_RoadImcData.GetRandCode();
	//m_nRandCode[1] = g_RoadImcData.GetRandCode();
	//m_nRandCode[2] = g_RoadImcData.GetRandCode();
	memset(chOut,0,sizeof(chOut));
	//sprintf(chOut,"防伪码：%08x %08x %08x", m_nRandCode[0], m_nRandCode[1], m_nRandCode[2]);
	sprintf(chOut,"防伪码：%08x ", m_nRandCode[0]);
	std::string strTemp8(chOut);
	strText += strTemp8;

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,(char*)strText.c_str());
	cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

	cvText.UnInit();

	return strText;
}

//获取叠字位置
void CDspDataProcess::GetTextPos(int &nWidth, int &nHeight, const int nRow, const int nCol,const bool bIsReSize,unsigned int uExtentHeight)
{
	int nPicW  = 0;
	int nPicH  = 0;
	if (bIsReSize)//是否压缩过的
	{
		nPicW = m_imgComposeResult->width * g_PicFormatInfo.nResizeScale/100.0;
		nPicH = m_imgComposeResult->height * g_PicFormatInfo.nResizeScale/100.0;
	}
	else
	{
		nPicW = m_imgComposeResult->width;
		nPicH = m_imgComposeResult->height;
	}
	
	nWidth = (nPicW * nCol) + 10;
	nHeight = nPicH * nRow;

	if(m_nWordOnPic == 1)//字直接叠加在图上
	{
		//
	}
	else //黑边上
	{
		if(m_nWordPos == 1)//黑边在上
		{
			if (bIsReSize && uExtentHeight != 0)//是否压缩过的
			{
				nHeight = nHeight + (uExtentHeight / 2 );
			}
			else
			{
				nHeight = nHeight + (m_nExtentHeight/2);
			}
			
		}
		else
		{
			if (bIsReSize && uExtentHeight != 0)//是否压缩过的
			{
				nHeight += nPicH;
				nHeight = nHeight - (uExtentHeight / 2 ); 
			}
			else
			{
				nHeight += nPicH;
				nHeight = nHeight - (m_nExtentHeight/2);
			}
			
		}
	}
	
	//LogNormal("GetTextPos,w:%d,h:%d ", nWidth, nHeight);
}

//合成前后车牌匹配和特写图
bool CDspDataProcess::SetVtsImage2x3(
	const RECORD_PLATE_DSP_MATCH &record_plate, 
	const RECORD_PLATE_DSP_MATCH &foundRecord,
	const bool bIsReSize)
{
#ifdef MATCH_LIU_YANG_DEBUG
	//LogNormal("pImg:%x record Array[]:%x, %x, %x", record_plate.pImg, record_plate.pImgArray[0], record_plate.pImgArray[1], record_plate.pImgArray[2]);
	//LogNormal("pImg:%x found Array[]:%x, %x, %x", foundRecord.pImg, foundRecord.pImgArray[0], foundRecord.pImgArray[1], foundRecord.pImgArray[2]);
	LogNormal("SetVtsImage2x3 car1:%s, car2:%s", record_plate.dspRecord.chText, foundRecord.dspRecord.chText);
	LogNormal("A: %d,%d,%d,%d", \
		record_plate.dspRecord.uCarPosLeft,record_plate.dspRecord.uCarPosTop,
		record_plate.dspRecord.uCarPosRight, record_plate.dspRecord.uCarPosBottom);
	LogNormal("B: %d,%d,%d,%d", \
		foundRecord.dspRecord.uCarPosLeft,foundRecord.dspRecord.uCarPosTop,
		foundRecord.dspRecord.uCarPosRight, foundRecord.dspRecord.uCarPosBottom);
#endif
	if(record_plate.pImgArray[0] == NULL || foundRecord.pImg == NULL)
	{
		return false;
	}
	bool bRet = true;

	int nExtentHeight = m_nExtentHeight;
	if (bIsReSize) //如果压缩，则取重新计算的后的扩展区域 压缩前扩展区域高度 =  实际扩展区域高度 / 压缩比例  ，实际扩展区域高度 = 字体大小 *2 +10
	{
		if (g_PicFormatInfo.nResizeScale >= 60 && g_PicFormatInfo.nResizeScale < 100  )
		{
			nExtentHeight = (g_PicFormatInfo.nFontSize * 4 / 5 * 2 + 10 )  * 100 / g_PicFormatInfo.nResizeScale;
		}
		else if (g_PicFormatInfo.nResizeScale >= 5 && g_PicFormatInfo.nResizeScale < 60  )
		{
			nExtentHeight = (g_PicFormatInfo.nFontSize * 3 / 5 * 2 + 10 )  * 100  / g_PicFormatInfo.nResizeScale;
		}
	}
	//合成图片清空
	cvSet(m_imgComposeMatch, cvScalar( 0,0, 0 ));
	IplImage *pImgDeal = NULL;//当前处理图片

	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;
	rectPic.width = foundRecord.pImg->width;
	rectPic.height = foundRecord.pImg->height - nExtentHeight;
	
	CvRect rtSrc;//取图区域,当前关注区域
	CvRect rtDst;//合成大图中目的写入区域
	rtDst.x = 0;
	rtDst.y = 0;
	rtDst.width = foundRecord.pImg->width;
	rtDst.height = foundRecord.pImg->height - nExtentHeight;

	//LogNormal("rectPic:%d,%d,%d,%d ", rectPic.x, rectPic.y, rectPic.width, rectPic.height);
	//LogNormal("foundRecord:%d,%d", foundRecord.pImg->width, foundRecord.pImg->height);
	
	bool bTeXie = false;

	int nRow = 3;
	int nCol = 2;
	for(int i = 0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			rtDst.x = j * rectPic.width;
			rtDst.y = i * rectPic.height + i*nExtentHeight;
			rtDst.width = rectPic.width;
			rtDst.height = rectPic.height;
			
			rtSrc.x = 0;
			rtSrc.y = 0;
			rtSrc.width = rectPic.width;
			rtSrc.height = rectPic.height;//
			//bTeXie = false;		
			if(m_nWordPos == 1)//黑边在上
			{
				rtDst.y += nExtentHeight;
			}

			if(0 == i)
			{
				if(0 == j)
				{
					bTeXie = true;
					if (record_plate.pImg != NULL)
					{
						pImgDeal = record_plate.pImg;
					}
					else
					{
						pImgDeal = record_plate.pImgArray[0];
					}
					
					GetCarRectFromPlate(record_plate.dspRecord, rtSrc);	
				}
				else if(1 == j)
				{
					pImgDeal = record_plate.pImgArray[0];			
				}
			}
			else if(1 == i)
			{
				if(0 == j)
				{
					pImgDeal = record_plate.pImgArray[1];
				}
				else if(1 == j)
				{
					pImgDeal = record_plate.pImgArray[2];
				}
			}
			else if(2 == i)
			{
				if(0 == j)
				{
					bTeXie = true;
					pImgDeal = foundRecord.pImg;
					//GetCarRect(foundRecord.dspRecord, rtSrc);
					GetCarRectFromPlate(foundRecord.dspRecord, rtSrc);
				}
				else if(1 == j)
				{
					pImgDeal = foundRecord.pImg;
				}
			}
			//bool bDeal = false;
			if(!bTeXie)
			{
				rtSrc.x = 0;
				rtSrc.y = 0;
				rtSrc.width = rectPic.width;
				//rtSrc.height = rectPic.height - m_nExtentHeight;
				rtSrc.height = rectPic.height;

			//	bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeMatch, rtDst);
			}
			else
			{
				if(m_nWordPos == 1)//黑边在上
				{
					rtDst.y -= nExtentHeight;					
				}
				//rtDst.height -= m_nExtentHeight; //不留黑边

			//	bDeal = DealComposeImgTeXie(pImgDeal, rtSrc, m_imgComposeMatch, rtDst);
			}
			
			
			//处理一张图片
			bool bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeMatch, rtDst);

			if(pImgDeal)
			{
				pImgDeal = NULL;
			}

			if(!bDeal)
			{
				bRet = false;
				break;//处理出错,中断处理
			}

			//SaveImgTest(m_imgComposeResult);
		}//End of for j		
	}//End of for i

	if(pImgDeal)
	{
		pImgDeal = NULL;
	}

	return bRet;
}


//处理一张图片
bool CDspDataProcess::DealComposeImg(IplImage* pImgDeal, CvRect &rtSrc, IplImage* pImgDest, CvRect &rtDst)
{
	bool bRet = true;

	if(pImgDeal == NULL || pImgDest == NULL)
	{
		return false;
	}

#ifdef MATCH_LIU_YANG_DEBUG
	LogNormal("rtSrc:%d,%d,%d,%d", rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height);
	//LogNormal("pImgDeal:w,%d,h,%d", pImgDeal->width, pImgDeal->height);
	LogNormal("rtDst:%d,%d,%d,%d", rtDst.x, rtDst.y, rtDst.width, rtDst.height);
	//LogNormal("pImgDest:w,%d,h,%d", pImgDest->width, pImgDest->height);
#endif

	bool bCheck1 = CheckRectInImg(rtSrc, pImgDeal);
	bool bCheck2 = CheckRectInImg(rtDst, pImgDest);

	//LogNormal("bCheck1:%d, bCheck:%d ", bCheck1, bCheck2);
	if(bCheck1 && bCheck2)
	{
		//check,img,rect
		cvSetImageROI(pImgDeal,rtSrc);
		cvSetImageROI(pImgDest,rtDst);
		cvResize(pImgDeal, pImgDest);
		cvResetImageROI(pImgDest);
		cvResetImageROI(pImgDeal);
	}

	return bRet;
}

bool CDspDataProcess::DealComposeImgTeXie( IplImage* pImgDeal, CvRect &rtSrc, IplImage* pImgDest, CvRect &rtDst )
{
	bool bRet = true;

	if(pImgDeal == NULL || pImgDest == NULL)
	{
		return false;
	}

#ifdef MATCH_LIU_YANG_DEBUG
	LogNormal("rtSrc:%d,%d,%d,%d", rtSrc.x, rtSrc.y, rtSrc.width, rtSrc.height);
	//LogNormal("pImgDeal:w,%d,h,%d", pImgDeal->width, pImgDeal->height);
	LogNormal("rtDst:%d,%d,%d,%d", rtDst.x, rtDst.y, rtDst.width, rtDst.height);
	//LogNormal("pImgDest:w,%d,h,%d", pImgDest->width, pImgDest->height);
#endif

	bool bCheck1 = CheckRectInImg(rtSrc, pImgDeal);
	bool bCheck2 = CheckRectInImg(rtDst, pImgDest);

	//LogNormal("bCheck1:%d, bCheck:%d ", bCheck1, bCheck2);
	
	if(bCheck1 && bCheck2)
	{
		//check,img,rect
		cvSetImageROI(pImgDeal,rtSrc);//原图中车辆位置

		CvRect rTemp;
		//double dScale1=(double) rtSrc.height/rtDst.height;
		//double dScale2=(double) rtSrc.width/rtDst.width;
		rTemp.x = rtDst.x;
		rTemp.y = rtDst.y;
		rTemp.height = rtDst.height;
		rTemp.width = rtDst.height * rtSrc.width / rtSrc.height;

		if (rTemp.width >= rtDst.width)
		{
			rTemp.width = rtDst.width ;
		}
		//LogNormal("rtDst:%d,%d,%d,%d", rtDst.x, rtDst.y, rtDst.width, rtDst.height);
		cvSetImageROI(pImgDest,rTemp);//右边大图区域
		cvResize(pImgDeal, pImgDest); //车辆resize到右边大图显示区域
		cvResetImageROI(pImgDest);
		cvResetImageROI(pImgDeal);
	}

	return bRet;
}

//存储违章图像2x3
void CDspDataProcess::SaveComposeVtsImageMatch(RECORD_PLATE& plate,IplImage* pImgSave)
{
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
	//大图宽高
	plate.uPicWidth = m_imgComposeMatch->width;
	plate.uPicHeight = m_imgComposeMatch->height;
	
	//SaveImgTest(m_imgComposeMatch, plate.chText);
	if (pImgSave != NULL)
	{
		plate.uPicSize = SaveImage(pImgSave,strPicPath,6);
	}
	else
	{
		plate.uPicSize = SaveImage(m_imgComposeMatch,strPicPath,6);
	}
	
	
	//LogNormal("SaveComposeVtsImageMatch .uPicSize:%d ", plate.uPicSize);
}

//获取车身位置
void CDspDataProcess::GetCarRectFromPlate(const RECORD_PLATE &plate, CvRect &rtCarPos)
{
	rtCarPos.x = plate.uCarPosLeft; 
	rtCarPos.y = plate.uCarPosTop; 
	rtCarPos.width = plate.uCarPosRight - plate.uCarPosLeft; 
	rtCarPos.height = plate.uCarPosBottom - plate.uCarPosTop;
#ifdef MATCH_LIU_YANG_DEBUG
	LogNormal("1GetCarRect x:%d,y:%d,w:%d,h:%d", rtCarPos.x, rtCarPos.y, rtCarPos.width, rtCarPos.height);
#endif

	if(rtCarPos.x <= 0 || rtCarPos.y <= 0 || rtCarPos.width <= 0 || rtCarPos.height <= 0)
	{
		//GetCarRect2(plate, rtCarPos);
		GetCarRect(plate, rtCarPos);
	}
#ifdef MATCH_LIU_YANG_DEBUG
	LogNormal("2GetCarRect x:%d,y:%d,w:%d,h:%d", rtCarPos.x, rtCarPos.y, rtCarPos.width, rtCarPos.height);
#endif
}

//#ifdef MATCH_LIU_YANG
//解码图片
//bType,数据类型是否为违章,false:卡口 true:违章
bool CDspDataProcess::SetVtsImgMatch(RECORD_PLATE_DSP_MATCH &plateMatch, const bool bType)
{
	bool bRet = false;

//#ifdef DEBUG_LIUYANG
//	LogNormal("SetVtsImgMatch bType:%d car:%s vio:%d", \
//		bType, plateMatch.dspRecord.chText, plateMatch.dspRecord.uViolationType);
//
//	LogNormal("key[%d,%d,%d,%d] ", \
//		plateMatch.uKeyArray[0], plateMatch.uKeyArray[1], plateMatch.uKeyArray[2], plateMatch.uKeyArray[3]);
//#endif

	bool bDecode = false;
	if(!bType)
	{
		std::string strPic = "";
		bool bCheck = CheckPlateMatch(plateMatch, 0);
		if(bCheck)
		{
			strPic.append((char*)(plateMatch.pPicArray[0]), plateMatch.nSizeArray[0]);		

			if(strPic.size() > 10)
			{
				bRet = DecodeJpgMatch(&plateMatch.pImg,strPic);
				UpdateImgKeyByChannelID(plateMatch.dspRecord.uChannelID, plateMatch.uKeyArray[0], 0);
			}
		}		
	}
	else
	{
		for(int i=0; i<3; i++)
		{
			std::string strPic = "";
			bool bCheck = CheckPlateMatch(plateMatch, i+1);

			if(bCheck)
			{
				strPic.append((char*)(plateMatch.pPicArray[i+1]), plateMatch.nSizeArray[i+1]);
				if(strPic.size() > 10)
				{
					bRet = DecodeJpgMatch(&plateMatch.pImgArray[i], strPic);
					UpdateImgKeyByChannelID(plateMatch.dspRecord.uChannelID, plateMatch.uKeyArray[i+1], i+1);
				}
			}			
		}

		//特写取卡口图
		if(0 == plateMatch.uKeyArray[0])
		{
			//LogNormal("3id:%d recId:%d Key:%lld,%lld,%lld,%lld Sta:%d car:%s!", \
			//	m_nChannelId, plateMatch.dspRecord.uChannelID, \
			//	plateMatch.uKeyArray[0], plateMatch.uKeyArray[1], plateMatch.uKeyArray[2], plateMatch.uKeyArray[3], \
			//	plateMatch.bKeyStateArray[0], plateMatch.dspRecord.chText);
			//assert(plateMatch.dspRecord.uChannelID != 0);		
			//assert(plateMatch.uKeyArray[0] != 0);
			//assert(plateMatch.bKeyStateArray[0] != 0);

			//取违章第一张图
			plateMatch.uKeyArray[0] = plateMatch.uKeyArray[1];
			plateMatch.bKeyStateArray[0] = plateMatch.bKeyStateArray[1];
		}

		std::string strPicKakou = "";
		bool bCheck = CheckPlateMatch(plateMatch, 0);
		if(bCheck)
		{
			strPicKakou.append((char*)(plateMatch.pPicArray[0]), plateMatch.nSizeArray[0]);
			if(strPicKakou.size() > 10)
			{
				bRet = DecodeJpgMatch(&plateMatch.pImg, strPicKakou);
				UpdateImgKeyByChannelID(plateMatch.dspRecord.uChannelID, plateMatch.uKeyArray[0], 0);
			}
		}		
	}

#ifdef MATCH_LIU_YANG_DEBUG
	LogNormal("SetVtsImgMatch %x, %x, %x, %x ", plateMatch.pImg, \
		plateMatch.pImgArray[0], plateMatch.pImgArray[1], plateMatch.pImgArray[2]);
#endif
	return bRet;
}
//#endif

//#ifdef MATCH_LIU_YANG
//解码需要比对的jpg图片
bool CDspDataProcess::DecodeJpgMatch(IplImage **ppImg, const string &strPic)
{
	bool bRet = false;
	
	if(strPic.size() > 10)
	{
		//加锁
		pthread_mutex_lock(&m_ImgTagMutex);
	
		if(!m_imgTagList[m_uCurrImgtag].bUse)
		{
			m_imgTagList[m_uCurrImgtag].bUse = true;

			*ppImg = m_imgTagList[m_uCurrImgtag].pImg;

			if(*ppImg)
			{
				bRet = DecodeJpg(*ppImg, strPic);

#ifdef MATCH_LIU_YANG_DEBUG
				LogNormal("DecodeJpgMatch strPic:%d, bRet:%d pImg:%x", strPic.size(), bRet, *ppImg);
#endif
				if(bRet)
				{
					m_uPrevImgag = m_uCurrImgtag;
					m_uCurrImgtag++;
					if(MAX_IMG_MATCH_TAG_NUM == m_uCurrImgtag)
					{
						m_uCurrImgtag = 0;
					}
				}
				else
				{
					LogNormal("decode fail strPic=%d,pImg=%x\n",strPic.size(),*ppImg);
				}
			}			

			m_imgTagList[m_uCurrImgtag].bUse = false;
		}
		pthread_mutex_unlock(&m_ImgTagMutex);
	}

	return bRet;
}
//#endif

//解码jpg图片
bool CDspDataProcess::DecodeJpg(IplImage *pImg, const string &strPic)
{
	bool bRet = false;

	if(strPic.size() > 10)
	{
		if(pImg != NULL)
		{
			//需要解码jpg图像
			CxImage image;
			image.Decode((BYTE*)(strPic.c_str()), strPic.size(), 3); //解码

			if(image.IsValid()&&image.GetSize()>0)
			{
				memcpy(pImg->imageData,image.GetBits(),image.GetSrcSize());
				bRet = true;
			}
		}		
	}

	return bRet;
}

//车型检测
void CDspDataProcess::DetectTruck(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context)
{	
	#ifdef GLOBALCARCLASSIFY
	 //车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
		  || (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
	{
		
		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);

		return;
    }

	#ifdef GLOBALCARLABEL
	if(!(m_nDetectDirection == 1 && plate.uColor == CARNUM_YELLOW && cardNum.strCarNum[6] != '$'))//尾牌为黄牌且不为“学“字则进入车型模块
	{
		return;
	}
	#endif

	int nRet = 0;
	int nMoreDetail = 0;
	{

		std::string strNumber;
		strNumber = cardNum.strCarNum;
		if(*(strNumber.c_str()+strNumber.size()-1)!='$')
		{
			struct timeval tv1,tv2;
			if(g_nPrintfTime == 1)
			{
				gettimeofday(&tv1,NULL);
			}

			//LogNormal("before mvTruckDetect m_imgSnap(%d,%d),m_nDayNight=%d\n",m_imgSnap->width,m_imgSnap->height,m_nDayNight);
			//车牌检测区域
			CvRect rtRoi;
			rtRoi.x = m_rtCarnumROI.x;
			rtRoi.width = m_rtCarnumROI.width;
			rtRoi.y = m_rtCarnumROI.y;
			rtRoi.height = m_rtCarnumROI.height;

			cvSetImageROI(m_imgSnap,rtRoi);

			bool IsForeCarnum=true;
			if(m_nDetectDirection == 1)
			{
				IsForeCarnum = false;
			}

			pthread_mutex_lock(&g_vehicleClassifyMutex);
			nRet = g_vehicleClassify.mvTruckDetect(m_imgSnap,context,m_nDayNightbyLight,IsForeCarnum,true,&nMoreDetail);
			pthread_mutex_unlock(&g_vehicleClassifyMutex);

			cvResetImageROI(m_imgSnap);

		}
		cardNum.subVehicleType = nRet;
	}

	plate.uTypeDetail = GetTypeDetail(nRet);
	plate.uDetailCarType = nMoreDetail;
#endif
}

//车身颜色检测
void CDspDataProcess::CarColorDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context)
{	
#ifdef GLOBALCARCOLOR
	//车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
		|| (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
	{

		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
		return;
	}
	object_color objColor;

	//车牌检测区域
	CvRect rtRoi;
	rtRoi.x = m_rtCarnumROI.x;
	rtRoi.width = m_rtCarnumROI.width;
	rtRoi.y = m_rtCarnumROI.y;
	rtRoi.height = m_rtCarnumROI.height;

	int nExtentHeight = 0;
	if(m_nWordPos == 1)
	{
		nExtentHeight = m_nExtentHeight;
	}

	pthread_mutex_lock(&g_carColorMutex);
	#ifndef ALGORITHM_DL
	g_carColor.mvSetImageWidthandHeight(m_imgSnap->width,m_imgSnap->height);
	g_carColor.mvGetCardCarColor(m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,&objColor,m_nDayNightbyLight,true);
	#endif
	pthread_mutex_unlock(&g_carColorMutex);


	plate.uCarColor1 = objColor.nColor1;
	plate.uWeight1 = objColor.nWeight1;
	plate.uCarColor2 = objColor.nColor2;
	plate.uWeight2 = objColor.nWeight2;

#endif
}

//车标检测
void CDspDataProcess::CarLabelDetect(CarInfo& cardNum,RECORD_PLATE& plate,carnum_context& context)
{	
#ifdef GLOBALCARLABEL
	//车牌位置合理性判断
	if(cardNum.iwidth <= 0 || cardNum.iheight <= 0 || cardNum.ix <= 0 || cardNum.iy <= 0 || cardNum.ix >= m_imgSnap->width || cardNum.iy >= m_imgSnap->height - m_nExtentHeight
		|| (cardNum.ix + cardNum.iwidth >= m_imgSnap->width) || (cardNum.iy + cardNum.iheight >= m_imgSnap->height - m_nExtentHeight))
	{

		LogError("车牌位置不合理,%d,%d,%d,%d\n",cardNum.ix,cardNum.iy,cardNum.iwidth,cardNum.iheight);
		return;
	}


	//车牌检测区域
	CvRect rtRoi;
	rtRoi.x = m_rtCarnumROI.x;
	rtRoi.width = m_rtCarnumROI.width;
	rtRoi.y = m_rtCarnumROI.y;
	rtRoi.height = m_rtCarnumROI.height;

	int uCarBrand = 0;
	int nTruckDetect = 0;
	int nExtentHeight = 0;
	if(m_nWordPos == 1)
	{
		nExtentHeight = m_nExtentHeight;
	}
	cvDrawRect(m_imgSnap, cvPoint(context.position.x, context.position.y+nExtentHeight), cvPoint(context.position.x+context.position.width-1, context.position.y+context.position.height-1), CV_RGB(255,0,0), 4);
	pthread_mutex_lock(&g_carLabelMutex);
	#ifndef ALGORITHM_DL
	g_carLabel.mvSetImageWidthandHeight(m_imgSnap->width,m_imgSnap->height-m_nExtentHeight);
	nTruckDetect = g_carLabel.mvGetClassifyEHDTexture( m_imgSnap->imageData+(m_imgSnap->widthStep*nExtentHeight),context,rtRoi,uCarBrand,m_nDayNightbyLight );
	#endif
	pthread_mutex_unlock(&g_carLabelMutex);				  		

	plate.uCarBrand = uCarBrand;

	CBrandSusection brandsub;
#ifdef DETAIL_OLDBRAND
	plate.uCarBrand = brandsub.GetOldBrandFromDetail(uCarBrand);
#else
	brandsub.GetCarLabelAndChildSub(plate.uCarBrand,plate.uDetailCarBrand);
#endif

	if(m_nDetectKind&DETECT_TRUCK)
	{
		plate.uTypeDetail = GetTypeDetail(nTruckDetect);
	}
#endif
}
bool CDspDataProcess::GetRoadName( double dLongitude,double dLatitude,sRoadNameInfo & RoadInfo)
{
	CXmlParaUtil xmlobj;
	RoadNameInfoList listRoadInfo;
	bool bRet = xmlobj.LoadRoadItudeInfo(listRoadInfo);
	LogNormal("[%s]:listRoadInfo.size = %d\n",__FUNCTION__,listRoadInfo.size());

	RoadNameInfoList::iterator iter = listRoadInfo.begin();
	RoadNameInfoList tempRoadInfoList;
	tempRoadInfoList.clear();
	for (;iter != listRoadInfo.end();iter++)
	{
		//查找经纬度对应的区间
		double dMaxLongitude = (iter->dStartPosX > iter->dEndPosX?iter->dStartPosX:iter->dEndPosX);
		double dMinLongitude = (iter->dStartPosX < iter->dEndPosX?iter->dStartPosX:iter->dEndPosX);
		double dMaxLatitude  = (iter->dStartPosY > iter->dEndPosY?iter->dStartPosY:iter->dEndPosY);
		double dMinLatitude  = (iter->dStartPosY < iter->dEndPosY?iter->dStartPosY:iter->dEndPosY);

		if (dLongitude < dMinLongitude || dLongitude >dMaxLongitude||
			dLatitude  < dMinLatitude  || dLatitude  >dMaxLatitude)
		{
			continue;
		}
		else
		{
			tempRoadInfoList.push_back(*iter);
		}
	}
	LogNormal("[%s]:tempRoadInfoList.size = %d\n",__FUNCTION__,tempRoadInfoList.size());
	//如果有对应多个区间则计算最短距离
	if (tempRoadInfoList.size() > 1)
	{		
		double dMinDist = GetMinDistance(dLongitude,dLatitude,tempRoadInfoList);
		//LogNormal("[%s]:m_MapRoadName.size = %d,dMinDist = %f\n",__FUNCTION__,m_MapRoadName.size(),dMinDist);
		if (m_pMapRoadName->size() > 0)
		{
			mapRoadName::iterator iter= m_pMapRoadName->find(dMinDist);
			if (iter != m_pMapRoadName->end())
			{
				RoadInfo = iter->second;
			}
			return true;
		}			
		
	}
	else if(tempRoadInfoList.size() == 1)
	{
		RoadNameInfoList::iterator iter = tempRoadInfoList.begin();
		RoadInfo = *iter; 
		LogNormal("[%s]chStartPos = %s,chEndPos = %s\n",__FUNCTION__,RoadInfo.chStartPos,RoadInfo.chEndPos);
		return true;
	}
	return false;
}

double CDspDataProcess::DistanceOfPointToLine( double dLongitude,double dLatitude,sRoadNameInfo &RoadInfo )
{

	double dDistance;

	dDistance = DistanceOfPointAndLine(dLongitude,dLatitude,RoadInfo.dStartPosX,RoadInfo.dStartPosY,RoadInfo.dEndPosX,RoadInfo.dEndPosY);
	//LogNormal("dDistance = %.8f\n",dDistance);
	return dDistance;
}

double CDspDataProcess::GetMinDistance(double dLongitude,double dLatitude, RoadNameInfoList &RoadInfoList)
{
	double dMinDist = 0.0;

    //计算距离
	LogNormal("[%s]:RoadInfoList.size = %d\n",__FUNCTION__,RoadInfoList.size());

	m_pMapRoadName->clear();
	if (!RoadInfoList.empty())
	{
		RoadNameInfoList::iterator iter = RoadInfoList.begin();
		int iCout = 0; 
		for (; iter != RoadInfoList.end() ; iter ++)
		{	
			if ( iCout == 0)
			{
				dMinDist = DistanceOfPointToLine(dLongitude,dLatitude,*iter);
				m_pMapRoadName->insert(make_pair(dMinDist,*iter));
				iCout++;
				continue;
			}
			double dDist = DistanceOfPointToLine(dLongitude,dLatitude,*iter);
			m_pMapRoadName->insert(make_pair(dDist,*iter));

			//printf("fDist = %5.2f\n",fDist);
			if ( dMinDist > dDist)
			{
				dMinDist = dDist;
				//printf("fMinDist[%d] = %5.2f\n",i,fMinDist);
			}
		}
	}
	

	return dMinDist;
}

double CDspDataProcess::DistanceOfPointAndLine( double X,double Y, double AX,double AY,double BX,double BY )
{
	double a=0, b=0, c=0, t, d; 

	//由A1X,A1Y,B1X,B1Y所构成的线段的方程 ax+by+c=0 
	if( AX != BX ) 
	{ 
		t = double(BY-AY)/double(BX-AX); //斜率 
		a = t; 
		b = -1; 
		c = double(AY)-t*double(AX); 
	} 
	else //垂直线 
	{ 
		a = 1; 
		c = 0 - double(AX); 
	} 

	//点到线段的距离 
	d= fabs(a*X+b*Y+c) / sqrt(a*a+b*b) ; 
	LogNormal("[%s]:dDist = %.8f\n",__FUNCTION__,d);
	return d; 
}

//获取违章代码
bool CDspDataProcess::GetVtsCode(const UINT32 uViolationType, UINT32 &uVtsCode, std::string &strViolationType)
{
	bool bRet = true;

	uVtsCode = 0;
	//6合1图片上除了需要叠加违章类型，还需要叠加 违章号码
	//16250 - 闯红灯
	//13010 - 逆行
	//12080 - 不按导向车道行驶
	//13442 - 压线
	//13440 - 大货禁行
	switch(uViolationType)
	{
	case DETECT_RESULT_EVENT_GO_AGAINST:     //2 车辆逆行
	case DETECT_RESULT_RETROGRADE_MOTION:    //26 逆行
		{
			uVtsCode = 13010;
			strViolationType = "逆向行驶";
			break;
		}	
	case DETECT_RESULT_EVENT_GO_CHANGE:      //8 违章变道
	case DETECT_RESULT_ELE_EVT_BIANDAO:      //29 变道
	case DETECT_RESULT_TAKE_UP_NONMOTORWAY:          //机占非
		{
			uVtsCode = 12080;
			strViolationType = "不按导向车道行驶";
			break;
		}
	case DETECT_RESULT_FORBID_RIGHT:                    //24 禁止右拐
		{
			uVtsCode = 12080;
			strViolationType = "禁止右拐";
			break;
		}
	case DETECT_RESULT_FORBID_STRAIGHT:                 //25 禁止前行
		{
			uVtsCode = 12080;
			strViolationType = "禁止前行";
			break;
		}
	case DETECT_RESULT_RED_LIGHT_VIOLATION:   //16 闯红灯
		{
			uVtsCode = 16250;
			strViolationType = "闯红灯";
			break;
		}

	case DETECT_RESULT_PRESS_LINE:				//压黄线
		{
			uVtsCode = 13450;
			strViolationType = "压黄线";
			break;
		}
	case DETECT_RESULT_PRESS_WHITELINE:			//压白线
		{
			uVtsCode = 13450;
			strViolationType = "压白线";
			break;
		}
	case DETECT_RESULT_PRESS_LEAD_STREAM_LINE:	//53 压导流线
		{
			uVtsCode = 13450;
			strViolationType = "压导流线";
			break;
		}
	case DETECT_RESULT_FORBID_LEFT:                   //23 禁止左拐
		{
			uVtsCode = 12080;
			strViolationType = "禁止左拐";
				
			break;
		}
	case DETECT_RESULT_BIG_IN_FORBIDDEN_ROAD:    //19 大车出现在禁行车道13
	case DETECT_RESULT_BIG_IN_FORBIDDEN_TIME:	 //20 大车出现在禁行时间(大货禁行)
		{
			uVtsCode = 13440;
			strViolationType = "大货禁行";
			break;
		}
	default:
		{
			bRet = false;
			break;
		}
		
	}

	return bRet;
}

#ifdef REDADJUST
//红绿灯增强
void CDspDataProcess::RedLightAdjustMatch(
	RECORD_PLATE_DSP_MATCH &record_plate)
{
	if(0 != record_plate.dspRecord.uViolationType)
	{
		for(int i=0; i<3; i++)
		{
			if(record_plate.pImgArray[i])
			{
				//LogNormal("1 record_plate:%s id:%d mId:%d RedLightAdjustMatch", \
				//	record_plate.dspRecord.chText, record_plate.dspRecord.uChannelID, m_nChannelId);
				RedLightAdjust(record_plate.dspRecord.uChannelID, record_plate.pImgArray[i]);
			}
		}		
	}
	else
	{
		if(record_plate.pImg)
		{
			//LogNormal("2 record_plate:%s id:%d mId:%d RedLightAdjustMatch", \
			//	record_plate.dspRecord.chText, record_plate.dspRecord.uChannelID, m_nChannelId);
			RedLightAdjust(record_plate.dspRecord.uChannelID, record_plate.pImg);
		}
	}
}
#endif

#ifdef REDADJUST
//红绿灯增强
void CDspDataProcess::RedLightAdjust(int nChannel, IplImage* pImage)
{
	if(pImage)
	{
		g_skpChannelCenter.RedLightAdjust(nChannel, pImage);
	}
	
}
#endif


//输出过滤
bool CDspDataProcess::OutPutVtsMatchFilter(const RECORD_PLATE_DSP_MATCH &record_plate)
{
	bool bDeal = false;	
	switch(record_plate.dspRecord.uViolationType)
	{
	case DETECT_RESULT_EVENT_GO_AGAINST:     //2 车辆逆行
	case DETECT_RESULT_RETROGRADE_MOTION:    //26 逆行		
	case DETECT_RESULT_EVENT_GO_CHANGE:      //8 违章变道
	case DETECT_RESULT_ELE_EVT_BIANDAO:      //29 变道
		{
			bDeal = false;//不输出
			break;
		}
	default:
		{
			bDeal = true;//输出
			break;
		}
	}

	return bDeal;
}

//输出车牌前后匹配合成图-浏阳版本-2x2格式
void CDspDataProcess::OutPutVtsMatch2x2(
	RECORD_PLATE_DSP_MATCH &record_plate, 
	RECORD_PLATE_DSP_MATCH &foundRecord, 
	RECORD_PLATE& plate)
{
#ifdef MATCH_LIU_YANG_DEBUG
	bool bDeal = OutPutVtsMatchFilter(record_plate);
	if(!bDeal)
	{
		return;
	}
#endif

	UINT32 uWidth = record_plate.dspRecord.uPicWidth;
	UINT32 uHeight = record_plate.dspRecord.uPicHeight;

	//printf("[%s]uW=%d,uH=%d,w=%d,h=%d\n",__FUNCTION__,uWidth,uHeight,m_imgSnap->width,m_imgSnap->height);
	LogNormal("[%s]:%s",__FUNCTION__,record_plate.dspRecord.chText);
	if (m_imgComposeMatch == NULL)
	{
		Init(record_plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
	}

	//解码图片
	bool bSetA = SetVtsImgMatch(record_plate, true);
	bool bSetB = SetVtsImgMatch(foundRecord, false);

	//LogNormal("pImgA:%x pImgB:%x OutPutVtsMatch2x3", record_plate.pImgArray[1], foundRecord.pImg);

	if(!bSetA || !bSetB)
	{
		LogNormal("bSetA:%d , bSetB:%d OutPutVtsMatch2x3 err!", bSetA, bSetB);
		return;
	}

	//红绿灯增强
#ifdef REDADJUST
	RedLightAdjustMatch(record_plate);
#endif

	ViolationInfo infoViolation;

	//PLATEPOSITION* pTimeStamp;
	PLATEPOSITION TimeStamp[4];
	SetVtsInfo(record_plate, foundRecord, plate, infoViolation, TimeStamp);

	//合成违章图片
	bool bSetVtsImg = SetVtsImage2x2(record_plate, foundRecord);
	//合成违章叠字
	SetVtsText2x2(m_imgComposeMatch, plate, infoViolation, TimeStamp, record_plate, foundRecord);//TimeStamp,SignalTimeStamp,infoViolation.redLightStartTime);

	if(!bSetVtsImg)
	{
		LogNormal("OutPutVtsMatch2x2 error!");
		return;
	}

	//获取录像以及图片路径
	pthread_mutex_lock(&g_Id_Mutex);
	//获取图片路径
	std::string strPicPath;
	int nSaveRet = GetVtsPicSavePath(plate, 2,strPicPath);
	pthread_mutex_unlock(&g_Id_Mutex);

	//删除已经存在的记录
	g_skpDB.DeleteOldRecord(strPicPath,false,false);

	//存储违章图像
	SaveComposeVtsImageMatch(plate);

	//保存记录
	if(nSaveRet>0)
	{		
		g_skpDB.SavePlate(record_plate.dspRecord.uChannelID,plate,0);
	}

	//将车牌信息送客户端
	SendResult(plate,infoViolation.carInfo.uSeq);
}


//合成前后车牌匹配和特写图
bool CDspDataProcess::SetVtsImage2x2(
	const RECORD_PLATE_DSP_MATCH &record_plate, 
	const RECORD_PLATE_DSP_MATCH &foundRecord)
{
#ifdef MATCH_LIU_YANG_DEBUG
	//LogNormal("pImg:%x record Array[]:%x, %x, %x", record_plate.pImg, record_plate.pImgArray[0], record_plate.pImgArray[1], record_plate.pImgArray[2]);
	//LogNormal("pImg:%x found Array[]:%x, %x, %x", foundRecord.pImg, foundRecord.pImgArray[0], foundRecord.pImgArray[1], foundRecord.pImgArray[2]);
	LogNormal("SetVtsImage2x2 car1:%s, car2:%s", record_plate.dspRecord.chText, foundRecord.dspRecord.chText);
	LogNormal("A: %d,%d,%d,%d", \
		record_plate.dspRecord.uCarPosLeft,record_plate.dspRecord.uCarPosTop,
		record_plate.dspRecord.uCarPosRight, record_plate.dspRecord.uCarPosBottom);
	LogNormal("B: %d,%d,%d,%d", \
		foundRecord.dspRecord.uCarPosLeft,foundRecord.dspRecord.uCarPosTop,
		foundRecord.dspRecord.uCarPosRight, foundRecord.dspRecord.uCarPosBottom);
#endif
	if(record_plate.pImgArray[0] == NULL || foundRecord.pImg == NULL)
	{
		return false;
	}

	bool bRet = true;	

	//合成图片清空
	cvSet(m_imgComposeMatch, cvScalar( 0,0, 0 ));
	IplImage *pImgDeal = NULL;//当前处理图片

	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;
	rectPic.width = foundRecord.pImg->width;
	rectPic.height = foundRecord.pImg->height - m_nExtentHeight;
	
	CvRect rtSrc;//取图区域,当前关注区域
	CvRect rtDst;//合成大图中目的写入区域
	rtDst.x = 0;
	rtDst.y = 0;
	rtDst.width = foundRecord.pImg->width;
	rtDst.height = foundRecord.pImg->height - m_nExtentHeight;

	//LogNormal("rectPic:%d,%d,%d,%d ", rectPic.x, rectPic.y, rectPic.width, rectPic.height);
	//LogNormal("foundRecord:%d,%d", foundRecord.pImg->width, foundRecord.pImg->height);
	
	bool bTeXie = false;

	int nRow = 2;
	int nCol = 2;
	for(int i = 0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			rtDst.x = j * rectPic.width;
			rtDst.y = i * rectPic.height + i*m_nExtentHeight;
			rtDst.width = rectPic.width;
			rtDst.height = rectPic.height;
			
			rtSrc.x = 0;
			rtSrc.y = 0;
			rtSrc.width = rectPic.width;
			rtSrc.height = rectPic.height;//
					
			if(m_nWordPos == 1)//黑边在上
			{
				rtDst.y += m_nExtentHeight;
			}

			if(0 == i)
			{
				if(0 == j)
				{
					pImgDeal = record_plate.pImgArray[0];					
				}
				else if(1 == j)
				{
					pImgDeal = record_plate.pImgArray[1];
				}				
			}
			else if(1 == i)
			{
				if(0 == j)
				{
					pImgDeal = record_plate.pImgArray[2];
				}
				else if(1 == j)
				{
					pImgDeal = foundRecord.pImg;
				}				
			}
			else
			{
				//error i,j
			}


			if(m_nWordPos == 1)//黑边在上
			{
				rtDst.y -= m_nExtentHeight;					
			}

			
			//处理一张图片
			bool bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeMatch, rtDst);

			if(pImgDeal)
			{
				pImgDeal = NULL;
			}

			if(!bDeal)
			{
				bRet = false;
				break;//处理出错,中断处理
			}

			//SaveImgTest(m_imgComposeResult);
		}//End of for j		
	}//End of for i

	if(pImgDeal)
	{
		pImgDeal = NULL;
	}

	return bRet;
}


//合成违章叠字
void CDspDataProcess::SetVtsText2x2(
	IplImage * pImage,
	const RECORD_PLATE  &plate,
	const ViolationInfo &vioInfo,
	PLATEPOSITION* pTimeStamp,
	const RECORD_PLATE_DSP_MATCH &record_plate,
	const RECORD_PLATE_DSP_MATCH &foundRecord)
{
	if(pImage == NULL)
	{
		return;
	}

	int nRow = 2;
	int nCol = 2;

	//文本初始化
    int nFontSize = g_PicFormatInfo.nFontSize;

	CvxText cvText;
	cvText.Init(nFontSize);

	int nWidth = 0;
	int nHeight = 0;

	wchar_t wchOut[255] = {'\0'};	
	std::string strText = "";

	std::string strDeviceId = "";
	std::string strPlace = "";
	int64_t ts = 0;
	for(int i=0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			//叠字位置
			GetTextPos(nWidth, nHeight, i, j);

			strDeviceId = std::string(record_plate.szDeviceID);
			strPlace = std::string(record_plate.dspRecord.chPlace);

			ts = 0;
			//取得时间戳
			if(i==0 && j==0)
			{
				ts = pTimeStamp[0].ts;
			}			
			else if(i==0 && j==1)
			{
				ts = pTimeStamp[1].ts;
			}
			else if(i==1 && j==0)
			{
				ts = pTimeStamp[2].ts;
			}
			else if(i==1 && j==1)
			{
				ts = pTimeStamp[3].ts;
				strDeviceId = std::string(foundRecord.szDeviceID);
				strPlace = std::string(foundRecord.dspRecord.chPlace);
			}			
			else{}

			//叠字内容
			strText = GetVtsText(pImage,plate, vioInfo, i, j, ts, strDeviceId, strPlace);

			//转换编码
			//memset(wchOut,0,sizeof(wchOut));
			//UTF8ToUnicode(wchOut,(char*)strText.c_str());
			//
			////叠字
			//cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));			
		}
	}
	
	cvText.UnInit();
}

void CDspDataProcess::OutPutVtsMatch1x3( 
	RECORD_PLATE_DSP_MATCH &record_plate, 
	RECORD_PLATE_DSP_MATCH &foundRecord, 
	RECORD_PLATE& plate )
{
	//LogNormal("[%s]:start!\n",__FUNCTION__);
//#ifdef MATCH_LIU_YANG

	UINT32 uWidth = record_plate.dspRecord.uPicWidth;
	UINT32 uHeight = record_plate.dspRecord.uPicHeight;
	
	//printf("[%s]uW=%d,uH=%d,w=%d,h=%d\n",__FUNCTION__,uWidth,uHeight,m_imgSnap->width,m_imgSnap->height);

	if (m_imgComposeMatchKaKou == NULL)
	{
		Init(record_plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
	}
	//解码图片
	bool bSetA = SetVtsImgMatch(record_plate, false);  //卡口
	bool bSetB = SetVtsImgMatch(foundRecord, false);
	
	//LogNormal("rPlate=%s,pImgA:%x pImgB:%x OutPutVtsMatch2x3",record_plate.dspRecord.chText,record_plate.pImg, foundRecord.pImg);

	if(!bSetA)
	{
		//for test
#ifdef LIU_YANG_TEST
		char jpg_name[256] = {0};
		FILE *pFile;
		sprintf(jpg_name, "./Log/pic/r13_%d_%s.jpg", 
		record_plate.dspRecord.uSeqID,record_plate.dspRecord.chText);
		pFile = fopen(jpg_name, "wb+");
		if (pFile != NULL)
		{
			fwrite(record_plate.pPicArray[0], record_plate.nSizeArray[0],1, pFile);
		}		
		fclose(pFile);
#endif //LIU_YANG_TEST
		LogNormal("bSetA:%d,%sOutPutVtsMatch1x3 err!", bSetA,record_plate.dspRecord.chText);
		return;
	}
	if (!bSetB)
	{
#ifdef LIU_YANG_TEST
		char jpg_name[256] = {0};
		FILE *pFile;
		sprintf(jpg_name, "./Log/pic/f13_%d_%s.jpg", 
		foundRecord.dspRecord.uSeqID,foundRecord.dspRecord.chText);
		pFile = fopen(jpg_name, "wb+");
		if (pFile != NULL)
		{
			fwrite(foundRecord.pPicArray[0], foundRecord.nSizeArray[0], 1, pFile);
		}	
		fclose(pFile);
#endif //LIU_YANG_TEST
		LogNormal("bSetB:%d,%sOutPutVtsMatch1x3 err!",bSetB,foundRecord.dspRecord.chText);
		return;
	}
//#endif //MATCH_LIU_YANG
	//红绿灯增强
#ifdef REDADJUST
	RedLightAdjustMatch(record_plate);
#endif

	ViolationInfo infoViolation;
	
	//PLATEPOSITION* pTimeStamp;
	PLATEPOSITION TimeStamp[4];
	SetVtsInfo(record_plate, foundRecord, plate, infoViolation, TimeStamp);

	//合成卡口图片
	bool bSetVtsImg = SetVtsImage1x3(record_plate,foundRecord); 

	if(!bSetVtsImg)
	{
		LogNormal("[%s]:SetVtsImage1x3 error!",__FUNCTION__);
		return;
	}
	//合成卡口叠字
	SetKaKouText1x3(m_imgComposeMatchKaKou, plate, infoViolation, TimeStamp, record_plate, foundRecord);

	//获取录像以及图片路径
	pthread_mutex_lock(&g_Id_Mutex);
	//获取图片路径
	std::string strPicPath;
	int nSaveRet = GetVtsPicSavePath(plate, 2,strPicPath);
	pthread_mutex_unlock(&g_Id_Mutex);

	//删除已经存在的记录
	g_skpDB.DeleteOldRecord(strPicPath,false,false);

	//memcpy(plate.chText+strlen(plate.chText),"***",strlen("***"));  //for test
	//存储卡口图像
	SaveComposeKaKouImageMatch(plate);

	//1x3合图，车牌位置需要更新为前牌车牌区域
	plate.uPosLeft = foundRecord.dspRecord.uPosLeft;
	plate.uPosTop = foundRecord.dspRecord.uPosTop;
	plate.uPosRight = foundRecord.dspRecord.uPosRight;
	plate.uPosBottom = foundRecord.dspRecord.uPosBottom;

	//保存记录
	if(nSaveRet>0)
	{
		//g_skpDB.SavePlate(m_nChannelId,plate,0);
		g_skpDB.SavePlate(record_plate.dspRecord.uChannelID,plate,0);
	}

	//将车牌信息送客户端
	//车牌号码
	//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	//LogNormal("--SendResult-plate.uChannelID=%d\n", plate.uChannelID);
    
	SendResult(plate,infoViolation.carInfo.uSeq);
	//LogNormal("[%s]:plate=%s,success!\n",__FUNCTION__,record_plate.dspRecord.chText);
}

bool CDspDataProcess::SetVtsImage1x3( 
	const RECORD_PLATE_DSP_MATCH &record_plate, 
	const RECORD_PLATE_DSP_MATCH &foundRecord )
{
	/*LogNormal("[%s]:fpImg=%x,rpImg = %x\n",
	__FUNCTION__,foundRecord.pImg,record_plate.pImg);*/
	if(foundRecord.pImg == NULL || record_plate.pImg == NULL)
	{
		return false;
	}

	bool bRet = true;	

	//合成图片清空
	cvSet(m_imgComposeMatchKaKou, cvScalar( 0,0, 0 ));
	IplImage *pImgDeal = NULL;//当前处理图片

	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;
	rectPic.width = foundRecord.pImg->width;
	rectPic.height = foundRecord.pImg->height - m_nExtentHeight;

	CvRect rtSrc;//取图区域,当前关注区域
	CvRect rtDst;//合成大图中目的写入区域
	rtDst.x = 0;
	rtDst.y = 0;
	rtDst.width = foundRecord.pImg->width;
	rtDst.height = foundRecord.pImg->height - m_nExtentHeight;

	//LogNormal("rectPic:%d,%d,%d,%d ", rectPic.x, rectPic.y, rectPic.width, rectPic.height); //0,0,2448,2048
	//LogNormal("foundRecord:%d,%d", foundRecord.pImg->width, foundRecord.pImg->height); //2448,2108

	bool bTeXie = false;

	int nRow = 1;
	int nCol = 3;
	for(int i = 0; i< nRow; i++)
	{
		for(int j = 0; j< nCol; j++)
		{
			rtDst.x = j * rectPic.width;
			rtDst.y = i * rectPic.height + i*m_nExtentHeight;
			rtDst.width = rectPic.width;
			rtDst.height = rectPic.height;

			rtSrc.x = 0;
			rtSrc.y = 0;
			rtSrc.width = rectPic.width;
			rtSrc.height = rectPic.height;//
			//bTeXie = false;
			if(m_nWordPos == 1)//黑边在上
			{
				rtDst.y += m_nExtentHeight;
			}

			if(0 == i)
			{
				if(0 == j)			//第一张前牌卡口
				{
					pImgDeal = foundRecord.pImg;					
				}
				else if(1 == j)		//第二张尾牌卡口
				{
					pImgDeal = record_plate.pImg; 
				}
				else if (2 == j)	//第三张前牌特写
				{
					bTeXie = true;
					pImgDeal = foundRecord.pImg;
					GetCarRectFromPlate(foundRecord.dspRecord, rtSrc);
				}
			}

			//bool bDeal =false;
			if(!bTeXie)
			{
				rtSrc.x = 0;
				rtSrc.y = 0;
				rtSrc.width = rectPic.width;
				//rtSrc.height = rectPic.height - m_nExtentHeight;
				rtSrc.height = rectPic.height;

				//处理一张图片
				//bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeMatchKaKou, rtDst);
			}
			else
			{
				if(m_nWordPos == 1)//黑边在上
				{
					rtDst.y -= m_nExtentHeight;					
				}

				//bDeal = DealComposeImgTeXie(pImgDeal, rtSrc, m_imgComposeMatchKaKou, rtDst);
			}
			
			bool bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeMatchKaKou, rtDst);
			//LogNormal("[%s]:bDeal = %d,pImgDeal = %x,plate=%s\n",__FUNCTION__,bDeal,pImgDeal,foundRecord.dspRecord.chText);
			if(pImgDeal)
			{
				pImgDeal = NULL;
			}

			if(!bDeal)
			{
				bRet = false;

				break;//处理出错,中断处理
			}

			//SaveImgTest(m_imgComposeResult);
		}//End of for j		
	}//End of for i

	if(pImgDeal)
	{
		pImgDeal = NULL;
	}

	return bRet;
}

void CDspDataProcess::SetKaKouText1x3( IplImage * pImage, 
	const RECORD_PLATE &plate,
	const ViolationInfo &vioInfo, 
	PLATEPOSITION* pTimeStamp, 
	const RECORD_PLATE_DSP_MATCH &record_plate, 
	const RECORD_PLATE_DSP_MATCH &foundRecord )
{
	if(pImage == NULL)
	{
		return;
	}

	int nRow = 1;
	int nCol = 3;

	//文本初始化
	int nFontSize = g_PicFormatInfo.nFontSize;

	CvxText cvText;
	cvText.Init(nFontSize);

	int nWidth = 0;
	int nHeight = 0;
	
	wchar_t wchOut[255] = {'\0'};	
	std::string strText = "";

	std::string strDeviceId = "";
	std::string strPlace = "";
	int64_t ts = 0;

	for(int i=0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			//叠字位置
			//GetTextPos(nWidth, nHeight, i, j);

			strDeviceId = std::string(record_plate.szDeviceID);
			strPlace = std::string(record_plate.dspRecord.chPlace);

			ts = 0;
			//取得时间戳
			if(i==0 && j==0)
			{
				ts = pTimeStamp[3].ts; //前牌卡口时间戳
				strDeviceId = std::string(foundRecord.szDeviceID);
				strPlace = std::string(foundRecord.dspRecord.chPlace);
			}			
			else if(i==0 && j==1)
			{
				//ts = pTimeStamp[1].ts;  
				ts = record_plate.pPlatePos[0].ts;//尾牌卡口
			}
			else if(i==0 && j==2)
			{
				ts = pTimeStamp[3].ts; 
				strDeviceId = std::string(foundRecord.szDeviceID);
				strPlace = std::string(foundRecord.dspRecord.chPlace);
			}			
			else{}
			//叠字内容
			GetKaKouText(pImage,plate, i, j, ts, strDeviceId, strPlace);
		}
	}

	cvText.UnInit();
}

void CDspDataProcess::SaveComposeKaKouImageMatch( RECORD_PLATE& plate )
{
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
	//大图宽高
	plate.uPicWidth = m_imgComposeMatchKaKou->width;
	plate.uPicHeight = m_imgComposeMatchKaKou->height;

	//SaveImgTest(m_imgComposeMatch, plate.chText);
	plate.uPicSize = SaveImage(m_imgComposeMatchKaKou,strPicPath,2);

	//LogNormal("SaveComposeVtsImageMatch .uPicSize:%d ", plate.uPicSize);
}

void CDspDataProcess::OutPutNoMatch( RECORD_PLATE_DSP_MATCH &plate )
{
	//LogNormal("OutPutNoMatch id:%d, uKey:%lld", \
	//	plate.dspRecord.uChannelID, plate.uKeyArray[0]);
	
	UINT32 uWidth = plate.dspRecord.uPicWidth;
	UINT32 uHeight = plate.dspRecord.uPicHeight;

	if (NULL == m_imgComposeNoMatch)
	{
		Init(plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
	}
	
	//解码图片
	bool bSetA = SetVtsImgMatch(plate, false);  //卡口



//#ifdef DEBUG_LIUYANG
//	LogNormal("OutPutNoMatch id:%d, uKey:%lld,%lld uCar:%s bSetA:%d ", \
//		plate.dspRecord.uChannelID, plate.uKeyArray[0], plate.uKeyArray[1], plate.dspRecord.chText, bSetA);
//#endif

	if(!bSetA)
	{
#ifdef LIU_YANG_TEST
		char jpg_name[256] = {0};
		FILE *pFile;
		sprintf(jpg_name, "./Log/pic/r12_%d_%s.jpg", 
			plate.dspRecord.uSeqID,plate.dspRecord.chText);
		pFile = fopen(jpg_name, "wb+");
		if (pFile != NULL)
		{
			fwrite(plate.pPicArray[0], plate.nSizeArray[0],1,  pFile);
		}	
		//int nBytes = fwrite(plate.pPicArray[0], plate.nSizeArray[0], 1, pFile);
		fclose(pFile);
#endif //LIU_YANG_TEST
		LogNormal("bSetA:%d,plate=%s OutPutNoMatch err!", bSetA,plate.dspRecord.chText);
		return;
	}

	//红绿灯增强
#ifdef REDADJUST
	RedLightAdjustMatch(plate);
#endif

	//获取图片路径
	std::string strPicPath;
	int nSaveRet = GetVtsPicSavePath(plate.dspRecord, 2,strPicPath);

	//判断是否有车牌的车
	bool bCarNum = true;
	bool bLoop = false;	

	CarInfo    carnums;
	char text[10] = "123456789";
	if(plate.dspRecord.chText[0] != 'L') //非武警牌照
	{
		memset(text, 0, 10);
		memcpy(text, plate.dspRecord.chText, 7);
		memcpy(carnums.strCarNum, text, 7);
	}
	else
	{
		memset(text, 0, 10);
		memcpy(text, plate.dspRecord.chText, 9);
		memcpy(carnums.strCarNum, text, 7);
		memcpy(carnums.wj, text+7, 2); //武警牌下面的小数字
	}

	carnums.color = plate.dspRecord.uColor;
	carnums.vehicle_type = plate.dspRecord.uType;
	carnums.ix = (plate.dspRecord.uPosLeft);
	carnums.iy = (plate.dspRecord.uPosTop);
	carnums.iwidth = (plate.dspRecord.uPosRight - plate.dspRecord.uPosLeft);
	carnums.iheight = (plate.dspRecord.uPosBottom - plate.dspRecord.uPosTop);
	carnums.mean = -1;
	carnums.stddev = -1;

	//carnums.VerticalTheta = record_plate.dspRecord.uVerticalTheta;
	//carnums.HorizontalTheta = record_plate.dspRecord.uHorizontalTheta;

	carnums.uTimestamp = plate.dspRecord.uTime;//(pHeader->ts)/1000/1000;
	carnums.ts = plate.dspRecord.uTime * 1000 * 1000 + plate.dspRecord.uMiTime * 1000;
	carnums.uSeq = plate.dspRecord.uSeqID;
	carnums.carnumrow = plate.dspRecord.uType;//?
	carnums.wx = 0; //(double)((pPlate->uLongitude) * 0.0001);
	carnums.wy = 0; //(double)((pPlate->uLatitude) * 0.0001);
	carnums.id = 0; //pHeader->uCameraId;
	carnums.vx = 0;
	carnums.vy = plate.dspRecord.uSpeed; //传入dsp相机线圈检测出速度
	carnums.RoadIndex = plate.dspRecord.uRoadWayID;
	carnums.nDirection = plate.dspRecord.uDirection;

	if( (carnums.strCarNum[0] == '*')&& (carnums.strCarNum[6] == '*') )
	{
		bCarNum = false;
	}

	std::string strCarNum = carnums.strCarNum;
	if(bCarNum)
	{
		//车牌号码转换
		CarNumConvert(strCarNum,carnums.wj);
	}
	memcpy(plate.dspRecord.chText,strCarNum.c_str(),strCarNum.size());
	//保存卡口图片
	PLATEPOSITION TimeStamp[2];
	TimeStamp[0].ts = plate.pPlatePos[0].ts;

	//截取小图区域
	/*CvRect rtPos;
	if(m_nSmallPic == 1)
	{
	rtPos = GetCarPos(plate.dspRecord);
	}*/
	//合成单张卡口
	bool bRet = SetKaKouImage1x2(plate);

	////#ifdef DEBUG_LIUYANG
	//LogNormal("OutPutNoMatch id:%d, uKey:%lld  bRet:%d SetKaKouImage1x2", \
	//	plate.dspRecord.uChannelID, plate.uKeyArray[0], bRet);
	////#endif

	if (!bRet)
	{
		LogNormal("SetKaKouImage1x2 fail\n");
	}
	//设置单张卡口叠字
	SetKaKouText1x2(m_imgComposeNoMatch,TimeStamp,plate);

	//保存卡口图片
	std::string strPicPath2;
	strPicPath2.append(plate.dspRecord.chPicPath,sizeof(plate.dspRecord.chPicPath));
	//大图宽高
	plate.dspRecord.uPicWidth = m_imgComposeNoMatch->width;
	plate.dspRecord.uPicHeight = m_imgComposeNoMatch->height;
	plate.dspRecord.uPicSize = SaveImage(m_imgComposeNoMatch,strPicPath2,0);

	//保存记录
	if(nSaveRet>0)
	{
		g_skpDB.SavePlate(plate.dspRecord.uChannelID,plate.dspRecord,0);
	}

	//将车牌信息送客户端
	SendResult(plate.dspRecord,plate.dspRecord.uSeq);
}

bool CDspDataProcess::SetKaKouImage1x2( const RECORD_PLATE_DSP_MATCH &plate )
{
	if(plate.pImg == NULL )
	{
		return false;
	}

	bool bRet = true;	

	//合成图片清空
	cvSet(m_imgComposeNoMatch, cvScalar( 0,0, 0 ));
	IplImage *pImgDeal = NULL;//当前处理图片

	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;
	rectPic.width = plate.pImg->width;
	rectPic.height = plate.pImg->height - m_nExtentHeight;

	CvRect rtSrc;//取图区域,当前关注区域
	CvRect rtDst;//合成大图中目的写入区域
	rtDst.x = 0;
	rtDst.y = 0;
	rtDst.width = plate.pImg->width;
	rtDst.height = plate.pImg->height - m_nExtentHeight;

	//LogNormal("rectPic:%d,%d,%d,%d ", rectPic.x, rectPic.y, rectPic.width, rectPic.height); //0,0,2448,2048
	//LogNormal("foundRecord:%d,%d", foundRecord.pImg->width, foundRecord.pImg->height); //2448,2108

	bool bTeXie = false;

	int nRow = 1;
	int nCol = 2;
	for(int i = 0; i< nRow; i++)
	{
		for(int j = 0; j< nCol; j++)
		{
			rtDst.x = j * rectPic.width;
			rtDst.y = i * rectPic.height + i*m_nExtentHeight;
			rtDst.width = rectPic.width;
			rtDst.height = rectPic.height;

			rtSrc.x = 0;
			rtSrc.y = 0;
			rtSrc.width = rectPic.width;
			rtSrc.height = rectPic.height;//
			//bTeXie = false;
			if(m_nWordPos == 1)//黑边在上
			{
				rtDst.y += m_nExtentHeight;
			}

			if(0 == i)
			{
				if(0 == j)			//第一张前牌卡口
				{
					pImgDeal = plate.pImg;					
				}
				else if (1 == j)	//第二张前牌特写
				{
					bTeXie = true;
					pImgDeal = plate.pImg;
					GetCarRectFromPlate(plate.dspRecord, rtSrc);
				}
			}
			//bool bDeal = false;
			if(!bTeXie)
			{
				rtSrc.x = 0;
				rtSrc.y = 0;
				rtSrc.width = rectPic.width;
				//rtSrc.height = rectPic.height - m_nExtentHeight;
				rtSrc.height = rectPic.height;

				//bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeNoMatch, rtDst);
			}
			else
			{
				if(m_nWordPos == 1)//黑边在上
				{
					rtDst.y -= m_nExtentHeight;					
				}

				//bDeal = DealComposeImgTeXie(pImgDeal, rtSrc, m_imgComposeNoMatch, rtDst);
			}

			//处理一张图片
			bool bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeNoMatch, rtDst);
			//LogNormal("[%s]:bDeal = %d,pImgDeal = %x,plate=%s\n",__FUNCTION__,bDeal,pImgDeal,foundRecord.dspRecord.chText);
			if(pImgDeal)
			{
				pImgDeal = NULL;
			}

			if(!bDeal)
			{
				bRet = false;

				break;//处理出错,中断处理
			}

			//SaveImgTest(m_imgComposeResult);
		}//End of for j		
	}//End of for i

	if(pImgDeal)
	{
		pImgDeal = NULL;
	}

	return bRet;
}

void CDspDataProcess::SetKaKouText1x2( IplImage * pImage, PLATEPOSITION* pTimeStamp, const RECORD_PLATE_DSP_MATCH &plate )
{
////#ifdef DEBUG_LIUYANG
//	LogNormal("SetKaKouText1x2 pImage:%x uKey:%lld ", pImage, plate.uKeyArray[0]);
////#endif

	if(pImage == NULL)
	{
		return;
	}

	int nRow = 1;
	int nCol = 2;

	//文本初始化
	int nFontSize = g_PicFormatInfo.nFontSize;

	CvxText cvText;
	cvText.Init(nFontSize);

	int nWidth = 0;
	int nHeight = 0;

	wchar_t wchOut[255] = {'\0'};	
	std::string strText = "";

	std::string strDeviceId = "";
	std::string strPlace = "";
	int64_t ts = 0;
	for(int i=0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			//叠字位置
			//GetTextPos(nWidth, nHeight, i, j);

			strDeviceId = std::string(plate.szDeviceID);
			strPlace = std::string(plate.dspRecord.chPlace);

			ts = 0;
			//取得时间戳
			if(i==0 && j==0)
			{
				ts = pTimeStamp[0].ts;
			}			
			else if(i==0 && j==1)
			{
				ts = pTimeStamp[0].ts;
				strDeviceId = std::string(plate.szDeviceID);
				strPlace = std::string(plate.dspRecord.chPlace);
			}			
			else{}

			GetKaKouText(pImage,plate.dspRecord,i,j,ts,strDeviceId,strPlace);
		}
	}

	cvText.UnInit();
}

void CDspDataProcess::OutPutVtsNoMatch( RECORD_PLATE_DSP_MATCH &record_plate )
{
	//LogNormal("OutPutVtsNoMatch id:%d, uKey:%lld", \
	//	record_plate.dspRecord.uChannelID, record_plate.uKeyArray[0]);
	UINT32 uWidth = record_plate.dspRecord.uPicWidth;
	UINT32 uHeight = record_plate.dspRecord.uPicHeight;

	if (NULL == m_imgComposeVtsNoMatch)
	{
		LogNormal("DspDataProcess uWidth=%d,uHeight=%d\n",uWidth,uHeight);
		Init(record_plate.dspRecord.uChannelID,uWidth,uHeight,uWidth,uHeight);
	}	

	//解码图片
	bool bSetA = SetVtsImgMatch(record_plate, true);  //违章
	
	if(!bSetA)
	{
		LogNormal("bSetA:%d , OutPutVtsNoMatch err!", bSetA);
		return;
	}

	//红绿灯增强
#ifdef REDADJUST
	RedLightAdjustMatch(record_plate);
#endif

	ViolationInfo infoViolation;

	//PLATEPOSITION* pTimeStamp;
	PLATEPOSITION TimeStamp[4];
	RECORD_PLATE plate;
	SetVtsInfoNoMatch(record_plate, plate, infoViolation, TimeStamp);

	bool bIsReSize = true;

	//合成违章图片
	bool bSetVtsImg = SetVtsImageNoMatch2x2(record_plate,bIsReSize);
	if(!bSetVtsImg)
	{
		LogNormal("SetVtsImageNoMatch2x2 error!");
		return;
	}

	//获取录像以及图片路径
	pthread_mutex_lock(&g_Id_Mutex);
	//获取图片路径
	std::string strPicPath;
	int nSaveRet = GetVtsPicSavePath(plate, 2,strPicPath);
	pthread_mutex_unlock(&g_Id_Mutex);

	//压缩4合一图片
	IplImage* pSrcImg = m_imgComposeVtsNoMatch;
	IplImage* pImgResize = NULL;
	if(g_PicFormatInfo.nResizeScale < 100 && g_PicFormatInfo.nResizeScale > 0)
	{
		pImgResize = cvCreateImage(cvSize(m_imgComposeVtsNoMatch->width*g_PicFormatInfo.nResizeScale/100.0,m_imgComposeVtsNoMatch->height*g_PicFormatInfo.nResizeScale/100.0),8,3);
		cvResize(m_imgComposeVtsNoMatch,pImgResize);
		pSrcImg = pImgResize;
	}

	//合成违章叠字
	SetVtsTextNoMatch2x2(pSrcImg, plate, infoViolation, TimeStamp, record_plate,bIsReSize);

	//存储违章图像
	SaveComposeVtsImageNoMatch(plate,pSrcImg);

	if(pImgResize != NULL)
	{
		cvReleaseImage(&pImgResize);
		pImgResize = NULL;
	}

	//删除已经存在的记录
	g_skpDB.DeleteOldRecord(strPicPath,false,false);

	//保存记录
	if(nSaveRet>0)
	{
		//g_skpDB.SavePlate(m_nChannelId,plate,0);
		g_skpDB.SavePlate(record_plate.dspRecord.uChannelID,plate,0);
	}

	//将车牌信息送客户端
	//车牌号码
	//memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	//LogNormal("--SendResult-plate.uChannelID=%d\n", plate.uChannelID);
	SendResult(plate,infoViolation.carInfo.uSeq);
}

void CDspDataProcess::SetVtsInfoNoMatch( 
	const RECORD_PLATE_DSP_MATCH &record_plate, 
	RECORD_PLATE& plate, 
	ViolationInfo &infoViolation, 
	PLATEPOSITION* pTimeStamp )
{
	CarInfo    carnums;
	SetCarInfo(record_plate, carnums);
	
	//电警数据
	{
		memcpy(&infoViolation.carInfo,&carnums,sizeof(CarInfo));
		infoViolation.evtType = (VIO_EVENT_TYPE)record_plate.dspRecord.uViolationType;//违章类型
		infoViolation.nChannel = record_plate.dspRecord.uRoadWayID;//车道编号
		infoViolation.nPicCount = 2;//图片数量
	}

	memcpy((char*)(&plate),(char*)(&record_plate.dspRecord),sizeof(RECORD_PLATE));
	plate.uCarColor1 = record_plate.dspRecord.uCarColor1;
	//plate.uCarColor2 = foundRecord.dspRecord.uCarColor1;

	//违章类型-TODO
	//plate.uViolationType = DETECT_MATCH_PLATE; //报前后牌匹配
	plate.uViolationType = infoViolation.evtType;//

	std::string strCarNum = infoViolation.carInfo.strCarNum;
	
//#ifdef MATCH_LIU_YANG_DEBUG
//	strCarNum = foundRecord.dspRecord.chText;
//#endif

	//判断是否有车牌的车
	bool bCarNum = true;
	if( (infoViolation.carInfo.strCarNum[0] == '*') && (infoViolation.carInfo.strCarNum[6] == '*') )
	{
		bCarNum = false;
	}

	if(bCarNum)
	{
		//车牌号码转换
		CarNumConvert(strCarNum,infoViolation.carInfo.wj);
	}

	//车牌号码
	memcpy(plate.chText,strCarNum.c_str(),strCarNum.size());
	int nPos = -1;
	string nStrNum = plate.chText;
	nPos = nStrNum.find("*");

	//车辆类型
	plate.uType =  infoViolation.carInfo.vehicle_type;
	//LogNormal("SetVtsInfo 11 uType:%d ", plate.uType);
	CarTypeConvert(plate);
	//LogNormal("SetVtsInfo 22 uType:%d ", plate.uType);

	//车型细分
	if(m_nDetectKind&DETECT_TRUCK)
		plate.uTypeDetail = GetTypeDetail(infoViolation.carInfo.subVehicleType);

	//车辆类型转换
	CarTypeConvert(plate);

	int nPicCount = infoViolation.nPicCount;//图片数量

	if(nPicCount > 2)
	{
		nPicCount = 2;
	}

	if(1 == g_PicFormatInfo.nSmallViolationPic)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0];
		infoViolation.frameSeqs[0] = infoViolation.frameSeqs[1];
		infoViolation.frameSeqs[1] = frameSeqs;
	}

	//PLATEPOSITION  TimeStamp[6];
	PLATEPOSITION  SignalTimeStamp;

//#ifdef MATCH_LIU_YANG
	for(int i=0; i<3; i++)
	{
		pTimeStamp[i].ts = record_plate.pPlatePos[i+1].ts;
		//LogNormal("ts:%d, %lld", i, pTimeStamp[i].ts);
	}

	//pTimeStamp[3].ts = foundRecord.pPlatePos[0].ts;
	//LogNormal("ts:3, %lld", pTimeStamp[3].ts);
//#endif

	if(nPicCount>=1)
	{
		UINT32 frameSeqs = infoViolation.frameSeqs[0] - infoViolation.dis[0];
		SignalTimeStamp.x = infoViolation.carInfo.ix;
		SignalTimeStamp.y = infoViolation.carInfo.iy;
		SignalTimeStamp.width = infoViolation.carInfo.iwidth;
		SignalTimeStamp.height = infoViolation.carInfo.iheight;
		SignalTimeStamp.nType = plate.uType;
		SignalTimeStamp.IsCarnum = bCarNum;

		bool bRet = false;
		//获取违章检测结果图像
		//车牌世界坐标
		plate.uLongitude = (UINT32)(infoViolation.carInfo.wx*10000*100);
		plate.uLatitude = (UINT32)(infoViolation.carInfo.wy*10000*100);
		//相机ID
		plate.uChannelID = infoViolation.carInfo.id;

		//发生位置(在当前图片上的)
		plate.uPosLeft  = infoViolation.carInfo.ix;
		plate.uPosTop   = infoViolation.carInfo.iy;
		plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
		plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);

		//LogNormal(":%s ,%d,%d,%d,%d record_plate", \
		//	record_plate.dspRecord.chText, record_plate.dspRecord.uPosLeft, record_plate.dspRecord.uPosTop, record_plate.dspRecord.uPosRight, record_plate.dspRecord.uPosBottom);
		//LogNormal(":%s ,%d,%d,%d,%d foundRecord", \
		//	foundRecord.dspRecord.chText, foundRecord.dspRecord.uPosLeft, foundRecord.dspRecord.uPosTop, foundRecord.dspRecord.uPosRight, foundRecord.dspRecord.uPosBottom);

		Picture_Key Pic_Key;
		Pic_Key.uSeq = infoViolation.carInfo.uSeq;
		Pic_Key.uCameraId = infoViolation.carInfo.id;	

		//printf("-----1111111---GetVtsImageMatch----\n");
		//SaveImgTest(record_plate.pImg);		
		//printf("-----22222222---GetVtsImageMatch----\n");
	}

	//经过时间(秒)
	plate.uTime = infoViolation.carInfo.uTimestamp;
	//毫秒
	plate.uMiTime = (infoViolation.carInfo.ts/1000)%1000;
	plate.uSignalTime = record_plate.dspRecord.uSignalTime;
	//地点
	//memcpy(plate.chPlace,m_strLocation.c_str(),m_strLocation.size());
	/*
	//行驶方向
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
	plate.uDirection = (m_nDirection % 4 + 1);
	*/

	//LogNormal("m_nDirection:%d", m_nDirection);
	//LogNormal("plate.uDirection:%d ", plate.uDirection);
	//LogNormal("infoViolation.carInfo.nDirection:%d", infoViolation.carInfo.nDirection);
	//	LogNormal("---plate.uDirection=%d=m_nDirection=%d\n", plate.uDirection,m_nDirection);

	//车牌结构
	plate.uPlateType = infoViolation.carInfo.carnumrow;
	//车牌颜色
	plate.uColor = infoViolation.carInfo.color;
	//车道编号
	plate.uRoadWayID = infoViolation.nChannel;
	//车速
	double dSpeed =   sqrt(infoViolation.carInfo.vx*infoViolation.carInfo.vx+infoViolation.carInfo.vy*infoViolation.carInfo.vy);
	plate.uSpeed = (UINT32)(dSpeed+0.5);
	//发生位置(在当前图片上的)
	plate.uPosLeft  = infoViolation.carInfo.ix;
	plate.uPosTop   = infoViolation.carInfo.iy;
	plate.uPosRight = infoViolation.carInfo.ix+infoViolation.carInfo.iwidth;
	plate.uPosBottom  = (infoViolation.carInfo.iy+infoViolation.carInfo.iheight);
	plate.uPicWidth = m_imgComposeVtsNoMatch->width;
	plate.uPicHeight = m_imgComposeVtsNoMatch->height;
}

bool CDspDataProcess::SetVtsImageNoMatch2x2( const RECORD_PLATE_DSP_MATCH &record_plate,const bool bIsReSize )
{
	if(record_plate.pImgArray[0] == NULL)
	{
		return false;
	}

	bool bRet = true;	

	int nExtentHeight = m_nExtentHeight;
	if (bIsReSize) //如果压缩，则取重新计算的后的扩展区域 压缩前扩展区域高度 =  实际扩展区域高度 / 压缩比例  ，实际扩展区域高度 = 字体大小 *2 +10
	{
		if (g_PicFormatInfo.nResizeScale >= 60 && g_PicFormatInfo.nResizeScale < 100  )
		{
			nExtentHeight = (g_PicFormatInfo.nFontSize * 4 / 5 * 2 + 10)  * 100 / g_PicFormatInfo.nResizeScale;
		}
		else if (g_PicFormatInfo.nResizeScale >= 5 && g_PicFormatInfo.nResizeScale < 60  )
		{
			nExtentHeight = (g_PicFormatInfo.nFontSize * 3 / 5  * 2 + 10) * 100  / g_PicFormatInfo.nResizeScale;
		}
	}
	//合成图片清空
	cvSet(m_imgComposeVtsNoMatch, cvScalar( 0,0, 0 ));
	IplImage *pImgDeal = NULL;//当前处理图片
	CvRect rectPic;
	rectPic.x = 0;
	rectPic.y = 0;
	rectPic.width = record_plate.pImgArray[0]->width;
	rectPic.height = record_plate.pImgArray[0]->height - nExtentHeight;  

	CvRect rtSrc;//取图区域,当前关注区域
	CvRect rtDst;//合成大图中目的写入区域
	rtDst.x = 0;
	rtDst.y = 0;
	rtDst.width = record_plate.pImgArray[0]->width;
	rtDst.height = record_plate.pImgArray[0]->height - nExtentHeight;

	//LogNormal("rectPic:%d,%d,%d,%d ", rectPic.x, rectPic.y, rectPic.width, rectPic.height);
	//LogNormal("foundRecord:%d,%d", foundRecord.pImg->width, foundRecord.pImg->height);

	bool bTeXie = false;
	int nRow = 2;
	int nCol = 2;
	for(int i = 0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			rtDst.x = j * rectPic.width;
			rtDst.y = i * rectPic.height + i*nExtentHeight;
			rtDst.width = rectPic.width;
			rtDst.height = rectPic.height;

			rtSrc.x = 0;
			rtSrc.y = 0;
			rtSrc.width = rectPic.width;
			rtSrc.height = rectPic.height;//
			//bTeXie = false;
			if(m_nWordPos == 1)//黑边在上
			{
				rtDst.y += nExtentHeight;
			}

			if(0 == i)
			{
				if(0 == j)   //第一张卡口特写
				{
					bTeXie = true;
					//LogNormal("record_plate.pImg2*2 = %p\n",record_plate.pImg);
					/*if(record_plate.pImg != NULL)
					{
						pImgDeal = record_plate.pImg;
					}
					else
					{
						if (DETECT_RESULT_RETROGRADE_MOTION == record_plate.dspRecord.uViolationType)//逆行
						{
							pImgDeal = record_plate.pImgArray[2];
						}
						else
						{
							pImgDeal = record_plate.pImgArray[0];
						}
					}*/
					if (record_plate.pImg != NULL)
					{
						pImgDeal = record_plate.pImg;
					}
					else
					{
						pImgDeal = record_plate.pImgArray[0];
					}
       				//GetCarRect(record_plate.dspRecord, rtSrc);
					GetCarRectFromPlate(record_plate.dspRecord, rtSrc);
				}
				else if(1 == j)
				{
					pImgDeal = record_plate.pImgArray[0];
				}
			}
			else if(1 == i)
			{
				if(0 == j)
				{
					pImgDeal = record_plate.pImgArray[1];
				}
				else if(1 == j)
				{
					pImgDeal = record_plate.pImgArray[2];
				}

			}
			else
			{
				//error i,j
			}

			if(!bTeXie)
			{
				rtSrc.x = 0;
				rtSrc.y = 0;
				rtSrc.width = rectPic.width;
				//rtSrc.height = rectPic.height - m_nExtentHeight;
				rtSrc.height = rectPic.height;
			}
			else
			{
				if(m_nWordPos == 1)//黑边在上
				{
					rtDst.y -= nExtentHeight;					
				}
				//rtDst.height -= m_nExtentHeight; //不留黑边	
			}

			//处理一张图片
			bool bDeal = false;
			/*if (bTeXie) //如果是特写图,等比例放大
			{
				bDeal = DealComposeImgTeXie(pImgDeal, rtSrc, m_imgComposeVtsNoMatch, rtDst);
			}
			else*/
			{
				 bDeal = DealComposeImg(pImgDeal, rtSrc, m_imgComposeVtsNoMatch, rtDst);
			}
			
			
			if(pImgDeal)
			{
				pImgDeal = NULL;
			}

			if(!bDeal)
			{
				bRet = false;
				break;//处理出错,中断处理
			}

			//SaveImgTest(m_imgComposeResult);
		}//End of for j		
	}//End of for i

	if(pImgDeal)
	{
		pImgDeal = NULL;
	}

	return bRet;
}

void CDspDataProcess::SetVtsTextNoMatch2x2( IplImage * pImage, 
	RECORD_PLATE &plate, 
	const ViolationInfo &vioInfo, 
	PLATEPOSITION* pTimeStamp, 
	const RECORD_PLATE_DSP_MATCH &record_plate,
	const bool bIsReSize)
{
	if(pImage == NULL)
	{
		return;
	}

	int nRow = 2;
	int nCol = 2;

	//文本初始化
	int nFontSize = g_PicFormatInfo.nFontSize;

	//CvxText cvText;
	//cvText.Init(nFontSize);

	int nWidth = 0;
	int nHeight = 0;

	char chOut[255] = {'\0'};
	wchar_t wchOut[255] = {'\0'};	

	std::string strDeviceId = "";
	std::string strPlace = "";
	int64_t ts = 0;
	for(int i=0; i<nRow; i++)
	{
		for(int j=0; j<nCol; j++)
		{
			//叠字位置
			GetTextPos(nWidth, nHeight, i, j);

			strDeviceId = std::string(record_plate.szDeviceID);
			strPlace = std::string(record_plate.dspRecord.chPlace);

			ts = 0;
			//取得时间戳
			if(i==0 && j==0)
			{
				//ts = pTimeStamp[0].ts;
				ts = pTimeStamp[0].ts;
			}			
			else if(i==0 && j==1)
			{
				ts = pTimeStamp[0].ts;
			}
			else if(i==1 && j==0)
			{
				ts = pTimeStamp[1].ts;
			}
			else if(i==1 && j==1)
			{
				ts = pTimeStamp[2].ts;
			}

			//叠字内容
			std::string strText = GetVtsText(pImage,plate, vioInfo, i, j, ts, strDeviceId, strPlace,bIsReSize);

			//转换编码
			//memset(wchOut,0,sizeof(wchOut));
			//UTF8ToUnicode(wchOut,(char*)strText.c_str());

			////叠字
			//cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
			//}
		}

		if (g_nServerType == 13)
		{
			std::string strText = "";
			int nFontSize = m_nExtentHeight/2;
			CvxText cvText;
			cvText.Init(nFontSize);

			//经过时间
			std::string strTime;
			ts = pTimeStamp[0].ts;
			UINT32 uTime = (UINT32)((ts/1000)/1000);
			strTime = GetTime(uTime,0);

			nWidth = 0;
			nHeight = (m_imgSnap->height)*(i+1)- m_nExtentHeight/2;
			
			if(plate.uViolationType == DETECT_RESULT_RED_LIGHT_VIOLATION)
			{
				sprintf(chOut,"%s 车道%5s%02d %s 红灯时间:%d秒",strPlace.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str(),plate.uSignalTime);//
			}
			else
			{
				sprintf(chOut,"%s 车道%5s%02d %s",strPlace.c_str(),g_strDetectorID.c_str(),plate.uRoadWayID,strTime.c_str());
			}
			strText += chOut;
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			//叠加第二行违章代码
			nHeight += m_nExtentHeight/2;
			strText = GetViolationType(plate.uViolationType,1);
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());
			cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			cvText.UnInit();
		}
	}

	//cvText.UnInit();

	return;
}

void CDspDataProcess::SaveComposeVtsImageNoMatch( RECORD_PLATE& plate,IplImage* pImgSave)
{
	std::string strPicPath;
	strPicPath.append(plate.chPicPath,sizeof(plate.chPicPath));
	//大图宽高
	plate.uPicWidth = m_imgComposeVtsNoMatch->width;
	plate.uPicHeight = m_imgComposeVtsNoMatch->height;

	//SaveImgTest(m_imgComposeMatch, plate.chText);
	if(pImgSave != NULL)
	{
		plate.uPicSize = SaveImage(pImgSave,strPicPath,4);
	}
	else
	{
		plate.uPicSize = SaveImage(m_imgComposeVtsNoMatch,strPicPath,4);
	}
	
}

void CDspDataProcess::GetKaKouText(  IplImage * pImage,
	const RECORD_PLATE &plate, 
	const int nRow, const int nCol, 
	const int64_t ts, 
	const std::string &strDeviceId, 
	const std::string &strPlace )
{
	//文本初始化
	int nFontSize = g_PicFormatInfo.nFontSize;

	CvxText cvText;
	cvText.Init(nFontSize);

	wchar_t wchOut[255] = {'\0'};	
	char chOut[255] = {'\0'};
	std::string strText = "";
	int nWidth = 0;
	int nHeight = 0;
	int nExtent = g_PicFormatInfo.nExtentHeight; //文字区域高度
	//叠字位置
	GetTextPos(nWidth, nHeight, nRow, nCol);

	UINT32 uTime = (UINT32)((ts/1000)/1000);
	UINT32 uMiTime = ((UINT32)(ts/1000)%1000);
	std::string strTime = GetTime(uTime);

	memset(chOut,0,sizeof(chOut));
	sprintf(chOut,"设备编号:%s 地点名称: %s 方向: %s 时间: %s:%03d 车道编号:%d ", 
		strDeviceId.c_str(),
		strPlace.c_str(),
		GetDirection(plate.uDirection).c_str(),
		strTime.c_str(),uMiTime,
		plate.uRoadWayID);

	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	printf("add text test 222222,height = %d,nExtent=%d,width=%d,pImg=%x,plate=%s\n",
		nHeight,m_nExtentHeight,nWidth,pImage,plate.chText);//height:2078,60
	cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
	nHeight = nHeight + m_nExtentHeight/2;

	//车辆类型
	std::string strCarType;
	if(plate.uType == SMALL_CAR)
	{
		strCarType = "小型";
	}
	else if(plate.uType == MIDDLE_CAR)
	{
		strCarType = "中型";
	}
	else if(plate.uType == BIG_CAR)
	{
		strCarType = "大型";
	}
	else
	{
		strCarType = "";
	}
	//车身颜色
	std::string strCarColor = GetObjectColor(plate.uCarColor1);
	//车牌颜色
	std::string strPlateColor = GetPlateColor(plate.uColor);

	std::string strCarBrand = "";

#ifdef GLOBALCARLABEL
	CBrandSusection BrandSub;
	UINT32 uCarLabel = plate.uCarBrand + plate.uDetailCarBrand;
	strCarBrand = BrandSub.GetCarLabelText(uCarLabel).c_str();
#else
	if(plate.uCarBrand < 131)
	{
		strCarBrand = g_strCarLabel[plate.uCarBrand];
	}
	else
	{
		//LogNormal("车标类型有误！");
	}
#endif

	sprintf(chOut,"车牌号码：%s 限速:%dkm/h  车速:%dkm/h 车身颜色: %s 车牌颜色:%s 车辆类型:%s 车标：%s", 
		plate.chText,
		plate.uLimitSpeed,
		plate.uSpeed,
		strCarColor.c_str(),
		strPlateColor.c_str(),
		strCarType.c_str(),
		strCarBrand.c_str());

	//转换编码
	memset(wchOut,0,sizeof(wchOut));
	UTF8ToUnicode(wchOut,chOut);
	//叠字
	cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));	
	cvText.UnInit();

}

void CDspDataProcess::PutGJVtsTextOnImage1x3_Elem( IplImage*pImage, 
	RECORD_PLATE &plate, int nIndex, 
	PLATEPOSITION* pTimeStamp, 
	PLATEPOSITION* pSignalTimeStamp, 
	int64_t redLightStartTime )
{
	if(m_nExtentHeight <= 0)
	{
		return;
	}

	CvxText m_cvText;
	m_cvText.Init(g_PicFormatInfo.nFontSize);

	UINT32 uViolationType = plate.uViolationType;

	int timeIndex = nIndex;	

	if(pTimeStamp != NULL)
	{
		if(m_nWordOnPic == 1)//字直接叠加在图上
		{
			//..
		}
		else
		{
			//LogTrace("PutVtsText.log", "222=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d=m_nWordPos=%d", \
			g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode, m_nWordPos);

			wchar_t wchOut[255] = {'\0'};
			char chOut[255] = {'\0'};

			int nWidth = 10;
			int nHeight = 0;

			string strText("");
		
			//LogNormal("******plate.uLongitude = %u,uLatitude = %u\n",plate.uLongitude,plate.uLatitude);
			double Longitude = (double)(plate.uLongitude/1000000) + (double)(plate.uLongitude - (plate.uLongitude/1000000)*1000000)*1.0/(1000000);
			double Latitude = (double)(plate.uLatitude/1000000) + (double)(plate.uLatitude - (plate.uLatitude/1000000)*1000000)*1.0/(1000000);

			sRoadNameInfo RoadInfo;
			bool bRet = GetRoadName(Longitude,Latitude,RoadInfo);
			
			if (bRet)
			{
				plate.uDirection = GetDRIVEDIR(RoadInfo.chDirection); //获取车辆行驶方向
				memcpy(plate.szLoctionID,RoadInfo.chPosNumber,strlen(RoadInfo.chPosNumber));
				memcpy(plate.szKaKouItem,RoadInfo.chKaKouItem,strlen(RoadInfo.chKaKouItem));
				sprintf(chOut,"卡口编号:%s 地点编号:%s 经度:%.6f 纬度:%.6f 路段名:%s 方向:%s 起始点:%s" , \
					RoadInfo.chKaKouItem,RoadInfo.chPosNumber,Longitude,Latitude,\
					RoadInfo.chRoadName,RoadInfo.chDirection,RoadInfo.chStartPos);			
			}
			else
			{
				sprintf(chOut,"卡口编号:%s 地点编号:%s 经度:%.6f 纬度:%.6f 路段名:%s 方向:%s 起始点:%s", \
					" "," ",Longitude,Latitude," "," "," ");
			}
			string strTmp(chOut);
			strText += strTmp;
			
			//获取叠字位置
			nWidth  = nIndex * (m_imgSnap->width) + 10;
			nHeight = m_imgSnap->height;
			if ( m_nWordPos == 1) //黑边在上
			{
				//nHeight = nHeight + m_nExtentHeight / 2;	
				nHeight = m_nExtentHeight / 2;
			}
			else
			{
				nHeight = nHeight - m_nExtentHeight / 2;
			}
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));
			
			memset(chOut,0,sizeof(chOut));
			strText = "";
			if (bRet)
			{
				sprintf(chOut,"终止点:%s",RoadInfo.chEndPos);
				string strTmp(chOut);
				strText += strTmp;
			}
			else
			{
				sprintf(chOut,"终止点:%s","");
				string strTmp(chOut);
				strText += strTmp;
			}

			//车牌号码
			if(g_PicFormatInfo.nCarNum == 1/* && 1 != g_PicFormatInfo.nSmallViolationPic*/)
			{
				std::string strCarNum(plate.chText);
				sprintf(chOut,"车牌号码:%s ",strCarNum.c_str());
				string strTmp(chOut);
				strText += strTmp;
			}				

			//违法时间
			{
				std::string strTime;
				strTime = GetTime((pTimeStamp+timeIndex)->uTimestamp,0);			
				sprintf(chOut,"违法时间:%s:%03d ",strTime.c_str(),(((pTimeStamp+timeIndex)->ts)/1000)%1000);			        
				string strTmp(chOut);
				strText += strTmp;
			}                    

			if(g_PicFormatInfo.nViolationType == 1)
			{
				//违章行为
				if(plate.uViolationType != 0)
				{
					std::string strViolationType = GetViolationType(plate.uViolationType,1);					
					sprintf(chOut,"违章行为:%s",strViolationType.c_str());
					string strTmp(chOut);
					strText += strTmp;
				}
			}

			{					
				int nRandCode = g_RoadImcData.GetRandCode();
				sprintf(chOut,"相机编号:%s 防伪码:%d",plate.szCameraCode,nRandCode);
				string strTmp(chOut);
				strText += strTmp;

			}
			//设置叠字位置
			nHeight = nHeight + m_nExtentHeight / 2;
			memset(wchOut,0,sizeof(wchOut));
			UTF8ToUnicode(wchOut,(char*)strText.c_str());
			m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(255,255,255));

			//特写图片叠字
			if( 0 == nIndex)
			{
			//	/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)			
				strText.clear();

				//设置叠字位置
				{
					nWidth = 20 + (m_imgSnap->width)*3;
					nHeight = 20 + m_nExtentHeight + m_nExtentHeight/2;

					if(0 == m_nWordPos)//down
					{
						nHeight = (pImage->height - m_nExtentHeight - 20);
						//LogNormal("22 down:w:%d, h:%d", nWidth, nHeight);
					}
				}

				//时间
				{
					std::string strTime;
					strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);			
					sprintf(chOut,"%s:%03d  ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);			        
					string strTmp(chOut);
					strText += strTmp;
				}   

				//写入图片
				{
					//LogTrace("PutVtsText.log", "=33===nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);
					memset(wchOut,0,sizeof(wchOut));
					UTF8ToUnicode(wchOut,(char*)strText.c_str());
					m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
				}
				/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)
			}
			

			/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)			
			strText.clear();

			//设置叠字位置
			{
				nWidth = 20 + (m_imgSnap->width)*nIndex;
				nHeight = 20 + m_nExtentHeight + m_nExtentHeight/2;

				if(0 == m_nWordPos)//down
				{
					nHeight = (pImage->height - m_nExtentHeight - 20);
					//LogNormal("33 down:w:%d, h:%d", nWidth, nHeight);
				}
			}

			//时间
			{
				std::string strTime;
				strTime = GetTime((pTimeStamp+nIndex)->uTimestamp,0);			
				sprintf(chOut,"%s:%03d  ",strTime.c_str(),(((pTimeStamp+nIndex)->ts)/1000)%1000);			        
				string strTmp(chOut);
				strText += strTmp;
			}   

#ifdef VTS_TEST_SEQ
			//记录帧号
			sprintf(chOut,"Seq:[%d]", \
				((pTimeStamp+nIndex)->uSeq));

			string strTmpTest(chOut);
			strText += strTmpTest;
#endif

			//写入图片
			{
				//LogTrace("PutVtsText.log", "=44===nWidth=%d=nHeight=%d==nIndex=%d==", nWidth, nHeight, nIndex);
				memset(wchOut,0,sizeof(wchOut));
				UTF8ToUnicode(wchOut,(char*)strText.c_str());
				m_cvText.putText(pImage, wchOut, cvPoint(nWidth, nHeight), CV_RGB(0,255,255));
			}
			/////////////////////////////////////////////第二行--单张图叠加时间戳(黄色字体)			
		}
	}
	else //空白区域
	{
		//LogTrace("PutVtsText.log", "333=g_PicFormatInfo.nSmallViolationPic=%d==g_nVtsPicMode=%d", \
		g_PicFormatInfo.nSmallViolationPic, g_nVtsPicMode);
	}

	m_cvText.UnInit();
}


//核查记录可用状态
bool CDspDataProcess::CheckPlateMatch(const RECORD_PLATE_DSP_MATCH &plateMatch, const int nIndex)
{
	bool bRet = false;

	if(plateMatch.bKeyStateArray[nIndex])
	{
		bRet = g_skpChannelCenter.CheckImgKeyState(plateMatch.dspRecord.uChannelID, plateMatch.uKeyArray[nIndex]);

		if(!bRet)
		{
			LogNormal("1id:%d recId:%d nIndex:%d uKey:%d State:%d car:%s CheckPlateMatch false1!", \
				m_nChannelId, plateMatch.dspRecord.uChannelID, nIndex, plateMatch.uKeyArray[nIndex], plateMatch.bKeyStateArray[nIndex], plateMatch.dspRecord.chText);
		}
	}
	else
	{
		LogNormal("2id:%d recId:%d nIndex:%d uKey:%d State:%d car:%s CheckPlateMatch false2!", \
			m_nChannelId, plateMatch.dspRecord.uChannelID, nIndex, plateMatch.uKeyArray[nIndex], plateMatch.bKeyStateArray[nIndex], plateMatch.dspRecord.chText);
	}

	return bRet;
}

//更新通道记录标记
bool CDspDataProcess::UpdateImgKeyByChannelID(const int nChannelId, const UINT64 &uKey, const int &bState)
{
	//更新uKey对应记录,为未使用状态
	g_skpChannelCenter.UpdateImgKeyByChannelID(nChannelId, uKey, 0);
}

void * CDspDataProcess::DoPopMatchData(void* lpParam)
{
	CDspDataProcess *pThis = (CDspDataProcess *)lpParam;

	if (pThis != NULL)
	{
		while(1)
		{
			MatchPlate matchPlate;
			if (g_matchPlateFortianjin.PopMatchData(matchPlate))
			{
				sMatchPlateData *pMatchData = new sMatchPlateData;
				pMatchData->param = lpParam;
				pMatchData->A = matchPlate.A;
				pMatchData->B = matchPlate.B;
				//pool_add_worker(DoDealPlate,(void*)pMatchData);
				DoDealPlate((void*)pMatchData);

			}
			RECORD_PLATE_DSP_MATCH plate;
			if (g_MvFBMatch2.PopPlateNoMatch(plate))			
			{
				//LogNormal("PopPlateNoMatch [%s]\n",plate.dspRecord.chText);
				sNoMatchPlateData *pNoMatchData = new sNoMatchPlateData;
				pNoMatchData->param = lpParam;
				pNoMatchData->plate = plate;
				//pool_add_worker(DoDealNoMatchPlate,(void *)pNoMatchData);
				DoDealNoMatchPlate((void *)pNoMatchData);
			}

			usleep(10000);
		}
	}
	return NULL;
}

void * CDspDataProcess::DoDealPlate( void* lpParam )
{
	sMatchPlateData *pMatchData = (sMatchPlateData*)lpParam;
	if (pMatchData == NULL)
	{
		return NULL;
	}

	CDspDataProcess *pThis = (CDspDataProcess *)pMatchData->param;
	RECORD_PLATE_DSP_MATCH A = pMatchData->A;
	RECORD_PLATE_DSP_MATCH B = pMatchData->B;
	delete pMatchData;
	pMatchData = NULL;

	if (pThis != NULL)
	{
		RECORD_PLATE plate;
		{
			if (0 == A.dspRecord.uViolationType) //卡口1*3
			{
				if(3 == g_nPicMode)
				pThis->OutPutVtsMatch1x3(A,B,plate);
			}
			else
			{
				if (1 == g_nVtsPicMode)  //违章2*2
				{
					pThis->OutPutVtsMatch2x2(A,B,plate);
				}
				else if(2 == g_nVtsPicMode) //违章3*2
				{
					pThis->OutPutVtsMatch2x3(A,B,plate);   
				}			
			}	
		}
	}
	return NULL;
}

void * CDspDataProcess::DoDealNoMatchPlate( void* lpParam )
{

	sNoMatchPlateData *pNoMatchData = (sNoMatchPlateData*)lpParam;

	if (pNoMatchData == NULL)
	{
		return NULL;
	}
	CDspDataProcess *pThis = (CDspDataProcess *)pNoMatchData->param;
	RECORD_PLATE_DSP_MATCH plate = pNoMatchData->plate;

	delete pNoMatchData;
	pNoMatchData = NULL;
	if ( pThis != NULL )
	{
		if (0 == plate.dspRecord.uViolationType) //卡口
		{
			if(3 == g_nPicMode)
			pThis->OutPutNoMatch(plate);
		}
		else
		{
			pThis->OutPutVtsNoMatch(plate);
		}	
	}
	return NULL;
}

int CDspDataProcess::DoPopData()
{
	pthread_t m_hId = 0;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, 1024*1024);   // 设置栈空间为1M
	if(pthread_create(&m_hId, &attr, DoPopMatchData, this) != 0)
	{
		printf("创建接收对比结果线程失败。");
		return -1;
	}
	pthread_attr_destroy(&attr);

	return 0;
}


#endif
