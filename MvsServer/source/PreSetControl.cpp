// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "PreSetControl.h"
#include "Common.h"
#include "CommonHeader.h"
#include "XmlParaUtil.h"

//构造
CPreSetControl::CPreSetControl()
{
    m_nChannelID = 0;
    m_nPreSetID = 0;
    m_nMonitorID = 0;
    m_nCameraId = 0;
    m_nCameraType = 0;
    m_bGetScaleObj = false;
    m_uLocalPreSetTime = 0;
}

//析构
CPreSetControl::~CPreSetControl()
{
}

//初始化
bool CPreSetControl::Init(int nChannelID)
{
    m_nChannelID = nChannelID;

    m_nPreSetID = g_skpDB.GetPreSet(m_nChannelID);

    m_listPreSetInfo.clear();

    printf("CPreSetControl::Init m_nChannelID=%d,m_nPreSetID=%d\n",m_nChannelID,m_nPreSetID);

    CXmlParaUtil xml;
    return xml.LoadPreSetInfo(m_nChannelID,m_nPreSetID,m_listPreSetInfo);
}

//释放
bool CPreSetControl::UnInit()
{
    m_listPreSetInfo.clear();
    return true;
}


//调用远景预置位
void CPreSetControl::GoToRemotePreSet(UINT32 uCurrentTime)
{
    if(m_bGetScaleObj)
    {
        if(m_uLocalPreSetTime > 0)
        {
            if(uCurrentTime >= m_uLocalPreSetTime + 20)//在近景预置位停留一段时间后再回归远景预置位
            {
                CAMERA_CONFIG cfg;
                cfg.nAddress = m_nCameraId;
                cfg.nIndex = GOTO_PRESET;
                cfg.fValue = m_nPreSetID;

				if(g_nPreSetMode == 0)
				{
					if(m_nCameraType == SANYO_CAMERA)
					{
						//g_SanYoCamera.ManualControl(cfg);
						cfg.uType = 1;
						g_skpChannelCenter.CameraControl(m_nChannelID,cfg);
					}
					else
					{
					   if(g_ytControlSetting.nProtocalType == 0)//虚拟键盘码协议
						g_VisKeyBoardControl.SendKeyData(cfg, m_nMonitorID, 1);
						else //pelco协议
						g_VisSerialCommunication.SendData(cfg);
					}
				}
                m_bGetScaleObj = false;
                m_uLocalPreSetTime = 0;
				sleep(5); // add by taojun 防止出现违章逆行的情况
                LogNormal("调用远景预置位%d,%d,%d",m_nCameraId,m_nMonitorID,m_nPreSetID);
            }
        }
    }
}

//调用近景预置位
void CPreSetControl::GoToLocalPreSet(CvRect rtPos,UINT32 uLocalPreSetTime)
{
    int nLocalPreSet = GetLocalPreSet(rtPos);

    if(nLocalPreSet > 0)
    {
        CAMERA_CONFIG cfg;
        cfg.nAddress = m_nCameraId;
        cfg.nIndex = GOTO_PRESET;
        cfg.fValue = nLocalPreSet;
		LogNormal("PreSetMode1=%d",g_nPreSetMode);
		
		if(g_nPreSetMode == 0)
		{
			LogNormal("PreSetMode2=%d",g_nPreSetMode);
			if(m_nCameraType == SANYO_CAMERA)
			{
				//g_SanYoCamera.ManualControl(cfg);
				cfg.uType = 1;
				g_skpChannelCenter.CameraControl(m_nChannelID,cfg);
			}
			else
			{
				if(g_ytControlSetting.nProtocalType == 0)//虚拟键盘码协议
				g_VisKeyBoardControl.SendKeyData(cfg, m_nMonitorID, 1);
				else //pelco协议
				g_VisSerialCommunication.SendData(cfg);
			}
		}

        //记住调用近景预置位时刻
        m_uLocalPreSetTime = uLocalPreSetTime;
        m_bGetScaleObj = true;
        LogNormal("调用近景预置位%d,%d,%d",m_nCameraId,m_nMonitorID,nLocalPreSet);
    }
    else
    {
        LogNormal("未找到合适的近景预置位");
    }
}

