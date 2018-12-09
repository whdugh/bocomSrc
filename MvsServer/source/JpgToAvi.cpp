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
// JpgToAvi.cpp: implementation of the CJpgToAvi class.
//
//////////////////////////////////////////////////////////////////////

#include "JpgToAvi.h"
#include "Common.h"
#include "CommonHeader.h"
#include "ximage.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJpgToAvi::CJpgToAvi()
{

	m_EncodeFile = NULL;

	//宽
	m_uWidth = 0;
	//高
	m_uHeight = 0;

}

CJpgToAvi::~CJpgToAvi()
{

}
//打开视频文件
bool CJpgToAvi::OpenFile(const char* chFileName)
{
    printf("chFileName=%s\n",chFileName);
	//初始化
	m_EncodeFile = fopen(chFileName,"wb");
	if(m_EncodeFile == NULL) return false;
	//保存录像文件名
	m_strFileName = chFileName;
	m_list.clear();
	m_elist.clear();
	m_PlateCount = 0;

	m_nFrame = 0;
	//先不写头，因为不知道总帧数
	if(g_nAviHeaderEx == 1)
	fseek(m_EncodeFile,sizeof(AVI_Head)+sizeof(AVI_Head_EX),SEEK_SET);
	else
	fseek(m_EncodeFile,sizeof(AVI_Head),SEEK_SET);
	return true;
}
//关闭视频文件
bool CJpgToAvi::CloseFile()
{
    //写入头
    WriteHeader();

	//关闭目标文件
	if(m_EncodeFile)
      fclose(m_EncodeFile);

	m_EncodeFile = NULL;

	printf("======m_strFileName.c_str()=%s\n",m_strFileName.c_str());
	g_skpDB.VideoSaveUpdate(m_strFileName,m_nChannelId);

	return true;
}
//添加一帖数据
bool CJpgToAvi::AddFrame(unsigned char* pBuffer,unsigned int nLength,const std::string& strEvent)
{
    if(m_EncodeFile==NULL)
    {
        return false;
    }
  //帧数加一
  m_nFrame++;

  unsigned char* pBuf = pBuffer+sizeof(SRIP_DETECT_HEADER);
  SRIP_DETECT_HEADER* sDHeader = (SRIP_DETECT_HEADER*)pBuffer;


  SRIP_DETECT_HEADER* sDetectHeader = (SRIP_DETECT_HEADER*)(strEvent.c_str());

  m_nChannelId = sDetectHeader->uChannelID;

  AVI_FrameInfo data;
  data.Length = nLength;
  data.Time  = (unsigned int)sDHeader->uTimestamp;
  data.Time2 = (unsigned int)((sDHeader->uTime64/1000)%1000);

  if(m_nFrame==1)
  {
    if(g_nAviHeaderEx == 1)
	data.Position = sizeof(AVI_Head)+sizeof(AVI_Head_EX);
	else
	data.Position = sizeof(AVI_Head);
  }
  else
  {
    Jpeg_List::iterator it = m_list.end();
    it--;
    data.Position = it->Position + it->Length;
  }
  m_list.push_back(data);


  if(sDHeader->uSeq==sDetectHeader->uSeq)
  {
	//////////////////////////保存事件信息
	int nOffset = sizeof(SRIP_DETECT_HEADER);

	int nSize =  (strEvent.size()-nOffset)/(sizeof(RECORD_EVENT));

	for(int i = 0; i < nSize; i ++)
	{
		RECORD_EVENT event;
		memcpy(&event,strEvent.c_str()+nOffset+i*sizeof(RECORD_EVENT),sizeof(RECORD_EVENT));

		bool bShow;
		memcpy(&bShow,event.chReserved,sizeof(bool));

		if(bShow==0)
		{
			int x	= event.uPosX;
			int y   = event.uPosY;

			AVI_EventInfo eventInfo;
			eventInfo.FramePos = m_nFrame;
			eventInfo.EventCoordX = x;
			eventInfo.EventCoordY = y;
			m_elist.push_back(eventInfo);
		}
	}
	//////////////////////////
  }


  int left = nLength;
  int nbr=0,index=0;
  while ( left> 0)
  {
	 nbr = left > 512 ? 512 : left;

     fwrite(pBuf+index,nbr,1,m_EncodeFile);

	 left -= nbr;
	 index += nbr;
  }

   //写入头
  WriteHeader();
  //
  fseek(m_EncodeFile,data.Position+data.Length,SEEK_SET);
  if(g_nAviHeaderEx == 1)
  m_uPicPos = data.Position+data.Length;

  return true;
}

