// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#include "CMDToCamera.h"
#include "SocketUtilCamera.h"
#include "Common.h"
#include "CommonHeader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCMDToCamera::CCMDToCamera()
{
	pthread_mutex_init(&m_sockMutex,NULL);

	m_sockThis = INVALID_SOCKET;
	m_wTargetPort = 0;

	m_pBackDataBig = NULL;
}

CCMDToCamera::~CCMDToCamera()
{
	Close();

	if(m_pBackDataBig != NULL)
	{
		delete [] m_pBackDataBig;
		m_pBackDataBig = NULL;
	}

	pthread_mutex_destroy(&m_sockMutex);
}

void CCMDToCamera::SetTargetAddr(const char *pszIP, unsigned short wPort)
{
    printf("===CCMDToCamera::SetTargetAddr()===pszIP=%s, wPort=%d===\n", pszIP, wPort);
	m_sTargetIP = pszIP;
	m_wTargetPort = wPort;

	m_pBackDataBig = new char[CMD_DATA_BIG_BUF_SIZE];
}

BOOL CCMDToCamera::CreateAndConnect()
{
	if(g_nGongJiaoMode == 1)
	{
		return false;
	}

	//LogTrace("CamTClose11.log", "==111==CreateAndConnect=m_sockThis=%d=m_sTargetIP=%s=\n", \
		m_sockThis, m_sTargetIP.c_str());

	if (m_sockThis != INVALID_SOCKET)
	{
		Close();
	}

	//LogTrace("CamTClose11.log", "==222==CreateAndConnect=m_sockThis=%d==\n", \
		m_sockThis, m_sTargetIP.c_str());

	BOOL bRet = FALSE;
	do
	{
		if (m_sTargetIP.empty())
		{
			break;
		}

		m_sockThis = socket(AF_INET, SOCK_STREAM, 0);

		//LogTrace("CamTClose11.log", "=333==CreateAndConnect=m_sockThis=%d==\n", \
			m_sockThis, m_sTargetIP.c_str());

		if (m_sockThis == INVALID_SOCKET)
		{
			break;
		}

 		//int nTimeOut = 10000;
 		//setsockopt(m_sockThis, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));
 		//setsockopt(m_sockThis, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, sizeof(nTimeOut));

		struct sockaddr_in addrCam;
		addrCam.sin_family = AF_INET;
		addrCam.sin_addr.s_addr = inet_addr(m_sTargetIP.c_str());
		addrCam.sin_port = htons(m_wTargetPort);

		//printf("===CCMDToCamera::CreateAndConnect()===m_sockThis=%d=m_sTargetIP=%s===m_wTargetPort=%d====\n", m_sockThis, m_sTargetIP.c_str(), m_wTargetPort);
		if(m_sockThis < 0)
		{
			LogNormal("=CreateAndConnect=m_sockThis=%d", m_sockThis);
			break;
		}
		else
		{
			if (!ConnectHasTimeout(m_sockThis, &addrCam, 5000))
			{
				printf("=====ConnectHasTimeout====Error!!\n");
				break;
			}
			else
			{
				bRet = TRUE;
			}			
		}
		
	} while(0);

	if (!bRet)
	{
		Close();
	}

	return bRet;
}


void CCMDToCamera::Close()
{
	if (m_sockThis != INVALID_SOCKET)
	{
		//LogTrace("CamTClose11.log", "=before==CCMDToCamera::Close()==m_sockThis=%d==m_sTargetIP=%s=!!\n", \
			m_sockThis, m_sTargetIP.c_str());
#ifdef DEBUG_DSP_CMD
		//LogNormal("IP:%s fd:%d CCMDToCamera::Close() \n", \
			m_sTargetIP.c_str(), m_sockThis);
#endif
		shutdown(m_sockThis, 2);
		close(m_sockThis);
		m_sockThis = INVALID_SOCKET;		
	}
}