//根据事件发生位置判断目标属于哪个近景预置位
int CPreSetControl::GetLocalPreSet(CvRect rtPos)
{
    CvPoint lt_pt1, rb_pt1, lt_pt2, rb_pt2;

    lt_pt1.x = rtPos.x;
    lt_pt1.y = rtPos.y;

    rb_pt1.x = rtPos.x + rtPos.width - 1;
    rb_pt1.y = rtPos.y + rtPos.height - 1;
	LogNormal("rtPos.width = %d, rtPos.height = %d \n", rtPos.width, rtPos.height);
     LogNormal("lt_pt1.x=%d,lt_pt1.y=%d,rb_pt1.x=%d,rb_pt1.y=%d\n",lt_pt1.x,lt_pt1.y,rb_pt1.x,rb_pt1.y);
	

    float fMaxPercent = 0;
    float fPercent = 0;

    int nLocalPreSet = 0;

   PreSetInfoList::iterator it_b = m_listPreSetInfo.begin();
   PreSetInfoList::iterator it_e = m_listPreSetInfo.end();

   while(it_b != it_e)
   {
       PreSetInfo info = *it_b;

       Point32fList::iterator it = info.listRegion.begin();

       printf("GetLocalPreSet info.nPreSetID=%d,info.listRegion.size=%d\n",info.nPreSetID,(int)info.listRegion.size());

        int i = 0;
        while(it != info.listRegion.end())
        {
            if(i == 0)
            {
                lt_pt2.x = it->x;
                lt_pt2.y = it->y;
            }
            else if(i == 2)
            {
                rb_pt2.x = it->x;
                rb_pt2.y = it->y;
            }

            //printf("it->x=%f,it->y=%f\n",it->x,it->y);


            it++;
            i++;
        }
        LogNormal("PresetLoc:ptLT(%d,%d),ptRB(%d,%d)\n",lt_pt2.x,lt_pt2.y,rb_pt2.x,rb_pt2.y);

       fPercent = CalOverlapPercent(lt_pt1,rb_pt1,lt_pt2,rb_pt2);

       if(fPercent > fMaxPercent)
       {
            fMaxPercent = fPercent;
            nLocalPreSet = info.nPreSetID;
       }
       LogNormal("fPercent=%f,info.nPreSetID=%d,nLocalPreSet=%d",fPercent,info.nPreSetID,nLocalPreSet);

       it_b++;
   }

   return nLocalPreSet;
}

//计算两个目标的重叠关系
float CPreSetControl::CalOverlapPercent(CvPoint lt_pt1, CvPoint rb_pt1, CvPoint lt_pt2, CvPoint rb_pt2)
{
   // printf("lt_pt1.d=%d,lt_pt1.y=%d,rb_pt1.x=%d,rb_pt1.y=%d,lt_pt2.x=%d,lt_pt2.y=%d,rb_pt2.x=%d,rb_pt2.y=%d\n",lt_pt1.x,lt_pt1.y,rb_pt1.x,rb_pt1.y,lt_pt2.x,lt_pt2.y,rb_pt2.x,rb_pt2.y);

	float fAreai = (rb_pt1.x-lt_pt1.x)*(rb_pt1.y-lt_pt1.y);
	float fAreaj = (rb_pt2.x-lt_pt2.x)*(rb_pt2.y-lt_pt2.y);

	float fOverX = min(rb_pt1.x, rb_pt2.x) - max(lt_pt1.x, lt_pt2.x);
	float fOverY = min(rb_pt1.y, rb_pt2.y) - max(lt_pt1.y, lt_pt2.y);

	float fAreaoverlap = 0;
	float fPercent = 0;

	if( fOverX>0 && fOverY>0  )
	{
		fAreaoverlap = fOverX*fOverY;
		if(fAreaj > 0)
		fPercent = fAreaoverlap/fAreaj;
	}

	printf("fPercent=%f\n",fPercent);

	return fPercent;
}
