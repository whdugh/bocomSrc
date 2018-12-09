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

/******************************************************************************/
//   描述:智能交通检测系统服务器
/******************************************************************************/
//头文件
#include "CommonHeader.h"
#include "Common.h"
#include "RoadCommunication.h"
#include "SobeyCommunication.h"
#include "MatchCommunication.h"
#include "SipService.h"
#include "HK_SipService.h"
#include "HikvisionCommunication.h"
#ifdef ZIONSDKVEDIO
#include "ZionCommunication.h"
#endif
#include "CenterServerOneDotEight.h"
#include "CopyToUSB.h"
#include "CFServer.h"
#include "MysqlTransitionSqliteCyToUSB.h"
#include "MvsCommunication.h"
#include "GpsChangeTimeSerial.h"
#include "MonitoringAndAlarm.h"
#include "DoorAlarm.h"
#include "DeviceStatus.h"
#include "DioConnet.h"
#include "XingTaiCenter.h"
#include "SJServer.h"
#include "YiChangCenter.h"
#include "DioComSignalProtocol.h"
#include "WuHanCenter.h"
#include "DioComSignalMachine.h"
#include "DioAlarm.h"
#include "MatchPlate.h"
#include "GpsSerialFortianjin.h"
#include "smallscreenctrl.h"
#include "WatchClient.h"
#include "YiHuaLuCenter.h"
#include "BXServer.h"
//#ifdef FBMATCHPLATE
#include "MatchPlateFortianjin.h"
//#endif

#ifdef YUHUANCENTER_OK
#include "YuHuanCenter.h"
#endif

#ifdef GET_VIDEO_FILE
#include "GetFileSocket.h"
#endif

