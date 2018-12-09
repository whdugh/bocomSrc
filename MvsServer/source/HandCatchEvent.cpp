#include "HandCatchEvent.h"



CHandCatchEvent::CHandCatchEvent(int nCheckTime)
{
	LogNormal("HandCheckTime:%d \n",nCheckTime);
	m_detectTime = 0;
	m_detectTestTime = 0;
#ifdef  HANDCATCHCAREVENT
	m_CStopObjStatusDet.mvInit(nCheckTime);
#endif
	m_tCatchCarEventCB = 0;
	m_pImgSrc =  cvCreateImageHeader(cvSize(1920,1080),8,3);
	m_bDetectTestPic = true;
}
CHandCatchEvent::~CHandCatchEvent()
{
	if(m_pImgSrc)
	{
		cvReleaseImageHeader(&m_pImgSrc);
	}
}
int CHandCatchEvent::SetObjRect(std::string& frame,CvRect rectDraw)
{
	LogNormal("CHandCatchEvent::SetObjRect,x=%d,y=%d,width=%d,height=%d \n",rectDraw.x,rectDraw.y,rectDraw.width,rectDraw.height);
	m_detectTestTime = GetTimeT();
	cvSetData(m_pImgSrc,(char *)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_pImgSrc->widthStep);
	int nCarId =  0;
#ifdef  HANDCATCHCAREVENT
	nCarId = m_CStopObjStatusDet.mvDrawOneRectanle(m_pImgSrc, rectDraw, GetTimeT());
#endif
	return nCarId+1000000;
}

int CHandCatchEvent::DelObjRect(int nCarId)
{
	LogNormal("Del Obj,CarId=%d \n");
#ifdef  HANDCATCHCAREVENT
	m_CStopObjStatusDet.mvDeleteOneRectanle(nCarId);
#endif
	return 0;
}

int CHandCatchEvent::DealFrame(std::string& frame)
{
#ifdef  HANDCATCHCAREVENT
	if((GetTimeT() - m_detectTime ) >= 2 )
	{

		m_detectTime = GetTimeT();

		vector<MvRectObject> vSrcRectObj;
		m_CStopObjStatusDet.mvGetAllRectObjectAtNow(vSrcRectObj);

		if(vSrcRectObj.size() > 0)
		{
			vector<RectObject>  vectRectObj;

			for(vector<MvRectObject>::iterator iter=vSrcRectObj.begin();iter!=vSrcRectObj.end();iter++)
			{
				
				RectObject tObj;

				tObj.nId = (*iter).nId;
				tObj.lpRect.left = (*iter).rct.x/4;
				tObj.lpRect.top  = (*iter).rct.y/4;
				tObj.lpRect.right = (*iter).rct.x/4 + (*iter).rct.width/4;
				tObj.lpRect.bottom = (*iter).rct.y/4 + (*iter).rct.height/4;
				tObj.bGetRoof = (*iter).bGetRoof;
				tObj.bLeave   = (*iter).bLeave;

				printf("RectObj: nId:%d,x=%d,y=%d,width=%d,height=%d,bGetRoof=%d,bLeave =%d \n",tObj.nId,tObj.lpRect.left,tObj.lpRect.top,tObj.lpRect.right,tObj.lpRect.bottom,tObj.bGetRoof,tObj.bLeave);


				vectRectObj.push_back(tObj);

				if(!(*iter).bLeave)
				{
					if((*iter).bGetRoof)
					{
						m_tCatchCarEventCB((*iter).nId+1000000,m_nContext);
					}
				}
			}
			//回调给客户端显示
			SRIP_DETECT_HEADER sDetectHeader = *((SRIP_DETECT_HEADER*)frame.c_str());
			m_tDrawCB(vectRectObj,sDetectHeader,m_nContext);
		}

		cvSetData(m_pImgSrc,(char *)(frame.c_str()+sizeof(SRIP_DETECT_HEADER)),m_pImgSrc->widthStep);
		//检测停止目标的状态（算法内部控制了隔段时间轮询）
		m_CStopObjStatusDet.mvCheckObjectStatus(m_pImgSrc, GetTimeT());
	}
#endif
	return ROADDETECT_OK;
}


int CHandCatchEvent::GetTimeT()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
}


int CHandCatchEvent::SetCatchCarEvent(TCatchCarEventCB tEventCB,int nContext)
{
	printf("nContext = %d \n",nContext);
	m_tCatchCarEventCB = tEventCB;
	m_nContext = nContext;
}


int CHandCatchEvent::SetCatchCarEventDraw(TCatchCarEventDrawCB tDrawCB,int nContext)
{
	m_tDrawCB = tDrawCB;
}