BOOL CCMDToCamera::SendCMD(unsigned int dwAttribute, unsigned int dwMethod, const char *pParam, unsigned int dwParamLen)
{
	BOOL bRet = FALSE;
	do
	{
		//if (m_sockThis == INVALID_SOCKET)
		{
			if (!CreateAndConnect()) //短连接
			{
			    printf("===CCMDToCamera::SendCMD===CreateAndConnect==ERROR!!\n");
				break;
			}
		}

		*(unsigned int *)(m_pSendData) = htonl(dwAttribute);
		*(unsigned int *)(m_pSendData + 4) = htonl(dwMethod);
		*(unsigned int *)(m_pSendData + 8) = htonl(dwParamLen);
		if (dwParamLen > 0)
		{
			memcpy(m_pSendData + 12, pParam, dwParamLen);
		}
		if (!SafeSend(m_sockThis, m_pSendData, dwParamLen + 12))
		{
		    printf("====SafeSend====dwParamLen=%d===\n", dwParamLen);
			break;
		}

		unsigned int dwRecvDataLenTemp = 0;
		if (!SafeRecv(m_sockThis, (char*)&dwRecvDataLenTemp, 4))
		{
		    printf("====SafeRecv===dwRecvDataLenTemp=%d===\n", ntohl(dwRecvDataLenTemp));
			break;
		}
		unsigned int dwRecvDataLen = ntohl(dwRecvDataLenTemp);

		if (dwRecvDataLen > CMD_DATA_BUF_SIZE)
		{
		    printf("====dwRecvDataLen > CMD_DATA_BUF_SIZE=====\n");
			break;
		}

		if (!SafeRecv(m_sockThis, m_pBackData, dwRecvDataLen))
		{
		    printf("====SafeRecv===....=\n");
			break;
		}

		bRet = TRUE;
	} while(0);

	if (!bRet)
	{
		LogError("发送命令失败! 命令ID:0x%X, 方法:%d.==\n", dwAttribute, dwMethod);
	}

	Close();

	return bRet;
}

BOOL CCMDToCamera::SendCMD(unsigned int dwAttribute, unsigned int dwMethod)
{
	return SendCMD(dwAttribute, dwMethod, 0, NULL);
}

BOOL CCMDToCamera::SendCMD(unsigned int dwAttribute, unsigned int dwMethod, unsigned int dwParam)
{
	unsigned int dwParamTemp = htonl(dwParam);
	return SendCMD(dwAttribute, dwMethod, (const char*)&dwParamTemp, 4);
}

BOOL CCMDToCamera::SendCMD(unsigned int dwAttribute, unsigned int dwMethod, const char *pParam)
{
	return SendCMD(dwAttribute, dwMethod, pParam, strlen(pParam) + 1);
}

char* CCMDToCamera::GetBackDataPtr()
{
	m_pBackData[CMD_DATA_BUF_SIZE] = '\0';
	return m_pBackData;
}

unsigned int CCMDToCamera::GetBackDataDWORD()
{
	unsigned int dwRet = *(unsigned int*)m_pBackData;
	dwRet = ntohl(dwRet);
	return dwRet;
}

float CCMDToCamera::GetBackDataFloat()
{
	unsigned int dwRet = *(unsigned int*)m_pBackData;
	dwRet = ntohl(dwRet);
	return (float)dwRet;
}