//主函数入口
int   main(int argc,char* argv[])
{
    #ifdef PRESSURE_TEST
    Detach(0,0);
    #endif
    //KillProcess("pftp",3);
	//信号
	sigset_t sig_new,sig_pending;
	//清空信号集
	sigemptyset(&sig_new);
	//添加信号
	sigaddset(&sig_new,SIGQUIT);
	sigaddset(&sig_new,SIGINT);
	//设置为当前信号集
	sigprocmask(SIG_BLOCK,&sig_new,NULL);

    //更新参数配置文件
	/*CUpdateParaMeter upd_para;
	if(upd_para.IsNeedUpdate())
	{
	    upd_para.DoUpdate();
	    upd_para.DeteUpdate();
	}*/
	//读配置
	ReadIni();

	//载入默认FTP参数
	g_FtpCommunication.SetConfig();

	//连接MYSQL 及 FS 初始化
	if(!g_skpDB.Init())
	{
		LogError("连接MYSQL或者初始化FS失败，系统启动中止！\r\n");
        g_bEndThread = true;
		return -1;
	}

	printf("-----------------------Mysql int-----\n");
	// MySQL初始化完成后才可以记录
	g_skpRoadLog.WriteLog("检测器启动\n",ALARM_CODE_DETECTOR_START,true);

	LogNormal("连接MYSQL 及 FS 初始化成功！\r\n");	

	
	//*********	
    if (!g_skpRoadCommunication.OpenSession())
    {
            LogError("*******失败，系统启动中止！\r\n");
            g_bEndThread = true;
            g_skpDB.UnInit();
            return -1;
     }

	g_WatchClient.Init();

	printf("------------------------disk manage-----\n");
	//磁盘管理
	if(!g_FileManage.Init())
	{
		LogError("启动磁盘管理服务失败，系统启动中止！\r\n");
        g_bEndThread = true;
	    g_skpDB.UnInit();
		return -1;
	}
	LogNormal("启动磁盘管理服务成功！\r\n");

	if(!g_skpRoadMsgCenter.Init())
	{
		LogError("初始化命令处理模块失败，系统启动中止！\r\n");
        g_bEndThread = true;
	    g_FileManage.UnInit();
	    g_skpDB.UnInit();
		return -1;
	}
	LogNormal("启动命令处理模块成功！\r\n");

	
	printf("------------------------communication init-----\n");
	//通讯初始化
	if(!g_skpRoadServer.Init())
	{
		LogError("启动侦听服务失败，系统启动中止！\r\n");
        g_bEndThread = true;
		g_skpRoadMsgCenter.UnInit();
	    g_FileManage.UnInit();
	    g_skpDB.UnInit();
		return -1;
	}
	LogNormal("启动侦听服务成功！\r\n");

	printf("------------------------usb copy-----\n");

 #ifdef GET_VIDEO_FILE
	CGetFileServerSocket skpFileServer;
	if(!skpFileServer.Init())
	{
		LogError("启动侦听下载服务失败，系统启动中止！\r\n");
		
		g_bEndThread = true;
		g_skpRoadServer.UnInit();
		g_skpRoadMsgCenter.UnInit();
		g_FileManage.UnInit();
		g_skpDB.UnInit();
		return -1;
	}
	LogNormal("启动侦听下载服务成功！\r\n");
#endif

	//dio 初始化！
	if(g_SignalSetInfo.nExist == 1)
	{
		if(g_SignalSetInfo.nMode == 0)
		{
			if (g_DioComSetting.nComUse == 10)    // dio 
			{ 
				if(g_SignalSetInfo.nType == 0)
				{
					if(g_DioComSignalProtocol.OpenDev() != 0)
					{
						LogError("打开 ut5510 失败！main");
					}
				}
			}
		}
		else if(g_SignalSetInfo.nMode == 1)
		{
			if( g_dioConnect.GPIO_Init()<0 ) 
			{
				LogNormal("DIO 初始化失败\n");
			}
		}
	}

	

//#ifdef FBMATCHPLATE
	// 前后车牌关联初始化
	if (g_matchPlateFortianjin.InitMatchPlate() < 0)
	{
		LogNormal("前后车牌关联初始化失败\n");
	}
//#endif


	//USB拷贝
	if (!g_copyToUSB.Init())
	{
		LogError("启动USB拷贝服务失败，系统启动中止！\r\n");
		g_bEndThread = true;
		g_skpRoadServer.UnInit();
#ifdef GET_VIDEO_FILE
		skpFileServer.UnInit();
#endif
		g_skpRoadMsgCenter.UnInit();
		g_FileManage.UnInit();
		g_skpDB.UnInit();
		return -1;
	}
	
	/// 本地数据库设备运行状态表
	if (g_nServerType == 13)
	{
		// DIO 报警初始化
		if (g_dioAlarm.InitDio() < 0)
		{
			LogNormal("DIO 报警初始化失败\n");
		}
		g_CheckupDeviceStatus.Init();
	}
	
	
	
	//智能控制器初始化
	if(g_nHasExpoMonitor)
	{
	    if(!g_GateKeeperServer.Init())
	    {
	        LogError("启动监控前端设备状态服务失败，系统启动中止！\r\n");
            g_bEndThread = true;
            g_GateKeeperServer.UnInit();
            return -1;
	    }
	}


	//视频爆闪初始化
	if(g_nFlashControl == 1)
	{
	    FlashSerial* flashSerial=FlashSerial::CreateStance();
        flashSerial->OpenDev();

		
	}

	if (g_DoorComSetting.nComUse == 9)
	{
		g_DoorAlarm.OpenDev();
		//g_AlarmSerial.OpenDev();
	}



	//比对服务器
	if(g_MatchHostInfo.uHasMatchHost == 1)
	{
        g_MatchCommunication.Init();
	}


#ifdef HK_SIP_SERVICE_OPEN
	//初始化SIP视频服务
	if(!g_hk_sipService.Init())
	{
		LogError("初始化HKSIP视频服务失败！\r\n");
		cerr<<"初始化HKSIP视频服务失败！"<<endl;
		return -1;
	}
	else
	{
		LogNormal("初始化HKSIP视频服务成功！\r\n");
		cerr<<"初始化HKSIP视频服务成功！"<<endl;
	}
#endif

	if (g_DistanceHostInfo.bDistanceCalculate == 1)
	{
		if (!g_mvsCommunication.Init())
		{
			LogError("初始化检测器之间通讯失败!\r\n");
			return -1;
		}
		else
		{
			LogNormal("初始化检测器之间通讯成功\r\n");
		}
	}


	#ifdef CFCAMERA
	if(!g_CFCommunication.Init())
	{
		LogError("初始化CF视频服务失败！\r\n");
		return -1;
	}
	#endif

	#ifdef HIKVISIONCAMERA
	if(!g_HikvisionCommunication.Init())
	{
		LogError("初始化HK模块失败，系统启动中止！\r\n");
		return -1;
	}
	#endif

#ifdef ZIONSDKVEDIO
	if(!g_ZionCommunication.Init())
	{
		LogError("初始化Zion模块失败，系统启动中止！\r\n");
		return -1;
	}
#endif


    if (1 == g_nHasCenterServer)
    {
        if (2 == g_nServerType)
        {
            g_CenterServer.Init();
        }
        //旅行时间中心端
        else if (3 == g_nServerType || 22 == g_nServerType)
        {
            g_TravelServer.Init();
        }
        else if(5 == g_nServerType)
        {
            g_LSServer.Init();
	#ifdef LS_QINGTIAN_IVAP
			g_BocomServerManage.Init();				
	#endif
        }
        else if(10 == g_nServerType || 7 == g_nServerType || 21 == g_nServerType || 23 == g_nServerType || 24 == g_nServerType) //鞍山电子警察
        {
            printf("===g_nServerType=%d==g_TJServer.Init()==\n", g_nServerType);
            g_TJServer.Init();
			//g_BocomServerManage.Init();
        }
        else if ( 0 == g_nServerType || 4 == g_nServerType || 6 == g_nServerType || 26 == g_nServerType)
        {
			g_BocomServerManage.Init();
        }
        else if(8 == g_nServerType)
        {
            g_TelComServer.Init();
        }
		else if(9 == g_nServerType)
        {
            g_HttpCommunication.Init();
        }
		else if(11 == g_nServerType)
        {
            g_BJServer.Init();
        }
		else if(12 == g_nServerType)
		{
			g_BaoKangServer.Init();
		}
		else if (13 == g_nServerType)
		{
			g_MyCenterServer.Init(); 				
		}
		else if (15 == g_nServerType)
		{
			g_XingTaiCenter.Init();
		}
		else if (16 == g_nServerType)
		{
			g_SJServer.Init();
		}
		/*else if (17 == g_nServerType)
		{
			g_YiChangCenter.Init();
		}*/
		else if (18 == g_nServerType)   // add yichang  
		{
			g_YiChangCenter.Init();
		}
		else if (19 == g_nServerType)
		{
			g_WuHanCenter.Init();			
		}
		else if (20 == g_nServerType)
		{
			#ifdef YUHUANCENTER_OK
			g_YuHuanCenter.Init();
			#endif
		}
		else if (29 == g_nServerType)
		{
			g_HttpUpload.Init();	
		}
		else if (30 == g_nServerType)
		{
			g_BXServer.Init();
			//LogNormal("宝信平台接入服务进入!\r\n");
		}
		if (23 == g_nServerType || 26 == g_nServerType)
		{
			g_YiHuaLuCenter.Init();
		}

    }
     //打开vis控制串口
     if(g_nKeyBoardID >0 && g_ytControlSetting.nNeedControl == 1)
     {
         if(g_nControlMode == 0)
         {
             g_VisSerialCommunication.OpenDev();
         }
         else
         {
             g_VisNetCommunication.mvConnOrLinkTest();
         }
         g_VisKeyBoardControl.BeginThread();


		 g_MVSToSerialServer.mvConnOrLinkTest();
		 g_MVSToSerialServer.testConnectThread();
     }

	 if(g_nClockMode == 1)
	 {
		 switch (g_GpsSetInfo.nType)
		 {
		 case 0:
			 g_GpsSerial.OpenDev();//清华紫光
			 break;
		 case 1:
			 g_GpsChangeTime.OpenDev();//上海交警用
			 break;
		 case 2:
			 g_GpsSerialFortianjin.OpenDev();//天津交警用
			 break;
		 default:
			 break;
		 }
	 }

	 GlobalInit();//算法全局初始化

	//处理视频采集
	if(!g_skpChannelCenter.Init())
	{
		LogError("启动视频采集处理服务失败，系统启动中止！\r\n");
        g_bEndThread = true;
     
        g_skpRoadServer.UnInit();
#ifdef GET_VIDEO_FILE
		skpFileServer.UnInit();
#endif
		g_skpRoadMsgCenter.UnInit();
	    g_FileManage.UnInit();
	    g_skpDB.UnInit();
		return -1;
	}
	LogNormal("启动视频采集处理服务成功！\r\n");

	if ( (1 == g_nHasCenterServer) && 
		(1 == g_nServerType || g_AmsHostInfo.uHasAmsHost > 0) )
	{
		//初始化AMS控制服务器通信模块
		g_AMSCommunication.Init();
		//获取AMS上的配置文件替换到本地
		g_AMSCommunication.mvConnOrLinkTest(true);
	}
	WriteMvsVersionToFile();
	LogNormal("系统必要初始化完成，可以开始提供服务...\r\n");

#ifdef SNMP_AGENT
	if(!g_SnmpAgent.Init())
	{
		LogError("SNMP代理服务启动失败，系统启动中止！\r\n");
	}
#endif
#ifdef SIP_SERVICE_OPEN
	if (g_VideoFormatInfo.nSip >0)
	{
		//初始化SIP视频服务
		if(!g_sipService.Init())
		{
			LogError("初始化SIP视频服务失败！\r\n");
			cerr<<"初始化SIP视频服务失败！"<<endl;
			return -1;
		}
		else
		{
			LogNormal("初始化SIP视频服务成功！\r\n");
			cerr<<"初始化SIP视频服务成功！"<<endl;
		}
	}
#endif


	#ifdef MONITOR
	g_SobeyCommunication.Init();
	#endif

	if(g_ScreenComSetting.nComUse == 11)
	{
		g_SmallScreenCtrl.OpenDev();
	}
	

	//信号处理
	while(!g_bEndThread)
	{
		sigpending(&sig_pending);
		if(sigismember(&sig_pending,SIGQUIT) || sigismember(&sig_pending,SIGINT))
		{
		    LogError("sigpending(&sig_pending);\n");
			break;
		}

        //printf("before mvConnOrLink = %u\n",GetTimeStamp());
        if(1 == g_nHasCenterServer)
        {
			if ( 1 == g_nServerType || g_AmsHostInfo.uHasAmsHost > 0)
			{
				g_AMSCommunication.mvConnOrLinkTest();
			}

            if ( 2 == g_nServerType)
            {
                g_CenterServer.mvConnOrLinkTest();
            }
            else if ( 3 == g_nServerType)
            {
                g_TravelServer.mvConnOrLinkTest();
            }
            else if(5 == g_nServerType)
            {
                g_LSServer.mvSendLinkTest();
            }
            else if(8 == g_nServerType)
            {
                g_TelComServer.mvConnOrLinkTest();
            }
			else if(9 == g_nServerType)
            {
                g_HttpCommunication.mvConnOrLinkTest();
            }
        }

#ifdef SIP_SERVICE_OPEN
		if (g_VideoFormatInfo.nSip >0)
		{
			//SIP视频服务器开启服务
			g_sipService.mvConnOrLinkTest();
		}
#endif

        if(g_nKeyBoardID >0 && (1 == g_nControlMode) && g_ytControlSetting.nNeedControl == 1)
        {
            g_VisNetCommunication.mvConnOrLinkTest();
			
			g_MVSToSerialServer.mvConnOrLinkTest();
			//g_MVSToSerialServer.testConnectThread();
        } 
        //printf("before GetSysInfo = %u\n",GetTimeStamp());
		//获取主机状态
		GetSysInfo();
        //printf("after GetSysInfo = %u\n",GetTimeStamp());

		if(g_nHasExpoMonitor) //获取环境监控设备信息
		{
            //LogNormal("=======MAIN===g_GateKeeperServer.mvSendLinkTest()======\n");
            //心跳测试
            if(!g_GateKeeperServer.mvSendLinkTest())
            {
                //LogNormal("=======main: g_GateKeeperServer LinkTest ERROR!!\n");
            }
            //g_GateKeeperServer.mvGetGateKeeperInfo();
		}

		//比对服务器
        if(g_MatchHostInfo.uHasMatchHost == 1)
        {
            g_MatchCommunication.mvConnOrLinkTest();
        }

		sleep(5);
	}
	//退出处理

	//线程退出
	g_bEndThread = true;

	LogNormal("系统退出，开始清理数据......\r\n");

	g_WatchClient.UnInit();

	//检测释放
	g_skpChannelCenter.UnInit();
	LogNormal("通道检测服务退出完成!\r\n");

	//服务释放
	g_skpRoadServer.UnInit();
#ifdef GET_VIDEO_FILE
	skpFileServer.UnInit();
#endif
	LogNormal("侦听服务退出完成!\r\n");
	//消息中心释放
	g_skpRoadMsgCenter.UnInit();
	LogNormal("命令处理中心退出完成!\r\n");
	//停止磁盘清理线程
	g_FileManage.UnInit();
	LogNormal("通道磁盘管理服务退出完成!\r\n");

	//关闭虚拟串口
	{
        g_skpRoadCommunication.CloseSession();
        LogNormal("关闭虚拟串口完成!\r\n");
    }

    //关闭爆闪灯串口
    if(g_nFlashControl == 1)
	{
	    FlashSerial* flashSerial=FlashSerial::CreateStance();
        flashSerial->Close();
        free(flashSerial);
        flashSerial = NULL;
	}

	if (g_DoorComSetting.nComUse == 9)
	{
		g_DoorAlarm.Close();
		//g_AlarmSerial.Close();
		
	}

	if(g_SignalSetInfo.nExist == 1)
	{
		if(g_SignalSetInfo.nMode == 0)
		{
			if (g_DioComSetting.nComUse == 10)
			{
				if(g_SignalSetInfo.nType == 0)
				{
					g_DioComSignalProtocol.Close();
				}
				else
				{
					//g_DioComSignalProtocol2.Close();
				}
			}
		}
		else if(g_SignalSetInfo.nMode == 1)
		{
			g_dioConnect.GPIO_unInit();
		}
	}

    //关闭VIS串口
    g_VisSerialCommunication.Close();

    //关闭Gps串口
    if(g_nClockMode == 1)
    {
		if(g_nClockMode == 1)
		{
			if(g_GpsSetInfo.nType == 0)
		 {
			 g_GpsSerial.Close();
		 }
			else
		 {
			 g_GpsChangeTime.Close();
		 }

		}
        
    }

    //比对服务器
	if(g_MatchHostInfo.uHasMatchHost == 1)
	{
        g_MatchCommunication.UnInit();
	}

#ifdef SIP_SERVICE_OPEN
	//注销SIP视频服务
	g_sipService.UnInit();
#endif

#ifdef HK_SIP_SERVICE_OPEN
	//注销HKSIP视频服务
	g_hk_sipService.UnInit();
#endif

#ifdef CFCAMERA
	g_CFCommunication.UnInit();
#endif

	#ifdef HIKVISIONCAMERA
	g_HikvisionCommunication.UnInit();
	#endif

#ifdef ZIONSDKVEDIO
	g_ZionCommunication.UnInit();
#endif


    if(1 == g_nHasCenterServer)
    {
        //中心端清理
        if (1 == g_nServerType || g_AmsHostInfo.uHasAmsHost > 0)
        {
            g_AMSCommunication.UnInit();
        }
        
		if (2 == g_nServerType)
        {
            g_CenterServer.UnInit();
        }
        else if(3 == g_nServerType || 22 == g_nServerType)
        {
            g_TravelServer.UnInit();
        }
        else if(5 == g_nServerType)
        {
            g_LSServer.UnInit();
		#ifdef LS_QINGTIAN_IVAP
			g_BocomServerManage.UnInit();
		#endif
        }
        else if(10 == g_nServerType || 7 == g_nServerType || 21 == g_nServerType)
        {
            g_TJServer.UnInit();
        }
        else if ( 0 == g_nServerType || 4 == g_nServerType || 6 == g_nServerType || 26 == g_nServerType)
        {
			g_BocomServerManage.UnInit();
        }
        else if(8 == g_nServerType)
        {
            g_TelComServer.UnInit();
        }
		else if(9 == g_nServerType)
        {
            g_HttpCommunication.UnInit();
        }
		else if(11 == g_nServerType)
        {
            g_BJServer.UnInit();
        }
		else if(12 == g_nServerType)
		{
			g_BaoKangServer.UnInit();
		}
		else if (13 == g_nServerType)
		{
			g_MyCenterServer.UnInit(); 				
		}
		else if (15 == g_nServerType)
		{
			g_XingTaiCenter.UnInit();
		}
		else if (16 == g_nServerType)
		{
			g_SJServer.UnInit();
		}
		else if (18 == g_nServerType)  // add yichang 
		{
			g_YiChangCenter.Unit();
		}
		else if (19 == g_nServerType)
		{
			g_WuHanCenter.Uninit();
		}
		else if (20 == g_nServerType)
		{
			#ifdef YUHUANCENTER_OK
			g_YuHuanCenter.Uninit();
			#endif
		}
		else if(29 == g_nServerType)
		{
			g_HttpUpload.UnInit();
		}
		else if (30 == g_nServerType)
		{
			g_BXServer.UnInit();
			LogNormal("宝信平台接入服务退出!\r\n");
		}
		if (23 == g_nServerType || 26 == g_nServerType)
		{
			g_YiHuaLuCenter.UnInit();
		}
    }

#ifdef MONITOR
	g_SobeyCommunication.UnInit();
#endif

	if (g_DistanceHostInfo.bDistanceCalculate == 1)
	{
		g_mvsCommunication.UnInit();
	}

    //环境监控设备信息释放
    if(g_nHasExpoMonitor)
    g_GateKeeperServer.UnInit();


	g_copyToUSB.Close();

	//DB FS 释放
	LogNormal("DB连接、FS模块退出完成!\r\n");
	g_skpRoadLog.WriteLog("检测器停止\n",ALARM_CODE_DETECTOR_HALT,true);
	g_skpDB.UnInit();

	return 0;
}