//写录像头
void CJpgToAvi::WriteHeader()
{
    //最后写入头
	rewind(m_EncodeFile);

	if(NULL == m_EncodeFile)
	{
		return;
	}

	AVI_Head head;
	head.Flag = 0xFFAAFFBB;
	head.FrameCount = m_list.size();
	head.EventCount = m_elist.size();
	if(g_nAviHeaderEx == 1)
	head.PlateCount = m_PlateCount;

    fwrite(&head,sizeof(int),4,m_EncodeFile);

	Jpeg_List::iterator it_b = m_list.begin();
	Jpeg_List::iterator it_e = m_list.end();
	int i = 0;
    while(it_b!=it_e)
	{
	//  memcpy(&(head.FInfo[i]),&(*it_b),sizeof(AVI_FrameInfo));
	//  printf("Time=%u,Time2=%u\n",head.FInfo[i].Time,head.FInfo[i].Time2);
	  fwrite(&(*it_b),sizeof(AVI_FrameInfo),1,m_EncodeFile);
	  i++;
	  it_b++;
	}

	fseek(m_EncodeFile,sizeof(AVI_Head)-16*sizeof(AVI_EventInfo),SEEK_SET);

	Event_List::iterator it = m_elist.begin();
	i = 0;
    while(it!=m_elist.end())
	{
	 //memcpy(&(head.EInfo[i]),&(*it),sizeof(AVI_EventInfo));
	 // printf("FramePos=%d,EventCoordX=%d,EventCoordY=%d\n",head.EInfo[i].FramePos,head.EInfo[i].EventCoordX,head.EInfo[i].EventCoordY);
	  fwrite(&(*it),sizeof(AVI_EventInfo),1,m_EncodeFile);
	  i++;
	  it++;
	}
	printf("======m_list.size()=%d===m_elist.size()=%d,sizeof(AVI_Head)=%d\n",m_list.size(),m_elist.size(),sizeof(AVI_Head));

}

//add ext-header data and big pictures.(zhangyaoyao)
void CJpgToAvi::AddBigPics(const string &strEvents)
{
    //if(g_nAviHeaderEx == 1)
    {
        AVI_Head_EX exHead;
        exHead.Flag = 0xFFCCFFDD;
        memcpy(exHead.chVersion, "v1.0.0.0", sizeof(exHead.chVersion));
        int nSize = strEvents.size()/sizeof(RECORD_EVENT);
        int nCount = 0;

        for(int i = 0; i < nSize; i++)
        {
            RECORD_EVENT event;
            memcpy(&event, strEvents.c_str() + i*sizeof(RECORD_EVENT), sizeof(RECORD_EVENT));

            bool bShow;
            memcpy(&bShow, event.chReserved, sizeof(bool));

            if(!bShow)
            {
                event.Position = m_uPicPos;
                if(event.uPicWidth > 2000)
                {
                    event.uPosX *= 6;
                    event.uPosY *= 6;
                }
                else if(event.uPicWidth > 1000)
                {
                    event.uPosX *= 4;
                    event.uPosY *= 4;
                }
                else if(event.uPicWidth > 500)
                {
                    event.uPosX *= 2;
                    event.uPosY *= 2;
                }
                //exHead.event[i] = event;
                memcpy(&(exHead.event[nCount++]), &event, sizeof(RECORD_EVENT));
                string strPic = event.chPicPath;
                FILE *pPic = fopen(strPic.c_str(), "r");
                if (pPic != NULL)
                {
                    char szBuff[1024] = {0};
                    while (1)
                    {
                        int nRes = fread(szBuff, sizeof(char), 1024, pPic);
                        if (nRes > 0)
                        {
                            m_uPicPos += nRes;
							if(m_EncodeFile)
							{
								fseek(m_EncodeFile, 0, SEEK_END);
								fwrite(szBuff, nRes, 1, m_EncodeFile);
							}
                            if (nRes < 1024)
                            {
								if(pPic)
								{
									fclose(pPic);
									pPic = NULL;
								}
                                break;
                            }
                        }
                        else
                        {
                            fclose(pPic);
                            pPic = NULL;
                            break;
                        }
                    }
                }
            }
        }
		if(m_EncodeFile)
		{
			fseek(m_EncodeFile, sizeof(AVI_Head), SEEK_SET);
			fwrite(&(exHead), sizeof(exHead), 1, m_EncodeFile);
		}
    }
}