//DSP相机控制
BOOL  CCMDToCamera::SendDspCMD(const DspCmdHeader &header, const char * pParam)
{	
	//加锁
	pthread_mutex_lock(&m_sockMutex);

#ifdef DEBUG_DSP_CMD
	//LogNormal("ip:%s, fd:%d SendDspCMD cmd:%x flag:%d \n", m_sTargetIP.c_str(), m_sockThis, header.aCmdID, header.aReplyFlag);
#endif

    BOOL bRet = false;
    BOOL bRetRecv = false;
    //bool bSend = false;
    char chTemp[256] = {0};
    int nSendTimes = 1;
    int nRecTimes = 3;
    int nMaxSendTimes = 3;

    unsigned int dwParamLen = 0;
    unsigned int tmp;

    if(header.aParamLen[3] == 0x00 && header.aParamLen[2] == 0x00 \
    && header.aParamLen[1] == 0x00 && header.aParamLen[0] == 0x00)
    {
        dwParamLen = 0;
    }
    else
    {
        tmp = (header.aParamLen[3] & 0x000000FF);
        dwParamLen = tmp<<24;
        tmp = (header.aParamLen[2] & 0x000000FF);
        dwParamLen += tmp<<16;
        tmp = (header.aParamLen[1] & 0x000000FF);
        dwParamLen += tmp<<8;
        tmp = (header.aParamLen[0] & 0x000000FF);
        dwParamLen += tmp;
    }

     //*(DWORD*)(m_pSendData) = htonl((char*)header);
    memcpy(m_pSendData, &header,6);

    if (dwParamLen > 0)
    {
        memcpy(m_pSendData + 6, pParam, dwParamLen); //组合通信消息[消息头+参数内容]
    }

    nSendTimes = nMaxSendTimes;
	do
	{
		if (!CreateAndConnect())
		{		
			if(nSendTimes > 0)
			{
				nSendTimes--;
				continue;
			}
			else
			{
				LogError("===CCMDToCamera::SendDspCMD===CreateAndConnect==ERROR!!\n");
				//解锁
				pthread_mutex_unlock(&m_sockMutex);
				return false;
			}				
		}
		for(int i=0; i<dwParamLen + 6; i++)
		{
		    printf("****====###########=======m_pSendData[%d]=0x%x===\n", i, m_pSendData[i]);
		}
		

		    if(!SafeSend(m_sockThis, m_pSendData, dwParamLen + 6))
		    {
		        //printf("====SafeSend====dwParamLen=%d===\n", dwParamLen);
                //LogError("====SafeSend==err==dwParamLen=%d===\n", dwParamLen);
                bRet = false;
				LogError("===发送命令失败! 命令ID:0x %X.==\n", header.aCmdID);
		    }
		    else
		    {
		        //LogError("====SafeSend=oK===dwParamLen=%d===\n", dwParamLen);
		        bRet = true;
		        //break;
		    }

           // if (!bRet)
            {
                //LogError("===11111===发送命令失败! 命令ID:0x %X.==\n", header.aCmdID);
               // Close();

               // nSendTimes --;
               // continue;
            }

			if (bRet)
			{
				unsigned int dwRecvDataLenTemp = 0;
				if(header.aCmdID == 0x35 || header.aCmdID == 0x64)
				{
					//LogNormal("==SendDspCMD==Rec..before==!");
					nRecTimes = 100;
				}
				else
				{
					nRecTimes = 2;
				}
				do{
					if ( !SafeRecv(m_sockThis, (char*)&dwRecvDataLenTemp, 4) )
					{
						//LogError("==111==SafeRecv===dwRecvDataLenTemp=%d===\n", ntohl(dwRecvDataLenTemp)); //ERROR 这里采用网络字节序
						//LogError("==111==SafeRecv===dwRecvDataLenTemp=%d===\n", dwRecvDataLenTemp);
						usleep(10*1000);
						//sleep(1);
					}
					else
					{
						//LogError("==222==SafeRecv===dwRecvDataLenTemp=%d===\n", dwRecvDataLenTemp);
						break;                    
					}

					if(dwRecvDataLenTemp > 0)
					{
						break;
					}

					nRecTimes--;
				}while(nRecTimes>0);

				unsigned int dwRecvDataLen = dwRecvDataLenTemp;//这里采用网络字节序

				if (dwRecvDataLen > CMD_DATA_BUF_SIZE)
				{
					//LogError("====dwRecvDataLen > CMD_DATA_BUF_SIZE=====\n");
					bRetRecv = false;
				}
				else if(dwRecvDataLen < 1)
				{
					//LogError("====Can't Recv Data!=====\n");
					bRetRecv = false;
				}
				else
				{
					//nRecTimes = 3;
					if(header.aCmdID == 0x35 || header.aCmdID == 0x64)
					{
						//LogNormal("==SendDspCMD==Rec..before==!");
						nRecTimes = 10;
					}
					else
					{
						nRecTimes = 2;
					}

					do
					{
						if(!SafeRecv(m_sockThis, m_pBackData, dwRecvDataLen))
						{
							printf("====SafeRecv===....=\n");
							//LogError("====SafeRecv==ERROR!!=dwRecvDataLen=%d==\n", dwRecvDataLen);
							bRetRecv = false;
							usleep(10*1000);
						}
						else
						{
							bRetRecv = true;
							//LogError("====SafeRecv===.OK...dwRecvDataLen=%d==\n", dwRecvDataLen);
							break;
						}


						nRecTimes --;
					}while(nRecTimes > 0);
				}
			}

        if(!bRet)
        {
            usleep(10*1000);
            nSendTimes --;
        }
        else
        {
           // nSendTimes --;
            break;
        }
    }while(nSendTimes>0);

	 Close();

	 //解锁
	pthread_mutex_unlock(&m_sockMutex);

	return bRet;
}

//DSP相机控制2
BOOL  CCMDToCamera::SendDspCMD2(const DspCmdHeader &header, const char * pParam)
{
	//加锁
	pthread_mutex_lock(&m_sockMutex);

#ifdef DEBUG_DSP_CMD
	//LogNormal("ip:%s, fd:%d SendDspCMD cmd:%x flag:%d \n", m_sTargetIP.c_str(), m_sockThis, header.aCmdID, header.aReplyFlag);
#endif

	memset(m_pBackData, 0, CMD_DATA_BUF_SIZE+1);

    BOOL bRet = false;
    BOOL bRetRecv = false;
    char chTemp[256] = {0};
    int nSendTimes = 1;
    int nRecTimes = 3;
    int nMaxSendTimes = 3;

    unsigned int dwParamLen = 0;
    unsigned int tmp;

    if(header.aParamLen[3] == 0x00 && header.aParamLen[2] == 0x00 \
    && header.aParamLen[1] == 0x00 && header.aParamLen[0] == 0x00)
    {
        dwParamLen = 0;
    }
    else
    {
        tmp = (header.aParamLen[3] & 0x000000FF);
        dwParamLen = tmp<<24;
        tmp = (header.aParamLen[2] & 0x000000FF);
        dwParamLen += tmp<<16;
        tmp = (header.aParamLen[1] & 0x000000FF);
        dwParamLen += tmp<<8;
        tmp = (header.aParamLen[0] & 0x000000FF);
        dwParamLen += tmp;
    }

    memcpy(m_pSendData, &header,6);

    if (dwParamLen > 0)
    {
        memcpy(m_pSendData + 6, pParam, dwParamLen); //组合通信消息[消息头+参数内容]
    }

    nSendTimes = nMaxSendTimes;
	do //begin of do 111
	{
		if (!CreateAndConnect())
		{		
			if(nSendTimes > 0)
			{
				nSendTimes--;
				continue;
			}
			else
			{
				LogError("===CCMDToCamera::SendDspCMD===CreateAndConnect==ERROR!!\n");
				//解锁
				pthread_mutex_unlock(&m_sockMutex);
				return false;
			}				
		}

		
		 if(!SafeSend(m_sockThis, m_pSendData, dwParamLen + 6))
		    {
		        //printf("====SafeSend====dwParamLen=%d===\n", dwParamLen);
                //LogError("====SafeSend==err==dwParamLen=%d===\n", dwParamLen);
                bRet = false;
				//LogError("===发送命令失败! 命令ID:0x %X.==\n", header.aCmdID);
		    }
		    else
		    {
		        //LogError("====SafeSend=oK===dwParamLen=%d===\n", dwParamLen);
		        bRet = true;
		        //break;
		    }

        if (bRet)
        {
			unsigned int dwRecvDataLenTemp = 0;
			DspCmdHeader dspRecHead;

			if(header.aCmdID == 0x35 || header.aCmdID == 0x64 || \
				header.aCmdID == 0x55 || header.aCmdID == 0x5b || header.aCmdID == 0x18)
			{
				//LogNormal("==SendDspCMD==Rec..before==!");
				nRecTimes = 100;
			}
			else
			{
				nRecTimes = 2;
			}
			do{ //begin of do 222
				if ( !SafeRecv(m_sockThis, (char*)&dspRecHead, 6) )
				{
					usleep(10*1000);
				}
				else
				{
					//memcpy((char*)&dwRecvDataLenTemp, (char*)&dspRecHead+2, 4);
					unsigned char uLenth[4];
					memcpy((char*)uLenth, (char*)&dspRecHead+2, 4);

					dwRecvDataLenTemp += uLenth[0] & 0x000000FF;
					dwRecvDataLenTemp += (uLenth[1] & 0x000000FF) << 8;
					dwRecvDataLenTemp += (uLenth[2] & 0x000000FF)  << 16;
					dwRecvDataLenTemp += (uLenth[3] & 0x000000FF)  << 24;

					printf("=SendDspCMD2=dwRecvDataLenTemp=%d=\n", dwRecvDataLenTemp);
					
					memcpy((char *)m_pBackData, (char*)&dspRecHead, 6);								


			//		LogError("==222==SafeRecv===dwRecvDataLenTemp=%d==dspRecHead.aCmdID=0x:%x=dspRecHead.aReplyFlag=0x:%x \n", \
			//			dwRecvDataLenTemp, dspRecHead.aCmdID, dspRecHead.aReplyFlag);

					break;                    
				}

				if(dwRecvDataLenTemp > 0)
				{
					break;
				}

				nRecTimes--;
			}while(nRecTimes>0);//End of do 222

			unsigned int dwRecvDataLen = dwRecvDataLenTemp;//这里采用网络字节序

			if (dwRecvDataLen > CMD_DATA_BUF_SIZE)
			{
				LogError("====dwRecvDataLen > CMD_DATA_BUF_SIZE=====\n");
				bRetRecv = false;
			}
			else if(dwRecvDataLen < 1)
			{
				//LogError("====Can't Recv Data!=====\n");
				if(header.aCmdID == 0x18)//回复不带数据包
				{
					bRetRecv = true;
				}
				else
				{
					bRetRecv = false;
				}				
			}
			else
			{
				//nRecTimes = 3;
				if(header.aCmdID == 0x35 || header.aCmdID == 0x64 \
					|| header.aCmdID == 0x55 || header.aCmdID == 0x5b || header.aCmdID == 0x18)
				{
					//LogNormal("==SendDspCMD==Rec..before==!");
					nRecTimes = 10;
				}
				else
				{
					nRecTimes = 2;
				}

				do
				{
					if(!SafeRecv(m_sockThis, m_pBackData+6, dwRecvDataLen))
					{
						//printf("====SafeRecv===....=\n");
						LogTrace("RecCmdData.log", \
							"====SafeRecv==ERROR!!=dwRecvDataLen=%d==\n", dwRecvDataLen);
						bRetRecv = false;
						usleep(10*1000);
					}
					else
					{
						bRetRecv = true;
						//LogTrace("RecCmdData.log", \
							"====SafeRecv===.OK...dwRecvDataLen=%d==\n", dwRecvDataLen);

						//LogTrace("CMD_rec.log", "##=cmd=0x:%x==\n", dspRecHead.aCmdID);
						/*for(int i=0; i<dwRecvDataLen+6; i++)
						{				
							if( (i+1)%10 == 0 )
							{
								LogTrace("CMD_rec.log", "=[%d]=%x==|\n", i , (*(BYTE*)(m_pBackData + i)));
							}
							else
							{
								LogTrace("CMD_rec.log", "=[%d]=%x==", i , (*(BYTE*)(m_pBackData + i)));
							}
						}	*/

						break;
					}

					nRecTimes --;
				}while(nRecTimes > 0);
			}

		}

        if(!bRet)
        {
            usleep(10*1000);
            nSendTimes --;
        }
        else
        {
            //nSendTimes --;
            break;
        }
    }while(nSendTimes>0); //End of do 111

	 Close();

	 //解锁
	pthread_mutex_unlock(&m_sockMutex);

	return bRet;
}