//添加一帖数据(定时录象)
bool CJpgToAvi::AddOneFrame(unsigned char* pBuffer,unsigned int nLength,SRIP_DETECT_HEADER* sDHeader)
{
    {
        if(m_EncodeFile==NULL)
        {
            return false;
        }
        printf("======22======AddOneFrame===========\n");
      //帧数加一
      m_nFrame++;

      AVI_FrameInfo data;
      data.Length = nLength;
      data.Time  = (unsigned int)sDHeader->uTimestamp;
      data.Time2 = (unsigned int)((sDHeader->uTime64/1000)%1000);
	  m_nChannelId = sDHeader->uChannelID;

      if(m_nFrame==1)
      {
          if(g_nAviHeaderEx == 1)
        data.Position = sizeof(AVI_Head)+sizeof(AVI_Head_EX);
        else
        data.Position = sizeof(AVI_Head);
      }
      else
      {
        Jpeg_List::iterator it = m_list.end();
        it--;
        data.Position = it->Position + it->Length;
      }
      m_list.push_back(data);


      int left = nLength;
      int nbr=0,index=0;
      while ( left> 0)
      {
         nbr = left > 512 ? 512 : left;

		 if(m_EncodeFile)
		 {
			fwrite(pBuffer+index,nbr,1,m_EncodeFile);
		 }

         left -= nbr;
         index += nbr;
      }

       //写入头
      WriteHeader();
      //
      fseek(m_EncodeFile,data.Position+data.Length,SEEK_SET);
      if(g_nAviHeaderEx == 1)
      m_uPicPos = data.Position+data.Length;
  }

  return true;
}

//添加车牌记录和图片
void CJpgToAvi::AddPlatePics(int nChannel,UINT32 uBeginTime,UINT32 uEndTime)
{
   // if(g_nAviHeaderEx == 1)
    {
        //从数据库中获取车牌记录
        SEARCH_ITEM search_item;
        search_item.uBeginTime = uBeginTime;
        search_item.uEndTime = uEndTime;
        search_item.uChannelId = nChannel;
        search_item.uPage = 1;
        search_item.uConditionType = 0;//0表示查询所有;1表示只查询车牌
        std::string response = g_skpDB.GetCarNum(search_item,SRIP_PAGE_SIZE);
        if(response.size()>sizeof(unsigned int))
        m_PlateCount = (response.size()-sizeof(unsigned int))/sizeof(RECORD_PLATE);
        else
        m_PlateCount = 0;

        AVI_Head_EX exHead;
        exHead.Flag = 0xFFCCFFDD;
        memcpy(exHead.chVersion, "v1.0.0.0", sizeof(exHead.chVersion));
        int nCount = 0;

        for(int i = 0; i < m_PlateCount; i++)
        {
            RECORD_PLATE plate;
            memcpy(&plate,response.c_str()+sizeof(unsigned int)+i*(sizeof(RECORD_PLATE)),sizeof(RECORD_PLATE));

            plate.Position = m_uPicPos;
            memcpy(&(exHead.plate[nCount++]), &plate, sizeof(RECORD_PLATE));
            string strPic = plate.chPicPath;
            FILE *pPic = fopen(strPic.c_str(), "r");
            if (pPic != NULL)
            {
                char szBuff[1024] = {0};
                while (1)
                {
                    int nRes = fread(szBuff, sizeof(char), 1024, pPic);
                    if (nRes > 0)
                    {
                        m_uPicPos += nRes;

						if(m_EncodeFile)
						{
							fseek(m_EncodeFile, 0, SEEK_END);
							fwrite(szBuff, nRes, 1, m_EncodeFile);
						}

						if (nRes < 1024)
						{
							if(pPic)
							{
								fclose(pPic);
								pPic = NULL;
							}							
							break;
						}						
                    }
                    else
                    {
						if(pPic)
						{
							fclose(pPic);
							pPic = NULL;
						}
                        break;
                    }
                }
            }
        }
		if(m_EncodeFile)
		{
			fseek(m_EncodeFile, sizeof(AVI_Head), SEEK_SET);
			fwrite(&(exHead), sizeof(exHead), 1, m_EncodeFile);
		}
    }
}

//rgb->jpg
bool CJpgToAvi::Encode(BYTE* pBuf,int nWidth,int nHeight,int* srcstep,BYTE* pOutBuf)
{
    //ippSetNumThreads(1);
    //printf("CJpgToAvi::Encode=================\n");
    CxImage image;
    //double t = (double)cvGetTickCount();
    bool bRet = image.IppEncode(pBuf,nWidth,nHeight,3,srcstep,pOutBuf,g_PicFormatInfo.nJpgQuality);
    //t = (double)cvGetTickCount() - t;
    //double dt = t/((double)cvGetTickFrequency()*1000.) ;
    //printf( "===========CJpgToAvi::Encode=%d ms\n",(int)dt);
    return bRet;
}