//DSP相机控制3
BOOL  CCMDToCamera::SendDspCMD3(const DspCmdHeader &header, const char * pParam)
{
	printf("==in SendDspCMD3===\n");
	struct timeval tv1, tv2;
	gettimeofday(&tv1,NULL);

	//加锁
	pthread_mutex_lock(&m_sockMutex);

#ifdef DEBUG_DSP_CMD
	//LogNormal("ip:%s, fd:%d SendDspCMD cmd:%x flag:%d \n", m_sTargetIP.c_str(), m_sockThis, header.aCmdID, header.aReplyFlag);
#endif

	memset(m_pBackData, 0, CMD_DATA_BUF_SIZE+1);

    BOOL bRet = false;
    BOOL bRetRecv = false;
    char chTemp[256] = {0};
    int nSendTimes = 1;
    int nRecTimes = 3;
    int nMaxSendTimes = 3;

    unsigned int dwParamLen = 0;
    unsigned int tmp;

    if(header.aParamLen[3] == 0x00 && header.aParamLen[2] == 0x00 \
    && header.aParamLen[1] == 0x00 && header.aParamLen[0] == 0x00)
    {
        dwParamLen = 0;
    }
    else
    {
        tmp = (header.aParamLen[3] & 0x000000FF);
        dwParamLen = tmp<<24;
        tmp = (header.aParamLen[2] & 0x000000FF);
        dwParamLen += tmp<<16;
        tmp = (header.aParamLen[1] & 0x000000FF);
        dwParamLen += tmp<<8;
        tmp = (header.aParamLen[0] & 0x000000FF);
        dwParamLen += tmp;
    }

	
    //memcpy(m_pSendData, &header,6);
	char *pSendData = NULL;

	if(dwParamLen >= 0 && dwParamLen < CMD_DATA_BIG_BUF_SIZE)
	{
		pSendData = new char[dwParamLen+6];//组合通信消息[消息头+参数内容]
		memset(pSendData, 0, dwParamLen+6);
		memcpy(pSendData, (char*)(&header), 6);
	}


	if(pSendData != NULL)
	{	
		nSendTimes = nMaxSendTimes;
		do //begin of do 111
		{		
			if (!CreateAndConnect())
			{		
				if(nSendTimes > 0)
				{
					nSendTimes--;
					continue;
				}
				else
				{
					LogError("===CCMDToCamera::SendDspCMD===CreateAndConnect==ERROR!!\n");

					if(pSendData != NULL)
					{
						delete [] pSendData;
						pSendData = NULL;
					}
					//解锁
					pthread_mutex_unlock(&m_sockMutex);					
					return false;
				}				
			}

			bRet = SafeSend(m_sockThis, pSendData, 6);

			if(bRet)
			{
				if(pSendData != NULL)
				 {					
					 memcpy(pSendData+6, pParam, dwParamLen);

					 if(!SafeSend(m_sockThis, pSendData+6, dwParamLen))
					 {
						 printf("====SafeSend==err==dwParamLen=%d===\n", dwParamLen);
						 //LogError("====SafeSend==err==dwParamLen=%d===\n", dwParamLen);
						 bRet = false;
						 //LogError("===发送命令失败! 命令ID:0x %X.==\n", header.aCmdID);
					 }
					 else
					 {
						 printf("===send 2222===ok !\n");
						 bRet = true;						 
					 }					 
				}//End of if(pSendData != NULL)

				printf("===send 333==0x[%x]=ok !\n", header.aCmdID);
			}

			 	
			if (bRet)
			{
				unsigned int dwRecvDataLenTemp = 0;
				DspCmdHeader dspRecHead;

				if(header.aCmdID == 0x35 || header.aCmdID == 0x64 \
					|| header.aCmdID == 0x55 || header.aCmdID == 0x5b \
					|| header.aCmdID == 0x5f || header.aCmdID == 0x6f)
				{
					//LogNormal("==SendDspCMD==Rec..before==!");
					nRecTimes = 10;
				}
				else
				{
					nRecTimes = 2;
				}
				do{ //begin of do 222
					if ( !SafeRecv(m_sockThis, (char*)&dspRecHead, 6) )
					{
						usleep(10*1000);
					}
					else
					{
						//memcpy((char*)&dwRecvDataLenTemp, (char*)&dspRecHead+2, 4);
						unsigned char uLenth[4];
						memcpy((char*)uLenth, (char*)&dspRecHead+2, 4);

						dwRecvDataLenTemp += uLenth[0] & 0x000000FF;
						dwRecvDataLenTemp += (uLenth[1] & 0x000000FF) << 8;
						dwRecvDataLenTemp += (uLenth[2] & 0x000000FF)  << 16;
						dwRecvDataLenTemp += (uLenth[3] & 0x000000FF)  << 24;

						printf("=SendDspCMD3=dwRecvDataLenTemp=%d=\n", dwRecvDataLenTemp);
						
						memcpy((char *)m_pBackData, (char*)&dspRecHead, 6);	
						break;                    
					}

					if(dwRecvDataLenTemp > 0)
					{
						break;
					}

					nRecTimes--;
				}while(nRecTimes>0);//End of do 222

				unsigned int dwRecvDataLen = dwRecvDataLenTemp;//这里采用网络字节序

				if(dwRecvDataLen < 1)
				{
					//LogError("====Can't Recv Data!=====\n");
					bRetRecv = false;
				}
				else if (dwRecvDataLen > CMD_DATA_BIG_BUF_SIZE)
				{
					LogError("====dwRecvDataLen > CMD_DATA_BUF_SIZE=====\n");
					bRetRecv = false;
				}				
				else
				{
					//nRecTimes = 3;
					if(header.aCmdID == 0x35 || header.aCmdID == 0x64 \
						|| header.aCmdID == 0x55 || header.aCmdID == 0x5b \
						|| header.aCmdID == 0x5f || header.aCmdID == 0x6f)
					{
						//LogNormal("==SendDspCMD==Rec..before==!");
						nRecTimes = 100;
					}
					else
					{
						nRecTimes = 2;
					}

					bool bRecData = false;

					do
					{
						if(dwRecvDataLen > 0)
						{						
							if(dwRecvDataLen < CMD_DATA_BIG_BUF_SIZE)
							{
								if(m_pBackDataBig != NULL)
								{
									memset(m_pBackDataBig, 0, CMD_DATA_BIG_BUF_SIZE);
									memcpy((char *)m_pBackDataBig, (char*)&dspRecHead, 6);	
									bRecData = SafeRecv(m_sockThis, m_pBackDataBig + 6, dwRecvDataLen);
								}
							}//End of if(dwRecvDataLen < CMD_DATA_BIG_BUF_SIZE) 	
							else
							{
								bRecData = false;
							}
						}										

						if(!bRecData)
						{
							//printf("====SafeRecv===....=\n");
							//LogTrace("RecCmdData.log", \
								"====SafeRecv==ERROR!!=dwRecvDataLen=%d==\n", dwRecvDataLen);
							bRetRecv = false;
							usleep(100*1000);
						}
						else
						{
							bRetRecv = true;
							//LogTrace("RecCmdData.log", \
								"====SafeRecv===.OK...dwRecvDataLen=%d==\n", dwRecvDataLen);

							/*LogTrace("CMD_rec.log", "##=cmd=0x:%x==\n", dspRecHead.aCmdID);
							for(int i=0; i<dwRecvDataLen+6; i++)
							{				
								if( (i+1)%10 == 0 )
								{
									LogTrace("CMD_rec.log", "=[%d]=%x==|\n", i , (*(BYTE*)(m_pBackData + i)));
								}
								else
								{
									LogTrace("CMD_rec.log", "=[%d]=%x==", i , (*(BYTE*)(m_pBackData + i)));
								}
							}
							*/

							break;
						}

						nRecTimes --;
					}while(nRecTimes > 0);
				}//End of else
			}//End of if (bRet)
		
			if(!bRet)
			{
				usleep(10*1000);
				nSendTimes --;
			}
			else
			{
				nSendTimes --;
				break;
			}
			
		}while(nSendTimes>0); //End of do 111

	}//End of if(pSendData != NULL)
	else
	{
		printf("===pSendData==NULL===\n");
	}

	 Close();
	 //解锁
	pthread_mutex_unlock(&m_sockMutex);


	printf("===berore free pSendData!\n");

	if(pSendData != NULL)
	{
		delete [] pSendData;
		pSendData = NULL;

		LogNormal("===after free pSendData!\n");
	}

	printf("==out SendDspCMD3==bRet=%d=\n", bRet);	
	gettimeofday(&tv2,NULL);

	int nDis = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) * 0.001;
	LogNormal("=nDis=[%d] ms==bRet=%d==IP[%s]=\n", nDis, bRet, m_sTargetIP.c_str());

	return bRet;
}
